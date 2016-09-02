// =====================================================================
// Recursive powers
// =====================================================================
// Last updated on 2/10/16
// =====================================================================

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int compute_power(int base, int power)
{
   if(power == 0)
   {
      return 1;
   }
   else if (power%2 == 0)
   {
      int half_power = compute_power(base, power/2);
      return half_power * half_power;
   }
   else 
   {
      return base * compute_power(base, power - 1);
   }
}


int main()
{
   int power;
   cout << "Enter power:" << endl;
   cin >> power;

   int base = 2;
   cout << "base = " << base 
        << "  base ** power = " << compute_power(base,power) << endl;
}


