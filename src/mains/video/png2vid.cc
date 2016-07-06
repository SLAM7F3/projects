// ========================================================================
// Program PNG2VID converts an input PNG image to a single-frame Group
// 99 video which can be viewed and manipulated using programs
// mains/video/VIDEO and mains/osg/VIDEO3D.
// ========================================================================
// Last updated on 10/17/05
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
//   png_filename="HAFB_roadways1.png";

   bool png_opened_successfully=pngfunc::open_png_file(png_filename);
   pngfunc::parse_png_file();
   unsigned char* charstar_array_ptr=pngfunc::generate_charstar_array();

   unsigned int dot_posn=png_filename.rfind(".png");
   string vid_filename=png_filename.substr(0,dot_posn)+".vid";
   int n_images=1;

   VidFile vid_out;
   vid_out.New_8U(vid_filename.c_str(),pngfunc::width,
                  pngfunc::height, n_images, 3);

   vid_out.WriteFrame(charstar_array_ptr, pngfunc::width*3);

   delete [] charstar_array_ptr;
   pngfunc::close_png_file();
}
