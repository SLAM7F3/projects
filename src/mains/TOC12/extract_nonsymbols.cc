// ==========================================================================
// Program EXTRACT_NONSYMBOLS searches for JPG, PNG and GIF files
// within ./images/non_signs/all_nonsigns/ .  Randomly looping over all
// such non-symbol image files, it randomly chooses a 32x32 window
// within some image. If the image entropy for the random window
// exceeds a minimal threshold, EXTRACT_NONSYMBOLS exports it to a
// file with name of the form nontext_123456.jpg. 

//				extract_nonsymbols

// ==========================================================================
// Last updated on 8/16/12; 8/17/12; 8/21/12
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

   nrfunc::init_default_seed(-20);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("gif");
   string nontext_subdir="./images/non_signs/all_nonsigns/";
   bool search_all_children_dirs_flag=false;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,nontext_subdir,search_all_children_dirs_flag);
   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   string extracted_nontext_images_subdir=
      nontext_subdir+"non_symbols/";
   filefunc::dircreate(extracted_nontext_images_subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   int width,height;
   int counter=0;
//  int n_extracted_windows=50;
//   int n_extracted_windows=55000;
//   int n_extracted_windows=75000;
   int n_extracted_windows=101000;
   while (counter < n_extracted_windows)
   {
      if (counter%1000==0) 
      {
         cout << endl;
         cout << "Generating non-symbol image " << counter << endl;
         cout << endl;
      }
      int i=nrfunc::ran1()*image_filenames.size();

// We do not want to oversample the synthetic blank TOC12 symbol
// images.  So ignore all but a small fraction of such incoming
// synthetic non-sign images:

      string image_filename=filefunc::getbasename(image_filenames[i]);
      string substr=image_filename.substr(0,9);
      if (substr=="synthetic")
      {
         bool synthetic_blank_sign_flag=true;
         if (nrfunc::ran1() < 0.95) synthetic_blank_sign_flag=false;
         if (!synthetic_blank_sign_flag) continue;
      }

      imagefunc::get_image_width_height(
         image_filenames[i],width,height);

      string output_filename=extracted_nontext_images_subdir+
         "nontext_"+stringfunc::integer_to_string(counter,6)+".jpg";
//      cout << "output_filename = " << output_filename << endl;

      int px=nrfunc::ran1()*(width-32);
      int py=nrfunc::ran1()*(height-32);
      imagefunc::crop_image(
         image_filenames[i],output_filename,32,32,px,py);

/*
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
      
      const double min_S=0.1;
//      const double min_S=0.25;
      if (S > min_S)
      {
         counter++;
         cout << "counter = " << counter 
              << " px = " << px << " py = " << py 
              << " S = " << S << endl;
      }
      else
      {
         cout << "REJECTED image!" << endl;
         filefunc::deletefile(output_filename);
      }
*/

      counter++;
    
   } // loop over index counter labeling all extracted nontext regions
   cout << endl;

   delete texture_rectangle_ptr;

}

