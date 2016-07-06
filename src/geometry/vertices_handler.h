// ==========================================================================
// Header file for vertices_handler class
// ==========================================================================
// Last modified on 4/27/09; 5/18/13; 10/22/13; 4/4/14
// ==========================================================================

#ifndef VERTICES_HANDLER_H
#define VERTICES_HANDLER_H

#include <map>
#include <vector>
#include "datastructures/Hashtable.h"
#include "math/ltthreevector.h"
#include "geometry/vertex.h"

class vertices_handler
{

  public:

   vertices_handler(bool edge_flag=false);
   vertices_handler(const vertices_handler& v);
   ~vertices_handler();

   vertices_handler& operator= (const vertices_handler& v);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const vertices_handler& v);

// Set & get member functions:

   unsigned int get_n_vertices() const;
   unsigned int get_vertex_map_size() const;
   bool vertex_in_table(int Vertex_ID);

   vertex* get_vertex_ptr(int Vertex_ID);

// Note added on 10/22/13: We should globally eliminate the next two
// get_vertex() methods in favor of the prevous get_vertex_ptr()
// method.  We can then explicitly test whether a vertex corresponding
// to the requested Vertex_ID actually exists by checking that the
// returned pointer is non-null...

   vertex& get_vertex(int Vertex_ID);
   const vertex& get_vertex(int Vertex_ID) const;

// Vertex chain member functions:

   void clear_vertex_chain();
   void set_chain_vertex(int i,const vertex& V);
   vertex& get_vertex_from_chain(int i);
   const vertex& get_vertex_from_chain(int i) const;

// Vertex table member functions:

   void purge_vertex_table();
   void update_vertex_table_key(const vertex& V);

// Vertex map member functions:

   void clear_vertex_map();
   vertex* get_vertex_ptr(const threevector& posn);
   void update_vertex_map(const vertex& V);
//   bool reset_vertex_posn(
//      const threevector& old_posn,const threevector& new_posn);
   bool reset_vertex_posn(
      threevector old_posn,const threevector& new_posn);
   void print_vertex_map_contents();

// Vertex container member functions:

   void clear_all_containers();
   void update_vertex_in_table_chain_and_map(const vertex& V);
   
  private: 

   std::vector<int> vertex_IDs_chain;	
     // vertex_IDs_chain independent var = vertex chain location index
     // dependent var = vertex ID
   std::vector<vertex> vertex_table;	// for 2 edge vertices
   Hashtable<vertex>* vertex_table_ptr; // for > 2 face or polyhedron vertices
     // *vertex_table_ptr independent var = vertex ID
     // dependent var = vertex

   typedef std::map<threevector,int,ltthreevector > VERTEX_MAP;
   VERTEX_MAP* vertex_map_ptr;
     // *vertex_map_ptr independent var = vertex posn
     // dependent var = vertex ID

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const vertices_handler& v);

   void update_vertex_ID_in_chain(const vertex& V);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int vertices_handler::get_n_vertices() const
{
   return vertex_IDs_chain.size();
}
 
inline unsigned int vertices_handler::get_vertex_map_size() const
{
   return vertex_map_ptr->size();
}

#endif  // vertices_handler.h
