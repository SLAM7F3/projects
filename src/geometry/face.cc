// =========================================================================
// Face class member function definitions
// =========================================================================
// Last modified on 7/7/11; 1/8/12; 1/29/12; 4/4/14
// =========================================================================

#include <iostream>
#include <set>
#include "math/constant_vectors.h"
#include "geometry/face.h"
#include "geometry/geometry_funcs.h"
#include "geometry/polygon.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void face::initialize_member_objects()
{
   ID=-1;
   handedness=unknown;
   area=-1;
   vertices_handler_ptr=NULL;
   edge_chain.clear();
}		 

void face::allocate_member_objects()
{
   vertices_handler_ptr=new vertices_handler;
}

// ---------------------------------------------------------------------
face::face(int id)
{
   initialize_member_objects();
   allocate_member_objects();
   ID=id;
}

// ---------------------------------------------------------------------
face::face(const threevector& p1,const threevector& p2,
           const threevector& p3,int id)
{
//   cout << "inside face(p1,p2,p3) constructor" << endl;

   initialize_member_objects();
   allocate_member_objects();
   ID=id;

   vector<vertex> vertices;
   vertices.push_back(vertex(p1,0));
   vertices.push_back(vertex(p2,1));
   vertices.push_back(vertex(p3,2));

//   cout << "vertices = " << endl;
//   templatefunc::printVector(vertices);

   vector<edge> input_edges;
   input_edges.push_back(edge(vertices[0],vertices[1],0));
   input_edges.push_back(edge(vertices[1],vertices[2],1));
   input_edges.push_back(edge(vertices[2],vertices[0],2));
   
   build_face(input_edges);
}

// ---------------------------------------------------------------------
face::face(const threevector& p1,const threevector& p2,
           const threevector& p3,const threevector& p4,int id)
{
//   cout << "inside face(p1,p2,p3,p4) constructor" << endl;

   initialize_member_objects();
   allocate_member_objects();
   ID=id;

   vector<vertex> vertices;
   vertices.push_back(vertex(p1,0));
   vertices.push_back(vertex(p2,1));
   vertices.push_back(vertex(p3,2));
   vertices.push_back(vertex(p4,3));

//   cout << "vertices = " << endl;
//   templatefunc::printVector(vertices);

   vector<edge> input_edges;
   input_edges.push_back(edge(vertices[0],vertices[1],0));
   input_edges.push_back(edge(vertices[1],vertices[2],1));
   input_edges.push_back(edge(vertices[2],vertices[3],2));
   input_edges.push_back(edge(vertices[3],vertices[0],3));

//   cout << "input_edges = " << endl;
//   templatefunc::printVector(input_edges);
   
   build_face(input_edges);
}

// ---------------------------------------------------------------------
face::face(const vector<threevector>& p,int id)
{
   initialize_member_objects();
   allocate_member_objects();
   ID=id;

   vector<vertex> vertices;
   for (unsigned int i=0; i<p.size(); i++)
   {
      vertices.push_back(vertex(p[i],i));
   }

//   cout << "vertices = " << endl;
//   templatefunc::printVector(vertices);

   vector<edge> input_edges;
   for (unsigned int i=0; i<vertices.size(); i++)
   {
      input_edges.push_back(
         edge(vertices[i],vertices[modulo(i+1,vertices.size())],i));
   }

   build_face(input_edges);
}

// ---------------------------------------------------------------------
face::face(const edge& e1,const edge& e2,const edge& e3,int id)
{
   initialize_member_objects();
   allocate_member_objects();
   ID=id;

   vector<edge> input_edges;
   input_edges.push_back(e1);
   input_edges.push_back(e2);
   input_edges.push_back(e3);
   build_face(input_edges);
}

// ---------------------------------------------------------------------
face::face(const edge& e1,const edge& e2,const edge& e3,const edge& e4,
           int id)
{
   initialize_member_objects();
   allocate_member_objects();
   ID=id;

   vector<edge> input_edges;
   input_edges.push_back(e1);
   input_edges.push_back(e2);
   input_edges.push_back(e3);
   input_edges.push_back(e4);
   build_face(input_edges);
}

// ---------------------------------------------------------------------
face::face(const vector<edge>& input_edges,int id)
{
   initialize_member_objects();
   allocate_member_objects();
   ID=id;

   build_face(input_edges);
}

// ---------------------------------------------------------------------
// Copy constructor:

