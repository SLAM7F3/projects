/* ==================================================================== */
/* Subroutine FOURN replaces data by its ndim-dimensional discrete
   Fourier transform, if isign is input as 1.  nn[1..ndim] is an integer
   array containing the lengths of each dimension (number of complex
   values), which MUST all be powers of 2.  Data is a real array of
   length twice the product of these lengths, in which the data are
   stored as in a multidimensional complex array: real and imaginary
   parts of each element are in consecutive locations, and the rightmost
   index of the array increases most rapidly as one proceeds along data.
   For a two-dimensional array, this is equivalent to storing the array
   by rows.  If isign is input as -1, data is replaced by its inverse
   transform times the product of the lengths of all dimensions.  		*/
/* ==================================================================== */
/* Last modified on 11/6/00.                                            */
/* ==================================================================== */

#include <math.h> 

namespace numrec
{
   
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr 
 
   void fourn(double data[], unsigned long nn[], int ndim, int isign) 
      { 
         int idim; 
         unsigned long i1,i2,i3,i2rev,i3rev,ip1,ip2,ip3,ifp1,ifp2; 
         unsigned long ibit,k1,k2,n,nprev,nrem,ntot; 
         double tempi,tempr; 
         double theta,wi,wpi,wpr,wr,wtemp; 
 
         for (ntot=1,idim=1;idim<=ndim;idim++) 
            ntot *= nn[idim]; 
         nprev=1; 
         for (idim=ndim;idim>=1;idim--) 
         { 
            n=nn[idim]; 
            nrem=ntot/(n*nprev); 
            ip1=nprev << 1; 
            ip2=ip1*n; 
            ip3=ip2*nrem; 
            i2rev=1; 
            for (i2=1;i2<=ip2;i2+=ip1) 
            { 
               if (i2 < i2rev) 
               { 
                  for (i1=i2;i1<=i2+ip1-2;i1+=2) 
                  { 
                     for (i3=i1;i3<=ip3;i3+=ip2) 
                     { 
                        i3rev=i2rev+i3-i2; 
                        SWAP(data[i3],data[i3rev]); 
                        SWAP(data[i3+1],data[i3rev+1]); 
                     } 
                  } 
               } 
               ibit=ip2 >> 1; 
               while (ibit >= ip1 && i2rev > ibit) 
               { 
                  i2rev -= ibit; 
                  ibit >>= 1; 
               } 
               i2rev += ibit; 
            } 
            ifp1=ip1; 
            while (ifp1 < ip2) 
            { 
               ifp2=ifp1 << 1; 
               theta=isign*6.28318530717959/(ifp2/ip1); 
               wtemp=sin(0.5*theta); 
               wpr = -2.0*wtemp*wtemp; 
               wpi=sin(theta); 
               wr=1.0; 
               wi=0.0; 
               for (i3=1;i3<=ifp1;i3+=ip1) 
               { 
                  for (i1=i3;i1<=i3+ip1-2;i1+=2) 
                  { 
                     for (i2=i1;i2<=ip3;i2+=ifp2) 
                     { 
                        k1=i2; 
                        k2=k1+ifp1; 
                        tempr=(double)wr*data[k2]-(double)wi*data[k2+1]; 
                        tempi=(double)wr*data[k2+1]+(double)wi*data[k2]; 
                        data[k2]=data[k1]-tempr; 
                        data[k2+1]=data[k1+1]-tempi; 
                        data[k1] += tempr; 
                        data[k1+1] += tempi; 
                     } 
                  } 
                  wr=(wtemp=wr)*wpr-wi*wpi+wr; 
                  wi=wi*wpr+wtemp*wpi+wi; 
               } 
               ifp1=ifp2; 
            } 
            nprev *= n; 
         } 
      } 
#undef SWAP 

} // numrec namespace



