/* ==================================================================== */
/* Subroutine SELECT returns the kth smallest value in the array
   arr[1..n].  We modified this routine on 2/27/01 so that the order
   of the input array arr is NOT disturbed by calling this subroutine!	*/
/* ==================================================================== */
/* Last modified on 2/10/03                                             */
/* ==================================================================== */

#include <math.h> 
#include "numrec/nrutil.h" 

namespace numrec
{
   
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp; 
 
   double select(unsigned long k, unsigned long n, double arr_input[]) 
      { 
         unsigned long i,ir,j,l,mid; 
         double a,temp; 
         double *arr;
         arr=vector(1,n);

// First copy all entries within arr_input array into working array
// arr whose order is messed up by this subroutine:
    
         for (i=1; i<=n; i++)
         {
            arr[i]=arr_input[i];
         }

         l=1; 
         ir=n; 
         for (;;) 
         { 
            if (ir <= l+1) 
            { 
               if (ir == l+1 && arr[ir] < arr[l]) 
               { 
                  SWAP(arr[l],arr[ir]) 
                     } 
               return arr[k]; 
            }
            else 
            { 
               mid=(l+ir) >> 1; 
               SWAP(arr[mid],arr[l+1]) 
                  if (arr[l+1] > arr[ir]) 
                  { 
                     SWAP(arr[l+1],arr[ir]) 
                        } 
               if (arr[l] > arr[ir]) 
               { 
                  SWAP(arr[l],arr[ir]) 
                     } 
               if (arr[l+1] > arr[l]) 
               { 
                  SWAP(arr[l+1],arr[l]) 
                     } 
               i=l+1; 
               j=ir; 
               a=arr[l]; 
               for (;;) 
               { 
                  do i++; while (arr[i] < a); 
                  do j--; while (arr[j] > a); 
                  if (j < i) break; 
                  SWAP(arr[i],arr[j]) 
                     } 
               arr[l]=arr[j]; 
               arr[j]=a; 
               if (j >= k) ir=j-1; 
               if (j <= k) l=i; 
            } 
         } 

         free_vector(arr,1,n);
      } 
#undef SWAP 

} // numrec namespace







