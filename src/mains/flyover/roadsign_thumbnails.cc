// ==========================================================================
// Program ROADSIGN_THUMBNAILS
// ==========================================================================
// Last updated on 4/2/15; 4/7/15
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "image/myimage.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/rotation.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================

   string roadsigns_subdir = "./roadsigns/";
   string thumbnails_subdir = roadsigns_subdir+"thumbnails/";
   vector<string> input_image_filenames = filefunc::image_files_in_subdir(
     roadsigns_subdir);

   for (unsigned int i = 0; i < input_image_filenames.size(); i++){
     string basename = filefunc::getbasename(input_image_filenames.at(i));
     string thumbnail_filename=thumbnails_subdir+stringfunc::prefix(basename)+"_thumbnail.jpg";
     int max_xdim = 32;
     int max_ydim = 32;

     videofunc::downsize_image(input_image_filenames.at(i), max_xdim, max_ydim, thumbnail_filename);
     cout << "Exported " << thumbnail_filename << endl;

     unsigned int thumbnail_width, thumbnail_height;
     imagefunc::get_image_width_height(thumbnail_filename, thumbnail_width, thumbnail_height);
     cout << "thumbnail_width = " << thumbnail_width << " thumbnail_height = " << thumbnail_height
	  << endl;

   }

}
