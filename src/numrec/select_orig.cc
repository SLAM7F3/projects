/* ==================================================================== */
/* Subroutine SELECT returns the kth smallest value in the array
   arr[1..n].  The input array will be rearranged to have this value
   in location arr[k], with all smaller elements moved to arr[1..k-1]
   (in arbitrary order) and all larger elements in arr[k+1..n) (also
   in arbitrary order).							*/
/* ==================================================================== */
/* Last modified on 2/28/98.                                            */
/* ==================================================================== */

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp; 
 
double select_nr(unsigned long k, unsigned long n, double arr[]) 

{ 
    unsigned long i,ir,j,l,mid; 
    double a,temp; 
 
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
} 
#undef SWAP 

