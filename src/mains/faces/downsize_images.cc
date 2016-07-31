// ====================================================================
// Program DOWNSIZE_IMAGES imports all images from within subdirectory
// ./homogenized_images/.  It downsizes any image within the
// input folder whose horizontal or vertical pixel dimensions exceeds
// max_xdim or max_ydim.  
// ====================================================================
// Last updated on 1/8/16; 1/9/16; 5/7/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <png.h>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;


int main(int argc, char** argv)
{  
   string input_imagesdir = "./resized_adiencefaces/";
//   string input_imagesdir = "./homogenized_images/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_imagesdir);

   cout << "Imported " << image_filenames.size() << " images" << endl;

   unsigned int max_xdim = 106;
   unsigned int max_ydim = 106;
//   unsigned int max_xdim = 2400;
//   unsigned int max_ydim = 2400;
   for(unsigned int i = 0; i < image_filenames.size(); i++)
   {
      string image_filename=image_filenames[i];
      videofunc::downsize_image(
         image_filename,max_xdim, max_ydim, image_filename);
   }
}

