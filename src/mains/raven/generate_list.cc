// ==========================================================================
// Program GENERATELIST is a little utility which we wrote in order to
// hard code an initial guess for a video camera's focal length (measured in
// pixels) into a list.txt file read by Noah's Bundler codes.  

// Bundler needs some estimate for the focal parameter.  And for video
// frames, focal information is contained within input frames' exif
// metadata tags.  So Noah told us in Dec 2010 that we need to
// recreate the list.txt file with focal parameter information
// included.  We then need to set the IMAGE_LIST environment variable
// to point towards list.txt.  In bash, chant	 

// 			export IMAGE_LIST=list.txt

// Then run Noah's BundlerVocab script on LLGrid.

//				generate_list

// ==========================================================================
// Last updated on 9/8/11; 1/20/12; 1/21/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/mathfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string list_filename="list.txt";
   ofstream outstream;
   filefunc::openfile(list_filename,outstream);

// Best estimate for cropped Raven front camera's focal parameter as
// of 1/21/12:

   double f=-1.86; 

   int n_horiz_pixels=647;
   int n_vert_pixels=480;
   cout << "f = " << f << endl;
   cout << "n_horiz_pixels = " << n_horiz_pixels << endl;
   cout << "n_vert_pixels = " << n_vert_pixels << endl;

   double f_n_horiz_pixels=fabs(f)*n_horiz_pixels;
   double f_n_vert_pixels=fabs(f)*n_vert_pixels;
   cout << "f*n_horiz_pixels = " << f_n_horiz_pixels << endl;
   cout << "f*n_vert_pixels = " << f_n_vert_pixels << endl;
   cout << endl;

   string jpgs_subdir;
   cout << "Enter full path to subdirectory holding input JPG files:" << endl;
   cin >> jpgs_subdir;
   filefunc::add_trailing_dir_slash(jpgs_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("JPG");
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,jpgs_subdir);

// Critical note:  f_noah in bundler = f_n_vert_pixels !

   for (int n=0; n<jpg_filenames.size(); n ++)
   {
      string curr_filename="images/"+filefunc::getbasename(jpg_filenames[n]);
      outstream << curr_filename
                << " 0 "
                << f_n_vert_pixels
                << endl;
   }
   filefunc::closefile(list_filename,outstream);

   string banner="Wrote images and estimated value for f*Npy to "+
      list_filename;
   outputfunc::write_banner(banner);
}
