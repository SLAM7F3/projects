// ==========================================================================
// Polyhedron class member function definitions
// ==========================================================================
// Last modified on 6/13/12; 5/18/13; 8/4/13; 4/4/14
// ==========================================================================

#include <map>
#include <set>
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "datastructures/Hashtable.h"
#include "math/ltthreevector.h"
#include "geometry/polygon.h"
#include "geometry/polyhedron.h"
#include "geometry/polyline.h"
#include "templates/mytemplates.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "datastructures/Triple.h"
#include "datastructures/Quadruple.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void polyhedron::initialize_member_objects()
{
   vertices_handler_ptr=NULL;
   edges.clear();
   faces.clear();

   n_external_edges=0;
   volume=0;
   faces_handedness=face::unknown;

   origin_vertex_ptr=NULL;
   COM=Zero_vector;
   face_network_ptr=NULL;
   triangleface_rectangle_map_ptr=NULL;
}

void polyhedron::allocate_member_objects()
{
//    cout << "inside polyhedron::allocate_member_objects()" << endl;

   index_map_ptr=new INDEX_MAP;
   bool edge_flag=false;
   vertices_handler_ptr=new vertices_handler(edge_flag);
}		       

polyhedron::polyhedron(int id)
{
//   cout << "inside polyhedron() constructor #1" << endl;
   initialize_member_objects();
   allocate_member_objects();
   ID=id;
}

polyhedron::polyhedron(const vector<threevector>& V,int id)
{
//   cout << "inside polyhedron() constructor #2" << endl;
   initialize_member_objects();
   allocate_member_objects();
   set_vertices(V);
   ID=id;
}

polyhedron::polyhedron(
   const threevector& origin,const vector<vertex>& V,const vector<edge>& E,
   const vector<face>& F,int id)
{
   initialize_member_objects();
   allocate_member_objects();
   reset_vertices_edges_faces(origin,V,E,F);
   ID=id;
}

// Copy constructor:

polyhedron::polyhedron(const polyhedron& p)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(p);
}

polyhedron::~polyhedron()
{
   delete index_map_ptr;
   delete vertices_handler_ptr;
   delete triangleface_rectangle_map_ptr;
}

// ---------------------------------------------------------------------
void polyhedron::docopy(const polyhedron& p)
{
//   cout << "inside polyhedron::docopy()" << endl;
   ID=p.ID;
   n_external_edges=p.n_external_edges;
   volume=p.volume;
   bounding_sphere_radius=p.bounding_sphere_radius;
   faces_handedness=p.faces_handedness;

   if (p.face_network_ptr != NULL)
   {
      face_network_ptr=new Network<face*>(*p.face_network_ptr);
   }

   delete vertices_handler_ptr;
   vertices_handler_ptr=new vertices_handler(*p.vertices_handler_ptr);

   set_origin_vertex_ptr(p.get_origin());
   COM=p.COM;
   bounding_sphere_center=p.bounding_sphere_center;
   bbox=p.bbox;

   for (unsigned int i=0; i<p.get_n_edges(); i++)
   {
      const edge* edge_ptr=p.get_edge_ptr(i);
      set_edge(edge_ptr->get_V1().get_ID(),edge_ptr->get_V2().get_ID(),
               edge_ptr->get_internal_edge_flag());
   }
   
   for (unsigned int i=0; i<p.get_n_faces(); i++)
   {
      const face* face_ptr=p.get_face_ptr(i);
      set_face(face_ptr->get_edge_chain());
      faces.back().set_handedness(face_ptr->get_handedness());
   }

}

// Overload = operator:

polyhedron& polyhedron::operator= (const polyhedron& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const polyhedron& p)
{
   outstream << endl;

//   cout << "inside polyhedron::operator<<" << endl;
//   cout << "&p = " << &p << endl;
//   cout << "sizeof(p) = " << sizeof(p) << endl;

   double x0=p.get_origin().get(0);
   double y0=p.get_origin().get(1);
   double z0=p.get_origin().get(2);
   outstream << "origin = " << x0 << "," << y0 << "," << z0 << endl;
   outstream << "COM = " << p.COM.get(0) << "," 
             << p.COM.get(1) << "," << p.COM.get(2) << endl;
   outstream << "volume = " << p.volume << endl;
   outstream << "faces_handedness = " << p.faces_handedness << endl;

   outstream << "*************** Polyhedron VERTICES ***************" << endl;
   outstream << "Number of vertices = " << p.get_n_vertices() << endl;
   for (unsigned int vertex_index=0; vertex_index<p.get_n_vertices(); vertex_index++)
   {
      vertex curr_V(p.get_vertex(vertex_index));
//      outstream << "Vertex ID = " << curr_V.get_ID()
//                << " x-x0 = " << curr_V.get_posn().get(0)-x0
//                << " y-y0 = " << curr_V.get_posn().get(1)-y0
//                << " z-z0 = " << curr_V.get_posn().get(2)-z0 << endl;
      outstream << "Vertex ID = " << curr_V.get_ID()
                << " x = " << curr_V.get_posn().get(0)
                << " y = " << curr_V.get_posn().get(1)
                << " z = " << curr_V.get_posn().get(2) << endl;

   }
   outstream << endl;   


   outstream << "*************** Polyhedron EDGES ***************" << endl;
   outstream << "Number of edges = " << p.get_n_edges() << endl;
   outstream << "Number of external edges = " 
             << p.get_n_external_edges() << endl;
   for (unsigned int edge_index=0; edge_index<p.get_n_edges(); edge_index++)
   {
      const edge* curr_edge_ptr=p.get_edge_ptr(edge_index);
      outstream << "Edge ID = " << curr_edge_ptr->get_ID()
                << " V0 ID = " << curr_edge_ptr->get_V1().get_ID()
                << " V1 ID = " << curr_edge_ptr->get_V2().get_ID()
                << " internal edge flag = " 
                << curr_edge_ptr->get_internal_edge_flag()
                << endl;
   }
   outstream << endl;
   
   outstream << "*************** Polyhedron FACES ***************" << endl;
   outstream << "Number of faces = " << p.get_n_faces() << endl;
   for (unsigned int face_index=0; face_index<p.get_n_faces(); face_index++)
   {
      const face* curr_face_ptr=p.get_face_ptr(face_index);
//      outstream << "face_index = " << face_index
//                << " curr_face_ptr = " << curr_face_ptr << endl;
      outstream << *curr_face_ptr << endl;

//      outstream << "Edge IDs: ";
//      for (unsigned int edge_index=0; edge_index<curr_face_ptr->get_n_edges(); 
//           edge_index++)
//      {
//         outstream << curr_face_ptr->get_edge_ID(edge_index) << " ";
//      }
      outstream << endl;
   }

   return outstream;
}

// =====================================================================
// Edge member functions
// =====================================================================

// Member function set_edge() returns the ID assigned to the new edge.

int polyhedron::set_edge(
   unsigned int vertex_index1,unsigned int vertex_index2,
   bool internal_edge_flag,int edge_ID)
{
//   cout << "inside polyhedron::set_edge()" << endl;
   
   if (vertex_index1 >= 0 && vertex_index1 < get_n_vertices() &&
       vertex_index2 >= 0 && vertex_index2 < get_n_vertices() &&
       vertex_index1 != vertex_index2)
   {
      if (edge_ID==-1) edge_ID=edges.size();
      edge curr_edge(get_vertex(vertex_index1),
                     get_vertex(vertex_index2),edge_ID);
      curr_edge.set_internal_edge_flag(internal_edge_flag);
      edges.push_back(curr_edge);

//      cout << "curr_edge = " << curr_edge.get_ID()
//           << " V1 = " << curr_edge.get_V1()
//           << " V2 = " << curr_edge.get_V2() << endl;
   }
   return edge_ID;
}

edge* polyhedron::get_edge_ptr(unsigned int edge_index)
{
   if (edge_index >= 0 && edge_index < get_n_edges())
   {
      return &(edges[edge_index]);
   }
   else
   {
      return NULL;
   }
}

const edge* polyhedron::get_edge_ptr(unsigned int edge_index) const
{
   if (edge_index >= 0 && edge_index < get_n_edges())
   {
      return &(edges[edge_index]);
   }
   else
   {
      return NULL;
   }
}

vertex& polyhedron::get_edge_V1(int edge_ID)
{
//   cout << "inside polyhedron::get_edge_V1, edge_ID = " << edge_ID
//        << endl;
   edge* edge_ptr=get_edge_ptr(edge_ID);
   return get_vertex(edge_ptr->get_V1().get_ID());
}

vertex& polyhedron::get_edge_V2(int edge_ID)
{
   edge* edge_ptr=get_edge_ptr(edge_ID);
   return get_vertex(edge_ptr->get_V2().get_ID());
}

bool polyhedron::get_internal_edge_flag(int edge_ID)
{
   edge* curr_edge_ptr=get_edge_ptr(edge_ID);
   return curr_edge_ptr->get_internal_edge_flag();
}

// =====================================================================
// Face member functions
// =====================================================================

void polyhedron::set_face(int i,int j,int k,int face_ID)
{
//   cout << "inside polyhedron::set_face, i = " << i 
//        << " j = " << j << " k = " << k << endl;
   
   vector<edge> face_edges;
   face_edges.push_back(*(get_edge_ptr(i)));
   face_edges.push_back(*(get_edge_ptr(j)));
   face_edges.push_back(*(get_edge_ptr(k)));
   set_face(face_edges,face_ID);
}

