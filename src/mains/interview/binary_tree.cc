// ==========================================================================
// Binary tree implementation
// ==========================================================================
// Last updated on 2/5/16
// ==========================================================================

#include <iostream>
#include <vector>
#include "math/mathfuncs.h"
// #include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

typedef int item_t;

typedef struct treenode
{
   item_t item;
   struct treenode *parent;
   struct treenode *left;
   struct treenode *right;
} treenode;


// ------------------------------------------------------------------
treenode* generate_new_treenode(item_t x, treenode *parent_treenode)
{
   treenode* new_treenode = new treenode;
   new_treenode->item = x;
   new_treenode->parent = parent_treenode;
   new_treenode->left = NULL;
   new_treenode->right = NULL;
   return new_treenode;
}

// ------------------------------------------------------------------
void print_treenode(treenode *curr_treenode)
{
   if(curr_treenode == NULL)
   {
      cout << "Current node = NULL" << endl;
      return;
   }
   else
   {
      cout << "Current node item = " << curr_treenode->item << endl;   
   }

   if(curr_treenode->parent == NULL)
   {
      cout << "Parent node = NULL" << endl;
   }
   else
   {
      cout << "Parent note item = " << curr_treenode->parent->item << endl;
   }
   
   if(curr_treenode->left == NULL)
   {
      cout << "Left node = NULL" << endl;
   }
   else
   {
      cout << "Left node item = " << curr_treenode->left->item << endl;
   }

   if(curr_treenode->right == NULL)
   {
      cout << "Right node = NULL" << endl;
   }
   else
   {
      cout << "Right node item = " << curr_treenode->right->item << endl;
   }
   cout << endl;
}

// ------------------------------------------------------------------
// GOOD SOLUTION:

void insert_node(treenode **curr_treenode_ptr, item_t x, treenode *parent_node)
{
   if(*curr_treenode_ptr == NULL)
   {
      treenode *new_treenode = generate_new_treenode(x, parent_node);
      *curr_treenode_ptr = new_treenode;
      return;
   }

   if(x < (*curr_treenode_ptr)->item)
   {
      insert_node( &( (*curr_treenode_ptr)->left), x, *curr_treenode_ptr);
   }
   else
   {
      insert_node( &( (*curr_treenode_ptr)->right), x, *curr_treenode_ptr);
   }
}

// ------------------------------------------------------------------
// SOLUTION WORKS BUT IS VERBOSE

treenode* insert_item(treenode *curr_treenode, item_t x)
{

// Case 0: curr_treenode is NULL

   if(curr_treenode == NULL)
   {
      treenode *new_treenode = generate_new_treenode(x, NULL);
      return new_treenode;
   }

// Case 1:  curr_treenode is a leaf (0 children)

   if(curr_treenode->left == NULL && curr_treenode->right == NULL)
   {
      treenode *new_treenode = generate_new_treenode(x, curr_treenode);
      if(curr_treenode->item > x)
      {
         curr_treenode->left = new_treenode;
      }
      else
      {
         curr_treenode->right = new_treenode;
      }
      return new_treenode;
   }

// Case 2a:  curr_treenode has one child on right

   if(curr_treenode->left == NULL && x < curr_treenode->item)
   {
      treenode *new_treenode = generate_new_treenode(x, curr_treenode);
      curr_treenode->left = new_treenode;
      return new_treenode;
   }
   else if(curr_treenode->left == NULL && x >= curr_treenode->item)
   {
      return insert_item(curr_treenode->right, x);
   }

// Case 2b:  curr_treenode has one child on left

   if(curr_treenode->right == NULL && x >= curr_treenode->item)
   {
      treenode *new_treenode = generate_new_treenode(x, curr_treenode);
      curr_treenode->right = new_treenode;
      return new_treenode;
   }
   else if(curr_treenode->right == NULL && x < curr_treenode->item)
   {
      return insert_item(curr_treenode->left, x);
   }

// Case 3:  curr_treenode has one left and one right child

   if(x < curr_treenode->item)
   {
      return insert_item(curr_treenode->left, x);
   }
   else 
   {
      return insert_item(curr_treenode->right, x);
   }
}

// ------------------------------------------------------------------
treenode* search_tree(treenode *curr_treenode, item_t x)
{
   if(curr_treenode == NULL) return NULL;
   
   if(curr_treenode->item == x) return curr_treenode;
   
   if(x < curr_treenode->item)
   {
      return search_tree(curr_treenode->left, x);
   }
   else
   {
      return search_tree(curr_treenode->right, x);
   }
}

// ------------------------------------------------------------------
// "Left-most" node contains item with minimal value

treenode* find_minimum_node(treenode *curr_treenode)
{
   treenode *min_node = NULL;
   
   if(curr_treenode == NULL) return NULL;
   
   min_node = curr_treenode;
   while (min_node->left != NULL)
   {
      min_node = min_node->left;
   }
   return min_node;
}

// ------------------------------------------------------------------
// "Right-most" node contains item with maximal value

treenode* find_maximum_node(treenode *curr_treenode)
{
   treenode *max_node = NULL;
   
   if(curr_treenode == NULL) return NULL;
   
   max_node = curr_treenode;
   while (max_node->right != NULL)
   {
      max_node = max_node->right;
   }
   return max_node;
}

// ------------------------------------------------------------------
// Method traverse_tree() performs in-order traversal of binary tree:

void traverse_tree(treenode *curr_treenode)
{
   if (curr_treenode != NULL)
   {
      traverse_tree(curr_treenode->left);
      print_treenode(curr_treenode);
      traverse_tree(curr_treenode->right);
   }
}

// ==========================================================================

int main()
{
   treenode* root_node = NULL;

   int n_iters = 0;
   cout << "Enter number of times to randomize integer sequence:" << endl;
   cin >> n_iters;
   
   for(int n = 0; n < n_iters; n++)
   {
     vector<int> ran_seq = mathfunc::random_sequence(100);
   }
   vector<int> ran_seq = mathfunc::random_sequence(100);

//   nrfunc::init_time_based_seed();
   int n_treenodes;
   cout << "Enter number of nodes to insert into tree:" << endl;
   cin >> n_treenodes;

   for(int i = 0; i < n_treenodes; i++)
   {
      int curr_item = ran_seq[i];
      cout << "Inserting " << curr_item << endl;

      insert_node(&root_node, curr_item, NULL);

/*
//      treenode* inserted_node = insert_item(root_node, curr_item);
      if(root_node == NULL)
      {
         root_node = inserted_node;
      }
*/

   } // loop over index i labeling tree nodes

   cout << endl;
   cout << "TREE:" << endl << endl;
   traverse_tree(root_node);

   treenode* min_node = find_minimum_node(root_node);
   cout << "Minimum node item = " << min_node->item << endl;
   treenode* max_node = find_maximum_node(root_node);
   cout << "Maximum node item = " << max_node->item << endl;

}

