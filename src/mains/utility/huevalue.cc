// ==========================================================================
// Program HUEVALUE generates an ascii RGB file containing colormaps
// which systematically vary both hue and value in order to increase
// dynamic range.
// ==========================================================================
// Last updated on 7/14/06; 8/7/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void hsv_to_RGB(double h,double s,double v,double& r,double& g,double& b)
{

// First make sure h, s and v lie within allowed intervals:

   if (s > 1)
   {
      cout << "Error in hsv_to_RGB" << endl;
      cout << "s = " << s << endl;
      s=1;
   }
   else if (s < 0)
   {
      cout << "Error in hsv_to_RGB" << endl;
      cout << "s = " << s << endl;
      s=0;
   }

   if (v > 1)
   {
      cout << "Error in hsv_to_RGB" << endl;
      cout << "v = " << v << endl;
      v=1;
   }
   else if (v < 0)
   {
      cout << "Error in hsv_to_RGB" << endl;
      cout << "v = " << v << endl;
      v=0;
   }

   if (s==0)
   {
      r=g=b=v;
   }
   else
   {
      if (h==360) h=0;
      h /= 60.0;
      int i=basic_math::round(floor(h));
      double frac=h-i;
      double p=v*(1-s);
      double q=v*(1-s*frac);
      double t=v*(1-s*(1-frac));
      switch (i)
      {
         case 0:
            r=v;
            g=t;
            b=p;
            break;
         case 1:
            r=q;
            g=v;
            b=p;
            break;
         case 2:
            r=p;
            g=v;
            b=t;
            break;
         case 3:
            r=p;
            g=q;
            b=v;
            break;
         case 4:
            r=t;
            g=p;
            b=v;
            break;
         case 5:
            r=v;
            g=p;
            b=q;
            break;
      } // switch statement
   }
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   string filename="new_hue_value.txt";
   ofstream outstream;
   filefunc::openfile(filename,outstream);

   int M=8;
   int N=32;
//   int N=38;

   double v_hi=1;
   double v_lo=0.502;
   double dv=(v_hi-v_lo)/(M-1);
         
   double h_lo=330;	// degs		(magenta)
//   double h_lo=300;	// degs		(magenta)
//   double h_lo=270;	// degs		(magenta)
//   double h_lo=240;	// degs		(magenta)
   double h_hi=0;	// degs		(red)

/*       
   double h_hi=360;	// degs		(red)
//   double h_hi=300;	// degs		(magenta)
   double h_lo=0;	// degs		(red)
*/

//   double dh=(h_hi-h_lo)/(N-1);
   double dh=(h_hi-h_lo)/N;

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
      double v=v_lo+m*dv;

      double r,g,b;
      const double s=1.0;
      hsv_to_RGB(h,s,v,r,g,b);

//      cout << "i = " << q+1 << "\t\t" 
//           << r << "\t" << g << "\t" << b << endl;
      const int precision=5;
      outstream << stringfunc::number_to_string(r,precision) << "\t\t" 
                << stringfunc::number_to_string(g,precision) << "\t\t" 
                << stringfunc::number_to_string(b,precision) << endl;
      
   } // loop over q index

   filefunc::closefile(filename,outstream);
}



   
   

