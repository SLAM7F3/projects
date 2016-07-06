/* ==================================================================== */
/* Subroutine banmul performs the matrix multiplication, b = A x, where
   A is band diagonal with m1 rows below the diagonal and m2 rows
   above.  The input vector x and output vector b are stored as x[1..n]
   and b[1..n].  The array a[1..n][1..m1+m2+1] stores A as follows:
   The diagonal elements are in a[1..n][m1+1].  Subdiagonal elements are
   in a[j..n][1..m1] (with j>1 appropriate to the number of elements on
   each subdiagonal).  Superdiagonal elements are in 
   a[1..j][m1+2..m1+m2+1] with j<n appropriate to the number of elements
   on each superdiagonal.						*/
/* ==================================================================== */
/* Last modified on 10/28/00                                            */
/* ==================================================================== */

#define NRANSI 
#include "numrec/nrutil.h" 
 
namespace numrec
{
   void banmul(double **a,unsigned long n,int m1,int m2,double x[],double b[]) 
      { 
         unsigned long i,j,k,tmploop; 
 
         for (i=1;i<=n;i++) 
         { 
            k=i-m1-1; 
            tmploop=LMIN(m1+m2+1,n-k); 
            b[i]=0.0; 
            for (j=LMAX(1,1-k);j<=tmploop;j++) b[i] += a[i][j]*x[j+k]; 
         } 
      } 
} // numrec namespace

#undef NRANSI 
