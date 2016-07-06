// ==========================================================================
// Templatized Forest class member function definitions
// ==========================================================================
// Last modified on 7/17/12; 7/18/12; 7/21/12
// ==========================================================================

#include "math/basic_math.h"
#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

// Initialization, constructor and destructor methods:

template <class T> void Forest<T>::allocate_member_objects() 
{
   treenodes_map_ptr=new TREENODES_MAP;
   trees_map_ptr=new TREES_MAP;
}

template <class T> void Forest<T>::initialize_member_objects() 
{
}

template <class T> Forest<T>::Forest()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

template <class T> Forest<T>::Forest(const Forest<T>& F)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(F);
}

template <class T> void Forest<T>::docopy(const Forest<T>& F)
{
}	

// Overload = operator:

template <class T> Forest<T>& Forest<T>::operator= (const Forest<T>& F)
{
   if (this==&F) return *this;
   docopy(F);
   return *this;
}

template <class T> Forest<T>::~Forest()
{
   std::cout << "inside Forest destructor" << std::endl;

   purge_trees();
   delete trees_map_ptr;
   delete treenodes_map_ptr;
}

// ---------------------------------------------------------------------
// Member function purge_trees()

template<class T> void Forest<T>::purge_trees() 
{
   std::vector<tree<T>* > trees_to_delete;

   for (trees_iter=trees_map_ptr->begin(); trees_iter != trees_map_ptr->end();
        trees_iter++)
   {
      trees_to_delete.push_back(trees_iter->second);
   }

   int n_trees=trees_to_delete.size();
   std::cout << "inside Forest::purge_trees(), n_trees = " << n_trees
             << std::endl;
   for (int n=0; n<n_trees; n++)
   {
      trees_to_delete[n]->set_destroy_treenodes_flag(true);
      delete trees_to_delete[n];
   }
   trees_map_ptr->clear();
}

