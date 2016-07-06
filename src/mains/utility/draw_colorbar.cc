// ==========================================================================
// Program DRAW_COLORBAR generates an XYZP file containing a
// discretized set of p-values ranging over scale_factor*{0, 0.1, 0.2,
// 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0} where scale_factor is a
// user entered parameter.  
// ==========================================================================
// Last updated on 3/3/05
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include "threeDgraphics/character.h"
#include "threeDgraphics/characterfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "math/threevector.h"
#include "geometry/parallelogram.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/threeDstring.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::ifstream;
   using std::ofstream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   unsigned int ninputlines;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);

   string colorbar_filename="./colorbar.xyzp";   
   filefunc::deletefile(colorbar_filename);

//   double scale_factor=1.0;
//   cout << "Enter maximum colorbar value:" << endl;
//   cin >> scale_factor;

/*
   while(true)
   {
      string numberstring;
      cout << "Enter numberstring:" << endl;
      cin >> numberstring;
      string stripped_numberstring=
         stringfunc::remove_leading_zeros(numberstring);
      cout << "string with leading zeros removed = "
           << stripped_numberstring << endl;
   }
*/
 
   
   double charsize;
   cout << "Enter character size in meters:" << endl;
   cout << "(10 meters is the default size)" << endl;
   cin >> charsize;

   double xtrans_global,ytrans_global,ztrans_global;
   cout << "Enter global x translation for entire colorbar:" << endl;
   cin >> xtrans_global;
   cout << "Enter global y translation for entire colorbar:" << endl;
   cin >> ytrans_global;
   cout << "Enter global z translation for entire colorbar:" << endl;
   cin >> ztrans_global;
   threevector global_trans(xtrans_global,ytrans_global,ztrans_global);

   int n_color_tiles;
   cout << "Enter number of color tiles:" << endl;
   cin >> n_color_tiles;
//   int n_color_tiles=7;
//   int n_color_tiles=8;
//   int n_color_tiles=11;

   double delta_color_value=0.1;
//   cout << "Enter delta color value:" << endl;
//   cin >> delta_color_value;
   
   bool display_powers_of_two_flag=true;
//   bool display_powers_of_two_flag=false;

   draw3Dfunc::draw_colorbar(
      n_color_tiles,charsize,global_trans,colorbar_filename,
      delta_color_value,display_powers_of_two_flag);
   

}


