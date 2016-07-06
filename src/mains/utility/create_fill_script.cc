// ========================================================================
// Program CREATE_FILL_SCRIPT is a little utility program which we
// wrote to generate an executable script that calls MEDIAN_FILL and
// LODTREE for every Bluegrass Lubbock tile.
// ========================================================================
// Last updated on 11/23/07
// ========================================================================

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
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string scriptfilename="run_median_fill";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;

   int n_start=110;
   int n_stop=896;
   
   vector<string> command;
   command.push_back("/home/cho/programs/c++/svn/projects/src/mains/bluegrass/median_fill \\");
   command.push_back("/home/cho/programs/c++/svn/projects/src/mains/OSG/lodtree \\");


   for (int n=n_start; n<=n_stop; n++)
   {
      string tdp_filename_prefix=
         "tile_"+stringfunc::number_to_string(n);
      string tdp_filename=tdp_filename_prefix+".tdp";
      string command0b="./"+tdp_filename;
      scriptstream << command[0] << endl;
      scriptstream << command0b << endl;

      string command1b="./"+tdp_filename_prefix+"_filled.tdp";
      scriptstream << command[1] << endl;
      scriptstream << command1b << endl << endl;
   }

   filefunc::closefile(scriptfilename,scriptstream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);
}
