// ========================================================================
// Program CONVERT_IMAGES generates an executable script which calls
// ImageMagick's conversion routine.  It can be used to convert an
// entire sequence of RGB images to PNG, JPG images to PNG, etc.
// ========================================================================
// Last updated on 2/10/06; 9/6/07; 6/18/11
// ========================================================================

#include <iostream>
#include <string>
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   int start_image,stop_image;
   cout << "Enter starting image number:" << endl;
   cin >> start_image;
   cout << "Enter stopping image number:" << endl;
   cin >> stop_image;

   string basefilename;
   cout << "Enter base filename:" << endl;
   cin >> basefilename;
  
//   string imagedir="./recorded_video/"+basefilename+"/";
//   string imagedir="./el_10.0/";
//   string imagedir="./el_25.0/";
//   string imagedir="./el_40.0/";
//   string imagedir="./az_180/";
//   string imagedir="./spase/";
//   string imagedir="./";

   string input_dir="./";
//   string output_dir="./rgb_images/";
//   string output_dir="./png_images/";
   string output_dir="./jpg_images/";

//   string init_suffix="PNG";
//   string init_suffix="rgb";
//   string init_suffix="tif";
//   string init_suffix="png";
   string init_suffix="ppm";
//   string final_suffix="png";
//   string final_suffix="rgb";
//   string final_suffix="bmp";
   string final_suffix="jpg";
   int quality=100;
   string scriptfilename="image_convert";

   int image_skip=1;
//   int n_digits=2;
//   int n_digits=4;
//   int n_digits=5;
   int n_digits=10;
   outputfunc::generate_conversion_script(
      start_image,stop_image,basefilename,input_dir,output_dir,
      scriptfilename,init_suffix,final_suffix,image_skip,n_digits,quality);
}
