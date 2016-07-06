// =========================================================================
// Vertices_Handler class member function definitions
// =========================================================================
// Last modified on 4/27/09; 5/18/13; 10/22/13; 4/4/14
// =========================================================================

#include <iostream>
#include "geometry/vertices_handler.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void vertices_handler::initialize_member_objects()
{
   vertex_table_ptr=NULL;
   vertex_map_ptr=NULL;
}		 

void vertices_handler::allocate_member_objects()
{
   vertex_map_ptr=new VERTEX_MAP;
}

// ---------------------------------------------------------------------
vertices_handler::vertices_handler(bool edge_flag)
{
   initialize_member_objects();
   allocate_member_objects();

   if (!edge_flag)
   {
      vertex_table_ptr=new Hashtable<vertex>(10);
   }
}

// ---------------------------------------------------------------------
// Copy constructor:

vertices_handler::vertices_handler(const vertices_handler& VH)
{
//    cout << "inside copy constructor" << endl;
   initialize_member_objects();
   docopy(VH);
}

vertices_handler::~vertices_handler()
{
//   cout << "inside vertices_handler destructor, this = " << this << endl;
   delete vertex_table_ptr;
   delete vertex_map_ptr;
}

// ---------------------------------------------------------------------
void vertices_handler::docopy(const vertices_handler& VH)
{
//   cout << "inside vertices_handler::docopy()" << endl;
//   cout << "this = " << this << endl;

   vertex_IDs_chain.clear();
   for (unsigned int i=0; i<VH.get_n_vertices(); i++)
   {
      vertex_IDs_chain.push_back(VH.vertex_IDs_chain[i]);
   }

   delete vertex_table_ptr;
   vertex_table_ptr=NULL;
   vertex_table.clear();

   if (VH.vertex_table_ptr != NULL)
   {
      vertex_table_ptr=new Hashtable<vertex>(*VH.vertex_table_ptr);   
   }
   else
   {
      for (unsigned int v=0; v<VH.vertex_table.size(); v++)
      {
         vertex_table.push_back(VH.vertex_table[v]);
      }
   }

   delete vertex_map_ptr;
   vertex_map_ptr=new VERTEX_MAP;

   for (VERTEX_MAP::iterator itr=VH.vertex_map_ptr->begin();
        itr != VH.vertex_map_ptr->end(); ++itr)
   {
      (*vertex_map_ptr)[itr->first]=itr->second;
   }
}

// Overload = operator:

vertices_handler& vertices_handler::operator= (const vertices_handler& VH)
{
   if (this==&VH) return *this;
   docopy(VH);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const vertices_handler& VH)
{
   outstream << endl;
   
   outstream << "---------- VERTICES_HANDLER VERTICES ----------" << endl;
   for (unsigned int v=0; v<VH.get_n_vertices(); v++)
   {
      vertex curr_V=VH.get_vertex_from_chain(v);
      outstream << "Vertex ID = " << curr_V.get_ID()
                << " : x = " << curr_V.get_posn().get(0)
                << " y = " << curr_V.get_posn().get(1)
                << " z = " << curr_V.get_posn().get(2) << endl;
   }
   outstream << endl;   

   return(outstream);
}

// =========================================================================
// Set & get member functions
// =========================================================================

// Boolean member function vertex_in_table() returns false if no
// vertex corresponding to the specified input ID actually exists
// within member STL vector vertex_table:

bool vertices_handler::vertex_in_table(int Vertex_ID)
{
//   cout << "inside vertices_handler::vertex_in_table(int Vertex_ID)" << endl;
//   cout << "this = " << this << endl;
//   cout << "Input Vertex_ID = " << Vertex_ID << endl;
//   cout << "vertex_table.size() = " << vertex_table.size() << endl;
//   cout << "vertex_table_ptr = " << vertex_table_ptr << endl;

   if (vertex_table_ptr==NULL)
   {
      for (unsigned int v=0; v<vertex_table.size(); v++)
      {
//         cout << "v = " << v << " vertex_table[v] = " << vertex_table[v]
//              << endl;
         
         if (vertex_table[v].get_ID()==Vertex_ID)
         {
            return true;
         }
      } // loop over index b
      return false;
   }
   else
   {
      Mynode<vertex>* currnode_ptr=vertex_table_ptr->retrieve_key(Vertex_ID);
      return (currnode_ptr != NULL);
   }
}

