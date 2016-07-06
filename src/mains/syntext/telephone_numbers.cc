// ==========================================================================
// Program TELEPHONE_NUMBERS synthesizes 10 or 7 digit telephone
// numbers which randomly include parentheses and hyphens.
// ==========================================================================
// Last updated on 3/16/16
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

   int n_phone_numbers = 10 * 1000;
   string text_files_subdir="./text_files/";
   string output_filename=text_files_subdir+"phone_numbers.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);

   for(int i = 0; i < n_phone_numbers; i++)
   {
      if(nrfunc::ran1() < 0.66)
      {
         // Prepend area code

         bool insert_parentheses = false;
         if(nrfunc::ran1() < 0.75)
         {
            insert_parentheses = true;
            outstream << "(";
         }

         for(int d = 0; d < 3; d++)
         {
            int curr_digit = 9.99999 * nrfunc::ran1();
            outstream << curr_digit;
         }
         if(insert_parentheses)
         {
            outstream << ")";
         }
         outstream << " ";
      } 


      for(int d = 0; d < 3; d++)
      {
         int curr_digit = 9.99999 * nrfunc::ran1();
         outstream << curr_digit;
      }

      if(nrfunc::ran1() < 0.25)
      {
         outstream << " ";
      }
      else
      {
         outstream << "-";
      }
      

      for(int d = 0; d < 4; d++)
      {
         int curr_digit = 9.99999 * nrfunc::ran1();
         outstream << curr_digit;
      }
      outstream << endl;
   } // loop over index i labeling last names
   filefunc::closefile(output_filename, outstream);

   string banner="Exported synthesized phone numbers to "+output_filename;
   outputfunc::write_banner(banner);
}

