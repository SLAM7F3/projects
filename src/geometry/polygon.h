// ==========================================================================
// Header file for polygon class
// ==========================================================================
// Last modified on 3/13/12; 4/16/12; 3/6/14; 4/4/14
// ==========================================================================

#ifndef POLYGON_H
#define POLYGON_H

#include <iostream>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include "math/constants.h"
#include "geometry/gpc.h"
#include "geometry/linesegment.h"
#include "geometry/mypoint.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"
#include "geometry/triangles_group.h"
#include "datastructures/Triple.h"

class contour;
class plane;
class polyline;
class rotation;

class polygon
{

  public:

// Initialization, constructor and destructor functions:

   polygon(void);
   polygon(int num_of_vertices);
   polygon(const std::vector<twovector>& currvertex);
   polygon(const threevector& interior_pnt,
           const std::vector<threevector>& currvertex);
   polygon(const std::vector<threevector>& currvertex);

   polygon(double min_x,double min_y,double max_x,double max_y);
   polygon(contour const *c_ptr);
   polygon(const polyline& p);
   polygon(const polygon& p);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~polygon();
   polygon& operator= (const polygon& p);
   friend std::ostream& operator<< (std::ostream& outstream,const polygon& p);
   std::ostream& write_to_textstream(std::ostream& textstream);
   void read_from_text_lines(unsigned int& i,std::vector<std::string>& line);

// Set and get member functions:

   void set_occluded_flag(bool flag);
   bool get_occluded_flag() const;
   void set_ID(int ID);
   int get_ID() const; 
   void set_nvertices(int n);
   void set_area(double a);
   void set_perimeter(double p);
   void set_origin(const threevector& o);
   void set_normal(const threevector& n);
   void set_vertex_avg(const threevector& va);

   unsigned int get_nvertices() const;
   double get_area() const;
   double get_perimeter() const;
   threevector& get_origin();
   threevector& get_normal();
   threevector& get_vertex_avg();

   void set_vertex(int n,const threevector& v);
   threevector& get_vertex(int n) const;

   linesegment& get_edge(int n);
   triangles_group* get_triangles_group_ptr();
   const triangles_group* get_triangles_group_ptr() const;

// Set & get member functions:

   plane* get_plane_ptr();

// Intrinsic polygon properties:

   threevector& compute_normal();

   double compute_area();
   threevector compute_COM();
   threevector compute_2D_COM();
   double projected_area(const threevector& uhat);
   double compute_perimeter();

   void sidelengths(double length[]) const;
   threevector& vertex_average();
   rotation natural_coordinate_system() const;
   std::vector<std::pair<threevector,bool> >* generate_interior_points_list(
      double ds_frac,const rotation& Rtrans,
      double max_dist_to_poly_edge=POSITIVEINFINITY,
      bool retain_pnts_far_from_edges=false,double extremal_overstep=2.0);
   std::vector<threevector> generate_polygon_point_cloud(double ds_frac);

// Polygon edge traversing methods:

   void initialize_edge_segments();
   void compute_edges();
   int edge_number(double frac);
   void edge_point(double frac,threevector& currpoint);
   void radial_direction_vector(double frac,threevector& r_hat);

   void eliminate_all_parallel_edges();
   bool consolidate_parallel_edges();

// Determining point, linesegment and polygon locations relative to
// one another:

   void locate_extremal_xy_points(
      double& min_x,double& min_y,double& max_x,double& max_y) const;
   void locate_extremal_xyz_points(
      double& min_x,double& min_y,double& min_z,
      double& max_x,double& max_y,double& max_z) const;
   void locate_extremal_ab_points(
      const threevector& alpha_hat,const threevector& beta_hat,
      double& min_alpha,double& min_beta,
      double& max_alpha,double& max_beta) const;

// Point location relative to polygon:

   bool point_inside_polygon(const twovector& point);
   bool point_inside_polygon(const threevector& point);
   bool point_on_perimeter(const threevector& point) const;
   bool point_on_perimeter(const threevector& point,double dx,double dy);
   double frac_distance_along_polygon(const threevector& point);
   bool point_outside_polygon(
      const threevector& point,double dx,double dy);
   double point_dist_to_polygon(const threevector& point);
   int closest_polygon_edge_to_point(const threevector& point);
   std::pair<double,threevector> closest_polygon_perimeter_point(
      const threevector& ext_point);
   double min_dist_to_convex_polygon(const threevector& external_point) const;
   std::pair<int,int> closest_vertices_to_external_point(
      int n_jump,int i,int& n_expensive_computations,
      const threevector& external_point,double ext_pnt_vertex_sqrd_dist[]) 
      const;
   Triple<bool,double,threevector> closest_polygon_point(
      const threevector& ext_point);

// Line segment relative to polygon:

