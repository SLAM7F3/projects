// =========================================================================
// Header file for stand-alone geo functions.
// =========================================================================
// Last modified on 12/15/09; 4/1/10; 5/13/10; 3/27/13
// =========================================================================

#ifndef GEOFUNCS_H
#define GEOFUNCS_H

#include <osg/CoordinateSystemNode>
#include <set>
#include <string>
#include "geometry/bounding_box.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

class AnimationController;
class geopoint;
class raster_parser;
class statevector;

class Ellipsoid_model;
class OGRLineString;


namespace geofunc
{
// Earth angular velocity in rads/sec : 
   static const double omega_earth=7.2921151467E-5;  

// Earth's mean equatorial radius in meters : 
   static const double R_earth=6378137.0;	

// Earth's flattening factor.  Recall earth's eccentricity =
// sqrt(2*f_earth_flatten - sqr(f_earth_flatten)):

//   static const double f_earth_flatten=3.352812898E-3;

//   static const double eccentricity=sqrt(
//      2*f_earth_flatten-sqr(f_earth_flatten));       

// Constant earth ellipsoid parameters defined within OSG's
// CoordinateSystemNode header file:

//   const double WGS_84_RADIUS_EQUATOR = 6378137.0;	 // = a
//   const double WGS_84_RADIUS_POLAR = 6356752.3142;	 // = b

// Recall e = sqrt(1-sqr(b/a))
   static const double eccentricity=0.08181919093;	 // Derived OSG value

// Recall f=1-sqrt(1-e*e):
   static const double f_earth_flatten = 0.003352810672; // Derived OSG value

// Newton's gravitational constant * earth mass in meters^3/sec^2:
   static const double Gnewton_times_mearth=3.9860044E14;

// --------------------------------------------------------------------------

// Phi_greenwich computation methods:

   double compute_phi_greenwich_at_midnight(
      double reference_time,double currtime);
   double compute_phi_greenwich(double curr_julian_date);
   threevector convert_MAXLIK_to_ECI_coords(
      double reference_time,double currtime,const threevector& V);

// Magnetic north pole methods:

   void approx_magnetic_north_pole_geolocation(
      int year,double& longitude,double& latitude);

// Ground point methods:

   double angle_between_groundpoints(
      const geopoint& point1,const geopoint& point2); 
   double distance_between_groundpoints(
      const geopoint& point1,const geopoint& point2); 
   double altitude_relative_to_tangent_plane(
      const geopoint& tangent_plane_point,const geopoint& point2);
   bool groundpoints_spherical_earth(
      const threevector& That_ECI,const statevector& satellite_statevector,
      double phi_greenwich,double currtime,geopoint& intercept_point);
   bool groundpoints_ellipsoidal_earth(
      const threevector& That_ECI,const statevector& satellite_statevector,
      double phi_greenwich,double currtime,geopoint& intercept_point);
   bool groundpoints_spherical_earth(
      const threevector& Rvec_ECI,const threevector& That_ECI,
      threevector& S_ECI);
   bool groundpoints_ellipsoidal_earth(
      const threevector& Rvec_ECI,const threevector& That_ECI,
      threevector& Evec_ECI);

   threevector posn_from_az_elev_range(double az,double elev,double range);
   std::vector<double> raise_polyline_above_ellipsoid(
      OGRLineString* poRing_ptr,Ellipsoid_model* Ellipsoid_model_ptr);

// OSG Coordinate System Node generation methods:

   osg::CoordinateSystemNode* generate_Coordinate_System_Node(
      int UTM_zone,bool northern_hemisphere_flag);
   std::string get_coordinate_system_string(
      int UTM_zone,bool northern_hemisphere_flag);
   void set_coordinate_system(
      osg::CoordinateSystemNode* CSN_ptr,
      int UTM_zone,bool northern_hemisphere_flag);
   std::pair<int,bool> get_UTM_zone(
      osg::CoordinateSystemNode* coord_sysnode_ptr);

// Country name methods:

   std::string generate_simplified_country_name(std::string country_name);

// Geocoord conversion methods:

   void MGRS_to_long_lat(
      std::string MGRS_str,double& longitude,double& latitude);

// UTM zone methods:

   void print_recommended_UTM_zonenumbers();
}

#endif // geofuncs.h



