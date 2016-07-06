// ========================================================================
// Program PNGBLACK is a little utility which we wrote to counter
// weird streaking when converting metafiles with black backgrounds to
// PNG images.  This program loops over every pixel in a PNG image.
// It identifies pixels whose R, G and B values are nearly equal and
// are relatively low as likely genuine black pixels.  It then resets
// those pixels' RGB values to (0,0,0).  The black backgrounds in the
// output PNG files then are truly constant and uniformly black.
// ========================================================================
// Last updated on 2/17/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "image/pngfuncs.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ========================================================================

int main(int argc, char* argv[])
{

// Parse PNG file:

   string png_filename;
   cout << "Enter png filename:" << endl;
   cin >> png_filename;

   bool png_opened_successfully=pngfunc::open_png_file(png_filename);
   pngfunc::parse_png_file();

   int mdim=pngfunc::height;
   int ndim=pngfunc::width;
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   const Triple<int,int,int> black(0,0,0);
   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         Triple<int,int,int> curr_rgb=pngfunc::get_pixel_RGB_values(px,py);
         int r=curr_rgb.first;
         int g=curr_rgb.second;
         int b=curr_rgb.third;
//         cout << "px = " << px << " py = " << py 
//              << " r = " << r
//              << " g = " << g
//              << " b = " << b << endl;
         if (abs(r-g) < 20 && abs(r-b) < 20 && r < 140)
            pngfunc::put_pixel_RGB_values(px,py,black);
      }
   }
   
   string output_png_filename="new_"+png_filename;
   pngfunc::write_output_png_file(output_png_filename);
   pngfunc::close_png_file();
}
