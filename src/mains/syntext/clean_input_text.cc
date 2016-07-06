// ==========================================================================
// Program CLEAN_INPUT_TEXT imports the context of
// ./text_files/all_text.txt.  It scans through every character within
// the input file and rejects any whose ascii value does not lie
// within the printable interval [32, 126].  CLEAN_INPUT_TEXT 
// shuffles and exports the cleaned text lines to 
// ./text_files/shuffled_all_text.txt".
// ==========================================================================
// Last updated on 3/22/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string text_files_subdir="./text_files/";
   string input_filename=text_files_subdir+"all_text.txt";
   filefunc::ReadInfile(input_filename);

   string output_filename=text_files_subdir+"cleaned_all_text.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   int n_bad_chars = 0;
   const int min_ascii_value = 32;     
   const int max_ascii_value = 126;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line = filefunc::text_line[i];
      vector<int> ascii_values = stringfunc::decompose_string_to_ascii_rep(
         curr_line);
      string cleaned_line="";
      for(unsigned int c = 0; c < ascii_values.size(); c++)
      {
         if(ascii_values[c] < min_ascii_value || ascii_values[c] > 
            max_ascii_value)
         {
//            cout << "i = " << i << " c = " << c 
//                 << " ascii_value = " << ascii_values[c] << endl;
//            outputfunc::enter_continue_char();
            n_bad_chars++;
            continue;
         }
         char curr_char = stringfunc::ascii_integer_to_char(
            ascii_values[c]);
      cleaned_line += stringfunc::char_to_string(curr_char);
//      cleaned_line += string(curr_char);
      }
      outstream << cleaned_line << endl;
   } // loop over index i labeling text lines
   filefunc::closefile(output_filename,outstream);

   cout << "n_bad_chars = " << n_bad_chars << endl;

   string unix_cmd="shuf "+output_filename+" > "+
      text_files_subdir+"shuffled_all_text.txt";
   sysfunc::unix_command(unix_cmd);

   string banner="Exported cleaned text lines to "+output_filename;
   outputfunc::write_banner(banner);
   banner="Exported shuffled cleaned text lines to "+
      text_files_subdir+"shuffled_all_text.txt";
   outputfunc::write_banner(banner);
}

