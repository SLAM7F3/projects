// ==========================================================================
// pong class member function definitions
// ==========================================================================
// Last modified on 12/17/16; 12/18/16; 12/26/16; 12/30/16
// ==========================================================================

// Notes: 

// 1.  On 12/16/16, we explicitly verified that the ALE method act(a)
// faithfully executes left/right paddle movements provided that the
// ALE parameter repeat_action_probability = 0 !  

// 2.  The extreme left location for the paddle is NOT against the
// left wall of the gameboard.  Instead, it is a black space position
// located to the right of the left wall.  On the other hand, the
// extreme right position for the paddle is against the right wall of
// the gameboard.  

#include <iostream>
#include <string>
#include "math/basic_math.h"
#include "games/pong.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void pong::initialize_output_subdirs()
{
   output_subdir="./cropped_frames/";
   filefunc::dircreate(output_subdir);
   orig_subdir=output_subdir+"orig_frames/";
   filefunc::dircreate(orig_subdir);
   pooled_subdir=output_subdir+"pooled_frames/";
   filefunc::dircreate(pooled_subdir);
   differenced_subdir=output_subdir+"differenced_frames/";
   filefunc::dircreate(differenced_subdir);
   maxed_subdir=output_subdir+"maxed_frames/";
   filefunc::dircreate(maxed_subdir);
}

void pong::initialize_grayscale_output_buffer()
{
   ALEScreen curr_screen = ale.getScreen();
   screen_width = curr_screen.width();
   screen_height = curr_screen.height();
   int n_pixels = screen_width * screen_height;
   grayscale_output_buffer.reserve(n_pixels);
}

void pong::initialize_member_objects()
{
//    cout << "inside pong::init_member_objs()" << endl;

   forced_game_over = false;
   compute_difference_flag = true;
   compute_max_flag = false;
   int random_seed = 1000 * nrfunc::ran1();
   ale.setInt("random_seed", random_seed);

   ale.setFloat("repeat_action_probability", 0);
   ale.setBool("display_screen", false);
//   ale.setBool("display_screen", true);
   ale.loadROM("/usr/local/ALE/roms/pong.bin");

// No screen content influencing game play appears outside following
// pixel bbox:

// ALE supported pong ROM

   min_px = 16;
   max_px = 144;
   min_py = 34;
   max_py = 194;

   cskip = 4;
   rskip = 5;
   n_reduced_xdim = (max_px - min_px) / cskip;   // 128 / 4 = 32
   n_reduced_ydim = (max_py - min_py) / rskip;   // 160 / 5 = 32
   n_reduced_pixels = n_reduced_xdim * n_reduced_ydim;  // 1024

// No screen content influencing game play appears before following
// episode frame number:

//   min_episode_framenumber = 0;
   min_episode_framenumber = 60;
   prev_framenumber = -1;

   screen_state_counter = 0;

   mu_z = 40.5;		// 12/17/16 estimate from 50 random episodes
   sigma_z = 60.8;      // 12/17/16 estimate from 50 random episodes

   mu_zdiff = 0.10;     // Estimate from 50 random episodes
   sigma_zdiff = 3.7;   // Estimate from 50 random episodes
//   median_zdiff = 0;  // Estimate from 50 random episodes
//   qw_zdiff = 0;      // Estimate from 50 random episodes

   difference_counter = 0;
   d_paddle = 0;
   max_paddle_x_size = 100 * 1000;
   for(int d = 0; d < max_paddle_x_size; d++)
   {
      paddle_x_values.push_back(-999);
   }
   paddle_x_values_filled = false;

   initialize_output_subdirs();
   initialize_grayscale_output_buffer();
}		       

void pong::allocate_member_objects()
{
   screen0_state_ptr = new genvector(n_reduced_pixels);
   screen1_state_ptr = new genvector(n_reduced_pixels);
   curr_state_ptr = screen0_state_ptr;
   next_state_ptr = screen1_state_ptr;

   for(int s = 0; s < n_screen_states; s++)
   {
      genvector* curr_screen_state_ptr = new genvector(n_reduced_pixels);
      curr_screen_state_ptr->clear_values();
      screen_state_ptrs.push_back(curr_screen_state_ptr);
   }

   curr_big_state_ptr = new genvector(n_screen_states * n_reduced_pixels);
   curr_big_state_ptr->clear_values();
}		       