face::face(const face& f)
{
//    cout << "inside copy constructor" << endl;
   initialize_member_objects();
   docopy(f);
}

face::~face()
{
//   cout << "inside face destructor, this = " << this << endl;
   delete vertices_handler_ptr;
   vertices_handler_ptr=NULL;
}

// ---------------------------------------------------------------------
void face::docopy(const face& f)
{
//   cout << "inside face::docopy()" << endl;
//   cout << "this = " << this << endl;

   ID=f.ID;
   handedness=f.handedness;
   area=f.area;
   COM=f.COM;
   normal=f.normal;
   face_poly=f.face_poly;

   edge_chain.clear();
   for (unsigned int i=0; i<f.edge_chain.size(); i++)
   {
      edge_chain.push_back(f.edge_chain[i]);
   }

//   cout << "vertices_handler_ptr = " << vertices_handler_ptr 
//        << endl;
   delete vertices_handler_ptr;
   vertices_handler_ptr=new vertices_handler(*f.vertices_handler_ptr);
}

// Overload = operator:

face& face::operator= (const face& f)
{
//   cout << "inside face::operator=, this = " << this << endl;
   if (this==&f) return *this;
   docopy(f);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const face& f)
{
   outstream << endl;
   outstream << "Face ID = " << f.ID << endl;
   outstream << "area = " << f.area << endl;
   outstream << "origin = COM: cx = " << f.COM.get(0)
             << " , cy = " << f.COM.get(1)
             << " , cz = " << f.COM.get(2) << endl;
   outstream << "normal: nx = " << f.normal.get(0) 
             << " , ny = " << f.normal.get(1) 
             << " , nz = " << f.normal.get(2) << endl;
   if (f.handedness==face::right_handed)
   {
      outstream << "Face is right handed" << endl;
   }
   else if (f.handedness==face::left_handed)
   {
      outstream << "Face is left handed" << endl;  
   }
   else if (f.handedness==face::unknown)
   {
      outstream << "Face handedness is unknown" << endl;
   }

   outstream << "---------- FACE VERTICES ----------" << endl;
   for (unsigned int v=0; v<f.get_n_vertices(); v++)
   {
      vertex curr_V(f.get_vertex_from_chain(v));
      outstream << "Vertex ID = " << curr_V.get_ID()
                << " : x = " << curr_V.get_posn().get(0)
                << " y = " << curr_V.get_posn().get(1)
                << " z = " << curr_V.get_posn().get(2) << endl;
   }
   outstream << endl;   

   outstream << "---------- FACE EDGES ----------" << endl;
   for (unsigned int e=0; e<f.get_n_edges(); e++)
   {
      edge curr_edge=f.edge_chain[e];
      outstream << "Edge ID = " << curr_edge.get_ID()
                << " : V0 ID = " << curr_edge.get_V1().get_ID()
                << " V1 ID = " << curr_edge.get_V2().get_ID()
                << " internal edge flag = " 
                << curr_edge.get_internal_edge_flag()
//                << " length = " << curr_edge.get_length() 
                << endl;
   }
   outstream << endl;

   return(outstream);
}

// =========================================================================
// Set & get member functions
// =========================================================================

// Member function get_polygon() returns a polygon formed from the
// face's vertex positions.

polygon face::get_polygon()
{
   return face_poly;
}

// =========================================================================
// Vertex map member functions
// =========================================================================

void face::update_vertex_map()
{
   vertices_handler_ptr->clear_vertex_map();
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertices_handler_ptr->update_vertex_map(get_vertex_from_chain(v));
   }
   
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edge_chain[e].update_vertex_map();
   }
}

// ---------------------------------------------------------------------

void face::reset_vertex_posn(
   const threevector& old_posn,const threevector& new_posn)
{
//   cout << "inside face::reset_vertex_posn()" << endl;
   vertices_handler_ptr->reset_vertex_posn(old_posn,new_posn);
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edge_chain[e].reset_vertex_posn(old_posn,new_posn);
   }
   update_vertex_map();
}

// =========================================================================
// Search member functions
// =========================================================================

// Member function find_edge_in_chain_given_edge performs a brute
// force search over all members of STL vector edge_chain for the
// input_edge.  It compares vertex positions between the input and
// candidate edges.  If a match is found, this method returns a
// pointer to the located edge.  Otherwise, it returns NULL.
 
