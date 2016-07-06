// ==========================================================================
// Program DOWNSIZE_IMAGES reads in a set of jpg files from the
// jpg_subdir specified in this main program.  It creates a
// downsampled/ subdirectory within jpg_subdir.  DOWNSIZE_IMAGES then
// runs the ImageMagick convert -resize 2400x2400 command on all input
// jpg files.  The downsized images have the same size as those output
// by BUNDLER.
// ==========================================================================
// Last updated on 2/19/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/sysfuncs.h"

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
   std::set_new_handler(sysfunc::out_of_memory);

   string jpg_subdir="/media/10EB-267A/lighthawk/camera2_raw_images/";
   string downsampled_jpg_subdir=jpg_subdir+"downsampled/";
   filefunc::dircreate(downsampled_jpg_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,jpg_subdir);

   for (int i=0; i<jpg_filenames.size(); i++)
   {
      string downsized_filename=downsampled_jpg_subdir+
         filefunc::getbasename(jpg_filenames[i]);
//      cout << downsized_filename << endl;
      string unix_cmd=
         "convert -resize 2400x2400 "+jpg_filenames[i]+" "+
         downsized_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }


}
