// ==========================================================================
// EllipsoidModel class member function definitions
// ==========================================================================
// Last modified on /1/07; 8/3/10; 1/29/12; 11/1/12
// ==========================================================================

// GDAL includes

#include <gdal_priv.h>
#include <cpl_string.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <iostream>
#include <set>
#include <string>
#include <osg/Quat>
#include "astro_geo/Clock.h"
#include "math/constants.h"
#include "astro_geo/Ellipsoid_model.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "math/rotation.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Ellipsoid_model::allocate_member_objects()
{
   Ellipsoid_model_refptr=new osg::EllipsoidModel;
   R_enr_to_XYZ_ptr=new genmatrix(3,3);
   R_ner_to_XYZ_ptr=new genmatrix(3,3);
   R_enr_to_ECI_ptr=new genmatrix(3,3);

   generate_Coordinate_System_Node();
}		       

void Ellipsoid_model::generate_Coordinate_System_Node()
{

#ifndef TOC12_LAPTOP_FLAG

   string format="WKT";	// Well known text string
   char* pszWKT = NULL;
   
   OGRSpatialReference oSRS;
   oSRS.SetWellKnownGeogCS( "WGS84" );
   
   oSRS.exportToWkt( &pszWKT );
   string coordinate_system(pszWKT);
   coord_sysnode_refptr=new osg::CoordinateSystemNode(
      format,coordinate_system);

#endif

}

void Ellipsoid_model::initialize_member_objects()
{
   coord_sysnode_refptr->setEllipsoidModel(Ellipsoid_model_refptr.get());
}		       

Ellipsoid_model::Ellipsoid_model()
{
   allocate_member_objects();

   initialize_member_objects();
}

// Copy constructor:

Ellipsoid_model::Ellipsoid_model(const Ellipsoid_model& E)
{
//   docopy(E);
}

// ---------------------------------------------------------------------
Ellipsoid_model::~Ellipsoid_model()
{
   delete R_enr_to_XYZ_ptr;
   delete R_ner_to_XYZ_ptr;
   delete R_enr_to_ECI_ptr;
}

// ---------------------------------------------------------------------
// Member function angle_between_groundpoints computes and returns
// angle gamma between two input geopoints located on the ellipsoid's
// surface in degrees.  This method generalizes
// geofunc::angle_between_groundpoints() which a spherical earth
// model.

double Ellipsoid_model::angle_between_groundpoints(
   const geopoint& point1,const geopoint& point2)
{
   threevector rhat1=ConvertLongLatAltToXYZ(
      point1.get_longitude(),point1.get_latitude(),0).unitvector();
   threevector rhat2=ConvertLongLatAltToXYZ(
      point2.get_longitude(),point2.get_latitude(),0).unitvector();
   return 180/PI*acos(rhat1.dot(rhat2));
}

// ==========================================================================
// Cartesian <--> Ellipsoidal coordinate conversion methods:
// ==========================================================================

// Member function ConvertLatLongAltToXYZ takes in latitude and
// longitude coordinates for some point measured in decimal degrees
// and its altitude measured in meters.  It returns the point's
// coordinates within the time-invariant XYZ coordinate system locked
// to the earth where the Y=0 plane passes through Greenwich.

threevector Ellipsoid_model::ConvertLongLatAltToXYZ(
   double longitude,double latitude,double altitude)
{
   longitude *= PI/180.0;
   latitude *= PI/180.0;
   double X,Y,Z;
   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude,longitude,altitude,X,Y,Z);
   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude,longitude,altitude,X,Y,Z);

   return threevector(X,Y,Z);
}

void Ellipsoid_model::ConvertXYZToLongLatAlt(
   const threevector& surface_posn,double& longitude,double& latitude,
   double& altitude)
{
   Ellipsoid_model_refptr->convertXYZToLatLongHeight(
      surface_posn.get(0),surface_posn.get(1),surface_posn.get(2),
      latitude,longitude,altitude);
   latitude *= 180/PI;
   longitude *= 180/PI;
}

