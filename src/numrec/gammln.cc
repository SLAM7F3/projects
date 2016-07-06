/* ==================================================================== */
/* Function gammln returns the value ln(Gamma(xx)) for xx > 0.  Full
   accuracy is obtained for xx>1.  For 0 < xx < 1, the reflection
   formula (6.1.4) can be used first.  					*/
/* ==================================================================== */
/* Last modified on 7/15/03                                            */
/* ==================================================================== */

#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <iomanip>
#include <math.h> 
#include "numrec/nr.h"

namespace numrec
{
   double gammln(double xx) 
      { 
         double x,y,tmp,ser; 
         static double cof[6]=
         {76.18009172947146,-86.50532032941677, 
          24.01409824083091,-1.231739572450155, 
          0.1208650973866179e-2,-0.5395239384953e-5}
         ; 
         int j; 
 
         y=x=xx; 
         tmp=x+5.5; 
         tmp -= (x+0.5)*log(tmp); 
         ser=1.000000000190015; 
         for (j=0;j<=5;j++) ser += cof[j]/++y; 
         return -tmp+log(2.5066282746310005*ser/x); 
      } 
} // numrec namespace






