// ==========================================================================
// Program REASONABLE_CHIPS imports a set of manually extracted image
// chips which correspond to text characters.  It discards any chips
// whose pixel sizes are too small.  It also ignores any chip whose RGB 
// standard deviation is too low.  The surviving image chips are
// copied to a subdirectory of mains/imagetext/images/ for subsequent
// processing.

//			./reasonable_chips

// ==========================================================================
// Last updated on 4/27/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main (int argc, char* argv[])
{
   string input_digits_subdir=
      "/home/pcho/sputnik/pwin/app/lab/lab_pcho/cropped_images/saved_patches/";
   string imagetext_subdir=
      "/home/pcho/programs/c++/svn/projects/src/mains/imagetext/";
   string output_digits_subdir=imagetext_subdir+
      "images/HouseNumbers/van/digits/";

   const int min_width = 8;
//   const int min_width = 15;
   const double min_sigma = 3;
//   const double min_sigma = 15;

   int d_start=0;
   int d_stop=9;
   cout << "Enter starting digit:" << endl;
   cin >> d_start;
   cout << "Enter stopping digit:" << endl;
   cin >> d_stop;
   for (int d=d_start; d<=d_stop; d++)
   {
      string curr_input_digit_subdir=
         input_digits_subdir+stringfunc::number_to_string(d)+"/";
      vector<string> image_filenames = filefunc::image_files_in_subdir(
	curr_input_digit_subdir);

      string curr_output_digit_subdir=
         output_digits_subdir+stringfunc::number_to_string(d)+"/";
      filefunc::dircreate(curr_output_digit_subdir);

      int n_exported_chips = 0;
      for (unsigned int i=0; i<image_filenames.size(); i++)
      {
         texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
            image_filenames[i],NULL);
         string basename=filefunc::getbasename(image_filenames[i]);
         int width=texture_rectangle_ptr->getWidth();
         int height=texture_rectangle_ptr->getHeight();
//         cout << "i = " << i 
//              << " basename = " << basename
//              << " width = " << width
//              << " height = " << height << endl;

	 if (width < min_width) continue;

         double mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B;
         texture_rectangle_ptr->get_pixel_region_RGB_moments(
            0,width-1,0,height-1,
            mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B);

         double mean_sigma = (sigma_R+sigma_G+sigma_B)/3;
//         cout << " sigma_R = " << sigma_R
//              << " sigma_B = " << sigma_G
//              << " sigma_B = " << sigma_B 
//              << " mean_sigma = " << mean_sigma << endl;
//         cout << endl;

	 if (mean_sigma < min_sigma) continue;

         string output_image_filename=curr_output_digit_subdir+
            stringfunc::number_to_string(mean_sigma,3)+"_"+
            stringfunc::number_to_string(d)+"_digit_"+
            stringfunc::number_to_string(i)+".jpg";
         string unix_cmd="cp "+image_filenames[i]+" "+output_image_filename;
         sysfunc::unix_command(unix_cmd);
	 n_exported_chips++;

         delete texture_rectangle_ptr;
      } // loop over index i labeling image filenames

      double reasonable_chip_frac = double(n_exported_chips) / image_filenames.size();

      cout << "Digit = " << d << endl;
      cout << "Number input image chips = " << image_filenames.size() << endl;
      cout << "Number output reasonable chips = " << n_exported_chips << endl;
      cout << "Reasonable chip fraction = " << reasonable_chip_frac << endl;
      cout << "Chips sorted by mean RGB std dev exported to " << curr_output_digit_subdir << endl;
      cout << endl;

   } // loop over index d labeling digits

}