// ---------------------------------------------------------------------
// Member function XYZ_direction_vector_between_surface_points

threevector Ellipsoid_model::XYZ_direction_vector_between_surface_points(
   double basept_latitude,double basept_longitude,
   double surfacept_latitude,double surfacept_longitude)
{
   threevector base_point=ConvertLongLatAltToXYZ(
      basept_longitude,basept_latitude,0);
   threevector surface_point=ConvertLongLatAltToXYZ(
      surfacept_longitude,surfacept_latitude,0);
   return (surface_point-base_point).unitvector();
}

// ---------------------------------------------------------------------
// Member function ConvertLongLatAlttoECI takes in a surface location
// along with a Clock object whose time is assume to have already been
// set.  It returns the time-dependent ECI coordinates for the surface
// location.

threevector Ellipsoid_model::ConvertLongLatAltToECI(
   double longitude,double latitude,double altitude,const Clock& clock)
{
   surface_posn_XYZ=ConvertLongLatAltToXYZ(longitude,latitude,altitude);
   rotation R(0,0,clock.get_phi_greenwich());
   threevector ECI_posn(R*surface_posn_XYZ);
   return ECI_posn;
}

void Ellipsoid_model::ConvertECIToLongLatAlt(
   const threevector& ECI_posn,const Clock& clock,
   double& longitude,double& latitude,double& altitude)
{
   rotation R(0,0,-clock.get_phi_greenwich());
   surface_posn_XYZ=R*ECI_posn;
   ConvertXYZToLongLatAlt(surface_posn_XYZ,longitude,latitude,altitude);
}

// Recall geocentric XYZ position is related to true ECI position via
// an azimuthal rotation about the earth's Z axis.  Since the
// ellipsoid is symmetric with respect to this azimuthal angle, we can
// call ConvertXYZToLongLatAlt rather than ConvertECIToLongLatAlt if
// all we want is to extract an altitude value.  The former method
// does not depend upon time whereas the latter does...

double Ellipsoid_model::eye_altitude(const threevector& ECI_posn)
{
   double latitude,longitude,altitude;
   ConvertXYZToLongLatAlt(ECI_posn,longitude,latitude,altitude);
   return altitude;
}

double Ellipsoid_model::log_eye_altitude(const threevector& ECI_posn)
{
   return log10(eye_altitude(ECI_posn));
}

// ==========================================================================
// ENR direction vector methods
// ==========================================================================

// Member function compute_east_north_radial_dirs takes in latitude
// and longitude coordinates measured in decimal degrees.  It computes
// the local east, north and radial unit vectors in the surface XYZ
// and time-independent ECI coordinate systems.

