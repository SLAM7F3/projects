// ==========================================================================
// Program FACETEST
// ==========================================================================
// Last updated on 7/30/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "image/imagefuncs.h"
#include "video/videofuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

//   string input_image_filename="test_image.jpg";
//   string input_image_filename="female_face_00001_106x106.jpg";
   string input_image_filename="male_face_99630_106x106.jpg";
//   string input_image_filename="male_face_09389_106x106.jpg";
   string output_image_filename="output_image.jpg";
   
   imagefunc::crop_image(input_image_filename, output_image_filename,
                         96, 96, 5, 5);

/*
   Magick::Image curr_image;
   curr_image.backgroundColor(Magick::Color(0,0,0));
   videofunc::import_IM_image(input_image_filename, curr_image);

//   void resize_image(
//      Magick::Image& curr_image,unsigned int new_xdim,unsigned int new_ydim);
//   void gaussian_blur_image(Magick::Image& curr_image,double sigma);

   double theta = -25;
   videofunc::crop_rotate_image(curr_image, theta);
   videofunc::export_IM_image(output_image_filename, curr_image);
*/


}

