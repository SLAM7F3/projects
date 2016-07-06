// ==========================================================================
// Program PANEL_MOVIES generates executable scripts which string
// together individual, subsampled wisp panel frames (output by
// program SUBSAMPLE_PANELS) into AVI movies.

// 				./panel_movies

// ==========================================================================
// Last updated on 2/22/13; 3/26/13; 4/22/13; 6/23/13
// ==========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   string output_filename="create_panel_movies";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   for (int p=0; p<10; p++)
   {
      string unix_cmd="mkmpeg4 -v -f 4 -b 24000000 -o ";
      unix_cmd += "subsampled_stable_uvcorrected_p"
         +stringfunc::number_to_string(p)+"_res0.avi ";
      unix_cmd += "subsampled_stable_p"+stringfunc::number_to_string(p)+
         "*.png";
      outstream << unix_cmd << endl;
   }

   filefunc::closefile(output_filename,outstream);
   string command="chmod a+x ./"+output_filename;
   sysfunc::unix_command(command);

   string banner="Executable script written to "+output_filename;
   outputfunc::write_banner(banner);
} 