// ---------------------------------------------------------------------
// Member function update_curr_big_state() assembles the contents of
// screen_state_ptrs[screen_state_counter],
// screen_state_ptrs[screen_state_counter+1 mod n_screen_states], ...,
// screen_state_ptrs[screen_state_counter+n_screen_states-1 mod
// n_screen_states] into member genvector *curr_big_state_ptr.

genvector* pong::update_curr_big_state()
{
   int big_index = 0;
   for(int s = screen_state_counter - (n_screen_states - 1); 
       s <= screen_state_counter; s++)
   {
      int reduced_s = modulo(s, n_screen_states);
      for(int i = 0; i < n_reduced_pixels; i++)
      {
         curr_big_state_ptr->put(
            big_index, screen_state_ptrs[reduced_s]->get(i));
         big_index++;
      }
   } // loop over index s labeling screen states
   return curr_big_state_ptr;
}

// ---------------------------------------------------------------------
pong::pong(int n_screen_states)
{
   this->n_screen_states = n_screen_states;
   initialize_member_objects();
   allocate_member_objects();
}

// Copy constructor:

pong::pong(const pong& S)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
pong::~pong()
{
   delete screen0_state_ptr;
   delete screen1_state_ptr;

   for(int s = 0; s < n_screen_states; s++)
   {
      delete screen_state_ptrs[s];
   }

   delete curr_big_state_ptr;
}

// ==========================================================================

// Vertical paddle positions for ALE supported pong ROM range
// from X = 0 (bottom wall) through 31 (top wall).
// ALE supported pong ROM's default starting position for paddle is X
// = 18 .  X = 15 corresponds to the paddle's vertical center
// position.

int pong::get_min_paddle_x() const
{
   return 0; // bottom wall
}

int pong::get_max_paddle_x() const
{
   return 31; // right wall
}

int pong::get_default_starting_paddle_x() const
{
   return 18;
}

int pong::get_center_paddle_x() const
{
   return 15;
}

void pong::set_paddle_x(int x)
{
   paddle_x = x;
}

int pong::get_paddle_x() const
{
   return paddle_x;
}

bool pong::increment_paddle_x() 
{
//   cout << "paddle_x = " << paddle_x << " max_paddle_x = "
//        << get_max_paddle_x() << endl;
   if(paddle_x >= get_max_paddle_x())
   {
      paddle_x = get_max_paddle_x();
      return false;
   }
   else
   {
      paddle_x++;
      return true;
   }
}