void polyhedron::set_face(int i,int j,int k,int l,int face_ID)
{
//   cout << "inside polyhedron::set_face(i,j,k,l,face_ID)" << endl;
   vector<edge> face_edges;
   face_edges.push_back(*(get_edge_ptr(i)));
   face_edges.push_back(*(get_edge_ptr(j)));
   face_edges.push_back(*(get_edge_ptr(k)));
   face_edges.push_back(*(get_edge_ptr(l)));
   set_face(face_edges,face_ID);
}

void polyhedron::set_face(const vector<int>& edge_indices,int face_ID)
{
   vector<edge> face_edges;
   for (unsigned int i=0; i<edge_indices.size(); i++)
   {
      face_edges.push_back(*(get_edge_ptr(edge_indices[i])));
   }
   set_face(face_edges,face_ID);
}

void polyhedron::set_face(const vector<edge>& face_edges,int face_ID)
{
//   cout << "inside polyhedron::set_face(face_edges)" << endl;
   
   if (face_ID==-1) face_ID=faces.size();
//   cout << "face_ID = " << face_ID << endl;

   face curr_face(face_ID);
   curr_face.build_face(face_edges);
   faces.push_back(curr_face);
}

face* polyhedron::get_face_ptr(unsigned int face_index)
{
   if (face_index >= 0 && face_index < get_n_faces())
   {
      return &(faces[face_index]);
   }
   else
   {
      return NULL;
   }
}

const face* polyhedron::get_face_ptr(unsigned int face_index) const
{
   if (face_index >= 0 && face_index < get_n_faces())
   {
      return &(faces[face_index]);
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
void polyhedron::compute_faces_handedness()
{
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face* curr_face_ptr=get_face_ptr(f);
      threevector p_hat( (curr_face_ptr->get_COM()-get_COM()).unitvector() );
//      face::HandednessType handedness=
         curr_face_ptr->handedness_wrt_direction(p_hat);
//      cout << "f = " << f << " *curr_face_ptr = " << *curr_face_ptr
//           << endl;

   } // loop over index f labeling polyhedron's faces
   
}

// ---------------------------------------------------------------------
// Member function ensure_faces_handedness 

void polyhedron::ensure_faces_handedness(
   face::HandednessType desired_handedness)
{
//   cout << "inside polyhedron::ensure_faces_handedness()" << endl;
   compute_COM();
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face* curr_face_ptr=get_face_ptr(f);
      ensure_face_handedness(curr_face_ptr,desired_handedness);
   } // loop over index f labeling polyhedron's faces
   faces_handedness=desired_handedness;
}

void polyhedron::ensure_face_handedness(
   face* curr_face_ptr,face::HandednessType desired_handedness)
{
//   cout << "inside polyhedron::ensure_face_handedness()" << endl;
//   cout << "get_COM() = " << get_COM() << endl;
   threevector p_hat( (curr_face_ptr->get_COM()-get_COM()).unitvector() );
   curr_face_ptr->force_handedness_wrt_direction(desired_handedness,p_hat);

//   face::HandednessType curr_handedness=
      curr_face_ptr->handedness_wrt_direction(p_hat);
//   cout << "curr_handedness = " << curr_handedness << endl;
//   outputfunc::enter_continue_char();
//   cout << " *curr_face_ptr = " << *curr_face_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function common_edge_between_two_faces takes in integers i
// and j labeling faces within *face_network_ptr.  It compares all
// edge indices which define *curr_face_ptr with those that define
// *next_face_ptr.  If any two edge indices match, the two faces are
// adjacent neighbors.  The matching edge ID is returned by this method.

int polyhedron::common_edge_between_two_faces(int i,int j)
{
//   cout << "inside polyhedron::common_edge_between_two_faces()" << endl;
//   cout << "face index i = " << i << " j = " << j << endl;
   
   face* curr_face_ptr=face_network_ptr->get_site_data_ptr(i);
   face* next_face_ptr=face_network_ptr->get_site_data_ptr(j);

   vector<int> common_edge_indices;
   for (unsigned int a=0; a<curr_face_ptr->get_n_edges(); a++)
   {
      for (unsigned int b=0; b<next_face_ptr->get_n_edges(); b++)
      {
         if (curr_face_ptr->get_edge_ID(a)==next_face_ptr->get_edge_ID(b))
         {
//            cout << "matching face edge ID = "
//                 << curr_face_ptr->get_edge_ID(a) << endl;
            
            return curr_face_ptr->get_edge_ID(a);
         }
      } // loop over index b labeling edges of *next_face_ptr
   } // loop over index a labeing edges of *curr_face_ptr

   return -1;
}

// ---------------------------------------------------------------------
// Member function instantiate_face_network instantiates the face
// network and identifies internal triangular face edges which do NOT
// represent polyhedron borders.

void polyhedron::instantiate_face_network()
{
//   cout << "inside polyhedron::instantiate_face_network()" << endl;
   
   relabel_IDs_for_all_face_edges_and_vertices();

   generate_face_network();
   identify_internal_edges();
}

// ---------------------------------------------------------------------
// Member function extract_zplane_faces()

vector<face*> polyhedron::extract_zplane_faces(double z)
{
   vector<face*> zplane_face_ptrs;
   
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face* face_ptr=get_face_ptr(f);
      
      bool face_lies_in_zplane_flag=true;
      for (unsigned int v=0; v<face_ptr->get_n_vertices(); v++)
      {
         threevector V(face_ptr->get_vertex_from_chain(v).get_posn());
         if (!nearly_equal(V.get(2),z))
         {
            face_lies_in_zplane_flag=false;
            break;
         }
      } // loop over index v labeling face vertices

      if (face_lies_in_zplane_flag) zplane_face_ptrs.push_back(face_ptr);

   } // loop over index f labeling faces
   return zplane_face_ptrs;
}

// ---------------------------------------------------------------------
// Member function compute_zplane_footprint()

polygon* polyhedron::compute_zplane_footprint(double z)
{
   polygon* footprint_polygon_ptr=NULL;
   vector<face*> zplane_face_ptrs=extract_zplane_faces(z);

   const double scalefactor=1.001;
   if (zplane_face_ptrs.size() > 0)
   {
      vector<polygon> z_polygons;
      for (unsigned int f=0; f<zplane_face_ptrs.size(); f++)
      {
         polygon curr_poly=zplane_face_ptrs[f]->compute_face_poly();
         threevector COM=curr_poly.compute_COM();
         curr_poly.scale(COM,scalefactor);
         z_polygons.push_back(curr_poly);
      } // loop over index f labeling z-plane faces
      
      vector<polygon> footprint_polygons=
         geometry_func::polygon_union(z_polygons);
      footprint_polygon_ptr=new polygon(footprint_polygons.front());
   }
   return footprint_polygon_ptr;
}

// ==========================================================================
// Vertex map member functions
// ==========================================================================

void polyhedron::update_vertex_map()
{
   vertices_handler_ptr->clear_vertex_map();
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertices_handler_ptr->update_vertex_map(get_vertex(v));
   }
   
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      get_edge_ptr(e)->update_vertex_map();
   }

   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      get_face_ptr(f)->update_vertex_map();
   }
}

// On 5/18/13, we discovered the hard and painful way that passing
// input parameter old_posn as a const reference into this method
// incorrectly ended up changing its value!  So we now pass old_posn
// by value rather than by reference into this method.

void polyhedron::reset_vertex_posn(
   threevector old_posn,const threevector& new_posn)
{
//   cout << "inside polyhedron::reset_vertex_posn()" << endl;

   vertices_handler_ptr->reset_vertex_posn(old_posn,new_posn);

   update_vertex_map();

   for (unsigned int e=0; e<get_n_edges(); e++)
   {
//      cout << "Edge e = " << e << endl;
      edge* curr_edge_ptr=get_edge_ptr(e);
      curr_edge_ptr->reset_vertex_posn(old_posn,new_posn);
   }

   for (unsigned int f=0; f<get_n_faces(); f++)
   {
//      cout << "Face f = " << f << endl;
      face* curr_face_ptr=get_face_ptr(f);
      curr_face_ptr->reset_vertex_posn(old_posn,new_posn);
   }
}

// ==========================================================================
// Center-of-mass member functions
// ==========================================================================

// Member function compute_COM

threevector& polyhedron::compute_COM()
{
//   cout << "inside polyhedron::compute_COM()" << endl;
   COM=compute_faces_centroid(faces);
   return COM;
}

// ---------------------------------------------------------------------
// Member function compute_faces_centroid computes the average of the
// input faces COMs weighted by their areas.  For reasons we don't
// understand as of early July 2007, this method does not return
// precisely the correct overall COM location as compute_COM()...

threevector polyhedron::compute_faces_centroid(const vector<face>& Faces) 
{
//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//   cout << "inside polyhedron::compute_faces_centroid()" << endl;

//   cout << "Faces = " << endl;
//   templatefunc::printVector(Faces);

   threevector numer;
   double denom=0;
   for (unsigned int f=0; f<Faces.size(); f++)
   {
      double curr_area=Faces[f].get_area();
      numer += curr_area*Faces[f].get_COM();
      denom += curr_area;
   }

   threevector avg=numer/denom;
//   cout << "face avg = " << avg << endl;
   return avg;
}