edge* face::find_edge_in_chain_given_edge(const edge& input_edge)
{
//   cout << "inside face::find_edge_in_chain_given_edge()" << endl;
//   cout << "this = " << this << endl;

   for (unsigned int e=0; e<edge_chain.size(); e++)
   {
      if (edge_chain[e].nearly_equal_posn(input_edge))
      {
         return &(edge_chain[e]);
      }
   } // loop over index e labeling edges within edge_chain STL vector

//   cout << "At end of face::find_edge_in_chain_given_edge(), edge not found in chain"
//        << endl;
   return NULL;
}

// ---------------------------------------------------------------------
// Member function point_inside_convex_face determines whether a
// twovector assumed to lie within the same plane as the current
// polygon object lies inside the polygon.  The polygon is assumed to
// be CONVEX!  We first compute the vectors u[n] which connect the
// point to each of the polygon's vertices.  We next construct the
// cross products c[n] = u[n] x u[n+1].  If the point lies inside the
// polygon, then c[n] should point in the same direction for all n,
// provided the face is CONVEX!

// We have consciously attempted to streamline this method as much as
// possible for speed purposes.  But we have empirically determined
// that it is still faster to use "midpoint line algorithm" type
// approaches for filling in polygons or otherwise working with
// polygon interior sections.

bool face::point_inside_convex_face(const threevector& point) const
{
//   cout << "inside face::point_inside_convex_face()" << endl;
//   cout << "face handedness = " << handedness << endl;

   threevector u[get_n_vertices()];
   for (unsigned int n=0; n<get_n_vertices(); n++)
   {
      u[n]=get_vertex_from_chain(n).get_posn()-point;
   }

   threevector cross[get_n_vertices()];
   for (unsigned int n=0; n<get_n_vertices(); n++)
   {
      cross[n]=u[n].cross(u[modulo(n+1,get_n_vertices())]);
   }

   for (unsigned int n=0; n<get_n_vertices(); n++)
   {
      double dotproduct=cross[n].dot(cross[modulo(n+1,get_n_vertices())]);
//      cout << "dotproduct = " << dotproduct << endl;
      if (sgn(dotproduct) < 1) return false;
   }
   return true;
}

// ---------------------------------------------------------------------
// Member function ray_intercept()

bool face::ray_intercept(
   const threevector& ray_basepoint,const threevector& ray_hat,
   threevector& projected_point_in_face_plane)
{
//   cout << "inside face::ray_intercept()" << endl;
   
   if (face_poly.ray_projected_into_poly_plane(
      ray_basepoint,ray_hat,get_COM(),projected_point_in_face_plane))
   {
      if (point_inside_convex_face(projected_point_in_face_plane))
      {
         return true;
      }
   }
   return false;
}

// =========================================================================
// Construction member functions
// =========================================================================

// Member function build_face takes in a set of edges and cyclically
// permutes them so that the edge with the lowest ID appears first
// within the vertex chain.  Vertex information for the current face
// object is stored within hashtable member *vertex_table_ptr.  This
// method also computes the face's normal threevector.

bool face::build_face(const vector<edge>& input_edges)
{
//   cout << "inside face::build_face()" << endl;
   
   vector<int> face_edge_indices;
   for (unsigned int i=0; i<input_edges.size(); i++)
   {
      face_edge_indices.push_back(input_edges[i].get_ID());
//      cout << "i = " << i << " face_edge_indices = " 
//           << face_edge_indices.back() << endl;
   }

   bool face_edge_indices_valid_flag=true;
   for (unsigned int i=0; i<face_edge_indices.size(); i++)
   {
      if (face_edge_indices[i] < 0)
      {
         face_edge_indices_valid_flag=false;
      }

      for (unsigned int j=i+1; j<face_edge_indices.size(); j++)
      {
         if (face_edge_indices[i]==face_edge_indices[j])
         {
            face_edge_indices_valid_flag=false;
         }
      }
   } // loop over index i labeling input edge IDs

   if (face_edge_indices_valid_flag)
   {

// Cyclically permute face_edge_indices so that face_edge_indices[0]
// corresponds to minimal edge index value:

      int min_edge_index=POSITIVEINFINITY;
      int min_edge_index_location=0;
      for (unsigned int i=0; i<face_edge_indices.size(); i++)
      {
         if (face_edge_indices[i] < min_edge_index)
         {
            min_edge_index=face_edge_indices[i];
            min_edge_index_location=i;
         }
      }

      vertices_handler_ptr->purge_vertex_table();

      vector<edge> init_edges;
      for (unsigned int i=0; i<input_edges.size(); i++)
      {
         init_edges.push_back(input_edges[
            modulo(i+min_edge_index_location,input_edges.size())]);

         edge curr_edge=init_edges.back();
         vertex V1=curr_edge.get_V1();
         vertex V2=curr_edge.get_V2();
         vertices_handler_ptr->update_vertex_table_key(V1);
         vertices_handler_ptr->update_vertex_table_key(V2);

//         cout << "i = " << i 
//              << " curr_edge = " << curr_edge << endl;
//         cout << "V1 = " << V1 << " V2 = " << V2 << endl;
      }

      compute_vertex_chain(init_edges);
      compute_edge_chain_from_vertex_chain();
      compute_area_COM_and_normal();

   } // face_edge_indices_valid_flag conditional
   else
   {
      cout << "Error in face::build_face()" << endl;
      cout << "Not all input edges are valid!" << endl;
      cout << "No face built..." << endl;
      outputfunc::enter_continue_char();
   }

//   cout << "face_edge_indices_valid_flag = "
//        << face_edge_indices_valid_flag << endl;
//   cout << "face normal = " << get_normal() << endl;
   return face_edge_indices_valid_flag;
}