void Ellipsoid_model::compute_east_north_radial_dirs(
   double latitude,double longitude)
{
//   cout << "inside Ellipsoid_model::compute_east_north_radial_dirs()" 
//	  << endl;
//   cout << "longitude = " << longitude << " latitude = " << latitude << endl;

   double height=0;		// meters
   latitude *= PI/180;
   longitude *= PI/180;

   Ellipsoid_model_refptr->computeLocalToWorldTransformFromLatLongHeight(
      latitude,longitude,height,localToWorld);

// The coordinates of the following cardinal direction vectors are
// measured in the geocentric XYZ basis locked to the spinning earth
// and NOT in the time-independent ECI basis:

   east_hat=threevector(localToWorld(0,0),localToWorld(0,1),
                        localToWorld(0,2));
   north_hat=threevector(localToWorld(1,0),localToWorld(1,1),
                         localToWorld(1,2));
   radial_hat=threevector(localToWorld(2,0),localToWorld(2,1),
                          localToWorld(2,2));

//   cout << "e_hat = " << east_hat << endl;
//   cout << "n_hat = " << north_hat << endl;
//   cout << "r_hat = " << radial_hat << endl;

// Note: On 12/16/06, we verified that radial_hat points directly out
// from ellipsoid's center.  Since ellipsoid bulges outward at its
// equator, radial_hat is NOT precisely perpendicular to the tangent
// plane attached onto the ellipsoid at (longitude,latitude)...

//   threevector e_hat(east_hat);
//   threevector n_hat(north_hat);
//   threevector r_hat(radial_hat);
//   double theta=1.390*PI/180;
//   east_hat=cos(theta)*e_hat+sin(theta)*n_hat;
//   north_hat=-sin(theta)*e_hat+cos(theta)*n_hat;

//   cout << "L2W East_hat in XYZ = " << east_hat << endl;
//   cout << "L2W North_hat in XYZ = " << north_hat << endl;
//   cout << "L2W radial_hat in XYZ = " << radial_hat << endl;
//   cout << "east_hat.north_hat = " << east_hat.dot(north_hat) << endl;
//   cout << "north_hat.radial_hat = " << north_hat.dot(radial_hat) << endl;
//   cout << "radial_hat.east_hat = " << radial_hat.dot(east_hat) << endl;

/*
//   double d_latitude=1E-6*PI/180;	// rad
//   double d_longitude=1E-6*PI/180;	// rad
   double d_latitude=0.1*PI/180;	// rad
   double d_longitude=0.1*PI/180;	// rad
   double d_height=1E-3;		// meter

// Compute east, north and radial unit vectors in XYZ coords:

   double Xplus,Yplus,Zplus;
   double Xminus,Yminus,Zminus;

   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude,longitude+d_longitude,height,Xplus,Yplus,Zplus);
   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude,longitude-d_longitude,height,Xminus,Yminus,Zminus);
   east_hat=threevector(Xplus-Xminus,Yplus-Yminus,Zplus-Zminus);
   east_hat=east_hat.unitvector();
   cout << "differential East_hat in XYZ = " << east_hat << endl;

   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude+d_latitude,longitude,height,Xplus,Yplus,Zplus);
   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude-d_latitude,longitude,height,Xminus,Yminus,Zminus);
   north_hat=threevector(Xplus-Xminus,Yplus-Yminus,Zplus-Zminus);
   north_hat=north_hat.unitvector();
   cout << "differential North_hat in XYZ = " << north_hat << endl;

   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude,longitude,height+d_height,Xplus,Yplus,Zplus);
   Ellipsoid_model_refptr->convertLatLongHeightToXYZ(
      latitude,longitude,height-d_height,Xminus,Yminus,Zminus);
   radial_hat=threevector(Xplus-Xminus,Yplus-Yminus,Zplus-Zminus);
   radial_hat=radial_hat.unitvector();
   cout << "differential Height_hat in XYZ = " << radial_hat << endl;
   
   cout << "east_hat.north_hat = " << east_hat.dot(north_hat) << endl;
   cout << "north_hat.radial_hat = " << north_hat.dot(radial_hat) << endl;
   cout << "radial_hat.east_hat = " << radial_hat.dot(east_hat) << endl;

   cout << "acos(diff e_hat . L2W east_hat) = " 
        << acos(e_hat.dot(east_hat))*180/PI << endl;
   cout << "acos(diff n_hat . L2W north_hat) = " 
        << acos(n_hat.dot(north_hat))*180/PI << endl;
   cout << "acos(diff r_hat . L2W radial_hat) = " 
        << acos(r_hat.dot(radial_hat))*180/PI << endl;

//   osg::Vec3d up(Ellipsoid_model_refptr->computeLocalUpVector(
//      X,Y,Z));
//   cout << "up = " << up.x() << "," << up.y() << "," << up.z() << endl;
*/
}

void Ellipsoid_model::compute_east_north_radial_dirs(
   const threevector& ECI_posn,const Clock& clock)
{
   double latitude,longitude,altitude;
   ConvertECIToLongLatAlt(ECI_posn,clock,longitude,latitude,altitude);
   compute_east_north_radial_dirs(latitude,longitude);
   convert_surface_to_ECI_directions(clock);
}

void Ellipsoid_model::convert_surface_to_ECI_directions(const Clock& clock)
{
   rotation R(0,0,clock.get_phi_greenwich());
   east_ECI_hat=R*east_hat;
   north_ECI_hat=R*north_hat;
   radial_ECI_hat=R*radial_hat;
}

