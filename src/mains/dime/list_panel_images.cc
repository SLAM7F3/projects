// ========================================================================
// Program LIST_PANEL_IMAGES is a little utility program which we
// wrote to generate a list of panel images that serves as input to
// ASIFTVID.

//			     list_panel_images

// ========================================================================
// Last updated on 3/18/13
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
   string bundler_IO_subdir="./bundler/DIME/Feb2013_DeerIsland/";
   string subsampled_panels_subdir=bundler_IO_subdir+
      "images/panels/subsampled_panels/";

   string substring=".png";
   vector<string> image_filenames=filefunc::files_in_subdir_matching_substring(
      subsampled_panels_subdir,substring);
   cout << "Number of png images in " << subsampled_panels_subdir << " = " 
        << image_filenames.size() << endl;

   int n_start,n_stop,n_skip;
   cout << "Enter starting image number:" << endl;
   cin >> n_start;
   cout << "Enter stopping image number:" << endl;
   cin >> n_stop;

   string scriptfilename="panel_images";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;

   string suffix=".png";
   for (int i=n_start; i<=n_stop; i++)
   {
      string imagenumber_str=stringfunc::integer_to_string(i,5);
      string filename=subsampled_panels_subdir+
         "subsampled_wisp_p2_res0_"+imagenumber_str+suffix;
      string command="--newpass "+filename;
      scriptstream << command << endl;
   }

   filefunc::closefile(scriptfilename,scriptstream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);

   string banner="Exported panel images to "+scriptfilename;
   outputfunc::write_big_banner(banner);
}