// ---------------------------------------------------------------------
// Member function compute_vertex_chain first extracts integer vertex
// index pairs (V1,V2) for all edges of face f.  It next performs a
// brute-force search for a vertex "chain" corresponding to the edges
// around f.  It stores this chain of vertices within member
// vertices_handler's vertex_chain member.

void face::compute_vertex_chain(const vector<edge>& init_edges)
{
//   cout << "inside face::compute_vertex_chain()" << endl;

   unsigned int n_edges=init_edges.size();
   vector<pair<int,int> > edge_vertex_pairs;
   for (unsigned int e=0; e<n_edges; e++)
   {
      edge_vertex_pairs.push_back(pair<int,int>(
         init_edges[e].get_V1().get_ID(),init_edges[e].get_V2().get_ID()));

//      cout << "e = " << e 
//           << " V1 index = " << edge_vertex_pairs.back().first
 //          << " V2 index = " << edge_vertex_pairs.back().second << endl;
   } // loop over index e labeling edges for face f

// Loop over index pairs for each edge.  Choose index which exists in
// both edge e and edge e+1:

//   vertices_handler_ptr->clear_all_containers();
   vertices_handler_ptr->clear_vertex_chain();

   for (unsigned int e=0; e<n_edges; e++)
   {
      pair<int,int> curr_pair=edge_vertex_pairs[e];
      pair<int,int> next_pair=edge_vertex_pairs[modulo(e+1,n_edges)];
      int c1=curr_pair.first;
      int c2=curr_pair.second;
      int n1=next_pair.first;
      int n2=next_pair.second;
//      cout << "e = " << e
//           << " c1 = " << c1 << " c2 = " << c2 
//           << " n1 = " << n1 << " n2 = " << n2 << endl;
      
      if (c1==n1 || c1==n2)
      {
         vertices_handler_ptr->update_vertex_in_table_chain_and_map(
            vertices_handler_ptr->get_vertex(c1));
      }
      else if (c2==n1 || c2==n2)
      {
         vertices_handler_ptr->update_vertex_in_table_chain_and_map(
            vertices_handler_ptr->get_vertex(c2));
      }
   } // loop over index e labeling edges for face f

//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function compute_edge_chain_from_vertex_chain loops over all
// vertices within ordered member vector vertex_chain, forms their
// corresponding edges and fills member vector edge_chain.  For GNU
// triangulated surface rendering purposes, we unfortunately cannot
// re-use member vector edges.

vector<edge>& face::compute_edge_chain_from_vertex_chain()
{
//   cout << "inside face::compute_edge_chain_from_vertex_chain()" << endl;
//   cout << "edge_chain.size() = " << edge_chain.size() << endl;

// First save IDs for edges within current chain before clearing
// edge_chain STL vector:

   int max_edge_ID=0;
   vector<int> edge_ID;
   for (unsigned int e=0; e<edge_chain.size(); e++)
   {
      edge_ID.push_back(edge_chain[e].get_ID());
      max_edge_ID=basic_math::max(max_edge_ID,edge_ID.back());
   }

// Fill any empty slots within edge_ID STL vector with -1 dummy ID
// values:

   for (unsigned int e=edge_chain.size(); e<get_n_vertices(); e++)
   {
      edge_ID.push_back(max_edge_ID++);
   }

   edge_chain.clear();
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertex curr_vertex(get_vertex_from_chain(v));
      vertex next_vertex(get_vertex_from_chain(
         modulo(v+1,get_n_vertices())));
      edge_chain.push_back(edge(curr_vertex,next_vertex,edge_ID[v]));
   }
   return edge_chain;
}

