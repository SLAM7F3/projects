// ==========================================================================
// Header file for polyline class
// ==========================================================================
// Last modified on 2/9/11; 11/7/11; 6/28/12; 4/4/14
// ==========================================================================

#ifndef POLYLINE_H
#define POLYLINE_H

#include <iostream>
#include <vector>
#include "math/fourvector.h"
#include "kdtree/kdtree.h"
#include "geometry/linesegment.h"

class polyline
{

  public:

// Initialization, constructor and destructor functions:

   polyline();
   polyline(const std::vector<threevector>& V);
   polyline(const threevector& l);
   polyline(const polyline& p);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~polyline();
   polyline& operator= (const polyline& p);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const polyline& p);

// Set and get member functions:

   unsigned int get_n_vertices() const;
   threevector& get_vertex(unsigned int n) const;
   threevector& get_last_vertex() const;

   unsigned int get_n_edges() const;
   linesegment& get_edge(int n);
   std::vector<double>& get_first_edge_2D_line_coeffs();
   const std::vector<double>& get_first_edge_2D_line_coeffs() const;
   double get_total_length() const;
   void set_regular_vertex_spacing(double ds);
   double get_regular_vertex_spacing() const;
   KDTree::KDTree<3, fourvector>* get_kdtree_ptr();
   const KDTree::KDTree<3, fourvector>* get_kdtree_ptr() const;

// Polyline manipulation member functions:

   void reset_vertex_altitudes(double constant_vertices_altitude);

// Intrinsic polyline properties:

   double compute_total_length();
   void compute_first_edge_2D_line_coeffs();
   threevector compute_vertices_COM() const;

// Distance to point member functions:

   double distance_to_initial_vertex(const threevector& currpoint) const;
   double distance_to_final_vertex(const threevector& currpoint) const;
   double min_distance_to_point(const threevector& currpoint);
   double min_distance_to_point(
      const threevector& currpoint,threevector& closest_point_on_polyline);
   double min_distance_to_point(
      const threevector& currpoint,threevector& closest_point_on_polyline,
      threevector& e_hat);
   double min_sqrd_distance_to_point(
      const threevector& currpoint,const threevector& V1,
      const threevector& V2);
   double min_sqrd_XY_distance_to_point(
      const threevector& currpoint,const threevector& V1,
      const threevector& V2);
   double min_sqrd_XY_distance_to_point(
      const threevector& currpoint,double approx_range_to_polyline);
   double min_sqrd_distance_to_ray(
      const threevector& grid_intercept_posn,
      const threevector& ray_basepoint,const threevector& ray_ehat,
      double approx_range_to_polyline);

// Multi-polyline intersection methods:

   std::vector<threevector> intersection_points_with_another_polyline(
      polyline& p,double max_dotproduct=0.99,
      double endpoint_distance_threshold=1E-8);

// Polyline edge traversing methods:

   void initialize_edge_segments();
   int edge_number(double frac);
   threevector edge_point(double frac);
   threevector edge_point_at_distance_along_polyline(double s);
   threevector edge_direction(double frac);
   int find_edge_containing_point(const threevector& currpoint) const;
   double frac_distance_along_polyline(const threevector& currpoint);
   double distance_along_polyline(const threevector& currpoint);
   std::vector<threevector> vertices_in_frac_interval(
      double f_start,double f_stop);

// Edge point resampling member functions:

   void compute_regularly_spaced_edge_points(
      double ds,std::vector<threevector>& V);
   void compute_regularly_spaced_edge_points(
      double ds,std::vector<threevector>& V,
      bool include_original_vertices_flag);
   void compute_regularly_spaced_edge_points(
      unsigned int nsamples,std::vector<threevector>& V,
      bool include_original_vertices_flag=false);
   std::vector<double> compute_irregularly_spaced_edge_fracs(
      double ds,double ds_fine);

   void generate_sampled_edge_points_kdtree();
   void generate_sampled_edge_points_kdtree(double ds);
   std::vector<threevector> smooth_raw_vertices(
      double ds,const std::vector<threevector>& raw_vertices);
   void smooth_raw_vertices(double ds);


  private:

   unsigned int n_vertices;
   double total_length,regular_vertex_spacing;
   std::vector<double> first_edge_2D_line_coeffs;
   threevector* vertex;
   std::vector<linesegment> edge;

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
// n_vertices...

   threevector vertex_stack[MAX_STATIC_VERTICES];
   threevector* vertex_heap_ptr;

   std::vector<fourvector> sampled_edge_points;
   KDTree::KDTree<3, fourvector>* kdtree_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const polyline& p);

   void assign_vertex_array_pointers(int num_of_vertices);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int polyline::get_n_vertices() const
{
   return n_vertices;
}

inline threevector& polyline::get_vertex(unsigned int n) const
{
   if (n >= n_vertices)
   {
      std::cout << "Error in polyline::get_vertex()!" << std::endl;
      std::cout << "Input n = " << n << " n_vertices = " << n_vertices
                << std::endl;
      exit(-1);
   }
   return vertex[n];
}

inline threevector& polyline::get_last_vertex() const
{
   return vertex[get_n_vertices()-1];
}

inline unsigned int polyline::get_n_edges() const
{
   return edge.size();
}

inline linesegment& polyline::get_edge(int n) 
{
   return edge[n];
}

inline std::vector<double>& polyline::get_first_edge_2D_line_coeffs()
{
   return first_edge_2D_line_coeffs;
}

inline const std::vector<double>& polyline::get_first_edge_2D_line_coeffs() 
                 const
{
   return first_edge_2D_line_coeffs;
}

inline double polyline::get_total_length() const
{
   return total_length;
}

inline void polyline::set_regular_vertex_spacing(double ds)
{
   regular_vertex_spacing=ds;
}

inline double polyline::get_regular_vertex_spacing() const
{
   return regular_vertex_spacing;
}

inline KDTree::KDTree<3, fourvector>* polyline::get_kdtree_ptr()
{
   return kdtree_ptr;
}

inline const KDTree::KDTree<3, fourvector>* polyline::get_kdtree_ptr() const
{
   return kdtree_ptr;
}

inline double polyline::distance_to_initial_vertex(
   const threevector& currpoint) const
{
   return (currpoint-vertex[0]).magnitude();
}

inline double polyline::distance_to_final_vertex(
   const threevector& currpoint) const
{
   return (currpoint-vertex[n_vertices-1]).magnitude();
}
 
#endif  // polyline.h



