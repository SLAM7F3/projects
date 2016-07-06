// ==========================================================================
// Program CREATE_NUMBERS generates random numerals containing 1 - 6
// digits and exports them to an output text file.
// ==========================================================================
// Last updated on 4/9/14; 1/16/15; 2/27/16; 2/29/16
// ==========================================================================

//   c='0';	// ascii = 48
//   c='9';	// ascii = 57

//   c='A';	// ascii = 65
//   c='Z';	// ascii = 90

//   c='a';	// ascii = 97
//   c='z';	// ascii = 122

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::set;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

   typedef set<int> INT_SET;
   INT_SET housenumbers_set;
   INT_SET::iterator hn_iter;

   string output_filename="house_numbers.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   int i = 0;
//   int n_numbers = 100 * 10;
   int n_numbers = 100 * 1000;
   int n_start, n_stop;
   while (i < n_numbers)
   {
      double random = nrfunc::ran1();
      if(random < 0.05)				// 1 digit
      {
         n_start = 0;
         n_stop = 10;
      }
      else if (random >= 0.05 && random < 0.2)	// 2 digits
      {
         n_start = 10;
         n_stop = 100;
      }
      else if (random >= 0.2 && random < 0.4)	// 3 digits
      {
         n_start = 100;
         n_stop = 1000;
      }
      else if (random >= 0.4 && random < 0.75)	// 4 digits
      {
         n_start = 1000;
         n_stop = 10000;
      }
      else if (random >= 0.75 && random < 0.95)  // 5 digits
      {
         n_start = 10000;
         n_stop = 100000;
      }
      else				        // 6 digits
      {
         n_start = 100000;
         n_stop = 1000000;
      }
      
      int dn=(n_stop - n_start) * nrfunc::ran1();
      int curr_n = n_start + dn;

      hn_iter = housenumbers_set.find(curr_n);
      if(hn_iter != housenumbers_set.end()) continue;

      if(curr_n >= 100)
      {
         housenumbers_set.insert(curr_n);
      }
      
      outstream << curr_n << endl;
      i++;
   } // loop over index i labeling output numbers

   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+stringfunc::number_to_string(n_numbers)+
      " random long integers to "+output_filename;
   outputfunc::write_banner(banner);
}
   

