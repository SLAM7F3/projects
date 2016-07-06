// ==========================================================================
// Header file for polyhedron class
// ==========================================================================
// Last modified on 6/13/12; 5/18/13; 8/4/13; 4/4/14
// ==========================================================================

#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "trimesh2/TriMesh.h"

#include "geometry/bounding_box.h"
#include "geometry/edge.h"
#include "geometry/face.h"
#include "math/fourvector.h"
#include "math/ltthreevector.h"
#include "network/Network.h"
#include "math/rotation.h"
#include "geometry/vertex.h"
#include "geometry/vertices_handler.h"

class contour;
class polyline;

class polyhedron
{

  public:

   typedef std::map<int,int> TRIANGLEFACE_RECTANGLE_MAP;

// Indep var for TRIANGLEFACE_RECTANGLE_MAP holds triangular face ID.
// Depend var for TRIANGLEFACE_RECTANGLE_MAP holds corresponding
// rectangle sideface ID.

// Initialization, constructor and destructor functions:

   polyhedron(int id=-1);
   polyhedron(const std::vector<threevector>& V,int id=-1);
   polyhedron(
      const threevector& origin,const std::vector<vertex>& V,
      const std::vector<edge>& E,const std::vector<face>& F,int id=-1);
   polyhedron(const polyhedron& p);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~polyhedron();
   polyhedron& operator= (const polyhedron& p);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const polyhedron& e);

// Set and get member functions:

   int get_ID() const;
   void set_origin(const threevector& origin);
   threevector& get_origin();
   const threevector& get_origin() const;
   std::vector<std::pair<threevector,threevector> >& 
      get_surfacepnts_normals();
   void get_bounding_sphere(threevector& center,double& radius);
   threevector& get_tip();
   const threevector& get_tip() const;
   threevector& get_base();
   const threevector& get_base() const;
   const bounding_box& get_bbox() const;
   TRIANGLEFACE_RECTANGLE_MAP* get_triangleface_rectangle_map_ptr();
   const TRIANGLEFACE_RECTANGLE_MAP* get_triangleface_rectangle_map_ptr()
      const ;

// Vertex member functions:

   unsigned int get_n_vertices() const;
   void set_vertices(const std::vector<threevector>& V);
   void set_vertices(const std::vector<vertex>& V);
   vertex& get_vertex(unsigned int n);
   const vertex& get_vertex(unsigned int n) const;
   vertices_handler* get_vertices_handler_ptr();
   const vertices_handler* get_vertices_handler_ptr() const;

// Edge member functions:

   unsigned int get_n_edges() const;
   unsigned int get_n_external_edges() const;
   int set_edge(
      unsigned int vertex_index1,unsigned int vertex_index2,
      bool internal_edge_flag=false,int edge_ID=-1);
   void set_edges(const std::vector<edge>& E);
   edge* get_edge_ptr(unsigned int edge_index);
   const edge* get_edge_ptr(unsigned int edge_index) const;
   vertex& get_edge_V1(int edge_ID);
   vertex& get_edge_V2(int edge_ID);
   bool get_internal_edge_flag(int edge_ID);

// Face member functions:

   face::HandednessType get_faces_handedness() const;
   unsigned int get_n_faces() const;
   void set_face(int i,int j,int k,int face_ID=-1);
   void set_face(int i,int j,int k,int l,int face_ID);
   void set_face(const std::vector<int>& edge_indices,int face_ID=-1);
   void set_face(const std::vector<edge>& face_edges,int face_ID=-1);
   void set_faces(const std::vector<face>& F);
   face* get_face_ptr(unsigned int face_index);
   const face* get_face_ptr(unsigned int face_index) const;
   void compute_faces_handedness();
   void ensure_faces_handedness(face::HandednessType desired_handedness);
   void ensure_face_handedness(
      face* curr_face_ptr,face::HandednessType desired_handedness);
   void instantiate_face_network();
   
   std::vector<face*> extract_zplane_faces(double z);
   polygon* compute_zplane_footprint(double z);

// Vertex map member functions:
   
   void update_vertex_map();
   void reset_vertex_posn(
      threevector old_posn,const threevector& new_posn);
//   void reset_vertex_posn(
//      const threevector& old_posn,const threevector& new_posn);

// Center-of-mass member functions

   threevector& compute_COM();
   threevector& get_COM();
   const threevector& get_COM() const;
   threevector compute_faces_centroid(const std::vector<face>& Faces);

