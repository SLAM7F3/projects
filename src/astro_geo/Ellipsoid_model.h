// ==========================================================================
// Header file for Ellipsoid_model class which provides a thinly
// wrapped but more user friendly version of the OSG Ellipsoid_model
// class.
// ==========================================================================
// Last modified on 7/29/07; 8/1/07; 8/3/10
// ==========================================================================

#ifndef ELLIPSOIDMODEL_H
#define ELLIPSOIDMODEL_H

#include <osg/CoordinateSystemNode>
#include <osgText/Text>
#include "math/threevector.h"

class Clock;
class geopoint;

class Ellipsoid_model
{
   
  public:

// Initialization, constructor and destructor functions:

   Ellipsoid_model();
   Ellipsoid_model(const Ellipsoid_model& E);
   ~Ellipsoid_model();
   Ellipsoid_model operator= (const Ellipsoid_model& E);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Ellipsoid_model& E);

// Set and get methods:

   const threevector& get_east_hat() const;
   const threevector& get_north_hat() const;
   const threevector& get_radial_hat() const;
   const threevector& get_east_ECI_hat() const;
   const threevector& get_north_ECI_hat() const;
   const threevector& get_radial_ECI_hat() const;

   double angle_between_groundpoints(
      const geopoint& point1,const geopoint& point2);

// Cartesian <--> Ellipsoidal coordinate conversion methods:

   threevector ConvertLongLatAltToXYZ(
      double longitude,double latitude,double altitude);
   void ConvertXYZToLongLatAlt(
      const threevector& surface_posn,double& longitude,double& latitude,
      double& altitude);
   threevector XYZ_direction_vector_between_surface_points(
      double basept_latitude,double basept_longitude,
      double surfacept_latitude,double surfacept_longitude);

   threevector ConvertLongLatAltToECI(
      double longitude,double latitude,double altitude,const Clock& clock);
   void ConvertECIToLongLatAlt(
      const threevector& ECI_posn,const Clock& clock,
      double& longitude,double& latitude,double& altitude);
   double eye_altitude(const threevector& ECI_posn);
   double log_eye_altitude(const threevector& ECI_posn);

// ENR direction vector methods:

   void compute_east_north_radial_dirs(double latitude,double longitude);
   void compute_east_north_radial_dirs(
      const threevector& ECI_posn,const Clock& clock);
   void convert_surface_to_ECI_directions(const Clock& clock);

   genmatrix* east_north_radial_to_XYZ_rotation(
      double latitude,double longitude);
   genmatrix* north_negeast_radial_to_XYZ_rotation(
      double latitude,double longitude);
   genmatrix* east_north_radial_to_ECI_rotation(
      double latitude,double longitude,const Clock& clock);
   osg::Quat rotate_zhat_to_rhat(double longitude,double latitude);

   threevector closest_surface_point_and_dirs_to_ECI_posn(
      const threevector& ECI_posn,const Clock& clock);
   void align_text_with_cardinal_dirs(
      double longitude,double latitude,osgText::Text* text_ptr,
      bool north_south_flag=false);

// Roll, pitch and yaw member functions

   void decompose_aircraft_body_axes_wrt_north_east_nadir_dirs(
      double longitude,double latitude,double yaw,double pitch,double roll,
      threevector& xbody_hat,threevector& ybody_hat,threevector& zbody_hat);

  private: 

   threevector surface_posn_XYZ;
   threevector east_hat,north_hat,radial_hat;
   threevector east_ECI_hat,north_ECI_hat,radial_ECI_hat;
   genmatrix *R_enr_to_XYZ_ptr,*R_ner_to_XYZ_ptr;
   genmatrix* R_enr_to_ECI_ptr;
   osg::Matrixd localToWorld;

   osg::ref_ptr<osg::CoordinateSystemNode> coord_sysnode_refptr;
   osg::ref_ptr<osg::EllipsoidModel> Ellipsoid_model_refptr;

   void allocate_member_objects();
   void generate_Coordinate_System_Node();
   void initialize_member_objects();
   void docopy(const Ellipsoid_model& E);
   
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline const threevector& Ellipsoid_model::get_east_hat() const
{
   return east_hat;
}

inline const threevector& Ellipsoid_model::get_north_hat() const
{
   return north_hat;
}

inline const threevector& Ellipsoid_model::get_radial_hat() const
{
   return radial_hat;
}

inline const threevector& Ellipsoid_model::get_east_ECI_hat() const
{
   return east_ECI_hat;
}

inline const threevector& Ellipsoid_model::get_north_ECI_hat() const
{
   return north_ECI_hat;
}

inline const threevector& Ellipsoid_model::get_radial_ECI_hat() const
{
   return radial_ECI_hat;
}

#endif  // Ellipsoid_model.h


