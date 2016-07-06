// =========================================================================
// map_unionfind class member function definitions
// =========================================================================
// Last modified on 7/11/13; 8/10/13; 11/13/13
// =========================================================================

#include <iostream>
#include "datastructures/map_unionfind.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void map_unionfind::allocate_member_objects()
{
   nodes_map_ptr=new NODES_MAP;
   data_for_set_map_ptr=new DATA_FOR_SET_MAP;
}

void map_unionfind::initialize_member_objects()
{
}		 

// ---------------------------------------------------------------------
map_unionfind::map_unionfind()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

map_unionfind::map_unionfind(const map_unionfind& uf)
{
   docopy(uf);
}

map_unionfind::~map_unionfind()
{
//   cout << "inside map_unionfind destructor" << endl;

// As of 7/11/13, we assume that OTHER classes are responsible for
// deleting dynamically instantiated objects to which a void* member
// of each NODE triple points...

/*
   for (data_for_set_map_iter=data_for_set_map_ptr->begin();
        data_for_set_map_iter != data_for_set_map_ptr->end();
        data_for_set_map_iter++)
   {
      void* data_ptr=data_for_set_map_iter->first;
      delete data_ptr;
      data_ptr=NULL;
   }
*/

   delete nodes_map_ptr;
}

// ---------------------------------------------------------------------
void map_unionfind::docopy(const map_unionfind& uf)
{
}

// Overload = operator:

