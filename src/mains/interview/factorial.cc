// =====================================================================
// Recursive factorial
// =====================================================================
// Last updated on 2/10/16
// =====================================================================

#include <algorithm>
#include <iostream>
#include <vector>
#include "math/mathfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

int factorial(int n)
{
   if (n== 0)
   {
      return 1;
   }
   else
   {
      return n * factorial(n-1);
   }
}


int main()
{
   int n;
   cout << "Enter n:" << endl;
   cin >> n;

   int n_factorial = factorial(n);
   cout << "n! = " << n_factorial << endl;
}