bool pong::decrement_paddle_x() 
{
//   cout << "paddle_x = " << paddle_x << " min_paddle_x = "
//        << get_min_paddle_x() << endl;
   if(paddle_x <= get_min_paddle_x())
   {
      paddle_x = get_min_paddle_x();
      return false;
   }
   else
   {
      paddle_x--;
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function push_back_paddle_x

void pong::push_back_paddle_x()
{
   paddle_x_values[d_paddle] = get_paddle_x();
   d_paddle++;
   if(d_paddle >= max_paddle_x_size)
   {
      d_paddle = 0;
      paddle_x_values_filled = true;
   }
}

// ---------------------------------------------------------------------
// Generate metafile plot of paddle X density probability distribution.

void pong::plot_paddle_x_dist(string output_subdir, string extrainfo)
{
   if(!paddle_x_values_filled) return;

   vector<double> X;
   for(int d = 0; d < max_paddle_x_size; d++)
   {
      X.push_back(double(paddle_x_values[d]));
   }
   double min_X = mathfunc::minimal_value(X);
   double max_X = mathfunc::maximal_value(X);
   int n_bins = max_X - min_X + 1;
   prob_distribution prob_X(X, n_bins);

   prob_X.set_densityfilenamestr(output_subdir+"paddle_X.meta");
   prob_X.write_density_dist(false, true);
}

// ---------------------------------------------------------------------
void pong::crop_center_ROI(vector<vector<unsigned char > >& byte_array)
{

// Retrieve curr frame's greyscale pixel values:

   grayscale_output_buffer.clear();
   ale.getScreenGrayscale(grayscale_output_buffer);

// Crop out center RoI from current frame:

   for(int py = min_py; py < max_py; py++)
   {
      vector<unsigned char> curr_byte_row;
      for(int px = min_px; px <  max_px; px++)
      {
         int curr_pixel = px + py * screen_width;
         curr_byte_row.push_back(grayscale_output_buffer[curr_pixel]);
      } // loop over px
      byte_array.push_back(curr_byte_row);
   } // loop over py
}

// ---------------------------------------------------------------------
// Member function max_pool() takes in row and column values within
// unsigned char array byte_array.  It returns the maximum pooled
// unsigned char value over a 5x4 cell.

unsigned char pong::max_pool(
   int r, int c, const vector<vector<unsigned char > >& byte_array)
{
   unsigned char z00 = byte_array[r][c];
   unsigned char z01 = byte_array[r][c+1];
   unsigned char z02 = byte_array[r][c+2];
   unsigned char z03 = byte_array[r][c+3];
   unsigned char max_z0 = basic_math::max(z00, z01, z02, z03);

   unsigned char z10 = byte_array[r+1][c];
   unsigned char z11 = byte_array[r+1][c+1];
   unsigned char z12 = byte_array[r+1][c+2];
   unsigned char z13 = byte_array[r+1][c+3];
   unsigned char max_z1 = basic_math::max(z10, z11, z12, z13);

   unsigned char z20 = byte_array[r+2][c];
   unsigned char z21 = byte_array[r+2][c+1];
   unsigned char z22 = byte_array[r+2][c+2];
   unsigned char z23 = byte_array[r+2][c+3];
   unsigned char max_z2 = basic_math::max(z20, z21, z22, z23);

   unsigned char z30 = byte_array[r+3][c];
   unsigned char z31 = byte_array[r+3][c+1];
   unsigned char z32 = byte_array[r+3][c+2];
   unsigned char z33 = byte_array[r+3][c+3];
   unsigned char max_z3 = basic_math::max(z30, z31, z32, z33);

   unsigned char z40 = byte_array[r+4][c];
   unsigned char z41 = byte_array[r+4][c+1];
   unsigned char z42 = byte_array[r+4][c+2];
   unsigned char z43 = byte_array[r+4][c+3];
   unsigned char max_z4 = basic_math::max(z40, z41, z42, z43);

   unsigned char max_z = basic_math::max(
      max_z0, max_z1, max_z2, max_z3, max_z4);
   return max_z;
}

// ---------------------------------------------------------------------
// Boolean member function crop_pool_difference_curr_frame() returns
// true if it successfully updates the current and previous ALE
// frames.

bool pong::crop_pool_difference_curr_frame(bool export_frames_flag)
{
   int curr_framenumber = ale.getEpisodeFrameNumber();

// Do not process current frame if its number does NOT differ from
// previously processed frame!

   if(curr_framenumber == prev_framenumber)
   {
      return false;
   }
   prev_framenumber = curr_framenumber;


   vector<vector<unsigned char> > byte_array;
   crop_center_ROI(byte_array);

   if(export_frames_flag)
   {
      string output_frame = orig_subdir+"frame_"+
         stringfunc::integer_to_string(curr_framenumber,5)+".png";
      videofunc::write_8bit_greyscale_pngfile(byte_array, output_frame);
   }

// Ping-pong pooled_byte_array and other_pooled_byte_array:
      
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

// rskip x cskip max pool cropped center for current frame:

   for(unsigned int r = 0; r < byte_array.size(); r += rskip)
   {
      vector<unsigned char> pooled_byte_row;
      for (unsigned int c = 0; c < byte_array[0].size(); c += cskip)
      {
         unsigned char max_z = max_pool(r, c, byte_array);
         pooled_byte_row.push_back(max_z);

      } // loop over index c
      pooled_byte_array_ptr->push_back(pooled_byte_row);
   } // loop over index r

   if(export_frames_flag)
   {
      string pooled_frame = pooled_subdir+"pooled_frame_"+
         stringfunc::integer_to_string(curr_framenumber,5)+".png";
      videofunc::write_8bit_greyscale_pngfile(
         *pooled_byte_array_ptr, pooled_frame);
   }

// Compute differences between current and other pooled byte arrays:

   vector<vector<unsigned char > > diff_pooled_byte_array;
   vector<unsigned char> diff_pooled_byte_row;

   if(other_pooled_byte_array_ptr->size() > 0)
   {
      int p = 0;
      for(unsigned int py = 0; py < pooled_byte_array_ptr->size(); py++)
      {
         diff_pooled_byte_row.clear();
         
         for(unsigned int px = 0; 
             px < pooled_byte_array_ptr->at(0).size(); px++)
         {

// Retain all paddle pixels within last column of differenced PONG
// image:

            unsigned char z_diff;
            const unsigned char z_diff_threshold(100);
            if(px == pooled_byte_array_ptr->at(0).size() - 1)
            {
               z_diff = pooled_byte_array_ptr->at(py).at(px);
               if(z_diff < z_diff_threshold)
               {
                  z_diff = 0;
               }
               
            }
            else
            {
               z_diff = pooled_byte_array_ptr->at(py).at(px) - 
                  other_pooled_byte_array_ptr->at(py).at(px);
            }

// Recall the vast majority of z_diff values = 0!  As of 12/26/16, we
// reset any non-zero component to unity.  So all screen state vectors
// are binary valued:

            double ren_z_diff = 0;
            if(z_diff != 0)
            {
               ren_z_diff = 1;
//               ren_z_diff = 255;
            }

            if(difference_counter == 0)
            {
               screen0_state_ptr->put(p++, ren_z_diff);
            }
            else
            {
               screen1_state_ptr->put(p++, ren_z_diff);
            }

            if(export_frames_flag)
            {
               diff_pooled_byte_row.push_back(z_diff);
            }
         } // loop over px

         if(export_frames_flag)
         {
            diff_pooled_byte_array.push_back(diff_pooled_byte_row);
         }
      } // loop over py 

      if(export_frames_flag)
      {
         string diff_frame = differenced_subdir+"differenced_frame_"+
            stringfunc::integer_to_string(curr_framenumber,5)+".png";
         videofunc::write_8bit_greyscale_pngfile(
            diff_pooled_byte_array, diff_frame);
      }
   } // other_pooled_byte_array.size > 0 conditional
               
// Clear other pooled byte array:
   for(unsigned int py = 0; py < other_pooled_byte_array_ptr->size(); py++)
   {
      other_pooled_byte_array_ptr->at(py).clear();
   }
   other_pooled_byte_array_ptr->clear();
   
   pingpong_curr_and_next_states();
   difference_counter = 1 - difference_counter;

   return true;
}

// ---------------------------------------------------------------------
// Boolean member function crop_pool_sum_curr_frame() returns
// true if it successfully updates the current and previous ALE
// frames.

bool pong::crop_pool_sum_curr_frame(bool export_frames_flag)
{
   int curr_framenumber = ale.getEpisodeFrameNumber();

// Do not process current frame if its number does NOT differ from
// previously processed frame!

   if(curr_framenumber == prev_framenumber)
   {
      return false;
   }
   prev_framenumber = curr_framenumber;

   vector<vector<unsigned char> > byte_array;
   crop_center_ROI(byte_array);

   if(export_frames_flag)
   {
      string output_frame = orig_subdir+"frame_"+
         stringfunc::integer_to_string(curr_framenumber,5)+".png";
      videofunc::write_8bit_greyscale_pngfile(byte_array, output_frame);
   }

// Ping-pong pooled_byte_array and other_pooled_byte_array:
      
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

// 4x3 max pool cropped center for current frame:

   for(unsigned int r = 0; r < byte_array.size(); r += rskip)
   {
      vector<unsigned char> pooled_byte_row;
      for (unsigned int c = 0; c < byte_array[0].size(); c += cskip)
      {
         unsigned char max_z = max_pool(r, c, byte_array);
         pooled_byte_row.push_back(max_z);

      } // loop over index c
      pooled_byte_array_ptr->push_back(pooled_byte_row);
   } // loop over index r

   if(export_frames_flag)
   {
      string pooled_frame = pooled_subdir+"pooled_frame_"+
         stringfunc::integer_to_string(curr_framenumber,5)+".png";
      videofunc::write_8bit_greyscale_pngfile(
         *pooled_byte_array_ptr, pooled_frame);
   }

// Compute differences between current and other pooled byte arrays:

   vector<vector<unsigned char > > maxed_pooled_byte_array;
   vector<unsigned char> maxed_pooled_byte_row;

   if(other_pooled_byte_array_ptr->size() > 0)
   {
      int p = 0;
      for(unsigned int py = 0; py < pooled_byte_array_ptr->size(); py++)
      {
         maxed_pooled_byte_row.clear();
         
         for(unsigned int px = 0; 
             px < pooled_byte_array_ptr->at(0).size(); px++)
         {
            unsigned char z_max = basic_math::max(
               pooled_byte_array_ptr->at(py).at(px) ,
               other_pooled_byte_array_ptr->at(py).at(px) );

// Recall the vast majority of z_diff values = 0!  So as of 12/16/16,
// we choose to subtract off the zero-valued median rather than a
// non-zero mean so as to yield state vectors which mostly have zero
// coordinates.  But we still divide by sigma_zdiff so that the
// non-zero coordinates have magnitudes close to unity:

            double ren_z_max = double(z_max - mu_z) / sigma_z;

            if(difference_counter == 0)
            {
               screen0_state_ptr->put(p++, ren_z_max);
            }
            else
            {
               screen1_state_ptr->put(p++, ren_z_max);
            }

            if(export_frames_flag)
            {
               maxed_pooled_byte_row.push_back(z_max);
            }
            
         } // loop over px
         if(export_frames_flag)
         {
            maxed_pooled_byte_array.push_back(maxed_pooled_byte_row);
         }
      } // loop over py 

      if(export_frames_flag)
      {
         string maxed_frame = maxed_subdir+"maxed_frame_"+
            stringfunc::integer_to_string(curr_framenumber,5)+".png";
         videofunc::write_8bit_greyscale_pngfile(
            maxed_pooled_byte_array, maxed_frame);
      }
   } // other_pooled_byte_array.size > 0 conditional
               
// Clear other pooled byte array:
   for(unsigned int py = 0; py < other_pooled_byte_array_ptr->size(); py++)
   {
      other_pooled_byte_array_ptr->at(py).clear();
   }
   other_pooled_byte_array_ptr->clear();
   
   pingpong_curr_and_next_states();
   difference_counter = 1 - difference_counter;

   return true;
}

// ---------------------------------------------------------------------
// Member function pingpong_curr_and_next_states() resets
// curr_state_ptr and next_state_ptr to point to screen0_state_ptr and
// streen1_state_ptr based upon the current value of
// difference_counter.

void pong::pingpong_curr_and_next_states()
{
//   cout << "inside spacenv::pingpong_curr_and_next_states()" << endl;
//   cout << "difference_counter = " << difference_counter << endl;

   if(difference_counter == 0)
   {
      curr_state_ptr = screen1_state_ptr;
      next_state_ptr = screen0_state_ptr;
   }
   else
   {
      curr_state_ptr = screen0_state_ptr;
      next_state_ptr = screen1_state_ptr;
   }

//   cout << "curr_state_ptr = " << curr_state_ptr << endl;
//   cout << "next_state_ptr = " << next_state_ptr << endl;
//   cout << "curr_state_ptr->mdim = " << curr_state_ptr->get_mdim() << endl;
//   cout << "next_state_ptr->mdim = " << next_state_ptr->get_mdim() << endl;
   
//   cout << "|next_s - curr_s| = " 
//        << (*next_state_ptr - *curr_state_ptr).magnitude() 
//        << endl;
}

// ---------------------------------------------------------------------
// Boolean member function crop_pool_curr_frame() returns true if it
// successfully updates the current and previous ALE frames.

bool pong::crop_pool_curr_frame(bool export_frames_flag)
{
   int curr_framenumber = ale.getEpisodeFrameNumber();
//   cout << "inside pong::crop_pool_curr_frame()" << endl;

// Do not process current frame if its number does NOT differ from
// previously processed frame!

   if(curr_framenumber <= prev_framenumber)
   {
      return false;
   }
   prev_framenumber = curr_framenumber;

   vector<vector<unsigned char> > byte_array;
   crop_center_ROI(byte_array);

   if(export_frames_flag)
   {
      string output_frame = orig_subdir+"frame_"+
         stringfunc::integer_to_string(curr_framenumber,5)+".png";
      videofunc::write_8bit_greyscale_pngfile(byte_array, output_frame);
   }

   screen_state_counter = curr_framenumber;
   genvector *curr_screen_state_ptr = screen_state_ptrs[
      screen_state_counter % n_screen_states];

   pooled_byte_array_ptr = &pooled_byte_array0;

// rskip x cskip max pool cropped center for current frame:

   int reduced_pixel_counter = 0;
   for(unsigned int r = 0; r < byte_array.size(); r += rskip)
   {
      vector<unsigned char> pooled_byte_row;
      for (unsigned int c = 0; c < byte_array[0].size(); c+= cskip)
      {
         unsigned char max_z = max_pool(r, c, byte_array);
         double zpool(max_z);
//         pooled_scrn_values.push_back(zpool);

         double ren_zpool = (zpool - mu_z) / sigma_z;
         curr_screen_state_ptr->put(reduced_pixel_counter, ren_zpool);
         reduced_pixel_counter++;

         pooled_byte_row.push_back(max_z);
      } // loop over index c
      pooled_byte_array_ptr->push_back(pooled_byte_row);
   } // loop over index r

   if(export_frames_flag)
   {
      string pooled_frame = pooled_subdir+"pooled_frame_"+
         stringfunc::integer_to_string(curr_framenumber,5)+".png";
      videofunc::write_8bit_greyscale_pngfile(
         *pooled_byte_array_ptr, pooled_frame);
   }

// Clear pooled byte array:
   for(unsigned int py = 0; py < pooled_byte_array_ptr->size(); py++)
   {
      pooled_byte_array_ptr->at(py).clear();
   }
   pooled_byte_array_ptr->clear();

   return true;
}

// ---------------------------------------------------------------------
void pong::mu_and_sigma_for_pooled_zvalues()
{
   double mu, sigma;
   mathfunc::mean_and_std_dev(pooled_scrn_values, mu, sigma);
   double median, quartile_width;
   mathfunc::median_value_and_quartile_width(
      pooled_scrn_values, median, quartile_width);

   cout << "pooled_scrn_values.size = " << pooled_scrn_values.size()
        << endl;
   cout << "mu = " << mu << " sigma = " << sigma << endl;
   cout << "median = " << median << " quartile_width = " << quartile_width 
        << endl;
}

// ---------------------------------------------------------------------
// Member function save_screen() writes out an RGB image corresponding
// to the current ALE screen.

string pong::save_screen(int episode_number, string curr_screen_filename)
{
   string caption="";
   return save_screen(episode_number, curr_screen_filename, caption);
}

string pong::save_screen(int episode_number, string curr_screen_filename,
                             string caption)
{
   string episode_subdir = screen_exports_subdir+
      stringfunc::integer_to_string(episode_number,5)+"/";
   filefunc::dircreate(episode_subdir);
   string curr_screen_path = episode_subdir + curr_screen_filename;
   screen_exporter_ptr->save(ale.getScreen(), curr_screen_path);

   if(caption.size() > 0)
   {
      imagefunc::add_text_to_image(
         "purple", caption, "west", "north", curr_screen_path, 
         curr_screen_path);
   }

   return curr_screen_path;
}
