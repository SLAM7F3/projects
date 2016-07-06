// ==========================================================================
// Program ER_PARAM_BOUNDS imports a set of manually checked extremal regions 
// that correspond to housenumber digits.  It also reads in original image
// chips corresponding to imported extremal region masks.  ER_PARAM_BOUNDS then
// computes a variety of simple features (e.g. bounding box width/height in
// pixels, extremal region area, perimeter and Euler number,
// maximal/minimal intensity variations for pixels corresponding to
// extremal region mask, etc).  It prints out maximal/minimal
// values for these simple features which can then be input as
// thresholds into the battery of incremental tests which we
// run to reject extremal regions as candidate numbers.

//			./ER_param_bounds

// ==========================================================================
// Last updated on 4/28/14; 5/5/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <dlib/disjoint_subsets.h>

#include "image/binaryimagefuncs.h"
#include "geometry/bounding_box.h"
#include "color/colortext.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "math/ltduple.h"
#include "numrec/nrfuncs.h"
#include "video/photogroup.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;    
using std::cout;
using std::endl;
using std::flush;
using std::set;
using std::string;
using std::vector;

typedef set<int> INT_SET;
// independent int = pixel ID

typedef struct _connected_component
{
   bool active_flag;
   int ID;
   int threshold_level;
   _connected_component* parent_cc_ptr;
   set<_connected_component*> children_cc_ptrs;
   INT_SET pixel_IDs;
   int bbox_min_px,bbox_max_px,bbox_min_py,bbox_max_py;
   int pixel_perim,n_holes;
   colorfunc::Color color;
} connected_component_t;

// ==========================================================================

