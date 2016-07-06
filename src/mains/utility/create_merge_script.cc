// ========================================================================
// Program CREATE_MERGE_SCRIPT is a little utility program which we
// wrote to generate an executable script that calls MERGEPOINTS and
// LODTREE for every RTV NYC tile.
// ========================================================================
// Last updated on 11/11/07
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
   string scriptfilename="run_mergepoints";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;

   int m_start=0;
   int m_stop=3;

   vector<int> n_start,n_stop;
   n_start.push_back(0);
   n_start.push_back(0);
   n_start.push_back(0);
   n_start.push_back(2);
   
   n_stop.push_back(3);
   n_stop.push_back(6);
   n_stop.push_back(7);
   n_stop.push_back(7);
   
   vector<string> command;
   command.push_back("/home/cho/programs/c++/svn/projects/src/mains/newyork/mergepoints \\");
   command.push_back("/home/cho/programs/c++/svn/projects/src/mains/OSG/lodtree \\");


   for (int m=m_start; m<=m_stop; m++)
   {
      for (int n=n_start[m]; n<=n_stop[m]; n++)
      {
         string xyname="x"+stringfunc::number_to_string(m)+
            "y"+stringfunc::number_to_string(n);
         scriptstream << command[0] << endl;
         string command0b=xyname+"_fused_filled.tdp --newpass ../../"
            +xyname+"_fused.tdp";
         scriptstream << command0b << endl;
         scriptstream << command[1] << endl;
         string command1b="../../"+xyname+"_fused_walls.tdp";
         scriptstream << command1b << endl << endl;
      }
      
   }

   filefunc::closefile(scriptfilename,scriptstream);

   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);
}
