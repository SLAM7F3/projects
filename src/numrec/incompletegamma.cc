// ====================================================================
// Numerical Recipes Incomplete Gamma function routines. Our little
// routine for computing Gamma(x) is contained within the mathfuncs
// library.
// ====================================================================
// Last modified on 8/12/03; 8/12/15
// ====================================================================

#include <iomanip>
#include <math.h>
#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include "math/constants.h" // Defn of POSITIVEINFINITY is contained within
			    // constants.h
#include "numrec/nr.h"

namespace numrec
{
   
/* ==================================================================== 
   Function gammp returns the ratio P(a,x) = gamma(a,x) / Gamma(a)	
   where gamma(a,x) = lower incomplete gamma function.		        
   ==================================================================== */

// On 11/17/98, we altered the function gammp below from its original
// Numerical Recipes version.

   double gammp(double a, double x) 
      { 
         void gcf(double *gammcf, double a, double x, double *gln); 
         void gser(double *gamser, double a, double x, double *gln); 
         void nrerror(char error_text[]); 
         double gamser,gammcf,gln; 
 
         if (x < 0.0)
         {
//      cout << "Negative argument x = " << x 
//           << " passed to gammp(a,x) function!" << endl;
//      cout << "a = " << a << endl;
//      cout << "Gammp function will return INFINITY" << endl;
            return (POSITIVEINFINITY);
         }
         else if (a <= 0.0)
         {
//      cout << "Negative argument a = " << a
//           << " passed to gammp(a,x) function!" << endl;
//      cout << "x = " << x << endl;
//      cout << "Gammp function will return INFINITY" << endl;
            return (POSITIVEINFINITY);
         }
         else if (x < (a+1.0)) 
         { 
            gser(&gamser,a,x,&gln); 
            return gamser; 
         }
         else 
         { 
            gcf(&gammcf,a,x,&gln); 
            return 1.0-gammcf; 
         } 
      } 

/* ==================================================================== */
/* Function gammq returns the Q(a,x)=1-P(a,x)				*/
/* ==================================================================== */

   double gammq(double a, double x) 
      { 
         void gcf(double *gammcf, double a, double x, double *gln); 
         void gser(double *gamser, double a, double x, double *gln); 
         void nrerror(char error_text[]); 
         double gamser,gammcf,gln; 
 
         if (x < 0.0 || a <= 0.0) 
            nrerror("Invalid arguments in routine gammq"); 
         if (x < (a+1.0)) 
         { 
            gser(&gamser,a,x,&gln); 
            return 1.0-gamser; 
         }
         else 
         { 
            gcf(&gammcf,a,x,&gln); 
            return gammcf; 
         } 
      } 

/* ==================================================================== 
   Function gser returns the incomplete gamma function P(a,x) evaluated
   by its series representation as gamser.  It also returns log(Gamma(a))
   as gln.								
   ==================================================================== */

   void gser(double *gamser, double a, double x, double *gln) 
      { 
         const int ITMAX=100;
         const double EPS=3.0E-7;
         
         double gammln(double xx); 
         void nrerror(char error_text[]); 
         int n; 
         double sum,del,ap; 
 
         *gln=gammln(a); 
         if (x <= 0.0) 
         { 
            if (x < 0.0) nrerror("x less than 0 in routine gser"); 
            *gamser=0.0; 
            return; 
         }
         else 
         { 
            ap=a; 
            del=sum=1.0/a; 
            for (n=1;n<=ITMAX;n++) 
            { 
               ++ap; 
               del *= x/ap; 
               sum += del; 
               if (fabs(del) < fabs(sum)*EPS) 
               { 
                  *gamser=sum*exp(-x+a*log(x)-(*gln)); 
                  return; 
               } 
            } 
            nrerror("a too large, ITMAX too small in routine gser"); 
            return; 
         } 
      } 

/* ==================================================================== */
/* Function gcf returns the incomplete gamma function Q(a,x) evaluated
   by its continued fraction representation as gammcf.  It also
   returns log(Gamma(a)) as gln.  					*/
/* ==================================================================== */

   void gcf(double *gammcf, double a, double x, double *gln) 
      { 
         const double ITMAX=100;
         const double EPS=3.0E-7;
         const double FPMIN=1E-30;
         
         double gammln(double xx); 
         void nrerror(char error_text[]); 
         int i; 
         double an,b,c,d,del,h; 
 
         *gln=gammln(a); 
         b=x+1.0-a; 
         c=1.0/FPMIN; 
         d=1.0/b; 
         h=d; 
         for (i=1;i<=ITMAX;i++) 
         { 
            an = -i*(i-a); 
            b += 2.0; 
            d=an*d+b; 
            if (fabs(d) < FPMIN) d=FPMIN; 
            c=b+an/c; 
            if (fabs(c) < FPMIN) c=FPMIN; 
            d=1.0/d; 
            del=d*c; 
            h *= del; 
            if (fabs(del-1.0) < EPS) break; 
         } 
         if (i > ITMAX) nrerror("a too large, ITMAX too small in gcf"); 
         *gammcf=exp(-x+a*log(x)-(*gln))*h; 
      } 

} // numrec namespace




