// ==========================================================================
// Program COMPUTE_COLOR_HISTOGRAMS loops over all images within an
// images directory.  It exports their color histograms to text files
// within a color_histograms subdirectory of the images directory.

// 			 ./compute_color_histograms

// ==========================================================================
// Last updated on 10/2/13; 10/5/13; 10/6/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "general/outputfuncs.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::ofstream;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   string liberalized_color="";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(liberalized_color);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

//   string bundler_IO_subdir="./bundler/BostonBombing/GoodMorningAmerica/";
//   string bundler_IO_subdir="./bundler/Korea/KCTV/";
//   string bundler_IO_subdir="./bundler/NewsWrap/";
//   string images_subdir=bundler_IO_subdir+"images/";
   string JAV_subdir=
      "/data/video/JAV/NewsWraps/early_Sep_2013/";
   string images_subdir=JAV_subdir+"jpg_frames/";
//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

//   string images_subdir="/data/ImageEngine/tidmarsh/";
//   string thumbnails_subdir=images_subdir+"thumbnails/";
//   vector<string> image_filenames=filefunc::image_files_in_subdir(
//      thumbnails_subdir);

//   string output_subdir=images_subdir+"color_histograms/";
   string output_subdir=JAV_subdir+"color_histograms/";
   filefunc::dircreate(output_subdir);

   int i_start=0;
//   int i_stop=10;
   int i_stop=image_filenames.size();

   ofstream outstream;
   for (int i=i_start; i<i_stop; i++)
   {
      string image_filename=image_filenames[i];
      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);
//      cout << "i = " << i 
//           << " image_filename = " << image_filename << endl;
      if (prefix.substr(0,10)=="thumbnail_")
      {
         prefix=prefix.substr(10,prefix.size()-10);
      }
      string color_histogram_filename=output_subdir+prefix+".color_hist";
         
// Check whether color histogram for current image already exists.  If
// so, move on...

         if (filefunc::fileexist(color_histogram_filename)) 
         {
            cout << "Color histogram file already exists" << endl;
         }
         else
         {
            descriptorfunc::compute_color_histogram(
               image_filename,color_histogram_filename,
               texture_rectangle_ptr,RGB_analyzer_ptr);
         }
   } // loop over index i labeling image filenames

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
}

