// ==========================================================================
// Header file for triangles_group class
// ==========================================================================
// Last modified on 1/12/12; 1/26/12; 10/20/13; 10/22/13; 4/4/14
// ==========================================================================

#ifndef TRIANGLES_GROUP_H
#define TRIANGLES_GROUP_H

#include <map>
#include <string>
#include <vector>

#include "geometry/triangle.h"
#include "geometry/vertices_handler.h"

class triangles_group 
{

  public:

   typedef std::map<int,triangle* > TRIANGLES_MAP;
   typedef std::map<int,threevector> DELAUNAY_TRIANGLES_VERTEX_MAP;

   typedef std::map<int,std::vector<int>* > VERTICES_NETWORK_MAP;
//   Independent int variable = vertex ID
//   Dependent vector variable holds IDs for all neighboring vertices

// Initialization, constructor and destructor functions:

   triangles_group();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~triangles_group();

// Set & get member functions:

   unsigned int get_n_vertices() const;
//    vertex& get_vertex(int vertex_ID);
   vertex* get_vertex_ptr(int vertex_ID);
   unsigned int get_n_triangles() const;
   triangle* get_triangle_ptr(int i);

// Triangle generation member functions:

   triangle* generate_new_triangle(
      const threevector& v1,const threevector& v2,const threevector& v3,
      int triangle_ID=-1);
   triangle* generate_new_triangle(
      const std::vector<vertex>& vertices,int triangle_ID=-1);

// Delaunay triangulation member functions:

   void update_triangle_vertices(const vertex& curr_Vertex);
   void delaunay_triangulate_vertices();
   const std::vector<int>* get_neighboring_vertex_IDs(int curr_vertex_ID) 
      const;

// Interior triangulation member functions:

   void inner_triangulate_vertices(double specified_Z=0);
   void reset_all_Z_values(double z);
   
// Collective triangle properties member functions:

   double compute_area_integral();
   std::vector<double> triangle_edge_lengths();

  private:
   
   vertices_handler* vertices_handler_ptr;
   TRIANGLES_MAP* triangles_map_ptr;
   DELAUNAY_TRIANGLES_VERTEX_MAP* delaunay_triangles_vertex_map_ptr;
   VERTICES_NETWORK_MAP* vertices_network_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const triangles_group& tg);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline bool triangles_group::get_vertex(int vertex_ID,vertex& curr_vertex)
{
   if (vertices_handler_ptr->vertex_in_table(vertex_ID))
   {
      curr_vertex=vertices_handler_ptr->get_vertex(vertex_ID);
      return true;
   }
   else
   {
      return false;
   }
}
*/

inline vertex* triangles_group::get_vertex_ptr(int vertex_ID)
{
   return vertices_handler_ptr->get_vertex_ptr(vertex_ID);
}

#endif  // triangles_group.h



