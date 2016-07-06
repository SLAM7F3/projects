// ==========================================================================
// Header file for deformable contour class
// ==========================================================================
// Last modified on 1/12/12; 1/26/12; 1/29/12; 4/4/14
// ==========================================================================

#ifndef CONTOUR_H
#define CONTOUR_H

#include <iostream>
#include <set>
#include <vector>
#include "geometry/linesegment.h"
#include "datastructures/Linkedlist.h"
#include "geometry/polygon.h"
#include "math/threevector.h"
#include "geometry/triangles_group.h"

class face;
class rotation;

class contour
{

  public:

// Initialization, constructor and destructor functions:

   contour(void);
   contour(int nvertices);
   contour(int nvertices,const threevector currvertex[]);
   contour(const std::vector<threevector>& vertices,
           bool vertices_lie_in_plane=true);
   contour(Linkedlist<threevector> const *perim_posn_list_ptr);
   contour(Linkedlist<std::pair<threevector,bool> > const *perim_list_ptr);
   contour(polygon const *poly_ptr);
   contour(const polyline& curr_polyline);
   contour(const face& f);
   contour(const contour& c);
   virtual ~contour();
   contour& operator= (const contour& c);
   friend std::ostream& operator<< (std::ostream& outstream,contour& c);
   friend std::ostream& operator<< (std::ostream& outstream,const contour& c);
   std::ostream& write_to_textstream(std::ostream& textstream);
   void read_from_text_lines(unsigned int& i,std::vector<std::string>& line,
                             bool vertices_lie_in_plane=true);

// Set and get member functions:

   void set_vertex(int n,const threevector& v,bool convex_vertex_flag=true);
   unsigned int get_nvertices() const;
   unsigned int get_nedges() const;
   double get_perimeter() const;
   double get_area() const;
   const threevector& get_origin() const;
   const threevector& get_normal() const ;
   const threevector& get_vertex_average() const; 
   Linkedlist<std::pair<threevector,bool> >* get_vertex_list_ptr();
   const Linkedlist<std::pair<threevector,bool> >* get_vertex_list_ptr() 
      const;
   std::pair<threevector,bool> get_vertex(int n) const;
   const linesegment& get_edge(int n) const;
   triangles_group* get_triangles_group_ptr();
   const triangles_group* get_triangles_group_ptr() const;

// Intrinsic properties methods:

   void guarantee_right_handed_vertex_ordering();
   bool right_handed_vertex_ordering() const;
   bool point_inside(const threevector& point,bool print_flag=false) const;
   bool point_within_contour(const threevector& point);
   bool point_within_contour(const twovector& point);

   double min_edge_length() const;
   threevector& vertex_average();
   void locate_extremal_xy_points(
      double& min_x,double& min_y,double& max_x,double& max_y) const;
   void identify_concave_vertices();
   bool is_convex() const;
   double compute_area();

// Contour edge methods:

   void initialize_edge_segments(bool compute_edges_flag=true);
   void compute_edges();
   double compute_perimeter();
   unsigned int edge_number(double frac) const;
   void edge_point(double frac,threevector& curr_point) const;
   threevector edge_point(double frac) const;
   int find_edge_containing_point(const threevector& currpoint) const;
   double frac_distance_along_contour(const threevector& currpoint);

   threevector radial_direction_vector(int edge_index) const;
   void regularize_vertices(double delta_s);

   double average_edge_orientation() const;
   double median_edge_orientation() const;
   std::vector<linesegment*>* consolidate_parallel_edges(
      double edge_angle_deviation=0);
   contour generate_consolidated_contour(double edge_angle_deviation=0);

   void align_edges_with_sym_dirs(
      const threevector& w_hat,const threevector& l_hat,int i_first=0,
      bool debug_flag=false);
   contour align_edges_with_sym_dirs(
      double avg_canonical_edge_orientation,double delta_theta,
      double edge_angle_deviation,double ds_min);
   void align_edges_with_sym_dirs(
      const threevector& w_hat,double delta_theta);

   Linkedlist<int>* locate_contour_corners(
      const threevector& w_hat,const threevector& l_hat);

// Contour triangulation methods:

   threevector& locate_origin();
   threevector& robust_locate_origin();
   triangles_group* generate_Delaunay_triangles();
   triangles_group* generate_interior_triangles();

// Distance to boundary methods:

