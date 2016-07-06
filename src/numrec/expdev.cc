/* ==================================================================== */
/* Subroutine expdev returns an exponentially distributed, positive,
   random deviate of unit mean, using ran1(idum) as the source of
   uniform deviates.  Subroutine rayleigh returns a random variable x
   which is distributed according to p(x) = 2 x Exp(-x^2).  

   Note:  p_exp(y) dy = p_rayleigh(x) dx where y=x^2.			*/
/* ==================================================================== */
/* Last modified on 4/25/99.                                            */
/* ==================================================================== */

#include <math.h> 
 
namespace numrec
{

   double expdev(long *idum) 
      { 
         double ran1(long *idum); 
         double dum; 
 
         do 
            dum=ran1(idum); 
         while (dum == 0.0); 
         return -log(dum); 
      } 

   double rayleigh(long *idum) 
      { 
         double ran1(long *idum); 
         double dum; 
 
         do 
            dum=ran1(idum); 
         while (dum == 0.0); 
         return sqrt(-log(dum)); 
      } 

} // numrec namespace
