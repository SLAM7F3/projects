// ========================================================================
// Program GENERATE_LODTREE_SCRIPT is a specialized program written to
// produce an executable script for running Ross' LODTREE program on
// TDP files formed by fusing DTED & TruEarth satellite imagery.  For
// reasons we couldn't figure out, the lodtree command already within
// FUSE_DTED_TRUEARTH failed to properly execute on Allie Hoch's
// laptop.  So we had to cluge together this hack in order to convert
// fused TDP files into final OSGA output.

//			generate_lodtree_script

// ========================================================================
// Last updated on 3/31/10; 4/5/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
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

   int start_lon,stop_lon,start_lat,stop_lat,specified_UTM_zonenumber;
   cout << "Enter starting longitude:" << endl;
   cin >> start_lon;
   cout << "Enter stopping longitude:" << endl;
   cin >> stop_lon;
   cout << "Enter starting latitude:" << endl;
   cin >> start_lat;
   cout << "Enter stopping latitude:" << endl;
   cin >> stop_lat;
   geofunc::print_recommended_UTM_zonenumbers();
   cout << "Enter specified UTM zonenumber:" << endl;
   cin >> specified_UTM_zonenumber;

   string output_filename="run_lodtree";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   string subdir_basename="/media/disk/tdp_files/";

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
         if (lon < 0)
         {
            string tdp_subdir=subdir_basename+"w"+extra_char+
               stringfunc::number_to_string(fabs(lon))+"/";
            tdp_filename=tdp_subdir+
               "w"+extra_char+stringfunc::number_to_string(fabs(lon))+
               dted_filename_prefix+".tdp";
         }
         else
         {
            string tdp_subdir=subdir_basename+"e"+extra_char+
               stringfunc::number_to_string(lon)+"/";
            tdp_filename=tdp_subdir+
               "e"+extra_char+stringfunc::number_to_string(lon)+
               dted_filename_prefix+".tdp";
         }
         cout << "tdp_filename = " << tdp_filename << endl;



         string prefix=stringfunc::prefix(tdp_filename);
         string output_tdp_filename=prefix+"_zp.tdp";
         string lodtree_cmd="/usr/local/bin/lodtree "+output_tdp_filename;
         outstream << lodtree_cmd << endl;

         string output_osga_filename=prefix+"_zp.osga";
         string final_osga_subdir="/media/disk/osga_files/fused_DTED_EO/";
         string mv_command="mv "+output_osga_filename+" "+final_osga_subdir;
         outstream << mv_command << endl;

      } // loop over lat index
   } // loop over lon index
   
   filefunc::closefile(output_filename,outstream);

   string unix_command="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_command);
}
