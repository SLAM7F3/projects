// =======================================================================
// Program MOSAIC imports a set of orthorectified aerial images.  It
// sorts them based upon their ground footprint size.  MOSAIC then
// simply overlays each footprint onto a composite.  The last
// overlayed image is the one with the largest footprint.
// =======================================================================
// Last updated on 12/5/13; 12/6/13; 12/10/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string rectified_images_subdir=bundler_IO_subdir+"rectified_images/";
   filefunc::dircreate(rectified_images_subdir);
   string mosaic_images_subdir=rectified_images_subdir+"subset/";
   filefunc::dircreate(mosaic_images_subdir);

// Import orthorectified aerial images from specified subdirectory:

   vector<string> image_files=filefunc::image_files_in_subdir(
      mosaic_images_subdir);

   texture_rectangle* mosaic_texture_rectangle_ptr=new texture_rectangle(
      image_files.back(),NULL);
   texture_rectangle* curr_texture_rectangle_ptr=new texture_rectangle(
      image_files.back(),NULL);

   int width=mosaic_texture_rectangle_ptr->getWidth();
   int height=mosaic_texture_rectangle_ptr->getWidth();

   string mosaic_filename="mosaic.jpg";
   mosaic_texture_rectangle_ptr->generate_blank_image_file(
      width,height,mosaic_filename,0);
   
// First count number of non-null valued RGB pixels in each input
// orthorectified image:

   vector<int> image_RGB_count;

   int R,G,B;
   int null_threshold=5;
   int null_value=127;
   for (int i=0; i<image_files.size(); i++)
   {
//       cout << "i = " << i << endl;
      curr_texture_rectangle_ptr->import_photo_from_file(image_files[i]);

      int RGB_counter=0;
      for (int px=0; px<width; px++)
      {
         for (int py=0; py<height; py++)
         {
            curr_texture_rectangle_ptr->get_pixel_RGB_values(
               px,py,R,G,B);
            if (fabs(R-null_value) < null_threshold && 
            fabs(G-null_value) < null_threshold && 
            fabs(B-null_value) < null_threshold) continue;
            if (R < 0 || G < 0 || B < 0) continue;

            RGB_counter++;
         }
      }
      image_RGB_count.push_back(RGB_counter);

   } // loop over index i labeling input images

// Sort image filenames by their RGB counts:

   templatefunc::Quicksort(image_RGB_count,image_files);

// Form poor-man's mosaic by simply laying orthrectified images on top
// of each other.  Last image to be laid down is the one with the
// largest ground footprint:

   for (int i=0; i<image_files.size(); i++)
   {
//       cout << "i = " << i << endl;
      curr_texture_rectangle_ptr->import_photo_from_file(image_files[i]);

      for (int px=0; px<width; px++)
      {
         for (int py=0; py<height; py++)
         {
            curr_texture_rectangle_ptr->get_pixel_RGB_values(
               px,py,R,G,B);
            if (fabs(R-null_value) < null_threshold && 
            fabs(G-null_value) < null_threshold && 
            fabs(B-null_value) < null_threshold) continue;
            if (R < 0 || G < 0 || B < 0) continue;

            mosaic_texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         }
      }
   } // loop over index i labeling input images

// Search for cropped bounding box which snuggly encloses mosaic:

   int px=0;
   int column_integral=0;
   while (column_integral==0)
   {
      for (int py=0; py<height; py++)
      {
         mosaic_texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
         column_integral += (R+G+B);
      }
//      cout << "px_lo = " << px_lo 
//           << " column_integral = " << column_integral << endl;
      px++;
   }
   int px_lo=px;
//   cout << "px_lo = " << px_lo << endl;

   px=width-1;
   column_integral=0;
   while (column_integral==0)
   {
      for (int py=0; py<height; py++)
      {
         mosaic_texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
         column_integral += (R+G+B);
      }
//      cout << "px_lo = " << px_lo 
//           << " column_integral = " << column_integral << endl;
      px--;
   }
   int px_hi=px;
//   cout << "px_hi = " << px_hi << endl;

   int py=0;
   int row_integral=0;
   while (row_integral==0)
   {
      for (int px=0; px<width; px++)
      {
         mosaic_texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
         row_integral += (R+G+B);
      }
      py++;
   }
   int py_lo=py;
//   cout << "py_lo = " << py_lo << endl;

   py=height-1;
   row_integral=0;
   while (row_integral==0)
   {
      for (int px=0; px<width; px++)
      {
         mosaic_texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
         row_integral += (R+G+B);
      }
      py--;
   }
   int py_hi=py;
//   cout << "py_hi = " << py_hi << endl;

   int new_py_hi=height-py_lo;
   int new_py_lo=height-py_hi;

   mosaic_texture_rectangle_ptr->write_curr_frame(
      px_lo,px_hi,new_py_lo,new_py_hi,mosaic_filename);

   delete mosaic_texture_rectangle_ptr;
   delete curr_texture_rectangle_ptr;

   string banner="Exported mosaic to "+mosaic_filename;
   outputfunc::write_big_banner(banner);
}
