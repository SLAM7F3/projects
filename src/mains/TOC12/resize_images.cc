// ==========================================================================
// Program RESIZE_IMAGES needs to be executed from within some
// subdirectory containg a set of PNG or JPG files.  It generates
// subsampled versions of each input image whose maximum width and
// height do not exceed 640 x 480.  The output resized images are
// exported to ./resized_images.

//			       resize_images

// ==========================================================================
// Last updated on 7/27/12; 7/28/12
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
   string resized_imagery_subdir="./resized_images/";
   filefunc::dircreate(resized_imagery_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("jpg");
   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,imagery_subdir);

   
   int max_xdim=640;
   int max_ydim=480;
   for (unsigned int i=0; i<image_filenames.size(); i++)
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

