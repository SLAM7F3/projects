// ==========================================================================
// Program SPACEINV
// ==========================================================================
// Last updated on 12/1/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "video/videofuncs.h"

int main(int argc, char** argv) 
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   
   ALEInterface ale;
 
//  enum Action {
//  PLAYER_A_NOOP           = 0,
//  PLAYER_A_FIRE           = 1,
//  PLAYER_A_UP             = 2,
//  PLAYER_A_RIGHT          = 3,
//  PLAYER_A_LEFT           = 4,
//  PLAYER_A_DOWN           = 5,
//  PLAYER_A_UPRIGHT        = 6,
//  PLAYER_A_UPLEFT         = 7,
//  PLAYER_A_DOWNRIGHT      = 8,
//  PLAYER_A_DOWNLEFT       = 9,
//  PLAYER_A_UPFIRE         = 10,
//  PLAYER_A_RIGHTFIRE      = 11,
//  PLAYER_A_LEFTFIRE       = 12,
//  PLAYER_A_DOWNFIRE       = 13,
//  PLAYER_A_UPRIGHTFIRE    = 14,
//  PLAYER_A_UPLEFTFIRE     = 15,
//  PLAYER_A_DOWNRIGHTFIRE  = 16,
//  PLAYER_A_DOWNLEFTFIRE   = 17,
//  }

// Space invaders actions: 0, 1, 3, 4, 11, 12

   nrfunc::init_time_based_seed();
   ale.setInt("random_seed", int(10000 * nrfunc::ran1()));
//   ale.setInt("random_seed", 123);

   //The default is already 0.25, this is just an example
   ale.setFloat("repeat_action_probability", 0.25);

   ale.setBool("display_screen", false);
//   ale.setBool("display_screen", true);
//   ale.setBool("sound", true);

   // Load the ROM file. (Also resets the system for new settings to
   // take effect.)
   ale.loadROM(argv[1]);

   // Get the vector of legal actions
   ActionVect legal_actions = ale.getLegalActionSet();
   ActionVect minimal_actions = ale.getMinimalActionSet();
 
   for(unsigned int a = 0; a < minimal_actions.size(); a++)
   {
      cout << "a = " << a << " minimal_action = " << minimal_actions[a] 
           << endl;
   }

   ALEScreen curr_screen = ale.getScreen();
   int width = curr_screen.width();
   int height = curr_screen.height();
   cout << "curr_screen.width = " << width
        << " curr_screen.height = " << height << endl;

   int n_pixels = curr_screen.width() * curr_screen.height();
   vector<unsigned char> grayscale_output_buffer;
   grayscale_output_buffer.reserve(n_pixels);


// Screen ROI for space invaders:

   int min_px = 22;
   int max_px = 138;
   int min_py = 30;
   int max_py = 192;
   int min_episode_framenumber = 100;
   int frame_skip = 3;

   int curr_frame_number = -1;
//   int n_episodes = 10;
   int n_episodes = 100;
