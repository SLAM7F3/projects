// ========================================================================
// Program TDP2OSGA generates a script which calls Ross Anderson's
// LODTREE program that converts TDP into OSGA files.  Ross' tiling
// program (mains/OSG/ross_tile) should be run before this program in
// order to chunk up the TDP into uniform subtiles and with standardized
// output file names.
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
   int x_start=0;
   int x_stop=3;
   int y_start=0;
   int y_stop=3;
//   string basefilename="manhattan.";
//   string basefilename="newyork.";
   string basefilename="lowell.";

   string suffix=".tdp";
   string tdp_dir="./rtv/tiles/";
//   string tdp_dir="./rtv/tiles/tdp_files/";
//   string tdp_dir="./alirt/tiles/";
   string scriptfilename="tiles_2_osga";
//   string scriptfilename="register_tdpfiles";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;
   
   for (int x=x_start; x<=x_stop; x++)
   {
      string x_str=stringfunc::number_to_string(x);
      for (int y=y_start; y<=y_stop; y++)
      {
         string y_str=stringfunc::number_to_string(y);
         string filename=tdp_dir+basefilename+"x"+x_str+".y"
            +y_str+suffix;
         string command="../mapping/lodtree "+filename;
//         string command="transform_points "+filename;
         scriptstream << command << endl;
      }
   }

   filefunc::closefile(scriptfilename,scriptstream);
   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);
}