// ---------------------------------------------------------------------
// Member function compute_east_north_radial_dirs takes in latitude
// and longitude coordinates measured in decimal degrees.  It computes
// the local east, north and radial unit vectors in the surface XYZ
// coordinate system.

genmatrix* Ellipsoid_model::east_north_radial_to_XYZ_rotation(
   double latitude,double longitude)
{
   compute_east_north_radial_dirs(latitude,longitude);
   R_enr_to_XYZ_ptr->put_column(0,east_hat);
   R_enr_to_XYZ_ptr->put_column(1,north_hat);
   R_enr_to_XYZ_ptr->put_column(2,radial_hat);
   return R_enr_to_XYZ_ptr;
}

genmatrix* Ellipsoid_model::north_negeast_radial_to_XYZ_rotation(
   double latitude,double longitude)
{
   compute_east_north_radial_dirs(latitude,longitude);
   R_ner_to_XYZ_ptr->put_column(0,north_hat);
   R_ner_to_XYZ_ptr->put_column(1,-east_hat);
   R_ner_to_XYZ_ptr->put_column(2,radial_hat);
   return R_ner_to_XYZ_ptr;
}

// ---------------------------------------------------------------------
// Member function east_north_radial_to_ECI_rotation computes the
// direction vectors east_hat, north_hat and radial_hat in ECI
// coordinates.  It returns the results within the columns of the
// output 3x3 genmatrix.

genmatrix* Ellipsoid_model::east_north_radial_to_ECI_rotation(
   double latitude,double longitude,const Clock& clock)
{
//   cout << "inside Ellipsoid_model::east_north_radial_to_ECI_rotation()"
//        << endl;
   
   compute_east_north_radial_dirs(latitude,longitude);
   convert_surface_to_ECI_directions(clock);

   R_enr_to_ECI_ptr->put_column(0,east_ECI_hat);
   R_enr_to_ECI_ptr->put_column(1,north_ECI_hat);
   R_enr_to_ECI_ptr->put_column(2,radial_ECI_hat);

//   cout << "east_ECI_hat = " << east_ECI_hat << endl;
//   cout << "north_ECI_hat = " << north_ECI_hat << endl;
//   cout << "radial_ECI_hat = " << radial_ECI_hat << endl;
//   cout << "*R_enr_to_ECI_ptr = " << *R_enr_to_ECI_ptr << endl;
   
   return R_enr_to_ECI_ptr;
}

// --------------------------------------------------------------------------
// Member function rotate_zhat_to_rhat returns the quaternion which
// rotates +z_hat into the radial direction vector at the point along
// the earth's surface labeled by the input longitude & latitude
// coordinates.

osg::Quat Ellipsoid_model::rotate_zhat_to_rhat(
   double longitude,double latitude)
{
   compute_east_north_radial_dirs(latitude,longitude);
   const osg::Vec3f Z_hat(0,0,1);
   osg::Quat q;
   q.makeRotate(Z_hat,osg::Vec3f(
      radial_hat.get(0),radial_hat.get(1),radial_hat.get(2)));
   return q;
}

// ---------------------------------------------------------------------
// Member function closest_surface_point_and_dirs_to_ECI_posn takes in
// some ECI position and computes its latitude, longitude and
// altitude, It computes the corresponding nadir point plus east,
// north and radial surface directions in ECI coordinats.

threevector Ellipsoid_model::closest_surface_point_and_dirs_to_ECI_posn(
   const threevector& ECI_posn,const Clock& clock)
{
   double latitude,longitude,altitude;
   ConvertECIToLongLatAlt(ECI_posn,clock,longitude,latitude,altitude);
   compute_east_north_radial_dirs(latitude,longitude);
   convert_surface_to_ECI_directions(clock);
   return ConvertLongLatAltToECI(longitude,latitude,0,clock);
}

// ---------------------------------------------------------------------
// Member function reorients input text so that it runs from west to
// east and lies flat within the local tangent plane at the geopoint
// specified by the input longitude and latitude.

