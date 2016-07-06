// ==========================================================================
// Program SVHN_PIXEL_SIZES scans through SVHN image chips and
// extracts their pixel widths and heights.  It then computes and
// exports the probability densities for SVHN image chip pixel widths
// and aspect ratios.  When this program is run on O(40K+) SVHN chips,
// it finds that the aspect ratio distribution is nearly gaussian with
// mean = 0.62 and standard deviation = 0.11.  Pixel widths appear to
// follow an approximate decaying exponential distribution which has a
// lower cutoff at width_lo = 23.

//                          ./svhn_pixel_sizes

// ==========================================================================
// Last updated on 1/28/16; 1/29/16; 1/30/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string images_subdir = 
      "/data1/imagetext/training_data/svhn_image_chips/super_train/";
//   string images_subdir = "./";
   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir,search_all_children_dirs_flag);

   int istart=0;
   int istop = image_filenames.size();
   int n_images = istop - istart;
   cout << "n_images = " << n_images << endl;

   vector<double> xdims, aspect_ratios;
   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_fraction(i,100,n_images);

      if ((i-istart)%1000 == 0)
      {
         double progress_frac = double(i - istart)/n_images;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      unsigned int width, height;
      imagefunc::get_image_width_height(
         image_filenames[i], width, height);
      double aspect_ratio = double(width) / double (height);
      xdims.push_back(width);
      aspect_ratios.push_back(aspect_ratio);
  
   } // loop over index i 

   prob_distribution xdim_prob(xdims, 100);
   xdim_prob.set_densityfilenamestr("xdim_density.meta");
   xdim_prob.set_xmin(0);
   xdim_prob.set_xmax(100);
   xdim_prob.set_xtic(20);
   xdim_prob.set_xsubtic(10);
   xdim_prob.writeprobdists(false, true);

   prob_distribution aspect_ratio_prob(aspect_ratios, 100);
   aspect_ratio_prob.set_xmin(0);
   aspect_ratio_prob.set_densityfilenamestr("aspect_ratio_density.meta");
   aspect_ratio_prob.write_density_dist(false, true);

} 