void polyhedron::compute_bounding_sphere_and_box()
{
//   cout << "inside polyhedron::compute_bounding_sphere_and_box()" << endl;
   geometry_func::compute_bounding_box(vertices_handler_ptr,bbox);
   vector<threevector> bbox_corners=bbox.get_bbox_corners();
   geometry_func::compute_bounding_sphere(
      bbox.get_bbox_corners(),bounding_sphere_center,bounding_sphere_radius);
//   geometry_func::compute_bounding_sphere(
//      vertices_handler_ptr,bounding_sphere_center,bounding_sphere_radius);
//   cout << "X = " << bounding_sphere_center.get(0)
//        << " Y = " << bounding_sphere_center.get(1)
//        << " Z = " << bounding_sphere_center.get(2)
//        << " R = " << bounding_sphere_radius << endl;

}

// ==========================================================================
// Moving around polyhedra member functions
// ==========================================================================

void polyhedron::translate(const threevector& rvec)
{
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      get_vertex(v).translate(rvec);
   }
   
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edges[e].translate(rvec);
   }
   
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      faces[f].translate(rvec);
   }

   geometry_func::translate(COM,rvec);
   update_vertex_map();
}

void polyhedron::absolute_position(const threevector& rvec)
{
   threevector delta=rvec-get_origin();
   translate(delta);
}

// ---------------------------------------------------------------------
void polyhedron::scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      get_vertex(v).scale(scale_origin,scalefactor);
   }
   
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edges[e].scale(scale_origin,scalefactor);
   }
   
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      faces[f].scale(scale_origin,scalefactor);
   }

   geometry_func::scale(COM,scale_origin,scalefactor);
   update_vertex_map();
}

// ---------------------------------------------------------------------
void polyhedron::rotate(const rotation& R)
{
   rotate(Zero_vector,R);
}

// ---------------------------------------------------------------------
void polyhedron::rotate(
   const threevector& rotation_origin,const rotation& R)
{
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      get_vertex(v).rotate(rotation_origin,R);
   }
   
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edges[e].rotate(rotation_origin,R);
   }
   
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      faces[f].rotate(rotation_origin,R);
   }

   geometry_func::rotate(COM,rotation_origin,R);
   update_vertex_map();
}

