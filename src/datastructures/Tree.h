// ==========================================================================
// Header file for templatized Tree class
// ==========================================================================
// Last modified on 11/7/06; 11/8/06; 11/9/06; 11/13/06; 1/6/07
// ==========================================================================


#ifndef T_TREE_H
#define T_TREE_H

#include <fstream>
#include <set>
#include <string>
#include <vector>
#include "datastructures/Treenode.h"
#include "datastructures/Triple.h"

template <class T>
class Tree
{

  public:
   
// Initialization, constructor and destructor functions:

   Tree();
   Tree(const Tree<T>& t);
   Tree<T>& operator= (const Tree<T>& t);
   virtual ~Tree();

   template <class T1>
      friend std::ostream& operator<< (
         std::ostream& outstream,const Tree<T1>& t);

// Set and get member functions:

   Treenode<T>* get_root_ptr();
   int size() const;
   int get_n_levels();
   Treenode<T>* get_Treenode_ptr(int n) ;
   Treenode<T>* get_ID_labeled_Treenode_ptr(int ID);
   Treenode<T>* get_total_indices_labeled_Treenode_ptr(
      const std::vector<int>& tot_indices);
   int get_ID(const std::vector<int>& tot_indices);

// Data retrieval & insertion member functions:

   int get_next_unused_ID() const;
   Treenode<T>* addChild(int parent_id);
   Treenode<T>* addChild(const std::vector<int>& tot_indices);
   std::vector<Treenode<T>* > retrieve_nodes_on_level(int level);
   int number_nodes_on_level(int level);
   void compute_columns_for_nodes_on_level(int level);
   Treenode<T>* get_node(int level,int column);
   Treenode<T>* generate_root_node();
   void purge_nodes();

// Node manipulation member functions:

  private: 

   Treenode<T>* root_ptr;
   std::vector<Triple<int,std::vector<int>,Treenode<T>* > > tree_nodes;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Tree<T>& t);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline Treenode<T>* Tree<T>::get_root_ptr()
{
   return root_ptr;
}

template <class T> inline int Tree<T>::size() const
{
   return tree_nodes.size();
}

#include "Tree.cc"

#endif  // T_datastructures/Tree.h






