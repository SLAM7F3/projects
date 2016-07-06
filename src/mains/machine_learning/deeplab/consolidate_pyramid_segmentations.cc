// ==========================================================================
// Program CONSOLIDATE_PYRAMID_SEGMENTATIONS imports segmented image
// outputs generated at half-size, full-size and double-size scales.
// It identifies pixels in each of these scales which have nontrivial
// hues (indicating at least binary classification for some object
// such as face or text).  The corresponding pixel location(s) within
// a consolidated image mask at full-size scale are colored.  

// A set of consolidated image masks are exported to
// consolidated_subdir.
// ==========================================================================
// Last updated on 5/22/16; 5/24/16; 5/27/16; 6/10/16
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
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
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string banner="Starting CONSOLIDATE_PYRAMID_SEGMENTATIONS program";
   outputfunc::write_big_banner(banner);

   bool include_foursized_images = false;
//   bool include_foursized_images = true;
//   bool include_doublesized_images = false;
   bool include_doublesized_images = true;
//   bool include_halfsized_images = false;
   bool include_halfsized_images = true;

   const int scale_pyramid_threshold = 550;	 // pixels   deeplab v1
//   const int scale_pyramid_threshold = 450;	 // pixels   deeplab v2

   if(argc != 2)
   {
      cout << "Must pass input_images_subdir (e.g. '/home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/images/faces/testing_images_05/') as command-line argument" << endl;
      exit(-1);
   }
   string input_images_subdir(argv[1]);
   filefunc::add_trailing_dir_slash(input_images_subdir);
   cout << "input_images_subdir = " << input_images_subdir << endl;

   string halfsized_subdir = input_images_subdir+"halfsized/";
   string fullsized_subdir = input_images_subdir+"fullsized/";
   string doublesized_subdir = input_images_subdir+"doublesized/";
   string foursized_subdir = input_images_subdir+"foursized/";

   vector<string> fullsized_image_filenames=
      filefunc::image_files_in_subdir(fullsized_subdir);
   int n_images = fullsized_image_filenames.size();
   cout << "n_images = " << n_images << endl;

   vector<string> image_ID_strings;
   for(int i = 0; i < n_images; i++)
   {
      string basename=filefunc::getbasename(fullsized_image_filenames[i]);
      string image_ID_str = basename.substr(6,5);
      image_ID_strings.push_back(image_ID_str);
   }

   string consolidated_subdir = input_images_subdir+"consolidated/";
   filefunc::dircreate(consolidated_subdir);
   string recombined_basedir="recombined_segmentations";
//   cout << "Enter recombined segmentations subdir relative to "
//        << text_database_subdir << endl;
//   cin >> recombined_basedir;
   filefunc::add_trailing_dir_slash(recombined_basedir);

   string halfsized_recombined_subdir=halfsized_subdir+recombined_basedir;
   string fullsized_recombined_subdir=fullsized_subdir+recombined_basedir;
   string doublesized_recombined_subdir=doublesized_subdir+recombined_basedir;
   string foursized_recombined_subdir=doublesized_subdir+recombined_basedir;

   vector<string> fullsized_segmented_image_filenames=
      filefunc::image_files_in_subdir(fullsized_recombined_subdir);
   vector<string> halfsized_segmented_image_filenames=
      filefunc::image_files_in_subdir(halfsized_recombined_subdir);
   vector<string> doublesized_segmented_image_filenames=
      filefunc::image_files_in_subdir(doublesized_recombined_subdir);
   vector<string> foursized_segmented_image_filenames=
      filefunc::image_files_in_subdir(foursized_recombined_subdir);

