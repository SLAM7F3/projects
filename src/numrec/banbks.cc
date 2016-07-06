/* ==================================================================== */
/* Given the arrays a, a1 and indx as returned from subroutine BANDEC,
   and given a right-hand side vector b[1..n], subroutine BANBKS solves
   the band diagonal linear equations A x = b.  The solution vector x 
   overwrites b[1..n].  The other input arrays are not modified, and 
   can be left in place for successive calls with different right-hand 
   sides.								*/
/* ==================================================================== */
/* Last modified on 10/28/00                                            */
/* ==================================================================== */

namespace numrec
{

#define SWAP(a,b) {dum=(a);(a)=(b);(b)=dum;} 
 
   void banbks(double **a, unsigned long n, int m1, int m2, double **al, 
               unsigned long indx[], double b[]) 

      { 
         unsigned long i,k,l; 
         int mm; 
         double dum; 
 
         mm=m1+m2+1; 
         l=m1; 
         for (k=1;k<=n;k++) 
         { 
            i=indx[k]; 
            if (i != k) SWAP(b[k],b[i]) 
                           if (l < n) l++; 
            for (i=k+1;i<=l;i++) b[i] -= al[k][i-k]*b[k]; 
         } 
         l=1; 
         for (i=n;i>=1;i--) 
         { 
            dum=b[i]; 
            for (k=2;k<=l;k++) dum -= a[i][k]*b[k+i-1]; 
            b[i]=dum/a[i][1]; 
            if (l < mm) l++; 
         } 
      } 
#undef SWAP 

} // numrec namespace









