// ==========================================================================
// Header file for union_find class
// ==========================================================================
// Last modified on 11/18/13; 4/8/14; 2/16/16; 2/17/16
// ==========================================================================

#ifndef UNIONFIND_H
#define UNIONFIND_H

#include <map>
#include <set>
#include <vector>

class union_find
{

  public:

   typedef std::map<int,std::pair<int,int> > NODES_MAP;

// independent int var: node_ID
// dependent int var 1: parent_ID
// dependent int var 2: rank

   typedef std::map<int,std::map<int,bool> > PARENT_NODES_MAP;
   
// independent int = parent_ID
// dependent STL map holds node_IDs vs dummy booleans

   union_find();
   union_find(const union_find& uf);
   ~union_find();

   union_find& operator= (const union_find& uf);
   friend std::ostream& operator<< 
      (std::ostream& outstream,union_find& uf);

// Set and get member functions:

   unsigned int get_n_nodes() const;
   void set_parent_ID(int node_ID,int parent_ID);
   int get_parent_ID(int node_ID);
   void set_rank(int node_ID,int r);
   int get_rank(int node_ID);

// Union-find algorithm member functions:

   void purgeNodes();
   void MakeSet(int node_ID);
   void CreateNode(int node_ID,int parent_node_ID);
   void DestroyNode(int node_ID);
   int Find(int node_ID);
   int Link(int node1_ID,int node2_ID);
   int Link(int node1_ID,int node2_ID,int& root1_ID,int& root2_ID);

// Node iteration member functions:

   bool node_in_map(int node_ID);
   int reset_curr_node_iterator();
   int increment_curr_node_iterator();
   int reset_next_node_iterator();
   int increment_next_node_iterator();

// Node and parent retrieval member functions:

   void get_all_nodes(std::vector<int>& node_IDs,std::vector<int>& parent_IDs);
   int fill_parent_nodes_map();
   int get_n_sibling_nodes(int node_ID);
   std::vector<int> get_sibling_nodes(int node_ID);
   std::vector<int> get_children_nodes_corresponding_to_parent(int parent_ID);
   int reset_parent_node_iterator();
   int increment_parent_node_iterator();

  private: 

   NODES_MAP* nodes_map_ptr;
   NODES_MAP::iterator nodes_map_iter;
   NODES_MAP::iterator curr_node_iter,next_node_iter;

   PARENT_NODES_MAP* parent_nodes_map_ptr;
   PARENT_NODES_MAP::iterator parent_nodes_map_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const union_find& uf);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int union_find::get_n_nodes() const
{
   return nodes_map_ptr->size();
}


#endif  // union_find.h
