// ====================================================================
// Subroutine SORT sorts an array arr[1..n] into ascending numerical
// order using the Quicksort algorithm.  n is input.  arr is replaced
// on output by its sorted rearrangement.
// ====================================================================
// Last modified on 8/12/03
// ====================================================================

#define NRANSI 
#include "numrec/nrutil.h" 
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp; 
 
void sort(unsigned long n, double arr[]) 
{ 
   const int M=7;
   const int NSTACK=50;
   
    unsigned long i,ir=n,j,k,l=1; 
    int jstack=0,*istack; 
    double a,temp; 
 
    istack=ivector(1,NSTACK); 
    for (;;) 
    { 
        if (ir-l < M) 
        { 
            for (j=l+1;j<=ir;j++) 
            { 
                a=arr[j]; 
                for (i=j-1;i>=1;i--) 
                { 
                    if (arr[i] <= a) break; 
                    arr[i+1]=arr[i]; 
                } 
                arr[i+1]=a; 
            } 
            if (jstack == 0) break; 
            ir=istack[jstack--]; 
            l=istack[jstack--]; 
        }
        else 
        { 
            k=(l+ir) >> 1; 
            SWAP(arr[k],arr[l+1]) 
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
                SWAP(arr[i],arr[j]); 
            } 
            arr[l]=arr[j]; 
            arr[j]=a; 
            jstack += 2; 
            if (jstack > NSTACK) nrerror("NSTACK too small in sort."); 
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
    free_ivector(istack,1,NSTACK); 
} 
#undef SWAP 
#undef NRANSI 