// Moving around polyhedron member functions:

   virtual void translate(const threevector& rvec);
   virtual void absolute_position(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

// Special polyhedra generation member functions:

   void reset_vertices_edges_faces(
      const threevector& origin,const std::vector<vertex>& V,
      const std::vector<edge>& E,const std::vector<face>& F);
   void generate_tetrahedron(const std::vector<threevector>& V);
   void generate_square_pyramid(const std::vector<threevector>& V);
   void generate_box(
      double min_X,double max_X,double min_Y,double max_Y,
      double min_Z,double max_Z,bool rectangular_faces_flag=false);
   void generate_box(const std::vector<threevector>& V);
   void generate_box_with_rectangular_faces(
      const std::vector<threevector>& V);
   void generate_prism_with_rectangular_faces(
      const polyline& bottom_polyline,double height,
      bool constant_top_face_height_flag=true);
   void generate_prism_with_rectangular_faces(
      const contour& bottom_contour,double height,
      bool constant_top_face_height_flag=true);
   void generate_prism_with_rectangular_faces(
      const std::vector<threevector>& bottom_vertices,double height,
      bool constant_top_face_height_flag=true);
   void generate_polyhedron(
      const threevector& origin,const std::vector<vertex>& V,
      const std::vector<edge>& E,const std::vector<face>& F);

// Surface of revolution member functions:

   void generate_surface_of_revolution(
      const std::vector<twovector> rz_vertices,
      unsigned int n_revolution_steps=20);
   
// Search member functions:

   int find_vertex_ID_given_posn(const threevector& posn,
                                 const std::vector<vertex>& Vertices);
   int find_edge_ID_given_vertices_posns(
      const vertex& V1,const vertex& V2,const std::vector<edge>& Edges);
   edge* find_edge_given_ID(int ID,std::vector<edge>& Edges);
   edge* find_edge_given_vertices(const vertex& V1,const vertex& V2);
   void relabel_IDs_for_all_edges(const std::vector<face>& Faces,
                                  std::vector<edge>& Edges);
   bool ray_intercept(
      const threevector& ray_basepoint,const threevector& ray_hat,
      threevector& closest_polyhedron_point);

// Polyhedron import/export member functions:

   void write_OFF_file(std::string off_filename);
   bool read_OFF_file(std::string off_filename,fourvector& volume_color);
   std::vector<polygon*> reconstruct_rectangular_sides_from_OFF_file(
      std::string off_filename);

   void write_PLY_file(std::string PLY_filename);
   bool read_PLY_file(std::string PLY_filename);

   std::vector<std::pair<threevector,threevector> >&
      generate_surface_point_cloud(double ds_frac);

  protected:

   face::HandednessType faces_handedness;
   vertex* origin_vertex_ptr; 
	// Alias for some vertex ptr within *vertices_handler_ptr
   std::vector<edge> edges;
   std::vector<face> faces;

// Note added on 7/20/08: index_map_ptr should eventually be
// eliminated in favor of vertices_handler_ptr->get_vertex_map_ptr():

   typedef std::map<threevector,int,ltthreevector > INDEX_MAP;
   INDEX_MAP* index_map_ptr;

   void set_origin_vertex_ptr(const threevector& origin);
   vertex* get_origin_vertex_ptr();
   const vertex* get_origin_vertex_ptr() const;

// Index relabeling member functions:

   void copy_vertices_handler_contents_to_vector(
      std::vector<vertex>& vertices);
   void copy_vector_contents_to_vertices_handler(
      const std::vector<vertex>& vertices);

   void relabel_IDs_for_all_face_edges_and_vertices();
   void relabel_IDs_for_face_edges_and_vertices(
      face* curr_face_ptr,const std::vector<vertex>& Vertices,
      const std::vector<edge>& Edges);
   void relabel_IDs_for_face_edges(
      face& curr_face,const std::vector<vertex>& Vertices,
      std::vector<edge>& Edges);

  private: 

   unsigned int ID,n_external_edges;
   double volume,bounding_sphere_radius;
   threevector COM,bounding_sphere_center;
   threevector tip,base;
   bounding_box bbox;
   Network<face* >* face_network_ptr;
   vertices_handler* vertices_handler_ptr;
   std::vector<std::pair<threevector,threevector> > surfacepnts_normals;
   TRIANGLEFACE_RECTANGLE_MAP* triangleface_rectangle_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const polyhedron& p);

   void compute_bounding_sphere_and_box();

   void clear_vertices_edges_faces();

// Face and edge connectivity member functions:

   void generate_face_network();
   void identify_internal_edges();

   int common_edge_between_two_faces(int i,int j);
   void relabel_vertex_IDs_for_face_edges(
      face* curr_face_ptr,const std::vector<vertex>& Vertices,
      const std::vector<edge>& Edges);
   void relabel_IDs_for_chain_vertices(
      face* curr_face_ptr,const std::vector<vertex>& Vertices);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int polyhedron::get_ID() const
{
   return ID;
}

inline face::HandednessType polyhedron::get_faces_handedness() const
{
   return faces_handedness;
}