// ---------------------------------------------------------------------
// Member function compute_face_poly()

polygon& face::compute_face_poly()
{
//   cout << "inside face::compute_face_poly()" << endl;
   vector<threevector> V;
   for (unsigned int i=0; i<get_n_vertices(); i++)
   {
      vertex curr_vertex(get_vertex_from_chain(i));
      V.push_back(curr_vertex.get_posn());
   }
   face_poly=polygon(V);

//   cout << "face_poly = " << face_poly << endl;
   return face_poly;
}

// ---------------------------------------------------------------------
// Member function compute_area_COM_and_normal computes the area,
// center-of-mass and normal vector for the current face object.  This
// method assumes that the face is CONVEX!

void face::compute_area_COM_and_normal()
{
//   cout << "inside face::compute_area_COM_and_normal()" << endl;

   compute_face_poly();
   area=face_poly.compute_area();
   COM=face_poly.compute_COM();
   normal=face_poly.get_normal();

//   cout << "area = " << area << endl;
//   cout << "COM = " << COM << endl;
//   cout << "normal = " << normal << endl;
}

// ---------------------------------------------------------------------
// Member function handedness_wrt_direction

face::HandednessType face::handedness_wrt_direction(const threevector& p_hat)
{
//   cout << "inside face::handedness_wrt_direction" << endl;
//   cout << "p_hat = " << p_hat << endl;
   double dotproduct=p_hat.dot(normal);
   if (nearly_equal(dotproduct,0))
   {
      handedness=unknown;
      cout << "inside face::handedness_wrt_direction" << endl;
      cout << "handedness = unknown" << endl;
      cout << "p_hat = " << p_hat << endl;
      cout << "normal = " << normal << endl;
      cout << "dotproduct = " << dotproduct << endl;
   }
   else if (dotproduct > 0)
   {
      handedness=right_handed;
   }
   else if (dotproduct < 0)
   {
      handedness=left_handed;
   }
   return handedness;
}

// ---------------------------------------------------------------------
// Member function handedness_wrt_direction

void face::force_handedness_wrt_direction(
   face::HandednessType desired_handedness,const threevector& p_hat)
{
//   cout << "inside face::force_handedness_wrt_direction()" << endl;
   
   handedness_wrt_direction(p_hat);
   if (handedness==unknown)
   {
      cout << "Trouble in face::force_handedness_wrt_direction()" 
           << endl;
      cout << "Current handedness is unknown" << endl;
      cout << "p_hat = " << p_hat << endl;
   }
   else if (
      (handedness==left_handed && desired_handedness==right_handed) ||
      (handedness==right_handed && desired_handedness==left_handed))
   {
      swap_vertex_order();
      handedness=desired_handedness;
   }
}

// ---------------------------------------------------------------------
// Member function reverses the ordering of the vertices and edges
// within member STL vectors vertex_chain, edges and edge_chain.  It
// also swaps the order of the vertices within each edge member of
// edge_chain.  Finally, it flips the handedness and normal of the
// current face object.

void face::swap_vertex_order()
{
//   cout << "inside face::Swap_vertex_order()" << endl;

//   cout << "======================================================" << endl;
//   cout << "Original face " << endl;
//   cout << "======================================================" << endl;
//   cout << *this << endl;
//   outputfunc::enter_continue_char();

   unsigned int n_vertices=get_n_vertices();
   vector<vertex> reversed_vertex_chain;
   vector<edge> reversed_edge_chain;
   for (unsigned int i=0; i<n_vertices; i++)
   {
      reversed_vertex_chain.push_back(
         get_vertex_from_chain(get_n_vertices()-1-i));
      reversed_edge_chain.push_back(edge_chain[get_n_vertices()-1-i]);
   }

   vertices_handler_ptr->clear_all_containers();
//   vertices_handler_ptr->clear_vertex_chain();
   edge_chain.clear();
   for (unsigned int i=0; i<n_vertices; i++)
   {
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(
         reversed_vertex_chain[i]);
      edge_chain.push_back(reversed_edge_chain[i]);
      edge_chain.back().swap_vertices();
   }

   if (handedness==right_handed)
   {
      handedness=left_handed;
   }
   else if (handedness==left_handed)
   {
      handedness=right_handed;
   }
   normal = -normal;

//   cout << "======================================================" << endl;
//   cout << "Swapped face " << endl;
//   cout << "======================================================" << endl;
//   cout << *this << endl;
//   outputfunc::enter_continue_char();
}

