// ==========================================================================
// Program RESIZE_IMAGES imports a set of raw images from some
// specified subdirectory.  (It's preferable if
// mains/imagesearch/LINK_RENAME_IMAGES has been run before this program
// is called!)  It first downsizes all images so that they do not
// exceed some reasonable pixel width and height. RESIZE_IMAGES then
// upsamples the homogenized images so that relatively small 2D
// objects have some possibility for being detected via Davis King's
// HOG template training code. RESIZE_IMAGES also checks for corrupted
// input image files.  If found, such corrupted images are simply
// ignored.

//				./resize_images

// ==========================================================================
// Last updated on 11/28/13; 11/30/13; 12/1/13; 6/7/14
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

//   bool train_HOG_template_flag=true;
   bool train_HOG_template_flag=false;
   cout << "train_HOG_template_flag = " << train_HOG_template_flag << endl;
   outputfunc::enter_continue_char();

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
//   string root_subdir=JAV_subdir;
//   string images_subdir=root_subdir+"jpg_frames/";

//   string root_subdir="./bundler/aleppo_1K/";
//   string images_subdir=root_subdir+"images/";
//   string root_subdir="./bundler/GrandCanyon/";
//   string images_subdir=root_subdir+"images/";

//   string root_subdir="/home/cho/Desktop/profile_faces/";
//   string images_subdir=root_subdir+"good/right_facing/";
//   string images_subdir=root_subdir+"good/left_facing/";
//   string images_subdir=root_subdir+"new_raw/homogenized_imagenames/";
//   string images_subdir=root_subdir+"difficult/";

   string root_subdir="/home/cho/Downloads/people/";
//   string images_subdir=root_subdir+"standing/individuals/";
//   string images_subdir=root_subdir+"standing/multiple_people/";
//   string images_subdir=root_subdir+"more_standing_walking/homogenized/";
   string images_subdir=root_subdir+"standing/difficult/";

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
   double max_ydim=mag_factor*480;

   if (train_HOG_template_flag) max_ydim=900;

   cout << "max_xdim = " << max_xdim 
        << " max_ydim = " << max_ydim << endl;

   unsigned int i_start=0;
   for (unsigned int i=i_start; i<image_filenames.size(); i++)
   {
//      if (i%50==0)
      {
         double progress_frac=double(i)/image_filenames.size();
         cout << "i = " << i << " progress_frac = " << progress_frac << endl;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

// Search for any white spaces within input image filenames.  If
// found, replace them with underscores:

      string curr_image_filename=image_filenames[i];
      cout << "curr_image_filename = " << curr_image_filename << endl;
      string dirname=filefunc::getdirname(curr_image_filename);
      string basename=filefunc::getbasename(curr_image_filename);
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename);
      string new_basename="";
      for (unsigned int s=0; s<substrings.size()-1; s++)
      {
         new_basename += substrings[s]+"_";
      }
      new_basename += substrings.back();
      curr_image_filename=dirname+new_basename;
      if (curr_image_filename != image_filenames[i])
      {
//         cout << "i = " << i << " image_filenames[i] = "
//              << image_filenames[i] << endl;
//         cout << "curr_image_filename = " << curr_image_filename << endl;
         string unix_cmd="mv \""+image_filenames[i]+"\" "+
            curr_image_filename;
         sysfunc::unix_command(unix_cmd);
//         outputfunc::enter_continue_char();
      }

// On 5/7/12, we discovered the hard way that input JPG files
// (e.g. from flickr) can be corrupted.  So we need to explicitly
// check for bad input images:

      if (!imagefunc::valid_image_file(curr_image_filename))
      {
         cout << "Skipping corrupted image " << curr_image_filename
              << endl;
         continue;
      }

      unsigned int width,height;
      imagefunc::get_image_width_height(curr_image_filename,width,height);
      double x_scale_factor=max_xdim/width;
      double y_scale_factor=max_ydim/height;
      double scale_factor=sqrt(x_scale_factor*y_scale_factor);

      if (train_HOG_template_flag) scale_factor=y_scale_factor;

      string resized_image_filename=output_subdir+basename;
      videofunc::resize_image(
         curr_image_filename,width,height,
         scale_factor*width,scale_factor*height,resized_image_filename);
      
   } // loop over index i labeling images

   string banner="Exported resized images to "+output_subdir;
   outputfunc::write_big_banner(banner);
}
