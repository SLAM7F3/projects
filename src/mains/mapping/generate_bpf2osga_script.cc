// ========================================================================
// Program GENERATE_BPF2OSGA_SCRIPT is a little utility program which
// reads in the names of all .bpf files within the current subdirectory.
// It generates a script which converts each of the input BPF files to TDP
// format.  The script also calls LODTREE to convert the TDP files to OSGA
// format.  Finally, the TDP files are moved into ./tdp and the OSGA files are
// moved into ./osga subdirectories of ./ .
// ========================================================================
// Last updated on 10/27/10; 11/2/10; 11/10/10
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
   string subdir="./";
   filefunc::dircreate("./tdp");
   filefunc::dircreate("./osga");

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("bpf");
   
   vector<string> bpf_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,subdir);

   string scriptfilename="run_bpf2osga";
   ofstream scriptstream;
   filefunc::openfile(scriptfilename,scriptstream);
   cout << "scriptfilename = " << scriptfilename << endl;

   for (int i=0; i<bpf_filenames.size(); i++)
   {
      string prefix=stringfunc::prefix(bpf_filenames[i]);
      cout << "i = " << i << " bpf_filename = " << bpf_filenames[i] 
           << " prefix = " << prefix << endl;      
      string unix_cmd=
         "/home/cho/programs/c++/svn/projects/src/mains/mapping/bpf2tdp ";
      unix_cmd += bpf_filenames[i];
      scriptstream << unix_cmd << endl;

      string tdp_filename=prefix+".tdp";
      unix_cmd=
         "/home/cho/programs/c++/svn/projects/src/mains/mapping/lodtree ";
      unix_cmd += tdp_filename;
      scriptstream << unix_cmd << endl;

      unix_cmd="mv "+tdp_filename+" ./tdp/";
      scriptstream << unix_cmd << endl;

      string osga_filename=prefix+".osga";
      unix_cmd="mv "+osga_filename+" ./osga/";
      scriptstream << unix_cmd << endl;
   } // loop over index i labeling bpf filenames

   filefunc::closefile(scriptfilename,scriptstream);
   string unix_command="chmod a+x "+scriptfilename;
   sysfunc::unix_command(unix_command);
}
