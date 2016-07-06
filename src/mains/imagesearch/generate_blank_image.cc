// ==========================================================================
// Program GENERATE_BLANK_IMAGE calls ImageMagick in order to generate
// a JPG or PNG image containing a blank grey background with
// specified horizontal and vertical pixel dimensions.
// ==========================================================================
// Last updated on 5/8/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include <Magick++.h>
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   int n_horiz_pixels,n_vertical_pixels;
   cout << "Enter xdim:" << endl;
   cin >> n_horiz_pixels;
   cout << "Enter ydim:" << endl;
   cin >> n_vertical_pixels;

//   string blank_filename="blank.jpg";
   string blank_filename="blank.png";

   videofunc::generate_blank_imagefile(
      n_horiz_pixels,n_vertical_pixels,blank_filename);
   
   string banner="Wrote blank image to "+blank_filename;
   outputfunc::write_big_banner(banner);

}