// ---------------------------------------------------------------------
void polyhedron::rotate(const threevector& rotation_origin,
                        double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ==========================================================================
// Special polyhedra member functions
// ==========================================================================

void polyhedron::clear_vertices_edges_faces()
{
   vertices_handler_ptr->clear_all_containers();
   edges.clear();
   faces.clear();
}

// ---------------------------------------------------------------------
void polyhedron::reset_vertices_edges_faces(
   const threevector& origin,const vector<vertex>& V,const vector<edge>& E,
   const vector<face>& F)
{
//   cout << "inside polyhedron::reset_vertices_edges_faces()" << endl;

   clear_vertices_edges_faces();
   set_vertices(V);
   set_edges(E);   
   set_faces(F);   
   set_origin_vertex_ptr(origin);
   instantiate_face_network();
}

// ---------------------------------------------------------------------
void polyhedron::generate_tetrahedron(const vector<threevector>& V)
{
   if (V.size() != 4)
   {
      cout << "Error in polyhedron::generate_tetrahedron()" << endl;
      cout << "# input vertices should equal 4 but actually equals "
           << V.size() << endl;
      return;
   }

   clear_vertices_edges_faces();
   set_vertices(V);

// We assume that the first vertex corresponds to the tetrahedron's apex
// which we take to be its origin:

   set_origin_vertex_ptr(V[0]);

   set_edge(0,1);
   set_edge(0,2);
   set_edge(0,3);
   set_edge(1,2);
   set_edge(2,3);
   set_edge(3,1);

// Anye Li pointed out on 3/9/07 that GTS uses a LEFT-handed ordering
// of vertices for triangle faces.  If a surface's number of triangle
// faces is even, then we can adopt a more mathematically conventional
// RIGHT-handed ordering of vertices, for (-1)**even = +1.  But if the
// surface's number of triangles is odd, we must conform to GTS'
// left-handed ordering convention.  According to the GTS
// documentation, triangle normals for all closed surfaces always
// point outward from the polyhedron's center.

   set_face(0,3,1);
   set_face(2,1,4);
   set_face(5,0,2);
   set_face(4,3,5);

   instantiate_face_network();

   update_vertex_map();
}

// ---------------------------------------------------------------------
void polyhedron::generate_square_pyramid(const vector<threevector>& V)
{
//   cout << "inside polyhedron::generate_square_pyramid(vector<threevector> V)" 
//        << endl;
   
   if (V.size() != 5)
   {
      cout << "Error in polyhedron::generate_tetrahedron()" << endl;
      cout << "# input vertices should equal 5 but actually equals "
           << V.size() << endl;
      return;
   }

   clear_vertices_edges_faces();
   set_vertices(V);

// We assume that the zeroth vertex corresponds to the pyramid's apex
// which we take to be its origin:

   set_origin_vertex_ptr(V[0]);

   set_edge(0,1);
   set_edge(0,2);
   set_edge(0,3);
   set_edge(0,4);
   set_edge(1,2);
   set_edge(2,3);
   set_edge(3,4);
   set_edge(4,1);
   set_edge(1,3);

// Anye Li pointed out on 3/9/07 that GTS uses a LEFT-handed ordering
// of vertices for triangle faces.  If a surface's number of triangle
// faces is even, then we can adopt a more mathematically conventional
// RIGHT-handed ordering of vertices, for (-1)**even = +1.  But if the
// surface's number of triangles is odd, we must conform to GTS'
// left-handed ordering convention.  According to the GTS
// documentation, triangle normals for all closed surfaces always
// point outward from the polyhedron's center.

   set_face(4,1,0);
   set_face(1,5,2);
   set_face(6,3,2);
   set_face(7,0,3);
   set_face(4,8,5);
   set_face(8,7,6);

   instantiate_face_network();
   update_vertex_map();
}

// ---------------------------------------------------------------------
void polyhedron::generate_box(
   double min_X,double max_X,double min_Y,double max_Y,
   double min_Z,double max_Z,bool rectangular_faces_flag)
{
//   cout << "inside polyhedron::generate_box()" << endl;
   vector<threevector> V;
   V.push_back(threevector(min_X,min_Y,max_Z));
   V.push_back(threevector(max_X,min_Y,max_Z));
   V.push_back(threevector(max_X,max_Y,max_Z));
   V.push_back(threevector(min_X,max_Y,max_Z));
   V.push_back(threevector(min_X,min_Y,min_Z));
   V.push_back(threevector(max_X,min_Y,min_Z));
   V.push_back(threevector(max_X,max_Y,min_Z));
   V.push_back(threevector(min_X,max_Y,min_Z));

   if (rectangular_faces_flag)
   {
      generate_box_with_rectangular_faces(V); // Needed for bbox point cloud
   }
   else
   {
      generate_box(V); // Needed for volume coloring via triangle mesh
   }
   set_origin_vertex_ptr(threevector(min_X,min_Y,min_Z));
}

// ---------------------------------------------------------------------
// This first generate_box() member function builds up a 3D box from a
// set of triangles.  As of Sep 2009, we believe we must call this
// version of generate_box in order to use OSG's Shape::TriangleMesh
// for translucently coloring a box's volume.

void polyhedron::generate_box(const vector<threevector>& V)
{
   if (V.size() != 8)
   {
      cout << "Error in polyhedron::generate_box()" << endl;
      cout << "# input vertices should equal 8 but actually equals "
           << V.size() << endl;
      return;
   }

   clear_vertices_edges_faces();
   set_vertices(V);

// Top face edges:

   set_edge(0,1);	// Edge 0
   set_edge(1,2);	// Edge 1
   set_edge(2,3);	// Edge 2
   set_edge(3,0);	// Edge 3
   set_edge(0,2);	// Edge 4
   
// Bottom face edges:

   set_edge(4,5);	// Edge 5
   set_edge(5,6);	// Edge 6
   set_edge(6,7);	// Edge 7
   set_edge(7,4);	// Edge 8
   set_edge(4,6);	// Edge 9
   
// Front face edges

   set_edge(0,4);	// Edge 10
   set_edge(5,1);	// Edge 11
   set_edge(4,1);	// Edge 12

// Back face edges

   set_edge(6,2);	// Edge 13
   set_edge(3,7);	// Edge 14
   set_edge(7,2);	// Edge 15

// Right face edge

   set_edge(5,2);	// Edge 16

// Left face edge

   set_edge(7,0);	// Edge 17

// Top triangular faces

   set_face(4,2,3);	// Face 0
   set_face(4,0,1);	// Face 1

// Bottom triangular faces

   set_face(9,6,5);	// Face 2
   set_face(9,8,7);	// Face 3

// Front triangular faces

   set_face(0,10,12);	// Face 4
   set_face(12,5,11);	// Face 5

// Back triangular faces

   set_face(15,14,2);	// Face 6
   set_face(7,15,13);	// Face 7

// Right triangular faces

   set_face(1,11,16);	// Face 8
   set_face(6,13,16);	// Face 9

// Left triangular faces

   set_face(3,14,17);	// Face 10
   set_face(17,8,10);	// Face 11

   instantiate_face_network();
   update_vertex_map();
}

// ---------------------------------------------------------------------
// Member function generate_box_with_rectangular_faces() builds up a
// 3D box from a set of rectangles .  As of Sep 2009, we believe we
// cannot generally call this method if we want to use OSG's
// Shape::TriangleMesh to translucently color a box's volume.

void polyhedron::generate_box_with_rectangular_faces(
   const vector<threevector>& V)
{
//   cout << "inside polyhedron::generate_box_with_rectangular_faces()"
//        << endl;

   if (V.size() != 8)
   {
      cout << "Error in polyhedron::generate_box()" << endl;
      cout << "# input vertices should equal 8 but actually equals "
           << V.size() << endl;
      return;
   }

   clear_vertices_edges_faces();
   set_vertices(V);

// Top face edges:

   set_edge(0,1);	// Edge 0
   set_edge(1,2);	// Edge 1
   set_edge(2,3);	// Edge 2
   set_edge(3,0);	// Edge 3
   
// Bottom face edges:

   set_edge(4,5);	// Edge 4
   set_edge(5,6);	// Edge 5
   set_edge(6,7);	// Edge 6
   set_edge(7,4);	// Edge 7
   
// Front face edges

   set_edge(0,4);	// Edge 8
   set_edge(5,1);	// Edge 9

// Back face edges

   set_edge(6,2);	// Edge 10
   set_edge(3,7);	// Edge 11

// Right face edge

// Left face edge

// Top rectangular face:

   set_face(0,1,2,3,0);	// Face 0

// Bottom face:

   set_face(7,6,5,4,1);	// Face 1

// Front rectangular faces

   set_face(0,8,4,9,2);	// Face 2

// Back rectangular faces

   set_face(6,11,2,10,3);	// Face 3

// Right rectangular faces

   set_face(10,1,9,5,4);	// Face 4

// Left rectangular faces

   set_face(3,11,7,8,5);	// Face 5

   instantiate_face_network();
   update_vertex_map();
}

// ---------------------------------------------------------------------
// Member function generate_prism_with_rectangular_faces takes in the
// vertices for a planar polygon along with a height. This method
// extrudes the bottom polygon upwards and forms a cylindrical prism.

void polyhedron::generate_prism_with_rectangular_faces(
   const polyline& bottom_polyline,double height,
   bool constant_top_face_height_flag)
{
   vector<threevector> bottom_vertices;
   for (unsigned int i=0; i<bottom_polyline.get_n_vertices(); i++)
   {
      bottom_vertices.push_back(bottom_polyline.get_vertex(i));
   }
   generate_prism_with_rectangular_faces(
      bottom_vertices,height,constant_top_face_height_flag);
}

void polyhedron::generate_prism_with_rectangular_faces(
   const contour& bottom_contour,double height,
   bool constant_top_face_height_flag)
{
   vector<threevector> bottom_vertices;
   for (unsigned int i=0; i<bottom_contour.get_nvertices(); i++)
   {
      bottom_vertices.push_back(bottom_contour.get_vertex(i).first);
   }
   generate_prism_with_rectangular_faces(
      bottom_vertices,height,constant_top_face_height_flag);
}

void polyhedron::generate_prism_with_rectangular_faces(
   const vector<threevector>& bottom_vertices,double height,
   bool constant_top_face_height_flag)
{
//   cout << "inside polyhedron::generate_prism_with_rectangular_faces()"
//        << endl;

   clear_vertices_edges_faces();

   unsigned int n_sides=bottom_vertices.size();

// First find maximum bottom face vertex altitude:

   double max_bottom_vertex_altitude=NEGATIVEINFINITY;
   for (unsigned int n=0; n<n_sides; n++)
   {
      threevector curr_bottom_face_vertex=bottom_vertices[n];
      max_bottom_vertex_altitude=
         basic_math::max(
            max_bottom_vertex_altitude,curr_bottom_face_vertex.get(2));
   }

// Top face vertices:

   vector<threevector> V;
   for (unsigned int n=0; n<n_sides; n++)
   {
      if (constant_top_face_height_flag)
      {
         threevector curr_top_face_vertex=bottom_vertices[n];
         curr_top_face_vertex.put(2,max_bottom_vertex_altitude+height);
         V.push_back(curr_top_face_vertex);
      }
      else
      {
         V.push_back(bottom_vertices[n]+threevector(0,0,height));
      }
   }

// Bottom face vertices:

   for (unsigned int n=0; n<n_sides; n++)
   {
      V.push_back(bottom_vertices[n]);
   }
   set_vertices(V);

// Top face edges:

   for (unsigned int n=0; n<n_sides; n++)
   {
      set_edge(n,modulo(n+1,n_sides));
   }
   
// Bottom face edges:

   for (unsigned int n=0; n<n_sides; n++)
   {
      set_edge(n+n_sides,modulo(n+1,n_sides)+n_sides);
   }

// Side face edges:

   for (unsigned int n=0; n<n_sides; n++)
   {
//      cout << "n = " << n << " n_sides = " << n_sides << endl;
      
      set_edge(n,n_sides+n);	// Vertical edge
      unsigned int bottom_vertex_ID=n_sides+n;
      unsigned int top_vertex_ID=n+1;
      if (top_vertex_ID==n_sides) top_vertex_ID=0;
//      cout << "top_vertex_ID = " << top_vertex_ID << endl;
      set_edge(bottom_vertex_ID,top_vertex_ID);  // Diagonal edge
   }	

// Rectangular side faces:

   for (unsigned int f=0; f<n_sides; f++)
   {
//      cout << "f = " << f << endl;
      set_face(f,2*f+2*n_sides, 2*f+1+2*n_sides);
      unsigned int side_edge_ID=2*f+2+2*n_sides;
      if (side_edge_ID==4*n_sides) side_edge_ID=2*n_sides;
      set_face(f+n_sides,side_edge_ID, 2*f+1+2*n_sides);

//      int e0=f;
//      int e1=2*f+2*n_sides;
//      int e2=2*f+1+2*n_sides;
      
//      cout << "First face edges: e0 = " << e0
//           << " e1 = " << e1 << " e2 = " << e2 << endl;

//      cout << "edge 0: V1 = " << get_edge_V1(e0)
//           << " V2 = " << get_edge_V2(e0) << endl;
//      cout << "edge 1: V1 = " << get_edge_V1(e1)
//           << " V2 = " << get_edge_V2(e1) << endl;
//      cout << "edge 2: V1 = " << get_edge_V1(e2)
//           << " V2 = " << get_edge_V2(e2) << endl;
      
//      e0=f+n_sides;
//      e1=2*f+2+2*n_sides;
//      e2=2*f+1+2*n_sides;
      
//      cout << "Second face edges: e0 = " << e0
//           << " e1 = " << e1 << " e2 = " << e2 << endl << endl;
   }

// Generate inner triangulation of top face using John Ratcliffe's
// triangulater class:

   vector<threevector> vertex_posns;
   for (unsigned int e=0; e<n_sides; e++)
   {
      vertex V1=get_edge_V1(e);
      vertex_posns.push_back(V1.get_posn());
//      cout << "e = " << e 
//           << " vertex V1 = " << V1 << endl;
   }
   contour top_contour(vertex_posns);
//   cout << "top_contour = " << top_contour << endl;

   triangles_group* triangles_group_ptr=top_contour.
      generate_interior_triangles();

// Loop over triangles within inner triangulation of top face contour.
// Add internal edges to polyhedron's edge list:
   
   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
      triangle* triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);
      
      vector<int> top_face_triangle_edge_IDs;
      for (unsigned int v=0; v<3; v++)
      {
         vertex curr_vertex=triangle_ptr->get_vertex(v);

//         cout << "t = " << t << " v = " << v
//              << " vertex_ID = " << curr_vertex.get_ID() << endl;
         edge curr_edge=triangle_ptr->get_edge(v);

// Check if curr triangle edge nearly overlaps with any previously
// instantiated top face edge.  If not, add the current triangle edge
// to the polyhedron's edge list:

         int top_edge_ID=-1;
         for (unsigned int e=0; e<n_sides; e++)
         {
            edge* existing_edge_ptr=get_edge_ptr(e);
            if (existing_edge_ptr->nearly_equal_posn(curr_edge))
            {
               top_edge_ID=existing_edge_ptr->get_ID();
               break;
            }
         } // loop over index e labeling existing top face edges
         if (top_edge_ID==-1)
         {
            vertex next_vertex=triangle_ptr->get_vertex(modulo(v+1,3));
            bool internal_edge_flag=true;
            top_edge_ID=set_edge(curr_vertex.get_ID(),next_vertex.get_ID(),
            internal_edge_flag);         
         }
         top_face_triangle_edge_IDs.push_back(top_edge_ID);
      } // loop over index v labeling vertices within current top face triangle

      set_face(top_face_triangle_edge_IDs);
   } // loop over index t labeling top face triangles
 
//   cout << "Top polyhedron face = " << endl;
//   cout << get_face_ptr(get_n_faces()-1) << endl;

/*
// Bottom face:

   vector<int> edge_indices;
   for (unsigned int e=0; e<n_sides; e++)
   {
      edge_indices.push_back(e+n_sides);
   }
   set_face(edge_indices);
*/

   instantiate_face_network();
   update_vertex_map();
}

// ---------------------------------------------------------------------
// Member function generate_polyhedron takes in vertex, edge and face
// information within input STL vectors.  It 

void polyhedron::generate_polyhedron(
   const threevector& origin,const vector<vertex>& V,const vector<edge>& E,
   const vector<face>& F)
{
//   cout << "inside polyhedron::generate_polyhedron()" << endl;

   clear_vertices_edges_faces();
   set_vertices(V);
   set_edges(E);   
   set_faces(F);   

   set_origin_vertex_ptr(origin);
   instantiate_face_network();
   update_vertex_map();
}

// ==========================================================================
// Face and edge connectivity member functions
// ==========================================================================

// Member function generate_face_network instantiates a
// Network<face*> and inserts each triangular face of the
// polyhedron into this network.  It then loops over each triangular
// face in the network and determines its adjacent triangle neighbors.  

