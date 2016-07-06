// ====================================================================
// Subroutine SORT2 sorts an array arr[1..n] into ascending order
// using Quicksort, while making the corresponding rearrangement of
// the array brr[1..n].
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#define NRANSI 
#include "numrec/nrutil.h" 
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp; 
 
void sort2(unsigned long n, double arr[], double brr[]) 
{ 
   const int M=7;
   const int NSTACK=50;
   
   unsigned long i,ir=n,j,k,l=1; 
   int *istack,jstack=0; 
   double a,b,temp; 
 
   istack=ivector(1,NSTACK); 
   for (;;) 
   { 
      if (ir-l < M) 
      { 
         for (j=l+1;j<=ir;j++) 
         { 
            a=arr[j]; 
            b=brr[j]; 
            for (i=j-1;i>=1;i--) 
            { 
               if (arr[i] <= a) break; 
               arr[i+1]=arr[i]; 
               brr[i+1]=brr[i]; 
            } 
            arr[i+1]=a; 
            brr[i+1]=b; 
         } 
         if (!jstack) 
         { 
            free_ivector(istack,1,NSTACK); 
            return; 
         } 
         ir=istack[jstack]; 
         l=istack[jstack-1]; 
         jstack -= 2; 
      }
      else 
      { 
         k=(l+ir) >> 1; 
         SWAP(arr[k],arr[l+1]) 
            SWAP(brr[k],brr[l+1]) 
            if (arr[l+1] > arr[ir]) 
            { 
               SWAP(arr[l+1],arr[ir]) 
                  SWAP(brr[l+1],brr[ir]) 
                  } 
         if (arr[l] > arr[ir]) 
         { 
            SWAP(arr[l],arr[ir]) 
               SWAP(brr[l],brr[ir]) 
               } 
         if (arr[l+1] > arr[l]) 
         { 
            SWAP(arr[l+1],arr[l]) 
               SWAP(brr[l+1],brr[l]) 
               } 
         i=l+1; 
         j=ir; 
         a=arr[l]; 
         b=brr[l]; 
         for (;;) 
         { 
            do i++; while (arr[i] < a); 
            do j--; while (arr[j] > a); 
            if (j < i) break; 
            SWAP(arr[i],arr[j]) 
               SWAP(brr[i],brr[j]) 
               } 
         arr[l]=arr[j]; 
         arr[j]=a; 
         brr[l]=brr[j]; 
         brr[j]=b; 
         jstack += 2; 
         if (jstack > NSTACK) nrerror("NSTACK too small in sort2."); 
         if (ir-i+1 >= j-l) 
         { 
            istack[jstack]=ir; 
            istack[jstack-1]=i; 
            ir=j-1; 
         }
         else 
         { 
            istack[jstack]=j-1; 
            istack[jstack-1]=l; 
            l=i; 
         } 
      } 
   } 
} 
#undef SWAP 
#undef NRANSI 

