// ========================================================================
// Program TRANSLATE generates a video where we precisely move a small
// subsection of the HAFB imagery by 3 pixels per frame.  We wrote
// this little utility in order to test KLT tracking under
// circumstances approximating the conditions of the Clewiston data
// collect.
// ========================================================================
// Last updated on 11/10/05
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "video/VidFile.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{
   double sensor_speed=80;	// meters/sec
   double dt=0.5;	// sec
   double GSD=1.0;	// meters/pixel
   int pixel_translate=basic_math::round(sensor_speed*dt*GSD);
   cout << "pixel_translate = " << pixel_translate << endl;
//   pixel_translate=0;
   int Upixels=1392;
//   int n_images=Upixels/pixel_translate;
   int n_images=28;

   string input_filename="single_bbox.vid";
   string output_filename="translate.vid";
   VidFile vid_in(input_filename);

   VidFile vid_out;
   vid_out.New_8U(output_filename.c_str(),vid_in.getWidth(),
                  vid_in.getHeight(), n_images, 1);

// We assume input represents 8-bit greyscale:

   unsigned char* pbyImgIn = 
      new unsigned char[vid_in.getWidth()*vid_in.getHeight()];
   unsigned char* pbyImgOut = 
      new unsigned char[vid_in.getWidth()*vid_in.getHeight()];

   vid_in.read_image(0,pbyImgIn);
   
   for (unsigned int n=0; n <= n_images; n++)
   {
      cout << n << " " << flush;
      int horiz_pixel_translation=n*pixel_translate;
      videofunc::translate_image(
         pbyImgIn,pbyImgOut,vid_in.getWidth(),vid_in.getHeight(),
         horiz_pixel_translation);
      vid_out.WriteFrame(pbyImgOut, vid_in.getWidth());
   }
   cout << endl;

   delete pbyImgIn;
   delete pbyImgOut;

}
