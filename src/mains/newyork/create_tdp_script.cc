// ========================================================================
// Program CREATE_TDP_SCRIPT is a little utility program which we
// wrote in order to 
// ========================================================================
// Last updated on 4/8/07
// ========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   int n_start=2236;
   int n_stop=2315;
   string basefilename="manhattan_";

   string suffix="_xyz.tdp";
   string tdp_dir="./alirt/";
//   string tdp_dir="./rtv/tdp_files/";
   string scriptfilename="convert_tdp_files";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;
   
   for (int i=n_start; i<=n_stop; i ++)
   {
      string filenumber_str=stringfunc::integer_to_string(i,4);
      string filename=tdp_dir+basefilename+filenumber_str+suffix;
      string command="xyz2tdp "+filename;
//      string command="../OSG/lodtree "+filename;
//      string command="transform_points "+filename;
      scriptstream << command << endl;
   }

   filefunc::closefile(scriptfilename,scriptstream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);
}