/*
// ---------------------------------------------------------------------
// Member function purge_treenodes()

template<class T> void Forest<T>::purge_treenodes() 
{
   std::cout << "inside Forest::purge_treenodes()" << std::endl;

   std::vector<treenode<T>* > treenodes_to_delete;

   treenode<T>* treenode_ptr=reset_treenodes_map_ptr();
   while (treenode_ptr != NULL)
   {
      treenodes_to_delete.push_back(treenode_ptr);
      treenode_ptr=get_next_treenode_ptr();
   }

   int n_treenodes=treenodes_to_delete.size();
   std::cout << "n_treenodes = " << n_treenodes << std::endl;
   for (int n=0; n<n_treenodes; n++)
   {
      delete treenodes_to_delete[n];
   }
   treenodes_map_ptr->clear();

   std::cout << "at end of Forest::purge_treenodes()" << std::endl;
}
*/

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,Forest<T>& F)
{
   int n_trees=F.get_n_trees();

   tree<T>* curr_tree_ptr=F.reset_trees_map_ptr();
   while (curr_tree_ptr != NULL)
   {
      outstream << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" 
                << std::endl;
      outstream << "Tree = " << *curr_tree_ptr << std::endl;
      curr_tree_ptr=F.get_next_tree_ptr();
   } // loop over index t labeling trees within current Forest

   outstream << "F.get_n_trees() = " << n_trees << std::endl;
   outstream << "F.get_n_treenodes() = " << F.get_n_treenodes() << std::endl;

   outstream << std::endl;
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

template<class T> int Forest<T>::get_n_treenodes() const
{
   return treenodes_map_ptr->size();
}

template<class T> int Forest<T>::get_n_trees() const
{
   return trees_map_ptr->size();
}

// ==========================================================================
// Treenode retrieval member functions
// ==========================================================================

template<class T> treenode<T>* Forest<T>::get_treenode_ptr(int ID)
{
   treenode_iter=treenodes_map_ptr->find(ID);
   if (treenode_iter==treenodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return treenode_iter->second;
   }
}

// ---------------------------------------------------------------------
template <class T> treenode<T>* Forest<T>::reset_treenodes_map_ptr()
{
//   std::cout << "inside Forest::reset_treenodes_ptr()" << std::endl;
   curr_treenode_iter=treenodes_map_ptr->begin();
   if (curr_treenode_iter==treenodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return curr_treenode_iter->second;
   }
}

// ---------------------------------------------------------------------
template <class T> treenode<T>* Forest<T>::get_next_treenode_ptr()
{
   curr_treenode_iter++;
   if (curr_treenode_iter==treenodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return curr_treenode_iter->second;
   }
}

// ==========================================================================
// Tree generation, retrieval and deletion member functions
// ==========================================================================

// Member function generate_new_tree()

template<class T> tree<T>* Forest<T>::generate_new_tree(
   treenode<T>* treenode_ptr)
{
   int tree_ID=treenode_ptr->get_ID();
   bool generate_treenode_maps_flag=false;
   tree<T>* tree_ptr=new tree<T>(tree_ID,generate_treenode_maps_flag);

   treenode<T>* rootnode_ptr=treenode_ptr->get_rootnode_ptr();
   if (rootnode_ptr != treenode_ptr)
   {
      std::cout << "inside Forest::generate_new_tree(), tree_ptr->get_ID()="
                << tree_ptr->get_ID()
                << " treenode_ptr->get_ID() = " << treenode_ptr->get_ID()
                << std::endl;
      std::cout << "rootnode_ptr->get_ID() = " << rootnode_ptr->get_ID()
                << std::endl;
      outputfunc::enter_continue_char();
   }
   
   tree_ptr->set_rootnode_ptr(rootnode_ptr);
   tree_ptr->recursively_generate_tree();
   (*trees_map_ptr)[tree_ID]=tree_ptr;

   return tree_ptr;
}

// ---------------------------------------------------------------------
template<class T> tree<T>* Forest<T>::get_tree_ptr(int tree_ID)
{
   trees_iter=trees_map_ptr->find(tree_ID);
   if (trees_iter==trees_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return trees_iter->second;
   }
}

// ---------------------------------------------------------------------
template<class T> void Forest<T>::delete_tree_ptr(int tree_ID)
{
   trees_iter=trees_map_ptr->find(tree_ID);
   if (trees_iter != trees_map_ptr->end())
   {
      tree<T>* tree_ptr=trees_iter->second;
      delete tree_ptr;
      trees_map_ptr->erase(trees_iter);
   }
}

// ---------------------------------------------------------------------
template <class T> tree<T>* Forest<T>::reset_trees_map_ptr()
{
   curr_tree_iter=trees_map_ptr->begin();
   return curr_tree_iter->second;
}

// ---------------------------------------------------------------------
template <class T> tree<T>* Forest<T>::get_next_tree_ptr()
{
   curr_tree_iter++;
   if (curr_tree_iter==trees_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return curr_tree_iter->second;
   }
}

// ==========================================================================
// Union-Find algorithm member functions
// ==========================================================================

// Member function MakeSet()

template<class T> tree<T>* Forest<T>::MakeSet(treenode<T>* treenode_ptr)
{
//   std::cout << "inside Forest::MakeSet()" << std::endl;
   
   treenode_ptr->set_parent_node_ptr(treenode_ptr);
   treenode_ptr->set_rank(0);
   (*treenodes_map_ptr)[treenode_ptr->get_ID()]=treenode_ptr;

   return generate_new_tree(treenode_ptr);
}

// ---------------------------------------------------------------------
// Member function Find()

template<class T> treenode<T>* Forest<T>::Find(treenode<T>* treenode_ptr)
{
//   std::cout << "inside Forest::Find()" << std::endl;
   
   treenode<T>* parent_node_ptr=treenode_ptr->get_parent_node_ptr();
   if (parent_node_ptr != treenode_ptr)
   {
      parent_node_ptr=Find(parent_node_ptr);
   }
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
// Member function Link()

template<class T> treenode<T>* Forest<T>::Link(
   treenode<T>* treenode1_ptr,treenode<T>* treenode2_ptr)
{
//   std::cout << "inside Forest::Link()" << std::endl;
//   std::cout << "n_trees = " << get_n_trees() << std::endl;

   int rank1=treenode1_ptr->get_rank();
   int rank2=treenode2_ptr->get_rank();

   if (treenode1_ptr->get_rank() > treenode2_ptr->get_rank())
   {
      templatefunc::swap(treenode1_ptr,treenode2_ptr);
   }

   if (treenode1_ptr->get_rank()==treenode2_ptr->get_rank())
   {
      treenode2_ptr->set_rank(treenode2_ptr->get_rank()+1);
   }

// Delete existing tree1 and tree2 but not their tree nodes:

   tree<T>* tree1_ptr=get_tree_ptr(treenode1_ptr->get_ID());
   tree<T>* tree2_ptr=get_tree_ptr(treenode2_ptr->get_ID());

   tree1_ptr->set_destroy_treenodes_flag(false);
   tree2_ptr->set_destroy_treenodes_flag(false);

   delete_tree_ptr(tree1_ptr->get_ID());
   delete_tree_ptr(tree2_ptr->get_ID());

// Generate new tree2:

   treenode1_ptr->set_parent_node_ptr(treenode2_ptr);

// As of 7/18/12, we *think* that treenode1_ptr should not have itself
// as a child once treenode2_ptr becomes its parent...

   treenode1_ptr->deleteChild(treenode1_ptr);

   tree<T>* tree12_ptr=generate_new_tree(treenode2_ptr);

//   std::cout << "n_trees = " << get_n_trees() << std::endl;
//   outputfunc::enter_continue_char();

   return treenode2_ptr;
}

// ---------------------------------------------------------------------
// Member function recursively_count_nodes()

template<class T> int Forest<T>::recursively_count_nodes()
{
//   std::cout << "inside Forest::recursively_count_nodes()" << std::endl;

   int n_nodes=0;
   tree<T>* tree_ptr=reset_trees_map_ptr();
   int tree_counter=1;
   while (tree_ptr != NULL)
   {
      int tree_ID=tree_ptr->get_ID();
      int curr_n_nodes = tree_ptr->recursively_count_nodes();
      n_nodes += curr_n_nodes;
//      std::cout << "tree_counter = " << tree_counter++
//                << " tree_ID = " << tree_ID
//                << " tree_ptr = " << tree_ptr
//                << " curr_n_nodes = " << curr_n_nodes 
//                << " n_nodes = " << n_nodes
//                << std::endl;
      tree_ptr=get_next_tree_ptr();
   }
//   std::cout << "Recursive node count = " << n_nodes << std::endl;
   return n_nodes;
}