void polyhedron::generate_face_network()
{
//   cout << "inside polyhedron::generate_face_network()" << endl;
//   cout << "get_n_faces() = " << get_n_faces() << endl;

   delete face_network_ptr;
   face_network_ptr=new Network<face* >(5*get_n_faces());
   
   for (unsigned int n=0; n<get_n_faces(); n++)
   {
      face* curr_face_ptr=get_face_ptr(n);
      face_network_ptr->insert_site(n,Site<face*>(curr_face_ptr));
//      cout << "n = " << n 
//           << " *curr_face_ptr = " << *curr_face_ptr << endl;
   } // loop over index labeling faces

   for (Mynode<int>* currnode_ptr=face_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      for (Mynode<int>* nextnode_ptr=currnode_ptr->get_nextptr();
           nextnode_ptr != NULL; nextnode_ptr=nextnode_ptr->get_nextptr())
      {
         int m=nextnode_ptr->get_data();

         if (common_edge_between_two_faces(n,m) >= 0)
         {
            face_network_ptr->add_symmetric_link(n,m);
//            cout << "Triangles " << n << " & " << m 
//                 << " are adjacent neighbors" << endl;
         }
      } // loop over nextnodes within *face_nextwork_ptr
   } // loop over currnodes within *face_network_ptr
}

// ---------------------------------------------------------------------
// Member function identify_internal_edges loops over each face and
// identifies its adjacent neighbors.  It then computes the absolute
// dotproduct between the faces' normals.  If the abs dotproduct == 1,
// the neighboring faces lie within the same plane and share at least
// one internal edge.  The edges common to parallel faces are marked
// as internal by this method.

void polyhedron::identify_internal_edges() 
{
//   cout << "inside polyhedron::identify_internal_edges()" << endl;

   for (Mynode<int>* currnode_ptr=face_network_ptr->
           get_entries_list_ptr()->get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      int n=currnode_ptr->get_data();
      face* curr_face_ptr=face_network_ptr->get_site_data_ptr(n);
      threevector curr_normal( curr_face_ptr->get_normal() );
      vector<int> nearest_face_neighbors=
         face_network_ptr->find_nearest_neighbors(n);

//      cout << "Face n = " << n << endl;
//      cout << "nearest_face_neighbors.size() = "
//           << nearest_face_neighbors.size() << endl;
//      cout << "neighboring face indices = " << endl;
//      templatefunc::printVector(nearest_face_neighbors);

      for (unsigned int j=0; j<nearest_face_neighbors.size(); j++)
      {
         threevector neighbor_normal (
            get_face_ptr(nearest_face_neighbors[j])->get_normal() );
         double abs_dotproduct=fabs(curr_normal.dot(neighbor_normal));

//         cout << "curr_normal.mag = " << curr_normal.magnitude() 
//              << " neighbor_normal.mag = " << neighbor_normal.magnitude()
//              << endl;
//         cout << "abs_dotproduct = " << abs_dotproduct << endl;
//         cout << endl;
         if (nearly_equal(abs_dotproduct,1,0.001))
         {
//            cout << "nearest face neighbor " << nearest_face_neighbors[j]
//                 << " is parallel to curr face" << endl;
            int common_edge_ID=common_edge_between_two_faces(
               n,nearest_face_neighbors[j]);
//            cout << "common_edge_ID = " << common_edge_ID << endl;
            if (common_edge_ID >= 0)
            {
               get_edge_ptr(common_edge_ID)->set_internal_edge_flag(true);
            }
         }
      } // loop over index j labeling nearest face neighbors

   } // loop over nodes within *face_network_ptr

// Compute number of internal and external edges:

   int n_internal_edges=0;
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      if (get_edge_ptr(e)->get_internal_edge_flag()) n_internal_edges++;
//      cout << "e = " << e << " internal edge flag = "
//           << get_edge_ptr(e)->get_internal_edge_flag() << endl;
   }
   n_external_edges=get_n_edges()-n_internal_edges;

//   cout << "n_internal_edges = " << n_internal_edges << endl;
//   cout << "n_external_edges = " << n_external_edges << endl;
//   outputfunc::enter_continue_char();
}

// ==========================================================================
// Surface of revolution member functions
// ==========================================================================

// Member function generate_surface_of_revolution() takes in a set of
// (R,Z) pairs which defines some 2D curve in 3D world-space.  It
// spins this curve about the +z_hat axis to form a surface of
// revolution.  A quasi-circular polygon is formed from each (R,Z)
// input pair.  Edges are then added between corresponding vertices
// from adjacent Z-plane polygons.  The revolution surface is
// triangulated so that it can be easily displayed as an OSG
// Polyhedron object.

void polyhedron::generate_surface_of_revolution(
   const vector<twovector> rz_vertices,unsigned int n_revolution_steps)
{
//   cout << "inside polyhedron::generate_surface_of_revolution()" << endl;
//   cout << "Enter n_revolution_steps:" << endl;
//   cin >> n_revolution_steps;
   
   double phi_start=0;
   double phi_stop=2*PI;
   double d_phi=(phi_stop-phi_start)/n_revolution_steps;

   clear_vertices_edges_faces();

   const double epsilon=1E-3;
   vector<polygon> revolution_polys;
   for (unsigned int i=0; i<rz_vertices.size(); i++)
   {
      double curr_r=rz_vertices[i].get(0);
      if (nearly_equal(curr_r,0)) curr_r=epsilon;
      double curr_z=rz_vertices[i].get(1);

      vector<threevector> XYZ_vertices;
      for (unsigned int n=0; n<n_revolution_steps; n++)
      {
         double phi=phi_start+n*d_phi;
         double curr_x=curr_r*cos(phi);
         double curr_y=curr_r*sin(phi);
         XYZ_vertices.push_back(threevector(curr_x,curr_y,curr_z));
      } // loop over index n labeling revolution step

      revolution_polys.push_back(polygon(XYZ_vertices));

// Load current revolution polygon's vertices in polyhedron's
// vertices_handler:

      set_vertices(XYZ_vertices);
//      cout << "i = " << i 
//           << " n_vertices = " << get_n_vertices() << endl;
//      cout << "revolution polygon = " 
//           << revolution_polys.back() << endl;

   } // loop over index i labeling rz vertex

   tip=revolution_polys.front().compute_COM();
   base=revolution_polys.back().compute_COM();

   unsigned int n_revolution_polys=revolution_polys.size();
   for (unsigned int i=0; i<n_revolution_polys; i++)
   {
//      cout << "--------------------------------------------------" << endl;
//      cout << "Revolution poly i = " << i << endl;

      int starting_vertex_ID=i*n_revolution_steps;

// Assign edges between vertices within a single revolution polygon:
      
      for (unsigned int n=starting_vertex_ID; n<starting_vertex_ID+
              n_revolution_steps; n++)
      {
         int vlo_ID=n;
         int vhi_ID=n+1;
         if (vhi_ID%n_revolution_steps==0) vhi_ID -= n_revolution_steps;
//         unsigned int edge_ID=
            set_edge(vlo_ID,vhi_ID);
//         edge* edge_ptr=get_edge_ptr(edge_ID);
//         cout << "Edge = " << *edge_ptr << endl;
      }

      if (i==n_revolution_polys-1) continue;

// Assign edges between vertices belonging to two revolution polygons:

      for (unsigned int n=starting_vertex_ID; n<starting_vertex_ID+
              n_revolution_steps; n++)
      {
//         unsigned int edge_ID=
            set_edge(n,n+n_revolution_steps);
//         edge* edge_ptr=get_edge_ptr(edge_ID);
//         cout << "Edge = " << *edge_ptr << endl;
      }
   } // loop over index i labeling revolution polygons


   for (unsigned int i=0; i<n_revolution_polys-1; i++)
   {
//      cout << "--------------------------------------------------" << endl;
//      cout << "Revolution poly i = " << i << endl;

      int starting_edge_ID=2*i*n_revolution_steps;
      for (unsigned int e=starting_edge_ID; e<starting_edge_ID+n_revolution_steps; 
           e++)
      {
         int elo_ID=e;
         int eright_ID=e+n_revolution_steps+1;
         if (eright_ID%n_revolution_steps==0) 
            eright_ID -= n_revolution_steps;

         int ehi_ID=e+2*n_revolution_steps;
         int eleft_ID=e+n_revolution_steps;

         vertex lower_left_corner=get_edge_ptr(elo_ID)->get_V1();
         vertex upper_right_corner=get_edge_ptr(ehi_ID)->get_V2();

// On 8/4/13, we discovered the painful way that the call to
// update_triangle_mesh() within
// Polyhedron::build_current_polyhedron() seg faults if we do not
// explicitly triangulate the input polyhedron.  So we chop each
// quadrilateral face into two triangles via an explicit internal
// edge:

         bool internal_edge_flag=true;
         int ediag_ID=set_edge(
            lower_left_corner.get_ID(),
            upper_right_corner.get_ID(),
            internal_edge_flag);

//         cout << "elo = " << elo_ID 
//              << " eright = " << eright_ID
//              << " ehi = " << ehi_ID
//              << " eleft = " << eleft_ID
//              << endl;

         set_face(elo_ID,eright_ID,ediag_ID,get_n_faces());
         set_face(ediag_ID,ehi_ID,eleft_ID,get_n_faces());

//         set_face(elo_ID,eright_ID,ehi_ID,eleft_ID,get_n_faces());

      } // loop over index e labeling polygon edges
   } // loop over index i labeling revolution polygons
}

