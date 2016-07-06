// ==========================================================================
// Program RESIZE_CHIPS needs to be executed from within some
// subdirectory containing a set of JPG image chips.  It generates
// subsampled versions of each input image whose maximum width and
// height do not exceed 32 x 32.  The output resized image chips are
// exported to ./resized_chips.

//			       resize_chips

// ==========================================================================
// Last updated on 7/27/12; 7/28/12; 6/21/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

   string imagery_subdir="./";
   string resized_imagery_subdir="./resized_image_chips/";
   filefunc::dircreate(resized_imagery_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("jpg");
   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,imagery_subdir);

   
   int max_xdim=32;
   int max_ydim=32;
   for (int i=0; i<image_filenames.size(); i++)
   {
      cout << "Downsizing image "+stringfunc::number_to_string(i)+" of "+
         stringfunc::number_to_string(image_filenames.size()) << endl;
      
      string downsized_image_filename=
         resized_imagery_subdir+image_filenames[i];
      videofunc::downsize_image(
         image_filenames[i],max_xdim,max_ydim,downsized_image_filename);
   }

   string banner="Exported "+stringfunc::number_to_string(
      image_filenames.size())+" downsized images "+resized_imagery_subdir;
   outputfunc::write_big_banner(banner);
} 

