// ==========================================================================
// Program RESIZE_BACKGROUNDS imports all image files from a
// specified folder.  It retrieves each image's width and height in
// pixels.  If the width or height exceed max_x[y]dim, the image is
// downsized.  If either the pixel width or height is less than
// composite_tile_size, the image is moved into a "small_pics" subdir
// of the input folder.  

// We wrote this little utility in order to ensure all background
// internet image inputs to program to COMPOSITE_STRING_TILES are
// sufficiently large [but not too large] for semantic segmentation
// purposes.
// ==========================================================================
// Last updated on 3/15/16; 4/6/16; 4/8/16; 4/9/16
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

   string backgrounds_subdir = "./images/new_internet/backgrounds8/";
   cout << "backgrounds_subdir = " << backgrounds_subdir << endl;

   string small_backgrounds_subdir = backgrounds_subdir+"small_pics/";
   filefunc::dircreate(small_backgrounds_subdir);
   
   vector<string> background_filenames=filefunc::image_files_in_subdir(
      backgrounds_subdir);
   int n_backgrounds = background_filenames.size();

   unsigned int individual_tile_size = 321;
   unsigned int n_individual_tiles = 3;
   unsigned int composite_tile_size = n_individual_tiles*individual_tile_size;
   unsigned int max_xdim = 3000;
   unsigned int max_ydim = 3000;

// ---------------------------------------------------------------
// Main loop over all tiles starts here:

   int n_small_pics = 0;
   int n_large_pics = 0;
   for(int t = 0; t < n_backgrounds; t++)
   {
      cout << "t = " << t << " of " << n_backgrounds << endl;
      string curr_background_filename=background_filenames[t];

      unsigned int width, height;
      imagefunc::get_image_width_height(
         curr_background_filename, width, height);

      if(width > max_xdim || height > max_ydim)
      {
         cout << "Downsizing image_filename = " << curr_background_filename 
              << endl;
         videofunc::downsize_image(
            curr_background_filename, max_xdim, max_ydim);
         imagefunc::get_image_width_height(
            curr_background_filename, width, height);
         n_large_pics++;
      }

      if(width < composite_tile_size || height < composite_tile_size)
      {
         string unix_cmd="mv '"+curr_background_filename+"' "
            +small_backgrounds_subdir;
         sysfunc::unix_command(unix_cmd);
         n_small_pics++;
      }
   }

   cout << "Downsized  " << n_large_pics << " large images" << endl;   
   cout << "Moved " << n_small_pics << " small images into " 
        << small_backgrounds_subdir << endl;
} 
