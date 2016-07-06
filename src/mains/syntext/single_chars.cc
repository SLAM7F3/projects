// ==========================================================================
// Program SINGLE_CHARS writes multiple copies of upper and lower case
// letters to an output text file.
// ==========================================================================
// Last updated on 4/7/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
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

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   string text_files_subdir="./text_files/";
   string output_filename=text_files_subdir+"single_chars.txt";
   
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);

   int max_iters = 30;
   int ascii_A = 65;
   int ascii_Z = 90;
   int ascii_a = 97;
   int ascii_z = 122;
   for(int iter = 0; iter < max_iters; iter++)
   {
      for(int c = ascii_A; c <= ascii_Z; c++)
      {
         outstream << stringfunc::ascii_integer_to_char(c) << endl;
      }
      for(int c = ascii_a; c <= ascii_z; c++)
      {
         outstream << stringfunc::ascii_integer_to_char(c) << endl;
      }
   }
   filefunc::closefile(output_filename, outstream);

   string banner="Exported single characters to "+output_filename;
   outputfunc::write_banner(banner);
}

