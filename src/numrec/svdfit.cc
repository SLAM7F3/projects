// ====================================================================
// Given a set of data points x[1..ndata], y[1..ndata] with individual
// standard deviations sig[1..ndata], SVDFIT uses chisq minimization
// to determine the coefficients a[1..ma] of the fitting function
// y=sum_i a_i x afunc_i(x).  Here we solve the fitting equations
// using singular value decomposition of the ndata x ma matrix as in
// section 2.6 of Numerical Recipes.  Arrays u[1..ndata][1..ma],
// v[1..ma][1..ma] and w[1..ma] provide workspace on input.  On
// output, they define the singular value decomposition, and they can
// be used to obtain the covariance matrix.  The programs returns
// values for the ma fit parameters a, and chisq.  The user supplies a
// routine funcs(x,afunc,ma) that returns the ma basis functions
// evaluated at x=X in the array afunc[1..ma]
// ====================================================================
// Last modified on 3/25/04
// ====================================================================

#define NRANSI 
#include <math.h>
#include "math/constants.h"
#include "numrec/nr.h"
#include "numrec/nrutil.h" 

namespace numrec
{
   bool svdfit(double x[], double y[], double sig[], int ndata, double a[], 
               int ma, double **u, double **v, double w[], double *chisq, 
               void (*funcs)(double, double [], int)) 
      { 

// We redefined TOL from 10**-5 to 10**-10 on 4/11/01 in order to
// obtain reasonable looking polynomial fits to translation data:

// We redefined TOL from 10**-10 to 10**-15 on 8/17/01 in order to
// obtain reasonable looking polynomial fits to maneuvering
// probability data:

         const double TOL=1.0E-17;
//         const double TOL=1.0E-15;
//         const double TOL=1.0E-10;
         void svbksb(double **u, double w[], double **v, int m, int n, 
                     double b[], double x[]); 
         bool svdcmp(double **a, int m, int n, double w[], double **v); 
         int j,i; 
         double wmax,tmp,thresh,sum,*b,*afunc; 

//    printf("Inside svdfit, ndata = %d \n",ndata);
//    for (i=1; i<= ndata; i++)
//    {
//       printf("%d \t %f \t %f \n",i,x[i],y[i]);
//    }

         b=vector(1,ndata); 
         afunc=vector(1,ma); 
         for (i=1;i<=ndata;i++) 
         { 
            (*funcs)(x[i],afunc,ma); 
            tmp=1.0/sig[i]; 
            for (j=1;j<=ma;j++) u[i][j]=afunc[j]*tmp; 
            b[i]=y[i]*tmp; 
         } 

         bool svdfit_successfully_completed=svdcmp(u,ndata,ma,w,v); 
         if (svdfit_successfully_completed)
         {
            wmax=0.0; 
            for (j=1;j<=ma;j++) 
               if (w[j] > wmax) wmax=w[j]; 
            thresh=TOL*wmax; 
            for (j=1;j<=ma;j++) 
               if (w[j] < thresh) w[j]=0.0; 
            svbksb(u,w,v,ndata,ma,b,a); 
            *chisq=0.0; 
            for (i=1;i<=ndata;i++) 
            { 
               (*funcs)(x[i],afunc,ma); 
               for (sum=0.0,j=1;j<=ma;j++) sum += a[j]*afunc[j]; 
               *chisq += (tmp=(y[i]-sum)/sig[i],tmp*tmp); 
            } 
         } // svdfit_successfully_completed
         
         free_vector(afunc,1,ma); 
         free_vector(b,1,ndata); 

         return svdfit_successfully_completed;
      } 

// ==========================================================================
// Subroutine numrec_poly returns an ordinary polynomial of degree
// np-1 which is used for least squares fitting.  The polynomial's
// basis functions are contained within array p[1..np]:

   void numrec_poly(double x,double p[],int np)
      {
         int j;

         p[1]=1.0;
         for (j=2; j<=np; j++)
         {
            p[j]=p[j-1]*x;
         }
      }

// ==========================================================================
// Subroutine numrec_cheby returns a Chebyshev polynomial of degree
// np-1 which is used for least squares fitting.  The polynomial's
// basis functions are contained within array p[1..np]:

   void numrec_cheby(double x,double p[],int np)
      {
         int i;

         for (i=0; i<np; i++)
         {
            p[i+1]=cheby_poly(x,i);
         }
      }

// ==========================================================================
// Subroutine numrec_hermite returns a Hermite polynomial of degree
// np-1 which is used for least squares fitting.  The polynomial's
// basis functions are contained within array p[1..np]:

   void numrec_hermite(double x,double p[],int np)
      {
         int i;

         for (i=0; i<np; i++)
         {
            p[i+1]=hermite_poly(x,i);
         }
      }

// ==========================================================================
// Subroutine numrec_harmonic_osc returns a polynomial of degree
// np-1 constructed from normalized quantum harmonic oscillator energy
// eigenfunctions which is used for least squares fitting.  The
// polynomial's basis functions are contained within array p[1..np]:

// Note: Input parameter y = (x-mu)/sigma.  I

   void numrec_harmonic_osc(double y,double p[],int np)
      {
         int i;

         for (i=0; i<np; i++)
         {
            p[i+1]=harmonic_osc_efunc(y,i);
         }
      }

// ==========================================================================
// Subroutine cheby_poly returns the value of basis Chebyshev
// polynomial T_j(x) where j = polyorder:

   double cheby_poly(double x,int poly_order)
      {
         double T;

         if (poly_order==0)
         {
            T=1;
         }
         else if (poly_order==1)
         {
            T=x;
         }
         else
         {
            T=2*x*cheby_poly(x,poly_order-1)-cheby_poly(x,poly_order-2);
         }
   
         return T;
      }

// ==========================================================================
// Subroutine hermite_poly returns the value of basis Hermite
// polynomial H_j(x) where j = polyorder:

   double hermite_poly(double x,int poly_order)
      {
         double H;

         if (poly_order==0)
         {
            H=1;
         }
         else if (poly_order==1)
         {
            H=2*x;
         }
         else
         {
            H=2*x*hermite_poly(x,poly_order-1)-
               2*(poly_order-1)*hermite_poly(x,poly_order-2);
         }
         return H;
      }

// ==========================================================================
// Subroutine harmonic_osc_efunc returns the value of the j=polyorder
// normalized quantum harmonic oscillator energy eigenfunction
// phi_j(y; mu=0; sigma=1).  Note: y = (x-mu)/sigma.

   double harmonic_osc_efunc(double y,int poly_order)
      {
         double prefactor=pow(1/(PI),0.25)/
            sqrt(pow(2,poly_order)*numrec_factorial(poly_order));
         double gaussian=exp(-y*y/2);
         double phi=prefactor*gaussian*hermite_poly(y,poly_order);
         return phi;
      }

   double numrec_factorial(int n)
      {
         if (n==0)
         {
            return 1;
         }
         else if (n > 0)
         {
            return exp(gammln(n+1));
         }
         else
         {
            return -1;
         }
      }
} // numrec namespace

#undef NRANSI 