// ==========================================================================
// Search member functions
// ==========================================================================

// Member function find_vertex_ID_given_posn implements a brute-force,
// dumb search for a particular vertex within input STL vector
// Vertices given an input position.  If found, it returns the vertex
// ID.

int polyhedron::find_vertex_ID_given_posn(
   const threevector& posn,const vector<vertex>& Vertices)
{
   for (unsigned int v=0; v<Vertices.size(); v++)
   {
      if (posn.nearly_equal(Vertices[v].get_posn()))
      {
         return Vertices[v].get_ID();
      }
   } // loop over index v labeling vertices
//   cout << "Vertex corresponding to x = " << posn.get(0)
//        << " y = " << posn.get(1)
//        << " z = " << posn.get(2) 
//        << " not found in polyhedron's vertices member" << endl;
//   cout << "Vertices = " << endl;
//   templatefunc::printVector(Vertices);

   return -1;
}

// ---------------------------------------------------------------------
// Member function find_edge_ID_given_vertices_posns performs a
// brute-force search over the entries within input STL vector Edges
// for a match to the input edge specified by its two vertices.  If a
// match is found, the edge's ID is returned.  If not, this method
// returns -1.

int polyhedron::find_edge_ID_given_vertices_posns(
   const vertex& V1,const vertex& V2,const vector<edge>& Edges)
{
//   cout << "inside polyhedron::find_edge_ID_given_vertices_posns()" << endl;
   for (unsigned int e=0; e<Edges.size(); e++)
   {
      threevector p1(Edges[e].get_V1().get_posn());
      threevector p2(Edges[e].get_V2().get_posn());
      if ( (p1.nearly_equal(V1.get_posn()) && 
                            p2.nearly_equal(V2.get_posn())) ||
            (p1.nearly_equal(V2.get_posn()) && 
             p2.nearly_equal(V1.get_posn())) )
      {
//         cout << "found edge ID = " << edges[e].get_ID() << endl;
         return  Edges[e].get_ID();
      }
   } // loop over index e labeling edges
//   cout << "Edge corresponding to V1 = " << V1
//        << " and V2 = " << V2 
//        << " not found in polyhedron's edges member" << endl;
   return -1;
}

// ---------------------------------------------------------------------
// Member function find_edge_given_ID

edge* polyhedron::find_edge_given_ID(int ID,vector<edge>& Edges)
{
   for (unsigned int e=0; e<Edges.size(); e++)
   {
      if (Edges[e].get_ID()==ID)
      {
         return &(Edges[e]);
      }
   }
   return NULL;
}

// ---------------------------------------------------------------------
// Member function find_edge_given_vertices()

edge* polyhedron::find_edge_given_vertices(const vertex& V1,const vertex& V2)
{
   int edge_ID=find_edge_ID_given_vertices_posns(V1,V2,edges);
   return find_edge_given_ID(edge_ID,edges);
}

// ==========================================================================
// Index relabeling member functions
// ==========================================================================

// Member function copy_vertices_handler_contents_to_vector transfers
// the contents of *vertices_handler_ptr to input STL vector vertices.
// We wrote this little utility method in July 2008 when we abandoned
// vector<vertex> vertices as a member of polyhedron in favor of
// *vertices_handler_ptr.

void polyhedron::copy_vertices_handler_contents_to_vector(
   vector<vertex>& vertices)
{
   vertices.clear();
   for (unsigned int v=0; v<vertices_handler_ptr->get_n_vertices(); v++)
   {
      vertices.push_back(vertices_handler_ptr->get_vertex_from_chain(v));
   }
}

void polyhedron::copy_vector_contents_to_vertices_handler(
   const vector<vertex>& vertices)
{
//   vertices_handler_ptr->clear_vertex_chain();
//   vertices_handler_ptr->purge_vertex_table();
//   vertices_handler_ptr->clear_vertex_map();
   vertices_handler_ptr->clear_all_containers();
   for (unsigned int v=0; v<vertices.size(); v++)
   {
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(vertices[v]);
   }
}

// ---------------------------------------------------------------------
// Member function relabel_IDs_for_all_face_edges_and_vertices
// iterates over every face, edge and vertex within member STL vectors
// faces, edges and vertices. It resets the IDs for every vertex
// belonging to an edge and for every edge belonging to a face
// according to the integer values stored within the member STL
// vectors.

void polyhedron::relabel_IDs_for_all_face_edges_and_vertices()
{
   vector<vertex> vertices;
   copy_vertices_handler_contents_to_vector(vertices);

   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      relabel_vertex_IDs_for_face_edges(get_face_ptr(f),vertices,edges);
   } // loop over index f labeling faces

   copy_vector_contents_to_vertices_handler(vertices);
}

void polyhedron::relabel_IDs_for_face_edges_and_vertices(
   face* curr_face_ptr,const vector<vertex>& Vertices,
   const vector<edge>& Edges)
{
//   cout << "inside polyhedron::relabel_IDs_for_face_edges_and_vertices()"
//        << endl;
   relabel_vertex_IDs_for_face_edges(curr_face_ptr,Vertices,Edges);
   relabel_IDs_for_chain_vertices(curr_face_ptr,Vertices);
}

void polyhedron::relabel_vertex_IDs_for_face_edges(
   face* curr_face_ptr,const vector<vertex>& Vertices,
   const vector<edge>& Edges)
{
//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//   cout << "inside polyhedron::relabel_vertex_IDs_for_face_edges()" << endl;

   index_map_ptr->clear();
   for (unsigned int v=0; v<Vertices.size(); v++)
   {
      vertex curr_vertex(Vertices[v]);
      (*index_map_ptr)[curr_vertex.get_posn()]=curr_vertex.get_ID();
   }

   INDEX_MAP::iterator index_iter;
   vector<edge>* face_edges_ptr=curr_face_ptr->get_edge_chain_ptr();
   for (unsigned int e=0; e<face_edges_ptr->size(); e++)
   {
      vertex V1(face_edges_ptr->at(e).get_V1());
      vertex V2(face_edges_ptr->at(e).get_V2());

      index_iter=index_map_ptr->find(V1.get_posn());
      V1.set_ID(index_iter->second);
      index_iter=index_map_ptr->find(V2.get_posn());
      V2.set_ID(index_iter->second);

      int curr_edge_ID=find_edge_ID_given_vertices_posns(V1,V2,Edges);
      face_edges_ptr->at(e).set_ID(curr_edge_ID);

      edge* chain_edge_ptr=curr_face_ptr->
         find_edge_in_chain_given_edge(face_edges_ptr->at(e));

      if (chain_edge_ptr != NULL)
      {
         chain_edge_ptr->set_ID(curr_edge_ID);
         vertex V1=chain_edge_ptr->get_V1();
         vertex V2=chain_edge_ptr->get_V2();

         index_iter=index_map_ptr->find(V1.get_posn());
         V1.set_ID(index_iter->second);

         index_iter=index_map_ptr->find(V2.get_posn());
         V2.set_ID(index_iter->second);

         chain_edge_ptr->get_vertices_handler_ptr()->clear_all_containers();
//         chain_edge_ptr->get_vertices_handler_ptr()->clear_vertex_chain();
//         chain_edge_ptr->get_vertices_handler_ptr()->purge_vertex_table();
//         chain_edge_ptr->get_vertices_handler_ptr()->clear_vertex_map();

         chain_edge_ptr->get_vertices_handler_ptr()->
            update_vertex_in_table_chain_and_map(V1);
         chain_edge_ptr->get_vertices_handler_ptr()->
            update_vertex_in_table_chain_and_map(V2);
      } // chain_edge_ptr != NULL conditional
   } // loop over index e labeling current face edges
}

void polyhedron::relabel_IDs_for_chain_vertices(
   face* curr_face_ptr,const vector<vertex>& Vertices)
{
//   cout << "???????????????????????????????????????????????????" << endl;
//   cout << "inside polyhedron::relabel_IDs_for_chain_vertices()" << endl;

   vector<vertex> vertices;
   for (unsigned int v=0; v<curr_face_ptr->get_n_vertices(); v++)
   {
      vertex* V_ptr=&(curr_face_ptr->get_vertex_from_chain(v));
      V_ptr->set_ID(find_vertex_ID_given_posn(V_ptr->get_posn(),Vertices));
      vertices.push_back(*V_ptr);
   } // loop over index v labeling vertices within vertex_chain

   curr_face_ptr->get_vertices_handler_ptr()->clear_all_containers();
//   curr_face_ptr->get_vertices_handler_ptr()->clear_vertex_chain();
//   curr_face_ptr->get_vertices_handler_ptr()->purge_vertex_table();
//   curr_face_ptr->get_vertices_handler_ptr()->clear_vertex_map();
   for (unsigned int v=0; v<vertices.size(); v++)
   {
      curr_face_ptr->get_vertices_handler_ptr()->
         update_vertex_in_table_chain_and_map(vertices[v]);
   }
}

// ---------------------------------------------------------------------
// Member function relabel_IDs_for_face_edges takes in a face along
// with a fixed set of Vertices whose labels are assumed to be known.
// It loops over every edge within the face.  Any face edge which does
// not exist within input STL vector Edges is assigned a new ID and
// appended to Edges.

