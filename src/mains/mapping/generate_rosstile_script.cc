// ========================================================================
// Program GENERATE_ROSSTILE_SCRIPT is a specialized program written
// to produce an executable script for running program ROSSTILE.  We
// wrote this utility in Dec 2009 in order to chop our TEC 2004 Boston
// ladar set into tiles which overlap EO geotif tiles that we created
// in Aug 2009 from MAGIS aerial imagery.

//			generate_rosstile_script

// ========================================================================
// Last updated on 12/22/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
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

   int n_rows=5;
   int n_columns=6;
   double start_easting=322971.349;
   double delta_easting=1500;
   double start_northing=4693637.204;
   double delta_northing=1500;
   string input_tdp_filename="./Boston_TEC.tdp";

   string output_filename="run_rosstile";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (int r=0; r<n_rows; r++)
   {
      double curr_northing=start_northing-r*delta_northing;
      for (int c=0; c<n_columns; c++)
      {
         double curr_easting=start_easting+c*delta_easting;         

         string output_subdir="./tdp_tiles/r"+stringfunc::number_to_string(r)
            +"c"+stringfunc::number_to_string(c)+"/";

         string cmd="ross_tile -o "+output_subdir+"  --crop "
            +stringfunc::number_to_string(curr_easting)+" "
            +stringfunc::number_to_string(curr_northing)+" "
            +stringfunc::number_to_string(curr_easting+delta_easting)+" "
            +stringfunc::number_to_string(curr_northing+delta_northing)+" "
            +input_tdp_filename;
         outstream << cmd << endl;
      } // loop over index c labeling columns
   } // loop over index r labeling rows
   
   filefunc::closefile(output_filename,outstream);

   string unix_command="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_command);
}
