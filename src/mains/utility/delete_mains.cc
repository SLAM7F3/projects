// ==========================================================================
// Program DELETE_MAINS takes in the name for some "build" script (e.g. 
// build_aerialEO).  It creates a new "anti build" script
// (e.g. build_anti_aerialEO) which contains /bin/rm unix commands
// corresponding to all of the make commands within the build script.  
// ==========================================================================
// Last updated on 12/29/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <sys/stat.h>   // Needed for stat() Unix system call
#include <vector>
#include <curl/curl.h>

#include "general/filefuncs.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"


using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

//   cout << "argc = " << argc << endl;
   string build_filename=argv[1];
   cout << "build_filename = " << build_filename << endl;
//   string build_filename="build_OSG";

   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      build_filename,"_");
   string delete_filename="build_anti_"+substrings.back();
   cout << "Delete_filename = " << delete_filename << endl;

   ofstream outstream;
   filefunc::openfile(delete_filename,outstream);


   filefunc::ReadInfile(build_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string make_line=filefunc::text_line[i];
//      cout << make_line << endl;

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         make_line,"=");
      
//      string substring=stringfunc::erase_chars_before_first_substring(
//         make_line,"=");
      string substring=substrings.back();
//      cout << substring << endl;
      string unix_cmd="/bin/rm ./"+substring;
      outstream << unix_cmd << endl;
   }
   
   filefunc::closefile(delete_filename,outstream);
   string unix_cmd="chmod a+x "+delete_filename;
   sysfunc::unix_command(unix_cmd);
}
