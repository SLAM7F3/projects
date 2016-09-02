// =====================================================================
// Recursively determine if a string is a palindrome
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

bool is_palindrome(string str)
{
//   cout << "str = " << str << endl;

// If string has 0 or 1 characters, it is a palindrome:

   if(str.size() <= 1)
   {
      return true;
   }

// If first and last characters inside string are different, string is
// not a palindrome:

   else if (str[0] != str[str.size()-1])
   {
      return false;
   }
   else
   {

// Strip first and last characters from string.  Then determine if
// substring is a palindrome:

      string substr = str.substr(1,str.size() - 2);
//      cout << "substr = " << substr << endl;
      return is_palindrome(substr);
   }
}

int main()
{
   string input_string;
   cout << "Enter string:" << endl;
   cin >> input_string;

   if(is_palindrome(input_string))
   {
      cout << "Input string is a palindrome" << endl;
   }
   else
   {
      cout << "Input string is NOT a palindrome" << endl;
   }
}


