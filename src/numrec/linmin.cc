// ====================================================================
// Given an n-dimensional point p[1..n] and an n-dimensional direction
// xi[1..n], subroutine LINMIN moves and resets p to where the
// function func(p) takes on a minimum along the direction xi from p.
// It replaces xi by the actual vector displacement that p was moved.
// It also returns as fret the value of func at the returned location
// p.  This is actually all accompolished by calling the routines
// MNBRAK and BRENT
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
   int ncom; 
   double *pcom,*xicom,(*nrfunc)(double []); 
 
   void linmin(double p[], double xi[], int n, double *fret, 
               double (*func)(double [])) 
      { 
         const double TOL=2.0E-4;
         double brent(double ax, double bx, double cx, 
                      double (*f)(double), double tol, double *xmin); 
         double f1dim(double x); 
         void mnbrak(double *ax, double *bx, double *cx, double *fa, 
                     double *fb, 
                     double *fc, double (*func)(double)); 
         int j; 
         double xx,xmin,fx,fb,fa,bx,ax; 
 
         ncom=n; 
         pcom=vector(1,n); 
         xicom=vector(1,n); 
         nrfunc=func; 
         for (j=1;j<=n;j++) 
         { 
            pcom[j]=p[j]; 
            xicom[j]=xi[j]; 
         } 
         ax=0.0; 
         xx=1.0; 
         mnbrak(&ax,&xx,&bx,&fa,&fx,&fb,f1dim); 
         *fret=brent(ax,xx,bx,f1dim,TOL,&xmin); 
         for (j=1;j<=n;j++) 
         { 
            xi[j] *= xmin; 
            p[j] += xi[j]; 
         } 
         free_vector(xicom,1,n); 
         free_vector(pcom,1,n); 
      } 
#undef NRANSI 

/* ==================================================================== */
/* Subroutine f1dim must accompany subroutine linmin			*/
/* ==================================================================== */

#define NRANSI 
#include "numrec/nrutil.h" 
 
   double f1dim(double x) 
      { 
         int j; 
         double f,*xt; 
 
         xt=vector(1,ncom); 
         for (j=1;j<=ncom;j++) xt[j]=pcom[j]+x*xicom[j]; 
         f=(*nrfunc)(xt); 
         free_vector(xt,1,ncom); 
         return f; 
      } 

} // numrec namespace

#undef NRANSI 

