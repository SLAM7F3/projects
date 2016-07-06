/* ==================================================================== */
/* Given the covariance matrix covar[1..ma][1..ma] of a fit for mfit of 
   ma total parameters and given their ordering lista[1..ma], routine
   COVSRT repacks the covariance matrix to the true order of the parameters
   Elements associated with fixed parameters will be zero.		*/
/* ==================================================================== */
/* Last modified on 11/10/98                                            */
/* ==================================================================== */

namespace numrec
{
   
#define SWAP(a,b) {swap=(a);(a)=(b);(b)=swap;} 

   void covsrt(double **covar, int ma, int ia[], int mfit) 
      { 
         int i,j,k; 
         double swap; 
 
         for (i=mfit+1;i<=ma;i++) 
            for (j=1;j<=i;j++) covar[i][j]=covar[j][i]=0.0; 
         k=mfit; 
         for (j=ma;j>=1;j--) 
         { 
            if (ia[j]) 
            { 
               for (i=1;i<=ma;i++) 
                  SWAP(covar[i][k],covar[i][j]) 
                     for (i=1;i<=ma;i++) SWAP(covar[k][i],covar[j][i]) 
                                            k--; 
            } 
         } 
      } 
#undef SWAP 
}






