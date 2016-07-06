// ==========================================================================
// Program DETECT_CORRUPTED searches for a grey-colored vertical
// stripe located on the RHS of PointGrey images after they have had
// white borders removed.  
// ==========================================================================
// Last updated on 10/30/12; 10/31/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "classification/signrecogfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string images_subdir=
      "/media/66368D22368CF3F9/TOC12/images/final_signs/quantized_colors/vid_21/raw_images/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   int n_corrupted_images=0;
   int n_images=image_filenames.size();
   for (int i=0; i<n_images; i++)
   {
      string image_filename=image_filenames[i];
//      cout << "i = " << i << " image = " << image_filename << endl;

      if (signrecogfunc::detect_corrupted_PointGrey_image(
         image_filename,texture_rectangle_ptr))
      {
         cout << "i = " << i 
              << " image_filename = " << filefunc::getbasename(image_filename)
              << endl;
         n_corrupted_images++;
      }
      
   } // loop over index i labeling image filenames

   double corrupted_frac=double(n_corrupted_images)/n_images;
   cout << "Corrupted image fraction = " << corrupted_frac << endl;

   delete texture_rectangle_ptr;
}
