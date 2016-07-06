// ========================================================================
// Program FLIP_LEFTFACING_IMAGES imports a set of images which are
// assumed to all be "left facing" (e.g. profiles of people all
// looking towards the left of the image plane).  It calls
// ImageMagick's convert flop command in order to flip the image about
// the center vertical axis.  The resulting output is "right-facing".

//			 ./flip_leftfacing_images

// ========================================================================
// Last updated on 11/29/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();  

//   string left_images_subdir="~/Desktop/profile_faces/good/left/";
//   string flipped_images_subdir=left_images_subdir+"flipped_images/";
   string right_images_subdir="~/Desktop/profile_faces/good/right/";
   string flipped_images_subdir=right_images_subdir+"flipped_images/";

   filefunc::dircreate(flipped_images_subdir);

//   vector<string> image_filenames=filefunc::image_files_in_subdir(
//      left_images_subdir);
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      right_images_subdir);
   
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      cout << i << " " << flush;
      string image_filename=image_filenames[i];
      string basename=filefunc::getbasename(image_filename);
      string flipped_image_filename=flipped_images_subdir+basename;

      string unix_cmd="convert "+image_filename+" -flop "+
         flipped_image_filename;
      sysfunc::unix_command(unix_cmd);
   } // loop over index i labeling left-handed images
   cout << endl;

}
