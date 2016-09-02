// ==========================================================================
// Reverse the order of words in a phrase
// ==========================================================================
// Last updated on 9/2/16
// ==========================================================================

#include <iostream>
#include <vector>
#include "math/mathfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================

int main()
{
   string orig_phrase = "Nvidia 3D graphics";
   cout << "orig_phrase = " << orig_phrase << endl;

   bool parsing_curr_word = false;
   vector<int> word_start_posn, word_stop_posn;
   for(unsigned int i = 0; i < orig_phrase.size(); i++)
   {
      char curr_char = orig_phrase[i];

      if(!parsing_curr_word && curr_char != ' ')
      {
         word_start_posn.push_back(i);
         parsing_curr_word = true;
//         cout << "word_start_posn = " << word_start_posn.back() << endl;
      }

      if(i == orig_phrase.size() - 1 || orig_phrase[i+1] == ' ')
      {
         word_stop_posn.push_back(i);
         parsing_curr_word = false;
//         cout << "word_stop_posn = " << word_stop_posn.back() << endl;
      }
   }
  
   string reversed_phrase = "";
   for(int w = int(word_start_posn.size()) - 1; w >= 0; w--)
   {
      for(int i = word_start_posn[w]; i <= word_stop_posn[w]; i++)
      {
         reversed_phrase += orig_phrase[i];
      }

      if(w > 0)
      {
         reversed_phrase += " ";
      }
   }
   cout << "Final reversed_phrase = " << reversed_phrase << endl;
}

