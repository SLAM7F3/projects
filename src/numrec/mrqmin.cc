/* ==================================================================== */
/* Levenberg-Marquardt method, attempting to reduce the value chisq of
   a fit between a set of points x[1..ndata], y[1..ndata] with
   individual standard deviations sig[1..ndata], and a nonlinear
   function dependent on ma coefficients a[1..ma].  The input array
   ia[1..ma] indicates by nonzero entries those components of a that
   should be fitted for, and by zero entries those components that
   should be held fixed at their input values.  The program returns
   current best-fit values for the parameters a[1..ma] and chisq.  The
   arrays covar[1..ma][1..ma], alpha[1..ma][1..ma] are used as working
   space during most iterations.  Supply a routine
   funcs(x,a,yfit,dyda,ma) that evaluates the fitting function yfit
   and its derivatives dyda[1..ma] with respect to the fitting
   parameters a at x.  On the first call, provide an initial guess for
   the parameters a, and set alambda<0 for initialization (which then
   sets alambda=0.001).  If a step succeeds, chisq becomes smaller and
   alambda decreases by a factor of 10.  If a step fails, alambda grows
   by a factor of 10.  You must call this routine repeatedly until
   convergence is acheived.  Then, make one final call with alambda=0
   so that covar[1..ma][1..ma] returns the covariance matrix and alpha
   the curvature matrix.  Parameters held fixed will return zero
   covariances.  							*/
/* ==================================================================== */
/* Last modified on 1/16/03                                             */
/* ==================================================================== */

#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <iomanip>
#include <math.h>

#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   void mrqmin(double x[], double y[], double sig[], int ndata, double a[], 
               int ia[], int ma, double **covar, double **alpha, 
               double *chisq, 
               void (*funcs)(double, double [], double *, double [], int), 
               double *alambda) 
      { 
         void covsrt(double **covar, int ma, int ia[], int mfit); 
         void gaussj(double **a, int n, double **b, int m); 
         void mrqcof(double x[], double y[], double sig[], int ndata, 
                     double a[], 
                     int ia[], int ma, double **alpha, double beta[], 
                     double *chisq, 
                     void (*funcs)(double, double [], double *, double [], 
                                   int)); 
         int j,k,l,m; 
         static int mfit; 
         static double ochisq,*atry,*beta,*da,**oneda; 

         if (*alambda < 0.0) 
         { 
            atry=vector(1,ma); 
            beta=vector(1,ma); 
            da=vector(1,ma); 

            for (mfit=0,j=1;j<=ma;j++) 
               if (ia[j]) mfit++; 
            oneda=matrix(1,mfit,1,1); 
            *alambda=0.001; 
            mrqcof(x,y,sig,ndata,a,ia,ma,alpha,beta,chisq,funcs); 
            ochisq=(*chisq); 

            for (j=1;j<=ma;j++) atry[j]=a[j]; 
         } 
         for (j=0,l=1;l<=ma;l++) 
         { 
            if (ia[l]) 
            { 
               for (j++,k=0,m=1;m<=ma;m++) 
               { 
                  if (ia[m]) 
                  { 
                     k++; 
                     covar[j][k]=alpha[j][k]; 
                  } 
               } 
               covar[j][j]=alpha[j][j]*(1.0+(*alambda)); 
               oneda[j][1]=beta[j]; 
            } 
         } 

         gaussj(covar,mfit,oneda,1); 
         for (j=1;j<=mfit;j++) da[j]=oneda[j][1]; 
         if (*alambda == 0.0) 
         { 
            covsrt(covar,ma,ia,mfit); 
            free_matrix(oneda,1,mfit,1,1); 
            free_vector(da,1,ma); 
            free_vector(beta,1,ma); 
            free_vector(atry,1,ma); 
            return; 
         } 

         for (j=0,l=1;l<=ma;l++) 
            if (ia[l]) atry[l]=a[l]+da[++j]; 

//    if (atry[1] < 0) atry[1]=TINY;

         mrqcof(x,y,sig,ndata,atry,ia,ma,covar,da,chisq,funcs); 

         if (*chisq < ochisq) 
         { 
            *alambda *= 0.1; 
            ochisq=(*chisq); 
            for (j=0,l=1;l<=ma;l++) 
            { 
               if (ia[l]) 
               { 
                  for (j++,k=0,m=1;m<=ma;m++) 
                  { 
                     if (ia[m]) 
                     { 
                        k++; 
                        alpha[j][k]=covar[j][k]; 
                     } 
                  } 
                  beta[j]=da[j]; 
                  a[l]=atry[l]; 
               } 
            } 
         }
         else 
         { 
            *alambda *= 10.0; 
            *chisq=ochisq; 
         } 

      } 
} // numrec namespace

#undef NRANSI 