//   bool display_frames = true;
   bool display_frames = false;

   string output_subdir="./cropped_frames/";
   filefunc::dircreate(output_subdir);
   string orig_subdir=output_subdir+"orig_frames/";
   filefunc::dircreate(orig_subdir);
   string pooled_subdir=output_subdir+"pooled_frames/";
   filefunc::dircreate(pooled_subdir);
   string differenced_subdir=output_subdir+"differenced_frames/";
   filefunc::dircreate(differenced_subdir);
   int difference_counter = 0;

   vector<vector<unsigned char > > pooled_byte_array0;
   vector<vector<unsigned char > > pooled_byte_array1;
   vector<vector<unsigned char > >* pooled_byte_array_ptr;
   vector<vector<unsigned char > >* other_pooled_byte_array_ptr;

   vector<double> z_differences;
   vector<double> scores;

   for(int episode = 0; episode< n_episodes; episode++) 
   {
      cout << "Starting episode " << episode << endl;
      float totalReward = 0;
      while (!ale.game_over()) {

         curr_frame_number = ale.getEpisodeFrameNumber();
         if(curr_frame_number > min_episode_framenumber)
         {
            if(curr_frame_number % frame_skip == 0)
            {

// Retrieve curr frame's greyscale pixel values:

               grayscale_output_buffer.clear();
               ale.getScreenGrayscale(grayscale_output_buffer);

// Crop out center RoI from current frame:

               vector<vector<unsigned char > > byte_array;
               for(int py = min_py; py < max_py; py++)
               {
                  vector<unsigned char> curr_byte_row;
                  for(int px = min_px; px <  max_px; px++)
                  {
                     int curr_pixel = px + py * width;
                     curr_byte_row.push_back(grayscale_output_buffer[
                                                curr_pixel]);
                  } // loop over px
                  byte_array.push_back(curr_byte_row);
               } // loop over py

               if(display_frames)
               {
                  string output_frame = orig_subdir+"frame_"+
                     stringfunc::integer_to_string(curr_frame_number,5)+
                     ".png";
                  videofunc::write_8bit_greyscale_pngfile(
                     byte_array, output_frame);
               }

// 2x2 max pool cropped center for current frame:
        
               if(difference_counter == 0)
               {
                  pooled_byte_array_ptr = &pooled_byte_array0;
                  other_pooled_byte_array_ptr = &pooled_byte_array1;
               }
               else
               {
                  pooled_byte_array_ptr = &pooled_byte_array1;
                  other_pooled_byte_array_ptr = &pooled_byte_array0;
               }

               for(unsigned int r = 0; r < byte_array.size(); r += 2)
               {
                  vector<unsigned char> pooled_byte_row;
                  for (unsigned int c = 0; c < byte_array[0].size(); c+= 2)
                  {
                     unsigned char z00 = byte_array[r][c];
                     unsigned char z01 = byte_array[r][c+1];
                     unsigned char z10 = byte_array[r+1][c];
                     unsigned char z11 = byte_array[r+1][c+1];
                     pooled_byte_row.push_back(
                        basic_math::max(z00, z01, z10, z11));
                  } // loop over index c
                  pooled_byte_array_ptr->push_back(pooled_byte_row);
               } // loop over index r

               if(display_frames)
               {
                  string pooled_frame = pooled_subdir+"pooled_frame_"+
                     stringfunc::integer_to_string(curr_frame_number,5)+
                     ".png";
                  videofunc::write_8bit_greyscale_pngfile(
                     *pooled_byte_array_ptr, pooled_frame);
               }

// Compute differences between current and other pooled byte arrays:

               vector<vector<unsigned char > > diff_pooled_byte_array;
               if(other_pooled_byte_array_ptr->size() > 0)
               {

                  for(unsigned int py = 0; py < pooled_byte_array_ptr->size(); 
                      py++)
                  {
                     vector<unsigned char> diff_pooled_byte_row;
                     for(unsigned int px = 0; 
                         px < pooled_byte_array_ptr->at(0).size(); px++)
                     {
                        unsigned char z_diff = 
                           pooled_byte_array_ptr->at(py).at(px) - 
                           other_pooled_byte_array_ptr->at(py).at(px);
                        diff_pooled_byte_row.push_back(z_diff);
                        z_differences.push_back(double(z_diff));
                     } // loop over px
                     diff_pooled_byte_array.push_back(diff_pooled_byte_row);
                  } // loop over py 

                  if(display_frames)
                  {
                     string diff_frame = differenced_subdir+
                        "differenced_frame_"+
                        stringfunc::integer_to_string(curr_frame_number,5)+
                        ".png";
                     videofunc::write_8bit_greyscale_pngfile(
                        diff_pooled_byte_array, diff_frame);
                  }
               } // other_pooled_byte_array.size > 0 conditional
               
// Clear other pooled byte array:

               for(unsigned int py = 0; 
                   py < other_pooled_byte_array_ptr->size(); py++)
               {
                  other_pooled_byte_array_ptr->at(py).clear();
               }
               other_pooled_byte_array_ptr->clear();

               difference_counter = 1 - difference_counter;
            } // curr_frame_number % frame_skip == 0 conditional
         } // curr_frame_number > min_episode_framenumber
         
         Action a = legal_actions[rand() % legal_actions.size()];
         // Apply the action and get the resulting reward
         float reward = ale.act(a);
         totalReward += reward;
      } // !game_over conditional

      cout << "Episode " << episode << " ended with score: " 
           << totalReward << endl;
      scores.push_back(totalReward);
      cout << " Final episode frame number = " << curr_frame_number << endl;
      ale.reset_game();
   } // loop over episodes
   

   double mu, sigma;
   mathfunc::mean_and_std_dev(z_differences, mu, sigma);
   cout << "z_difference = " << mu << " +/- " << sigma << endl;

   mathfunc::mean_and_std_dev(scores, mu, sigma);
   double median, qw;
   mathfunc::median_value_and_quartile_width(scores, median, qw);

   cout << "score = " << mu << " +/- " << sigma << endl;
   cout << "median = " << median << " quartile width = " << qw << endl;
   cout << "max score = " << mathfunc::maximal_value(scores) << endl;

// z_difference = 0.937752 +/- 11.0019
// z_difference = 0.936237 +/- 10.9916
// z_difference = 0.951648 +/- 11.0808
// z_difference = 0.944325 +/- 11.0395

// score = 147.6 +/- 98.1898
// score = 169.6 +/- 115.999
// score = 160.55 +/- 95.4513
// median = 155 quartile width = 55
// median = 135 quartile width = 52.5

// max score = 605
// max score = 640
// max score = 660

   return 0;
}

