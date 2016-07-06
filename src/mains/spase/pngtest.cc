// ==========================================================================
// Program PNGTEST
// ==========================================================================
// Last updated on 1/13/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <png.h>
#include <set>
#include <string>
#include <vector>
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "datastructures/Linkedlist.h"
#include "image/myimage.h"
#include "general/outputfuncs.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ostream;
   using std::pair;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
   const double max_x=0.5;  // meters
   const double max_y=0.5;  // meters
	
   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Parse PNG file:

//   string png_filename="415179-03D.png";
   string png_filename="HAFB_map.png";

   bool png_opened_successfully=pngfunc::open_png_file(png_filename);
   pngfunc::parse_png_file();
   pngfunc::draw_png_file();
   pngfunc::close_png_file();

}
