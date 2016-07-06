// =======================================================================
// Program LINK_RENAME_IMAGES imports a set of raw image filenames
// from a specified subdirectory.  It first alphabetically sorts the
// input image filenames.  It next wraps the filenames in double
// quotes to shield subsequent link commands from white spaces.
// After querying the user to enter the first image's ID,
// LINK_RENAME_IMAGES generates an executable script which links (or
// copies) the input raw filenames to homogenized and ordered output
// filenames.

//	 		./link_rename_images

// =======================================================================
// Last updated on 12/1/13; 1/2/16; 5/4/16
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

//   string root_subdir="/home/cho/Desktop/profile_faces/";
//   string images_subdir=root_subdir+"new_raw/";
//   string images_subdir=root_subdir+"difficult/resized_images/";

//   string root_subdir="/home/cho/Downloads/people/";
//   string images_subdir=root_subdir+"more_standing_walking/";
//   string images_subdir="//media/MoreData/bundler/pear_tree/iPhone_images/";
   
//   string root_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/machine_learning/faces/";
//   string images_subdir=root_subdir+"google_images/batch1/";
//   string images_subdir=root_subdir+"google_images/all_google_images/";

   //   string root_subdir="/media/DataTransfer/faces/";
//   string images_subdir=root_subdir+"flickr/";
   //   string images_subdir=root_subdir+"NewsWraps/early_Sep_2013/";

   //   string images_subdir=
   //     "/home/pcho/programs/c++/svn/projects/src/mains/syntext/images/internet/backgrounds/";
   string images_subdir="./";
   string output_subdir = "./homogenized_images/";
   filefunc::dircreate(output_subdir);

   vector<string> image_filenames=
      filefunc::image_files_in_subdir(images_subdir);
   
// On 10/29/13, we discovered the painful way that image_filenames is
// generally NOT sorted alphabetically (or by date).  So we explicitly
// sort the strings within STL vector image_filenames:

   std::sort(image_filenames.begin(),image_filenames.end());

   string output_filename=images_subdir+"rename_image_files";
//   string output_filename=images_subdir+"link_image_files";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   int image_ID_offset=0;
   cout << "Enter starting image's ID:" << endl;
   cin >> image_ID_offset;
   
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      string curr_image_filename=image_filenames[i];
      cout << "curr_image_filename = " << curr_image_filename << endl;

// Search for any white spaces or parentheses within input image
// filenames.  If found, replace them with underscores:

      string dirname=filefunc::getdirname(curr_image_filename);
      string basename=filefunc::getbasename(curr_image_filename);
      string separator_chars=" '()\t\n";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename,separator_chars);
      string new_basename="";
      for (unsigned int s=0; s<substrings.size()-1; s++)
      {
         new_basename += substrings[s]+"_";
      }
      new_basename += substrings.back();
      curr_image_filename=dirname+new_basename;
      if (curr_image_filename != image_filenames[i])
      {
//         cout << "i = " << i << " image_filenames[i] = "
//              << image_filenames[i] << endl;
//         cout << "curr_image_filename = " << curr_image_filename << endl;
         string unix_cmd="mv \""+image_filenames[i]+"\" "+
            curr_image_filename;
         sysfunc::unix_command(unix_cmd);
//         outputfunc::enter_continue_char();
      }

// On 5/7/12, we discovered the hard way that input JPG files
// (e.g. from flickr) can be corrupted.  So explicitly
// check for bad input images and ignore them if found:

      if (!imagefunc::valid_image_file(curr_image_filename))
      {
         cout << "Skipping corrupted image " << curr_image_filename
              << endl;
         continue;
      }

// Form soft link between regularized input image filename and new
// homogenized image filename:

      string image_subdir=filefunc::getdirname(curr_image_filename);
      string suffix=stringfunc::suffix(curr_image_filename);
      int image_ID=i+image_ID_offset;
      
      string homogenized_image_filename=output_subdir+
         "image_"+stringfunc::integer_to_string(
         image_ID,5)+"."+suffix;
//      string unix_cmd="ln -s \""+curr_image_filename
      string unix_cmd="cp \""+curr_image_filename
         +"\" "+homogenized_image_filename;
      outstream << unix_cmd << endl;

   }
   filefunc::closefile(output_filename,outstream);
   filefunc::make_executable(output_filename);
   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}
