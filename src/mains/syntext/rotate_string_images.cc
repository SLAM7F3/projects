// ==========================================================================
// Program ROTATE_STRING_IMAGES imports a set of synthetic string
// images and masks generated via program CREATE_STRING_IMAGES.  For
// each image, it instantiates a virtual camera whose horizontal FOV
// and aspect ratio are random gaussian variables.  The string image
// and mask are also rotated in 3 dimensions according to random az,
// el and roll gaussian variables.  After perspectively projecting the
// string image and mask into the virtual camera, ImageMagick is used
// to generate and export their 2D imageplane renderings.  

//                    ./rotate_string_images

// ==========================================================================
// Last updated on 3/10/16; 3/13/16; 3/15/16; 3/16/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "text/textfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

   string synthetic_subdir;
   synthetic_subdir="./training_data/mini_synthetic_words/";
//   synthetic_subdir="./training_data/synthetic_words/";
   string zero_subdir = synthetic_subdir+"00000/";
   string masks_subdir = zero_subdir + "masks/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      zero_subdir);
   vector<string> mask_filenames=filefunc::image_files_in_subdir(
      masks_subdir);

   cout << "image_filenames.size() = " << image_filenames.size() 
        << endl;
   cout << "mask_filenames.size() = " << mask_filenames.size() 
        << endl;
   if(image_filenames.size() != mask_filenames.size())
   {
      exit(-1);
   }

   string rotated_images_subdir=synthetic_subdir+"rotated_images/";
   filefunc::dircreate(rotated_images_subdir);
   string rotated_masks_subdir=synthetic_subdir+"rotated_masks/";
   filefunc::dircreate(rotated_masks_subdir);

   double max_pixel_width = 3 * 321;

   cout << "Total number of string images which can be processed = " 
        << image_filenames.size() << endl;
   unsigned int i_start=0;
   cout << "Enter index for starting string image to process:" << endl;
   cin >> i_start;

   unsigned int i_stop = image_filenames.size();
   cout << "Enter index for stopping string to process:" << endl;
   cin >> i_stop;

   for(unsigned int i = i_start; i < i_stop; i++)
   {
      if ((i-i_start)%50 == 0)
      {
         cout << "Processing i = " << i << " i_start = " << i_start 
              << " i_stop = " << i_stop << endl;
         double progress_frac = double(i - i_start)/(i_stop-i_start);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      bool rot_successful_flag = textfunc::rotate_image_and_mask(
         image_filenames[i], mask_filenames[i],
         rotated_images_subdir,rotated_masks_subdir, max_pixel_width);
      

/*


      string rotated_image_filename=rotated_images_subdir+
         filefunc::getbasename(image_filenames[i]);
      string image_basename=filefunc::getbasename(rotated_image_filename);

      string separator_chars="_.";
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         image_basename,separator_chars);
      string curr_index = substrings[1];

// On 3/15/16, we empirically observed occasional pairs such as 

// string_007400_just_foreground.png
// string_007400.png

// In this case, we skip the first one since it leads to association
// problems with mask files!

      if(i < i_stop-1)
      {
         string next_image_basename=filefunc::getbasename(
            image_filenames[i+1]);
         vector<string> next_substrings = 
            stringfunc::decompose_string_into_substrings(
               next_image_basename,separator_chars);
         string next_index = next_substrings[1];
         if(next_index == curr_index) continue;
      }

      string mask_basename="mask_"+curr_index+".png";
      string mask_filename=masks_subdir+mask_basename;
      string rotated_mask_filename=rotated_masks_subdir+mask_basename;

//      cout << "mask_basename = " << mask_basename << endl;
//      cout << "mask_filename = " << mask_filename << endl;
//      cout << "rot_mask_filename = " << rotated_mask_filename << endl;
      
      if(!filefunc::fileexist(mask_filename)) continue;

//      string rotated_mask_filename=rotated_masks_subdir+
//         filefunc::getbasename(mask_filenames[i]);

// For a nontrivial fraction of input images, do NOT apply any rotation:

      double non_rotate_frac = 0.40;
      if(nrfunc::ran1() < non_rotate_frac)
      {
         rotated_image_filename=stringfunc::prefix(rotated_image_filename)+
            "_NoRot.png";
         string unix_cmd="cp "+image_filenames[i]+" "+rotated_image_filename;
         sysfunc::unix_command(unix_cmd);
         rotated_mask_filename=stringfunc::prefix(rotated_mask_filename)+
            "_NoRot.png";
         unix_cmd="cp "+mask_filenames[i]+" "+rotated_mask_filename;
         sysfunc::unix_command(unix_cmd);
      }
      else
      {
         double FOV_u = (45 + 15 * nrfunc::gasdev());
         FOV_u = basic_math::max(FOV_u, 10.0);
         FOV_u = basic_math::min(FOV_u, 80.0);
         FOV_u *= PI/180;

         double aspect_ratio = 1.333 + 0.2 * nrfunc::gasdev();
         aspect_ratio = basic_math::max(0.5, aspect_ratio);
         aspect_ratio = basic_math::min(2.0, aspect_ratio);

         double img_az = 0 + 50 * nrfunc::gasdev();
         img_az = basic_math::max(img_az, -80.0);
         img_az = basic_math::min(img_az, 80.0);
         img_az *= PI/180;

         double img_el = 0 + 15 * nrfunc::gasdev();
         img_el = basic_math::max(img_el, -30.0);
         img_el = basic_math::min(img_el, 30.0);
         img_el *= PI/180;

         double img_roll = 0 + 3 * nrfunc::gasdev();
         img_roll = basic_math::max(img_roll, -15.0);
         img_roll = basic_math::min(img_roll, 15.0);
         img_roll *= PI/180;

         string background_color="none";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            image_filenames[i], background_color, rotated_image_filename);

         background_color="black";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            mask_filenames[i], background_color, rotated_mask_filename);

      } // non-rotation conditional

      bool rot_img_exists = filefunc::fileexist(rotated_image_filename);
      bool rot_mask_exists= filefunc::fileexist(rotated_mask_filename);
      if(rot_img_exists && rot_mask_exists)
      {
      }
      else
      {
         cout << "Rotated image = " << rotated_image_filename
              << " rot_img_exists = " << rot_img_exists << endl;
         cout << "Rotated mask = " << rotated_mask_filename
              << " rot_mask_exists = " << rot_mask_exists << endl;
         filefunc::deletefile(rotated_image_filename);
         filefunc::deletefile(rotated_mask_filename);
         outputfunc::enter_continue_char();
      }
*/

   } // loop over index i labeling image filenames
   
}

