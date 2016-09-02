// ==========================================================================
// Given an array of integers, write an algorithm that brings all the 0
// elements to the end of the array and returns the # of non-zero
// elements.

// See https://leetcode.com/problemset/algorithms/ for similar
// interview problems.

// ==========================================================================
// Last updated on 9/2/16
// ==========================================================================

#include <iostream>
#include <vector>
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================

int main()
{
   nrfunc::init_time_based_seed();

   int n_items = 10;
   vector<int> int_array, new_int_array;
   for(int i = 0; i < n_items; i++)
   {
      double curr_ran =  nrfunc::ran1();
      double threshold = 0.15;
      int curr_int = 0;
      if(curr_ran > threshold)
      {
         curr_int = curr_ran /threshold;
      }
      int_array.push_back(curr_int);
      cout << "i = " << i 
           << " curr_int = " << curr_int << endl;

   } // loop over index i 

   for(unsigned int i = 0; i < int_array.size(); i++)
   {
      if(int_array[i] > 0)
      {
         new_int_array.push_back(int_array[i]);
      }
   }
   int n_nonzeros = new_int_array.size();
   int n_zeros = n_items - n_nonzeros;
   for(unsigned int i = 0; i < n_zeros; i++)
   {
      new_int_array.push_back(0);
   }
   
   cout << "Number of non-zeros = " << n_nonzeros << endl;
   cout << "Number of zeros = " << n_zeros << endl;
   for(unsigned int i = 0; i < new_int_array.size(); i++)
   {
      cout << "i = " << i << " new_int = " << new_int_array[i]
           << endl;
   }
   
}

