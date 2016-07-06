// ========================================================================
// Program GENERATE_THUMBNAILS reads in a set of panorama panel images
// and generates subsampled versions of them.

// To run this program, chant  

// 	~/programs/c++/svn/projects/src/mains/hmi/generate_thumbnails

// ========================================================================
// Last updated on 2/21/11; 7/5/11; 7/13/11
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

   sysfunc::clearscreen();

   string images_subdir="./D7/";
   cout << "Enter full path for directory containing initial raw D7 jpeg images:" 
        << endl;
   cin >> images_subdir;
   filefunc::add_trailing_dir_slash(images_subdir);

   string cropped_images_subdir=images_subdir+"cropped/";
   filefunc::dircreate(cropped_images_subdir);

   string panels_subdir=cropped_images_subdir+"panels/";
   filefunc::dircreate(panels_subdir);

   string thumbnails_subdir=panels_subdir+"thumbnails/";
   filefunc::dircreate(thumbnails_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   vector<string> image_panel_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,panels_subdir);

   for (unsigned int i=0; i<image_panel_filenames.size(); i++)
   {
      string thumbnail_filename=videofunc::get_thumbnail_filename(
         image_panel_filenames[i]);
      cout << thumbnail_filename << endl;

      string convert_cmd="convert -resize 232x313 "
         +image_panel_filenames[i]+" "+thumbnail_filename;
      sysfunc::unix_command(convert_cmd);
   }

   string banner="Thumbnail images written to "+thumbnails_subdir;
   outputfunc::write_big_banner(banner);


}