// On 5/22/16, we learned the hard and painful way that the total
// number of fullsized, halfsized and doublesized segmented image
// files may not be equal!  

   string image_filename;
   texture_rectangle *tr_ptr = NULL;
   texture_rectangle *halfsized_tr_ptr = NULL;
   texture_rectangle *doublesized_tr_ptr = NULL;
   texture_rectangle *foursized_tr_ptr = NULL;

   unsigned int istart = 0;
   unsigned int istop = image_ID_strings.size();
   for(unsigned int i = istart; i < istop; i++)
   {
      cout << "Consolidating fullsized, halfsized and doublesized images "
           << i << endl;

      double progress_frac = double(i - istart)/(istop - istart);
      if(i%10 == 0)
      {
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string curr_ID_str = image_ID_strings[i];
      string fullsized_segmented_filename = 
         fullsized_recombined_subdir+"segmented_image_"+curr_ID_str+".png";
      string halfsized_segmented_filename = 
         halfsized_recombined_subdir+"segmented_image_"+curr_ID_str+".png";
      string doublesized_segmented_filename = 
         doublesized_recombined_subdir+"segmented_image_"+curr_ID_str+".png";
      string foursized_segmented_filename = 
         foursized_recombined_subdir+"segmented_image_"+curr_ID_str+".png";

      if(!filefunc::fileexist(fullsized_segmented_filename)) continue;

      tr_ptr = new texture_rectangle(fullsized_segmented_filename, NULL);
      int full_xdim = tr_ptr->getWidth();
      int full_ydim = tr_ptr->getHeight();
      double purple_h = 300;

// Transfer text pixel labels from half-sized segmented images to
// full-sized segmented image:

      if(filefunc::fileexist(halfsized_segmented_filename) &&
         include_halfsized_images && 
         (full_xdim >= 2 * scale_pyramid_threshold || 
          full_ydim >= 2 * scale_pyramid_threshold) )
      {
         delete halfsized_tr_ptr;
         halfsized_tr_ptr = new texture_rectangle(
            halfsized_segmented_filename, NULL);

         for(unsigned int qy = 0; qy < halfsized_tr_ptr->getHeight(); qy++)
         {
            for(unsigned int qx = 0; qx < halfsized_tr_ptr->getWidth(); 
                qx++)
            {
               double h, s, v;
               halfsized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
               
               if(s > 0)
               {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;
                  double halfsized_h = purple_h;
                  double smin = 0.66;
                  double smax = 1.0;
                  double vmin = 0.5;
                  double vmax = 1.0;
                  
                  tr_ptr->get_pixel_hsv_values(2*qx, 2*qy, h, s, v);
                  s = smin + s * (smax - smin);
                  v = vmin + v * (vmax - vmin);
                  tr_ptr->set_pixel_hsv_values(2*qx,2*qy,halfsized_h, s, v);
                     
                  tr_ptr->get_pixel_hsv_values(2*qx+1, 2*qy, h, s, v);
                  s = smin + s * (smax - smin);
                  v = vmin + v * (vmax - vmin);
                  tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy,halfsized_h,s,v);
                  
                  tr_ptr->get_pixel_hsv_values(2*qx, 2*qy+1, h, s, v);
                  s = smin + s * (smax - smin);
                  v = vmin + v * (vmax - vmin);
                  tr_ptr->set_pixel_hsv_values(2*qx,2*qy+1,halfsized_h,s,v);
                  
                  tr_ptr->get_pixel_hsv_values(2*qx+1, 2*qy+1, h, s, v);
                  s = smin + s * (smax - smin);
                  v = vmin + v * (vmax - vmin);
                  tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy+1,halfsized_h,s,v);
               }
            } // loop over qx
         } // loop over qy
      } // half_sized_segmented_filename exists conditional

      if(filefunc::fileexist(foursized_segmented_filename) &&
         include_foursized_images &&
         (full_xdim <= 0.85 * scale_pyramid_threshold || 
          full_ydim <= 0.85 * scale_pyramid_threshold) )
      {
         delete foursized_tr_ptr;
         foursized_tr_ptr = new texture_rectangle(
            foursized_segmented_filename, NULL);

         for(unsigned int qy = 0; qy < foursized_tr_ptr->getHeight(); 
             qy++)
         {
            for(unsigned int qx = 0; qx < foursized_tr_ptr->getWidth(); 
                qx++)
            {
               double h, s, v;
               foursized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
               if(s > 0)
               {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;
                  double foursized_h = purple_h;
                  double smin = 0.66;
                  double smax = 1.0;
                  double vmin = 0.5;
                  double vmax = 1.0;

                  tr_ptr->get_pixel_hsv_values(qx/4, qy/4, h, s, v);
                  s = smin + s * (smax - smin);
                  v = vmin + v * (vmax - vmin);

// Don't color pixel again if it's already colored

                  if(fabs(h-purple_h) > 20){
                     tr_ptr->set_pixel_hsv_values(qx/4,qy/4,foursized_h,s,v);
                  }
               }
            } // loop over qx
         } // loop over qy
      } // foursized_segmented_filename exists conditional

      if(filefunc::fileexist(doublesized_segmented_filename) &&
         include_doublesized_images &&
         (full_xdim <= scale_pyramid_threshold || 
          full_ydim <= scale_pyramid_threshold) )
      {
         delete doublesized_tr_ptr;
         doublesized_tr_ptr = new texture_rectangle(
            doublesized_segmented_filename, NULL);

         for(unsigned int qy = 0; qy < doublesized_tr_ptr->getHeight(); 
             qy++)
         {
            for(unsigned int qx = 0; qx < doublesized_tr_ptr->getWidth(); 
                qx++)
            {
               double h, s, v;
               doublesized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
               if(s > 0)
               {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;
                  double doublesized_h = purple_h;
                  double smin = 0.66;
                  double smax = 1.0;
                  double vmin = 0.5;
                  double vmax = 1.0;

                  tr_ptr->get_pixel_hsv_values(qx/2, qy/2, h, s, v);
                  s = smin + s * (smax - smin);
                  v = vmin + v * (vmax - vmin);

// Don't color pixel again if it's already colored

                  if(fabs(h-purple_h) > 20){
                     tr_ptr->set_pixel_hsv_values(qx/2,qy/2,doublesized_h,s,v);
                  }

               }
            } // loop over qx
         } // loop over qy
      } // doublesized_segmented_filename exists conditional

// Loop over all pixels within segmented texture rectangle.  Count
// total number whose saturation != 0:

      for(unsigned int qy = 0; qy < tr_ptr->getHeight(); qy++)
      {
         for(unsigned int qx = 0; qx < tr_ptr->getWidth(); qx++)
         {
            double h, s, v;
            tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
            if(s > 0)
            {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;

               double fullsized_h = purple_h;
               tr_ptr->set_pixel_hsv_values(qx,qy,fullsized_h,s,v);
            }
         } // loop over qx
      } // loop over qy

      string consolidated_segmented_image = consolidated_subdir+
         "segmented_image_"+curr_ID_str+".png";
      tr_ptr->write_curr_frame(consolidated_segmented_image);
      delete tr_ptr;

   } // loop over index i labeling segmented images

   banner="Exported consolidated image masks to "+consolidated_subdir;
   outputfunc::write_banner(banner);
}

