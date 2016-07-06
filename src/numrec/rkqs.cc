// =============================================================
// Fifth-order Runge-Kutta step with monitoring of local truncation
// error to ensure accuracy and adjust stepsize.  Input are the
// dependent variable vector y[1..n] and its derivative dydx[1..n] at
// the starting value of the independent variable x.  Also input are
// the stepsize to be attempted htry, the required accuracy eps and
// the vector yscal[1..n] against which the error is scaled.  On
// output, y and x are replaced by their new values, hdid is the
// stepsize that was actually accompolished, and hnext is the
// estimated next stepsize.  derivs is the user-supplied routine that
// computes the right-hand side derivatives.
// =============================================================	
// Last modified on 8/12/03
// =============================================================	

#include <math.h>
#define NRANSI
#include "numrec/nrutil.h"

namespace numrec
{
   void rkqs(double y[],double dydx[],int n,double *x,double htry, double eps,
             double yscal[], double *hdid, double *hnext,
             void (*derivs)(double, double [], double []))
      {
         const double SAFETY=0.9;
         const double PGROW=-0.2;
         const double PSHRNK=-0.25;
         const double ERRCON=1.89E-4;
         
         void rkck(double y[], double dydx[], int n, double x, double h,
                   double yout[], double yerr[], 
                   void (*derivs)(double, double [], double []));
         int i;
         double errmax,h,xnew,*yerr,*ytemp;

         yerr=vector(1,n);
         ytemp=vector(1,n);
         h=htry;
         for (;;)
         {
            rkck(y,dydx,n,*x,h,ytemp,yerr,derivs);
            errmax=0.0;
            for (i=1;i<=n;i++) errmax=FMAX(errmax,fabs(yerr[i]/yscal[i]));
            errmax /= eps;

            if (errmax > 1.0) 
            {
               h=SAFETY*h*pow(errmax,PSHRNK);
               if (h < 0.1*h) h *= 0.1;
               xnew=(*x)+h;
               if (xnew == *x) nrerror((char *) "stepsize underflow in RKQS");
               continue;
            }
            else
            {
               if (errmax > ERRCON) *hnext=SAFETY*h*pow(errmax,PGROW);
               else *hnext=5.0*h;
               *x += (*hdid=h);
               for (i=1;i<=n;i++) y[i]=ytemp[i];
               break;
            }

//	printf("errmax,h,hnext = %G, %G, %G \n",errmax, h,*hnext);
	
         }
         free_vector(ytemp,1,n);
         free_vector(yerr,1,n);
      }
} // numrec namespace

#undef NRANSI








