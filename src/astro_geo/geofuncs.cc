// As of 11/1/2012, we have to insert ugly C-preprocessor definitions
// and conditionals in order to compile our code tree on the TOC12
// laptop which does NOT have libgdal installed!

// #define TOC12_LAPTOP_FLAG

// ==========================================================================
// GEOFUNCS stand-alone methods
// ==========================================================================
// Last modified on 1/29/12; 11/1/12; 11/2/12; 3/27/13
// ==========================================================================

#include "ogrsf_frmts.h"
#include <ogr_spatialref.h>
#include <iostream>
#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/astrofuncs.h"
#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mathfuncs.h"
#include "astro_geo/mgrs.h"
#include "math/mypolynomial.h"
#include "geometry/polygon.h"
#include "image/raster_parser.h"
#include "math/rotation.h"
#include "math/statevector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

#include "astro_geo/Ellipsoid_model.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace geofunc
{
 
// ==========================================================================
// Phi_greenwich computation methods
// ==========================================================================

// Recall that in ECI coordinates, the Greenwich meridian lies at UTC
// midnight within a plane which is rotated relative to the ECI
// xhat-zhat plane about ECI zhat by some azimuthal angle
// phi_greenwich.  (Recall further that ECI xhat points towards
// ARIES.)  Method compute_phi_greenwich returns the value of
// phi_greenwich in radians which depends implicitly upon the Julian
// date that can be constructed from input parameters pass_date and
// currtime.  (Recall currtime = number of seconds since UTC midnight
// on pass date.)

   double compute_phi_greenwich_at_midnight(
      double reference_time,double currtime)
      {
//   const double ref_julian_date=astrofunc::julian_day(1970,1,1); // 1970-01-01 00:00:00
         const double ref_julian_date=2440587.5;	        // 1970-01-01 00:00:00
         double days_since_ref=(reference_time+currtime)/(24.0*3600.0);
         double curr_julian_date=ref_julian_date+days_since_ref;
         double GST=astrofunc::greenwich_sidereal_time(curr_julian_date,0);
         double phi_greenwich=GST/24.0*(2*PI);
         return phi_greenwich;
      }

   double compute_phi_greenwich(double curr_julian_date)
      {
         double GST=astrofunc::greenwich_sidereal_time(curr_julian_date);
         double phi_greenwich=GST/24.0*(2*PI);
         return phi_greenwich;
      }

// ---------------------------------------------------------------------
// Recall that the Greenwich meridian is defined to lie within the
// xhat-zhat plane at 0 hours GMT (= midnight UTC).  In ECI
// coordinates, the Greenwich meridian at midnight UTC is rotated
// about +zhat via azimuthal angle phi_greenwich which depends upon
// the date.  Method convert_MAXLIK_to_ECI_coords therefore
// azimuthally rotates input threevector V about +zhat by
// phi_greenwich:

   threevector convert_MAXLIK_to_ECI_coords(
      double reference_time,double currtime,const threevector& V)
      {
         double phi_greenwich=compute_phi_greenwich_at_midnight(
            reference_time,currtime);
         rotation R(0,0,phi_greenwich);
         return threevector(R*V);
      }

// ==========================================================================
// Magnetic north pole methods
// ==========================================================================

// Member function approx_magnetic_north_pole_geolocation is based
// upon (very limited) measurements and predicted estimates for the
// magnetic north pole's geolocation from 2001 - 2005.  We fit lines
// to the following data which we read off in July 2007 from the
// Canadian website http://gsc/nrcan.gc.ca/geomag/nmp/northpole_e.php:

//	Year 	Latitude (north)	Longitude 

//	2001	81.3			-110.8
//	2002	81.6			-111.6
//	2003	82.0			-112.4
//	2004	82.3			-113.4
//	2005	82.7			-114.4

// Using the extrapolated fits, this method returns an approximate
// location for the magnetic north pole's geolocation as a function of
// input year.

   void approx_magnetic_north_pole_geolocation(
      int year,double& longitude,double& latitude)
      {
         vector<double> longitude_coeff,latitude_coeff;

         longitude_coeff.push_back(1.6901800000e+03);
         longitude_coeff.push_back(-9.0000000000e-01);
         mypolynomial longitude_poly(1,longitude_coeff);
         longitude=longitude_poly.value(year);
         
         latitude_coeff.push_back(-6.1906999999e+02);
         latitude_coeff.push_back(3.4999999999e-01);
         mypolynomial latitude_poly(1,latitude_coeff);
         latitude=latitude_poly.value(year);
      }

// ==========================================================================
// Ground point methods
// ==========================================================================

// Method angle_between_groundpoints returns the angle in degrees
// between the two input geopoint locations on a spherical earth.

   double angle_between_groundpoints(
      const geopoint& point1,const geopoint& point2) 
      {
         double long1=point1.get_longitude()*PI/180;
         double lat1=point1.get_latitude()*PI/180;
         double long2=point2.get_longitude()*PI/180;
         double lat2=point2.get_latitude()*PI/180;
   
         double term1=cos(lat1)*cos(lat2)*cos(long1-long2);
         double term2=sin(lat1)*sin(lat2);
         double gamma=acos(term1+term2);
         return gamma*180/PI;
      }
   
// ---------------------------------------------------------------------
// Method distance_between_groundpoints returns the distance in
// kilometers between two surface points assuming a spherical earth:

   double distance_between_groundpoints(
      const geopoint& point1,const geopoint& point2) 
      {
         return 0.001*R_earth*PI/180*
            angle_between_groundpoints(point1,point2);
      }
   
// ---------------------------------------------------------------------
// Method altitude_relative_to_tangent_plane() takes in some point on
// the spherical earth's surface within tangent_plane_point.  It also
// imports a secondary point that may lie on or above the spherical
// earth.  This method computes and returns the 2nd point's altitude
// relative to the tangent plane point.

   double altitude_relative_to_tangent_plane(
      const geopoint& tangent_plane_point,const geopoint& point2) 
      {
         double theta=PI/180*geofunc::angle_between_groundpoints(
            tangent_plane_point,point2);	// radians
         double R_earth=6371*1000;
         double relative_altitude=(R_earth+point2.get_altitude())*cos(theta)
            -R_earth;
         return relative_altitude;
      }

// ---------------------------------------------------------------------
// Method groundpoints_spherical_earth takes in a direction vector in
// ECI coordinates.  If the direction vector does not intersect the
// surface of the earth, this boolean method returns false.
// Otherwise, it computes the intercept point's latitude and longitude
// coordinates on a spherical earth model.

   bool groundpoints_spherical_earth(
      const threevector& That_ECI,const statevector& satellite_statevector,
      double phi_greenwich,double currtime,geopoint& intercept_point)
      {
         threevector Rvec_ECI(satellite_statevector.get_position()); 
         // Points FROM earth TO satellite in ECI coords
         double R=Rvec_ECI.magnitude();

// Ground intercept point:

         bool intercept_point_on_earth=true;
         double dotproduct=Rvec_ECI.dot(That_ECI);
         double discriminant=sqr(dotproduct)-(sqr(R)-sqr(R_earth));
         if (discriminant < 0)
         {
            cout << "Trouble in geofunc::groundpoints_spherical_earth" 
                 << endl;
            cout << "Ground intercept point is not on earth!" << endl;
            intercept_point_on_earth=false;
         }
         else
         {
            double D=-dotproduct-sqrt(discriminant);
            threevector Shat_ECI((Rvec_ECI+D*That_ECI)/R_earth);
            if (fabs(Shat_ECI.magnitude()) > 1.05)
            {
               cout << "Trouble in geofunc::groundpoints_spherical_earth" 
                    << endl;
               cout << "Ground intercept point is not on earth!" << endl;
               intercept_point_on_earth=false;
            }
            intercept_point.set_latitude(asin(Shat_ECI.get(2)));
            double curr_phi=atan2(Shat_ECI.get(1),Shat_ECI.get(0));
            intercept_point.set_longitude(curr_phi-omega_earth*currtime
                                    -phi_greenwich);

// Convert longitudes and latitudes into degrees:

            intercept_point.set_longitude(
               basic_math::phase_to_canonical_interval(
               intercept_point.get_longitude()*180/PI,-180,180));
            intercept_point.set_latitude(
               basic_math::phase_to_canonical_interval(
               intercept_point.get_latitude()*180/PI,-180,180));

//   cout << "Pointing latitude = " << intercept_point.get_latitude() << " degrees" << endl;
//   cout << "Pointing longitude = " << intercept_point.get_longitude() << " degrees" << endl;
//   newline();
         } // discriminant < 0 conditional
         return intercept_point_on_earth;
      }

// ---------------------------------------------------------------------
// Method groundpoints_ellipsoidal_earth takes in a direction vector
// in ECI coordinates.  If the direction vector does not intersect the
// surface of the earth, this boolean method returns false.
// Otherwise, it computes the latitude and longitude coordinates of
// the intercept point on the ground using an ellipsoidal earth model.

   bool groundpoints_ellipsoidal_earth(
      const threevector& That_ECI,const statevector& satellite_statevector,
      double phi_greenwich,double currtime,geopoint& intercept_point)
      {
         bool intercept_point_on_earth=true;
         
// First obtain approximate latitude and longitude coordinates for
// intercept point using a spherical earth model:

         geopoint approx_intercept_point;
         if (groundpoints_spherical_earth(
            That_ECI,satellite_statevector,phi_greenwich,currtime,
            approx_intercept_point))
         {
            double theta=approx_intercept_point.get_latitude()*PI/180;
   
// Use the approximate latitude value as seed in Newton search for
// refined latitude value based upon ellipsoidal earth model:

            threevector Rvec_ECI(satellite_statevector.get_position());
            // Points FROM earth TO satellite in ECI coords
            double R=Rvec_ECI.magnitude();
            double dotproduct=Rvec_ECI.dot(That_ECI);

// Earth eccentricity:
            const double e=sqrt(f_earth_flatten-sqr(f_earth_flatten));	
            const double min_frac_change=1E-9;
            const int max_iters=10;
            int iter=0;
            double frac_change,D;
            frac_change=D=0;
            do
            {
               double denom=1-sqr(e*cos(theta));
               double r=R_earth*sqrt((1-sqr(e))/denom);
               double drdtheta=-sqr(e)*sin(2*theta)*r/(2*denom);
               double discriminant=sqr(dotproduct)-(sqr(R)-sqr(r));

               if (discriminant < 0) 
               {
                  intercept_point_on_earth=false;
                  return intercept_point_on_earth;
               }
               else
               {
                  D=-dotproduct-sqrt(discriminant);
                  double dDdtheta=r*drdtheta/(D+dotproduct);
                  double F=Rvec_ECI.get(2)+D*That_ECI.get(2)-r*sin(theta);
                  double dFdtheta=dDdtheta*That_ECI.get(2)-drdtheta*sin(theta)
                     -r*cos(theta);
                  double theta_new=theta-F/dFdtheta;
                  iter++;
                  if (theta != 0) frac_change=fabs((theta_new-theta)/theta);
                  theta=theta_new;
               }
            }
            while (frac_change > min_frac_change && iter < max_iters);

            threevector Evec_ECI(Rvec_ECI+D*That_ECI); 
            intercept_point.set_latitude(theta);
            double curr_phi=atan2(Evec_ECI.get(1),Evec_ECI.get(0));
            intercept_point.set_longitude(
               curr_phi-omega_earth*currtime-phi_greenwich);

// Convert longitudes and latitudes into degrees:

            intercept_point.set_longitude(
               basic_math::phase_to_canonical_interval(
               intercept_point.get_longitude()*180/PI,-180,180));
            intercept_point.set_latitude(
               basic_math::phase_to_canonical_interval(
                  intercept_point.get_latitude()*180/PI,-180,180));

//            cout << "Approx latitude = " << approx_intercept_point.get_latitude() 
//                 << endl;
//            cout << "Pointing latitude = " << intercept_point.get_latitude() 
//                 << " degrees" << endl;
//            cout << "Approx longitude = " << approx_intercept_point.get_longitude() 
//                 << endl;
//            cout << "Pointing longitude = " << intercept_point.get_longitude() 
//                 << " degrees" << endl;
//            newline();
         }
         else
         {
            intercept_point_on_earth=false;
         } // groundpoints_spherical_earth conditional
         
         return intercept_point_on_earth;
      }

// ----------------------------------------------------------------
// This overloaded version of groundpoints_spherical_earth takes in
// the camera's position as well as its orientation in ECI
// coordinates.  If the lookpoint does not fall on the earth, this
// boolean method returns false.  Otherwise, it returns lookpoint's
// ECI location in output threevector S_ECI.

   bool groundpoints_spherical_earth(
      const threevector& Rvec_ECI,const threevector& That_ECI,
      threevector& S_ECI)
      {
         double R=Rvec_ECI.magnitude();

// Ground intercept point:

         bool intercept_point_on_earth=true;
         double dotproduct=Rvec_ECI.dot(That_ECI);
         double discriminant=sqr(dotproduct)-(sqr(R)-sqr(R_earth));
         if (discriminant < 0)
         {
            intercept_point_on_earth=false;
         }
         else
         {
            double D=-dotproduct-sqrt(discriminant);
            threevector Shat_ECI((Rvec_ECI+D*That_ECI)/R_earth);
            if (fabs(Shat_ECI.magnitude()) > 1.05)
            {
               intercept_point_on_earth=false;
            }
            S_ECI=R_earth*Shat_ECI;
         }

//         cout << "S_ECI = " << S_ECI << endl;
//         cout << "Shat_ECI = " << S_ECI.unitvector() << endl;
         return intercept_point_on_earth;
      }
   
// ---------------------------------------------------------------------
// Method groundpoints_ellipsoidal_earth takes in a direction vector
// in ECI coordinates.  If the direction vector does not intersect the
// surface of the earth, this boolean method returns false.
// Otherwise, it computes the ECI coordinates of the intercept point
// on the ground using the WGS-84 ellipsoidal earth model.

   bool groundpoints_ellipsoidal_earth(
      const threevector& Rvec_ECI,const threevector& That_ECI,
      threevector& Evec_ECI)
      {
//         cout << "inside geofunc::groundpoints_ellipsoidal_earth()" << endl;
         
         bool intercept_point_on_earth=true;
         
// First obtain approximate latitude and longitude coordinates for
// intercept point using a spherical earth model:

         threevector Svec_ECI;
         if (groundpoints_spherical_earth(Rvec_ECI,That_ECI,Svec_ECI))
         {
            threevector Shat_ECI(Svec_ECI.unitvector());
            double theta=asin(Shat_ECI.get(2));

// Use the approximate latitude value as seed in Newton search for
// refined latitude value based upon ellipsoidal earth model:

            double R=Rvec_ECI.magnitude();
            double dotproduct=Rvec_ECI.dot(That_ECI);

            const double min_frac_change=1E-9;
            const int max_iters=10;
            int iter=0;
            double frac_change=0;
            double D=0;
            do
            {
               double denom=1-sqr(geofunc::eccentricity*cos(theta));
               double r=R_earth*sqrt((1-sqr(geofunc::eccentricity))/denom);
               double drdtheta=-sqr(geofunc::eccentricity)*sin(2*theta)*r/
                  (2*denom);
               double discriminant=sqr(dotproduct)-(sqr(R)-sqr(r));

               if (discriminant < 0) 
               {
                  intercept_point_on_earth=false;
//                  cout << "intercept point NOT on earth" << endl;
                  return intercept_point_on_earth;
               }
               else
               {
                  D=-dotproduct-sqrt(discriminant);
                  double dDdtheta=r*drdtheta/(D+dotproduct);
                  double F=Rvec_ECI.get(2)+D*That_ECI.get(2)-r*sin(theta);
                  double dFdtheta=dDdtheta*That_ECI.get(2)-drdtheta*sin(theta)
                     -r*cos(theta);
                  double theta_new=theta-F/dFdtheta;
                  iter++;
                  if (theta != 0) frac_change=fabs((theta_new-theta)/theta);
                  theta=theta_new;
               }
            }
            while (frac_change > min_frac_change && iter < max_iters);
            Evec_ECI=Rvec_ECI+D*That_ECI; 
         }
         else
         {
            intercept_point_on_earth=false;
         } // groundpoints_spherical_earth conditional

//         cout << "Evec_ECI = " << Evec_ECI << endl;
//         cout << "Ehat_ECI = " << Evec_ECI.unitvector() << endl;
         return intercept_point_on_earth;
      }

// ----------------------------------------------------------------
// Method posn_for_az_elev_range takes in azimuth and elevation
// (measured in degrees) along with range (measured in meters).  It
// returns the Cartesian location of the model assuming in the local
// UTM coordinate system.

// Recall az=0 points towards local north, az=90 points towards local
// east, az=180 points toward local south and az=270 points towards
// local west.

   threevector posn_from_az_elev_range(double az,double elev,double range)
      {
         double cos_az=cos(az*PI/180);
         double sin_az=sin(az*PI/180);
         double cos_el=cos(elev*PI/180);
         double sin_el=sin(elev*PI/180);

// We assume that xhat points in the east direction, yhat points in
// the north direction and zhat points in the radially outward
// direction:
   
         return range*threevector(cos_el*sin_az,cos_el*cos_az,sin_el);
      }

// ----------------------------------------------------------------
// Method raise_polyline_above_ellipsoid() takes in an OGR LineString
// along with an Ellipsoid model.  It computes and returns a new set
// of LineString vertex altitudes so that the entire LineString lies
// above the Ellipsoid's surface.

   vector<double> raise_polyline_above_ellipsoid(
      OGRLineString* poRing_ptr,Ellipsoid_model* Ellipsoid_model_ptr)
      {
         vector<double> point_altitude;
         vector<threevector> curr_polyline_vertices;

// First compute location of polyline vertices assuming each has zero
// altitude:

         int n_points=poRing_ptr->getNumPoints();
         for (int p=0; p<n_points; p++)
         {
            double longitude=poRing_ptr->getX(p);
            double latitude=poRing_ptr->getY(p);
            curr_polyline_vertices.push_back(
               Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
                  longitude,latitude,0));
            point_altitude.push_back(0);
         } // loop over p index labeling vertices for a particular polygon

// Next compute midpoints' distances below ellipsoid's surface given
// that their endpoints lie exactly on the ellipsoid.  Translate each
// segment's endpoints radially outward by an amount greater than this
// distance:

         for (int p=0; p<n_points; p++)
         {
            threevector curr_midpoint=0.5*(
               curr_polyline_vertices[p]
               +curr_polyline_vertices[modulo(p+1,n_points)]);
            double mid_longitude,mid_latitude,mid_altitude;
            Ellipsoid_model_ptr->ConvertXYZToLongLatAlt(
               curr_midpoint,mid_longitude,mid_latitude,mid_altitude);
            double alt=basic_math::max(
               point_altitude[p],-1.5*mid_altitude,200.0);
            point_altitude[p]=alt;
         }
      
         return point_altitude;
      }

