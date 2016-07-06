// ==========================================================================
// Header file for face class
// ==========================================================================
// Last modified on 9/30/09; 1/8/12; 1/29/12; 4/4/14
// ==========================================================================

#ifndef FACE_H
#define FACE_H

#include <vector>
#include "geometry/edge.h"
#include "geometry/polygon.h"
#include "geometry/vertices_handler.h"

class rotation;

class face
{

  public:

   enum HandednessType
   {
      right_handed=0,left_handed,unknown
   };

   face(int id=-1);
   face(const threevector& p1,const threevector& p2,const threevector& p3,
        int id=-1);
   face(const threevector& p1,const threevector& p2,const threevector& p3,
        const threevector& p4,int id=-1);
   face(const std::vector<threevector>& p,int id=-1);
   face(const edge& e1,const edge& e2,const edge& e3,int id=-1);
   face(const edge& e1,const edge& e2,const edge& e3,const edge& e4,
        int id=-1);
   face(const std::vector<edge>& input_edges,int id=-1);
   face(const face& f);
   ~face();
   face& operator= (const face& f);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const face& f);

// Vertices_handler set & get member functions:

   unsigned int get_n_vertices() const;
   vertex& get_vertex_from_chain(int i);
   const vertex& get_vertex_from_chain(int i) const;
   vertices_handler* get_vertices_handler_ptr();
   const vertices_handler* get_vertices_handler_ptr() const;

// Set and get member functions:

   void set_ID(int id);
   int get_ID() const;
   void set_handedness(HandednessType h);
   HandednessType get_handedness() const;
   double get_area() const;

   unsigned int get_n_edges() const;
   edge& get_edge(int n);
   const edge& get_edge(int n) const;
   int get_edge_ID(int n) const;
   std::vector<edge>* get_edge_chain_ptr();
   std::vector<edge>& get_edge_chain();
   const std::vector<edge>& get_edge_chain() const;

   threevector& get_origin();
   const threevector& get_origin() const;
   threevector& get_COM();
   const threevector& get_COM() const;
   threevector& get_normal();
   const threevector& get_normal() const;
   polygon get_polygon();

// Construction methods:

   bool build_face(const std::vector<edge>& input_edges);
   polygon& compute_face_poly();
   HandednessType handedness_wrt_direction(const threevector& p_hat);
   void force_handedness_wrt_direction(
      HandednessType desired_handedness,const threevector& p_hat);
   void swap_vertex_order();

// Vertex map member functions:

   void update_vertex_map();
   void reset_vertex_posn(
      const threevector& old_posn,const threevector& new_posn);

// Search member functions:

   edge* find_edge_in_chain_given_edge(const edge& input_edge);
   bool point_inside_convex_face(const threevector& point) const;
   bool ray_intercept(
      const threevector& ray_basepoint,const threevector& ray_hat,
      threevector& projected_point_in_face_plane);

// Above Z-plane member functions:

   bool above_Zplane(double z);
   bool below_Zplane(double z);
   bool part_above_Zplane(double z,face& face_part_above_Zplane);
   
// Moving around face member functions:

   void translate(const threevector& rvec);
   void absolute_position(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

// Triangulation member functions:

   bool triangles_above_Zplane(
      double z,std::vector<face>& triangle_parts_above_Zplane);
   void triangulate(std::vector<face>& face_triangles);

   void compute_area_COM_and_normal();

  private: 

   int ID;
   HandednessType handedness;
   double area;
   threevector COM,normal;
   polygon face_poly;
   std::vector<edge> edge_chain;
   vertices_handler* vertices_handler_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const face& p);

   void compute_vertex_chain(const std::vector<edge>& init_edges);
   std::vector<edge>& compute_edge_chain_from_vertex_chain();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Vertices_handler member functions:

inline unsigned int face::get_n_vertices() const
{
   return vertices_handler_ptr->get_n_vertices();
}

inline vertex& face::get_vertex_from_chain(int i)
{
   return vertices_handler_ptr->get_vertex_from_chain(i);
}

inline const vertex& face::get_vertex_from_chain(int i) const
{
   return vertices_handler_ptr->get_vertex_from_chain(i);
}

inline vertices_handler* face::get_vertices_handler_ptr()
{
   return vertices_handler_ptr;
}

inline const vertices_handler* face::get_vertices_handler_ptr() const
{
   return vertices_handler_ptr;
}

// --------------------------------------------------------------------------

inline void face::set_ID(int id)
{
   ID=id;
}

inline int face::get_ID() const
{
   return ID;
}

inline void face::set_handedness(face::HandednessType h)
{
   handedness=h;
}

inline face::HandednessType face::get_handedness() const
{
   return handedness;
}

inline double face::get_area() const
{
   return area;
}

inline unsigned int face::get_n_edges() const
{
   return edge_chain.size();
}

inline edge& face::get_edge(int n) 
{
   return edge_chain[n];
}

inline const edge& face::get_edge(int n) const
{
   return edge_chain[n];
}

inline int face::get_edge_ID(int n) const
{
   return edge_chain[n].get_ID();
}

inline std::vector<edge>* face::get_edge_chain_ptr()
{
   return &edge_chain;
}

inline std::vector<edge>& face::get_edge_chain()
{
   return edge_chain;
}

inline const std::vector<edge>& face::get_edge_chain() const
{
   return edge_chain;
}

inline threevector& face::get_origin()
{
   return COM;
}

inline const threevector& face::get_origin() const
{
   return COM;
}

inline threevector& face::get_COM()
{
   return COM;
}

inline const threevector& face::get_COM() const
{
   return COM;
}

inline threevector& face::get_normal()
{
   return normal;
}

inline const threevector& face::get_normal() const
{
   return normal;
}

// Above Z-plane methods

inline bool face::above_Zplane(double z)
{
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      if (get_vertex_from_chain(v).below_Zplane(z)) return false;
   }
   return true;
}

inline bool face::below_Zplane(double z)
{
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      if (get_vertex_from_chain(v).above_Zplane(z)) return false;
   }
   return true;
}

#endif  // face.h
