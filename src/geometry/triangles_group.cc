// ==========================================================================
// Triangles_Group class member function definitions
// ==========================================================================
// Last modified on 10/20/13; 10/22/13; 10/23/13; 4/4/14
// ==========================================================================

#include <algorithm>
#include <del_interface.hpp>
#include "geometry/triangles_group.h"
#include "geometry/triangulater.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

using namespace tpp;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void triangles_group::allocate_member_objects()
{
   triangles_map_ptr=new TRIANGLES_MAP;

   bool edge_flag=true;
   vertices_handler_ptr=new vertices_handler(edge_flag);
}		       

void triangles_group::initialize_member_objects()
{
   delaunay_triangles_vertex_map_ptr=NULL;
   vertices_network_map_ptr=NULL;
}

triangles_group::triangles_group() 
{
   allocate_member_objects();
   initialize_member_objects();
}

triangles_group::~triangles_group()
{
   delete vertices_handler_ptr;

   for (TRIANGLES_MAP::iterator iter=triangles_map_ptr->begin(); 
        iter != triangles_map_ptr->end(); iter++)
   {
      delete iter->second;
   }
   delete triangles_map_ptr;
   delete delaunay_triangles_vertex_map_ptr;
   delete vertices_network_map_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

unsigned int triangles_group::get_n_vertices() const
{
   return vertices_handler_ptr->get_vertex_map_size();
}

unsigned int triangles_group::get_n_triangles() const
{
   return triangles_map_ptr->size();
}

triangle* triangles_group::get_triangle_ptr(int i)
{
   TRIANGLES_MAP::iterator iter=triangles_map_ptr->find(i);
   if (iter==triangles_map_ptr->end()) 
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

// ==========================================================================
// Triangle generation member functions
// ==========================================================================

// Member function generate_new_triangle()

triangle* triangles_group::generate_new_triangle(
   const threevector& v1,const threevector& v2,const threevector& v3,
   int triangle_ID) 
{
//   cout << "inside triangles_group::generate_new_triangle() #1" << endl;
//   cout << "v1 = " << v1 << endl;
//   cout << "v2 = " << v2 << endl;
//   cout << "v3 = " << v3 << endl;

   if (vertices_handler_ptr->get_vertex_ptr(v1)==NULL)
   {
      vertex V1(v1,vertices_handler_ptr->get_vertex_map_size());
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(V1);
   }

   if (vertices_handler_ptr->get_vertex_ptr(v2)==NULL)
   {
      vertex V2(v2,vertices_handler_ptr->get_vertex_map_size());
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(V2);
   }

   if (vertices_handler_ptr->get_vertex_ptr(v3)==NULL)
   {
      vertex V3(v3,vertices_handler_ptr->get_vertex_map_size());
      vertices_handler_ptr->update_vertex_in_table_chain_and_map(V3);
   }

   vector<vertex> vertices;
   vertices.push_back(*(vertices_handler_ptr->get_vertex_ptr(v1)));
   vertices.push_back(*(vertices_handler_ptr->get_vertex_ptr(v2)));
   vertices.push_back(*(vertices_handler_ptr->get_vertex_ptr(v3)));

// On 10/23/13, we discovered the painful way that the vertices in the
// STL vector can sometimes have overlapping IDs.  So we explicitly
// check whether all vertex IDs are unique.  If not, this method
// returns NULL:

   if (vertices[0].get_ID()==vertices[1].get_ID() 
   || vertices[1].get_ID()==vertices[2].get_ID() 
   || vertices[2].get_ID()==vertices[0].get_ID())
   {
      cout << "Trouble in triangles_group::generate_new_triangle() #1" << endl;
//      cout << "vertices[0] = " << vertices[0] << endl;
//      cout << "vertices[1] = " << vertices[1] << endl;
//      cout << "vertices[2] = " << vertices[2] << endl;
//      outputfunc::enter_continue_char();
      return NULL;
   }
   
   return generate_new_triangle(vertices,triangle_ID);
}

triangle* triangles_group::generate_new_triangle(
   const vector<vertex>& vertices,int triangle_ID)
{
//   cout << "inside triangles_group::generate_new_triangle() #2" << endl;

   if (triangle_ID==-1) triangle_ID=triangles_map_ptr->size();

   triangle* triangle_ptr=new triangle(vertices,triangle_ID);
   return triangle_ptr;
}

// ==========================================================================
// Delaunay triangulation member functions
// ==========================================================================

void triangles_group::update_triangle_vertices(const vertex& curr_Vertex)
{
//   cout << "inside triangles_group::update_triangle_vertices()" << endl;
//   cout << "curr_Vertex = " << curr_Vertex << endl;
   vertices_handler_ptr->update_vertex_in_table_chain_and_map(curr_Vertex);
}

// ---------------------------------------------------------------------
void triangles_group::delaunay_triangulate_vertices()
{
//   cout << "inside triangles_group::delaunay_triangulate_vertices()"
//        << endl;
   
   if (delaunay_triangles_vertex_map_ptr==NULL)
   {
      delaunay_triangles_vertex_map_ptr=new DELAUNAY_TRIANGLES_VERTEX_MAP;
   }

   Delaunay::Point tempP;
   vector< Delaunay::Point > v;

   for (unsigned int n=0; n<vertices_handler_ptr->get_n_vertices(); n++)
   {
      if (!vertices_handler_ptr->vertex_in_table(n)) continue;

      vertex curr_vertex(vertices_handler_ptr->get_vertex(n));
      tempP[0]=curr_vertex.get_posn().get(0);
      tempP[1]=curr_vertex.get_posn().get(1);
      v.push_back(tempP);
   }

   Delaunay delobject(v);
   delobject.Triangulate();

   int counter=0;
   for (Delaunay::fIterator fit  = delobject.fbegin(); 
        fit != delobject.fend();  ++fit)
   {
//      cout << delobject.Org(fit)  << ", " 
//           << delobject.Dest(fit) << ", " 
//           << delobject.Apex(fit) << " \t: Area = " 
//           << delobject.area(fit) << endl;

      if (counter%1000==0) cout << counter/1000 << " " << flush;
      counter++;

      int origin_ID=delobject.Org(fit);
      int destination_ID=delobject.Dest(fit);
      int apex_ID=delobject.Apex(fit);

      if (!vertices_handler_ptr->vertex_in_table(origin_ID)) continue;
      if (!vertices_handler_ptr->vertex_in_table(destination_ID)) continue;
      if (!vertices_handler_ptr->vertex_in_table(apex_ID)) continue;

      threevector origin_posn=
         vertices_handler_ptr->get_vertex(origin_ID).get_posn();
      threevector destination_posn=
         vertices_handler_ptr->get_vertex(destination_ID).get_posn();
      threevector apex_posn=
         vertices_handler_ptr->get_vertex(apex_ID).get_posn();

//      cout << "origin = " << origin_posn << endl;
//      cout << "destination = " << destination_posn << endl;
//      cout << "apex = " << apex_posn << endl;
      
      triangle* curr_triangle_ptr=generate_new_triangle(
         origin_posn,destination_posn,apex_posn);
      if (curr_triangle_ptr==NULL) continue;

//      cout << "*curr_triangle_ptr = " << *curr_triangle_ptr << endl;
//      double triangle_area=
	curr_triangle_ptr->compute_area();
//      cout << "triangle_area = " << triangle_area << endl;

// Add new triangle to *triangles_map_ptr:

      int triangle_ID=triangles_map_ptr->size();
      (*triangles_map_ptr)[triangle_ID]=curr_triangle_ptr;

// Add Delaunay triangle vertices to *delaunay_triangle_vertex_map_ptr:

      DELAUNAY_TRIANGLES_VERTEX_MAP::iterator vertex_iter=
         delaunay_triangles_vertex_map_ptr->find(origin_ID);
      if (vertex_iter==delaunay_triangles_vertex_map_ptr->end()) 
      {
         (*delaunay_triangles_vertex_map_ptr)[origin_ID]=origin_posn;
      }

      vertex_iter=delaunay_triangles_vertex_map_ptr->find(origin_ID);
      if (vertex_iter==delaunay_triangles_vertex_map_ptr->end()) 
      {
         (*delaunay_triangles_vertex_map_ptr)[origin_ID]=origin_posn;
      }

   } // loop over Delaunay triangle faces

// Generate neighboring vertices network:

   if (vertices_network_map_ptr==NULL)
   {
      vertices_network_map_ptr=new VERTICES_NETWORK_MAP;
   }

   for (int curr_vertex_ID=0; curr_vertex_ID < delobject.nvertices(); 
        curr_vertex_ID++)
   { 
      vector<int>* neighboring_vertex_IDs_ptr=NULL;

      VERTICES_NETWORK_MAP::iterator vertex_iter=
         vertices_network_map_ptr->find(curr_vertex_ID);
      if (vertex_iter==vertices_network_map_ptr->end()) 
      {
         neighboring_vertex_IDs_ptr=new vector<int>;
         (*vertices_network_map_ptr)[curr_vertex_ID]=
            neighboring_vertex_IDs_ptr;
      }
      else
      {
         neighboring_vertex_IDs_ptr=vertex_iter->second;
      }

      vector<int> vav;
      delobject.trianglesAroundVertex(curr_vertex_ID,vav);
      for (unsigned int i=0; i<vav.size(); i++)
      {
         if (vav[i]==curr_vertex_ID) continue;

// Perform brute force search for vav[i] within neighboring vertex IDs:

         bool neighbor_vertex_already_inside_flag=false;
         for (unsigned int j=0; j<neighboring_vertex_IDs_ptr->size(); j++)
         {
            if (vav[i]==neighboring_vertex_IDs_ptr->at(j))
            {
               neighbor_vertex_already_inside_flag=true;
               break;
            }
         } // loop over index j labeling neighboring vertex IDs
         if (!neighbor_vertex_already_inside_flag)
         {
            neighboring_vertex_IDs_ptr->push_back(vav[i]);
         }
      } // loop over index i labeling current Delaunay triangle vertices

      std::sort(
         neighboring_vertex_IDs_ptr->begin(),
         neighboring_vertex_IDs_ptr->end());
      
//      cout << "vertex ID = " << curr_vertex_ID << endl;
//      cout << "Neighboring vertex IDs:" << endl;
//      for (unsigned int i=0; i<neighboring_vertex_IDs_ptr->size(); i++)
//      {
//         cout << neighboring_vertex_IDs_ptr->at(i) << " ";
//      }
//      cout << endl;

   } // loop over curr vertex IDs
   cout << endl;
}

// ---------------------------------------------------------------------
const vector<int>* triangles_group::get_neighboring_vertex_IDs(
   int curr_vertex_ID) const
{
   VERTICES_NETWORK_MAP::iterator vertex_iter=
      vertices_network_map_ptr->find(curr_vertex_ID);
   return vertex_iter->second;
}

// ==========================================================================
// Interior triangulation member functions
// ==========================================================================

// Member function inner_triangulate_vertices() calls
// static functions within John Ratcliff's triangulater class upon the
// input set of vertices assumed to already be stored within
// *vertices_handler_ptr.  The vertices must lie within an XY plane
// whose Z value is passed as an input.  But they can define a
// complicated contour containing zero
// holes. Inner_triangulate_vertices() generates a triangulation of the
// input contour vertices where all triangles are guaranteed to live
// inside the contour.

void triangles_group::inner_triangulate_vertices(double specified_Z)
{
//   cout << "inside triangles_group::inner_triangulate_vertices()" << endl;

   twovector V2;
   Triangulater::TwovectorVector input_V2s,output_V2s;
   for (unsigned int n=0; n<vertices_handler_ptr->get_n_vertices(); n++)
   {
      vertex curr_vertex(vertices_handler_ptr->get_vertex(n));
//      cout << "n = " << n << " curr_vertex = " << curr_vertex << endl;

      V2.put(0,curr_vertex.get_posn().get(0));
      V2.put(1,curr_vertex.get_posn().get(1));
      input_V2s.push_back(V2);
   } // loop over index n labeling input vertices
   
   Triangulater::Process(input_V2s,output_V2s);

   unsigned int n_triangles = output_V2s.size()/3;
   for (unsigned int t=0; t<n_triangles; t++)
   {
      const twovector& p1 = output_V2s[t*3+0];
      const twovector& p2 = output_V2s[t*3+1];
      const twovector& p3 = output_V2s[t*3+2];

      threevector origin_posn=p1;
      threevector destination_posn=p2;
      threevector apex_posn=p3;

      origin_posn.put(2,specified_Z);
      destination_posn.put(2,specified_Z);
      apex_posn.put(2,specified_Z);

      triangle* curr_triangle_ptr=generate_new_triangle(
         origin_posn,destination_posn,apex_posn);
//      cout << "t = " << t << " curr_triangle = " << *curr_triangle_ptr
//           << endl;
      if (curr_triangle_ptr==NULL) continue;
      
      curr_triangle_ptr->compute_area();

// Add new triangle to *triangles_map_ptr:

      int triangle_ID=triangles_map_ptr->size();
      (*triangles_map_ptr)[triangle_ID]=curr_triangle_ptr;

// Need to add triangle vertices into neighboring vertices network !!!

/*
// Generate neighboring vertices network:

   if (vertices_network_map_ptr==NULL)
   {
      vertices_network_map_ptr=new VERTICES_NETWORK_MAP;
   }
*/

   } // loop over index t labeling inner triangles
}

// ---------------------------------------------------------------------
// Member function reset_all_Z_values() resets all triangle Z values
// to the specified input value.

void triangles_group::reset_all_Z_values(double z)
{
   for (unsigned int t=0; t<get_n_triangles(); t++)
   {
      triangle* triangle_ptr=get_triangle_ptr(t);
      triangle_ptr->reset_all_Z_values(z);
   }
}

// ==========================================================================
// Collective triangle properties member functions
// ==========================================================================

// Member function compute_area_integral() returns the sum of all
// triangle areas.

double triangles_group::compute_area_integral()
{
   double area_integral=0;
   for (unsigned int n=0; n<get_n_triangles(); n++)
   {
      area_integral += get_triangle_ptr(n)->get_area();
   }
   return area_integral;
}

// ---------------------------------------------------------------------
// Member function triangle_edge_lengths() iterates over all vertices
// within *vertices_network_map_ptr.  It computes the distance between
// each vertex in the network with its nearest neighbors.  The
// distances are returned within an STL vector.  Note that the
// returned STL vector should contain each triangle edge length twice!

vector<double> triangles_group::triangle_edge_lengths()
{
//   cout << "inside triangles_group::triangle_edge_lengths()" << endl;
   
   vector<double> triangle_edge_lengths;
   for (VERTICES_NETWORK_MAP::iterator vertex_iter=
           vertices_network_map_ptr->begin();
        vertex_iter != vertices_network_map_ptr->end(); vertex_iter++)
   {
      vertex* curr_vertex_ptr=get_vertex_ptr(vertex_iter->first);
      if (curr_vertex_ptr==NULL) continue;

      vector<int>* neighbor_vertex_IDs_ptr=vertex_iter->second;
      for (unsigned int n=0; n<neighbor_vertex_IDs_ptr->size(); n++)
      {
         vertex* neighbor_vertex_ptr=get_vertex_ptr(
            neighbor_vertex_IDs_ptr->at(n));
         if (neighbor_vertex_ptr==NULL) continue;

         triangle_edge_lengths.push_back(
            (curr_vertex_ptr->get_posn()-
             neighbor_vertex_ptr->get_posn()).magnitude());
      }
   } // loop over vertex_iter

   return triangle_edge_lengths;
}
