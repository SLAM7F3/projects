// ==========================================================================
// Program FACETEST
// ==========================================================================
// Last updated on 7/30/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "image/imagefuncs.h"
#include "video/videofuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   string image_filename="face1.jpg";
   Magick::Image IM_image(image_filename);

   int py_start = 47;
   int py_stop = 49;
   int px_start = 47;
   int px_stop = 49;
   
   for(int py = py_start; py <= py_stop; py++)
   {
      for(int px = px_start; px <= px_stop; px++)
      {
         int R, G, B;
         videofunc::get_pixel_RGB(IM_image, px, py, R, G, B);
       
         cout << "px = " << px << " py = " << py 
              << " R = " << R << " G = " << G << " B = " << B
              << endl;
      }
   }


}



