// =========================================================================
// union_find class member function definitions
// =========================================================================
// Last modified on 11/18/13; 4/8/14; 2/16/16; 2/17/16
// =========================================================================

#include <iostream>
#include "templates/mytemplates.h"
#include "datastructures/union_find.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void union_find::allocate_member_objects()
{
   nodes_map_ptr=new NODES_MAP;
   parent_nodes_map_ptr=new PARENT_NODES_MAP;
}

void union_find::initialize_member_objects()
{
}		 

// ---------------------------------------------------------------------
union_find::union_find()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

union_find::union_find(const union_find& uf)
{
   docopy(uf);
}

union_find::~union_find()
{
//   cout << "inside union_find destructor" << endl;

   delete nodes_map_ptr;
   delete parent_nodes_map_ptr;
}

// ---------------------------------------------------------------------
void union_find::docopy(const union_find& uf)
{
}

// Overload = operator:

union_find& union_find::operator= (const union_find& uf)
{
   if (this==&uf) return *this;
   docopy(uf);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,union_find& uf)
{
   outstream << "n_nodes = " << uf.get_n_nodes() << endl;

   vector<int> node_IDs,parent_IDs;
   uf.get_all_nodes(node_IDs,parent_IDs);

   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
      outstream << "node_ID = " << node_IDs[n]
                << " parent_ID = " << parent_IDs[n] 
                << " root_ID = " << uf.Find(node_IDs[n])
                << endl;
   }

   uf.fill_parent_nodes_map();
   for(uf.parent_nodes_map_iter = uf.parent_nodes_map_ptr->begin();
       uf.parent_nodes_map_iter != uf.parent_nodes_map_ptr->end();
       uf.parent_nodes_map_iter++)
   {
      int parent_ID = uf.parent_nodes_map_iter->first;
      vector<int> children_node_IDs = 
         uf.get_children_nodes_corresponding_to_parent(parent_ID);
      cout << "Parent node ID = " << parent_ID << endl;
      cout << children_node_IDs.size() << " children IDs:" << endl;
      for(unsigned int c = 0; c < children_node_IDs.size(); c++)
      {
         cout << children_node_IDs[c] << "  ";
      }
      cout << endl;
   }
   
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void union_find::set_parent_ID(int node_ID,int parent_ID)
{
   nodes_map_iter=nodes_map_ptr->find(node_ID);
   if (nodes_map_iter != nodes_map_ptr->end())
   {
      nodes_map_iter->second.first=parent_ID;
   }
}

// --------------------------------------------------------------------------
int union_find::get_parent_ID(int node_ID)
{
   nodes_map_iter=nodes_map_ptr->find(node_ID);
   if (nodes_map_iter==nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return nodes_map_iter->second.first;
   }
}

// --------------------------------------------------------------------------
void union_find::set_rank(int node_ID,int rank)
{
   nodes_map_iter=nodes_map_ptr->find(node_ID);
   if (nodes_map_iter != nodes_map_ptr->end())
   {
      nodes_map_iter->second.second=rank;
   }
}

// --------------------------------------------------------------------------
int union_find::get_rank(int node_ID)
{
   nodes_map_iter=nodes_map_ptr->find(node_ID);
   if (nodes_map_iter==nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return nodes_map_iter->second.second;
   }
}

// ==========================================================================
// Union-find algorithm member functions
// ==========================================================================

void union_find::purgeNodes()
{
   nodes_map_ptr->clear();
}

// --------------------------------------------------------------------------
void union_find::MakeSet(int node_ID)
{
   CreateNode(node_ID,node_ID);
}

// --------------------------------------------------------------------------
void union_find::CreateNode(int node_ID,int parent_node_ID)
{
   (*nodes_map_ptr)[node_ID].first=parent_node_ID;	// parent ID
   (*nodes_map_ptr)[node_ID].second=0;			// rank
}

// --------------------------------------------------------------------------
void union_find::DestroyNode(int node_ID)
{
   NODES_MAP::iterator iter=nodes_map_ptr->find(node_ID);
   if (iter != nodes_map_ptr->end())
   {
      nodes_map_ptr->erase(iter);
   }
}

