/* ==================================================================== */
/* Routine QROMB returns the integral of the function func from a to b.
   Integration is performed by Romberg's method of order 2k, where
   e.g. k=2 is Simpson's rule.						*/
/* ==================================================================== */
/* Last modified on 9/11/98.                                            */
/* ==================================================================== */

#include <math.h> 

// #define EPS 1.0e-6
#define JMAX 200   // JMAX limits the total number of steps
#define JMAXP (JMAX+1) 
#define K 5 	   // K = number of points used in the extrapolation
 
// EPS = fractional accuracy desired, as determined by the
// extrapolation error estimate.  We've modified routine QROMB so that we
// pass this tolerance level as a parameter to the routine.

double qromb(double (*func)(double), double a, double b, double EPS) 
{ 
    void polint(double xa[], double ya[], int n, double x, double *y, 
	        double *dy); 
    double trapzd(double (*func)(double), double a, double b, int n); 
    void nrerror(char error_text[]); 
    double ss,dss; 
    double s[JMAXP+1],h[JMAXP+1]; 
    int j; 
 
    h[1]=1.0; 
    for (j=1;j<=JMAX;j++) 
    { 
       s[j]=trapzd(func,a,b,j); 
        if (j >= K) 
        { 
            polint(&h[j-K],&s[j-K],K,0.0,&ss,&dss); 

// Note: We changed the < sign in the following line to <= so that
// QROMB doesn't go into an infinite loop when the value of the integral
// equals zero.

            if (fabs(dss) <= EPS*fabs(ss)) return ss; 
        } 
        s[j+1]=s[j]; 
        h[j+1]=0.25*h[j]; 
    } 
    nrerror("Too many steps in routine qromb"); 
    return 0.0; 
} 
//#undef EPS 
#undef JMAX 
#undef JMAXP 
#undef K 




