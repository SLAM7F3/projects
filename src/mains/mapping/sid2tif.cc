// ==========================================================================
// Program SID2TIF takes in the names of MrSID files contained within
// some text file.  It generates a batch file in which each line runs
// LizardTech's MrSID decoding into geotif file output.

//					sid2tif 

// ==========================================================================
// Last updated on 3/29/10; 8/18/10; 9/12/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string sid_filenames_file;
   cout << "Enter name of file containing MrSID filenames:" << endl;
   cin >> sid_filenames_file;

   filefunc::ReadInfile(sid_filenames_file);

   ofstream outstream;
   string output_filename="run_sid2tif";
   filefunc::openfile(output_filename,outstream);

   for (int n=0; n<filefunc::text_line.size(); n++)
   {
      string curr_sid_filename=filefunc::text_line[n];
      string prefix=stringfunc::prefix(curr_sid_filename);
      string output_geotif_filename=prefix+".tif";
      string unix_command=
         "mrsidgeodecode -i "+curr_sid_filename+" -o "+output_geotif_filename
         +" -of tifg";
      cout << unix_command <<  endl;
      outstream << unix_command << endl;
      unix_command="mogrify -format png "+output_geotif_filename;
//      outstream << unix_command << endl;
      
   } // loop over index n

   filefunc::closefile(output_filename,outstream);
   string unix_command="chmod a+x "+output_filename;
   sysfunc::unix_command(unix_command);
}
