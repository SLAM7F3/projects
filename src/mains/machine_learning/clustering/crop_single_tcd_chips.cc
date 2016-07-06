// ==========================================================================
// Program CROP_SINGLE_TCD_CHIPS imports a set of TCD chips generated
// by Weiyu et al.  Each chip has a large view on its left and two
// smaller views vertically aligned on the
// right. CROP_SINGLE_TCD_CHIPS performs vertical profiling to search
// for a column of red pixels which separates the large view on the
// left from the two small views on the right.  The program then
// exports just the large view on the left to an output jpg file with
// a simplified basename.

// 			./crop_single_tcd_chips

// ==========================================================================
// Last updated on 2/8/16; 2/12/16
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/polygon.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();   

   string tcd_subdir="/data/peter_stuff/imagery/tcd_chips/";
   string images_subdir=tcd_subdir+"bv_76101";
//   string images_subdir="/home/pcho/scratch/human_machine/bv_76101/";
//   string images_subdir="/home/pcho/scratch/human_machine/chips/";
   string single_chips_subdir=tcd_subdir+"single_chips/";
   filefunc::dircreate(single_chips_subdir);

   bool search_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
     images_subdir, search_children_dirs_flag);
   unsigned int n_images = image_filenames.size();
   cout << "n_images = " << n_images << endl;

   unsigned int i_start = 0;
//   unsigned int i_start = 89157;
   unsigned int i_stop = n_images;
   for(unsigned int i = i_start; i < i_stop; i++)
   {
      outputfunc::update_progress_and_remaining_time(
         i-i_start, 5000, i_stop - i_start);

      texture_rectangle curr_tr(image_filenames[i], NULL);
      int xdim = curr_tr.getWidth();
      int ydim = curr_tr.getHeight();

//      string output_filename="output_"+stringfunc::integer_to_string(
//         i,5)+".jpg";
//      curr_tr.write_curr_frame(output_filename);

// Scan from left to right of curr_tr.  Search for vertical column of
// pixels which is all red:
 
      int px_stop = -1;
      for(int px = 0; px < xdim; px++)
      {
         int n_red_pixels = 0;
         for(int py = 0; py < ydim; py++)
         {
            double h, s, v;
            curr_tr.get_pixel_hsv_values(px, py, h, s, v);
            h = basic_math::phase_to_canonical_interval(h, -180, 180);
            if(fabs(h) < 10 && s > 0.8 && v > 0.75)
            {
               n_red_pixels++;
            }
         }
         double red_column_frac = double(n_red_pixels)/ydim;
//         cout << "px = " << px 
//              << " red_column_frac = " << red_column_frac << endl;

         if(red_column_frac > 0.66)
         {
            px_stop = px - 1;
            break;
         }
      } // loop over index px

      string single_chip_filename=single_chips_subdir+
         "c_"+stringfunc::integer_to_string(i,6)+".jpg";
      curr_tr.write_curr_frame(0, px_stop, 0, ydim - 1, single_chip_filename);

      if(i == i_start)
      {
         string banner="Exported "+single_chip_filename;
         outputfunc::write_banner(banner);
      }
      
   } // loop over index i labeling input images
}

