// ==========================================================================
// Program COMPANY_NAMES imports a set of company names from a text
// file.  It removes all leading and trailing white space from the
// names.  It also removes any apostrophes occuring within any company
// name.  The cleaned company names are exported to another text file.
// ==========================================================================
// Last updated on 3/5/16; 3/13/16; 3/16/16; 3/21/16
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
   string input_filename=text_files_subdir+"companies.txt";
   filefunc::ReadInfile(input_filename);
   vector<string> company_names;
   
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_name = filefunc::text_line[i];
      company_names.push_back(curr_name);
   }
   int n_company_names=company_names.size();


   string output_filename=text_files_subdir+"company_names.txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);

   string separator_chars="'";
   for(int i = 0; i < n_company_names; i++)
   {
      string curr_name = company_names[i];
      curr_name = stringfunc::remove_leading_whitespace(curr_name);
      curr_name = stringfunc::remove_trailing_whitespace(curr_name);

      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_name, separator_chars);
      int n_substrings = substrings.size();
      curr_name="";
      for(int s = 0; s < n_substrings; s++)
      {
         curr_name += substrings[s];
      }

      outstream << curr_name << endl;
   } // loop over index i labeling last names
   filefunc::closefile(output_filename, outstream);

   string banner="Exported cleaned company names to "+output_filename;
   outputfunc::write_banner(banner);
}

