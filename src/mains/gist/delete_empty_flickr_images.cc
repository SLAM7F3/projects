// ==========================================================================
// Program DELETE_EMPTY_FLICKR_IMAGES scans through a specified
// subdirectory containing new, raw images semi-automatically
// downloaded from flickr.  In April 2013, we empirically found that
// several downloaded flickr images are essentially content-free.
// Such empty flickr images all seem to have byte-size = 9218.  So
// this program deletes any flickr image with this particular byte
// count.


//		       ./delete_empty_flickr_images

// ==========================================================================
// Last updated on 4/14/13; 4/15/13
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   timefunc::initialize_timeofday_clock();

   const int empty_flickr_image_bytesize=9218;

   string lacie_subdir="/media/LACIE_SHARE/";
//   string lacie_subdir="/media/LACIE_SHARE_/";
   string flickr_images_subdir=
      lacie_subdir+"gist/all_images/flickr/random_examples/";
//   string flickr_images_subdir=lacie_subdir+"gist_images/Random_pictures_2/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      flickr_images_subdir);
   for (int i=0; i<image_filenames.size(); i++)
   {
      int filesize=filefunc::size_of_file_in_bytes(image_filenames[i]);
      if (filesize != empty_flickr_image_bytesize) continue;

      cout << "image_filename = " << image_filenames[i] << endl;
      filefunc::deletefile(image_filenames[i]);
//      outputfunc::enter_continue_char();

   } // loop over index i labeling image filenames


}

