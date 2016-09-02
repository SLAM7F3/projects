// =====================================================================
// Quicksort.  A simple explanation of the tricky partition step in the 
// Quicksort algorithm is provided in khanacademy.org -> linear-time
// partitioning.
// =====================================================================
// Last updated on 2/5/16; 2/10/16
// =====================================================================

#include <iostream>
#include <vector>
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"

using std::cout;
using std::endl;
using std::vector;

void swap(int& i, int& j)
{
   int tmp = j;
   j = i;
   i = tmp;
}

// Note:  Khan academy's "q" index = "leftwall" here.  

// Elements A[low] thru A[leftwall - 1] are group "L" consisting of
// elements known to be less than or equal to the pivot.

// Elements A[leftwall] thru A[j-1] are group "G" consisting of
// elements known to be greater than the pivot.

// Elements A[j] thru A[high] are group "U" consisting of elements
// whose relationship to the pivot is currently unknown because they have
// not yet been compared.

int Partition(vector<int>& A, int low, int high)
{

// Choose rightmost element of subarray A[high] as pivot:

   int pivot = A[high];

// Initialize indices leftwall and j to low:

   int leftwall = low;
   for (int j = low; j < high; j++)
   {

// At each step, compare A[j] (= leftmost element in currently
// unexplored group U) with the pivot.  If A[j] is greater than the
// pivot, increment j.  The line dividing groups G and U then slides
// over one position to the right.

// If A[j] is less than or equal to the pivot, swap A[j] with
// A[leftwall] (the leftmost element in group G), and increment j.  We
// thereby slide the line dividing groups L and G and the line
// dividing groups G and U over one position to the right.

      if(A[j] <= pivot)
      {
         swap(A[j], A[leftwall]);
         leftwall++;
      }
   }

// Once we get to the pivot, group U is empty.  Swap the pivot with
// the leftmost element in group G.  This final swap puts the pivot
// between groups L and G.  

   swap(A[high], A[leftwall]);

// Return index leftwall so that calling Quicksort function knows
// where the parittions are:

   return leftwall;
}

void Quicksort(vector<int>& A, int low, int high)
{
   if(low < high)
   {
      int pivot_location = Partition(A, low, high);
      Quicksort(A, low, pivot_location - 1);
      Quicksort(A, pivot_location + 1, high);
   }
}


int main()
{
   nrfunc::init_time_based_seed();

   vector<int> sequence = mathfunc::random_sequence(100);
   int n_items = 10;
   for(int i = 0; i < n_items; i++)
   {
      cout << sequence[i] << endl;
   }
   
   Quicksort(sequence, 0, n_items-1);

   cout << "After Quicksorting:" << endl;
   for(int i = 0; i < n_items; i++)
   {
      cout << sequence[i] << endl;
   }
}


