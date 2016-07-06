// ==========================================================================
// Program MAGICK
// ==========================================================================
// Last updated on 1/31/16
// ==========================================================================

#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <vector>
#include <Magick++.h> 

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/mypolynomial.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::set;
using std::string;
using std::vector;

// --------------------------------------------------------------------------
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
//   InitializeMagick(*argv);

   // Construct the image object. Seperating image construction from the 
   // the read operation ensures that a failure to read the image file 
   // doesn't render the image object useless. 


/*

  convert -background 'rgb(0,0,0)' -fill 'rgb(124.89804,105.47478,88.86060)' 
  -font './fonts/reasonable_fonts/Attract more women.ttf' 
  -pointsize 50 -size 29x41 -gravity East label:'6' 
  ./char_images/char_000003.jpg

*/

   int width = 200;
   int height = 100;


   Magick::Geometry chip_size(width, height, 0, 0);
   Magick::ColorRGB background_color(0, 0, 1.0);
   Magick::Image image(chip_size, background_color);
   
   string curr_font = "./fonts/reasonable_fonts/Attract more women.ttf";
   image.font(curr_font);

   double pointsize = 50;
   image.fontPointsize(pointsize);

   Magick::ColorRGB foreground_color(1, 0, 0);

   image.fillColor(foreground_color);
      
   string label = "6";
   Magick::GravityType gravity = Magick::SouthWestGravity;
//      Magick::GravityType gravity = Magick::NorthEastGravity;
   image.annotate(label, chip_size, gravity);


   // Write the image to a file 
   string output_filename="output.jpg";
   image.write( output_filename ); 
   cout << "Exported "+output_filename << endl;
      
   return 0; 
}


