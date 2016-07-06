// ==========================================================================
// Program SVHN_COLOR_DISTS scans through SVHN image chips and
// extracts their hsv contents.  It creates probability density plots
// for SVHN hues, saturations and values.

//                          ./svhn_color_dists

// ==========================================================================
// Last updated on 2/1/16
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
#include "video/texture_rectangle.h"
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

   string images_subdir = "/data/TrainingImagery/svhn/extra/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   int istart=0;
   int istop = image_filenames.size();
   int n_images = istop - istart;
   cout << "n_images = " << n_images << endl;
   //   istop = 50 * 1000;

   vector<double> hues, saturations, values;
   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_fraction(i,1000,n_images);

      if ((i-istart)%1000 == 0)
      {
         cout << "i = " << i << endl;
         double progress_frac = double(i - istart)/n_images;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      texture_rectangle curr_tr(image_filenames[i], NULL);
      for(unsigned int py = 0; py < curr_tr.getHeight(); py++)
      {
         for(unsigned int px = 0; px < curr_tr.getWidth(); px++)
         {
            double h, s, v;
            curr_tr.get_pixel_hsv_values(px, py, h, s, v);
            hues.push_back(h);
            saturations.push_back(s);
            values.push_back(v);
         }
      }

   } // loop over index i 

   prob_distribution hues_prob(hues, 100);
   hues_prob.set_densityfilenamestr("hues_density.meta");
   hues_prob.set_xmin(0);
   hues_prob.set_xmax(360);
   hues_prob.set_xtic(20);
   hues_prob.set_xsubtic(10);
   hues_prob.writeprobdists(false, true);

   prob_distribution saturations_prob(saturations, 100);
   saturations_prob.set_densityfilenamestr("saturations_density.meta");
   saturations_prob.set_xmin(0);
   saturations_prob.set_xmax(1);
   saturations_prob.set_xtic(0.1);
   saturations_prob.set_xsubtic(0.05);
   saturations_prob.writeprobdists(false, true);

   prob_distribution values_prob(values, 100);
   values_prob.set_densityfilenamestr("values_density.meta");
   values_prob.set_xmin(0);
   values_prob.set_xmax(1);
   values_prob.set_xtic(0.1);
   values_prob.set_xsubtic(0.05);
   values_prob.writeprobdists(false, true);


} 


