// ==========================================================================
// Program LL2UTM is a little utility which converts a pair of
// longitude & latitude geocoords (entered in degs, mins, secs form) to
// an (easting,northing) pair.
// ==========================================================================
// Last updated on 1/7/11
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

/*
   double lon_degs,lon_mins,lon_secs;
   cout << "Enter longitude degs:" << endl;
   cin >> lon_degs;
   cout << "Enter longitude mins:" << endl;
   cin >> lon_mins;
   cout << "Enter longitude secs:" << endl;
   cin >> lon_secs;
   double longitude=latlongfunc::dms_to_decimal_degs(
      lon_degs,lon_mins,lon_secs);

   string W_char;
   cout << "Enter 'W' for western longitude:" << endl;
   cin >> W_char;
   if (W_char=="W") longitude *=-1;
   cout << "longitude = " << longitude << endl;

   double lat_degs,lat_mins,lat_secs;
   cout << "Enter latitude degs:" << endl;
   cin >> lat_degs;
   cout << "Enter latitude mins:" << endl;
   cin >> lat_mins;
   cout << "Enter latitude secs:" << endl;
   cin >> lat_secs;
   double latitude=latlongfunc::dms_to_decimal_degs(
      lat_degs,lat_mins,lat_secs);
   cout << "latitude = " << latitude << endl;
*/

   double longitude,latitude;
   cout << "Enter longitude:" << endl;
   cin >> longitude;
   cout << "Enter latitude:" << endl;
   cin >> latitude;
   
   geopoint curr_point(longitude,latitude);
   cout << "curr_point = " << curr_point << endl;

}
