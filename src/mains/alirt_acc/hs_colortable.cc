// ==========================================================================
// Program HS_COLORTABLE generates an RGB colortable for metafile
// images.  Its M columns each corresponds to a different saturation
// value, while its N rows correspond to different hues.  As the
// independent variable q monotonically increases from 0 to MN-1,
// different hues from magenta to red are covered, while saturations
// range from none (s=0 --> pure hues) to total (s=1 --> pure white).
// We cooked up this utility program in order to experiment with
// displaying 4D information via 2D color images !
// ==========================================================================
// Last updated on 8/30/03
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "general/outputfuncs.h"
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
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];

   parameter_input(argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

   int M=6;
   int N=41;

   double s_hi=1;
   double s_lo=0;
//   double s_lo=0.5;
   double ds=(s_hi-s_lo)/(M-1);

   double h_hi=300;	// degs		(magenta)
   double h_lo=0;	// degs		(red)
   double dh=(h_hi-h_lo)/(N-1);

   for (int q=0; q<M*N-1; q++)
   {
      int m;
      int n=q/M;
      if (is_even(n))
      {
         m=q%M;
      }
      else
      {
         m=M-1-(q%M);
      }

      double s=s_lo+m*ds;
      double h=h_lo+n*dh;
      double v=1;
      double R,G,B;
      colorfunc::hsv_to_RGB(h,v,s,R,G,B);
//      colorfunc::hsv_to_RGB(h,s,v,R,G,B);
//      cout << "q = " << q << " m = " << m << " n = " << n << endl;
//      cout << "s = " << s << " h = " << h << endl;
//      cout << "R = " << R << " G = " << G << " B = " << B << endl;
//      outputfunc::newline();
      cout << q << "\t" << R << " \t" << G << "\t"
           << B << "\t" << 0 << "\t" << endl;
   }
}






