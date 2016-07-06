// ==========================================================================
// Program RESIZE_FACE_IMAGES imports a set of raw images from some
// specified subdirectory.  It first downsizes all images so that they
// do not exceed some reasonable pixel width and height.
// RESIZE_FACE_IMAGES then upsamples the homogenized images so that
// relatively small faces have some possibility for being detected via
// Davis King's HOG detector.
// ==========================================================================
// Last updated on 11/27/13; 11/28/13; 11/30/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

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
   timefunc::initialize_timeofday_clock();      


//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
//   string root_subdir=JAV_subdir;
//   string images_subdir=root_subdir+"jpg_frames/";

//   string root_subdir="./bundler/aleppo_1K/";
//   string images_subdir=root_subdir+"images/";
//   string root_subdir="./bundler/GrandCanyon/";
//   string images_subdir=root_subdir+"images/";

/*
   string root_subdir="/home/cho/Desktop/profile_faces/good/";
//   string images_subdir=root_subdir+"right_facing/";
   string images_subdir=root_subdir+"left_facing/";
*/

   string root_subdir="/home/cho/Downloads/people/standing/";
   string images_subdir=root_subdir+"individuals/";
//   string images_subdir=root_subdir+"multiple_people/";

//   string root_subdir="/home/cho/Downloads/people/more_standing_walking/";
//   string images_subdir=root_subdir;
   
   vector<string> image_filenames=filefunc::image_files_in_subdir(    
      images_subdir);
   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   int mag_factor=2;
//   int mag_factor=3;
//   int mag_factor=4;
//   int mag_factor=5;
//   cout << "Enter image magnification factor:" << endl;
//   cin >> mag_factor;

   string output_subdir=images_subdir+"resized_images/";
   filefunc::dircreate(output_subdir);

   double max_xdim=mag_factor*640;
//   double max_ydim=mag_factor*480;


// FAKE FAKE: 
   double max_ydim=900;

   cout << "max_xdim = " << max_xdim 
        << " max_ydim = " << max_ydim << endl;

   int i_start=0;
   for (int i=i_start; i<image_filenames.size(); i++)
   {
      if (i%50==0)
      {
         double progress_frac=double(i)/image_filenames.size();
         cout << "i = " << i << " progress_frac = " << progress_frac << endl;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      int width,height;
      imagefunc::get_image_width_height(image_filenames[i],width,height);
      double x_scale_factor=max_xdim/width;
      double y_scale_factor=max_ydim/height;
//      double scale_factor=sqrt(x_scale_factor*y_scale_factor);

// FAKE FAKE:  

      double scale_factor=y_scale_factor;

      string basename=filefunc::getbasename(image_filenames[i]);
//      cout << "image_filename = " << image_filenames[i] << endl;

      string resized_image_filename=output_subdir+basename;
      videofunc::resize_image(
         image_filenames[i],width,height,
         scale_factor*width,scale_factor*height,resized_image_filename);
      
   } // loop over index i labeling images
   

   string banner="Exported resized images to "+output_subdir;
   outputfunc::write_big_banner(banner);
}