// =========================================================================
// Above Z-plane methods
// =========================================================================

// Member function part_above_Zplane loops over every edge within the
// current face.  It starts at some vertex which lies above z=Z.  All
// subsequent vertices which also lies above z=Z are added to an STL
// vector.  For any edge which has one vertex pointing upward and one
// pointing downward, this method computes the point along the edge
// where it intersects the z=Z plane and adds that intersection point
// to the STL vector.  Once the method has finished iterating over all
// the base face edges, it constructs and returns a new face using the
// contents of the STL vector as input.

bool face::part_above_Zplane(double z,face& face_part_above_Zplane)
{
//   cout << "****************************************************" << endl;
//   cout << "inside face::part_above_Zplane()" << endl;

   if (below_Zplane(z)) 
   {
//      cout << "face below Zplane" << endl;
//      cout << "face = " << *this << endl;
      return false;
   }
   
// Find some vertex on current face object lying above Z=z:

   unsigned int v_init=0;
   while (get_vertex_from_chain(v_init).get_posn().get(2) < z 
          && v_init < get_n_vertices())
   {
      v_init++;
   }

//   cout << "v_init = " << v_init << endl;
//   cout << "get_n_vertices() = " << get_n_vertices() << endl;

   vector<threevector> pnts_above_z;
   for (unsigned int e=0; e<get_n_vertices(); e++)
   {
      int v=modulo(v_init+e,get_n_vertices());

//      cout << "e = " << e << " v = " << v << endl;
//      cout << "vertex_chain[v] = " << get_vertex_from_chain(v) << endl;

      vertex curr_vertex(get_vertex_from_chain(v));
      vertex next_vertex(get_vertex_from_chain(
         modulo(v+1,get_n_vertices())));
      
      if (curr_vertex.above_Zplane(z))
      {
         pnts_above_z.push_back(get_vertex_from_chain(v).get_posn());
      }
      if ((curr_vertex.above_Zplane(z) && next_vertex.below_Zplane(z)) ||
          (curr_vertex.below_Zplane(z) && next_vertex.above_Zplane(z)) )
      {
         edge curr_edge(get_vertex_from_chain(v),
                        get_vertex_from_chain(
                           modulo(v+1,get_n_vertices())));

         threevector intersection_posn(NEGATIVEINFINITY,NEGATIVEINFINITY,
                                       NEGATIVEINFINITY);
//         cout << "curr_edge = " << curr_edge << endl;
         if (curr_edge.intersection_with_Zplane(z,intersection_posn))
         {
            pnts_above_z.push_back(intersection_posn);
         }
//         cout << "intersection_posn = " << intersection_posn << endl;
      } // vertex_above_zplane conditional
   } // loop over index e labeling edges

//   cout << "pnts_above_z = " << endl;
//   templatefunc::printVector(pnts_above_z);

   if (pnts_above_z.size() > 2)
   {
      face_part_above_Zplane=face(pnts_above_z);
//   cout << "curr face = " << *this << endl;
//   cout << "face_part_above_Zplane = " << face_part_above_Zplane << endl;
      return true;
   }
   else
   {
      cout << "NO FACE PART ABOVE Z PLANE" << endl;
      return false;
   }
}

// =========================================================================
// Triangulation member functions
// =========================================================================

// Member function triangles_above_Zplane 

bool face::triangles_above_Zplane(
   double z,vector<face>& triangle_parts_above_Zplane)
{
   face face_part_above_Zplane(-1);
   if (!part_above_Zplane(z,face_part_above_Zplane))
   {
      return false;
   }
   face_part_above_Zplane.triangulate(triangle_parts_above_Zplane);
   return true;
}

// ---------------------------------------------------------------------
// Member function triangulate performs a brute force search over
// every possible set of triangulations of a CONVEX face.  Starting at
// a given vertex on the face, it forms a triangle fan to every other
// pair of vertices.  Any candidate triangle within the fan whose area
// equals 0 or the entire face's area is rejected.  The fan containing
// the greatest number of valid triangles is returned by this method.

