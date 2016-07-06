// ====================================================================
// Given a function func and given distinct initial points ax and bx,
// subroutine MNBRAK searches in the downhill direction (defined by
// the function as evaluated at the initial points) and returns new
// points ax, bx and cx that bracket a minimum of the function.  Also
// returned are the function values at the three points fa, fb and fc.
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#include <math.h> 
#define NRANSI 
#include "numrec/nrutil.h" 

namespace numrec
{
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d); 
 
   void mnbrak(double *ax,double *bx,double *cx,double *fa,double *fb, 
               double *fc,double (*func)(double)) 

      { 
         const double GOLD=1.618034;
         const double GLIMIT=100.0;
         const double TINY=1E-20;
         
         double ulim,u,r,q,fu,dum; 
 
         *fa=(*func)(*ax); 
         *fb=(*func)(*bx); 
         if (*fb > *fa) 
         { 
            SHFT(dum,*ax,*bx,dum) 
               SHFT(dum,*fb,*fa,dum) 
               } 
         *cx=(*bx)+GOLD*(*bx-*ax); 
         *fc=(*func)(*cx); 
         while (*fb > *fc) 
         { 
            r=(*bx-*ax)*(*fb-*fc); 
            q=(*bx-*cx)*(*fb-*fa); 
            u=(*bx)-((*bx-*cx)*q-(*bx-*ax)*r)/ 
               (2.0*SIGN(FMAX(fabs(q-r),TINY),q-r)); 
            ulim=(*bx)+GLIMIT*(*cx-*bx); 
            if ((*bx-u)*(u-*cx) > 0.0) 
            { 
               fu=(*func)(u); 
               if (fu < *fc) 
               { 
                  *ax=(*bx); 
                  *bx=u; 
                  *fa=(*fb); 
                  *fb=fu; 
                  return; 
               }
               else if (fu > *fb) 
               { 
                  *cx=u; 
                  *fc=fu; 
                  return; 
               } 
               u=(*cx)+GOLD*(*cx-*bx); 
               fu=(*func)(u); 
            }
            else if ((*cx-u)*(u-ulim) > 0.0) 
            { 
               fu=(*func)(u); 
               if (fu < *fc) 
               { 
                  SHFT(*bx,*cx,u,*cx+GOLD*(*cx-*bx)) 
                     SHFT(*fb,*fc,fu,(*func)(u)) 
                     } 
            }
            else if ((u-ulim)*(ulim-*cx) >= 0.0) 
            { 
               u=ulim; 
               fu=(*func)(u); 
            }
            else 
            { 
               u=(*cx)+GOLD*(*cx-*bx); 
               fu=(*func)(u); 
            } 
            SHFT(*ax,*bx,*cx,u) 
               SHFT(*fa,*fb,*fc,fu) 
               } 
      } 
#undef SHFT 

} // numrec namespace

#undef NRANSI 

