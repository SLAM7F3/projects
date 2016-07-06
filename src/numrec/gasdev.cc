/* =================================================================	*/
/* Returns a normally distributed deviate with zero mean and unit
   variance, using ran1(idum) as the source of uniform deviates.	*/
/* =================================================================	*/
/* Last modified on 10/22/98						*/
/* ==================================================================	*/

#include <math.h>

namespace numrec
{
   double gasdev(long *idum)
      {
         double ran1(long *idum);
         static int iset=0;
         static double gset;
         double fac,rsq,v1,v2;

         if  (iset == 0) {
            do {
               v1=2.0*ran1(idum)-1.0;
               v2=2.0*ran1(idum)-1.0;
               rsq=v1*v1+v2*v2;
            } while (rsq >= 1.0 || rsq == 0.0);
            fac=sqrt(-2.0*log(rsq)/rsq);
            gset=v1*fac;
            iset=1;
            return v2*fac;
         } else {
            iset=0;
            return gset;
         }
      }
} // numrec namespace 

