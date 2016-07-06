// ====================================================================
// Runge-Kutta driver with adaptive stepsize control.  Integrate
// starting values ystart[1..nvar] from x1 to x2 with accuracy eps,
// storing intermediate results in global variables.  h1 should be set
// as a guessed first stepsize, hmin as the minimum allowed stepsize
// (can be zero).  On output, nok and nbad are the number of good and
// bad (but retried and fixed) steps taken, and ystart is replaced by
// values at the end of the integration interval.  derivs is the
// user-supplied routine for calculating the RHS derivative, while
// rkqs is the name of the stepper routine to be used.  */
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#include <math.h>
#define NRANSI
#include "numrec/nrutil.h"

namespace numrec
{
   int kmax,kount;
   double *xp,**yp,dxsav;

// User storage for intermediate results.  Preset kmax and dxsav in
// the calling program.  If kmax!=0, results are stored at approximate
// intervals dxsav in the arrays xp[1..kount], yp[1..nvar][1..kount],
// where kount is output by odeint.  Defining declarations for these
// variables, with memory allocations xp[1..kmax] and
// yp[1..nvar][1..kmax] for the arrays, should be in the calling
// program.

   void odeint(double ystart[], int nvar, double x1, double x2, double eps, 
               double h1,double hmin,double *hnew,int *nok, int *nbad,
               void (*derivs)(double, double [], double []),
               void (*rkqs)(double [], double [], int, double *, double, 
                            double, double [], double *, double *, 
                            void (*)(double, double [], double [])))
      {
         const int MAXSTP=10000;
         const double TINY=1E-30;
         
         int nstp,i;
         double xsav,x,hnext,hdid,h;
         double *yscal,*y,*dydx;

         yscal=vector(1,nvar);
         y=vector(1,nvar);
         dydx=vector(1,nvar);
         x=x1;
         h=SIGN(h1,x2-x1);
         *nok = (*nbad) = kount = 0;
         for (i=1;i<=nvar;i++) y[i]=ystart[i];
         if (kmax > 0) xsav=x-dxsav*2.0;

         for (nstp=1;nstp<=MAXSTP;nstp++) 
         {
            (*derivs)(x,y,dydx);
            for (i=1;i<=nvar;i++)
               yscal[i]=fabs(y[i])+fabs(dydx[i]*h)+TINY;
            if (kmax > 0 && kount < kmax-1 && fabs(x-xsav) > fabs(dxsav)) {
               xp[++kount]=x;
               for (i=1;i<=nvar;i++) yp[i][kount]=y[i];
               xsav=x;
            }
            if ((x+h-x2)*(x+h-x1) > 0.0) h=x2-x;
            (*rkqs)(y,dydx,nvar,&x,h,eps,yscal,&hdid,&hnext,derivs);
            if (hdid == h) ++(*nok); else ++(*nbad);
            if ((x-x2)*(x2-x1) >= 0.0) {
               for (i=1;i<=nvar;i++) ystart[i]=y[i];
               if (kmax) {
                  xp[++kount]=x;
                  for (i=1;i<=nvar;i++) yp[i][kount]=y[i];
               }
               free_vector(dydx,1,nvar);
               free_vector(y,1,nvar);
               free_vector(yscal,1,nvar);

/* On 9/8/99, PLC added the following line to routine ODEINT so that 
   the refined step size would be returned t as a parameter o the calling 
   routine.  This refined step serves as a useful guess for the input
   step size for the next call to odeint.				*/

               *hnew=h;
               return;
            }
            if (fabs(hnext) <= hmin) nrerror((char *) "Step size too small in ODEINT");
            h=hnext;
         }
         nrerror((char *) "Too many steps in routine ODEINT");
      }
} // numrec namespace

#undef NRANSI




