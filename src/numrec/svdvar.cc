// ====================================================================
// To evaluate the covariance matrix cvm[1..ma][1..ma] of the fit for
// ma parameters obtained by svdfit, call this routine with matrix
// v[1..ma][1..ma] as returned from svdfit.
// ====================================================================
// Last modified on 7/30/03
// ====================================================================

#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   void svdvar(double **v, int ma, double w[], double **cvm) 
      { 
         int k,j,i; 
         double sum,*wti; 
 
         wti=vector(1,ma); 
         for (i=1;i<=ma;i++) 
         { 
            wti[i]=0.0; 
            if (w[i]) wti[i]=1.0/(w[i]*w[i]); 
         } 
         for (i=1;i<=ma;i++) 
         { 
            for (j=1;j<=i;j++) 
            { 
               for (sum=0.0,k=1;k<=ma;k++) sum += v[i][k]*v[j][k]*wti[k]; 
               cvm[j][i]=cvm[i][j]=sum; 
            } 
         } 
         free_vector(wti,1,ma); 
      } 
} // numrec namespace

#undef NRANSI 






