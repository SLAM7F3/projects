/* ===================================================================== */
/* Subroutine lubksb solves the set of n linear equations AX=B.  Here
   A is input, not as the matrix A but rather as its LU decomposition,
   determined by the routine ludcmp.  INDX is input as the permutation
   vector returned by ludcmp.  B is input as the right-hand side vector
   B, and returns with the solution vector X.  A, N and INDX are not
   modified by this routine and can be left in place for successive calls
   with different right-hand sides B.  This routine takes into account
   the possibility that B will begin with many zero elements, so it is
   efficient for use in matrix inversion.				 */
/* ===================================================================== */
/* Last modified on 12/22/98						 */
/* ===================================================================== */

namespace numrec
{
   void lubksb(double **a, int n, int *indx, double b[])
      {
         int i,ii=0,ip,j;
         double sum;

         for (i=1;i<=n;i++) 
         {
            ip=indx[i];
            sum=b[ip];
            b[ip]=b[i];
            if (ii)
               for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
            else if (sum) ii=i;
            b[i]=sum;
         }

         for (i=n;i>=1;i--) 
         {
            sum=b[i];
            for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
            b[i]=sum/a[i][i];
         }
      }
} // numrec namespace













