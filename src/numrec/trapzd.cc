/* ==================================================================== */
/* Routine TRAPZD computes the n'th stage of refinement of an extended 
   trapezoidal rule.  func is input as a pointer to the function to be 
   integrated between limits a and b.  When called with n=1,the routine 
   returns the crudest estimate of int_a^b f(x) dx.  Subsequent calls with
   n=2,3,... (in that sequential order) will improve the accuracy by adding
   2**(n-2) additional interior points.					*/
/* ==================================================================== */
/* Last modified on 9/11/98.                                            */
/* ==================================================================== */

#define FUNC(x) ((*func)(x)) 
 
double trapzd(double (*func)(double), double a, double b, int n) 

{ 
    double x,tnm,sum,del; 
    static double s; 
    int it,j; 
 
    if (n == 1) 
    { 
        return (s=0.5*(b-a)*(FUNC(a)+FUNC(b))); 
    }
    else 
    { 
        for (it=1,j=1;j<n-1;j++) it <<= 1; 
        tnm=it; 
        del=(b-a)/tnm; 
        x=a+0.5*del; 
        for (sum=0.0,j=1;j<=it;j++,x+=del) sum += FUNC(x); 
        s=0.5*(s+(b-a)*sum/tnm); 
        return s; 
    } 
} 
#undef FUNC 