// --------------------------------------------------------------------------
int union_find::Find(int node_ID)
{
//   cout << "inside union_find::find(), node_ID = " << node_ID << endl;
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
// Member function Link() merges the two sets to which node1_ID and node2_ID
// belong.  It returns the ID of the root node for the merged set.

int union_find::Link(int node1_ID,int node2_ID)
{
//   cout << "inside compute_union(), node1_ID = " << node1_ID 
//        << " node2_ID = " << node2_ID << endl;
   int root1_ID,root2_ID;
   return Link(node1_ID,node2_ID,root1_ID,root2_ID);
}

// --------------------------------------------------------------------------
int union_find::Link(
   int node1_ID,int node2_ID,int& root1_ID,int& root2_ID)
{
//   cout << "inside compute_union(), node1_ID = " << node1_ID 
//        << " node2_ID = " << node2_ID << endl;

   root1_ID=Find(node1_ID);
   root2_ID=Find(node2_ID);
   if (root1_ID==root2_ID) return root1_ID;

// Nodes 1 and 2 are not already in the same set.  Merge them:

   int root1_rank=get_rank(root1_ID);
   int root2_rank=get_rank(root2_ID);

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

// ==========================================================================
// Node interation member functions
// ==========================================================================

bool union_find::node_in_map(int node_ID)
{
   nodes_map_iter=nodes_map_ptr->find(node_ID);
   if (nodes_map_iter==nodes_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

// --------------------------------------------------------------------------
int union_find::reset_curr_node_iterator()
{
   curr_node_iter=nodes_map_ptr->begin();
   if (curr_node_iter==nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return curr_node_iter->first;
   }
}

// --------------------------------------------------------------------------
int union_find::increment_curr_node_iterator()
{
   curr_node_iter++;
   if (curr_node_iter==nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return curr_node_iter->first;
   }
}

// --------------------------------------------------------------------------
int union_find::reset_next_node_iterator()
{
   next_node_iter=curr_node_iter;
   next_node_iter++;
   if (next_node_iter==nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return next_node_iter->first;
   }
}

// --------------------------------------------------------------------------
int union_find::increment_next_node_iterator()
{
   next_node_iter++;
   if (next_node_iter==nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return next_node_iter->first;
   }
}

// =========================================================================
// Node and parent retrieval member functions
// =========================================================================

// Member function get_all_nodes() iterates over all nodes within
// *nodes_map_ptr.  It returns the IDs for all nodes within STL vector
// node_IDs and the IDs for the nodes parents within STL vector
// parent_IDs.  

void union_find::get_all_nodes(vector<int>& node_IDs,vector<int>& parent_IDs)
{
   int node_ID=reset_curr_node_iterator();
   while (node_ID >= 0)
   {
      int parent_ID=get_parent_ID(node_ID);
      node_IDs.push_back(node_ID);
      parent_IDs.push_back(parent_ID);

      node_ID=increment_curr_node_iterator();
   }

   templatefunc::Quicksort(node_IDs,parent_IDs);
}

// --------------------------------------------------------------------------
// Member function fill_parent_nodes_map() iterates over all nodes in
// *nodes_map_ptr.  It fills *parent_nodes_map_ptr with unique
// "children nodes" corresponding to parent node IDs.

int union_find::fill_parent_nodes_map()
{
   int node_ID=reset_curr_node_iterator();
   while (node_ID >= 0)
   {
      int parent_ID=get_parent_ID(node_ID);

// Check whether node_ID already exists within "children nodes" map
// parent_nodes_map_iter->second.  If not, add node_ID to the
// "children nodes" map:

      parent_nodes_map_iter=parent_nodes_map_ptr->find(parent_ID);
      if (parent_nodes_map_iter==parent_nodes_map_ptr->end())
      {
         map<int,bool> children_map;
         children_map[node_ID]=true;
         (*parent_nodes_map_ptr)[parent_ID]=children_map;
      }
      else
      {
         map<int,bool>::iterator children_nodes_map_iter=
            parent_nodes_map_iter->second.find(node_ID);
         if (children_nodes_map_iter==parent_nodes_map_iter->second.end())
         {
            (parent_nodes_map_iter->second)[node_ID]=true;
         }
      }

      node_ID=increment_curr_node_iterator();
   } // while loop

   return parent_nodes_map_ptr->size();
}

// --------------------------------------------------------------------------
// Member function get_n_sibling_nodes() returns the number of nodes
// which share the same parent with the specified input node.

int union_find::get_n_sibling_nodes(int node_ID)
{
   return get_sibling_nodes(node_ID).size();
}

// --------------------------------------------------------------------------
vector<int> union_find::get_sibling_nodes(int node_ID)
{
   int parent_ID = get_parent_ID(node_ID);
   vector<int> sibling_node_IDs;

   int curr_node_ID=reset_curr_node_iterator();
   while (curr_node_ID >= 0)
   {
      if(get_parent_ID(curr_node_ID) == parent_ID)
      {
         sibling_node_IDs.push_back(curr_node_ID);
      }
      curr_node_ID=increment_curr_node_iterator();
   } // while loop

   return sibling_node_IDs;
}

// --------------------------------------------------------------------------
// Member function get_children_nodes_corresponding_to_parent()
// iterates over all nodes with the specified input parent ID.  It
// returns the childrens' node IDs in an STL vector.

vector<int> union_find::get_children_nodes_corresponding_to_parent(
   int parent_ID)
{
   vector<int> children_node_IDs;

   parent_nodes_map_iter=parent_nodes_map_ptr->find(parent_ID);
   if (parent_nodes_map_iter != parent_nodes_map_ptr->end())
   {
      map<int,bool>::iterator children_nodes_map_iter;
      
      for (children_nodes_map_iter=parent_nodes_map_iter->second.begin();
           children_nodes_map_iter != parent_nodes_map_iter->second.end();
           children_nodes_map_iter++)
      {
         children_node_IDs.push_back(children_nodes_map_iter->first);
      }
   }

   return children_node_IDs;
}

// --------------------------------------------------------------------------
int union_find::reset_parent_node_iterator()
{
   parent_nodes_map_iter=parent_nodes_map_ptr->begin();
   if (parent_nodes_map_iter==parent_nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return parent_nodes_map_iter->first;
   }
}

// --------------------------------------------------------------------------
int union_find::increment_parent_node_iterator()
{
   parent_nodes_map_iter++;
   if (parent_nodes_map_iter==parent_nodes_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return parent_nodes_map_iter->first;
   }
}
