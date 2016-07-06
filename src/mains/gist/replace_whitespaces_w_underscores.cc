// ==========================================================================
// Program REPLACE_WHITESPACES_W_UNDERSCORES scans through all flickr
// image filenames.  It replaces any space, tab or parentheses in
// input flickr image filenames with underscores.

//		  ./replace_whitespaces_w_underscores

// ==========================================================================
// Last updated on 4/13/13; 4/14/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   timefunc::initialize_timeofday_clock();

   string flickr_subdir="/media/LACIE_SHARE_/gist_images/Random_pictures_2/";
//   string flickr_subdir="./all_images/flickr/";

   vector<string> input_image_subdirs;
   input_image_subdirs.push_back(flickr_subdir);

/*
   input_image_subdirs.push_back(flickr_subdir+"coast/");
   input_image_subdirs.push_back(flickr_subdir+"coast_testing/");
   input_image_subdirs.push_back(flickr_subdir+"coast_training/");
   input_image_subdirs.push_back(flickr_subdir+"forest/");
   input_image_subdirs.push_back(flickr_subdir+"forest/letter_images/");
   input_image_subdirs.push_back(flickr_subdir+"forest/number_images/");
   input_image_subdirs.push_back(flickr_subdir+"highway/");
   input_image_subdirs.push_back(flickr_subdir+"insidecity/");
   input_image_subdirs.push_back(flickr_subdir+"mountain/");
   input_image_subdirs.push_back(flickr_subdir+"opencountry/");
   input_image_subdirs.push_back(flickr_subdir+"random_examples/");
   input_image_subdirs.push_back(flickr_subdir+"street/");
   input_image_subdirs.push_back(flickr_subdir+"tallbldg/");
*/

   for (unsigned int input_image_subdir_index=0; input_image_subdir_index < 
           input_image_subdirs.size(); input_image_subdir_index++)
   {
      outputfunc::print_elapsed_time();
      string curr_image_subdir=input_image_subdirs[input_image_subdir_index];
      
      vector<string> image_filenames=filefunc::image_files_in_subdir(
         curr_image_subdir);
      int n_images=image_filenames.size();
      cout << "n_images = " << n_images << endl;

      int imagenumber_start=0;
      int imagenumber_stop=n_images-1;
      for (int imagenumber=imagenumber_start; imagenumber<=imagenumber_stop; 
           imagenumber++)
      {
         string input_filename=image_filenames[imagenumber];
         string dirname=filefunc::getdirname(input_filename);
         string image_basename=filefunc::getbasename(input_filename);

// Alter any image basename which contains white spaces, dashes or
// parentheses:

         string separator_chars=" -()\t\n";
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               image_basename,separator_chars);
         if (substrings.size()==1) continue;

         string new_basename="";
         for (unsigned int s=0; s<substrings.size(); s++)
         {
            new_basename += substrings[s];
            if (s < substrings.size()-1) new_basename += "_";
         }

         cout << "imagenumber = " << imagenumber 
              << " Renaming " << image_basename << endl;
//         cout << "Revised basename = " << new_basename << endl;
         string output_filename=dirname+new_basename;
         string unix_cmd="mv '"+input_filename+"' "+output_filename;
//         cout << "unix_cmd = " << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);

      } // loop over imagenumber index labeling input images

   } // loop over input_image_subdir_index
}

