// ==========================================================================
// Header file for CAMERA_FRUSTUM class
// ==========================================================================
// Last modified on 2/28/12; 2/29/12; 3/1/12
// ==========================================================================

#ifndef CAMERA_FRUSTUM_H
#define CAMERA_FRUSTUM_H

#include <iostream>
#include <string>
#include <vector>

class face;
class linesegment;
class plane;
class polygon;

class camera_frustum
{

  public:

// Initialization, constructor and destructor functions:

   camera_frustum(
      const threevector& apex,
      const threevector& Uhat,const threevector& Vhat,
      const threevector& What,
      const std::vector<threevector>& UV_corner_world_rays);
   
   camera_frustum(const camera_frustum& c);
   ~camera_frustum();
   camera_frustum& operator= (const camera_frustum& c);
   friend std::ostream& operator<< 
      (std::ostream& outstream,camera_frustum& c);

// Set & get member functions:


// View frustum culling member functions:

   bool PointInsideFrustum(const threevector& XYZ,double tolerance=0);
   bool PointOnFrustum(const threevector& XYZ);
   bool PointOutsideFrustum(const threevector& XYZ,double tolerance=1E-4);
   bool SphereInsideFrustum(const threevector& XYZ,double radius);
   void BoxInsideFrustum(
      const bounding_box* bbox_ptr,
      bool& inside_flag,bool& outside_flag,bool& intersects_flag);
   
// Intersection member functions:

   std::vector<threevector> RayFrustumIntersection(
      const threevector& ray_basepoint,const threevector& r_hat);
   linesegment* SegmentFrustumIntersection(const linesegment& l);
   polygon* PolygonFrustumIntersection(polygon& p);
   polygon* FaceFrustumIntersection(face& f);

  protected:

  private:

   threevector world_posn,Uhat,Vhat,What;
   std::vector<threevector> UV_corner_world_ray;
   std::vector<plane*> frusta_plane_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const camera_frustum& c);

   void compute_frusta_planes();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif // camera_frustum.h