int main (int argc, char* argv[])
{
   string imagetext_subdir=
      "/home/pcho/programs/c++/svn/projects/src/mains/imagetext/";
   string vandigits_subdir=imagetext_subdir+"images/HouseNumbers/van/digits/";

   int min_cc_pixel_size=99999;
   int max_cc_pixel_size=0;
   int min_bbox_width=99999;
   int max_bbox_width=0;
   int min_bbox_height=99999;
   int max_bbox_height=0;

   double min_aspect_ratio=99999;
   double max_aspect_ratio=-1;

   unsigned int n_total_masks=0;
   string max_CC_basename;
   string max_bbox_width_basename,max_bbox_height_basename;
   double max_ER_delta_intensity=0;
   double min_ER_delta_intensity=1;
   double max_sigma_ER_intensity=0;
   string max_sigma_ER_basename;

   double max_Euler_number=-999;
   double min_Euler_number=999;

   double max_sqr_inverse_compactness=-1;
   double min_sqr_inverse_compactness=9999;

   double max_bbox_intensity_mu=-1;
   double min_bbox_intensity_mu=9999;
   double max_bbox_intensity_sigma=-1;
   double min_bbox_intensity_sigma=9999;

   double min_delta_foreground_background_mu=9999;
   double max_sigma_background_intensity=-1;

   for (int d=0; d<10; d++)
   {
      string digit_subdir=vandigits_subdir+stringfunc::number_to_string(d)
         +"/";
      string bright_ERs_subdir=digit_subdir+"bright_extremal_regions/";
      string dark_ERs_subdir=digit_subdir+"dark_extremal_regions/";
      string bright_masks_subdir=bright_ERs_subdir+"good_masks/";
      string dark_masks_subdir=bright_ERs_subdir+"good_masks/";

      cout << "d = " << d << " digit_subdir = " << digit_subdir << endl;

      vector<string> bright_masks_filenames=
         filefunc::image_files_in_subdir(bright_masks_subdir);
      vector<string> dark_masks_filenames=
         filefunc::image_files_in_subdir(dark_masks_subdir);

      unsigned int n_bright_masks=bright_masks_filenames.size();
      unsigned int n_dark_masks=dark_masks_filenames.size();
      cout << "n_bright_masks = " << n_bright_masks
           << " n_dark_masks = " << n_dark_masks << endl;
      n_total_masks += n_bright_masks + n_dark_masks;
      
      for (unsigned int iter=0; iter<2; iter++)
      {
         unsigned int n_masks;
         if (iter==0)		// bright ERs
         {
            n_masks=n_bright_masks;
         }
         else if (iter==1) 	// dark ERs
         {
            n_masks=n_dark_masks;
         }

         texture_rectangle* texture_rectangle_ptr=NULL;
         texture_rectangle* mask_texture_rectangle_ptr=NULL;
         for (unsigned int i=0; i<n_masks; i++)
         {
            string basename;
            if (iter==0)
            {
               mask_texture_rectangle_ptr=new texture_rectangle(
                  bright_masks_filenames[i],NULL);
               basename=filefunc::getbasename(bright_masks_filenames[i]);
               string image_filename=digit_subdir+basename;
               texture_rectangle_ptr=new texture_rectangle(
                  image_filename,NULL);
            }
            else
            {
               mask_texture_rectangle_ptr=new texture_rectangle(
                  dark_masks_filenames[i],NULL);
               basename=filefunc::getbasename(dark_masks_filenames[i]);
               string image_filename=digit_subdir+basename;
               texture_rectangle_ptr=new texture_rectangle(
                  image_filename,NULL);
            }
                 
            int chip_width=mask_texture_rectangle_ptr->getWidth();
            int chip_height=mask_texture_rectangle_ptr->getHeight();
	    twoDarray* pbinary_twoDarray_ptr=new twoDarray(
               chip_width,chip_height);
	    pbinary_twoDarray_ptr->clear_values();

            int n_ER_pixels=0;         
            int R,G,B;
            int min_px=chip_width;
            int max_px=0;
            int min_py=chip_height;
            int max_py=0;
            double max_ER_v=0;
            double min_ER_v=1;
            double mu_ER_intensity=0;
            double sigma_ER_intensity=0;

	    int pfill=1;
            for (int py=0; py<chip_height; py++)
            {
               for (int px=0; px<chip_width; px++)
               {
                  mask_texture_rectangle_ptr->get_pixel_RGB_values(
                     px,py,R,G,B);
                  if (R > 128)
                  {

                     
                     min_px = basic_math::min(min_px,px);
                     max_px = basic_math::max(max_px,px);
                     min_py = basic_math::min(min_py,py);
                     max_py = basic_math::max(max_py,py);

// Retrieve RGB values for pixels which (hopefully) correspond to
// character foreground:

                     texture_rectangle_ptr->get_pixel_RGB_values(
                        px,py,R,G,B);

                     double r=R/255.0;
                     double g=G/255.0;
                     double b=B/255.0;
                     double h,s,v;
                     colorfunc::RGB_to_hsv(r,g,b,h,s,v);
                     max_ER_v = basic_math::max(max_ER_v,v);
                     min_ER_v = basic_math::min(max_ER_v,v);
                     sigma_ER_intensity = mathfunc::incremental_std_dev(
                        n_ER_pixels,v,mu_ER_intensity,sigma_ER_intensity);
                     mu_ER_intensity = mathfunc::incremental_mean(
                        n_ER_pixels,v,mu_ER_intensity);


		     pbinary_twoDarray_ptr->put(px,py,pfill);
                     n_ER_pixels++;
                  } // R > 128 conditional

               } // loop over px
            } // loop over py

            int curr_bbox_width=max_px-min_px;
            int curr_bbox_height=max_py-min_py;
	    double curr_aspect_ratio=double(curr_bbox_width)/
               double(curr_bbox_height);
	    min_aspect_ratio=basic_math::min(
               min_aspect_ratio,curr_aspect_ratio);
	    max_aspect_ratio=basic_math::max(
               max_aspect_ratio,curr_aspect_ratio);


            min_bbox_width=basic_math::min(min_bbox_width,curr_bbox_width);
//            max_bbox_width=basic_math::max(max_bbox_width,curr_bbox_width);
            if (curr_bbox_width > max_bbox_width)
            {
               max_bbox_width=curr_bbox_width;
               max_bbox_width_basename=basename;
            }
            
            min_bbox_height=basic_math::min(min_bbox_height,curr_bbox_height);
//            max_bbox_height=basic_math::max(max_bbox_height,curr_bbox_height);
            if (curr_bbox_height > max_bbox_height)
            {
               max_bbox_height=curr_bbox_height;
               max_bbox_height_basename=basename;
            }

            min_cc_pixel_size=basic_math::min(min_cc_pixel_size,n_ER_pixels);
            if (n_ER_pixels > max_cc_pixel_size)
            {
               max_cc_pixel_size=n_ER_pixels;
               max_CC_basename=basename;
            }
            
            max_cc_pixel_size=basic_math::max(max_cc_pixel_size,n_ER_pixels);

            double curr_delta_ER_v=max_ER_v-min_ER_v;
            min_ER_delta_intensity=basic_math::min(
               min_ER_delta_intensity,curr_delta_ER_v);
            max_ER_delta_intensity=basic_math::max(
               max_ER_delta_intensity,curr_delta_ER_v);


            if (sigma_ER_intensity > max_sigma_ER_intensity)
            {
               max_sigma_ER_intensity=sigma_ER_intensity;
               max_sigma_ER_basename=basename;
            }
//	    max_sigma_ER_intensity=basic_math::max(
//	      max_sigma_ER_intensity,sigma_ER_intensity);

	    double Euler_number,perimeter,area;
	    binaryimagefunc::image_Euler_number_perimeter_area(
	      pfill,pbinary_twoDarray_ptr,Euler_number,perimeter,area);
	    max_Euler_number=basic_math::max(max_Euler_number,Euler_number);
	    min_Euler_number=basic_math::min(min_Euler_number,Euler_number);

            double sqr_inverse_compactness=sqr(perimeter)/area;
            max_sqr_inverse_compactness=basic_math::max(
               max_sqr_inverse_compactness,sqr_inverse_compactness);
            min_sqr_inverse_compactness=basic_math::min(
               min_sqr_inverse_compactness,sqr_inverse_compactness);

            delete mask_texture_rectangle_ptr;
	    delete pbinary_twoDarray_ptr;

// Compute 1st and 2nd moments for all intensity values inside image chip:

	    double mu_chip_intensity,sigma_chip_intensity;
	    texture_rectangle_ptr->get_pixel_region_intensity_moments(
	      0,chip_width-1,0,chip_height-1,
              mu_chip_intensity,sigma_chip_intensity);

	    max_bbox_intensity_mu=basic_math::max(
               max_bbox_intensity_mu,mu_chip_intensity);
	    min_bbox_intensity_mu=basic_math::min(
               min_bbox_intensity_mu,mu_chip_intensity);
	    max_bbox_intensity_sigma=basic_math::max(
               max_bbox_intensity_sigma,sigma_chip_intensity);
	    min_bbox_intensity_sigma=basic_math::min(
               min_bbox_intensity_sigma,sigma_chip_intensity);

// Compute 1st and 2nd moments for "background" pixels inside image chip
// which do not reside inside extremal region (hopefully representing
// a character):

	    int n_foreground_pixels=area;
	    int n_background_pixels=chip_width*chip_height 
               - n_foreground_pixels;
	    double mu_background_intensity, sigma_background_intensity;
	    mathfunc::mean_std_dev_for_subset(
	      n_foreground_pixels, n_background_pixels,
              mu_ER_intensity, sigma_ER_intensity,
	      mu_chip_intensity, sigma_chip_intensity,
	      mu_background_intensity, sigma_background_intensity);

            double delta_foreground_background_mu=
               fabs(mu_ER_intensity - mu_background_intensity);
            min_delta_foreground_background_mu=basic_math::min(
               delta_foreground_background_mu,
               min_delta_foreground_background_mu);
            max_sigma_background_intensity=basic_math::max(
               max_sigma_background_intensity,sigma_background_intensity);



         } // loop over index i labeling bright/dark masks

      } // loop over iter index labeling bright/dark ERs

   } // loop over index d labeling digits

   cout << "=================================================================" 
        << endl;

   cout << "n_total_masks = " << n_total_masks << endl;

   cout << "min_cc_pixel_size = " << min_cc_pixel_size << endl;
   cout << "max_cc_pixel_size = " << max_cc_pixel_size << endl;
   cout << "max_CC_basename = " << max_CC_basename << endl;
   cout << endl;

   cout << "min_bbox_width = " << min_bbox_width << endl;
   cout << "min_bbox_height = " << min_bbox_height << endl;

   cout << "max_bbox_width = " << max_bbox_width << endl;
   cout << "max_bbox_width_basename = " << max_bbox_width_basename << endl;
   cout << "max_bbox_height = " << max_bbox_height << endl;
   cout << "max_bbox_height_basename = " << max_bbox_height_basename << endl;

   cout << "min_aspect_ratio  = " << min_aspect_ratio << endl;
   cout << "max_aspect_ratio  = " << max_aspect_ratio << endl;

   cout << "max_ER_delta_intensity = " << max_ER_delta_intensity << endl;
   cout << "min_ER_delta_intensity = " << min_ER_delta_intensity << endl;
   cout << "max_sigma_ER_intensity = " << max_sigma_ER_intensity << endl;
   cout << "max_sigma_ER_basename = " << max_sigma_ER_basename << endl;

   cout << "max_Euler_number = " << max_Euler_number << endl;
   cout << "min_Euler_number = " << min_Euler_number << endl;
   cout << "max_n_holes = " << 1-min_Euler_number << endl;
   cout << "min_n_holes = " << 1-max_Euler_number << endl;

   cout << "sqrt(max_sqr_inverse_compactness) = " 
        << sqrt(max_sqr_inverse_compactness) << endl;
   cout << "sqrt(min_sqr_inverse_compactness) = " 
        << sqrt(min_sqr_inverse_compactness) << endl;

   cout << "max bbox mu intensity = " << max_bbox_intensity_mu << endl;
   cout << "min bbox mu intensity = " << min_bbox_intensity_mu << endl;
   cout << "max bbox sigma intensity = " << max_bbox_intensity_sigma << endl;
   cout << "min bbox sigma intensity = " << min_bbox_intensity_sigma << endl;

   cout << "min_delta_foreground_background_mu = "
        << min_delta_foreground_background_mu << endl;
   cout << "max_sigma_background_intensity = "
        << max_sigma_background_intensity << endl;
}


