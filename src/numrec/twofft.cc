/* ==================================================================== */
/* Given two real input arrays data1[1..n] and data2[1..n], routine 
   TWOFFT calls FOUR1 and returns two complex output arrays, fft1 and
   fft2, each of complex length n (i.e. real dimensions [1..2n]), which
   contain the discrete Fourier transforms of the respective datas.
   n MUST be an integer power of 2.					*/
/* ==================================================================== */
/* Last modified on 9/22/98.                                            */
/* ==================================================================== */

namespace numrec
{
   void twofft(double data1[], double data2[], double fft1[], double fft2[], 
               unsigned long n) 
      { 
         void four1(double data[], unsigned long nn, int isign); 
         unsigned long nn3,nn2,jj,j; 
         double rep,rem,aip,aim; 
 
         nn3=1+(nn2=2+n+n); 
         for (j=1,jj=2;j<=n;j++,jj+=2) 
         { 
            fft1[jj-1]=data1[j]; 
            fft1[jj]=data2[j]; 
         } 
         four1(fft1,n,1); 
         fft2[1]=fft1[2]; 
         fft1[2]=fft2[2]=0.0; 
         for (j=3;j<=n+1;j+=2) 
         { 
            rep=0.5*(fft1[j]+fft1[nn2-j]); 
            rem=0.5*(fft1[j]-fft1[nn2-j]); 
            aip=0.5*(fft1[j+1]+fft1[nn3-j]); 
            aim=0.5*(fft1[j+1]-fft1[nn3-j]); 
            fft1[j]=rep; 
            fft1[j+1]=aim; 
            fft1[nn2-j]=rep; 
            fft1[nn3-j] = -aim; 
            fft2[j]=aip; 
            fft2[j+1] = -rem; 
            fft2[nn2-j]=aip; 
            fft2[nn3-j]=rem; 
         } 
      } 
} // numrec namespace

