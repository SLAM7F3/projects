// ==========================================================================
// Program DECIMAL_NUMBERS synthesizes numerals containing 1 or 2
// digits after a decimal point as well as money numerals.
// ==========================================================================
// Last updated on 3/18/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
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

   int n_decimal_numbers = 5 * 1000;
   string text_files_subdir="./text_files/";
   string output_filename=text_files_subdir+"decimal_numbers.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);

   string whole_str, decimal_str, numeral_str;
   for(int i = 0; i < n_decimal_numbers; i++)
   {
      double ran_digits = nrfunc::ran1();
      int whole_digits;
      if(ran_digits < 0.3)
      {
         whole_digits = 10 * nrfunc::ran1();
      }
      else if (ran_digits < 0.6)
      {
         whole_digits = 100 * nrfunc::ran1();
      }
      else if (ran_digits < 0.9)
      {
         whole_digits = 1000 * nrfunc::ran1();
      }
      else
      {
         whole_digits = 10000 * nrfunc::ran1();
      }
      whole_str = stringfunc::number_to_string(whole_digits);

      int n_decimal_digits = basic_math::round(1+nrfunc::ran1());
      decimal_str = stringfunc::number_to_string(
         0.95*nrfunc::ran1(),n_decimal_digits);
      decimal_str = decimal_str.substr(1,decimal_str.size()-1);

      numeral_str = whole_str + decimal_str;
//      cout << whole_digits << "  " 
//           << whole_str << "  " 
//           << decimal_str << "  " << endl;

      if(nrfunc::ran1() < 0.1)
      {
         numeral_str = "$"+numeral_str;
      }
      else if (nrfunc::ran1() < 0.2)
      {
         numeral_str = "$ "+numeral_str;
      }
      outstream << numeral_str << endl;

   } // loop over index i labeling last names
   filefunc::closefile(output_filename, outstream);

   string banner="Exported synthesized phone numbers to "+output_filename;
   outputfunc::write_banner(banner);
}