map_unionfind& map_unionfind::operator= (const map_unionfind& uf)
{
   if (this==&uf) return *this;
   docopy(uf);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,map_unionfind& uf)
{
   outstream << "n_nodes = " << uf.get_n_nodes() << endl;

   map_unionfind::NODES_MAP* nodes_map_ptr=uf.get_nodes_map_ptr();
   for (map_unionfind::NODES_MAP::iterator node_iter=
           nodes_map_ptr->begin(); node_iter != nodes_map_ptr->end(); 
        node_iter++)
   {
      DUPLE node_ID=node_iter->first;
      DUPLE parent_ID=uf.get_parent_ID(node_ID);
//      int rank_ID=uf.get_rank(node_ID);
      DUPLE root_ID=uf.Find(node_ID);
      
      outstream << "node_ID = " << node_ID.first << "," << node_ID.second
                << " parent_ID = " << parent_ID.first << ","
                << parent_ID.second
                << " root_ID = " << root_ID.first << "," << root_ID.second
//                << " rank = " << rank
                << endl;
   }

   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

DUPLE map_unionfind::form_node_ID(int frame_ID,int feature_ID) const
{
//   DUPLE duple(frame_ID,feature_ID);
//   return duple;
   return DUPLE(frame_ID,feature_ID);
}

bool map_unionfind::set_data_ptr(DUPLE node_ID,void* data_ptr)
{
//   cout << "inside map_unionfind::set_data_ptr()" << endl;
//   cout << "node_ID = " << node_ID << " data_ptr = " << data_ptr << endl;
   node_iter=nodes_map_ptr->find(node_ID);
   if (node_iter==nodes_map_ptr->end()) 
   {
      cout << "Warning in map_unionfind::set_data_ptr()" << endl;
      cout << "node_ID = " << node_ID.first << "," << node_ID.second 
           << " not found in map_unionfind!" << endl;
      return false;
   }
   
   node_iter->second->third=data_ptr;
   data_for_set_map_iter=data_for_set_map_ptr->find(data_ptr);
   if (data_for_set_map_iter==data_for_set_map_ptr->end())
   {
      (*data_for_set_map_ptr)[data_ptr]=1;
   }

   return true;
}

void map_unionfind::set_data_label(void* data_ptr,int data_label)
{
   data_for_set_map_iter=data_for_set_map_ptr->find(data_ptr);
   if (data_for_set_map_iter != data_for_set_map_ptr->end())
   {
      data_for_set_map_iter->second=data_label;
   }
}

void map_unionfind::set_data_label(DUPLE node_ID,int data_label)
{
   node_iter=nodes_map_ptr->find(node_ID);
   void* data_ptr=node_iter->second->third;
   if (data_ptr != NULL)
   {
      set_data_label(data_ptr,data_label);
   }
}

void* map_unionfind::get_data_ptr(DUPLE node_ID)
{
   node_iter=nodes_map_ptr->find(node_ID);
   return node_iter->second->third;
}

void* map_unionfind::get_root_data_ptr(DUPLE node_ID)
{
   DUPLE root_ID=Find(node_ID);
   if (root_ID.first < 0) 
   {
      return NULL;
   }
   else
   {
      node_iter=nodes_map_ptr->find(root_ID);
      return node_iter->second->third;
   }
}

// ==========================================================================
// Union-find algorithm member functions
// ==========================================================================

map_unionfind::NODE* map_unionfind::MakeSet(DUPLE node_ID)
{
   return CreateNode(node_ID,node_ID);
}

// --------------------------------------------------------------------------
map_unionfind::NODE* map_unionfind::CreateNode(
   DUPLE node_ID,DUPLE parent_node_ID)
{
   NODE* node_ptr=NULL;

// Check whether a node with the specified node_ID exists within
// nodes_map_ptr.  If not, create it:
   
   node_iter=nodes_map_ptr->find(node_ID);   
   if (node_iter==nodes_map_ptr->end())
   {
      node_ptr=new NODE(parent_node_ID,0,NULL);
      (*nodes_map_ptr)[node_ID]=node_ptr;
   }
   else
   {
      node_ptr=node_iter->second;
   }
   node_ptr->first=parent_node_ID;
   node_ptr->second=0;
   node_ptr->third=NULL;

   return node_ptr;
}

// --------------------------------------------------------------------------
// Boolean member function ElementOf() returns true if input DUPLE
// node_ID exists within *this:

bool map_unionfind::ElementOf(DUPLE node_ID)
{
   DUPLE parent_ID=get_parent_ID(node_ID);
//   cout << "parent_ID = " << parent_ID.first << "," << parent_ID.second
//        << endl;
   if (parent_ID.first==-1) 
   {
      return false;
   }
   else
   {
      return true;
   }
}

// --------------------------------------------------------------------------
// Member function Find() returns the parent_ID=root_ID corresponding
// to input node_ID.

DUPLE map_unionfind::Find(DUPLE node_ID)
{
//   cout << "inside map_unionfind::find(), node_ID = " 
//        << node_ID.first << "," << node_ID.second << endl;
   DUPLE parent_ID=get_parent_ID(node_ID);
//   cout << "parent_ID = " << parent_ID.first << "," << parent_ID.second
//        << endl;
   if (parent_ID.first==-1) return parent_ID;
   
   if (parent_ID != node_ID)
   {
      parent_ID=Find(parent_ID);
      set_parent_ID(node_ID,parent_ID);
   }
   return parent_ID;
}

// --------------------------------------------------------------------------
// Member function Link() returns the root_ID for the combined set.

DUPLE map_unionfind::Link(DUPLE node1_ID,DUPLE node2_ID)
{
   DUPLE root1_ID=Find(node1_ID);
   DUPLE root2_ID=Find(node2_ID);

   if (root1_ID.first==root2_ID.first &&
       root1_ID.second==root2_ID.second) return root1_ID;

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
void map_unionfind::purgeNodes()
{
   nodes_map_ptr->clear();
}


// ==========================================================================
// Inlined methods:
// ==========================================================================

int map_unionfind::get_n_nodes() const
{
   return nodes_map_ptr->size();
}


map_unionfind::NODE* map_unionfind::get_node_ptr(DUPLE node_ID)
{
   node_iter=nodes_map_ptr->find(node_ID);
   if (node_iter==nodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return node_iter->second;
   }
}

void map_unionfind::set_parent_ID(DUPLE node_ID,DUPLE parent_ID)
{
   node_iter=nodes_map_ptr->find(node_ID);
   node_iter->second->first=parent_ID;
}

DUPLE map_unionfind::get_parent_ID(DUPLE node_ID)
{
//   cout << "inside map_unionfind::get_parent_ID(), node_ID = " 
//        << node_ID.first << "," << node_ID.second << endl;
   node_iter=nodes_map_ptr->find(node_ID);
   if (node_iter==nodes_map_ptr->end())
   {
      DUPLE parent_ID(-1,-1);
      return parent_ID;
   }
   else
   {
      return node_iter->second->first;
   }
}

void map_unionfind::set_rank(DUPLE node_ID,int rank)
{
   node_iter=nodes_map_ptr->find(node_ID);
   node_iter->second->second=rank;
}

int map_unionfind::get_rank(DUPLE node_ID)
{
   node_iter=nodes_map_ptr->find(node_ID);
   return node_iter->second->second;
}

