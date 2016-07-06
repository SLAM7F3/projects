// ==========================================================================
// Header file for triangle class 
// ==========================================================================
// Last modified on 1/26/12; 2/29/12; 10/18/13
// ==========================================================================

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>
#include "geometry/edge.h"
#include "geometry/vertex.h"

class bounding_box;
class polygon;

class triangle
{

  public:

   triangle(const std::vector<vertex>& Vertices,int ID=-1);
   triangle(const triangle& t);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~triangle();
   triangle& operator= (const triangle& t);
   friend std::ostream& operator<< 
      (std::ostream& outstream,triangle& t);

// Set and get member functions:

   int get_ID() const;
   double get_area();
   void set_vertex(int v,const vertex& curr_vertex);
   void set_vertex_posn(int v,const threevector& posn);
   vertex& get_vertex(int v);
   const vertex& get_vertex(int v) const;
   vertex* get_vertex_ptr(int v);
   edge& get_edge(int e);
   const edge& get_edge(int e) const;
   polygon* get_triangle_poly_ptr();
   const polygon* get_triangle_poly_ptr() const;

// Triangle properties member functions:

   double compute_area();
   bool vertices_inside_bbox(const bounding_box& bbox);

// Triangle manipulation member functions:

   void reset_all_Z_values(double z);
   polygon* generate_polygon();
   void delete_triangle_poly_ptr();

  private:

   int ID;
   double area;
   std::vector<vertex> Vertices;
   std::vector<edge> Edges;

   polygon* triangle_poly_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const triangle& d);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int triangle::get_ID() const
{
   return ID;
}

inline double triangle::get_area()
{
   if (area < 0) compute_area();
   return area;
}

inline void triangle::set_vertex(int v,const vertex& curr_vertex)
{
   Vertices[v]=curr_vertex;
}

inline void triangle::set_vertex_posn(int v,const threevector& posn)
{
   Vertices[v].set_posn(posn);
}

inline vertex& triangle::get_vertex(int v)
{
   return Vertices[v];
}

inline const vertex& triangle::get_vertex(int v) const
{
   return Vertices[v];
}

inline vertex* triangle::get_vertex_ptr(int v)
{
   return &(Vertices[v]);
}

inline edge& triangle::get_edge(int e)
{
   return Edges[e];
}

inline const edge& triangle::get_edge(int e) const
{
   return Edges[e];
}

inline polygon* triangle::get_triangle_poly_ptr()
{
   return triangle_poly_ptr;
}

inline const polygon* triangle::get_triangle_poly_ptr() const
{
   return triangle_poly_ptr;
}

#endif  // triangle.h