void Ellipsoid_model::align_text_with_cardinal_dirs(
   double longitude,double latitude,osgText::Text* text_ptr,
   bool north_south_flag)
{
//   cout << "inside Ellipsoid_model::align_text_with_cardinal_dirs()"
//        << endl;
   
   genmatrix* R_ptr;
   if (north_south_flag)
   {
      R_ptr=north_negeast_radial_to_XYZ_rotation(latitude,longitude);
   }
   else
   {
      R_ptr=east_north_radial_to_XYZ_rotation(latitude,longitude);
   }

   double chi;
   threevector n_hat;
//   cout << "*R_ptr = " << *R_ptr << endl;
   mathfunc::decompose_orthogonal_matrix(*R_ptr,chi,n_hat);
//   cout << "chi = " << chi*180/PI << endl;
//   cout << "n_hat = " << n_hat << endl;

   osg::Quat q(chi,osg::Vec3(n_hat.get(0),n_hat.get(1),n_hat.get(2)));
   text_ptr->setRotation(q);
}

// ==========================================================================
// Roll, pitch and yaw member functions
// ==========================================================================

// Member function takes in the longitude,latitude geocoordinates for
// some point on the earth's surface.  It also takes in the yaw, pitch
// and roll angles of an aircraft body.  Recall in the
// yaw=pitch=roll=0 limit,

// xbody_hat = north_hat
// ybody_hat = east_hat
// zbody_hat = nadir

// For arbitrary yaw, pitch and roll angles, this method computes and
// returns xbody_hat, ybody_hat and zbody_hat wrt in terms of
// north_hat, east_hat and nadir_hat.

void Ellipsoid_model::decompose_aircraft_body_axes_wrt_north_east_nadir_dirs(
   double longitude,double latitude,double yaw,double pitch,double roll,
   threevector& xbody_hat,threevector& ybody_hat,threevector& zbody_hat)
{
   yaw *= PI/180;
   pitch *= PI/180;
   roll *= PI/180;
   
   rotation Rz(z_hat,yaw);
   rotation Ry(y_hat,pitch);
   rotation Rx(x_hat,roll);
   
   rotation Rbody_to_world=Rx*Ry*Rz;
//   cout << "Rbody_to_world = " << Rbody_to_world << endl;

   compute_east_north_radial_dirs(latitude,longitude);
   
   threevector column0,column1,column2;
   Rbody_to_world.get_column(0,column0);
   Rbody_to_world.get_column(1,column1);
   Rbody_to_world.get_column(2,column2);
   
//   cout << "column0 = " << column0 << endl;
//   cout << "column1 = " << column1 << endl;
//   cout << "column2 = " << column2 << endl;
   
   threevector north_hat=get_north_hat();
   threevector east_hat=get_east_hat();
   threevector nadir_hat=-get_radial_hat();

   threevector xbody_coeffs(
      column0.dot(north_hat),column0.dot(east_hat),column0.dot(nadir_hat));
   threevector ybody_coeffs(
      column1.dot(north_hat),column1.dot(east_hat),column1.dot(nadir_hat));
   threevector zbody_coeffs(
      column2.dot(north_hat),column2.dot(east_hat),column2.dot(nadir_hat));
   
   xbody_hat=xbody_coeffs.get(0)*north_hat+xbody_coeffs.get(1)*east_hat+
      xbody_coeffs.get(2)*nadir_hat;
   ybody_hat=ybody_coeffs.get(0)*north_hat+ybody_coeffs.get(1)*east_hat+
      ybody_coeffs.get(2)*nadir_hat;
   zbody_hat=zbody_coeffs.get(0)*north_hat+zbody_coeffs.get(1)*east_hat+
      zbody_coeffs.get(2)*nadir_hat;

// NEN = North East Nadir 

   cout << "xbody_hat in NEN coord sys= " << xbody_hat << endl;
   cout << "ybody_hat in NEN coord sys= " << ybody_hat << endl;
   cout << "zbody_hat in NEN coord sys= " << zbody_hat << endl;
}
