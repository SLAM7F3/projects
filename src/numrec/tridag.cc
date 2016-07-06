/* ==================================================================== */
/* Subroutine TRIDAG solves for a vector u[1..n], the tridiagonal
linear set given by equation (2.4.1).  a[1..n], b[1..n], c[1..n] and
r[1..n] are input vectors and are not modified.				*/
/* ==================================================================== */
// Last modified on 7/15/03
/* ==================================================================== */

#define NRANSI 
#include "numrec/nrutil.h" 
// #include "math/complex.h"

namespace numrec
{
/*
   void tridag(complex a[],complex b[],complex c[], complex r[], complex u[], 
               unsigned long n) 
      { 
         unsigned long j; 
         complex bet; 
         complex *gam=new complex[n+1];

         if (b[1].getmod() == 0.0) nrerror("Error 1 in tridag"); 
         u[1]=r[1]/(bet=b[1]); 
         for (j=2;j<=n;j++) 
         { 
            gam[j]=c[j-1]/bet; 
            bet=b[j]-a[j]*gam[j]; 
            if (bet.getmod() == 0.0) nrerror("Error 2 in tridag"); 
            u[j]=(r[j]-a[j]*u[j-1])/bet; 
         } 
         for (j=(n-1);j>=1;j--) 
            u[j] -= gam[j+1]*u[j+1]; 

         delete [] gam;
      } 
*/

} // numrec namespace

#undef NRANSI 