   void linesegment_inside_polygon(
      const linesegment& l,bool& l_inside_poly,bool& l_outside_poly);
   void polygon_inside_polygon(
      const polygon& p,bool& p_inside_poly,bool& p_outside_poly);
   bool partial_overlap(polygon& p);
   bool linesegment_intersection_with_polygon(
      const linesegment& l,unsigned int& n_intersection_pnts,
      int intersected_poly_side[],threevector intersection_pnt[]) const;
   void locate_polygon_intersection_edges(
      int n_intersected_poly_sides,int& j,polygon& q,
      int intersected_poly_side[],threevector intersection_point[],
      threevector intersection_poly_vertex[]);
   bool test_intersection_polygon_vertex(
      int& j,const threevector& candidate_intersection_polygon_vertex,
      threevector* intersection_poly_vertex) const;

// Polygon plane member functions:

   plane* recompute_plane();
   plane* set_plane(const fourvector& input_pi);
   bool lies_on_plane(const plane& input_plane);

// Polygon projection member funtions:

   polygon xy_projection() const;
   polygon planar_projection(const threevector& n);
   polygon planar_projection(const plane& p) const;
   polygon* generate_bounding_box() const;
   double zvalue_for_xypoint_in_poly(const threevector& point_xy) const;
   bool point_behind_polygon(const threevector& point);
   bool ray_projected_into_poly_plane(
      const threevector& ray_basepoint,const threevector& ray_hat,
      const threevector& pnt_inside_poly,
      threevector& projected_point_in_plane);
   bool projection_into_xy_plane_along_ray(
      const threevector& ray_hat,polygon& xy_poly_projection);
   double polygon_difference(polygon& poly) const;

// Moving around polygon member functions:

   virtual void translate(const threevector& rvec);
   virtual void translate(const twovector& rvec);
   virtual void absolute_position(const threevector& rvec); 
   void extend(const threevector& extension_origin,double length);
   virtual void scale(double scalefactor);
   virtual void scale(const threevector& scale_origin,double scalefactor);
   virtual void scale(
      const threevector& scale_origin,const threevector& scalefactor);
   virtual void rotate(const rotation& R);
   virtual void rotate(const threevector& rotation_origin,
                       double thetax,double thetay,double thetaz);
   void rotate(
      const threevector& rotation_origin,const threevector& axis_direction,
      double alpha);
   virtual void rotate(const threevector& rotation_origin,const rotation& R);

// General Polygon Clipper member functions:

   gpc_vertex_list* generate_GPC_vertex_list_from_vertices();
   gpc_polygon* generate_GPC_polygon();

// Polygon triangulation member functions:

   triangles_group* generate_Delaunay_triangles();
   triangles_group* generate_interior_XY_triangles();
   triangles_group* generate_interior_triangles();

// =====================================================================
// Polygon vertex perturbation methods
// =====================================================================

   void delta_rotation(
      const threevector& rotation_origin,const threevector& axis_direction,
      double dtheta,std::vector<threevector>& dvertex);
   void delta_rotation_xy_proj(
      const threevector& rotation_origin,const threevector& axis_direction,
      double dtheta,std::vector<threevector>& dvertex_xy);
   void delta_rotation_max_xy_proj(
      const threevector& rotation_origin,const threevector& axis_direction,
      double dtheta,threevector& dvertex_max_xy);
   void frac_xycell_change_induced_by_rotation(
      const threevector& rotation_origin,const threevector& axis_direction,
      double dtheta,double deltax,double deltay,double& frac_change);
   double rot_to_induce_one_xycell_change(
      const threevector& rotation_origin,const threevector& axis_direction,
      double deltax,double deltay);

   void delta_scale(
      const threevector& scale_origin,const threevector& scalefactor,
      std::vector<threevector>& dvertex);
   void delta_scale_xy_proj(
      const threevector& scale_origin,const threevector& scalefactor,
      std::vector<threevector>& dvertex_xy);
   void delta_scale_max_xy_proj(
      const threevector& scale_origin,const threevector& scalefactor,
      threevector& dvertex_max_xy);
   void frac_xycell_change_induced_by_scaling(
      const threevector& scale_origin,const threevector& scalefactor,
      double deltax,double deltay,double& frac_change);
   double scaling_to_induce_one_xycell_change(
      const threevector& scale_origin,double deltax,double deltay);

   void compute_dvertex_max_xy(
      const std::vector<threevector>& dvertex_xy,threevector& dvertex_max_xy);
   void compute_frac_xycell_change(
      const threevector& dvertex_max_xy,double deltax,double deltay,
      double& frac_change);

