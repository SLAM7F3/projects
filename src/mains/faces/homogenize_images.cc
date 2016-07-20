// =======================================================================
// Program HOMOGENIZE_IMAGES imports a set of raw image filenames
// from the current working directory.  It first alphabetically sorts
// the input image filenames.  It next wraps the filenames in double
// quotes to shield subsequent unix commands from white spaces.
// After querying the user to enter the first image's ID,
// HOMOGENIZE_IMAGES generates an executable script which copies
// the input raw filenames to homogenized and ordered output
// filenames.

//	 		./homogenize_images

// =======================================================================
// Last updated on 5/4/16; 5/7/16; 5/9/16; 7/19/16
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

   string images_subdir="./";
   string output_subdir = "./homogenized_images/";
   filefunc::dircreate(output_subdir);

   vector<string> image_filenames=
      filefunc::image_files_in_subdir(images_subdir);
   
// On 10/29/13, we discovered the painful way that image_filenames is
// generally NOT sorted alphabetically (or by date).  So we explicitly
// sort the strings within STL vector image_filenames:

   std::sort(image_filenames.begin(),image_filenames.end());

   int image_ID_offset=0;
   cout << "Enter starting image's ID:" << endl;
   cin >> image_ID_offset;

   int istart = 0;
//   int istart = 769;
   int istop = image_filenames.size();

   int homogenized_image_ID = image_ID_offset + istart;

   for (int i=istart; i<istop; i++)
   {
      string curr_image_filename=image_filenames[i];
      cout << "i = " << i << endl;
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
      
      string homogenized_image_filename=output_subdir+
         "image_"+stringfunc::integer_to_string(
            homogenized_image_ID,5)+"."+suffix;
      string unix_cmd="cp \""+curr_image_filename
         +"\" "+homogenized_image_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// As of 4/7/16, we force input images to be in JPG rather than PNG
// format to avoid alpha channel headaches.  Delete original PNG file
// after its JPG replacement has been generated:

      string homogenized_jpg_filename = homogenized_image_filename;
      if(suffix=="png" || suffix =="PNG")
      {
         string prefix = stringfunc::prefix(homogenized_image_filename);
         homogenized_jpg_filename=prefix+".jpg";
         string unix_cmd = "convert "+homogenized_image_filename+" "
            +homogenized_jpg_filename;
         sysfunc::unix_command(unix_cmd);
         if(!filefunc::deletefile(homogenized_image_filename))
         {
            cout << "Couldn't delete " << homogenized_image_filename << endl;
            outputfunc::enter_continue_char();
         }
      }

// Force all JPEG files to end with a ".jpg" suffix:

      else if (suffix=="JPG" || suffix=="jpeg")
      {
         string prefix = stringfunc::prefix(homogenized_image_filename);
         homogenized_jpg_filename=prefix+".jpg";
         string unix_cmd = "mv "+homogenized_image_filename+" "+
            homogenized_jpg_filename;
         sysfunc::unix_command(unix_cmd);
      }

      texture_rectangle *tr_ptr = new texture_rectangle(
         homogenized_jpg_filename,NULL);
      int n_channels = tr_ptr->getNchannels();
      if(n_channels != 3)
      {
         filefunc::deletefile(homogenized_jpg_filename);
      }
      else
      {
         homogenized_image_ID++;
      }
   }

   string banner="Exported homogenized jpg files to "+output_subdir;
   outputfunc::write_big_banner(banner);
}
