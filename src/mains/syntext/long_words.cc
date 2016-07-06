// ==========================================================================
// Program LONG_WORDS
// ==========================================================================
// Last updated on 3/29/12; 3/1/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   //   string input_filename="english.words";
   string input_filename="wordsEn.txt";
   
   filefunc::ReadInfile(input_filename);

//   const int min_letter_count=8;
   const int min_letter_count=1;
   vector<string> cleaned_strings;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[i]);
      for (unsigned int s=0; s<substrings.size(); s++)
      {
         string curr_string=substrings[s];
         int curr_string_length=curr_string.size();
         if (curr_string_length < min_letter_count) continue;

//         cout << substrings[s] << endl;
         string cleaned_substring="";
         for (int c=0; c<curr_string_length; c++)
         {
            char curr_char=stringfunc::string_to_char(curr_string.substr(c,1));
//            cout << "c = " << c << " char = " << curr_char << endl;
            bool char_OK=false;

            int ascii_int=stringfunc::char_to_ascii_integer(curr_char);
            if (ascii_int >= 48 && ascii_int <=57) char_OK=true;
            if (ascii_int >= 65 && ascii_int <=90) char_OK=true;
            if (ascii_int >= 97 && ascii_int <=122) char_OK=true;
            if (!char_OK) continue;
            cleaned_substring += stringfunc::char_to_string(curr_char);
         }
         int cleaned_string_length=cleaned_substring.size();
         if (cleaned_string_length < min_letter_count) continue;

         cleaned_strings.push_back(cleaned_substring);
      }
   }


   cout << "cleaned_strings.size() = " << cleaned_strings.size() << endl;

   string output_filename="english_words.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   for (unsigned int i=0; i<cleaned_strings.size(); i++)
   {
      outstream << cleaned_strings[i] << endl;
   }
   filefunc::closefile(output_filename,outstream);
   
   
} 

