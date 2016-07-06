// ==========================================================================
// Program EXTRACT_NONTEXT searches for JPG, PNG and GIF files within
// training_data/not_text/ .  Randomly looping over all such non-text
// image files, it randomly chooses a chip_width x chip_height window
// within some image.
// If the image entropy for the random window exceeds a minimal
// threshold, EXTRACT_NONTEXT exports it to a file with name of the
// form nontext_123456.jpg. 
// ==========================================================================
// Last updated on 6/25/12; 8/23/12; 6/1/14; 6/13/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   int chip_width = 40;
   int chip_height = 40;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("gif");
   string nontext_subdir="/home/pcho/non_text/";
   bool search_all_children_dirs_flag=true;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,nontext_subdir,search_all_children_dirs_flag);
   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   string extracted_nontext_images_subdir="./training_data/nontext_patches/";
   filefunc::dircreate(extracted_nontext_images_subdir);

   nrfunc::init_time_based_seed();

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   unsigned int width,height;
   int counter_start=0;
//   int counter_start=54990;
   int counter=counter_start;
   int n_extracted_windows=105000;
   while (counter < n_extracted_windows)
   {
      int i=nrfunc::ran1()*image_filenames.size();
      imagefunc::get_image_width_height(
         image_filenames[i],width,height);

      string output_filename=extracted_nontext_images_subdir+
         "nontext_"+stringfunc::integer_to_string(counter,6)+".jpg";

      int px=nrfunc::ran1()*(width-chip_width);
      int py=nrfunc::ran1()*(height-chip_height);
      imagefunc::crop_image(
         image_filenames[i],output_filename,chip_width,chip_height,px,py);

      double S=-1;

// As of 6/3/12, we've seen some nonnegligible number of times when
// *texture_rectangle_ptr is unable to open the current
// output_filename.  If the following import_photo_from_file() call
// fails, we effectively skip this current iteration and try again...

      if (texture_rectangle_ptr->import_photo_from_file(output_filename))
      {
         bool filter_intensities_flag=false;
         bool greyscale_flag=false;
         S=texture_rectangle_ptr->compute_image_entropy(
            filter_intensities_flag,greyscale_flag);
      }
      
      const double min_S=0.25;
      if (S > min_S)
      {
         counter++;
         cout << "counter = " << counter << " S = " << S << endl;
      }
   } // loop over index counter labeling all extracted nontext regions
   cout << endl;

   delete texture_rectangle_ptr;

}

