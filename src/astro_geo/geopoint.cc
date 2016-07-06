// ==========================================================================
// Geopoint class member function definitions
// ==========================================================================
// Last modified on 5/9/10; 6/20/10; 5/9/11
// ==========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

geopoint::geopoint(void)
{
   set_longitude(0);
   set_latitude(0);
   altitude=time=dt=0;
}

geopoint::geopoint(double currlong,double currlat)
{
   set_longitude(currlong);
   set_latitude(currlat);
   altitude=time=dt=0;
   recompute_UTM_coords();
}

geopoint::geopoint(bool northern_hemisphere_flag,int UTM_zone,
                   double easting,double northing,double height)
{
   set_northern_hemisphere_flag(northern_hemisphere_flag);
   set_UTM_zonenumber(UTM_zone);
   set_UTM_easting(easting);
   set_UTM_northing(northing);
   set_altitude(height);
   time=dt=0;
   recompute_LL_coords();
}

geopoint::geopoint(bool northern_hemisphere_flag,int UTM_zone,
                   const threevector& east_north_height)
{
   set_northern_hemisphere_flag(northern_hemisphere_flag);
   set_UTM_zonenumber(UTM_zone);
   set_UTM_easting(east_north_height.get(0));
   set_UTM_northing(east_north_height.get(1));
   set_altitude(east_north_height.get(2));
   time=dt=0;
   recompute_LL_coords();
}

geopoint::geopoint(double currlong,double currlat,double curralt)
{
   set_longitude(currlong);
   set_latitude(currlat);
   altitude=curralt;
   time=dt=0;
   recompute_UTM_coords();
}

geopoint::geopoint(double currlong,double currlat,double curralt,
                   int specified_UTM_zonenumber)
{
   set_longitude(currlong);
   set_latitude(currlat);
   altitude=curralt;
   time=dt=0;
   recompute_UTM_coords(specified_UTM_zonenumber);
}

// This next constructor takes in an STL vector of strings which is
// assumed to contain longitude, latitude and altitude information.
// We wrote this method for ActiveMQ message handling purposes.

geopoint::geopoint(
   const vector<string>& value_substrings,int specified_UTM_zonenumber)
{
//   cout << "specified_UTM_zonenumber = " << specified_UTM_zonenumber
//        << endl;
   
   double curr_longitude=stringfunc::string_to_number(value_substrings[0]);
   double curr_latitude=stringfunc::string_to_number(value_substrings[1]);
   double curr_altitude=stringfunc::string_to_number(value_substrings[2]);

   set_longitude(curr_longitude);
   set_latitude(curr_latitude);
   altitude=curr_altitude;
   time=dt=0;
      
   recompute_UTM_coords(specified_UTM_zonenumber);
}

// Copy constructor:

geopoint::geopoint(const geopoint& g)
{
   docopy(g);
}

geopoint::~geopoint()
{
}

// ---------------------------------------------------------------------
void geopoint::docopy(const geopoint& g)
{
   northern_hemisphere_flag=g.northern_hemisphere_flag;
   UTM_zonenumber=g.UTM_zonenumber;
   UTM_easting=g.UTM_easting;
   UTM_northing=g.UTM_northing;

   longitude=g.longitude;
   latitude=g.latitude;
   altitude=g.altitude;
   
   long_degs=g.long_degs;
   long_mins=g.long_mins;
   long_secs=g.long_secs;

   lat_degs=g.lat_degs;
   lat_mins=g.lat_mins;
   lat_secs=g.lat_secs;

   time=g.time;
   dt=g.dt;
}	

// Overload = operator:

geopoint geopoint::operator= (const geopoint& g)
{
   if (this==&g) return *this;
   docopy(g);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const geopoint& g)
{

// As of 2/13/09, we believe that the UTM and lat/long coordinates
// should NOT be recomputed within this method.  Recall that UTM
// coordinates may depend upon a specified UTM zone which does not
// necessarily match the value of UTM_zonenumber:

//   g.recompute_UTM_coords(); 
//   g.recompute_LL_coords();

   int lon_int_degs,lat_int_degs;
   double lon_decimal_mins,lat_decimal_mins;
   latlongfunc::decimal_degs_to_int_degs_decimal_minutes(
      g.longitude,lon_int_degs,lon_decimal_mins);
   latlongfunc::decimal_degs_to_int_degs_decimal_minutes(
      g.latitude,lat_int_degs,lat_decimal_mins);

   outstream.precision(14);
   outstream << endl;
   outstream << "UTM zone number = " << g.UTM_zonenumber
             << " northern hemisphere flag = " 
             << g.northern_hemisphere_flag << endl;
   outstream << "Easting = " << g.UTM_easting << " meters" << endl;
   outstream << "Northing = " << g.UTM_northing << " meters" << endl << endl;

   outstream << "longitude = " << endl;
   outstream << g.longitude << " degs = " << endl;
   outstream << lon_int_degs << " degs " 
             << lon_decimal_mins << "' = " << endl;
   outstream << g.long_degs << " degs " 
             << g.long_mins << "' "
             << g.long_secs << "\" " << endl;
   outstream << endl;
   outstream << "latitude = " << endl;
   outstream << g.latitude << " degs = " << endl;
   outstream << lat_int_degs << " degs " 
             << lat_decimal_mins << "' = " << endl;
   outstream << g.lat_degs << " degs " 
             << g.lat_mins << "' "
             << g.lat_secs << "\" " << endl;
   outstream << "altitude = " << g.altitude << " meters" << endl;
//   outstream << "time = " << g.time << endl;
//   outstream << "dt = " << g.dt << endl << endl;
   return outstream;
}

// ---------------------------------------------------------------------
void geopoint::set_longitude(double lng)
{
   longitude=lng;
   latlongfunc::decimal_degs_to_dms(longitude,long_degs,long_mins,long_secs);
}

// ---------------------------------------------------------------------
void geopoint::set_latitude(double lat)
{
   latitude=lat;
   latlongfunc::decimal_degs_to_dms(latitude,lat_degs,lat_mins,lat_secs);
}

// ==========================================================================
// Update member functions
// ==========================================================================

void geopoint::recompute_UTM_coords(int specified_UTM_zonenumber)
{
//   cout << "inside geopoint::Recompute_UTM_coords(),specified_UTM_zonenumber="
//        << specified_UTM_zonenumber << endl;

   if (specified_UTM_zonenumber >=0)
   {
      bool specified_northern_hemisphere_flag=true;
      latlongfunc::LL_to_northing_easting(
         longitude,latitude,
         specified_northern_hemisphere_flag,specified_UTM_zonenumber,
         UTM_easting,UTM_northing);
      UTM_zonenumber=specified_UTM_zonenumber;
   }
   else
   {
      latlongfunc::LLtoUTM(
         latitude,longitude,UTM_zonenumber,northern_hemisphere_flag,
         UTM_northing,UTM_easting);
   }

//   cout << "At end of geopoint::recompute_UTM_coords():" << endl;
//   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
//   cout << "UTM_easting = " << UTM_easting
//        << " UTM_northing = " << UTM_northing << endl;
}

void geopoint::recompute_LL_coords()
{
   latlongfunc::UTMtoLL(
      UTM_zonenumber,northern_hemisphere_flag,
      UTM_northing,UTM_easting,latitude,longitude);
   latlongfunc::decimal_degs_to_dms(longitude,long_degs,long_mins,long_secs);
   latlongfunc::decimal_degs_to_dms(latitude,lat_degs,lat_mins,lat_secs);
}
