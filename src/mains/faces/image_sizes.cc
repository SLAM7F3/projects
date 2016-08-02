// ==========================================================================
// Program IMAGE_SIZES reads in a set of photo image filenames.  It
// calls ImageMagick's ping command to extract image width and height
// measured in pixels.  Image size results are saved into output text
// file image_sizes.dat.

//                             ./image_sizes

// ==========================================================================
// Last updated 7/7/16; 7/19/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   timefunc::initialize_timeofday_clock(); 
   string input_imagesdir = "./";
//   string input_imagesdir = "./homogenized_images/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_imagesdir);
   cout << "Imported " << image_filenames.size() << " images" << endl;

   ofstream outstream;
   string output_filename=input_imagesdir+"image_sizes.dat";
   filefunc::openfile(output_filename, outstream);
   outstream << "# Image basename  xdim  ydim " << endl << endl;

   int n_images = image_filenames.size();
   for(int i = 0; i < n_images; i++)
   {
      if(i%5000 == 0)
      {
         double progress_frac = double(i)/double(n_images);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_filename=image_filenames[i];
      int width, height;
      imagefunc::get_image_width_height(image_filename, width, height);
      outstream << filefunc::getbasename(image_filenames[i])
                << "  " << width
                << "  " << height << endl;

      if(imagefunc::corrupted_jpg_file(image_filename))
      {
         cout << "image_filename = " << image_filename << " is corrupted!"
              << endl;
         outputfunc::enter_continue_char();
      }

      if(width != 106 || height != 106)
      {
         cout << "i = " << i 
              << " image_filename = " << image_filename
              << " width = " << width
              << " height = " << height
              << endl;
//         filefunc::deletefile(image_filename);
         outputfunc::enter_continue_char();
      }

   }

   filefunc::closefile(output_filename, outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_banner(banner);
}

