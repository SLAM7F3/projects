/* ==================================================================== */
/* Note: We renamed Numerical Recipe's function beta(double z,double w) 
   as beta_function(double z,double w) on 2/26/00 in order to avoid
   clashes with other source codes that use the very common variable
   name beta.  								*/
/* ==================================================================== */
/* Last modified on 2/26/00.                                            */
/* ==================================================================== */

#include <math.h> 

namespace numrec
{
   double beta_function(double z, double w) 
      { 
         double gammln(double xx); 
 
         return exp(gammln(z)+gammln(w)-gammln(z+w)); 
      } 
}

