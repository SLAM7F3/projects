// ==========================================================================
// Header file for stand-alone geometry methods
// ==========================================================================
// Last updated on 5/9/13; 8/8/13; 10/18/13; 8/5/16
// ==========================================================================

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "geometry/gpc.h"
#include "geometry/linesegment.h"
#include "geometry/plane.h"
#include "geometry/polyline.h"
#include "math/threevector.h"

class bounding_box;
class contour;
class parallelepiped;
class polygon;
class rotation;
class vertices_handler;
template<class T> class Linkedlist;

namespace geometry_func
{

// Moving around threevector methods:

   void translate(threevector& V,const threevector& rvec);
   void scale(threevector& V,const threevector& scale_origin,
              const threevector& scalefactor);
   void rotate(threevector& V,const threevector& rotation_origin,
	       const rotation& R);
   
// Point-to-line methods:

   double point_to_line_distance(
      const threevector& curr_point,
      const threevector& line_basepoint,const threevector& line_e_hat);
   double line_to_line_squared_distance(
      const threevector& v1,const threevector& e_hat,
      const threevector& w1,const threevector& f_hat);

// Triangle methods:
   
   double compute_triangle_area(
      const threevector& v1,const threevector& v2,const threevector& v3);
   threevector compute_triangle_COM(
      const threevector& v1,const threevector& v2,const threevector& v3);
   bool SameHalfPlane(const threevector& P,const threevector& Q,
                      const threevector& V1,const threevector& V2);
   bool PointInsideTriangle(
      const threevector& P,const threevector& V1,
      const threevector& V2,const threevector& V3);
   bool Point_in_Triangle(
      const twovector& P,const twovector& V1,
      const twovector& V2,const twovector& V3);
   bool Point_in_Triangle(
      const threevector& P,const threevector& V1,
      const threevector& V2,const threevector& V3);

   bool ray_intersects_Triangle(
      polygon* triangle_ptr,const threevector& ray_basept,
      const threevector& r_hat);
   bool ray_intersects_Triangle(
      polygon* triangle_ptr,const threevector& ray_basept,
      const threevector& r_hat,threevector& intersection_pt);

// Contour methods:

   bool right_handed_vertex_ordering(const std::vector<threevector>& vertex,
                                     const threevector& normal);
   void remove_short_contour_edges(double min_edge_length,contour*& c_ptr);
   bool remove_short_contour_edges(
      double min_edge_length,Linkedlist<threevector>* vertex_list_ptr);
   void simplify_orthogonal_contour(
      Linkedlist<threevector>*& vertex_list_ptr);
   void eliminate_nearly_degenerate_intersection_point_triples(
      double min_distance,Linkedlist<threevector>*& perim_posn_list_ptr);
   void decompose_orthogonal_concave_contour_into_large_rectangles(
      Linkedlist<contour*>*& subcontour_list_ptr);
   double abutting_contours(contour const *c1_ptr,contour const *c2_ptr,
                            int& abutting_edge1,int& abutting_edge2);
   contour* rectangle_contours_bounding_box(
      contour const *c1_ptr,contour const *c2_ptr,
      int abutting_edge1,int abutting_edge2);
   void consolidate_rectangle_subcontours(
      Linkedlist<contour*>*& subcontour_list_ptr);
   void delete_small_rectangle_subcontours(
      contour* contour_ptr,Linkedlist<contour*>*& subcontour_list_ptr);

// Contour decomposition methods:

   std::vector<polyline> decompose_contour_into_subcontours(
      const std::vector<threevector>& contour_corners);
   polyline cleave_subcontour(
      const std::vector<threevector>& contour_corners,
      std::vector<threevector>& reduced_corners);

// Writing and reading geometrical objects to I/O files:

   void write_contour_info_to_file(
      contour const *c_ptr,std::string contour_filename);
   int nvertices_in_contour_file(std::string contour_filename);
   contour* read_contour_info_from_file(std::string contour_filename);

// Planar methods:

