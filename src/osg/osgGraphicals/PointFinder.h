// ==========================================================================
// Header file for POINTFINDER class 
// ==========================================================================
// Last modified on 6/19/08; 2/19/09; 2/20/09; 2/22/09; 9/16/09; 10/16/13
// ==========================================================================

#ifndef POINTFINDER_H
#define POINTFINDER_H

#include <iostream>
#include <vector>
#include <osgUtil/SceneView>
#include "osg/osgSceneGraph/DataGraphsGroup.h"
#include "math/threevector.h"

namespace osgGA
{
   class Custom3DManipulator;
}

class PointFinder
{
  public:

// Initialization, constructor and destructor functions:

   PointFinder();
   PointFinder(DataGraphsGroup* DGG_ptr);
   ~PointFinder();

// Set and get methods

   void set_minimal_allowed_range(double r);
   void set_maximal_rho(double r);
   void set_max_cone_halfangle(double theta);
   void pushback_DataGraphsGroup_ptr(DataGraphsGroup* DGG_ptr);

   void set_nearest_worldspace_point(const threevector& p);
   threevector& get_nearest_worldspace_point();
   const threevector& get_nearest_worldspace_point() const;

// Transformations between world xyz and screen XYZ coordinate systems:

   bool find_closest_world_point(
      osgGA::Custom3DManipulator* CM_3D_ptr,
      double X,double Y,threevector& closest_worldspace_point);
   bool find_closest_world_point(
      const threevector& ray_basepoint,const threevector& ray_ehat,
      threevector& closest_worldspace_point,double tan_theta=.004363323);
   bool find_smallest_relative_angle_world_point(
      const threevector& ray_basepoint,const threevector& ray_ehat,
      threevector& closest_worldspace_point);
   bool find_altitude_given_longitude_and_latitude(
      double longitude,double latitude,double& altitude,double Z1=-100);
   bool find_altitude_given_easting_and_northing(
      double easting,double northing,double& altitude,double Z1=-100);

 private:

   double minimal_allowed_range,maximal_rho;
   double max_tan_theta;
   threevector nearest_worldspace_point;
   std::vector<DataGraphsGroup*> DataGraphsGroup_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();

}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void PointFinder::set_minimal_allowed_range(double r)
{
   minimal_allowed_range=r;
}

inline void PointFinder::set_maximal_rho(double rho)
{
   maximal_rho=rho;
}

// Member function set_max_cone_halfangle takes in maximum search cone
// half-angle theta measured in degrees.

inline void PointFinder::set_max_cone_halfangle(double theta)
{
   max_tan_theta=tan(theta*PI/180);
   std::cout << "max_tan_theta = " << max_tan_theta << std::endl;
}

inline void PointFinder::pushback_DataGraphsGroup_ptr(
   DataGraphsGroup* DGG_ptr)
{
   DataGraphsGroup_ptrs.push_back(DGG_ptr);
}

inline void PointFinder::set_nearest_worldspace_point(const threevector& p)
{
   nearest_worldspace_point=p;
}

inline threevector& PointFinder::get_nearest_worldspace_point()
{
   return nearest_worldspace_point;
}

inline const threevector& PointFinder::get_nearest_worldspace_point() const
{
   return nearest_worldspace_point;
}

#endif // PointFinder.h