// ==========================================================================
// OSG Coordinate System Node methods:
// ==========================================================================

   osg::CoordinateSystemNode* generate_Coordinate_System_Node(
      int UTM_zone,bool northern_hemisphere_flag)
      {
         string format="WKT";	// Well known text string
         return new osg::CoordinateSystemNode(
            format,get_coordinate_system_string(
               UTM_zone,northern_hemisphere_flag));
      }

// ----------------------------------------------------------------   
   string get_coordinate_system_string(
      int UTM_zone,bool northern_hemisphere_flag)
      {

#ifndef TOC12_LAPTOP_FLAG

         char* pszWKT = NULL;
         
         OGRSpatialReference oSRS;
         oSRS.SetWellKnownGeogCS( "WGS84" );
         
         int northern_hemisphere=1;
         if (!northern_hemisphere_flag) northern_hemisphere=0;
         oSRS.SetUTM(UTM_zone,northern_hemisphere);
         
         oSRS.exportToWkt( &pszWKT );
         string coordinate_system_str(pszWKT);
//         cout << "coordinate_system string = " 
//              << coordinate_system_str << endl;
         return coordinate_system_str;

#endif

      }

// ----------------------------------------------------------------   
   void set_coordinate_system(
      osg::CoordinateSystemNode* CSN_ptr,
      int UTM_zone,bool northern_hemisphere_flag)
      {
         CSN_ptr->setCoordinateSystem(get_coordinate_system_string(
            UTM_zone,northern_hemisphere_flag));
      }

