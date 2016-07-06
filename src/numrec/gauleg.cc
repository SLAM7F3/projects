// ====================================================================
// Given the lower and upper limits of integration x1 and x2 and given
// n, routine GAULEG returns arrays x[1..n] and w[1..n] containing the
// abscissas and weights of the Gauss-Legendre n-point quadrature
// formula.
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#include <math.h> 
 
void gauleg(double x1, double x2, double x[], double w[], int n) 
{ 
   const double EPS=3.0E-11;
   int m,j,i; 
   double z1,z,xm,xl,pp,p3,p2,p1; 
 
   m=(n+1)/2; 
   xm=0.5*(x2+x1); 
   xl=0.5*(x2-x1); 
   for (i=1;i<=m;i++) 
   { 
      z=cos(3.141592654*(i-0.25)/(n+0.5)); 
      do 
      { 
         p1=1.0; 
         p2=0.0; 
         for (j=1;j<=n;j++) 
         { 
            p3=p2; 
            p2=p1; 
            p1=((2.0*j-1.0)*z*p2-(j-1.0)*p3)/j; 
         } 
         pp=n*(z*p1-p2)/(z*z-1.0); 
         z1=z; 
         z=z1-p1/pp; 
      }
      while (fabs(z-z1) > EPS); 
      x[i]=xm-xl*z; 
      x[n+1-i]=xm+xl*z; 
      w[i]=2.0*xl/((1.0-z*z)*pp*pp); 
      w[n+1-i]=w[i]; 
   } 
} 

