// ====================================================================
// Program PNGTEST; See http://zarb.org/~gc/html/libpng.html and
// http://www.libpng.org/pub/png/libpng-1.2.5-manual.html
// ====================================================================
// Last updated on 1/5/16; 1/6/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <png.h>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;


int main(int argc, char** argv)
{  
   int width = 1000;
   int height = 800;
   string png_filename="test.png";


                     
   vector<vector<unsigned char> > byte_array;
   for(int py = 0; py < height; py++)
   {
      vector<unsigned char> curr_byte_row;
      for(int px = 0; px < width; px++)
      {
         int curr_sum = px + py;
         int curr_value = curr_sum % 255;
         unsigned char curr_char = curr_value;
         curr_byte_row.push_back( curr_char );
      }
      byte_array.push_back(curr_byte_row);
   }

   videofunc::write_8bit_greyscale_pngfile(byte_array, png_filename);
}

