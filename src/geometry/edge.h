// ==========================================================================
// Header file for edge class
// ==========================================================================
// Last modified on 1/8/12; 1/27/12; 1/29/12
// ==========================================================================

#ifndef EDGE_H
#define EDGE_H

#include <vector>
#include "geometry/vertices_handler.h"
#include "geometry/vertex.h"

class rotation;

class edge
{

  public:

   edge(int id=-1);
   edge(const vertex& V1,const vertex& V2,int id=-1);
   edge(const edge& e);
   ~edge();
   edge& operator= (const edge& e);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const edge& e);

// Set and get member functions:

   void set_ID(int id);
   int get_ID() const;
   void set_internal_edge_flag(bool flag);
   bool get_internal_edge_flag() const;

   vertices_handler* get_vertices_handler_ptr();
   const vertices_handler* get_vertices_handler_ptr() const;
   vertex& get_V1();
   const vertex& get_V1() const;
   vertex& get_V2();
   const vertex& get_V2() const;

   threevector get_ehat() const;
   double get_length() const;
   threevector get_frac_posn(double frac) const;

// Vertex map member functions:

   void update_vertex_map();
   void reset_vertex_posn(
      const threevector& old_posn,const threevector& new_posn);

// Comparison member functions:

   void swap_vertices();
   bool nearly_equal_posn(const edge& e);

// Above Z-plane member functions:

   bool above_Zplane(double z);
   bool below_Zplane(double z);
   bool intersection_with_Zplane(double z,threevector& intersection_posn);

// Moving around edge member functions:

   void translate(const threevector& rvec);
   void absolute_position(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

  private: 

   int ID;
   bool internal_edge_flag; 
	// true if edge does NOT represent polyhedron border
   vertices_handler* vertices_handler_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const edge& p);

   void set_V1(const vertex& V);
   void set_V2(const vertex& V);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void edge::set_ID(int id)
{
   ID=id;
}

inline int edge::get_ID() const
{
   return ID;
}

inline void edge::set_internal_edge_flag(bool flag) 
{
   internal_edge_flag=flag;
}

inline bool edge::get_internal_edge_flag() const
{
   return internal_edge_flag;
}

// --------------------------------------------------------------------------
inline vertices_handler* edge::get_vertices_handler_ptr()
{
   return vertices_handler_ptr;
}

inline const vertices_handler* edge::get_vertices_handler_ptr() const
{
   return vertices_handler_ptr;
}

inline void edge::set_V1(const vertex& V)
{
//   std::cout << "inside edge::set_V1, V = " << V << std::endl;
   if (vertices_handler_ptr->get_n_vertices()==2)
   {
      vertices_handler_ptr->set_chain_vertex(0,V);
//      vertices_handler_ptr->update_vertex_table_key(V);
//      vertices_handler_ptr->update_vertex_map(V);
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(V);
   }
}

inline vertex& edge::get_V1()
{
   return vertices_handler_ptr->get_vertex_from_chain(0);
}

inline const vertex& edge::get_V1() const
{
   return vertices_handler_ptr->get_vertex_from_chain(0);
}

inline void edge::set_V2(const vertex& V)
{
//   std::cout << "inside edge::set_V2, V = " << V << std::endl;
   if (vertices_handler_ptr->get_n_vertices()==2)
   {
      vertices_handler_ptr->set_chain_vertex(1,V);
//      vertices_handler_ptr->update_vertex_table_key(V);
//      vertices_handler_ptr->update_vertex_map(V);
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(V);
   }
}

inline vertex& edge::get_V2()
{
   return vertices_handler_ptr->get_vertex_from_chain(1);
}

inline const vertex& edge::get_V2() const
{
   return vertices_handler_ptr->get_vertex_from_chain(1);
}

// Member function get_ehat returns the direction vector pointing from
// V1 to V2:

inline threevector edge::get_ehat() const
{
   return (get_V2().get_posn()-get_V1().get_posn()).unitvector();
}

inline double edge::get_length() const
{
   return (get_V2().get_posn()-get_V1().get_posn()).magnitude();
}

// Above Z-plane member functions

// Boolean member function above[below]_Zplane returns true if the entire
// edge lies above [below] or on the plane Z=z.  

inline bool edge::above_Zplane(double z)
{
   return (get_V1().above_Zplane(z) && get_V2().above_Zplane(z));
}

inline bool edge::below_Zplane(double z)
{
   return (get_V1().below_Zplane(z) && get_V2().below_Zplane(z));
}


#endif  // edge.h
