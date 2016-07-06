/* ==================================================================== */
/* Routine  is used by routine mrqmin to evaluate the linearized
   fitting matrix alpha[1..mfit][1..mfit] and vector beta[1..mfit] as
   in eqn (15.5.8) in the 1992 edition of Numerical Recipes.  MRQCOF
   also computes the values of chisq.					*/
/* ==================================================================== */
/* Last modified on 7/15/03                                             */
/* ==================================================================== */

#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <iomanip>
#include <math.h>

#define NRANSI 
#include "numrec/nrutil.h" 
 
namespace numrec
{
   void mrqcof(double x[], double y[], double sig[], int ndata, double a[], 
               int ia[], int ma, double **alpha, double beta[], double *chisq, 
               void (*funcs)(double, double [], double *, double [], int)) 
      { 
         int i,j,k,l,m,mfit=0; 
         double ymod,wt,sig2i,dy,*dyda; 
 
         dyda=vector(1,ma); 
    
         for (j=1;j<=ma;j++) 
            if (ia[j]) mfit++; 

         for (j=1;j<=mfit;j++) 
         { 
            for (k=1;k<=j;k++) alpha[j][k]=0.0; 
            beta[j]=0.0; 
         } 

         *chisq=0.0; 

         for (i=1;i<=ndata;i++) 
         { 
            (*funcs)(x[i],a,&ymod,dyda,ma); 
            sig2i=1.0/(sig[i]*sig[i]); 
            dy=y[i]-ymod; 
            for (j=0,l=1;l<=ma;l++) 
            { 
               if (ia[l]) 
               { 
                  wt=dyda[l]*sig2i; 
                  for (j++,k=0,m=1;m<=l;m++) 
                     if (ia[m]) alpha[j][++k] += wt*dyda[m]; 
                  beta[j] += dy*wt; 
               } 
            } 
            *chisq += dy*dy*sig2i; 
         } 
         for (j=2;j<=mfit;j++) 
            for (k=1;k<j;k++) alpha[k][j]=alpha[j][k]; 
         free_vector(dyda,1,ma); 
      } 
} // numrec namespace

#undef NRANSI 






