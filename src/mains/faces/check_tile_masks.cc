// ====================================================================
// Program CHECK_TILE_MASKS
// ====================================================================
// Last updated on 5/30/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   bool binary_classification = false;
   bool vertical_classification = false;
   bool horizontal_classification = false;
   bool quadrant_classification = false;

   cout << "Enter 'b' to perform binary face classification:" << endl;
   cout << "Enter 'v' to perform vertical face half classification:" << endl;
   cout << "Enter 'h' to perform horizontal face half classification:" << endl;
   cout << "Enter 'q' to perform face quadrant classification:" << endl;

   string classification_str;
   cin >> classification_str;

   if (classification_str == "b")
   {
      binary_classification = true;
   }
   else if (classification_str == "v")
   {
      vertical_classification = true;
   }
   else if (classification_str == "h")
   {
      horizontal_classification = true;
   }
   else if (classification_str == "q")
   {
      quadrant_classification = true;
   }
   else
   {
      cout << "Incorrect input" << endl;
      exit(-1);
   }
   
   timefunc::initialize_timeofday_clock(); 

   bool visualize_masks_flag = true;
//   bool visualize_masks_flag = false;
   cout << "visualize_masks_flag = " << visualize_masks_flag << endl;

// Recall image chips and masks should equal 321x321 in pixel size
// in order to meet deeplab's GPU card and batch size memory
// requirements:
   
   const unsigned int deeplab_tile_size = 321;	

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_data_subdir=faces_rootdir+"labeled_data/";   

   int faces_ID = -1;
   cout << "Enter faces ID (-1 for default):" << endl;
   cin >> faces_ID;
   string faces_subdir=labeled_data_subdir+"faces";
   if(faces_ID >= 0)
   {
      faces_subdir += "_"+stringfunc::integer_to_string(faces_ID,2);
   }
   filefunc::add_trailing_dir_slash(faces_subdir);


   string deeplab_inputs_subdir=faces_subdir+"deeplab_inputs/";
   string tiles_subdir=deeplab_inputs_subdir+"tiles/";
   string eightbit_masks_subdir=deeplab_inputs_subdir+"eightbit_masks/";

   vector<string> mask_filenames = filefunc::image_files_in_subdir(
      eightbit_masks_subdir);

   int istart = 0;
   int istop = mask_filenames.size();
   for(int i = istart ; i < istop; i++)
   {
      if(i%1000 == 0)
      {
         double progress_frac = double(i-istart)/double(istop-istart);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

//      cout << "i = " << i 
//           << " mask_filename = " << mask_filenames[i] << endl;
      texture_rectangle mask_tr(mask_filenames[i], NULL);

//      cout << "width = " << mask_tr.getWidth()
//           << " height = " << mask_tr.getHeight() << endl;
//      cout << "n_channels = " << mask_tr.getNchannels() << endl;
    
      int n_zeros = 0;
      int n_ones = 0;
      int n_others = 0;
      for(unsigned int py = 0; py < deeplab_tile_size; py++)
      {
         for(unsigned int px = 0; px < deeplab_tile_size; px++)
         {
            int curr_intensity = mask_tr.get_pixel_intensity_value(px,py);
            
            if(curr_intensity == 0)
            {
               n_zeros++;
            }
            else if (curr_intensity == 1)
            {
               n_ones++;
            }
            else
            {
               n_others++;
            }
         } // loop over px index
      } // loop over py index
      if(n_others > 0)
      {
         cout << "i = " << i << " n_zeros = " << n_zeros
              << " n_ones = " << n_ones
              << " n_others = " << n_others << endl;
         cout << "mask_filename = " << mask_filenames[i]
              << endl;
         outputfunc::enter_continue_char();
      }
   } // loop over index i labeling image mask filenames
   
}

