/* ==================================================================== */
/* Computes (a^2+b^2)**0.5 without destructive underflow or overflow    */
/* ==================================================================== */
/* Last modified on 1/5/99. 						*/
/* ==================================================================== */

#include <math.h> 
#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   double pythag(double a, double b) 
      { 
         double absa,absb; 
         absa=fabs(a); 
         absb=fabs(b); 
         if (absa > absb) return absa*sqrt(1.0+SQR(absb/absa)); 
         else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb))); 
      } 
} // numrec namespace

#undef NRANSI 