/*
// We want to minimize expensive connected component construction.  So
// we impose a few basic criteria which extremal regions corresponding
// to characters are very likely to satisfy:

// 1.  Ignore any extremal region containing too few or too many
// pixels to be of interest:

      if (region_ptr->pixel_IDs.size() < min_cc_pixel_size) continue;
      if (region_ptr->pixel_IDs.size() > max_cc_pixel_size) continue;

// 2. Ignore any extremal regions whose bbox horiz/vert pixel sizes
// are too small or large:

      int bbox_width = region_ptr->bbox_max_px-region_ptr->bbox_min_px;
      int bbox_height = region_ptr->bbox_max_py-region_ptr->bbox_min_py;

      if (bbox_width < 0.66 * width) continue;
      if (bbox_height < 0.66 * height) continue;

      if (bbox_width > 0.95 * width && bbox_height > 0.95 * height) continue;

// 3.  Ignore any extremal region whose brightness variation exceeds
// some threshold fraction of the maximum possible brightness
// difference:

      if (region_ptr->max_value - region_ptr->min_value > max_value_spread)
         continue;

// 4.  Ignore any extremal regions whose "inverse compactness" =
// perimeter / sqrt(area) exceeds some large threshold:

      double inverse_compactness=double(region_ptr->pixel_perim)/
         sqrt(region_ptr->pixel_IDs.size());
      if (inverse_compactness > max_inverse_compactness_for_chars) continue;

// 4.  Ignore any extremal regions whose number of 4-holes exceeds
// some threshold:

      int n_four_holes=basic_math::max(0,1-region_ptr->Euler_number);
      if (n_four_holes > max_n_four_holes) continue;


// Parameter thresholds based upon 1020 cleaned masks:

min_cc_pixel_size = 62
max_cc_pixel_size = 15998

min_bbox_width = 7
min_bbox_height = 9
max_bbox_width = 130
max_bbox_height = 201

min_aspect_ratio  = 0.302326
max_aspect_ratio  = 1.18182

max_ER_delta_intensity = 0.486275
min_ER_delta_intensity = 0
max_sigma_ER_intensity = 0.155921

max_Euler_number = 1
min_Euler_number = -1
max_n_holes = 2
min_n_holes = 0

sqrt(max_sqr_inverse_compactness) = 11.7191
sqrt(min_sqr_inverse_compactness) = 4.17029

max bbox mu intensity = 0.812686
min bbox mu intensity = 0.0339702
max bbox sigma intensity = 0.332331
min bbox sigma intensity = 0.0167477
min_delta_foreground_background_mu = 0.0275733
max_sigma_background_intensity = 0.199364

*/