   linesegment fit_line_to_multiple_planes(const std::vector<plane>& P);
   threevector intersection_of_three_planes(const plane& P0,const plane& P1,
                                            const plane& P2);

// 2D line methods:

   threevector closest_point_to_origin(const threevector& l);

// 3D line methods:

//   fourvector threeD_line_thru_two_points(
//      const threevector& r1,const threevector& r2);
   bool multi_line_intersection_point(
      const std::vector<linesegment>& lines,threevector& intersection_point);
   bool multi_line_intersection_point(
      const std::vector<linesegment>& lines,
      double sigma_theta,double sigma_phi,
      threevector& intersection_point,threevector& sigma_intersection_point);

// General Polygon Clipper (GPC) methods:

   gpc_vertex* generate_GPC_vertex(const threevector& V);
   void destroy_GPC_vertex(gpc_vertex* gpc_vertex_ptr);

   gpc_vertex_list* generate_GPC_vertex_list(
      const std::vector<threevector>& vertices);
   void destroy_GPC_vertex_list(gpc_vertex_list* gpc_vertex_list_ptr);

   void destroy_GPC_polygon(gpc_polygon* gpc_poly_ptr);

   void print_GPC_vertex(gpc_vertex* gpc_vertex_ptr);
   void print_GPC_vertex_list(gpc_vertex_list* gpc_vertex_list_ptr);
   void print_GPC_polygon(gpc_polygon* gpc_poly_ptr);

   std::vector<polygon> convert_GPC_to_polygons(gpc_polygon* gpc_polygon_ptr);
   std::vector<polygon> polygon_union(polygon& poly1,polygon& poly2);
   std::vector<polygon> polygon_union(
      polygon& new_poly,std::vector<polygon>& old_polys);
   std::vector<polygon> polygon_union(std::vector<polygon>& input_polys);

   std::vector<polygon> polygon_intersection(polygon& poly1,polygon& poly2);
   polygon* polygon_intersection(std::vector<polygon>& polys);
   polygon* polygon_intersection(const std::vector<polygon*>& poly_ptrs);

   std::vector<polygon> polygon_difference(polygon& poly1,polygon& poly2);
   std::vector<polygon> polygon_difference(
      std::vector<polygon>& polys,polygon& poly2);
   std::vector<polygon> polygon_difference(
      std::vector<polygon>& polys,std::vector<polygon>& qolys);

// Bounding methods

   void compute_bounding_sphere(
      const std::vector<threevector>& V,
      threevector& sphere_center,double& sphere_radius);
   void compute_bounding_sphere(
      vertices_handler* vertices_handler_ptr,
      threevector& sphere_center,double& sphere_radius);
   void compute_bounding_box(
      vertices_handler* vertices_handler_ptr,bounding_box& bbox);
   void write_bboxes_to_file(std::ofstream& outstream, 
                             std::vector<bounding_box>& curr_image_bboxes);

// Convex hull methods

   polygon* compute_convex_hull(const std::vector<twovector>& V);

// Homogeneous 2-vector methods

   threevector homogeneous_line_coords(
      const twovector& UV1,const twovector& UV2);
   twovector homogeneous_lines_intersection(
      const threevector& l1,const threevector& l2);

// Ellipse methods

   bool point_on_ellipse(
      double a,double b,double c,double xcenter,double ycenter,
      const twovector& point);
   bool point_inside_ellipse(
      double a,double b,double c,double xcenter,double ycenter,
      const twovector& point);
   bool quadratic_form_to_ellipse_params(
      double a,double b,double c,
      double& major_axis,double& minor_axis,double& major_axis_phi);
   std::vector<twovector> ellipse_points(
      int n_points,double a,double b,double c,double xcenter,double ycenter);

// ==========================================================================
// Inlined methods:
// ==========================================================================
   
} // geometry_func namespace

#endif  // geometry_funcs.h
