// ==========================================================================
// Program WRITE_RUN_SAR_PROP is a little utility we wrote to
// downsample the number of simulated SAR images for programs
// ANALYZE_SAR and SAR_PROPAGATOR.
// ==========================================================================
// Last updated on 3/12/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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

   int n_start=0;
   int n_stop=300;
   int n_step=5;

   string packages_subdir="./bundler/GEO/SAR_5-25-flight1/packages/";

   string output_filename="run_SAR";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (int n=n_start; n<n_stop; n+= n_step)
   {
      string package_filename=packages_subdir+"photo_"+
         stringfunc::integer_to_string(n,4)+".pkg";
      outstream << "--region_filename "+package_filename << endl;
   }
   
   filefunc::closefile(output_filename,outstream);
   
   

}
