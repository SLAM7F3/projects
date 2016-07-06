/* ==================================================================== */
/* Routine CONVLV convolves or deconvolves a real data set data [1..n]
  (including any user-supplied zero padding) with a response function
  respns[1..n].  The response function must be stored in wrap around
  order in the first m elements of respns, where m is an odd integer <= n.
  Wrap around order means that the first half of the array respns contains
  the impulse response function at positive times, while the second half of
  the array contains the impulse response function at negative times,
  counting down from the highest element respns[m].  On input, isign
  is +1 for convolution and -1 for deconvolution.  The answer is returned
  in the first n components of ans.  However, ans must be supplied in the
  calling program with dimension [1..2*n], for consistency with
  twofft.  n MUST be an integer power of two.				*/
/* ==================================================================== */
/* Last modified on 10/12/00                                            */
/* ==================================================================== */

#define NRANSI 
#include "numrec/nrutil.h" 
 
namespace numrec
{
   void convlv(double data[], unsigned long n, double respns[], 
               unsigned long m, int isign, double ans[]) 
      { 
         void realft(double data[], unsigned long n, int isign); 
         void twofft(double data1[], double data2[], double fft1[], 
                     double fft2[], 
                     unsigned long n); 
         unsigned long i,no2; 
         double dum,mag2,*fft; 
 
         fft=vector(1,n<<1); 

         for (i=1;i<=(m-1)/2;i++) 
         {
            respns[n+1-i]=respns[m+1-i]; 
         }
    
         for (i=(m+3)/2;i<=n-(m-1)/2;i++) 
         {
            respns[i]=0.0; 
         }
    
         twofft(data,respns,fft,ans,n); 
         no2=n>>1; 
         for (i=2;i<=n+2;i+=2) 
         { 
            if (isign == 1) 
            { 
               ans[i-1]=(fft[i-1]*(dum=ans[i-1])-fft[i]*ans[i])/no2; 
               ans[i]=(fft[i]*dum+fft[i-1]*ans[i])/no2; 
            }
            else if (isign == -1) 
            { 
               if ((mag2=SQR(ans[i-1])+SQR(ans[i])) == 0.0) 
                  nrerror( (char *) "Deconvolving at response zero in convlv"); 
               ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/mag2/no2; 
               ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/mag2/no2; 
            }
            else nrerror( (char *) "No meaning for isign in convlv"); 
         } 
         ans[2]=ans[n+1]; 
         realft(ans,n,-1); 
         free_vector(fft,1,n<<1); 
      } 
} // numrec namespace

#undef NRANSI 










