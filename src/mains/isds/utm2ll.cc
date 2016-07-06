// ==========================================================================
// Program UTM2LL
// ==========================================================================
// Last updated on 2/22/07; 5/2/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   cout.precision(12);
   
   while (true)
   {
      cout << "========================================================="
           << endl << endl;

// Baghdad:

      bool northern_hemisphere_flag=true;
      int UTM_zone=38;

      double easting,northing;
      cout << "Enter easting:" << endl;
      cin >> easting;
      cout << "Enter northing:" << endl;
      cin >> northing;
      
      geopoint curr_geopoint(northern_hemisphere_flag,UTM_zone,
                             easting,northing);
      
      cout << "Longitude = " << curr_geopoint.get_longitude() 
           << " Latitude = " << curr_geopoint.get_latitude() << endl;

   }
}