// ---------------------------------------------------------------------
vertex* vertices_handler::get_vertex_ptr(int Vertex_ID)
{
   if (vertex_in_table(Vertex_ID))
   {
      return &(get_vertex(Vertex_ID));
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
vertex& vertices_handler::get_vertex(int Vertex_ID)
{
//   cout << "inside vertices_handler::get_vertex(int Vertex_ID)" << endl;
//   cout << "this = " << this << endl;
//   cout << "Input Vertex_ID = " << Vertex_ID << endl;
//   cout << "vertex_table.size() = " << vertex_table.size() << endl;
//   cout << "vertex_table_ptr = " << vertex_table_ptr << endl;

   if (vertex_table_ptr==NULL)
   {
      for (unsigned int v=0; v<vertex_table.size(); v++)
      {
//         cout << "v = " << v << " vertex_table[v] = " << vertex_table[v]
//              << endl;
         
         if (vertex_table[v].get_ID()==Vertex_ID)
         {
            return vertex_table[v];
         }
      } // loop over index b
      cout << "Error in vertices_handler::get_vertex()" << endl;
      cout << "No vertex corresponding to ID = " << Vertex_ID 
           << " found!" << endl;
      exit(-1);
   }
   else
   {
      Mynode<vertex>* currnode_ptr=vertex_table_ptr->retrieve_key(Vertex_ID);
      return currnode_ptr->get_data();
   }
}

// ---------------------------------------------------------------------
const vertex& vertices_handler::get_vertex(int Vertex_ID) const
{
//   cout << "inside const vertices_handler::get_vertex(int Vertex_ID) const" << endl;
//   cout << "this = " << this << endl;

//   cout << "Vertex_ID = " << Vertex_ID << endl;
//   cout << "vertex_table.size() = " << vertex_table.size() << endl;
//   cout << "vertex_table_ptr = " << vertex_table_ptr << endl;

   if (vertex_table_ptr==NULL)
   {
      for (unsigned int v=0; v<vertex_table.size(); v++)
      {
//         cout << "v = " << v << " vertex_table[v] = " << vertex_table[v]
//              << endl;
         
         if (vertex_table[v].get_ID()==Vertex_ID)
         {
            return vertex_table[v];
         }
      } // loop over index b
      cout << "Error in vertices_handler::get_vertex()" << endl;
      cout << "No vertex corresponding to ID = " << Vertex_ID 
           << " found!" << endl;
      exit(-1);
   }
   else
   {
      Mynode<vertex>* currnode_ptr=vertex_table_ptr->retrieve_key(Vertex_ID);
      return currnode_ptr->get_data();
   }
}

// =========================================================================
// Vertex chain member functions
// =========================================================================

void vertices_handler::clear_vertex_chain()
{
//   cout << "inside vertices_handler::clear_vertex_chain()" << endl;
   vertex_IDs_chain.clear();
}

// ---------------------------------------------------------------------
// Member function update_vertex_ID_in_chain takes in vertex V and
// searches for its ID within STL vector member vertex_IDs_chain.  If
// it does not exist, this method pushes back the ID onto
// vertex_IDs_chain.

void vertices_handler::update_vertex_ID_in_chain(const vertex& V)
{
//   cout << "inside vertices_handler::update_vertex_ID_in_chain()" << endl;
//   cout << "this = " << this << endl;
//   cout << "Vertex V = " << V << endl;
   int V_ID=V.get_ID();

   for (unsigned int v=0; v<get_n_vertices(); v++)
   {
      if (vertex_IDs_chain[v]==V_ID)
      {
//         cout << "Error in vertices_handler::update_vertex_ID_in_chain()"
//              << endl;
//         cout << "v = " << v << " vertex_IDs_chain[v] = "
//              << vertex_IDs_chain[v] 
//              << " V_ID = " << V_ID << endl;
         return;
      }
   }
   
   vertex_IDs_chain.push_back(V_ID);
}

void vertices_handler::set_chain_vertex(int i,const vertex& V)
{
//   cout << "inside vertices_handler::set_chain_vertex, i = " << i
//        << " V = " << V << endl;
   vertex_IDs_chain[i]=V.get_ID();
}

vertex& vertices_handler::get_vertex_from_chain(int i)
{
   return get_vertex(vertex_IDs_chain[i]);
}

const vertex& vertices_handler::get_vertex_from_chain(int i) const
{
   return get_vertex(vertex_IDs_chain[i]);
}

// =========================================================================
// Vertex table member functions
// =========================================================================

void vertices_handler::purge_vertex_table()
{
   vertex_table.clear();
   if (vertex_table_ptr != NULL) vertex_table_ptr->purge_all_entries();
}

void vertices_handler::update_vertex_table_key(const vertex& V)
{
//   cout << "inside vertices_handler::update_vertex_table_key(), V = "
//        << V << endl;
   
   if (vertex_table_ptr==NULL)
   {
      for (unsigned int v=0; v<vertex_table.size(); v++)
      {
         if (vertex_table[v].get_ID()==V.get_ID())
         {
            vertex_table[v]=V;
            return;
         }
      }
      vertex_table.push_back(V);
   }
   else
   {
      vertex_table_ptr->update_key(V.get_ID(),V);
   }
}

// =========================================================================
// Vertex map member functions
// =========================================================================

void vertices_handler::clear_vertex_map()
{
   vertex_map_ptr->clear();
}

// ---------------------------------------------------------------------
// Member function get_vertex_ptr takes in threevector posn and
// searches member STL map *vertex_map_ptr for a corresponding vertex
// ID.  If none is found, this method returns NULL.  Otherwise, it
// returns a pointer to the located vertex.

vertex* vertices_handler::get_vertex_ptr(const threevector& posn)
{
//   cout << "inside vertices_handler::get_vertex_ptr()" << endl;
   
   VERTEX_MAP::iterator vertex_iter=vertex_map_ptr->find(posn);
   if (vertex_iter==vertex_map_ptr->end()) 
   {
//      cout << "Vertex posn not found inside *vertex_map_ptr" << endl;
      return NULL;
   }
   else
   {
//      cout << "Vertex found in map !!!" << endl;
   }
   
   int vertex_ID=vertex_iter->second;
//   cout << "Retrieved vertex ID = " << vertex_ID << endl;
//   cout << "vertex_in_table(vertex_ID) = " << vertex_in_table(vertex_ID)
//        << endl;

   if (!vertex_in_table(vertex_ID))
   {
      return NULL;
   }
   else
   {
      return &(get_vertex(vertex_ID));
   }
}

// ---------------------------------------------------------------------
void vertices_handler::update_vertex_map(const vertex& V)
{
//   cout << "inside vertices_handler::update_vertex_map(V)" << endl;
//   cout << "this = " << this << endl;
//   cout << "Vertex V = " << V << endl;

   vertex* curr_vertex_ptr=get_vertex_ptr(V.get_posn());
//   cout << "curr_vertex_ptr = " << curr_vertex_ptr << endl;
   if (curr_vertex_ptr==NULL)
   {
      (*vertex_map_ptr)[V.get_posn()]=V.get_ID();
   }
   else
   {
//      cout << "V.get_ID() = " << V.get_ID() << endl;
      curr_vertex_ptr->set_ID(V.get_ID());
   }
   curr_vertex_ptr=get_vertex_ptr(V.get_posn());
//   cout << "vertex_map_ptr->size() = " << vertex_map_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function reset_vertex_posn takes input threevector old_posn
// and searches member STL map *vertex_map_ptr for a corresponding
// vertex ID.  If the ID is found, this method deletes the old vertex
// entry within *vertex_map_ptr and instantiates a new entry
// containing the vertex's new position.  It also updates the vertex's
// information within members vertex_IDs_chain and
// vertex_table/*vertex_table_ptr.  This boolean method returns false
// if it does not successfully reset the vertex posn.

// On 5/18/13, we discovered the hard and painful way that passing
// input parameter old_posn as a const reference into this method
// incorrectly ended up changing its value!  So we now pass old_posn
// by value rather than by reference into this method.

bool vertices_handler::reset_vertex_posn(
   threevector old_posn, const threevector& new_posn)
{
//   cout << "inside vertices_handler::reset_vertex_posn()" << endl;
   
   VERTEX_MAP::iterator vertex_iter=vertex_map_ptr->find(old_posn);
   if (vertex_iter==vertex_map_ptr->end()) return false;
   
   int vertex_ID=vertex_iter->second;
   vertex_map_ptr->erase(vertex_iter);

   vertex new_V(new_posn,vertex_ID);
//   cout << "new_V = " << new_V << endl;

   (*vertex_map_ptr)[new_posn]=vertex_ID;

   update_vertex_in_table_chain_and_map(new_V);

   return true;
}

// ---------------------------------------------------------------------
// Member function print_vertex_map_contents

void vertices_handler::print_vertex_map_contents()
{
   cout << "inside vertices_handler::print_vertex_map_contents()" << endl;
   cout << "vertex_map_ptr->size() = " << vertex_map_ptr->size() << endl;

   int vertex_map_counter=0;
   for (VERTEX_MAP::iterator itr=vertex_map_ptr->begin();
        itr != vertex_map_ptr->end(); ++itr)
   {
      cout << "vertex map counter = " << vertex_map_counter++ << endl;
      cout << "posn = " << itr->first
           << " ID = " << itr->second << endl;
   }
}

// =========================================================================
// Vertex container member functions
// =========================================================================

// Member function clear_all_containers

void vertices_handler::clear_all_containers()
{
   clear_vertex_chain();
   purge_vertex_table();
   clear_vertex_map();
}

// ---------------------------------------------------------------------
void vertices_handler::update_vertex_in_table_chain_and_map(const vertex& V)
{
//   cout << "inside vertices_handler::update_vertex_in_table_chain_and_map()"
//        << endl;

   update_vertex_table_key(V);
   update_vertex_ID_in_chain(V);
   update_vertex_map(V);
}
