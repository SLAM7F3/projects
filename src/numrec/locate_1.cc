/* ==================================================================== */
/* Given an array xx[1..n] and given a value x, routine locate returns
   a value j such that x is between xx[j] and xx[j+1].  xx must be 
   monotonic, either increasing or decreasing.  j=0 or j=n is returned to 
   indicate that x lies out or range.	       				

   Note: In the original version of this recipe, the parameters n and
   j were passed as unsigned long integers.  We have deleted the
   unsigned qualifier and simply pass these as long integers.  We have
   made the appropriate modification to the declaration statement within 
   numrec/nr.h									*/
/* ==================================================================== */
/* Last modified on 11/24/98.                                           */
/* ==================================================================== */

void locate(double xx[], long n, double x, long *j) 
{ 
    unsigned long ju,jm,jl; 
    int ascnd; 
 
    jl=0; 
    ju=n+1; 
    ascnd=(xx[n] > xx[1]); 
    while (ju-jl > 1) 
    { 
        jm=(ju+jl) >> 1; 
        if (x > xx[jm] == ascnd) 
            jl=jm; 
        else 
            ju=jm; 
    } 
    *j=jl; 
} 



