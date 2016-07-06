// ==========================================================================
// Program EQUALIZE is a play-pen for histogram equalization.

// ==========================================================================
// Last updated on 5/9/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

#include "video/camerafuncs.h"
#include "geometry/plane.h"
#include "math/rotation.h"
#include "video/videofuncs.h"

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

   string images_subdir="./images/";
//   string image_filename=images_subdir+"kermit000.jpg";
   string cropped_images_subdir=images_subdir+
      "HouseNumbers/van/cropped_images/";
   string orig_images_subdir=images_subdir+
      "HouseNumbers/van/original_images/";
//   string image_filename=cropped_images_subdir+"cropped_numbers4.jpg";
//   string image_filename=cropped_images_subdir+"cropped_numbers5.jpg";
//   string image_filename=cropped_images_subdir+"cropped_hn13.jpg";
   string image_filename=orig_images_subdir+"housenumbers1.jpg";

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->import_photo_from_file(image_filename);

   int xdim=texture_rectangle_ptr->getWidth();
   int ydim=texture_rectangle_ptr->getHeight();
   twoDarray* ztwoDarray_ptr=new twoDarray(xdim,ydim);

   double h,s,v;
   for (int py=0; py<ydim; py++)
   {
      for (int px=0; px<xdim; px++)
      {
         texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
         ztwoDarray_ptr->put(px,py,255*v);
      } // loop over px index
   } // loop over py index
   
   imagefunc::equalize_intensity_histogram(ztwoDarray_ptr);


   for (int py=0; py<ydim; py++)
   {
      for (int px=0; px<xdim; px++)
      {
         texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
         texture_rectangle_ptr->set_pixel_hsv_values(
            px,py,h,s,ztwoDarray_ptr->get(px,py)/255.0);
      } // loop over px index
   } // loop over py index

   delete ztwoDarray_ptr;
      
   string output_filename="hist_equalized.jpg";
   texture_rectangle_ptr->write_curr_frame(output_filename);

   delete texture_rectangle_ptr;

   string banner="Exported output image to "+output_filename;
   outputfunc::write_big_banner(banner);
} 

