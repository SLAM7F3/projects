// ==========================================================================
// Delaunay_Tree class member function definitions
// ==========================================================================
// Last modified on 12/15/05; 7/12/06
// ==========================================================================

#include "datastructures/containerfuncs.h"
#include "delaunay/Delaunay_point.h"
#include "delaunay/Delaunay_tree.h"
#include "delaunay/DT_node.h"
#include "datastructures/Triple.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void Delaunay_tree::allocate_member_objects()
{
   root = new DT_node();
   vertex_network_ptr=new Network<threevector*>(10*n_vertices);
}		       

void Delaunay_tree::initialize_member_objects()
{
   nb=0;
}

Delaunay_tree::Delaunay_tree(vector<threevector>* UV_ptr)
{
   vertices_ptr=UV_ptr;
   n_vertices=vertices_ptr->size();
   for (int n=0; n<n_vertices; n++)
   {
      vertex_ID.push_back(n);
   }

   allocate_member_objects();
   initialize_member_objects();

   new DT_node(root, 0);
   new DT_node(root, 1);
   new DT_node(root, 2);
   root->neighbors[0]->neighbors[1] =  root->neighbors[1];
   root->neighbors[0]->neighbors[2] =  root->neighbors[2];
   root->neighbors[1]->neighbors[0] =  root->neighbors[0];
   root->neighbors[1]->neighbors[2] =  root->neighbors[2];
   root->neighbors[2]->neighbors[0] =  root->neighbors[0];
   root->neighbors[2]->neighbors[1] =  root->neighbors[1];

   build_vertex_network();
}

Delaunay_tree::Delaunay_tree(const vector<int>& ID,vector<threevector>* UV_ptr)
{
   vertices_ptr=UV_ptr;
   n_vertices=vertices_ptr->size();
   vertex_ID=ID;

   allocate_member_objects();
   initialize_member_objects();

   new DT_node(root, 0);
   new DT_node(root, 1);
   new DT_node(root, 2);
   root->neighbors[0]->neighbors[1] =  root->neighbors[1];
   root->neighbors[0]->neighbors[2] =  root->neighbors[2];
   root->neighbors[1]->neighbors[0] =  root->neighbors[0];
   root->neighbors[1]->neighbors[2] =  root->neighbors[2];
   root->neighbors[2]->neighbors[0] =  root->neighbors[0];
   root->neighbors[2]->neighbors[1] =  root->neighbors[1];

   build_vertex_network();
}

Delaunay_tree::~Delaunay_tree()               
{
   nb++;
   delete vertex_network_ptr;
}

void Delaunay_tree::build_vertex_network()
{
   for (int i=0; i<n_vertices; i++)
   {
      threevector* curr_vertex_ptr=&( vertices_ptr->at(i) );
      *this += new Delaunay_point(vertex_ID[i],curr_vertex_ptr->get(0),
                                  curr_vertex_ptr->get(1));

// Insert curr_vertex_ptr into Network member *vertex_network_ptr:

      vertex_network_ptr->insert_site(
         vertex_ID[i],Site<threevector*>( curr_vertex_ptr ) );

   } // loop over index i labeling Delaunay vertices
}

// ---------------------------------------------------------------------
Delaunay_tree& Delaunay_tree::operator+=(Delaunay_point* p)      // insertion
{

   DT_node* n;
   DT_node* created;
   DT_node* last;
   DT_node* first;
   Delaunay_point* q;
   Delaunay_point* r;
   Index i;

   root->nb = ++nb;
   if ( ! ( n = root->find_conflict(p) ) ) return *this ;
   // test if p is already inserted
   for ( i=0; (int) i < 3- (int) n->flag.is_infinite(); i++ )
      if ( ( p->X()==n->vertices[i]->X() )&&( p->Y()==n->vertices[i]->Y() ) )
         return *this;
   n->flag.kill();
   // we will turn cw around first vertex of n, till next triangle
   // is not in conflict
   q = n->vertices[0];
   while( n->neighbors[ i=n->cw_neighbor_Index(q) ]->conflict(p) )
   {n = n->neighbors[i];n->flag.kill();}

   first = last = new DT_node(n,p,i);
   // we will turn cw around r, till next triangle is not in conflict
   r = n->vertices[(((int) i)+2)%3];
   while(true)
   {
      i = n->cw_neighbor_Index(r);
      if (n->neighbors[i]->flag.is_dead())
      {
         n = n->neighbors[i]; 
         continue;
      }
      if (n->neighbors[i]->conflict(p) )
      {
         n = n->neighbors[i];
         n->flag.kill();
         continue;
      }
      break;
   }

   while ( true ) 
   {
      // n is killed by p
      // n->neighbors[i] is not in conflict with p
      // r is vertex i+1 of n
      created = new DT_node(n,p,i);
      created->neighbors[2]=last;
      last->neighbors[1]=created;
      last=created;
      r = n->vertices[(((int) i)+2)%3];   // r turn in n ccw
      if (r==q) break;
      // we will turn cw around r, till next triangle is not in conflict
      while(true)
      {
         i = n->cw_neighbor_Index(r);
         if (n->neighbors[i]->flag.is_dead())
         {
            n = n->neighbors[i]; 
            continue;
         }
         if (n->neighbors[i]->conflict(p) )
         {
            n = n->neighbors[i];
            n->flag.kill();
            continue;
         }
         break;
      }
   }
   first->neighbors[2]=last;
   last->neighbors[1]=first;
   return *this;
}

// ---------------------------------------------------------------------
// Member function compute_triangles first recursively loops over
// every Delaunay vertex and finds each pair of neighboring vertices
// which form a triangle.  It then performs a sanity check on the area
// of each candidate Delaunay triangle and eliminates any which are
// degenerate.  This method returns an STL vector of non-degenerate
// triangle indices.

vector<Triple<int,int,int> > Delaunay_tree::compute_triangles()
{
   root->nb = ++nb;
   root->output(vertex_network_ptr);

   vector<Triple<int,int,int> > triangles=
      vertex_network_ptr->find_all_triangles();

// Next compute area of each candidate Delaunay triangle:

   vector<int> degenerate_triangle_indices;
   for (unsigned int i=0; i<triangles.size(); i++)
   {
      Triple<int,int,int> curr_triangle=triangles[i];
//      cout << "triangle = " << curr_triangle << endl;

      int index1=containerfunc::locate_vector_index(
         vertex_ID,curr_triangle.first);
      int index2=containerfunc::locate_vector_index(
         vertex_ID,curr_triangle.second);
      int index3=containerfunc::locate_vector_index(
         vertex_ID,curr_triangle.third);

      double area=triangle_area(
         (*vertices_ptr)[index1],(*vertices_ptr)[index2],
         (*vertices_ptr)[index3]);
//      cout << "area = " << area << endl;

      if (nearly_equal(fabs(area),0)) 
         degenerate_triangle_indices.push_back(i);
   }

   for (unsigned int j=0; j<degenerate_triangle_indices.size(); j++)
   {
      triangles.erase(triangles.begin()+degenerate_triangle_indices[j]);
   }
   
   return triangles;
}

