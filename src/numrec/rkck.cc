/* ==================================================================== */
/* Given values for n variables y[1..n] and their derivatives dydx[1..n]
   known at x, use the fifth-order Cash-Karp Runge-Kutta method to advance
   the solution over an interval h and return the incremented variables
   as yout[1..n].  Also return an estimate of the local truncation error in
   yout using the embedded fourth-order method.  The user supplies the
   routine derivs(x,y,dydx), which returns derivatives dydx at x.	*/
/* ==================================================================== */
/* Last updated on 1/16/03						*/
/* ==================================================================== */

#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <iomanip>
#include <math.h>
#include "numrec/nrutil.h" 

namespace numrec
{
   void rkck(double y[],double dydx[],int n,double x,double h,double yout[],
             double yerr[], void (*derivs)(double, double [], double []))
      {
         int i;
         static double a2=0.2,a3=0.3,a4=0.6,a5=1.0,a6=0.875,b21=0.2,
            b31=3.0/40.0,b32=9.0/40.0,b41=0.3,b42 = -0.9,b43=1.2,
            b51 = -11.0/54.0, b52=2.5,b53 = -70.0/27.0,b54=35.0/27.0,
            b61=1631.0/55296.0,b62=175.0/512.0,b63=575.0/13824.0,
            b64=44275.0/110592.0,b65=253.0/4096.0,c1=37.0/378.0,
            c3=250.0/621.0,c4=125.0/594.0,c6=512.0/1771.0,
            dc5 = -277.0/14336.0;
         double dc1=c1-2825.0/27648.0,dc3=c3-18575.0/48384.0,
            dc4=c4-13525.0/55296.0,dc6=c6-0.25;
         double *ak2,*ak3,*ak4,*ak5,*ak6,*ytemp;

         ak2=vector(1,n);
         ak3=vector(1,n);
         ak4=vector(1,n);
         ak5=vector(1,n);
         ak6=vector(1,n);
         ytemp=vector(1,n);

         for (i=1;i<=n;i++) ytemp[i]=y[i]+b21*h*dydx[i];
         (*derivs)(x+a2*h,ytemp,ak2);

         for (i=1;i<=n;i++) ytemp[i]=y[i]+h*(b31*dydx[i]+b32*ak2[i]);
         (*derivs)(x+a3*h,ytemp,ak3);

         for (i=1;i<=n;i++) ytemp[i]=y[i]+h*(b41*dydx[i]+b42*ak2[i]
                                             +b43*ak3[i]);
         (*derivs)(x+a4*h,ytemp,ak4);

         for (i=1;i<=n;i++) 
            ytemp[i]=y[i]+h*(b51*dydx[i]+b52*ak2[i]+b53*ak3[i]+b54*ak4[i]);
         (*derivs)(x+a5*h,ytemp,ak5);

         for (i=1;i<=n;i++)
            ytemp[i]=y[i]+h*(b61*dydx[i]+b62*ak2[i]+b63*ak3[i]
                             +b64*ak4[i]+b65*ak5[i]);
         (*derivs)(x+a6*h,ytemp,ak6);

         for (i=1;i<=n;i++)
            yout[i]=y[i]+h*(c1*dydx[i]+c3*ak3[i]+c4*ak4[i]+c6*ak6[i]);

         for (i=1;i<=n;i++)
            yerr[i]=h*(dc1*dydx[i]+dc3*ak3[i]+dc4*ak4[i]+dc5*ak5[i]+
                       dc6*ak6[i]);

         free_vector(ytemp,1,n);
         free_vector(ak6,1,n);
         free_vector(ak5,1,n);
         free_vector(ak4,1,n);
         free_vector(ak3,1,n);
         free_vector(ak2,1,n);
      }
} // numrec namespace
