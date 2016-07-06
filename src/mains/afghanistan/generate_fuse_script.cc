// ========================================================================
// Program GENERATE_FUSE_SCRIPT is a specialized program written to
// produce an executable script for running program FUSE_DTED_TRUEARTH.

//			generate_fuse_script

// ========================================================================
// Last updated on 12/13/09; 3/30/10; 4/5/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
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

   string output_filename="run_fuse_dted_truearth";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (int lon=start_lon; lon <= stop_lon; lon++)
   {
      for (int lat=start_lat; lat <= stop_lat; lat++)
      {
         outstream << "fuse_dted_truearth " << lat << "  " << lon
                   << "  " << specified_UTM_zonenumber << endl;
      } // loop over lat index
   } // loop over lon index
   

   filefunc::closefile(output_filename,outstream);

   string unix_command="chmod a+x ./"+output_filename;
   sysfunc::unix_command(unix_command);
}