// ----------------------------------------------------------------   
   pair<int,bool> get_UTM_zone(osg::CoordinateSystemNode* coord_sysnode_ptr)
      {

#ifndef TOC12_LAPTOP_FLAG

         OGRSpatialReference oSRS;
         char* wkt=const_cast<char*>(
            coord_sysnode_ptr->getCoordinateSystem().c_str());
//         cout << "wkt = " << wkt << endl;
         oSRS.importFromWkt( &wkt );
         int northern_hemisphere;
         int UTM_zone=oSRS.GetUTMZone(&northern_hemisphere);
//         cout << "UTM_zone = " << UTM_zone 
//              << " northern_hemisphere = " << northern_hemisphere << endl;
         
         bool northern_hemisphere_flag=true;
         if (northern_hemisphere==0) northern_hemisphere_flag=false;
         return pair<int,bool>(UTM_zone,northern_hemisphere_flag);
#endif

      }


// ==========================================================================
// Country name methods
// ==========================================================================

// Method generate_simplified_country_name removes all blank spaces
// and periods from an input country's name.  It also converts
// ampersands into "and".  We wrote this little utility to transform
// country names coming from the world borders GIS layer into forms
// easy to work with in linux.

   string generate_simplified_country_name(string country_name)
      {
//         cout << "inside geofunc::generate_simplified_country_name()" 
//              << endl;

         string simplified_country_name=stringfunc::find_and_replace_char(
            country_name," ","_");
         simplified_country_name=stringfunc::find_and_replace_char(
            simplified_country_name,"&","and");
         simplified_country_name=stringfunc::find_and_replace_char(
            simplified_country_name,".","");
//   cout << "simplified_country_name = " << simplified_country_name << endl;
         return simplified_country_name;
      }


