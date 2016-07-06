// ==========================================================================
// Program UTM2LL is a little utility which converts a pair of easting
// and northing geocoords (with a hardwired UTM zone number and
// northern hemisphere flag boolean) into a lon,lat pair.
// ==========================================================================
// Last updated on 11/15/10; 12/9/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "astro_geo/geopoint.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);


   double easting,northing;
   cout << "Enter easting:" << endl;
   cin >> easting;
   cout << "Enter northing:" << endl;
   cin >> northing;
   cout << endl;

   bool northern_hemisphere_flag=true;
   int UTM_zonenumber=19;	// Boston
//   int UTM_zonenumber=42;	// Afghanistan
   
   geopoint curr_point(northern_hemisphere_flag,UTM_zonenumber,
		       easting,northing);
   cout << "curr_point = " << curr_point << endl;

}
