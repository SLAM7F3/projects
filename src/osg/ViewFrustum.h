// ==========================================================================
// Header file for ViewFrustum class
// ==========================================================================
// Last updated on 9/21/07; 11/16/07; 11/19/07; 6/7/14
// ==========================================================================

#ifndef VIEWFRUSTUM_H
#define VIEWFRUSTUM_H

#include <osg/Matrix>
#include <vector>
#include "geometry/plane.h"
#include "math/threevector.h"

// class osg::BoundingSphere;
class WindowManager;

class ViewFrustum
{

  public:

// Initialization, constructor and destructor functions:

   ViewFrustum(WindowManager* WM_ptr);
   ~ViewFrustum();

// Set & get member functions:

   const threevector& get_camera_posn() const;
   const threevector& get_camera_Zhat() const;
   const std::vector<threevector>& get_ray() const;
   const std::vector<threevector>& get_vertex() const;
   double get_horiz_FOV() const;
   double get_vert_FOV() const;

   void reset_viewmatrix(
      const threevector& Uhat,const threevector& Vhat,
      const threevector& camera_posn);
   void retrieve_camera_posn_and_orientation();

// Frustum parameter evaluation member functions:

   void compute_params_planes_and_vertices();
   std::vector<plane>& compute_planes();
   std::vector<threevector>& compute_vertices();
   void compute_fixed_params();

// Ray tracing member functions:

   threevector get_ray(double fu,double fv);

// Culling member functions:

   bool point_inside(const threevector& V);
   bool sphere_inside(const osg::BoundingSphere& sphere);
   int sphere_inside(const threevector& sphere_center, double radius);

  private:

   bool fixed_params_already_computed;
   double n,f,aspect,FOV_horiz,FOV_vert;
   double tan_half_FOV_up,tan_half_FOV_right;
   double cos_half_FOV_up,cos_half_FOV_right;
   threevector camera_Xhat,camera_Yhat,camera_Zhat,camera_posn;

   double z_groundplane;

   WindowManager* WindowManager_ptr;
   osg::Matrix ProjectionMatrix;
   std::vector<plane> frustum_plane;
   std::vector<threevector> ray,vertex;

   void allocate_member_objects();
   void initialize_member_objects();


}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline const threevector& ViewFrustum::get_camera_posn() const
{
   return camera_posn;
}

inline const threevector& ViewFrustum::get_camera_Zhat() const
{
   return camera_Zhat;
}

inline const std::vector<threevector>& ViewFrustum::get_ray() const
{
   return ray;
}

inline const std::vector<threevector>& ViewFrustum::get_vertex() const
{
   return vertex;
}

inline double ViewFrustum::get_horiz_FOV() const
{
   return FOV_horiz;
}

inline double ViewFrustum::get_vert_FOV() const
{
   return FOV_vert;
}

#endif  // ViewFrustum.h
