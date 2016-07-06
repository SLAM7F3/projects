// ========================================================================
// Program GENERATE_TIF2OSGA_SCRIPT is a specialized program written to
// produce an executable script for running program TIF2TDP and
// LODTREE on a set of Z-tiles.

//			generate_tif2osga_script

// ========================================================================
// Last updated on 4/14/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;

   ofstream outstream;
   string output_filename="run_t2t";
   filefunc::openfile(output_filename,outstream);

//   string subdir="/media/66368D22368CF3F9/TOC11/FOB_blessing/MWP_tiles/";
   string subdir="/media/66368D22368CF3F9/TOC11/FOB_blessing/new_MWP_tiles/";
   string tif_filenames=subdir+"tif_files";
   string osga_subdir=subdir+"osga_files/";
   
   filefunc::ReadInfile(tif_filenames);

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string prefix=stringfunc::prefix(filefunc::text_line[i]);
      string unix_cmd="tif2tdp "+subdir+filefunc::text_line[i];
      outstream << unix_cmd << endl;
      string tdp_filename=prefix+".tdp";
      unix_cmd="lodtree "+subdir+tdp_filename;
      outstream << unix_cmd << endl;    
      unix_cmd="mv 3*.osga "+osga_subdir;
      outstream << unix_cmd << endl;    
   }
   filefunc::closefile(output_filename,outstream);

   string unix_command="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_command);

}
