/* ==================================================================== */
/* Given a set of data points x[1..ndat], y[1..ndat] with individual
   standard deviations sig[1..ndat], use chisq minimization to fit for
   some or all of the coefficients a[1..ma] of a function that depends
   linearly on a, y=sum_i a_i x afunc_i(x).  The input array ia[1..ma]
   indicates by nonzero entries those components of a that should be
   fitted for, and by zero entires, those components that should be
   held fixed at their input values.  The program returns values for
   a[1..ma], chisq and the covariance matrix covar[1..ma][1..ma].
   (Parameters held fixed will return zero covariances.)  The user
   supplies a routine funcs(x,afunc,ma) that returns the ma basis
   functions evaluated at x in the array afunc[1..ma].			*/
/* ==================================================================== */
/* Last modified on 11/28/01                                            */
/* ==================================================================== */

#define NRANSI 
#include "numrec/nrutil.h" 
 
namespace numrec
{
   void lfit(double x[],double y[],double sig[],int ndat,double a[],int ia[], 
             int ma, double **covar, double *chisq, 
             void (*funcs)(double, double [], int)) 
      { 
         void covsrt(double **covar, int ma, int ia[], int mfit); 
         void gaussj(double **a, int n, double **b, int m); 
         int i,j,k,l,m,mfit=0; 
         double ym,wt,sum,sig2i,**beta,*afunc; 
 
         beta=matrix(1,ma,1,1); 
         afunc=vector(1,ma); 
         for (j=1;j<=ma;j++) 
            if (ia[j]) mfit++; 
         if (mfit == 0) nrerror("lfit: no parameters to be fitted"); 
         for (j=1;j<=mfit;j++) 
         { 
            for (k=1;k<=mfit;k++) covar[j][k]=0.0; 
            beta[j][1]=0.0; 
         } 
         for (i=1;i<=ndat;i++) 
         { 
            (*funcs)(x[i],afunc,ma); 
            ym=y[i]; 
            if (mfit < ma) 
            { 
               for (j=1;j<=ma;j++) 
                  if (!ia[j]) ym -= a[j]*afunc[j]; 
            } 
            sig2i=1.0/SQR(sig[i]); 
            for (j=0,l=1;l<=ma;l++) 
            { 
               if (ia[l]) 
               { 
                  wt=afunc[l]*sig2i; 
                  for (j++,k=0,m=1;m<=l;m++) 
                     if (ia[m]) covar[j][++k] += wt*afunc[m]; 
                  beta[j][1] += ym*wt; 
               } 
            } 
         } 
         for (j=2;j<=mfit;j++) 
            for (k=1;k<j;k++) 
               covar[k][j]=covar[j][k]; 
         gaussj(covar,mfit,beta,1); 
         for (j=0,l=1;l<=ma;l++) 
            if (ia[l]) a[l]=beta[++j][1]; 
         *chisq=0.0; 
         for (i=1;i<=ndat;i++) 
         { 
            (*funcs)(x[i],afunc,ma); 
            for (sum=0.0,j=1;j<=ma;j++) sum += a[j]*afunc[j]; 
            *chisq += SQR((y[i]-sum)/sig[i]); 
         } 
         covsrt(covar,ma,ia,mfit); 
         free_vector(afunc,1,ma); 
         free_matrix(beta,1,ma,1,1); 
      } 
} // numrec namespace
 
#undef NRANSI 
