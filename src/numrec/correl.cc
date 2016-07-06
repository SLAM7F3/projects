/* ==================================================================== */
/* Routine CORREL computes the correlation of two real data sets
   data1[1..n] and data2[1..n] (including any user-supplied zero
   padding).  n MUST be an integer power of two.  The answer is
   returned as the first n points in ans[1..2*n] stored in wrap-around
   order - i.e. correlations at increasingly negative lags are in
   ans[n] on down to ans[n/2+1], while correlations at increasingly
   positive lags are in ans[1] (zero lag) on up to ans[n/2].  Note
   that ans must be supplied in the calling program with length at
   least 2*n, since it is also used as working space.  Sign
   convention of this routine: if data1 lags data2, i.e. is shifted to
   the right of it, then ans will show a peak at positive lags.  */
/* ==================================================================== */
// Last modified on 7/15/03                                             
/* ==================================================================== */

#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <iomanip>
#include <math.h>

#define NRANSI 
#include "numrec/nrutil.h" 
 
namespace numrec
{
   void correl(double data1[], double data2[], unsigned long n, double ans[]) 
      { 
         void realft(double data[], unsigned long n, int isign); 
         void twofft(double data1[], double data2[], double fft1[], 
                     double fft2[], 
                     unsigned long n); 
         unsigned long no2,i; 
         double dum,*fft; 
 
         fft=vector(1,n<<1); 
         twofft(data1,data2,fft,ans,n); 
         no2=n>>1; 
         for (i=2;i<=n+2;i+=2) 
         { 
            ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/no2; 
            ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/no2; 
         } 
         ans[2]=ans[n+1]; 
         realft(ans,n,-1); 
         free_vector(fft,1,n<<1); 
      } 
} // numrec namespace

#undef NRANSI 







