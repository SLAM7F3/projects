// ==========================================================================
// Program TREETEST
// ==========================================================================
// Last updated on 3/16/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "datastructures/BinaryTree.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   typedef twovector DATA;
   typedef BinaryTreeNode<DATA> BTreeNode;
   typedef BinaryTree<DATA> BTree;

   BTree* BinaryTree_ptr=new BTree();

   DATA root_data=twovector(0,0);
   BinaryTree_ptr->generate_root_node(root_data);


   DATA left_data=twovector(1,1);
   BinaryTree_ptr->addLeftChild(0,left_data);


   DATA right_data=twovector(2,2);   
   BinaryTree_ptr->addRightChild(0,right_data);

   left_data=twovector(0,3);
   BinaryTree_ptr->addLeftChild(1,left_data);


   left_data=twovector(3,0);
   BinaryTree_ptr->addLeftChild(3,left_data);

   cout << "*BinaryTree_ptr = " << *BinaryTree_ptr << endl;
} 

