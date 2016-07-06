// ========================================================================
// Program REGULARIZE_IMAGE_NAMES scans through a specified
// subdirectory for jpeg files whose names are of the form
// "basename_n.jpg" where "basename" is some string and "n" is some
// integer < 100000.  REGULARIZE_IMAGE_NAMES adds leading zeros to the
// integer label so that the new number has precisely 5 digits:

// 		i.e. basename_1.jpg --->  basename_00001.jpg

// Run this program by chanting 

//    ~/programs/c++/svn/projects/src/mains/utility/regularize_image_names  

// ========================================================================
// Last updated on 10/5/12; 12/3/12
// ========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   string imagedir="./";
   cout << "Enter subdirectory containing all images:"<< endl;
   cin >> imagedir;
   filefunc::add_trailing_dir_slash(imagedir);

   string basefilename;
   cout << "Enter base filename:" << endl;
   cout << "  e.g. Day2_Rover_Vid_img_ " << endl;
   cin >> basefilename;

   int start_image=1;
   int stop_image=10000;
   int n_digits=5;
   string image_suffix="jpg";
   for (int i=start_image; i<=stop_image; i++)
   {
      string input_image_filename=imagedir+basefilename
         +stringfunc::number_to_string(i)+"."+image_suffix;
      string output_image_filename=imagedir+basefilename
         +stringfunc::integer_to_string(i,n_digits)+"."+image_suffix;

      string unix_cmd="mv "+input_image_filename+" "+output_image_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }

}