  protected:

   bool occluded_flag;
   unsigned int ID,nvertices;
   double area,perimeter;
   linesegment* edge;
   plane* plane_ptr;

// Origin must be some point which lies inside polygon.  But it
// doesn't have to lie at the "center" of the polygon.  It is
// generally not possible to even define a "center" point for an
// arbitrary polygon:

   threevector origin,normal;
   threevector vertex_avg;
   threevector* vertex_ptr;

   void assign_vertex_array_pointers(int num_of_vertices);
   polygon(int num_of_vertices,const threevector currvertex[]);

  private: 

   enum Limits
   {
      MAX_STATIC_VERTICES=40
   };

// We follow Andrew Bradley's approach and work with two internal
// pointers to threevector arrays.  The first points to a relatively
// small stack which allows for fast access.  The second potentially
// points to an arbitrarily large heap which suffers from slower
// access.  The public vertex pointer is assigned to one of these two
// internal threevector array pointers depending upon the value of
// nvertices...

   threevector vertex_stack[MAX_STATIC_VERTICES];
   threevector* vertex_heap_ptr;

   triangles_group* triangles_group_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void docopy(const polygon& p);

   bool locate_origin();
   void guarantee_right_handed_vertex_ordering();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void polygon::set_occluded_flag(bool flag)
{
   occluded_flag=flag;
}

inline bool polygon::get_occluded_flag() const
{
   return occluded_flag;
}

// ---------------------------------------------------------------------
inline void polygon::set_ID(int ID)
{
   this->ID=ID;
}

inline int polygon::get_ID() const
{
   return ID;
}

// ---------------------------------------------------------------------
inline std::ostream& polygon::write_to_textstream(std::ostream& textstream)
{
   textstream << nvertices << " # n_vertices" << std::endl;
   for (unsigned int i=0; i<nvertices; i++)
   {
      textstream << get_vertex(i).get(0) << " "
                 << get_vertex(i).get(1) << " "
                 << get_vertex(i).get(2) << std::endl;
   }
   return textstream;
}

// ---------------------------------------------------------------------
inline void polygon::read_from_text_lines(
   unsigned int& i,std::vector<std::string>& line)
{
   int n_vertices;
   std::istringstream input_string_stream(line[i++]);
   input_string_stream >> n_vertices;

   std::vector<threevector> Vertices;
   for (int n=0; n<n_vertices; n++)
   {
      std::vector<double> V=stringfunc::string_to_numbers(line[i++]);
      Vertices.push_back(threevector(V[0],V[1],V[2]));
   }
   *this=polygon(Vertices);
}

// ---------------------------------------------------------------------
// Set and get member functions:

inline void polygon::set_nvertices(int n) 
{
   nvertices=n;
}

inline void polygon::set_area(double a) 
{
   area=a;
}

inline void polygon::set_perimeter(double p) 
{
   perimeter=p;
}

inline void polygon::set_origin(const threevector& o) 
{
   origin=o;
}

inline void polygon::set_normal(const threevector& n) 
{
   normal=n;
}

inline void polygon::set_vertex_avg(const threevector& va) 
{
   vertex_avg=va;
}

inline unsigned int polygon::get_nvertices() const
{
   return nvertices;
}

inline double polygon::get_area() const
{
   return area;
}

inline double polygon::get_perimeter() const
{
   return perimeter;
}

inline threevector& polygon::get_origin() 
{
   return origin;
}

inline threevector& polygon::get_normal() 
{
   return normal;
}

inline threevector& polygon::get_vertex_avg() 
{
   return vertex_avg;
}


inline void polygon::set_vertex(int n,const threevector& V)
{
   vertex_ptr[n]=V;
}

inline threevector& polygon::get_vertex(int n) const
{
   return vertex_ptr[n];
}


inline linesegment& polygon::get_edge(int n) 
{
   return edge[n];
}

inline triangles_group* polygon::get_triangles_group_ptr()
{
   return triangles_group_ptr;
}

inline const triangles_group* polygon::get_triangles_group_ptr() const
{
   return triangles_group_ptr;
}

// ---------------------------------------------------------------------
// Member function sidelengths computes the length of the nth side
// joining vertex n and n+1 and returns the result as array element
// length[n].  This dumb little method exists only for speed
// purposes....

inline void polygon::sidelengths(double length[]) const
{
   for (unsigned int i=0; i<nvertices; i++)
   {
      length[i]=(get_vertex(modulo(i+1,nvertices))-
      get_vertex(i)).magnitude();
   }
}

#endif  // polygon.h