inline void polyhedron::set_origin_vertex_ptr(const threevector& origin)
{
   origin_vertex_ptr=get_vertices_handler_ptr()->get_vertex_ptr(origin);
}

inline vertex* polyhedron::get_origin_vertex_ptr()
{
   return origin_vertex_ptr;
}

inline const vertex* polyhedron::get_origin_vertex_ptr() const
{
   return origin_vertex_ptr;
}

inline void polyhedron::set_origin(const threevector& origin)
{
   set_origin_vertex_ptr(origin);
}

inline threevector& polyhedron::get_origin() 
{
   if (origin_vertex_ptr != NULL)
   {
      return origin_vertex_ptr->get_posn();
   }
   else
   {
      return COM;
   }
}

inline const threevector& polyhedron::get_origin() const
{
   if (origin_vertex_ptr != NULL)
   {
      return origin_vertex_ptr->get_posn();
   }
   else
   {
      return COM;
   }
}

inline unsigned int polyhedron::get_n_vertices() const
{
   return vertices_handler_ptr->get_n_vertices();
}

inline void polyhedron::set_vertices(const std::vector<threevector>& V)
{
   for (int i=0; i<int(V.size()); i++)
   {
      unsigned int curr_vertex_index=get_n_vertices();
      vertex curr_vertex(V[i],curr_vertex_index);
//      vertex curr_vertex(V[i],i);
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(curr_vertex);
   }
   compute_bounding_sphere_and_box();
}

inline void polyhedron::set_vertices(const std::vector<vertex>& V)
{
//   std::cout << "inside polyhedron::set_vertices()" << std::endl;
   for (int v=0; v<int(V.size()); v++)
   {
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(V[v]);
   }
   compute_bounding_sphere_and_box();
}

inline vertex& polyhedron::get_vertex(unsigned int n) 
{
   if (n < get_n_vertices())
   {
      return vertices_handler_ptr->get_vertex_from_chain(n);
   }
   else
   {
      std::cout << "Error in polyhedron::get_vertex()" << std::endl;
      std::cout << "Invalid vertex index n = " << n << std::endl;
      exit(-1);
   }
}

inline const vertex& polyhedron::get_vertex(unsigned int n) const
{
   if (n < get_n_vertices())
   {
      return vertices_handler_ptr->get_vertex_from_chain(n);
   }
   else
   {
      std::cout << "Error in polyhedron::get_vertex()" << std::endl;
      std::cout << "Invalid vertex index n = " << n << std::endl;
      exit(-1);
   }
}

inline vertices_handler* polyhedron::get_vertices_handler_ptr()
{
   return vertices_handler_ptr;
}

inline const vertices_handler* polyhedron::get_vertices_handler_ptr() const
{
   return vertices_handler_ptr;
}

// --------------------------------------------------------------------------
inline unsigned int polyhedron::get_n_edges() const
{
   return edges.size();
}

inline unsigned int polyhedron::get_n_external_edges() const
{
   return n_external_edges;
}

inline void polyhedron::set_edges(const std::vector<edge>& E)
{
//   std::cout << "inside polyhedron::set_edges() " << std::endl;
   for (int e=0; e<int(E.size()); e++)
   {
      edges.push_back(E[e]);
//      std::cout << "e = " << e << " edges.back() = "
//                << edges.back() << std::endl;
   }
}

inline unsigned int polyhedron::get_n_faces() const
{
   return faces.size();
}

inline void polyhedron::set_faces(const std::vector<face>& F)
{
   for (int f=0; f<int(F.size()); f++)
   {
      faces.push_back(F[f]);
   }
}

inline threevector& polyhedron::get_COM() 
{
   return COM;
}

inline const threevector& polyhedron::get_COM() const
{
   return COM;
}

inline std::vector<std::pair< threevector, threevector> >& 
polyhedron::get_surfacepnts_normals() 
{
   return surfacepnts_normals;
}

inline void polyhedron::get_bounding_sphere(threevector& center,double& radius)
{
   center=bounding_sphere_center;
   radius=bounding_sphere_radius;
}

inline const bounding_box& polyhedron::get_bbox() const
{
   return bbox;
}

inline threevector& polyhedron::get_tip()
{
   return tip;
}

inline const threevector& polyhedron::get_tip() const
{
   return tip;
}

inline threevector& polyhedron::get_base() 
{
   return base;
}

inline const threevector& polyhedron::get_base() const
{
   return base;
}

inline polyhedron::TRIANGLEFACE_RECTANGLE_MAP* 
polyhedron::get_triangleface_rectangle_map_ptr()
{
   return triangleface_rectangle_map_ptr;
}

inline const polyhedron::TRIANGLEFACE_RECTANGLE_MAP* 
polyhedron::get_triangleface_rectangle_map_ptr() const
{
   return triangleface_rectangle_map_ptr;
}


#endif  // polyhedron.h



