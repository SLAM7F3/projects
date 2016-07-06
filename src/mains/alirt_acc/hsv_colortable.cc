// ==========================================================================
// Program HSV_COLORTABLE generates an RGB colortable for 3D Group 94
// Dataviewer images.  Its M columns each corresponds to a different
// saturation or intensity value, while its N rows correspond to
// different hues.  As the independent variable q monotonically
// increases from 0 to MN-1, different hues from magenta to red are
// covered, while saturations range from none (s=1 --> pure hues) to
// total (s=0 --> pure greys).  We cooked up this utility program in
// order to experiment with displaying 4D information via 2D color
// images !
// ==========================================================================
// Last updated on 7/5/04
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <new>
#include <string>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
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
   unsigned int ninputlines;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);

//   int M=12;
   int M=24;		// orig
//   int M=36;
   //   int M=48;
//   int N=32;
   int N=64;		// orig

//   int M=6;
//   int N=41;

//   int M=12;
//   int N=64;

   double h_hi=300;	// degs		(magenta)
   double h_lo=0;	// degs		(red)
   double dh=(h_hi-h_lo)/double(N-1);

   double s_hi=1;
   double s_lo=0.0;
   double ds=(s_hi-s_lo)/double(M-1);

//   double v_hi=1;
//   double v_lo=0.0;
//   double dv=(v_hi-v_lo)/double(M-1);

   int counter=0;
   for (int q=0; q<M*N; q++)
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

      double h=h_lo+n*dh;
      double s=s_lo+m*ds;
//      double s=1;
//    double v=v_lo+m*dv;
      double v=1;

      double r,g,b;
      colorfunc::hsv_to_RGB(h,s,v,r,g,b);
//      double magnitude=sqrt(sqr(r)+sqr(g)+sqr(b));
      
      if (m==0)
      {
//         cout << "s = " << s << " h = " << h << endl;
//         cout << "R = " << r << " G = " << g << " B = " << b << endl;
//         cout << "sqrt(R**2+G**2+B**2) = " << magnitude << endl;
//         outputfunc::newline();
//         cout << h << " \t\t" << magnitude << endl;

      }
      
//      cout << "q = " << q << " m = " << m << " n = " << n << endl;
//      outputfunc::newline();
//      cout << q << "\t" << R << " \t" << G << "\t" << B << "\t" << endl;
//      cout << q << "\t" << r << " \t" << g << "\t" << b << "\t" << endl;

      cout.precision(4);
      cout.setf(ios::showpoint);
//      cout << "         map[" << counter++ << "]=" << r << ";\t"
//           << " map[" << counter++ << "]=" << g << ";\t"
//           << " map[" << counter++ << "]=" << b << ";" << endl;

      cout << "         map[" << counter++ << "]=" << r << ";" << endl;
      cout << "         map[" << counter++ << "]=" << g << ";" << endl;
      cout << "         map[" << counter++ << "]=" << b << ";" << endl;

   }
}