   bool ray_pierces_me(
      double tan_elevation,const threevector& ray_basepoint,
      const threevector& ray_hat,int contour_vertex_skip);
   polygon compute_rectangle_sideface(int i,int vertex_skip=1);
   Linkedlist<polygon>* compute_vertices_voronoi_diagram() const;

// Orthogonal contour methods:

   Linkedlist<contour*>* 
      decompose_orthogonal_concave_contour_into_subcontours() const;
   Linkedlist<contour*>* 
      decompose_orthogonal_concave_contour_into_subcontours(
         linesegment& cleaving_line,
         Linkedlist<int>*& starting_right_vertex_indices_list_ptr,
         Linkedlist<int>*& starting_left_vertex_indices_list_ptr) const;
   contour* extract_largest_interior_rectangle() const;
   int anti_parallel_partner_edge(
      int edge_index,double& min_inter_edge_distance) const;
   linesegment orthogonal_concave_contour_cleaving_line(
      contour* rectangle_ptr) const;
   Linkedlist<std::pair<int,threevector> >* 
      starting_vertices_on_cleaving_line(
         const linesegment& cleaving_line) const;
   Linkedlist<threevector>* subcontour_vertices_to_right_of_cleaving_line(
      Mynode<std::pair<int,threevector> >* vertex_node_ptr,
      const linesegment& cleaving_line) const;

   contour* orthogonal_contour_bbox() const;
   void cleave_orthogonal_concave_contour(
      std::vector<contour*>& convex_contour) const;
   void cleave_orthogonal_concave_contour(contour*& c1_ptr,contour*& c2_ptr)
      const;
   double compute_orthogonal_contour_area();
   void compute_orthogonal_convex_contour_area();
   void compute_orthogonal_concave_contour_area();
   threevector orthogonal_concave_contour_COM() const;

// Contour manipulation member functions:

   void translate(const threevector& rvec);
   void scale(double scalefactor);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void xy_projection();
   double max_z_value() const;
   void filter_z_values(double correlation_length);
   void shrink_inwards(double delta_r);

  protected:

   unsigned int nvertices;
   double perimeter,area;
   threevector origin,normal,vertex_avg;
   Linkedlist<std::pair<threevector,bool> >* vertex_list_ptr;
   std::vector<linesegment> edge;

  private: 

   triangles_group* triangles_group_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void finish_construction();
   void docopy(const contour& c);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void contour::set_vertex(
   int n,const threevector& v,bool convex_vertex_flag) 
{
   std::pair<threevector,bool> p;
   p.first=v;
   p.second=convex_vertex_flag;	
   vertex_list_ptr->get_node(n)->set_data(p);
}

inline unsigned int contour::get_nvertices() const
{
   return nvertices;
}

inline unsigned int contour::get_nedges() const
{
   return edge.size();
}

inline double contour::get_perimeter() const
{
   return perimeter;
}

inline double contour::get_area() const
{
   return area;
}

inline const threevector& contour::get_origin() const
{
   return origin;
}

inline const threevector& contour::get_normal() const
{
   return normal;
}

inline const threevector& contour::get_vertex_average() const
{
   return vertex_avg;
}

inline Linkedlist<std::pair<threevector,bool> >* 
contour::get_vertex_list_ptr() 
{
   return vertex_list_ptr;
}

inline const Linkedlist<std::pair<threevector,bool> >* 
contour::get_vertex_list_ptr() const
{
   return vertex_list_ptr;
}

inline std::pair<threevector,bool> contour::get_vertex(int n) const
{
   return vertex_list_ptr->get_node(n)->get_data();
}

inline const linesegment& contour::get_edge(int n) const
{
   return edge[n];
}

inline triangles_group* contour::get_triangles_group_ptr()
{
   return triangles_group_ptr;
}

inline const triangles_group* contour::get_triangles_group_ptr() const
{
   return triangles_group_ptr;
}

// ---------------------------------------------------------------------
// Member function radial_direction_vector returns within output
// threevector r_hat a direction unit vector which points in the
// outwardly radial direction for the edge with label edge_index.

inline threevector contour::radial_direction_vector(int edge_index) const
{
   return edge[edge_index].get_ehat().cross(normal);
}

#endif  // contour.h


