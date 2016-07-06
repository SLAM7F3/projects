// ==========================================================================
// Program DECMINS takes in lon-lat geocoords in integer degs and
// decimal minutes.  It converts these input coords into all lon-lat
// forms as well as UTM.
// ==========================================================================
// Last updated on 5/9/11
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
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

   cout.precision(12);

   int lon_degs;
   double lon_mins;
   cout << "Enter integer longitude degs:" << endl;
   cin >> lon_degs;
   cout << "Enter decimal longitude mins:" << endl;
   cin >> lon_mins;
   double longitude=latlongfunc::dm_to_decimal_degs(lon_degs,lon_mins);

   string W_char;
   cout << "Enter 'W' for western longitude:" << endl;
   cin >> W_char;
   if (W_char=="W") longitude *=-1;
//   cout << "longitude = " << longitude << endl;

   int lat_degs;
   double lat_mins;
   cout << "Enter integer latitude degs:" << endl;
   cin >> lat_degs;
   cout << "Enter decimal latitude mins:" << endl;
   cin >> lat_mins;
   double latitude=latlongfunc::dm_to_decimal_degs(lat_degs,lat_mins);
//   cout << "latitude = " << latitude << endl;

   geopoint curr_point(longitude,latitude);
   cout << "curr_point = " << curr_point << endl;

}
