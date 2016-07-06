// ==========================================================================
// Header file for plane class
// ==========================================================================
// Last modified on 3/2/12; 3/14/12; 2/9/13; 4/4/14
// ==========================================================================

#ifndef PLANE_H
#define PLANE_H

#include <vector>
#include "math/fourvector.h"
#include "math/threevector.h"

class bounding_box;
class linesegment;
class polygon;

class plane
{

  public:

   plane();
   plane(const threevector& V1,const threevector& V2,const threevector& V3);
   plane(const threevector& n,const threevector& P);
   plane(const std::vector<threevector>& V);
   plane(const fourvector& input_pi);
   plane(const plane& p);
   ~plane();
   plane& operator= (const plane& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const plane& p);

// Set and get methods:

   void set_amin(double a);
   void set_amax(double a);
   void set_bmin(double b);
   void set_bmax(double b);
   void set_nmin(double n);
   void set_nmax(double n);
   double get_amin() const;
   double get_amax() const;
   double get_bmin() const;
   double get_bmax() const;
   double get_nmin() const;
   double get_nmax() const;
   void set_origin(const threevector& o);
   threevector get_origin() const;
   fourvector get_pi() const;
   threevector get_ahat() const;
   threevector get_bhat() const;
   threevector get_nhat() const;
   void set_ahat(const threevector& ahat);
   void set_bhat(const threevector& bhat);
   void set_nhat(const threevector& nhat);
   

// Intrinsic plane property methods:

   bool point_on_plane(const threevector& V,double tolerance=0.0001) const;
   double signed_distance_from_plane(const threevector& V) const;
   bool point_in_front_of_plane(const threevector& V) const;
   threevector projection_into_plane(const threevector& V) const;
   double z_coordinate(double x,double y) const;
   void construct_2D_coord_system();
   void reset_ahat(const threevector& a_vec);
   void compute_extremal_planar_coords(const std::vector<threevector>& Vworld);

   void parity_flip_2D_coord_system();
   bool planar_coords(const threevector& V,twovector& Vplanar) const;
   std::vector<twovector> planar_coords(const std::vector<threevector>& V3)
      const;

// Intersection member functions:

   bool infinite_line_intersection(
      const threevector& ray_basept,const threevector& r_hat,
      threevector& intersection_pt) const;
   bool ray_intersection(
      const threevector& ray_basept,const threevector& r_hat,
      threevector& intersection_pt) const;
   bool ray_intersection(
      const threevector& ray_basept,const threevector& r_hat) const;
   bool linesegment_intersection(
      const linesegment& l,threevector& intersection_pnt);
   bool polygon_intersection_query(const polygon& poly);
   int polygon_side_query(const polygon& poly) const;
   int decompose_intersecting_triangle(
      const polygon& triangle,linesegment& intersection_segment,
      polygon& intersection_triangle,polygon& intersection_quadrilateral);
   polygon decompose_intersecting_triangle(
      const polygon& triangle,int plane_side);

   polygon* polygon_above_or_below_plane(
      polygon& poly,int plane_side_sgn);
   polygon* polygon_cleaved_by_plane(
      polygon& poly,int plane_side_sgn);

// World <--> planar coodinate system transformation methods:

   threevector coords_wrt_plane(const threevector& Vworld) const;
   std::vector<threevector>* coords_wrt_plane(
      const std::vector<threevector>& Vworld) const;
   std::vector<threevector>* coords_wrt_plane(
      const std::vector<fourvector>& Vworld) const;
//   std::vector<threevector>* coords_wrt_plane(osg::Vec3Array* vertices_ptr);

   threevector world_coords(const twovector& Vplanar);
   threevector world_coords(const threevector& Vplanar);

// RANSAC construction member functions:

   std::vector<threevector> renormalize_input_points(
      const std::vector<threevector>& V_input);
   bool compute_candidate_plane(const std::vector<threevector>& V);
   std::vector<int> identify_inliers_indices(
      double max_delta,const std::vector<threevector>& V);
   void RANSAC_fit_to_points(
      double max_delta,const std::vector<threevector>& V_input,
      unsigned int max_n_iters=100);

// Ground plane estimation member functions:

   threevector find_ground_plane_origin(
      const threevector& n_hat,const std::vector<threevector>& V_input);
   void estimate_ground_plane(const std::vector<threevector>& V_input);

   void compute_bbox_pn_vertices(
      const bounding_box* bbox_ptr,threevector& p_vertex,
      threevector& n_vertex) const;

  private: 

   double a_min,a_max,b_min,b_max,n_min,n_max;
   threevector xhat,yhat,zhat;
   threevector n_hat,a_hat,b_hat,origin;
   fourvector pi;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const plane& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void plane::set_amin(double a)
{
   a_min=a;
}

inline void plane::set_amax(double a)
{
   a_max=a;
}

inline void plane::set_bmin(double b)
{
   b_min=b;
}

inline void plane::set_bmax(double b)
{
   b_max=b;
}

inline void plane::set_nmin(double n)
{
   n_min=n;
}

inline void plane::set_nmax(double n)
{
   n_max=n;
}

inline double plane::get_amin() const
{
   return a_min;
}

inline double plane::get_amax() const
{
   return a_max;
}

inline double plane::get_bmin() const
{
   return b_min;
}

inline double plane::get_bmax() const
{
   return b_max;
}

inline double plane::get_nmin() const
{
   return n_min;
}

inline double plane::get_nmax() const
{
   return n_max;
}

inline void plane::set_origin(const threevector& o)
{
   origin=o;
}

inline threevector plane::get_origin() const
{
   return origin;
}

inline fourvector plane::get_pi() const
{
   return pi;
}

inline threevector plane::get_ahat() const
{
   return a_hat;
}

inline threevector plane::get_bhat() const
{
   return b_hat;
}

inline threevector plane::get_nhat() const
{
   return n_hat;
}

inline void plane::set_ahat(const threevector& ahat)
{
   a_hat=ahat;
}

inline void plane::set_bhat(const threevector& bhat)
{
   b_hat=bhat;
}

inline void plane::set_nhat(const threevector& nhat)
{
   n_hat=nhat;
}


// ---------------------------------------------------------------------
// Member function signed_distance_from_plane takes in a threevector
// and returns its perpendicular distance to the current plane object.
// If this distance is negative, the point lies on the side of the
// plane anti-parallel to normal direction vector n_hat.

inline double plane::signed_distance_from_plane(const threevector& V) const
{
   return n_hat.dot(V-origin);
}

// ---------------------------------------------------------------------
// Boolean member function point_in_front_of_plane() returns true
// [false] if input threevector V lies on positive side of current
// plane object.

inline bool plane::point_in_front_of_plane(const threevector& V) const
{
   double signed_distance=signed_distance_from_plane(V);
   if (signed_distance > 0)
   {
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function projection_into_plane takes in a threevector and
// returns its projection within the current plane object.  

inline threevector plane::projection_into_plane(const threevector& V) const
{
   return V-signed_distance_from_plane(V)*n_hat;
}

#endif  // plane.h
