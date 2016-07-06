// ========================================================================
// Program GENERATE_TDP2TIF_SCRIPT is a specialized program written to
// produce an executable script for running program TDP2TIF

//			generate_tdp2tif_script

// ========================================================================
// Last updated on 12/16/09; 4/5/10
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

   int start_lon,stop_lon,start_lat,stop_lat;
   cout << "Enter starting longitude:" << endl;
   cin >> start_lon;
   cout << "Enter stopping longitude:" << endl;
   cin >> stop_lon;
   cout << "Enter starting latitude:" << endl;
   cin >> start_lat;
   cout << "Enter stopping latitude:" << endl;
   cin >> stop_lat;
   
   string output_filename="run_tdp2tif";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   string subdir_basename="/media/disk/tdp_files/";
//   string subdir_basename="/media/TruEarth_01/DTED/tdp/";
   for (int lon=start_lon; lon <= stop_lon; lon++)
   {
      string extra_char="";
      if (mathfunc::ndigits_before_decimal_point(lon)==2)
      {
         extra_char="0";
      }

      for (int lat=start_lat; lat <= stop_lat; lat++)
      {
         string dted_filename_prefix="n"+stringfunc::integer_to_string(lat,2);

         string tdp_filename;
         string tdp_suffix="_zp.tdp";
         if (lon < 0)
         {
            string tdp_subdir=subdir_basename+"w"+extra_char+
               stringfunc::number_to_string(fabs(lon))+"/";
            tdp_filename=tdp_subdir+
               "w"+extra_char+stringfunc::number_to_string(fabs(lon))+
               dted_filename_prefix+tdp_suffix;
         }
         else
         {
            string tdp_subdir=subdir_basename+"e"+extra_char+
               stringfunc::number_to_string(lon)+"/";
            tdp_filename=tdp_subdir+
               "e"+extra_char+stringfunc::number_to_string(lon)+
               dted_filename_prefix+tdp_suffix;
         }
         cout << "tdp_filename = " << tdp_filename << endl;
         outstream << "tdp2tif "+tdp_filename << endl;
      } // loop over lat index
   } // loop over lon index

   filefunc::closefile(output_filename,outstream);

   string unix_command="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_command);
}
