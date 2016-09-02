// ==========================================================================
// Compress string such as 'AAABBCCCCCCAAAAA' to '3A2B6C5A'  

// See https://leetcode.com/problemset/algorithms/ for similar
// interview problems.
// ==========================================================================
// Last updated on 9/2/16
// ==========================================================================

#include <iostream>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

string num_to_string(int x)
{
   char buffer[100];
   sprintf(buffer, "%d", x);
   return string(buffer);
}

// ==========================================================================

int main()
{
   string orig_phrase = "AAABBCCCCCCAAAAA";
   string compressed_phrase = "";

   char curr_char = orig_phrase[0];
   int curr_char_counter = 0;

   for(unsigned int i = 0; i < orig_phrase.size(); i++)
   {
      if(orig_phrase[i] == curr_char)
      {
         curr_char_counter++;
      }
      else
      {
         compressed_phrase += num_to_string(curr_char_counter)+curr_char;
         curr_char = orig_phrase[i];
         curr_char_counter = 1;
      }
   } // loop over index i 
   compressed_phrase += num_to_string(curr_char_counter) + curr_char;

   cout << "compressed_phrase = " << compressed_phrase << endl;
}

