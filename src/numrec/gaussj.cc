/* ==================================================================== */
/* Linear equation solution by Gauss-Jordan elimination (see eqn 2.1.1
   in Numerical Recipes book).  a[1..n][1..n] is an input matrix of
   n by n elements.  b[1..n][1..m] is an input matrix of size n by m
   containing the m right-hand side vectors.  On output, a is replaced by
   its matrix inverse, and b is replaced by the corresponding set of 
   solution vectors.							*/
/* ==================================================================== */
/* Last modified on 10/12/00					        */
/* ==================================================================== */

#include <math.h> 
#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   
#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;} 
 
   void gaussj(double **a, int n, double **b, int m) 

      { 
         int *indxc,*indxr,*ipiv; 
         int i,icol,irow,j,k,l,ll; 
         double big,dum,pivinv,temp; 
 
         indxc=ivector(1,n); 
         indxr=ivector(1,n); 
         ipiv=ivector(1,n); 
         for (j=1;j<=n;j++) ipiv[j]=0; 
         for (i=1;i<=n;i++) 
         { 
            big=0.0; 
            for (j=1;j<=n;j++) 
               if (ipiv[j] != 1) 
                  for (k=1;k<=n;k++) 
                  { 
                     if (ipiv[k] == 0) 
                     { 
                        if (fabs(a[j][k]) >= big) 
                        { 
                           big=fabs(a[j][k]); 
                           irow=j; 
                           icol=k; 
                        } 
                     }
                     else if (ipiv[k] > 1) nrerror( (char *) 
                        "gaussj: Singular Matrix-1"); 
                  } 
            ++(ipiv[icol]); 
            if (irow != icol) 
            { 
               for (l=1;l<=n;l++) 
                  SWAP(a[irow][l],a[icol][l]) 
                     for (l=1;l<=m;l++) SWAP(b[irow][l],b[icol][l]) 
                     } 
            indxr[i]=irow; 
            indxc[i]=icol; 
            if (a[icol][icol] == 0.0) nrerror( (char *) "gaussj: Singular Matrix-2"); 
            pivinv=1.0/a[icol][icol]; 
            a[icol][icol]=1.0; 
            for (l=1;l<=n;l++) a[icol][l] *= pivinv; 
            for (l=1;l<=m;l++) b[icol][l] *= pivinv; 
            for (ll=1;ll<=n;ll++) 
               if (ll != icol) 
               { 
                  dum=a[ll][icol]; 
                  a[ll][icol]=0.0; 
                  for (l=1;l<=n;l++) a[ll][l] -= a[icol][l]*dum; 
                  for (l=1;l<=m;l++) b[ll][l] -= b[icol][l]*dum; 
               } 
         } 
         for (l=n;l>=1;l--) 
         { 
            if (indxr[l] != indxc[l]) 
               for (k=1;k<=n;k++) 
                  SWAP(a[k][indxr[l]],a[k][indxc[l]]); 
         } 
         free_ivector(ipiv,1,n); 
         free_ivector(indxr,1,n); 
         free_ivector(indxc,1,n); 
      } 
#undef SWAP 
} // numrec namespace


#undef NRANSI 

