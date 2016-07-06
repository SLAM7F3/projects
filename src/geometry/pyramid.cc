// ==========================================================================
// Pyramid class member function definitions
// ==========================================================================
// Last modified on 1/13/09; 9/30/11; 1/29/12; 4/4/14
// ==========================================================================

#include "templates/mytemplates.h"
#include "math/rotation.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"
#include "geometry/pyramid.h"


using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void pyramid::allocate_member_objects()
{
}		       

void pyramid::initialize_member_objects()
{
   base_ptr=NULL;
   zplane_face_ptr=NULL;
}

pyramid::pyramid()
{
//   cout << "inside pyramid() constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
   generate_canonical_square_pyramid();
}

pyramid::pyramid(const vector<threevector>& V)
{
//   cout << "inside pyramid(vector<V>) constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
   generate_square_pyramid(V);
}

pyramid::pyramid(const threevector& apex_posn,
                 const vector<threevector>& base_vertices)
{
//   cout << "inside pyramid(a,base_vertices) constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();

   vector<threevector> V;
   V.push_back(apex_posn);
   for (unsigned int i=0; i<base_vertices.size(); i++)
   {
      V.push_back(base_vertices[i]);
   }
   generate_square_pyramid(V);
}

pyramid::pyramid(
   const threevector& apex,const vector<vertex>& V,const vector<edge>& E,
   const vector<face>& F):
   polyhedron(apex,V,E,F)
{
//   cout << "inside pyramid(a,V,E,F) constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

pyramid::pyramid(const pyramid& p):
   polyhedron(p)
{
//   cout << "inside pyramid copy constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
   docopy(p);
}

pyramid::~pyramid()
{
   delete base_ptr;
   delete zplane_face_ptr;
}

// ---------------------------------------------------------------------
void pyramid::docopy(const pyramid& p)
{
//   cout << "inside pyramid::docopy()" << endl;

   delete base_ptr;
   if (p.base_ptr != NULL)
   {
      base_ptr=new face(*p.base_ptr);
   }
   
   delete zplane_face_ptr;
   if (p.zplane_face_ptr != NULL)
   {
      zplane_face_ptr=new face(*p.zplane_face_ptr);
   }
 }

// Overload = operator:

pyramid& pyramid::operator= (const pyramid& p)
{
   if (this==&p) return *this;
   polyhedron::operator=(p);
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const pyramid& p)
{
//   cout << "inside pyramid operator<<" << endl;

   outstream << (polyhedron&)p << endl;
   outstream << "APEX: " << endl;
   outstream << p.get_apex() << endl;
   outstream << endl;
   
   outstream << "BASE: " << endl;
   if (p.base_ptr==NULL)
   {
      outstream << "Base not computed" << endl;
   }
   else
   {
      outstream << *p.base_ptr << endl;
   }

   for (unsigned int v=0; v<p.base_ptr->get_n_vertices(); v++)
   {
      threevector curr_V(p.base_ptr->get_vertex_from_chain(v).get_posn());
      double sidelength=(p.get_apex().get_posn()-curr_V).magnitude();
      cout << "v = " << v << " sidelength = " << sidelength << endl;
   }


// FAKE FAKE:  Weds July 22 at 7:53 am
// Comment out next section for debugging purposes only...

/*
   outstream << "Z-PLANE FACE: " << endl;
   if (p.zplane_face_ptr==NULL)
   {
      outstream << "Zplane face not computed" << endl;
   }
   else
   {
      outstream << *p.zplane_face_ptr << endl;
   }
*/
 
   return outstream;
}

// =====================================================================
// Set & get member functions
// =====================================================================

// Member function get_center_axis_direction() returns the
// direction vector pointing from the pyramid's apex to the center of
// its bottom face.

threevector pyramid::get_center_axis_direction() const 
{
   threevector symmetry_axis=get_base_ptr()->get_COM()-get_apex().get_posn();
   return symmetry_axis.unitvector();
}

// =====================================================================
// Building member functions
// =====================================================================

void pyramid::generate_canonical_square_pyramid()
{
//   cout << "inside pyramid::generate_canonical_square_pyramid()" << endl;

   vector<threevector> V;
   V.push_back(threevector(0,0,0));		// apex
   V.push_back(threevector(-0.5,-0.5,-1));
   V.push_back(threevector(0.5,-0.5,-1));
   V.push_back(threevector(0.5,0.5,-1));
   V.push_back(threevector(-0.5,0.5,-1));
   generate_square_pyramid(V);
}

// ---------------------------------------------------------------------
// Member function generate_scaled_rotated_translated_square_pyramid
// takes in an apex position, a set of 4 side direction rays and a
// desired altitude for a pyramid.  It scales, rotates and translates
// a canonical square pyramid so that the current pyramid object
// matches the input data.

void pyramid::generate_scaled_rotated_translated_square_pyramid(
   const threevector& apex_posn,const vector<threevector>& UV_corner_dir,
   double altitude,double& Uscale,double& Vscale,
   threevector& Uhat,threevector& Vhat)
{
   vector<threevector> b;
   for (unsigned int c=0; c<4; c++)
   {
      b.push_back(apex_posn+UV_corner_dir[c]);
   }
   Uscale=(b[1]-b[0]).magnitude();
   Vscale=(b[2]-b[1]).magnitude();

   Uhat=(b[1]-b[0]).unitvector();
   Vhat=(b[2]-b[1]).unitvector();
   
   plane base(b[0],b[1],b[2]);
   double prelim_altitude=fabs(base.signed_distance_from_plane(apex_posn));
   
   double magnification=altitude/prelim_altitude;
   Uscale *= magnification;
   Vscale *= magnification;

   generate_scaled_rotated_translated_square_pyramid(
      apex_posn,Uscale,Vscale,altitude,Uhat,Vhat);
}

// ---------------------------------------------------------------------
// Member function generate_scaled_rotated_translated_square_pyramid
// resets the current pyramid object to its canonical square form.  It
// then scales, rotates and translates the canonical pyramid so that
// its U and V vectors along with its altitude and apex match those
// specified by the input parameters.

void pyramid::generate_scaled_rotated_translated_square_pyramid(
   const threevector& apex_posn,
   double Uscale,double Vscale,double altitude,
   const threevector& Uhat,const threevector& Vhat)
{
   generate_canonical_square_pyramid();
   scale(Zero_vector,threevector(Uscale,Vscale,altitude));

   rotation R(Uhat,Vhat,Uhat.cross(Vhat));
   rotate(Zero_vector,R);

   translate(apex_posn);
   update_vertex_map();
}

void pyramid::generate_scaled_rotated_translated_square_pyramid(
   const threevector& scale_factors,const genmatrix& R,
   const threevector& apex_posn)
{
   generate_canonical_square_pyramid();
   scale(Zero_vector,scale_factors);

   if (R.get_mdim()==3 && R.get_ndim()==3)
   {
      rotation r(R);
      rotate(Zero_vector,r);
   }
   else
   {
      cout << "Error in pyramid::generate_scaled_rotated_translated_square_pyramid()" << endl;
      cout << "R.get_mdim() = " << R.get_mdim()
           << " R.get_ndim() = " << R.get_ndim() << endl;
      exit(-1);
   }

   translate(apex_posn);
   update_vertex_map();
}

// ---------------------------------------------------------------------
void pyramid::generate_square_pyramid(const vector<threevector>& V)
{
//   cout << "inside pyramid::generate_square_pyramid(V)" << endl;

   polyhedron::generate_square_pyramid(V);

   vector<threevector> base_posns;
   for (unsigned int i=0; i<4; i++)
   {
      base_posns.push_back(V[i+1]);
   }

   delete base_ptr;
   base_ptr=new face(base_posns);

   vector<vertex> vertices;
   copy_vertices_handler_contents_to_vector(vertices);
   relabel_IDs_for_face_edges_and_vertices(base_ptr,vertices,edges);
   copy_vector_contents_to_vertices_handler(vertices);

   update_vertex_map();

//   cout << "*this = " << *this << endl;
//   cout << "base = " << base << endl;
}

// ---------------------------------------------------------------------
void pyramid::generate_or_reset_square_pyramid(
   const threevector& apex_posn,const vector<threevector>& base_vertices)
{
//   cout << "inside pyramid::generate_or_reset_square_pyramid()" << endl;
   if (get_vertices_handler_ptr()->get_n_vertices() > 0)
   {
      reset_square_pyramid_vertices(apex_posn,base_vertices);
   }
   else
   {
      generate_square_pyramid(apex_posn,base_vertices);
   }
}

// ---------------------------------------------------------------------
void pyramid::generate_square_pyramid(
   const threevector& apex_posn,const vector<threevector>& base_vertices)
{
//   cout << "inside pyramid::generate_square_pyramid(apex,base_vertices)" 
//        << endl;

   vector<threevector> V;
   V.push_back(apex_posn);
   for (unsigned int i=0; i<base_vertices.size(); i++)
   {
      V.push_back(base_vertices[i]);
   }

   generate_square_pyramid(V);
}

// ---------------------------------------------------------------------
// Member function reset_square_pyramid_vertices reassigns the apex
// and base vertices of the current pyramid object to the threevectors
// passed as inputs.

void pyramid::reset_square_pyramid_vertices(
   const threevector& new_apex,const vector<threevector>& new_base_vertices)
{
//   cout << "inside pyramid::reset_square_pyramid_vertices()" << endl;

   base_ptr->get_vertices_handler_ptr()->clear_vertex_map();

//   cout << "new_apex = " << new_apex << endl;
//   cout << "origin_vertex_ptr = " << origin_vertex_ptr << endl;
//   cout << "*origin_vertex_ptr = " << *origin_vertex_ptr << endl;
//   cout << "get_apex() = " << get_apex() << endl;
//   cout << "get_apex().get_posn() = "
//        << get_apex().get_posn() << endl;

   reset_vertex_posn(get_apex().get_posn(),new_apex);

   for (unsigned int v=1; v<=4; v++)
   {
//      cout << "v = " << v << endl;
//      cout << "get_vertex(v).get_posn() = "
//           << get_vertex(v).get_posn() << endl;
//      cout << " new_base_vertices[v-1] = " 
//           << new_base_vertices[v-1] << endl;
      
      reset_vertex_posn(get_vertex(v).get_posn(),new_base_vertices[v-1]);

      vertex curr_vertex=base_ptr->get_vertex_from_chain(v-1);
      curr_vertex.set_posn(new_base_vertices[v-1]);
      base_ptr->get_vertices_handler_ptr()->set_chain_vertex(v-1,curr_vertex);
      base_ptr->get_vertices_handler_ptr()->
         update_vertex_in_table_chain_and_map(curr_vertex);
   }
   update_vertex_map();
//   base_ptr->update_vertex_map();

//   cout << "*this = " << *this << endl;
//   cout << "*base_ptr = " << *base_ptr << endl;

//   cout << "vertices_handler_ptr->get_n_vertices() = "
//        << get_vertices_handler_ptr()->get_n_vertices() << endl;
//   cout << "base_ptr->get_n_vertices() = "
//        << base_ptr->get_n_vertices() << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function ensure_faces_handedness

void pyramid::ensure_faces_handedness(face::HandednessType desired_handedness)
{
//   cout << "inside pyramid::ensure_faces_handedness" << endl;
   compute_COM();
   polyhedron::ensure_faces_handedness(desired_handedness);
   threevector p_hat( (base_ptr->get_COM()-get_COM()).unitvector() );
   base_ptr->force_handedness_wrt_direction(desired_handedness,p_hat);
}

// ==========================================================================
// Search member functions
// ==========================================================================

void pyramid::relabel_IDs_for_all_face_edges_and_vertices()
{
//   cout << "inside pyramid::relable_IDs_for_all_face_edges_and_vertices()" << endl;

   polyhedron::relabel_IDs_for_all_face_edges_and_vertices();

   vector<vertex> vertices;
   copy_vertices_handler_contents_to_vector(vertices);
   relabel_IDs_for_face_edges_and_vertices(base_ptr,vertices,edges);
   relabel_IDs_for_face_edges_and_vertices(zplane_face_ptr,vertices,edges);
   copy_vector_contents_to_vertices_handler(vertices);
}

// ==========================================================================
// Above Z-plane member functions
// ==========================================================================

// Member function lies_above_Zplane_check() loops over all pyramid
// vertices.  If any one has a z component that lies below the input z
// argument, this boolean method returns false.  Otherwise, it returns
// true.

bool pyramid::lies_above_Zplane_check(double z)
{
//   cout << "inside pyramid::lies_above_Zplane_check()" << endl;

   bool lies_above_Zplane_flag=true;
   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      if (get_vertex(v).get_posn().get(2) < z)
      {
         lies_above_Zplane_flag=false;
         break;
      }
   }
   return lies_above_Zplane_flag;
}

// ---------------------------------------------------------------------
// Member function extract_parts_above_Zplane loops over all faces
// within the current pyramid object.  For each face, it first
// computes the part located above Z=z.  It then decomposes the
// subface into triangles and returns them within output STL vector
// triangles_above_Zplane.  This method next extracts from the
// triangles the vertices and edges lying above Z=z and stores them
// within vertices_above_Zplane and edges_above_Zplane.  Finally, this
// method returns the z=Z plane face within output face zplane_face.

void pyramid::extract_parts_above_Zplane(
   double z,vector<vertex>& vertices_above_Zplane,
   vector<edge>& edges_above_Zplane,vector<face>& triangles_above_Zplane)
{
//   cout << "inside pyramid::extract_parts_above_Zplane()" << endl;
//   cout << "z = " << z << endl;
//   cout << "vertices_above_Zplane.size() = "
//        << vertices_above_Zplane.size() << endl;
//   cout << "apex = " << get_apex() << endl;
//   cout << "base = " << *(get_base_ptr()) << endl;
   
   index_map_ptr->clear();
   for (unsigned int v=0; v<vertices_above_Zplane.size(); v++)
   {
      vertex* curr_vertex_ptr=&(vertices_above_Zplane[v]);
      (*index_map_ptr)[curr_vertex_ptr->get_posn()]=curr_vertex_ptr->get_ID();
   }

   int v_counter=0;
   int e_counter=0;

   face curr_face_above_Zplane(-1);
   vector<face> faces_above_Zplane;
   for (unsigned int f=0; f<get_n_faces(); f++)
   {
      face* face_ptr=get_face_ptr(f);

      if (face_ptr->part_above_Zplane(z,curr_face_above_Zplane))
      {
//         ensure_face_handedness(
//            &curr_face_above_Zplane,get_faces_handedness());

// Extract vertices from curr_face_above_Zplane.  Add those which are
// not already stored within vertices_above_Zplane to the STL vector:

         for (unsigned int v=0; v<curr_face_above_Zplane.get_n_vertices(); v++)
         {
            vertex curr_vertex=curr_face_above_Zplane.
               get_vertex_from_chain(v);
            INDEX_MAP::iterator index_iter=index_map_ptr->find(
               curr_vertex.get_posn());

            if (index_iter==index_map_ptr->end())
            { 
               vertex new_vertex(curr_vertex.get_posn(),v_counter++);
               vertices_above_Zplane.push_back(new_vertex);
               (*index_map_ptr)[new_vertex.get_posn()]=new_vertex.get_ID();
            }
         } // loop over index v labeling vertices above z-plane

// Extract edges from curr_face_above_Zplane.  Add those which are not
// already stored within edges_above_Zplane to the STL vector:

         vector<edge>* edge_chain_ptr=curr_face_above_Zplane.
            get_edge_chain_ptr();
         for (unsigned int e=0; e<edge_chain_ptr->size(); e++)
         {
            if (find_edge_ID_given_vertices_posns(
               edge_chain_ptr->at(e).get_V1().get_posn(),
               edge_chain_ptr->at(e).get_V2().get_posn(),edges_above_Zplane) 
                < 0)
            {
               edges_above_Zplane.push_back(edge_chain_ptr->at(e));
               edges_above_Zplane.back().set_ID(e_counter++);
            }
         } // loop over index e labeling edges above z-plane

//         cout << "curr_face_above_Zplane = "
//              << curr_face_above_Zplane << endl;

         faces_above_Zplane.push_back(curr_face_above_Zplane);
      } // face_part_above_Zplane==true conditional
   } // loop over index f labeling pyramid faces

//   cout << "Vertices above Zplane = " << endl;
//   templatefunc::printVector(vertices_above_Zplane);
//   cout << "Edges above Zplane = " << endl;
//   templatefunc::printVector(edges_above_Zplane);
//   cout << "Faces above Zplane = " << endl;
//   templatefunc::printVector(faces_above_Zplane);

//   outputfunc::enter_continue_char();

// Decompose faces above Zplane into triangles:

//   cout << "=====================================================" << endl;
//   cout << "Faces above Zplane" << endl;
//   cout << "=====================================================" << endl;
//   cout << "faces_above_Zplane.size() = " << faces_above_Zplane.size() 
//        << endl;

   vector<face> face_triangles;
   for (unsigned int f=0; f<faces_above_Zplane.size(); f++)
   {
      face_triangles.clear();
      face* curr_face_ptr=&(faces_above_Zplane[f]);
      if (curr_face_ptr->get_n_edges()==3)
      {
         face_triangles.push_back(*curr_face_ptr);
      }
      else
      {
         curr_face_ptr->triangulate(face_triangles);
//         face_triangles=curr_face_ptr->triangulate();
      }
//      cout << "f = " << f << " face_triangles.size() = " 
//           << face_triangles.size() << endl;
      for (unsigned int t=0; t<face_triangles.size(); t++)
      {
//         ensure_face_handedness(
//            &face_triangles[t],get_faces_handedness());
         triangles_above_Zplane.push_back(face_triangles[t]);
      }
   } // loop over index f labeling faces above Zplane

// Relabel edges and vertices for triangles above Z-plane using IDs
// taken from edges_above_Zplane and vertices_above_Zplane STL
// vectors:

//   cout << "=====================================================" << endl;
//   cout << "Triangles above Zplane" << endl;
//   cout << "=====================================================" << endl;

//   cout << "triangles_above_Zplane.size() = " 
//        << triangles_above_Zplane.size() << endl;
//   templatefunc::printVector(triangles_above_Zplane);

   for (unsigned int t=0; t<triangles_above_Zplane.size(); t++)
   {
//      cout <<  "t = " << t << endl;
      triangles_above_Zplane[t].set_ID(t);
      relabel_IDs_for_face_edges_and_vertices(
         &triangles_above_Zplane[t],vertices_above_Zplane,edges_above_Zplane);
      relabel_IDs_for_face_edges(
         triangles_above_Zplane[t],vertices_above_Zplane,edges_above_Zplane);
//      cout  << " triangle above Zplane = " << triangles_above_Zplane[t] 
//            << endl;
   } // loop over index t labeling triangles above Zplane

   form_zplane_face(z,vertices_above_Zplane,edges_above_Zplane,
                    triangles_above_Zplane);

//   cout << "=====================================================" << endl;
//   cout << "Triangles above Zplane" << endl;
//   cout << "=====================================================" << endl;
//   cout << "triangles_above_Zplane.size() = " 
//        << triangles_above_Zplane.size() << endl;
//   for (unsigned int t=0; t<triangles_above_Zplane.size(); t++)
//   {
//      cout <<  "t = " << t
//           << " triangle above Zplane = " << triangles_above_Zplane[t] 
//           << endl;
//   } // loop over index t labeling triangles above Zplane

// Make sure edge and vertex IDs within edges_above_Zplane STL vector
// match those within triangles_above_Zplane:

   relabel_IDs_for_all_edges(triangles_above_Zplane,edges_above_Zplane);
}

// ---------------------------------------------------------------------
// Member function form_zplane_face takes in vertices and edges
// located above the z=Z plane.  It computes the average location of
// the vertices with z=Z which lies inside the zplane face provided it
// is convex.  It then generates a polygon object from the unordered
// vertices and the average interior location.  An ordered set of
// vertices is then read out of the polygon and used to generate a
// face object.  The face's handedness is forced to equal that of all
// other pyramid faces.  The face's triangles are appended to the
// input/output STL vector triangles_above_Zplane.  The face itself is
// also returned by this method.

bool pyramid::form_zplane_face(
   double z,const vector<vertex>& vertices_above_Zplane,
   vector<edge>& edges_above_Zplane,
   vector<face>& triangles_above_Zplane)
{
//   cout << "inside pyramid::form_zplane_face()" << endl;

// First perform some basic sanity checks on number of vertices
// located above and within Zplane as well as number of side rays
// pointing upwards:

//   cout << "vertices_above_Zplane.size() = "
//        << vertices_above_Zplane.size() << endl;
//   templatefunc::printVector(vertices_above_Zplane);

   int n_vertices_within_zplane=0;
   for (unsigned int v=0; v<vertices_above_Zplane.size(); v++)
   {
      double curr_z=vertices_above_Zplane[v].get_posn().get(2);
      if (nearly_equal(curr_z,z))
      {
         n_vertices_within_zplane++;
      }
   }
//   cout << "n_vertices_within_zplane = "
//        << n_vertices_within_zplane << endl;

   if (vertices_above_Zplane.size()==0 || 
      n_vertices_within_zplane < 3) return false;

   threevector avg_posn_within_Zplane;
   vector<threevector> posns_within_Zplane;
   for (unsigned int v=0; v<vertices_above_Zplane.size(); v++)
   {
      threevector curr_posn(vertices_above_Zplane[v].get_posn());
      if (nearly_equal(curr_posn.get(2),z))
      {
         avg_posn_within_Zplane += curr_posn;
         posns_within_Zplane.push_back(curr_posn);
      }
   }

// Note added on 7/10 at 7:17 am...

// Should really check that no 3 posns_within_Zplane are collinear.
// If so, eliminate the middle posn...Perform this refinement only
// after we start working with base_above_Zplane() method below.

   avg_posn_within_Zplane /= posns_within_Zplane.size();
//   cout << "avg_posn_within_Zplane = " << avg_posn_within_Zplane << endl;

//   cout << "posns within Z plane = " << endl;
//   templatefunc::printVector(posns_within_Zplane);

   polygon zplane_poly(avg_posn_within_Zplane,posns_within_Zplane);
//   cout << "zplane_poly = " << zplane_poly << endl;

   vector<threevector> ordered_posns;
   for (unsigned int n=0; n<zplane_poly.get_nvertices(); n++)
   {
      ordered_posns.push_back(zplane_poly.get_vertex(n));
   }

   delete zplane_face_ptr;
   zplane_face_ptr=new face(ordered_posns,triangles_above_Zplane.size());
   relabel_IDs_for_face_edges_and_vertices(
      zplane_face_ptr,vertices_above_Zplane,edges_above_Zplane);

// Compute centroid for triangles lying above z-plane.  Use this new
// COM to set handedness of zplane face to conform with that of all
// other faces:

   threevector new_COM(compute_faces_centroid(triangles_above_Zplane));
   threevector p_hat( (zplane_face_ptr->get_COM()-new_COM).unitvector() );
   zplane_face_ptr->force_handedness_wrt_direction(
      get_faces_handedness(),p_hat);

//   cout << "zplane_face = " << *zplane_face_ptr << endl;

//   vector<face> zplane_triangles=zplane_face_ptr->triangulate();
   vector<face> zplane_triangles;
   zplane_face_ptr->triangulate(zplane_triangles);
   for (unsigned int t=0; t<zplane_triangles.size(); t++)
   {
      zplane_triangles[t].force_handedness_wrt_direction(
         get_faces_handedness(),p_hat);
      relabel_IDs_for_face_edges_and_vertices(
         &zplane_triangles[t],vertices_above_Zplane,edges_above_Zplane);
      relabel_IDs_for_face_edges(
         zplane_triangles[t],vertices_above_Zplane,edges_above_Zplane);
      zplane_triangles[t].set_ID(triangles_above_Zplane.size());
      triangles_above_Zplane.push_back(zplane_triangles[t]);

//      cout << "zplane triangle:" << endl;
//      cout << triangles_above_Zplane.back() << endl;
   }
   return true;
}

// ==========================================================================
// Moving around polyhedra member functions
// ==========================================================================

void pyramid::translate(const threevector& rvec)
{
   polyhedron::translate(rvec);

   if (base_ptr != NULL) base_ptr->translate(rvec);
   if (zplane_face_ptr != NULL) zplane_face_ptr->translate(rvec);
}

void pyramid::absolute_position(const threevector& rvec)
{
//   cout << "inside pyramid::absolute_posn()" << endl;
   threevector delta=rvec-get_origin();
   translate(delta);
}

void pyramid::scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   polyhedron::scale(scale_origin,scalefactor);
   if (base_ptr != NULL) base_ptr->scale(scale_origin,scalefactor);
   if (zplane_face_ptr != NULL) 
      zplane_face_ptr->scale(scale_origin,scalefactor);
}

void pyramid::rotate(const rotation& R)
{
   rotate(Zero_vector,R);
}

void pyramid::rotate(const threevector& rotation_origin,const rotation& R)
{
//   cout << "inside pyramid::rotate(rot_origin,R)" << endl;
   polyhedron::rotate(rotation_origin,R);

   if (base_ptr != NULL) base_ptr->rotate(rotation_origin,R);
   if (zplane_face_ptr != NULL) zplane_face_ptr->rotate(rotation_origin,R);
}

void pyramid::rotate(const threevector& rotation_origin,
                     double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}
