// ==========================================================================
// Program FIND_AND_REPLACE was created to help rapidly search for
// particular strings within multiple files.  It takes in a file
// containing a list of .cc and .h files (generated via the unix
// "find" command").  This program scans through each file within the
// list and searches for a particular string which is entered by the
// user (e.g. "nrfuncs.h").  It then replaces that string with
// [prepends] some user specified string [e.g. "numrec/"].  We used
// this program to move all .h files within the /include directory
// into specialized subdirectories of /include.  We then had to make
// corresponding modifications to #include statements within many .h
// and .cc files.  This program saved us from having to perform all
// these tedious modifications completely by hand.

// ==========================================================================
// Last updated on 6/4/06; 6/13/06
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
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
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string inputfilename="./allfiles.txt";
   vector<string> filename;
   filefunc::parameter_input(inputfilename,filename);

   string search_string,insert_string;
//   search_string="io/DataSetFile.h";
   cout << "Enter search string to be modified:" << endl;
   cin >> search_string;
//   insert_string="io/DataSetFile.h";
   cout << "Enter substring to replace search string:" << endl;
   cin >> insert_string;
   outputfunc::enter_continue_char();

   filefunc::text_line.reserve(10000);

   for (unsigned int j=0; j<filename.size(); j++)
   {
//      outputfunc::write_banner("Processing file "+filename[j]);
      filefunc::text_line.clear();

// Search current file and determine whether it contains the desired
// search string:
      
      if (filefunc::ReadInfile(filename[j],false))
      {
         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            string::size_type substr_pos=filefunc::text_line[i].find(
               search_string);
            if (substr_pos != string::npos)
            {
               cout << "File = " << filename[j] << endl;
               cout << "Search string found in line = " << i << endl;
               cout << "Original version of line[i] = " 
                    << filefunc::text_line[i] << endl;

               string prefix=filefunc::text_line[i].substr(
                  0,substr_pos);
//               cout << "prefix = " << prefix << endl;
               string suffix=filefunc::text_line[i].substr(
                  substr_pos+search_string.length(),
                  filefunc::text_line[i].length());
//               cout << "suffix = " << suffix << endl;
               string modified_line=prefix+insert_string+suffix;
               filefunc::text_line[i]=modified_line;

//               filefunc::text_line[i].insert(substr_pos,insert_string);

               cout << "New version of line[i] = " 
                    << filefunc::text_line[i] << endl;
            }
         } // loop over index i labeling line number within current file
      }
 
// Delete original file:

      string unixcommandstr="/bin/rm "+filename[j];
//      cout << "unixcommandstr = " << unixcommandstr << endl;
      sysfunc::unix_command(unixcommandstr);

// Regenerate new file possibly containing altered line:

      ofstream filestream;
      filefunc::openfile(filename[j],filestream);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         filestream << filefunc::text_line[i] << endl;
      }
      filefunc::closefile(filename[j],filestream);

   } // loop over index j labeling current file being processed
   
  
}