void polyhedron::relabel_IDs_for_face_edges(
   face& curr_face,const vector<vertex>& Vertices,vector<edge>& Edges)
{
//   cout << "inside polyhedron::relabel_IDs_for_face_edges()" << endl;
   for (unsigned int e=0; e<curr_face.get_n_edges(); e++)
   {
      edge* curr_edge_ptr=&(curr_face.get_edge(e));
      int edge_within_list_ID=find_edge_ID_given_vertices_posns(
         curr_edge_ptr->get_V1(),curr_edge_ptr->get_V2(),Edges);
      
      if (edge_within_list_ID==-1)
      {
         curr_face.get_edge(e).set_ID(Edges.size());
         Edges.push_back(curr_face.get_edge(e));
      }
      else
      {
         find_edge_given_ID(edge_within_list_ID,Edges)->
            set_ID(curr_edge_ptr->get_ID());
      }
   } // loop over index e labeling face edges
}

// ---------------------------------------------------------------------
// Member function relabel_IDs_for_all_edges transfers edge ID and
// vertex information from all faces within input STL vector Faces to
// input STL vector edges.

void polyhedron::relabel_IDs_for_all_edges(
   const vector<face>& Faces,vector<edge>& Edges)
{
//   cout << "inside polyhedron::relabel_IDs_for_all_edges()" << endl;
   for (unsigned int f=0; f<Faces.size(); f++)
   {
      const face* curr_face_ptr=&(Faces[f]);
      for (unsigned int e=0; e<curr_face_ptr->get_n_edges(); e++)
      {
         const edge* curr_edge_ptr=&(curr_face_ptr->get_edge(e));
         Edges[curr_edge_ptr->get_ID()]=*curr_edge_ptr;
      } // loop over index e labeling curr_face edges
   } // loop over index f labeling faces
//   cout << "Edges =  " << endl;
//   templatefunc::printVector(Edges);
}

// ---------------------------------------------------------------------
// Member function ray_intercept() takes in the basepoint and
// direction vector for some ray.  It loops over every face and checks
// whether the ray intercepts any of them.  If so, it computes the
// distance between the ray's basepoint and the candidate intercept
// point on the polyhedron.  This boolean method returns true if the
// ray intercepts the polyhedron and the intercepted point within
// closest_polyhedron_point.

bool polyhedron::ray_intercept(
   const threevector& ray_basepoint,const threevector& ray_hat,
   threevector& closest_polyhedron_point)
{
   vector<threevector> intercept_points;
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face* face_ptr=get_face_ptr(f);

      threevector projected_point_in_face_plane;
      if (face_ptr->ray_intercept(
         ray_basepoint,ray_hat,projected_point_in_face_plane))
      {
         intercept_points.push_back(projected_point_in_face_plane);
      }
   } // loop over index f labeling faces

   double closest_distance=POSITIVEINFINITY;
   for (unsigned int i=0; i<intercept_points.size(); i++)
   {
      double curr_distance=(intercept_points[i]-ray_basepoint).magnitude();
      if (curr_distance < closest_distance)
      {
         closest_distance=curr_distance;
         closest_polyhedron_point=intercept_points[i];
      }
   } // loop over index i labeling candidate intercept points
   return (intercept_points.size() > 0);
}

// ==========================================================================
// Polyhedron import/export member functions
// ==========================================================================

// Member function write_OFF_file outputs polyhedron information in
// Object File Format (.off) which represents an industry standard.

void polyhedron::write_OFF_file(string off_filename)
{
   ofstream off_stream;
   off_stream.precision(15);
   filefunc::openfile(off_filename,off_stream);
   off_stream << "OFF" << endl;
   off_stream << get_n_vertices() << " # number of vertices" << endl;
   off_stream << get_n_faces() << " # number of faces" << endl;
   off_stream << get_n_edges() << " # number of edges" << endl << endl;

   off_stream << "# Vertex coordinates:" << endl << endl;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      off_stream << get_vertex(v).get_posn().get(0) << " "
                 << get_vertex(v).get_posn().get(1) << " "
                 << get_vertex(v).get_posn().get(2) << " " << endl;
   } // loop over index v labeling vertices
   off_stream << endl;

   off_stream << "# Indices for face vertices:" << endl << endl;
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face curr_face(faces[f]);
      off_stream << curr_face.get_n_vertices() << " ";
      for (unsigned int v=0; v<curr_face.get_n_vertices(); v++)
      {
         off_stream << curr_face.get_vertex_from_chain(v).get_ID() << " ";
      }
      off_stream << endl;
   } // loop over index f labeling faces
   
   off_stream << endl;
}

// ---------------------------------------------------------------------
// Member function read_OFF_file() imports polyhedron information from a
// specified Object File Format (.off) file.  It reconstructs the
// current polyhedron's vertices, edges and faces from the information
// in the .off file.  If any color information is specified within the
// input .off file, it is returned within fourvector volume_color.

bool polyhedron::read_OFF_file(string off_filename,fourvector& volume_color)
{
//   cout << "inside polyhedron::read_OFF_file()" << endl;
   
   bool flag=filefunc::ReadInfile(off_filename);
   if (!flag)
   {
      cout << "Error in polyhedron::read_OFF_file()" << endl;
      cout << "Couldn't read in off_filename = " << off_filename << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   int line_number=0;
   string label=filefunc::text_line[line_number++];
   if (label != "OFF") 
   {
      cout << "Error in polyhedron::read_OFF_file()" << endl;
      cout << "Input file is not in Object File Format!" << endl;
      return false;
   }
   
   unsigned int n_vertices=stringfunc::string_to_number(
      filefunc::text_line[line_number++]);
   unsigned int n_faces=stringfunc::string_to_number(
      filefunc::text_line[line_number++]);
//   unsigned int n_edges=stringfunc::string_to_number(
//      filefunc::text_line[line_number++]);
   
//   cout << "n_vertices = " << n_vertices << endl;
//   cout << "n_faces = " << n_faces << endl;
//   cout << "n_edges = " << n_edges << endl;
   
// Parse vertex coordinates:

   vector<vertex> V;
   for (unsigned int v=0; v<n_vertices; v++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[line_number++]);
      threevector curr_V(column_values[0],column_values[1],column_values[2]);
      vertex curr_vertex(curr_V,v);
      V.push_back(curr_vertex);
   }
   set_vertices(V);

// Parse faces & extract edges:

   int curr_edge_ID=0;
   vector<edge> E;
   vector<face> F;
   volume_color=fourvector(-1,-1,-1,-1);
   for (unsigned int f=0; f<n_faces; f++)
   {
//      cout << "f = " << f << endl;
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[line_number++]);
      unsigned int n_vertices=column_values[0];

      if (column_values.size()==n_vertices+1)
      {
      }
      else if (column_values.size()==n_vertices+1+4)
      {
         double R=column_values[n_vertices+1];
         double G=column_values[n_vertices+2];
         double B=column_values[n_vertices+3];
         double A=column_values[n_vertices+4];
         volume_color=fourvector(R,G,B,A);
//         cout << "volume_color = " << volume_color << endl;
      }
      else
      {
         cout << "Error in input OFF file!" << endl;
         cout << "n_vertices = " << n_vertices 
              << " column_values.size() = " << column_values.size() << endl;
      }

      vector<vertex> face_vertices;
      for (unsigned int v=0; v<n_vertices; v++)
      {
         int curr_vertex_ID=column_values[v+1];
         vertex curr_vertex=get_vertices_handler_ptr()->
            get_vertex(curr_vertex_ID);
         face_vertices.push_back(curr_vertex);
      }
      face_vertices.push_back(face_vertices.front());

      vector<edge> curr_face_edges;

      for (unsigned int fv=0; fv<face_vertices.size()-1; fv++)
      {
         vertex V1=face_vertices[fv];
         vertex V2=face_vertices[fv+1];

         vertex Vfirst,Vsecond;
         if (V1.get_ID() < V2.get_ID())
         {
            Vfirst=V1;
            Vsecond=V2;
         }
         else
         {
            Vfirst=V2;
            Vsecond=V1;
         }
         int Vfirst_ID=Vfirst.get_ID();
         int Vsecond_ID=Vsecond.get_ID();

         edge curr_edge(Vfirst,Vsecond,curr_edge_ID);
         
// Perform brute force search to see if curr_edge already exists
// within edges STL vector member of current polyhedron:

         bool curr_edge_already_exists_flag=false;
         
         for (unsigned int e=0; e<E.size(); e++)
         {
            edge prev_edge=E[e];
            if (Vfirst_ID==prev_edge.get_V1().get_ID() &&
            Vsecond_ID==prev_edge.get_V2().get_ID())
            {
               curr_edge=prev_edge;
               curr_edge_already_exists_flag=true;
            }
            
         } // loop over index e labeling polyhedron's existing edges

         curr_face_edges.push_back(curr_edge);

         if (!curr_edge_already_exists_flag) 
         {
            E.push_back(curr_edge);
            curr_edge_ID++;
         }
      } // loop over index fv labeling current face's vertices

      face curr_face(curr_face_edges,f);
      F.push_back(curr_face);
//      cout << "curr_face = " << curr_face << endl;

   } // loop over index f labeling faces

   threevector origin=V.front().get_posn();
//   cout << "E.size() = " << E.size() << endl;

   reset_vertices_edges_faces(origin,V,E,F);
//   cout << "Reconstructed polyhedron = " << *this << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function reconstruct_rectangular_sides_from_OFF_file() 
// assumes the input OFF file corresponds to a rectangular prism.
// In this special case, the first n_vertices worth of face indices
// correspond to upper and lower triangle pairs which form rectangular
// side faces.  This method recovers the 4 vertices for each
// rectangular side and reconstructs a face from them.  The set of all
// polyhedron rectangle sides is returned within an STL vector.