void face::triangulate(vector<face>& face_triangles)
{
//   cout << "inside face::triangulate()" << endl;

// First make sure current face object has at least 3 vertices:

   if (get_n_vertices() <= 2) 
   {
      cout << "Error in face::triangulate()!" << endl;
      cout << "n_vertices = " << get_n_vertices() << endl;
      exit(-1);
   }
   
   unsigned int max_n_valid_triangles=0;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vector<face> triangle_candidates;
      threevector starting_posn(get_vertex_from_chain(v).get_posn());
      for (unsigned int i=0; i<get_n_vertices()-1; i++)
      {
         threevector next_posn(
            get_vertex_from_chain(modulo(v+i+1,get_n_vertices())).get_posn());
         threevector next_next_posn(
            get_vertex_from_chain(modulo(v+i+2,get_n_vertices())).get_posn());
         
         double curr_area=geometry_func::compute_triangle_area(
            starting_posn,next_posn,next_next_posn);
         double area_frac=curr_area/get_area();
         const double min_area_frac=1E-4;
         const double max_area_frac=0.99;
         if (area_frac > min_area_frac && area_frac < max_area_frac)
         {
            triangle_candidates.push_back(
               face(starting_posn,next_posn,next_next_posn));
         }
      } // loop over index i labeling following positions

//      cout << "index v = " << v
//           <<  " n_valid_triangles = " << triangle_candidates.size() << endl;

      if (triangle_candidates.size() > max_n_valid_triangles)
      {
         max_n_valid_triangles=triangle_candidates.size();
         face_triangles.clear();
         for (unsigned int t=0; t<triangle_candidates.size(); t++)
         {
            face_triangles.push_back(triangle_candidates[t]);
         }
      }
      
   } // loop over v index labeling candidate starting positions

//   cout << "# final triangle faces = " << face_triangles.size() << endl;
//   cout << "Final set of face triangles:" << endl;
//   templatefunc::printVector(face_triangles);
}

// =========================================================================
// Moving around face member functions
// =========================================================================

void face::translate(const threevector& rvec)
{
//   cout << "inside face::translate()" << endl;
//   cout << "get_n_vertices() = " << get_n_vertices() << endl;

   vector<vertex> translated_vertices;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertex curr_vertex=get_vertex_from_chain(v);
      curr_vertex.translate(rvec);
      translated_vertices.push_back(curr_vertex);
   }
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertices_handler_ptr->update_vertex_table_key(translated_vertices[v]);
   }

   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edge_chain[e].translate(rvec);
   }

   compute_area_COM_and_normal();
   update_vertex_map();
//   cout << "At end of face::translate()" << endl;
}

void face::absolute_position(const threevector& rvec)
{
//   cout << "inside face::abs posn()" << endl;
   threevector delta=rvec-COM;
   translate(delta);
}

// ---------------------------------------------------------------------
void face::scale(const threevector& scale_origin,
                 const threevector& scalefactor)
{
//   cout << "inside face::scale()" << endl;

   vector<vertex> scaled_vertices;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertex curr_vertex=get_vertex_from_chain(v);
      curr_vertex.scale(scale_origin,scalefactor);
      scaled_vertices.push_back(curr_vertex);
   }
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertices_handler_ptr->update_vertex_table_key(scaled_vertices[v]);
   }

   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edge_chain[e].scale(scale_origin,scalefactor);
   }

   compute_area_COM_and_normal();
   update_vertex_map();
}

// ---------------------------------------------------------------------
void face::rotate(const rotation& R)
{
   rotate(Zero_vector,R);
}

void face::rotate(const threevector& rotation_origin,const rotation& R)
{
//   cout << "inside face::rotate()" << endl;
   vector<vertex> rotated_vertices;
   
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertex curr_vertex=get_vertex_from_chain(v);
      curr_vertex.rotate(rotation_origin,R);
      rotated_vertices.push_back(curr_vertex);
   }
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      vertices_handler_ptr->update_vertex_table_key(rotated_vertices[v]);
   }
   
   for (unsigned int e=0; e<get_n_edges(); e++)
   {
      edge_chain[e].rotate(rotation_origin,R);
   }

   compute_area_COM_and_normal();
   update_vertex_map();
}

void face::rotate(const threevector& rotation_origin,
                  double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}
