// ==========================================================================
// Header file for templatized Forest class
// ==========================================================================
// Last modified on 7/16/12; 7/17/12; 7/18/12
// ==========================================================================

#ifndef T_FOREST_H
#define T_FOREST_H

#include <iostream>
#include "datastructures/tree.h"

template <class T>
class Forest
{

  public:

   typedef std::map<int,treenode<T>* > TREENODES_MAP;

// Independent var: treenode integer ID
// Dependent var: pointer to treenode

   typedef std::map<int,tree<T>* > TREES_MAP;

// Independent var: Tree integer ID
// Dependent var: pointer to Tree

// Initialization, constructor and destructor functions:

   Forest();
   Forest(const Forest<T>& F);
   Forest<T>& operator= (const Forest<T>& F);
   virtual ~Forest();
   void purge_trees();
//   void purge_treenodes();

   template <class T1>
      friend std::ostream& operator<< (
         std::ostream& outstream,Forest<T1>& F);

// Set & get member functions:

   int get_n_treenodes() const;
   int get_n_trees() const;

// Treenode retrieval member functions:

   treenode<T>* get_treenode_ptr(int ID);
   treenode<T>* reset_treenodes_map_ptr();
   treenode<T>* get_next_treenode_ptr();

   int recursively_count_nodes();

// Tree generation,retrieval and deletion member functions:

   tree<T>* generate_new_tree(treenode<T>* treenode_ptr);
   void delete_tree_ptr(treenode<T>* treenode_ptr);
   void delete_tree_ptr(int tree_ID);
   tree<T>* get_tree_ptr(int tree_ID);
   tree<T>* reset_trees_map_ptr();
   tree<T>* get_next_tree_ptr();

// Union-find algorithm member functions:

   tree<T>* MakeSet(treenode<T>* treenode_ptr);
   treenode<T>* Find(treenode<T>* treenode_ptr);
   treenode<T>* Link(treenode<T>* treenode1_ptr,treenode<T>* treenode2_ptr);

  private: 

   TREENODES_MAP* treenodes_map_ptr;
   typename TREENODES_MAP::iterator treenode_iter;
   typename TREENODES_MAP::iterator curr_treenode_iter;

   TREES_MAP* trees_map_ptr;
   typename std::map<int,tree<T>* >::iterator trees_iter;
   typename std::map<int,tree<T>* >::iterator curr_tree_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Forest<T>& F);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

#include "Forest.cc"

#endif  // T_datastructures/Forest.h






