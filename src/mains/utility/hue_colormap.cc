// ==========================================================================
// Program HUE_COLORMAP generates an RGB colormap file which
// corresponds to pure red down to pure purple hues with unit
// saturation and intensity values.
// ==========================================================================
// Last updated on 3/31/06
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include "color/colorfuncs.h"
#include "general/sysfuncs.h"

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
  
   double hue_start=300;
   double hue_stop=0;
   int nbins=63;
   double d_hue=(hue_stop-hue_start)/(nbins-1);

   for (int n=0; n<nbins; n++)
   {
      double h=hue_start+n*d_hue;
      double s=1;
      double v=1;
      double r,g,b;
      colorfunc::hsv_to_RGB(h,s,v,r,g,b);
      cout << r << "     " << g << "     " << b << endl;
   }
   

}


