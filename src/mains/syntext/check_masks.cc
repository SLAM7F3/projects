// ==========================================================================
// Program CHECK_MASKS imports a set of rotated or output character
// [word] masks.  Looping over each input mask, it prints out the
// mask's maximum pixel value.
// ==========================================================================
// Last updated on 3/8/16; 3/10/16; 3/27/16; 2/28/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "text/textfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

   int syn_words_ID = -1;
   cout << "Enter synthetic words ID (-1 for default):" << endl;
   cin >> syn_words_ID;
   string synthetic_subdir="./training_data/synthetic_words";
   if(syn_words_ID >= 0)
   {
      synthetic_subdir += "_"+stringfunc::integer_to_string(syn_words_ID,2);
   }
   filefunc::add_trailing_dir_slash(synthetic_subdir);

//   string rotated_char_masks_subdir=synthetic_subdir+"rotated_char_masks/";
//   string rotated_word_masks_subdir=synthetic_subdir+"rotated_word_masks/";
   string output_char_masks_subdir=synthetic_subdir+"output_charmasks/";
   string output_word_masks_subdir=synthetic_subdir+"output_wordmasks/";
   vector<string> char_mask_filenames = filefunc::image_files_in_subdir(
      output_char_masks_subdir);
//      rotated_char_masks_subdir);
   vector<string> word_mask_filenames = filefunc::image_files_in_subdir(
      output_word_masks_subdir);
//      rotated_word_masks_subdir);

   for(unsigned int i = 0; i < char_mask_filenames.size(); i++)
   {
      texture_rectangle mask_tr(char_mask_filenames[i],NULL);
      int n_channels = mask_tr.getNchannels();

      int xdim = mask_tr.getWidth();
      int ydim = mask_tr.getHeight();

      int intensity_min = POSITIVEINFINITY;
      int intensity_max = NEGATIVEINFINITY;
      int Rmin = POSITIVEINFINITY;
      int Amin = POSITIVEINFINITY;
      int Rmax = NEGATIVEINFINITY;
      int Amax = NEGATIVEINFINITY;
      
      for(int py = 0; py < ydim; py++)
      {
         for(int px = 0; px < xdim; px++)
         {
            if(n_channels == 1)
            {
               int intensity = mask_tr.get_pixel_intensity_value(px,py);
               intensity_min = basic_math::min(intensity_min, intensity);
               intensity_max = basic_math::max(intensity_max, intensity);
            }
            else if(n_channels == 4)
            {
               int R,G,B,A;
               mask_tr.get_pixel_RGBA_values(px,py,R,G,B,A);
               Rmin = basic_math::min(Rmin, R);
               Rmax = basic_math::max(Rmax, R);
               Amin = basic_math::min(Amin, A);
               Amax = basic_math::max(Amax, A);
            }
         } // loop over px
      } // loop over py

      if(n_channels == 1)
      {
         cout << "i = " << i << " intensity_min = " << intensity_min
              << " intensity_max = " << intensity_max << endl;
         if(intensity_max > 5) outputfunc::enter_continue_char();
      }
      else if(n_channels == 4)
      {
         cout << "i = " << i << " Rmin = " << Rmin << " Rmax = " << Rmax
              << " Amin = " << Amin << " Amax = " << Amax << endl;
      }
      
   } // loop over index i labeling mask images
}

