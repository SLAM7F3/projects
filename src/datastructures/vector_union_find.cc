// =========================================================================
// vector_union_find class member function definitions
// =========================================================================
// Last modified on 7/25/12; 7/28/12; 7/31/12
// =========================================================================

#include <iostream>
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "datastructures/vector_union_find.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void vector_union_find::allocate_member_objects()
{
   nodes_vector_ptr=new NODES_VECTOR;
   data_for_set_map_ptr=new DATA_FOR_SET_MAP;
}

void vector_union_find::initialize_member_objects()
{
}		 

// ---------------------------------------------------------------------
vector_union_find::vector_union_find()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

vector_union_find::vector_union_find(const vector_union_find& uf)
{
   docopy(uf);
}

vector_union_find::~vector_union_find()
{
//   cout << "inside vector_union_find destructor" << endl;

// As of 7/28/12, we assume the void* member of each NODE triple is a
// pointer to a dynamically instantiated object.  But multiple nodes
// generally share the same void* data pointers.  So in order to
// uniquely delete the dynamically instantiated data objects, we
// iterate over *data_for_set_map_ptr which should contain precisely
// one copy of each data object's pointer:

   for (data_for_set_map_iter=data_for_set_map_ptr->begin();
        data_for_set_map_iter != data_for_set_map_ptr->end();
        data_for_set_map_iter++)
   {
      void* data_ptr=data_for_set_map_iter->first;
      delete data_ptr;
   }

   delete nodes_vector_ptr;
}

// ---------------------------------------------------------------------
void vector_union_find::docopy(const vector_union_find& uf)
{
}

// Overload = operator:

vector_union_find& vector_union_find::operator= (const vector_union_find& uf)
{
   if (this==&uf) return *this;
   docopy(uf);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,vector_union_find& uf)
{
   outstream << "n_nodes = " << uf.get_n_nodes() << endl;

   for (unsigned int n=0; n<uf.get_n_nodes(); n++)
   {
      int node_ID=n;
      int parent_ID=uf.nodes_vector_ptr->at(n).first;
//      int rank=uf.nodes_vector_ptr->at(n).second;
      int root_ID=uf.Find(node_ID);
      
      outstream << "node_ID = " << node_ID
                << " parent_ID = " << parent_ID
                << " root_ID = " << root_ID
//                << " rank = " << rank
                << endl;
   }

   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void vector_union_find::set_data_ptr(int node_ID,void* data_ptr)
{
   nodes_vector_ptr->at(node_ID).third=data_ptr;
   data_for_set_map_iter=data_for_set_map_ptr->find(data_ptr);
   if (data_for_set_map_iter==data_for_set_map_ptr->end())
   {
      (*data_for_set_map_ptr)[data_ptr]=1;
   }
}

void vector_union_find::set_data_label(void* data_ptr,int data_label)
{
   data_for_set_map_iter=data_for_set_map_ptr->find(data_ptr);
   if (data_for_set_map_iter != data_for_set_map_ptr->end())
   {
      data_for_set_map_iter->second=data_label;
   }
}

void vector_union_find::set_data_label(int node_ID,int data_label)
{
   void* data_ptr=nodes_vector_ptr->at(node_ID).third;
   if (data_ptr != NULL)
   {
      set_data_label(data_ptr,data_label);
   }
}

void* vector_union_find::get_data_ptr(int node_ID)
{
   return nodes_vector_ptr->at(node_ID).third;
}

void* vector_union_find::get_root_data_ptr(int node_ID)
{
   int root_ID=Find(node_ID);
   if (root_ID < 0) 
   {
      return NULL;
   }
   else
   {
      return nodes_vector_ptr->at(root_ID).third;
   }
}

// ==========================================================================
// Union-find algorithm member functions
// ==========================================================================

void vector_union_find::initializeNodes(int n_nodes)
{
//   cout << "inside vector_union_find::initializeNodes(), n_nodes = "
//        << n_nodes << endl;
   nodes_vector_ptr->reserve(n_nodes);
   NODE empty_node(-1,-1,NULL);
   for (int i=0; i<n_nodes; i++)
   {
      nodes_vector_ptr->push_back(empty_node);
   }
}

// --------------------------------------------------------------------------
void vector_union_find::MakeSet(int node_ID)
{
   CreateNode(node_ID,node_ID);
}

// --------------------------------------------------------------------------
void vector_union_find::CreateNode(int node_ID,int parent_node_ID)
{
   set_parent_ID(node_ID,parent_node_ID);
   set_rank(node_ID,0);
   set_data_ptr(node_ID,NULL);
}

// --------------------------------------------------------------------------
int vector_union_find::Find(int node_ID)
{
//   cout << "inside vector_union_find::find(), node_ID = " << node_ID << endl;
   int parent_ID=get_parent_ID(node_ID);
   if (parent_ID==-1) return parent_ID;
   
   if (parent_ID != node_ID)
   {
      parent_ID=Find(parent_ID);
      set_parent_ID(node_ID,parent_ID);
   }
   return parent_ID;
}

// --------------------------------------------------------------------------
// Member function Link() returns the root_ID for the combined set.

int vector_union_find::Link(int node1_ID,int node2_ID)
{
   int root1_ID=Find(node1_ID);
   int root2_ID=Find(node2_ID);
   
   if (root1_ID==root2_ID) return root1_ID;

// Nodes 1 and 2 are not already in the same set.  Merge them:

   int root1_rank=get_rank(root1_ID);
//   cout << "root1_rank = " << root1_rank << endl;
   int root2_rank=get_rank(root2_ID);
//   cout << "root2_rank = " << root2_rank << endl;

   if (root1_rank < root2_rank)
   {
      set_parent_ID(root1_ID,root2_ID);
      return root2_ID;
   }
   else if (root1_rank > root2_rank)
   {
      set_parent_ID(root2_ID,root1_ID);
      return root1_ID;
   }
   else
   {
      set_parent_ID(root2_ID,root1_ID);
      set_rank(root1_ID,root1_rank+1);
      return root1_ID;
   }
}

// --------------------------------------------------------------------------
void vector_union_find::purgeNodes()
{
   nodes_vector_ptr->clear();
}

