/* ==================================================================== */
/* Solve A.X=B for a vector X, where A is specified by the arrays
   u[1..m][1..n], w[1..n], v[1..n][1..n] as returned by svdcmp.  m and
   n are the dimensions of a, and will be equal for square matrices.
   b[1..m] is the input right-hand side.  x[1..n] is the output
   solution vector.  No input quantities are destroyed, so the routine
   may be called sequentially with different b's.  			*/
/* ==================================================================== */
/* Last modified on 10/12/00                                            */
/* ==================================================================== */

#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   void svbksb(double **u, double w[], double **v, int m, int n, double b[], 
               double x[]) 
      { 
         int jj,j,i; 
         double s,*tmp; 
 
         tmp=vector(1,n); 
         for (j=1;j<=n;j++) 
         { 
            s=0.0; 
            if (w[j]) 
            { 
               for (i=1;i<=m;i++) s += u[i][j]*b[i]; 
               s /= w[j]; 
            } 
            tmp[j]=s; 
         } 
         for (j=1;j<=n;j++) 
         { 
            s=0.0; 
            for (jj=1;jj<=n;jj++) s += v[j][jj]*tmp[jj]; 
            x[j]=s; 
         } 
         free_vector(tmp,1,n); 
      } 
} // numrec namespace

#undef NRANSI 



