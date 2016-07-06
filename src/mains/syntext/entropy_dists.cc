// ==========================================================================
// Program ENTROPY_DISTS imports all non-text images used as
// backgrounds for synthetic text images.  Looping over each image, it
// randomly picks a set of bounding boxes and computes the RGB
// entropies within them.  ENTROPY_DISTS outputs the entropy
// distribution for the R,G,B channels.

// After running ENTROPY_DISTS on O(500) large, background internet
// images, we found to good approximation

// 	median +/- quartile width = 0.25 +/- 0.08 

// for entropy density within the R, G and B color channels.
// ==========================================================================
// Last updated on 4/6/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
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
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

//   string nontext_subdir = "./images/internet/non_text/";
   //   string nontext_subdir = "./images/internet/backgrounds5/";
   string nontext_subdir = "./images/internet/backgrounds6/";



   vector<string> image_filenames=filefunc::image_files_in_subdir(
      nontext_subdir);

   vector<double> r_entropy, g_entropy, b_entropy;
   for(unsigned int i = 0; i < image_filenames.size(); i++)
   {
      cout << "i = " << i 
           << " image_filename = " << image_filenames[i]
           << endl;
      if(i%10 == 0)
      {
         double progress_frac = double(i)/image_filenames.size();
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }
      
      texture_rectangle *tr_ptr = new texture_rectangle(
         image_filenames[i], NULL);
      int width = tr_ptr->getWidth();
      int height = tr_ptr->getHeight();
      int n_channels = tr_ptr->getNchannels();
      if(n_channels < 3) continue;

      twoDarray *SrtwoDarray_ptr = new twoDarray(width,height);
      twoDarray *SgtwoDarray_ptr = new twoDarray(width,height);
      twoDarray *SbtwoDarray_ptr = new twoDarray(width,height);
      tr_ptr->RGB_entropy_integral_images(
         SrtwoDarray_ptr, SgtwoDarray_ptr, SbtwoDarray_ptr);

      int min_bbox_width = 40;
      int max_bbox_width = 400;
      int min_bbox_height = 20;
      int max_bbox_height = 200;

      int n_bboxes = 10;
      for(int b = 0; b < n_bboxes; b++)
      {
         int px_lo = nrfunc::ran1() * (width-1 -  max_bbox_width);
         int px_hi = px_lo + min_bbox_width + 0.9*nrfunc::ran1() * 
            (max_bbox_width - min_bbox_width);
         int py_lo = nrfunc::ran1() * (height-1 - max_bbox_height);
         int py_hi = py_lo + min_bbox_height + 0.9*nrfunc::ran1() * 
            (max_bbox_height - min_bbox_height);

         px_lo = basic_math::min(width-1,px_lo);
         px_lo = basic_math::max(0,px_lo);
         px_hi = basic_math::min(width-1,px_hi);
         px_hi = basic_math::max(0,px_hi);
         
         py_lo = basic_math::min(height-1,py_lo);
         py_lo = basic_math::max(0,py_lo);
         py_hi = basic_math::min(height-1,py_hi);
         py_hi = basic_math::max(0,py_hi);
         
         if(px_lo >= px_hi) continue;
         if(py_lo >= py_hi) continue;


         double n_pixels = (px_hi - px_lo) * (py_hi - py_lo);
         
         double Sr, Sg, Sb;
         tr_ptr->bbox_RGB_entropies(
            px_lo, px_hi, py_lo, py_hi,
            SrtwoDarray_ptr, SgtwoDarray_ptr, SbtwoDarray_ptr, 
            Sr, Sg, Sb);

         double Sr_density = Sr/n_pixels;
         double Sg_density = Sg/n_pixels;
         double Sb_density = Sb/n_pixels;
         
         r_entropy.push_back(Sr_density);
         g_entropy.push_back(Sg_density);
         b_entropy.push_back(Sb_density);
      }
      
      delete SrtwoDarray_ptr;
      delete SgtwoDarray_ptr;
      delete SbtwoDarray_ptr;
      delete tr_ptr;
   }
   
   prob_distribution prob_rentropy(r_entropy, 100, 0);
   prob_rentropy.set_densityfilenamestr("r_entropy.meta");
   prob_rentropy.write_density_dist(false,true);

   prob_distribution prob_gentropy(g_entropy, 100, 0);
   prob_gentropy.set_densityfilenamestr("g_entropy.meta");
   prob_gentropy.write_density_dist(false,true);

   prob_distribution prob_bentropy(b_entropy, 100, 0);
   prob_bentropy.set_densityfilenamestr("b_entropy.meta");
   prob_bentropy.write_density_dist(false,true);

   string banner="Exported RGB entropy distributions";
   outputfunc::write_banner(banner);
}

