/* ***************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare,
 *  Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence 
 *  Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * ***************************************************************************
 *  sharedLibraryInterfaceExample.cpp 
 *
 *  Sample code for running an agent with the shared library interface. 
 ************************************************************************** */

#include <iostream>
#include <SDL.h>
#include <unistd.h>
#include <ale_interface.hpp>
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "video/videofuncs.h"

using namespace std;

int main(int argc, char** argv) {
   if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
      return 1;
   }

   ALEInterface ale;
   nrfunc::init_time_based_seed();

/*
enum Action {
    PLAYER_A_NOOP           = 0,
    PLAYER_A_FIRE           = 1,
    PLAYER_A_UP             = 2,
    PLAYER_A_RIGHT          = 3,
    PLAYER_A_LEFT           = 4,
    PLAYER_A_DOWN           = 5,
    PLAYER_A_UPRIGHT        = 6,
    PLAYER_A_UPLEFT         = 7,
    PLAYER_A_DOWNRIGHT      = 8,
    PLAYER_A_DOWNLEFT       = 9,
    PLAYER_A_UPFIRE         = 10,
    PLAYER_A_RIGHTFIRE      = 11,
    PLAYER_A_LEFTFIRE       = 12,
    PLAYER_A_DOWNFIRE       = 13,
    PLAYER_A_UPRIGHTFIRE    = 14,
    PLAYER_A_UPLEFTFIRE     = 15,
    PLAYER_A_DOWNRIGHTFIRE  = 16,
    PLAYER_A_DOWNLEFTFIRE   = 17,
}
*/
 
// Breakout: 0, 1, 3, 4
// Pong:  0, 1, 3, 4, 11, 12
// Space invaders: 0, 1, 3, 4, 11, 12
// Enduro: 0, 1, 3, 4, 5, 8, 9, 11, 12
// Ms pacman: 0, 2, 3, 4, 5, 6, 7, 8, 9

  // Get & Set the desired settings
   ale.setInt("random_seed", 123);
   //The default is already 0.25, this is just an example
//   ale.setFloat("repeat_action_probability", 0.25);
   ale.setFloat("repeat_action_probability", 0);

//   ale.setBool("display_screen", false);
   ale.setBool("display_screen", true);
//   ale.setBool("sound", true);


   // Load the ROM file. (Also resets the system for new settings to
   // take effect.)
   ale.loadROM(argv[1]);

   // Get the vector of legal actions
   ActionVect legal_actions = ale.getLegalActionSet();
   cout << "legal_actions.size() = " << legal_actions.size() << endl;

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

   string output_subdir="./ale_frames/";
   filefunc::dircreate(output_subdir);

   int min_px = 0;
   int max_px = 160;
   int min_py = 0;
   int max_py = 210;

// Screen ROI for Pong:

   min_px = 16;
   max_px = 144;
   min_py = 34;
   max_py = 194;

// Screen ROI for Breakout:

/*

// Buggy breakout_1978.bin which has life counter:

   int min_px = 6;
   int max_px = 156;
   int min_py = 40;
   int max_py = 240;
*/

/*
// good values for breakout:

   int min_px = 23;
   int max_px = 129;
   int min_py = 16;
   int max_py = 175;
*/

// Screen ROI for space invaders:

//   int min_px = 27;
//   int max_px = 133;
//   int min_py = 12;
//   int max_py = 190;

// (max_px - min_px) * (max_py - min_py) =
// (133 - 27) * (190 - 12) = 
// 106 * 178
// --> 53 * 89

   int action_counter = 0;
   int n_ups = 0;
   int n_downs = 0;

   // Play episodes
   int n_episodes = 10;

   for (int episode = 0; episode< n_episodes; episode++) {
      cout << "Starting episode " << episode << endl;
      float totalReward = 0;
      int life_frame_counter = 0;
      int n_prev_lives = -1;
      while (!ale.game_over()) {

         int n_curr_lives = ale.lives();
         if(n_curr_lives != n_prev_lives)
         {
            life_frame_counter =  0;
            n_prev_lives = n_curr_lives;
         }
         
         int curr_frame_number = ale.getEpisodeFrameNumber();
         life_frame_counter++;
         grayscale_output_buffer.clear();
         ale.getScreenGrayscale(grayscale_output_buffer);

         if(curr_frame_number % 3 == 0)
         {
            vector<vector<unsigned char > > byte_array;
            for(int py = min_py; py < max_py; py++)
            {
               vector<unsigned char> curr_byte_row;
               for(int px = min_px; px <  max_px; px++)
               {
                  int curr_pixel = px + py * width;
                  curr_byte_row.push_back(grayscale_output_buffer[
                                             curr_pixel]);
               }
               byte_array.push_back(curr_byte_row);
            }

            string output_frame = output_subdir+"frame_"+
               stringfunc::integer_to_string(curr_frame_number,5)+
               ".png";
            videofunc::write_8bit_greyscale_pngfile(byte_array, output_frame);
         } // curr_frame_number % 3 == 0 conditional
         
         int action_index = 0; // no operation
         if(life_frame_counter < 60)
         {
//            action_index = 1;  // fire ball
         }
         else
         {
/*
            if(nrfunc::ran1() < 0.5)
            {
               action_index = 3;   // right move
//               action_index = 2;   // up move
            }
            else
            {
               action_index = 4;   // right move
//               action_index = 5;   // down move
            }
*/

         }

         if(n_ups < 14 && life_frame_counter > 10 
            && life_frame_counter % 4 == 0)
         {
            action_index = 3;
            n_ups++;
         }

/*
         if(n_downs < 18 && life_frame_counter > 10 
            && life_frame_counter % 4 == 0)
         {
            action_index = 4;
            n_downs++;
         }
*/

         action_counter++;
         cout << "action_counter = " << action_counter 
              << " n_ups = " << n_ups 
              << " n_downs = " << n_downs
              << endl;
         usleep(200 * 1000);

// As with many other Atari games, the player paddle also moves every
// other frame, adding a degree of temporal aliasing to the domain

//         action_index = curr_frame_number / 10;

//         action_index = rand() % legal_actions.size();

         Action a = legal_actions[action_index];

         // Apply the action and get the resulting reward
         float reward = ale.act(a);
         totalReward += reward;

         cout << "Episode = " << episode 
              << " life_frame_counter = " << life_frame_counter
//               << " episode frame = " << ale.getEpisodeFrameNumber()
              << " action_index = " << action_index
//              << " n_lives = " << ale.lives() 
//              << " life_frame_counter = " << life_frame_counter 
              << endl;

//         cout << "frame = " << curr_frame_number
//              << " curr_reward = " << reward
//              << " total_reward = " << totalReward << endl;
      }
      cout << "Episode " << episode << " ended with score: " 
           << totalReward << endl;
      ale.reset_game();
   }

   return 0;
}

