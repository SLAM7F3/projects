// =====================================================================
// Binary search of sorted array.
// =====================================================================
// Last updated on 2/10/16
// =====================================================================

#include <algorithm>
#include <iostream>
#include <vector>
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

int binary_search(int search_value, vector<int>& A, 
                  int& min_index, int& max_index)
{
   if(search_value < A[min_index])
   {
      return -1;    
   }
   else if (search_value > A[max_index])
   {
      return -1;
   }

   int curr_index = 0.5 * (min_index + max_index);
   cout << "curr_index = " << curr_index << endl;
   if(A[curr_index] == search_value)
   {
      return curr_index;
   }
   else if (A[curr_index] < search_value)
   {
      min_index = curr_index + 1;
      return binary_search(search_value, A, min_index, max_index);
   }
   else // if (A[curr_index] > search_value)
   {
      max_index = curr_index - 1;
      return binary_search(search_value, A, min_index, max_index);
   }
}


int main()
{
   nrfunc::init_time_based_seed();

   vector<int> init_sequence = mathfunc::random_sequence(100);
   vector<int> sequence;
   
   int n_items = 30;
   for(int i = 0; i < n_items; i++)
   {
      sequence.push_back(init_sequence[i]);
      cout << sequence[i] << endl;
   }
   
   std::sort(sequence.begin(), sequence.end());

   cout << "After STL sorting:" << endl;
   for(int i = 0; i < n_items; i++)
   {
      cout << sequence[i] << endl;
   }

   int search_value;
   cout << "Enter value within array to search for:" << endl;
   cin >> search_value;
   
   int min_index = 0;
   int max_index = sequence.size() - 1;
   int search_value_index = binary_search(
      search_value, sequence, min_index, max_index);
   if(search_value_index == -1)
   {
      cout << "Search value does not lie within sequence sequence"
           << endl;
   }
   else
   {
      cout << "Search value is located at index " << search_value_index
           << endl;

   }
}


