// ==========================================================================
// Header file for templatized Binary Tree class
// ==========================================================================
// Last modified on 3/16/12; 3/17/12
// ==========================================================================

#ifndef T_BINARYTREE_H
#define T_BINARYTREE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "datastructures/BinaryTreeNode.h"
#include "templates/mytemplates.h"

template <class T>
class BinaryTree
{

  public:
   
   typedef std::map<int,BinaryTreeNode<T>* > BINARYTREENODES_MAP;

// Indep var: BinaryTreeNode integer ID
// Depend var: pointer to BinaryTreeNode

// Initialization, constructor and destructor functions:

   BinaryTree();
   BinaryTree(const BinaryTree<T>& t);
   BinaryTree<T>& operator= (const BinaryTree<T>& t);
   virtual ~BinaryTree();

   template <class T1>
      friend std::ostream& operator<< (
         std::ostream& outstream,const BinaryTree<T1>& t);

// Set and get member functions:

   BINARYTREENODES_MAP* get_BinaryTreeNodes_map_ptr()
   {
      return BinaryTreeNodes_map_ptr;
   }
  
   const BINARYTREENODES_MAP* get_BinaryTreeNodes_map_ptr() const
   {
      return BinaryTreeNodes_map_ptr;
   }

// Tree information member functions

   int size() const;
   int get_next_unused_ID();
   int get_n_levels() const;
   BinaryTreeNode<T>* get_root_ptr();

// Node generation & retrieval member functions:

   BinaryTreeNode<T>* generate_root_node();
   BinaryTreeNode<T>* generate_root_node(const T& root_data);
   BinaryTreeNode<T>* get_node_ptr(int ID);
   BinaryTreeNode<T>* get_parent_node_ptr(int child_ID);
   std::vector<BinaryTreeNode<T>*> get_descendant_node_ptrs(int ID);

// Node insertion member functions:

   BinaryTreeNode<T>* addLeftChild(int parent_ID,T& data);
   BinaryTreeNode<T>* addLeftChild(
      BinaryTreeNode<T>* parent_node_ptr,int left_child_ID,T& data);
   BinaryTreeNode<T>* addRightChild(int parent_ID,T& data);
   BinaryTreeNode<T>* addRightChild(
      BinaryTreeNode<T>* parent_node_ptr,int right_child_ID,T& data);
   
   void addExistingNodeToTree(
      BinaryTreeNode<T>* parent_node_ptr,BinaryTreeNode<T>* child_node_ptr);

// Node removal member functions:

   void purge_nodes();

// Node placement member functions

   void compute_all_gxgy_coords();
   void compute_children_gxgy_coords(BinaryTreeNode<T>* parent_node_ptr);

  private: 

   BinaryTreeNode<T>* root_ptr;
   BINARYTREENODES_MAP* BinaryTreeNodes_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const BinaryTree<T>& t);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:


#include "BinaryTree.cc"

#endif  // T_BINARYTREE_H





