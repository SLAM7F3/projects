// ==========================================================================
// Program DOWNSIZE_ORTHOIMAGES imports a set of orthoimages which may
// have very large pixel dimensions.  It exports a downsized set of
// orthoimages whose maximum horizontal and vertical pixel dimensions do not
// exceed 4000.

// We wrote this utility in order to ensure reasonably-sized purple-shaded
// orthoimages are sent to tagger for Ninja markup.
// ==========================================================================
// Last updated on 11/12/15; 12/4/15
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
#include "time/timefuncs.h"
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

//   string orthoimages_subdir = "./ortho_images/";
//   string orthoimages_subdir = "/data/peter_stuff/imagery/ortho_logos/13_det1308_Nov10/";
   string orthoimages_subdir = "/data/peter_stuff/imagery/ortho_logos/13_det1310_Dec4_5K_trial2/";
   string downsized_subdir = orthoimages_subdir+"downsized/";
   filefunc::dircreate(downsized_subdir);
   bool search_all_children_dirs_flag = true;
   vector<string> input_image_filenames = filefunc::image_files_in_subdir(
     orthoimages_subdir, search_all_children_dirs_flag);
   cout << "input_image_filenames.size() = " << input_image_filenames.size() << endl;

   unsigned int max_xdim = 4000;
   unsigned int max_ydim = 4000;
   timefunc::initialize_timeofday_clock();

   unsigned int n_images = input_image_filenames.size();
   for (unsigned int i = 0; i < n_images; i++){
     string image_filename=input_image_filenames.at(i);
     string basename = filefunc::getbasename(image_filename);
     string downsized_filename=downsized_subdir+stringfunc::prefix(basename)+"_downsized.jpg";

     double progress_frac = double(i)/n_images;
     outputfunc::print_elapsed_and_remaining_time(progress_frac);

     unsigned int xdim,ydim, new_xdim, new_ydim;
     imagefunc::get_image_width_height(image_filename,xdim,ydim);
     videofunc::downsize_image(input_image_filenames.at(i), max_xdim, max_ydim, xdim, ydim, downsized_filename,
			       new_xdim, new_ydim);
     cout << "Processing image " << i << " of " << n_images << endl;
     cout << "Exported " << downsized_filename << endl;

     unsigned int downsized_width, downsized_height;
     imagefunc::get_image_width_height(downsized_filename, downsized_width, downsized_height);
     cout << "Orig width = " << xdim << " downsized_width = " << downsized_width 
	  << " Orig height = " << ydim << " downsized_height = " << downsized_height
	  << endl;
   }

}