// ==========================================================================
// Geocoord conversion methods
// ==========================================================================

// Method MGRS_to_long_lat() takes in an MGRS string and returns its
// corresponding longitude & latitude coordinates in degrees.

   void MGRS_to_long_lat(
      string MGRS_str,double& longitude,double& latitude)
      {
         TEC::Convert_MGRS_To_Geodetic (
            const_cast<char*>(MGRS_str.c_str()),&latitude,&longitude);
         longitude *= 180/PI;
         latitude *= 180/PI;
      }

// ==========================================================================
// UTM zone methods
// ==========================================================================

   void print_recommended_UTM_zonenumbers()
      {
         cout << endl;
         cout << "Recommended UTM zone numbers for various world regions:" 
              << endl;
         cout << endl;
         cout << "California/Arizona: 12" << endl;
         cout << "New Mexico/Texas: 13" << endl;
         cout << "Milwaukee, WI: 16" << endl;
         cout << "Haiti: 18" << endl;
         cout << "New York City: 18" << endl;
         cout << "Boston, Lowell in MA: 19" << endl;
	 cout << "Puerto Rico: 20" << endl;
         cout << "Baghdad, Iraq, Yeman, Somalia: 38" << endl;
         cout << "Afghanistan/Pakistan: 42" << endl;
         cout << "Korea: 52" << endl;
         cout << endl;
      }

   
} // geofunc namespace


   
