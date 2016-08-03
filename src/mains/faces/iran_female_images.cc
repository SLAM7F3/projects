// ====================================================================
// Program IRAN_FEMALE_IMAGES

// 			 ./iran_female_images
// ====================================================================
// Last updated on 8/3/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string iran_female_faces_subdir = faces_rootdir + "iran_female_faces/";
   string output_faces_subdir="resized_iran_female_faces/";
   filefunc::dircreate(output_faces_subdir);

   vector<string> image_filenames = filefunc::image_files_in_subdir(
      iran_female_faces_subdir);

   int max_xdim = 106;
   int max_ydim = 106;
   string gender = "female";
   Magick::Image IM_image;
   for(int unsigned i = 0; i < image_filenames.size(); i++)
   {
      string output_filename = output_faces_subdir+gender+"_iran_"+
         stringfunc::integer_to_string(i,5)+".jpg";
      videofunc::import_IM_image(image_filenames[i], IM_image);
      videofunc::downsize_image(
         image_filenames[i], max_xdim, max_ydim, output_filename);
   } // loop over index i labeing image filenames
}

