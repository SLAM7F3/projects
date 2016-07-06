// ==========================================================================
// SAM class member function definitions
// ==========================================================================
// Last modified on 3/6/08; 3/12/08; 3/13/08; 3/25/08
// ==========================================================================

#include <iostream>
#include <vector>
#include "math/constants.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "robots/SAM.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SAM::allocate_member_objects()
{
}		       

void SAM::initialize_member_objects()
{
   entity_ID="=1";
   sam_type=other;
}

void SAM::initialize_params_based_on_type()
{
   IOC=-1;	
   max_range=max_target_altitude=warhead_weight=max_speed=NEGATIVEINFINITY;
   
   if (sam_type==SA2)				
   {
      IOC=1959;
      max_range=35;		// kms
      max_target_altitude=32;	// kms
      max_speed=1361;		// m/sec
   }
   else if (sam_type==SA3)			
   {
      IOC=1961;
      max_range=25;		// kms
      max_target_altitude=25;	// kms
      max_speed=1021;		// m/sec
   }
   else if (sam_type==SA4)
   {
      IOC=1974;
      max_range=50;		// kms
      max_target_altitude=24.5;	// kms
      warhead_weight=150;	// kgs
      max_speed=1000;		// m/sec
   }
   else if (sam_type==SA5)			
   {
      IOC=1967;
      max_range=250;		// kms
      max_target_altitude=30.5;	// kms
      warhead_weight=215;	// kgs
      max_speed=1930;		// m/sec
   }
   else if (sam_type==SA6)
   {
      IOC=1978;
      max_range=24;		// kms
      max_target_altitude=14;	// kms
      warhead_weight=57;	// kgs
      max_speed=600;		// m/sec
   }
   else if (sam_type==SA8)
   {
      IOC=1980;
      max_range=10;		// kms
      max_target_altitude=5;	// kms
      warhead_weight=15;	// kgs
      max_speed=540;		// m/sec
   }
   else if (sam_type==SA10)
   {
      IOC=1980;
      max_range=150;		// kms
      max_target_altitude=27;	// kms
      warhead_weight=143;	// kgs
      max_speed=2000;		// m/sec
   }
   else if (sam_type==SA11)
   {
      IOC=1980;
      max_range=35;		// kms
      max_target_altitude=22;	// kms
      warhead_weight=70;	// kgs
      max_speed=850;		// m/sec
   }
   else if (sam_type==SA12)
   {
      IOC=1986;
      max_range=100;		// kms
      max_target_altitude=30;	// kms
      warhead_weight=150;	// kgs
      max_speed=2400;		// m/sec
   }
   else if (sam_type==SA13)
   {
      IOC=1975;
      max_range=5;		// kms
      max_target_altitude=3;	// kms
      warhead_weight=3;		// kgs
      max_speed=550;		// m/sec
   }
   else if (sam_type==SA14)			
   {
      IOC=1978;
      max_range=6;		// kms
      max_target_altitude=6;	// kms
      warhead_weight=1;		// kg
      max_speed=600;		// m/sec
   }
   else if (sam_type==SA15)
   {
      IOC=1991;
      max_range=12;		// kms
      max_target_altitude=6;	// kms
      warhead_weight=15;	// kgs
      max_speed=860;		// m/sec
   }
   else if (sam_type==SA16)		       
   {
      IOC=1983;
      max_range=5;		// kms
      max_target_altitude=3.5;	// kms
      warhead_weight=2;		// kgs
      max_speed=680;		// m/sec
   }
   else if (sam_type==SA18)			
   {
      IOC=1984;
      max_range=5.2;		// kms
      max_target_altitude=3.5;	// kms
      warhead_weight=2;		// kgs
      max_speed=680;		// m/sec
   }
   else if (sam_type==FT2000)
   {
      IOC=2005;
      max_range=100;		// kms
      max_target_altitude=20;	// kms
      warhead_weight=60;	// kgs
   }
   else if (sam_type==FM90)
   {
      IOC=1998;
      max_range=15;		// kms
      max_target_altitude=6;	// kms
      max_speed=750;		// m/sec
   }
   else if (sam_type==HQ2)	// same as SA-2			     	
   {
      IOC=1959;
      max_range=35;		// kms
      max_target_altitude=32;	// kms
      max_speed=1361;		// m/sec
   }
   else if (sam_type==Pegasus)
   {
      IOC=1999;
      max_range=10.5;		// kms
      max_target_altitude=6;	// kms
      warhead_weight=12;	// kgs
      max_speed=884;		// m/sec
   }
}

// ---------------------------------------------------------------------
string SAM::get_name() const
{
   switch (sam_type)
   {
      case SA2:
         return "SA-2";
         break;
      case SA3:
         return "SA-3";
         break;
      case SA4:
         return "SA-4";
         break;
      case SA5:
         return "SA-5";
         break;
      case SA6:
         return "SA-6";
         break;
      case SA7:
         return "SA-7";
         break;
      case SA8:
         return "SA-8";
         break;
      case SA9:
         return "SA-9";
         break;
      case SA10:
         return "SA-10";
         break;
      case SA11:
         return "SA-11";
         break;
      case SA12:
         return "SA-12";
         break;
      case SA13:
         return "SA-13";
         break;
      case SA14:
         return "SA-14";
         break;
      case SA15:
         return "SA-15";
         break;
      case SA16:
         return "SA-16";
         break;
      case SA18:
         return "SA-18";
         break;
      case FT2000:
         return "FT-2000";
         break;
      case FM90:
         return "FM-90";
         break;
      case HQ2:
         return "HQ-2";
         break;
      case Pegasus:
         return "Pegasus";
         break;
      default:
         return "Unknown";
   }
}


