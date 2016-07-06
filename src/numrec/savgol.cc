/* ==================================================================== */
/* Subroutine SAVGOL returns in c[1..np], in wrap-around order (!)
which is consistent with the argument respns in routine CONVLV, a set
of Savitzky-Golay filter coefficients.  nl= number of leftward (past)
data points used; nr=number of rightward (future) data points.  The
total number of data points used nl+nr+1.  ld is the order of the
derivative desired (e.g. ld=0 for smoothed function).  m is the order
of the smoothing polynomial, which also equals the highest conserved
moment.  Typical values are m=2 or m=4.  				*/
/* ==================================================================== */
/* Last modified on 7/30/01                                             */
/* ==================================================================== */

#include <math.h> 
#define NRANSI 
#include "numrec/nrutil.h" 
 
namespace numrec
{
   void savgol(double c[], int np, int nl, int nr, int ld, int m) 
      { 
         void lubksb(double **a, int n, int *indx, double b[]); 
         void ludcmp(double **a, int n, int *indx, double *d); 
         int imj,ipj,j,k,kk,mm,*indx; 
         double d,fac,sum,**a,*b; 
 
         if (np < nl+nr+1 || nl < 0 || nr < 0 || ld > m || nl+nr < m) 
            nrerror((char *) "bad args in savgol"); 
         indx=ivector(1,m+1); 
         a=matrix(1,m+1,1,m+1); 
         b=vector(1,m+1); 
         for (ipj=0;ipj<=(m << 1);ipj++) 
         { 
            sum=(ipj ? 0.0 : 1.0); 
            for (k=1;k<=nr;k++) sum += pow((double)k,(double)ipj); 
            for (k=1;k<=nl;k++) sum += pow((double)-k,(double)ipj); 
            mm=FMIN(ipj,2*m-ipj); 
            for (imj = -mm;imj<=mm;imj+=2) a[1+(ipj+imj)/2][1+(ipj-imj)/2]=
                                              sum; 
         } 
         ludcmp(a,m+1,indx,&d); 
         for (j=1;j<=m+1;j++) b[j]=0.0; 
         b[ld+1]=1.0; 
         lubksb(a,m+1,indx,b); 
         for (kk=1;kk<=np;kk++) c[kk]=0.0; 
         for (k = -nl;k<=nr;k++) 
         { 
            sum=b[1]; 
            fac=1.0; 
            for (mm=1;mm<=m;mm++) sum += b[mm+1]*(fac *= k); 
            kk=((np-k) % np)+1; 
            c[kk]=sum; 
         } 
         free_vector(b,1,m+1); 
         free_matrix(a,1,m+1,1,m+1); 
         free_ivector(indx,1,m+1); 
      } 
} // numrec namespace

#undef NRANSI 
