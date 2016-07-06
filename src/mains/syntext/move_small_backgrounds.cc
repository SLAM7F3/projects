// ==========================================================================
// Program MOVE_SMALL_BACKGROUNDS imports all image files from a
// specified folder.  It retrieves each image's width and height in
// pixels.  If either the pixel width or height is less than
// composite_tile_size, the image is moved into a "small_pics" subdir
// of the input folder.

// We wrote this little utility in order to ensure all background
// internet image inputs to program to COMPOSITE_STRING_TILES are
// sufficiently large for semantic segmentation purposes.
// ==========================================================================
// Last updated on 3/15/16; 4/6/16
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/sysfuncs.h"
#include "text/textfuncs.h"
#include "time/timefuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string backgrounds_subdir = "./images/new_internet/backgrounds4/";
   cout << "backgrounds_subdir = " << backgrounds_subdir << endl;

//   string backgrounds_subdir = "./images/internet/non_symbols/";
//    string backgrounds_subdir = "./images/internet/backgrounds/";
   string small_backgrounds_subdir = backgrounds_subdir+"small_pics/";
   filefunc::dircreate(small_backgrounds_subdir);
   
   vector<string> background_filenames=filefunc::image_files_in_subdir(
      backgrounds_subdir);
   int n_backgrounds = background_filenames.size();

   unsigned int individual_tile_size = 321;
   unsigned int n_individual_tiles = 3;
   unsigned int composite_tile_size = n_individual_tiles*individual_tile_size;

// ---------------------------------------------------------------
// Main loop over all tiles starts here:

   int n_small_pics = 0;
   for(int t = 0; t < n_backgrounds; t++)
   {
      cout << "t = " << t << " of " << n_backgrounds << endl;
      string curr_background_filename=background_filenames[t];

      unsigned int width, height;
      imagefunc::get_image_width_height(
         curr_background_filename, width, height);
      if(width < composite_tile_size || height < composite_tile_size)
      {
         string unix_cmd="mv '"+curr_background_filename+"' "
            +small_backgrounds_subdir;
//         cout << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
         n_small_pics++;
      }
   }
   
   cout << "Moved " << n_small_pics << " small images into " 
        << small_backgrounds_subdir << endl;
} 