SAM::SAM(int ID)
{
   allocate_member_objects();
   initialize_member_objects();

   this->ID=ID;
   entity_ID=stringfunc::number_to_string(ID);
}

// Copy constructor:

SAM::SAM(const SAM& s)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(s);
}

SAM::~SAM()
{
}

// ---------------------------------------------------------------------
void SAM::docopy(const SAM& s)
{
//   cout << "inside SAM::docopy()" << endl;

   ID=s.ID;
}

// Overload = operator:

SAM& SAM::operator= (const SAM& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const SAM& s)
{
   outstream << endl;

//   cout << "inside SAM::operator<<" << endl;
   outstream << "SAM ID = " << s.ID 
             << " type = " << s.get_name() << endl;
   outstream << "range = " << s.max_range
             << " tgt alt = " << s.max_target_altitude
             << " warhd wgt = " << s.warhead_weight
             << " speed = " << s.max_speed << endl;
   outstream << "site longitude = " 
             << s.get_site_location().get_longitude()
             << " latitude = "
             << s.get_site_location().get_latitude()
             << " altitude = "
             << s.get_site_location().get_altitude()
             << endl;
   outstream << "country owner = " << s.get_country_owner() << endl;
   return(outstream);
}

// =====================================================================
// Member function generate_threat_region converts the site location
// into UTM coordinates.  After drawing a 20-sided polygon around the
// center location in UTM space, it stores the longitude, latitude and
// altitude values for each polygon vertex in member STL vector
// threat_region_long_lat_alt_vertices.  This method returns a polygon
// object in UTM coordinates.

polygon& SAM::generate_threat_region()
{
   threevector center_posn(
      site_location.get_UTM_easting(),
      site_location.get_UTM_northing(),
      site_location.get_altitude());

   double radius=max_range*1000;	// meters

   const int n_poly_sides=20;
   double theta_start=0;
   double theta_stop=2*PI;
   double d_theta=(theta_stop-theta_start)/(n_poly_sides);

   vector<threevector> vertices;
   for (int n=0; n<n_poly_sides; n++)
   {
      double theta=theta_start+n*d_theta;
      threevector curr_V=center_posn+
         radius*threevector(cos(theta),sin(theta));
      vertices.push_back(curr_V);

      double curr_longitude,curr_latitude;
      latlongfunc::UTMtoLL(
         site_location.get_UTM_zonenumber(),
         site_location.get_northern_hemisphere_flag(),
         curr_V.get(1),curr_V.get(0),curr_latitude,curr_longitude);

      threat_region_long_lat_alt_vertices.push_back(
         threevector(curr_longitude,curr_latitude,curr_V.get(2)));

//      cout << "n = " << n
//           << " lla = " << threat_region_long_lat_alt_vertices.back()
//           << endl;
   }
   
   threat_region=polygon(vertices);
//   cout << "threat_region = " << threat_region << endl;
   return threat_region;
}

// ---------------------------------------------------------------------
// Member function generate_dynamic_URL enters SAM parameters into a
// URL which can be sent to Dave Ceddia's PHP template on touchy.  The
// parameters will subsequently be extracted from the URL and
// displayed within a dynamically generated web page.

string SAM::generate_dynamic_URL()
{
   string dynamic_URL="http://touchy/rco/sam.php?";

   geopoint location=get_site_location();
   dynamic_URL += "site="+stringfunc::number_to_string(get_ID());
   dynamic_URL += "&system="+get_name();
   dynamic_URL += "&country="+get_country_owner();
   dynamic_URL += "&lat="+stringfunc::number_to_string(
      location.get_latitude(),6)+" degs";
   dynamic_URL += "&lon="+stringfunc::number_to_string(
      location.get_longitude(),6)+" degs";
         
   dynamic_URL += "&range=";
   double curr_range=get_max_range();
   if (curr_range > 0)
   {
      dynamic_URL += stringfunc::number_to_string(curr_range,1)+" kms";
   }
   else
   {
      dynamic_URL += "Unknown";
   }
         
   dynamic_URL += "&max_speed=";
   double max_speed=get_max_speed();
   if (max_speed > 0)
   {
      dynamic_URL += stringfunc::number_to_string(max_speed,1)+" m/s";
   }
   else
   {
      dynamic_URL += "Unknown";
   }

   dynamic_URL += "&max_alt=";
   double max_tgt_altitude=get_max_target_altitude();
   if (max_tgt_altitude > 0)
   {
      dynamic_URL += stringfunc::number_to_string(max_tgt_altitude,1)
         +" kms";
   }
   else
   {
      dynamic_URL += "Unknown";
   }

   dynamic_URL += "&weight=";
   double warhead_weight=get_warhead_weight();
   if (warhead_weight > 0)
   {
      dynamic_URL += stringfunc::number_to_string(warhead_weight,1)+" kgs";
   }
   else
   {
      dynamic_URL += "Unknown";
   }

   dynamic_URL += "&ioc=";
   double IOC=get_IOC();
   if (warhead_weight > 0)
   {
      dynamic_URL += stringfunc::number_to_string(IOC);
   }
   else
   {
      dynamic_URL += "Unknown";
   }

   return dynamic_URL;
}

