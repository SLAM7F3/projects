// ==========================================================================
// Program MINI_IMAGE
// ==========================================================================
// Last updated on 10/1/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "image/binaryimagefuncs.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::ofstream;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
//   int width=17;
//   int height=9;

   int width=7;
   int height=7;
   int n_images=1;
   int n_channels=3;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      width,height,n_images,n_channels,NULL);

   string blank_filename="blank.jpg";
   double grey_level=0.5;
   texture_rectangle_ptr->generate_blank_image_file(
      width,height,blank_filename,grey_level);

   int R,G,B;
   for (int pu=0; pu<width; pu++)
   {
      for (int pv=0; pv<height; pv++)
      {
         R=G=B=0;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      } // loop over pv index
   } // loop over pu index
   
   R=G=B=255;
   texture_rectangle_ptr->set_pixel_RGB_values(5,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(5,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(1,5,R,G,B);

/*
   texture_rectangle_ptr->set_pixel_RGB_values(2,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(3,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(6,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(7,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(10,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(11,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(14,1,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(15,1,R,G,B);

   texture_rectangle_ptr->set_pixel_RGB_values(1,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(2,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(3,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(4,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(5,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(6,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(7,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(8,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(11,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(12,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(13,2,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(14,2,R,G,B);

   texture_rectangle_ptr->set_pixel_RGB_values(3,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(4,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(5,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(6,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(10,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(11,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(12,3,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(13,3,R,G,B);

   texture_rectangle_ptr->set_pixel_RGB_values(2,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(3,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(4,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(5,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(9,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(10,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(11,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(14,4,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(15,4,R,G,B);

   texture_rectangle_ptr->set_pixel_RGB_values(1,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(2,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(3,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(6,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(7,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(11,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(12,5,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(13,5,R,G,B);

   texture_rectangle_ptr->set_pixel_RGB_values(2,6,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(3,6,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(9,6,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(10,6,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(14,6,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(15,6,R,G,B);

   texture_rectangle_ptr->set_pixel_RGB_values(6,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(7,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(8,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(9,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(12,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(13,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(14,7,R,G,B);
   texture_rectangle_ptr->set_pixel_RGB_values(15,7,R,G,B);
*/
   string output_filename="mini_binary.jpg";
//   string output_filename="wiki_binary.jpg";
   texture_rectangle_ptr->write_curr_frame(output_filename);

   delete texture_rectangle_ptr;
}

