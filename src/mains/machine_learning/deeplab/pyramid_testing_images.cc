// ====================================================================
// Program PYRAMID_TESTING_IMAGES imports all testing images within a
// specified input subdirectory.  It downsizes by factors of 2 and 4
// each test image within the input folder.  It also upsamples by a
// factor of 2 each input image.  Finally, PYRAMID_TESTING_IMAGES
// moves all the original images into their own "fullsized"
// subdirectory.

// Run this program on both validation and testing imagery sets.  Then
// move the pyramided images into their own subdir.  Create softlinks
// to all pyramided images.  

// ====================================================================
// Last updated on 5/13/16; 5/25/16; 6/15/16; 7/21/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string faces_subdir = "/data/TrainingImagery/faces/";
   string root_subdir = faces_subdir+"labeled_data/";

   string input_images_basedir = "faces_07_testing_images";
   cout << "Enter input images basedir relative to " << root_subdir << ":" 
        << endl;
   cout << " (e.g. faces_11/testing_images)" << endl;
   cin >> input_images_basedir;

   string input_images_subdir = root_subdir+input_images_basedir;
   filefunc::add_trailing_dir_slash(input_images_subdir);

   //    string quartersized_subdir = input_images_subdir+"quartersized/";
   string halfsized_subdir = input_images_subdir+"halfsized/";
   string fullsized_subdir = input_images_subdir+"fullsized/";
   string doublesized_subdir = input_images_subdir+"doublesized/";
   //   string foursized_subdir = input_images_subdir+"foursized/";

//   filefunc::dircreate(quartersized_subdir);
   filefunc::dircreate(halfsized_subdir);
   filefunc::dircreate(fullsized_subdir);
   filefunc::dircreate(doublesized_subdir);
//   filefunc::dircreate(foursized_subdir);
  
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir);
   cout << "Imported " << image_filenames.size() << " images from "
        << input_images_subdir << endl;

   int istart = 0;
   int istop = image_filenames.size();
   for(int i = istart; i < istop; i++)
   {
      if(i%50 == 0)
      {
         double progress_frac = double(i-istart)/double(istop-istart);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_filename=image_filenames[i];
      string basename=filefunc::getbasename(image_filename);
      cout << "i = " << i << " image_filename = " << image_filename << endl;
      unsigned int width, height;
      imagefunc::get_image_width_height(image_filename,width,height);

/*
      string quartersized_image_filename=quartersized_subdir+
         stringfunc::prefix(basename)+".jpg";
      videofunc::resize_image(
         image_filename,width,height,width/4,height/4,
         quartersized_image_filename);
*/

      string halfsized_image_filename=halfsized_subdir+
         stringfunc::prefix(basename)+".jpg";
      videofunc::resize_image(
         image_filename,width,height,width/2,height/2,
         halfsized_image_filename);

      string doublesized_image_filename=doublesized_subdir+
         stringfunc::prefix(basename)+".jpg";
      videofunc::resize_image(
         image_filename,width,height,2*width,2*height,
         doublesized_image_filename);

/*
      string foursized_image_filename=foursized_subdir+
         stringfunc::prefix(basename)+".jpg";
      videofunc::resize_image(
         image_filename,width,height,4*width,4*height,
         foursized_image_filename);
*/

      string unix_cmd="mv "+image_filename+" "+fullsized_subdir;
      sysfunc::unix_command(unix_cmd);
   }

//   cout << "Exported quarter-sized images to " << quartersized_subdir << endl;
   cout << "Exported half-sized images to " << halfsized_subdir << endl;
   cout << "Moved full-sized images to " << fullsized_subdir << endl;
   cout << "Exported double-sized images to " << doublesized_subdir << endl;
//   cout << "Exported four-sized images to " << foursized_subdir << endl;

}

