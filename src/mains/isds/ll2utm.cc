// ==========================================================================
// Program LL2UTM converts latitude & longitude information manually
// extracted for various features from Google Earth into UTM
// coordinates.  Results are written to ascii text file output.
// ==========================================================================
// Last updated on 6/21/06; 11/21/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
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

   string filename="feature_LL_UTM.txt";
   ofstream filestream;
   filefunc::openfile(filename,filestream);

   cout.precision(15);
   filestream.precision(12);

   filestream << "# ID" << "\t"
              << "Lat (degs)" << "\t"
              << "Long (degs)" << "\t"
              << "Easting (m)"<< "\t"
              << "Northing (m)" << "\t"
              << "El (m)" << endl << endl;

   int feature_counter=0;
   int degs,mins;
   double secs,elevation;
   while (true)
   {
      cout << "========================================================="
           << endl << endl;
      double Lat;
      cout << "Enter latitude:" << endl;
      cin >> Lat;
/*
      cout << "Degs = " << endl;
      cin >> degs;
      cout << "Mins =" << endl;
      cin >> mins;
      cout << "Secs = " << endl;
      cin >> secs;
      double Lat=latlongfunc::dms_to_decimal_degs(degs,mins,secs);
      cout << "Latitude in decimal_degs = " << Lat << endl << endl;
*/

      cout << "Enter longitude:" << endl;
      double Long;
      cin >> Long;
/*
      cout << "Degs = " << endl;
      cin >> degs;
      cout << "Mins =" << endl;
      cin >> mins;
      cout << "Secs = " << endl;
      cin >> secs;
      double Long=latlongfunc::dms_to_decimal_degs(degs,mins,secs);
      const double longitude_sgn=-1;	
		// For longitudes measured WEST of Greenwich
      Long *= longitude_sgn;
*/

      cout << "Longitude in decimal_degs = " << Long << endl << endl;

      bool northern_hemisphere_flag;
      int UTM_zonenumber;
      double UTMNorthing,UTMEasting;
      latlongfunc::LLtoUTM(Lat,Long,UTM_zonenumber,northern_hemisphere_flag,
                           UTMNorthing,UTMEasting);

//      cout << "Enter elevation in feet:" << endl;
//      cin >> elevation;
//      elevation *= 0.3048;	// Convert from feet to meters

      cout << "UTM zonenumber = " << UTM_zonenumber
           << " northern hemisphere flag = " << northern_hemisphere_flag
           << endl;
      cout << "  UTMEasting = " << UTMEasting 
           << " UTMNorthing = " << UTMNorthing 
           << " Elevation = " << elevation << endl;

      filestream << feature_counter << "\t"
                 << Lat << "\t"
                 << Long << "\t"
                 << UTMEasting << "\t"
                 << UTMNorthing << "\t"
                 << elevation << endl;
      
/*
      double new_lat,new_long;
      latlongfunc::UTMtoLL(
         UTM_zone,UTMNorthing,UTMEasting,new_lat,new_long);
      latlongfunc::decimal_degs_to_dms(new_lat,degs,mins,secs);
      cout << "Inverted latitude: degs = " << degs << " mins = " << mins
           << " secs = " << secs << endl;
      latlongfunc::decimal_degs_to_dms(new_long,degs,mins,secs);
      cout << "Inverted longitude: degs = " << degs << " mins = " << mins
           << " secs = " << secs << endl;
*/
      feature_counter++;
   }

   filefunc::closefile(filename,filestream);
}
