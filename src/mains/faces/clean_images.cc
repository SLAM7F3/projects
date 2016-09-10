// =======================================================================
// Program CLEAN_IMAGES scans through all images within a specified
// subdirectory.  It first converts any PNG files into JPG images to
// avoid alpha-channel headaches.  It next renames an ".jpeg" or
// ".JPG" suffixes as ".jpg".  Finally, it explicitly checks the
// number of color channels within the JPG files.  CLEAN_IMAGES moves
// any JPG file which doesn't have exactly 3 color channels into an
// eightbit_images subdirectory.

// We wrote this utility in order to homogenize highly-variable
// internet imagery.

//	   		     ./clean_images

// =======================================================================
// Last updated on 5/9/16; 9/10/16
// =======================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

   string faces_subdir = "./images/faces/";
//   string wildtext_subdir = "./images/wildtext/";
   string root_subdir = faces_subdir;
//    string root_subdir = wildtext_subdir;

   cout << "Enter input images basedir relative to " << root_subdir 
        << ":" << endl;
   string input_images_subdir;
   input_images_subdir="testing_images_01";
//   cin >> input_images_subdir;
   filefunc::add_trailing_dir_slash(input_images_subdir);
   input_images_subdir=root_subdir+input_images_subdir;

   input_images_subdir = 
      "./";
//      "/home/pcho/Downloads/test_faces/";
//      "/media/DataTransfer/faces/google_images/Jul2016/gimages21/";

   string eightbit_images_subdir = input_images_subdir+
      "eightbit_images/";
   filefunc::dircreate(eightbit_images_subdir);
   

   vector<string> image_filenames=
      filefunc::image_files_in_subdir(input_images_subdir);
   int n_orig_filenames = image_filenames.size();
   cout << "n_orig_filenames = " << n_orig_filenames << endl;
   int n_eightbit_images = 0;
      
   int istart = 0;
   cout <<  "Enter starting image number:" << endl;
   cin >> istart;
   int istop = image_filenames.size();

   timefunc::initialize_timeofday_clock(); 

   for (int i = istart; i < istop; i++)
   {
      if(i%100 == 0)
      {
         double progress_frac = double(i - istart)/double(istop - istart);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string currimage_filename=image_filenames[i];
      string currimage_suffix = stringfunc::suffix(currimage_filename);
      string currjpg_filename = currimage_filename;
      if(currimage_suffix=="png" || currimage_suffix =="PNG")
      {

// As of 4/7/16, we force input images to be in JPG rather than PNG
// format to avoid alpha channel headaches.  Delete original PNG file
// after its JPG replacement has been generated:

         string prefix = stringfunc::prefix(currimage_filename);
         currjpg_filename=prefix+".jpg";
         string unix_cmd = "convert "+currimage_filename+" "+currjpg_filename;
         cout << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
         if(!filefunc::deletefile(currimage_filename))
         {
            cout << "Couldn't delete " << currimage_filename << endl;
            outputfunc::enter_continue_char();
         }
      }

// Force all JPEG files to end with a ".jpg" suffix:

      else if (currimage_suffix=="JPG" || currimage_suffix=="jpeg")
      {
         string prefix = stringfunc::prefix(currimage_filename);
         currjpg_filename=prefix+".jpg";
         string unix_cmd = "mv "+currimage_filename+" "+currjpg_filename;
         sysfunc::unix_command(unix_cmd);
      }

      texture_rectangle *tr_ptr = new texture_rectangle(currjpg_filename,NULL);
      int n_channels = tr_ptr->getNchannels();
      if(n_channels != 3)
      {
         cout << "currjpg_filename  = " << currjpg_filename
              << " has n_channels = " << n_channels << endl;
         n_eightbit_images++;
         cout << "n_eightbit_images = " << n_eightbit_images << endl;

         string unix_cmd = "mv "+currjpg_filename+" "+
            eightbit_images_subdir;
         sysfunc::unix_command(unix_cmd);
      }
   } // loop over index i labeling all input images

   vector<string> cleaned_image_filenames=
      filefunc::image_files_in_subdir(input_images_subdir);
   int n_final_filenames = cleaned_image_filenames.size();
   cout << "n_final_filenames = " << n_final_filenames << endl;
}