vector<polygon*> polyhedron::reconstruct_rectangular_sides_from_OFF_file(
   string off_filename)
{
//   cout << "inside polyhedron::reconstruct_rectangular_sides_from_OFF_file()" 
//        << endl;

   vector<polygon*> rectangle_ptrs;

   filefunc::ReadInfile(off_filename);
   int line_number=0;
   string label=filefunc::text_line[line_number++];
   if (label != "OFF") 
   {
      cout << "Input file is Not in Object File Format!" << endl;
      return rectangle_ptrs;
   }
   
   unsigned int n_vertices=stringfunc::string_to_number(
      filefunc::text_line[line_number++]);
//   unsigned int n_faces=stringfunc::string_to_number(
//      filefunc::text_line[line_number++]);
//   unsigned int n_edges=stringfunc::string_to_number(
//      filefunc::text_line[line_number++]);
   
//   cout << "n_vertices = " << n_vertices << endl;
//   cout << "n_faces = " << n_faces << endl;
//   cout << "n_edges = " << n_edges << endl;
   
// Parse vertex coordinates:

   vector<vertex> V;
   for (unsigned int v=0; v<n_vertices; v++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[line_number++]);
      threevector curr_V(column_values[0],column_values[1],column_values[2]);
      vertex curr_vertex(curr_V,v);
      V.push_back(curr_vertex);
   }

// Parse rectangle side faces:

   unsigned int n_rectangles=n_vertices/2;

   if (triangleface_rectangle_map_ptr==NULL)
   {
      triangleface_rectangle_map_ptr=new TRIANGLEFACE_RECTANGLE_MAP;
   }

   for (unsigned int r=0; r<n_rectangles; r++)
   {
//      cout << "r = " << r << endl;
      vector<vertex> face_vertices;
      for (unsigned int t=0; t<2; t++)
      {

         vector<double> column_values=
            stringfunc::string_to_numbers(filefunc::text_line[line_number++]);
         unsigned int n_vertices=column_values[0];
         if (n_vertices != 3)
         {
            cout << "Error in polyhedron::reconstruct_rectangular_sides_from_OFF_file()" << endl;
         }

         for (unsigned int v=0; v<n_vertices; v++)
         {
            int curr_vertex_ID=column_values[v+1];
            vertex curr_vertex=get_vertices_handler_ptr()->
               get_vertex(curr_vertex_ID);
            face_vertices.push_back(curr_vertex);
         }

// Save association between (two) triangular face IDs with rectangular
// side face ID within member STL map *triangleface_rectangle_map_ptr:

         int triangle_face_ID=2*r+t;
         (*triangleface_rectangle_map_ptr)[triangle_face_ID]=r;

      } // loop over index t labeling triangles comprising side rectangle

      vector<threevector> rectangle_vertices;
      rectangle_vertices.push_back(face_vertices[5].get_posn());
      rectangle_vertices.push_back(face_vertices[3].get_posn());
      rectangle_vertices.push_back(face_vertices[2].get_posn());
      rectangle_vertices.push_back(face_vertices[0].get_posn());
      polygon* rectangle_ptr=new polygon(rectangle_vertices);
      rectangle_ptrs.push_back(rectangle_ptr);
   } // loop over index r labeling rectangular faces

   return rectangle_ptrs;
}

// ---------------------------------------------------------------------
// Member function write_PLY_file outputs polyhedron information in
// PLY file format (.ply) which represents an industry standard.

void polyhedron::write_PLY_file(string ply_filename)
{
   ofstream ply_stream;
   ply_stream.precision(12);
   filefunc::openfile(ply_filename,ply_stream);
   ply_stream << "ply" << endl;
   ply_stream << "format ascii 1.0" << endl;
   ply_stream << "element vertex " << get_n_vertices() << endl;
   ply_stream << "property float x " << endl;
   ply_stream << "property float y " << endl;
   ply_stream << "property float z " << endl;
   ply_stream << "element face " << get_n_faces() << endl;
   ply_stream << "property list uchar int vertex_index " << endl;
   ply_stream << "end_header" << endl;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      ply_stream << get_vertex(v).get_posn().get(0) << " "
                 << get_vertex(v).get_posn().get(1) << " "
                 << get_vertex(v).get_posn().get(2) << " " << endl;
   } // loop over index v labeling vertices

   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face curr_face(faces[f]);
      ply_stream << curr_face.get_n_vertices() << " ";
      for (unsigned int v=0; v<curr_face.get_n_vertices(); v++)
      {
         ply_stream << curr_face.get_vertex_from_chain(v).get_ID() << " ";
      }
      ply_stream << endl;
   } // loop over index f labeling faces
   
   ply_stream << endl;
}

// ---------------------------------------------------------------------
// Member function read_PLY_file() imports polyhedron information from
// a specified .ply file.  As of April 2012, we use the trimesh2
// library in order to parse polyhedra which are described in the ply
// file in terms of triangles.  This method reconstructs the
// polyhedron's vertices, edges and faces from the input triangle mesh
// information within the input ply file.

bool polyhedron::read_PLY_file(string ply_filename)
{
//   cout << "inside polyhedron::read_PLY_file()" << endl;
   
   TriMesh* m = TriMesh::read(ply_filename.c_str());
   if (!m) return false;

   unsigned int n_vertices=m->vertices.size();
   unsigned int n_faces=m->faces.size();

   vector<threevector> V;
   for (unsigned int v=0; v<n_vertices; v++)
   {
      threevector curr_V(
         m->vertices[v][0],m->vertices[v][1],m->vertices[v][2]);
      V.push_back(curr_V);
   }

   polyhedron* polyhedron_ptr=new polyhedron(V);
//   cout << "polyhedron = " << *this << endl;

   m->need_faces();
   for (unsigned int f=0; f<n_faces; f++)
   {
      int triangle_vertex_ID_0=m->faces[f][0];
      int triangle_vertex_ID_1=m->faces[f][1];
      int triangle_vertex_ID_2=m->faces[f][2];
      
//      cout << "Triangle index f = " << f << endl;
//      cout << "vertex_ID_0 = " << triangle_vertex_ID_0 << endl;
//      cout << "vertex_ID_1 = " << triangle_vertex_ID_1 << endl;
//      cout << "vertex_ID_2 = " << triangle_vertex_ID_2 << endl;
//      cout << endl;

      vertex vertex0=polyhedron_ptr->get_vertex(triangle_vertex_ID_0);
      vertex vertex1=polyhedron_ptr->get_vertex(triangle_vertex_ID_1);
      vertex vertex2=polyhedron_ptr->get_vertex(triangle_vertex_ID_2);
      
      edge* edge01_ptr=
         polyhedron_ptr->find_edge_given_vertices(vertex0,vertex1);
      int edge_ID_0,edge_ID_1,edge_ID_2;
      if (edge01_ptr==NULL) 
      {
         edge_ID_0=polyhedron_ptr->set_edge(
            triangle_vertex_ID_0,triangle_vertex_ID_1);
      }
      else
      {
         edge_ID_0=edge01_ptr->get_ID();
      }
      edge01_ptr=polyhedron_ptr->get_edge_ptr(edge_ID_0);

      edge* edge12_ptr=
         polyhedron_ptr->find_edge_given_vertices(vertex1,vertex2);
      if (edge12_ptr==NULL)
      {
         edge_ID_1=polyhedron_ptr->set_edge(
            triangle_vertex_ID_1,triangle_vertex_ID_2);
      }
      else
      {
         edge_ID_1=edge12_ptr->get_ID();
      }
      edge12_ptr=polyhedron_ptr->get_edge_ptr(edge_ID_1);

      edge* edge20_ptr=
         polyhedron_ptr->find_edge_given_vertices(vertex2,vertex0);
      if (edge20_ptr==NULL)
      {
         edge_ID_2=polyhedron_ptr->set_edge(
            triangle_vertex_ID_2,triangle_vertex_ID_0);
      }
      else
      {
         edge_ID_2=edge20_ptr->get_ID();
      }
      edge20_ptr=polyhedron_ptr->get_edge_ptr(edge_ID_2);
      polyhedron_ptr->set_face(edge_ID_0,edge_ID_1,edge_ID_2);
   }

//   m->need_normals();
//   cout << "Vertex 0 has normal " << m->normals[0] << endl;

   polyhedron_ptr->instantiate_face_network();

   *this=*polyhedron_ptr;
   delete polyhedron_ptr;

   return true;
}

// ---------------------------------------------------------------------
// Member function generate_surface_point_cloud() loops over all faces
// and evenly samples their points.  This method appends each surface
// point with the corresponding face normal and returns the pairs of
// threevectors within an STL vector.

vector<pair<threevector,threevector> >& 
polyhedron::generate_surface_point_cloud(double ds_frac)
{
//   cout << "inside polyhedron::generate_point_cloud()" << endl;

   surfacepnts_normals.clear();
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face* face_ptr=get_face_ptr(f);
      threevector n_hat=face_ptr->get_normal();
      vector<threevector> curr_point_cloud=face_ptr->get_polygon().
         generate_polygon_point_cloud(ds_frac);
      for (unsigned int i=0; i<curr_point_cloud.size(); i++)
      {
         pair<threevector,threevector> pnt_normal(curr_point_cloud[i],n_hat);
         surfacepnts_normals.push_back(pnt_normal);
      } // loop over index i labeling interior points for face f
   } // loop over index f labeling faces

//   cout << "surfacepnts_normals.size() = " << surfacepnts_normals.size() 
//    << endl;

   return surfacepnts_normals;
}

