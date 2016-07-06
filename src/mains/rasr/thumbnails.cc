// ========================================================================
// Program THUMBNAILS reads in a set of panorama panel images and
// generates subsampled versions of them.

//  				thumbnails 

// ========================================================================
// Last updated on 2/21/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string image_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/rasr_aud_dset2/images/panels/";
//      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/rasr_auditorium/images/panels/";
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   
   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,image_subdir);

   for (int i=0; i<image_filenames.size(); i++)
   {
      string thumbnail_filename=videofunc::get_thumbnail_filename(
         image_filenames[i]);
      cout << thumbnail_filename << endl;

      string convert_cmd="convert -resize 192x192 "+image_filenames[i]+" "+
         thumbnail_filename;
//      string convert_cmd="convert -resize 12x12 "+image_filenames[i]+" "+
//         thumbnail_filename;
      sysfunc::unix_command(convert_cmd);
   }

}
