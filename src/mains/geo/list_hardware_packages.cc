// ========================================================================
// Program LIST_HARDWARE_PACKAGES is a little utility program which we
// wrote to generate a list of selected FLIR hardware packages which
// serve as inputs to programs RESTRICTED_ASIFT, TRIANGULATE and
// PARSE_SSBA.

//			    list_hardware_packages

// ========================================================================
// Last updated on 3/7/13; 3/8/13; 4/22/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

   string bundler_IO_subdir="./bundler/GEO/20120105_1402/";
   string packages_subdir=bundler_IO_subdir+"packages/hardware_in/";
   filefunc::dircreate(packages_subdir);
   string image_subdir=bundler_IO_subdir+"images/";
//   string packages_subdir="./bundler/GEO/20120105_1130/packages/hardware_in/";
//   string image_subdir="./bundler/GEO/20120105_1130/images/";

   string substring=".jpg";
   vector<string> image_filenames=filefunc::files_in_subdir_matching_substring(
      image_subdir,substring);
   cout << "Number of jpeg images in " << image_subdir << " = " 
        << image_filenames.size() << endl;

   int n_start,n_stop,n_skip;
   cout << "Enter starting image number:" << endl;
   cin >> n_start;
   cout << "Enter stopping image number:" << endl;
   cin >> n_stop;
   cout << "Enter image number skip:" << endl;
   cin >> n_skip;

   string scriptfilename="raw_hardware_packages";
   string list_filename="list_tmp.txt";
   ofstream scriptstream,liststream;
   filefunc::openfile(scriptfilename,scriptstream);
   filefunc::openfile(list_filename,liststream);

   for (int i=n_start; i<n_stop; i += n_skip)
   {
      string imagenumber_str=stringfunc::integer_to_string(i,4);
      string filename=packages_subdir+"photo_"+imagenumber_str+".pkg";
      string command="--region_filename "+filename;
      scriptstream << command << endl;

      string curr_image_filename="images/"+filefunc::getbasename(
         image_filenames[i]);
      liststream << curr_image_filename << endl;
   }

   filefunc::closefile(scriptfilename,scriptstream);
   filefunc::closefile(list_filename,liststream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);

   string banner="Exported hardware packages to "+scriptfilename;
   outputfunc::write_big_banner(banner);
   
}
