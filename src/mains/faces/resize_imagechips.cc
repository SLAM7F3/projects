// ==========================================================================
// Program RESIZE_IMAGECHIPS imports training, validation and testing
// image chips generated by AUGMENT_CHIPS.  Any training [validation,
// testing] image chip which has pixel width or height greater than
// 106 [96] is downsized so that its maximal pixel extent does not
// exceed 106 [96].  RESIZE_IMAGECHIPS then calls ImageMagick and
// superposes the image chips onto black backgrounds which are 106x106
// [96x96] in size.

// Note: We discovered the hard and painful way on 7/22/2016 that
// Caffe's image --> LMDB converter appears to expect JPG [rather than
// PNG] imagery input by default!

// Run program RESIZE_IMAGECHIPS from within the subdirectory holding
// input image chips:

//                          ./resize_imagechips

// ==========================================================================
// Last updated on 8/1/16; 8/2/16; 9/7/16; 9/9/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string images_subdir = "./";
   bool search_all_children_dirs_flag = false;
   //   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir,search_all_children_dirs_flag);
   string resized_chips_subdir=images_subdir+"resized_chips/";
   filefunc::dircreate(resized_chips_subdir);

   char training_char;
   cout << "Enter 't' if input image chips are to be used for training:"
        << endl;
   cin >> training_char;

   int max_xdim = 106;  // Training image chip size 
   int max_ydim = 106;  //   for FACE01 neural net
   if(training_char != 't')
   {
      max_xdim = 96;  // Validation and testing image chip size 
      max_ydim = 96;  //   for FACE01 neural net
   }
   cout << "max_xdim = " << max_xdim << " max_ydim = " << max_ydim
        << endl;

   int n_bad_aspect_ratios = 0;
   int istart=0;
   int istop = image_filenames.size();
   int n_images = istop - istart;
   cout << "n_images within current working subdir = " << n_images << endl;
   
   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_fraction(i,1000,n_images);

      if ((i-istart)%1000 == 0)
      {
         double progress_frac = double(i - istart)/n_images;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

// As of Sep 2016, we upsample image chips so that their new pixel
// sizes precisely equal max_xdim x max_ydim.  But we ignore any input
// image chip whose aspect ratio is either too small or too large:

      unsigned int xdim,ydim;
      imagefunc::get_image_width_height(image_filenames[i],xdim,ydim);
      double aspect_ratio = double(xdim) / double(ydim);

// Calculated aspect ratio for face bboxes:

      const double aspect_ratio_median = 0.7447; 
      const double aspect_ratio_quartile_width = 0.1054;

      double min_aspect_ratio = aspect_ratio_median 
         - 4 * aspect_ratio_quartile_width;
      double max_aspect_ratio = aspect_ratio_median 
         + 4 * aspect_ratio_quartile_width;

      if(aspect_ratio < min_aspect_ratio ||
         aspect_ratio > max_aspect_ratio)
      {
         n_bad_aspect_ratios++;
         continue;
      }

      string basename = filefunc::getbasename(image_filenames[i]);
      string padded_jpg_filename=resized_chips_subdir
         +stringfunc::prefix(basename);
      padded_jpg_filename += "_"+
         stringfunc::number_to_string(max_xdim)+"x"+
         stringfunc::number_to_string(max_ydim)+".jpg";

      videofunc::force_size_image(
         image_filenames[i], max_xdim, max_ydim,
         padded_jpg_filename);

/*
      string unix_cmd="convert -size ";
      unix_cmd += stringfunc::number_to_string(max_xdim)+"x"+
         stringfunc::number_to_string(max_ydim)+" ";
      unix_cmd += " xc:black "+image_filenames[i];
      unix_cmd += " -gravity center -composite ";
      unix_cmd += padded_jpg_filename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
*/

   } // loop over index i 

   double frac_bad_aspect_ratios = double(n_bad_aspect_ratios) / n_images;
   cout << "Fraction of image chips ignored due to bad aspect ratios = " 
        << frac_bad_aspect_ratios << endl;
} 


