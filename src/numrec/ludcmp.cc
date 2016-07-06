// ===================================================================
// The following matrix inversion routines are taken from chapter 2 of
// NUMERICAL RECIPES in C.

//  Given an N x N matrix A, subroutine ludcmp replaces it by the LU
//  decomposition of a rowwise permutation of itself.  A and N are
//  input.  A is output, arranged as in equation (2.3.14); INDX is an
//  output vector which records the row permutation effected by the
//  partial pivoting; d is output as +/- 1 depending on whether the
//  number of row interchanges was even or odd, respectively.  Routine
//  ludcmp is used in combination with lubksb to solve linear
//  equations or invert a matrix.
// ===================================================================	
// Last modified on 8/12/03
// ===================================================================	

#include <math.h>
#define NRANSI
#include "numrec/nrutil.h"

namespace numrec
{
   bool ludcmp(double **a, int n, int *indx, double *d)
      {
         const double TINY=1E-15;
         bool ludcmp_successfully_completed=true;
         int i,imax,j,k;
         double big,dum,sum,temp;
         double *vv;
    
         vv=vector(1,n);
         *d=1.0;

         for (i=1;i<=n;i++)
         {
            big=0.0;
            for (j=1;j<=n;j++)
               if ((temp=fabs(a[i][j])) > big) big=temp;
            if (big == 0.0) 
            {
               ludcmp_successfully_completed=false;
               return ludcmp_successfully_completed;
//               nrerror("Singular matrix in routine ludcmp");
            }
            vv[i]=1.0/big;
         }

         for (j=1;j<=n;j++)
         {
            for (i=1;i<j;i++)
            {
               sum=a[i][j];
               for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
               a[i][j]=sum;
            }
            big=0.0;
            for (i=j;i<=n;i++)
            {
               sum=a[i][j];
               for (k=1;k<j;k++)
                  sum -= a[i][k]*a[k][j];
               a[i][j]=sum;
               if ( (dum=vv[i]*fabs(sum)) >= big)
               {
                  big=dum;
                  imax=i;
               }
            }
            if (j != imax)
            {
               for (k=1;k<=n;k++)
               {
                  dum=a[imax][k];
                  a[imax][k]=a[j][k];
                  a[j][k]=dum;
               }
               *d = -(*d);
               vv[imax]=vv[j];
            }
            indx[j]=imax;
            if (a[j][j] == 0.0) a[j][j]=TINY;
            if (j != n)
            {
               dum=1.0/(a[j][j]);
               for (i=j+1;i<=n;i++) a[i][j] *= dum;
            }
         }
         free_vector(vv,1,n);
         return ludcmp_successfully_completed;
      }
} // numrec namespace

#undef NRANSI


