// ==========================================================================
// Program CONVERT_GREY_IMAGE_TO_RGB queries the user to enter the
// name for some 8-byte greyscale or 24-byte RGB image.  It
// instantiates a texture_rectangle object which converts the input
// 8-byte greyscale image to a nearly identical 24-byte RGB image.  In
// order to force the output image to really have 3 color channels,
// some small random fluctuations are added to all non-zero valued
// pixels.

// We wrote this utility program in order to work with "black and
// white" images which truly correspond to 3 RGB channels rather
// than a single greyscale channel.

//			 convert_grey_image_to_rgb

// ==========================================================================
// Last updated on 8/7/12; 8/31/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

//   string image_filename="skull.jpg";
//   cout << "Enter image filename:" << endl;
//   cin >> image_filename;

   string subdir="./";
   vector<string> image_files=filefunc::image_files_in_subdir(subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   for (int i=0; i<image_files.size(); i++)
   {
      string image_filename=filefunc::getbasename(image_files[i]);

      texture_rectangle_ptr->import_photo_from_file(image_filename);
      texture_rectangle_ptr->instantiate_ptwoDarray_ptr();

      int n_channels=texture_rectangle_ptr->getNchannels();
//   cout << "Number of color channels = " << n_channels << endl;
      if (n_channels==3) 
      {
         texture_rectangle_ptr->convert_color_image_to_luminosity();
      }
      else if (n_channels==1)
      {
         texture_rectangle_ptr->
            fill_ptwoDarray_from_single_channel_byte_data();
      }
   
      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
   
      texture_rectangle* RGB_texture_rectangle_ptr=
         new texture_rectangle(width,height,1,3,NULL);

      string blank_filename="blank.jpg";
      RGB_texture_rectangle_ptr->generate_blank_image_file(
         width,height,blank_filename,0.5);
      filefunc::deletefile(blank_filename);

      bool randomize_blue_values_flag=true;
      RGB_texture_rectangle_ptr->convert_single_twoDarray_to_three_channels(
         texture_rectangle_ptr->get_ptwoDarray_ptr(),
         randomize_blue_values_flag);

      string output_RGB_filename="RGB_"+image_filename;
      RGB_texture_rectangle_ptr->write_curr_frame(output_RGB_filename);

   
      string banner="Exported 3-channel RGB version of input file to "+
         output_RGB_filename;
      outputfunc::write_big_banner(banner);

   }

   delete texture_rectangle_ptr;
      
}

