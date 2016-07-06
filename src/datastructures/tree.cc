// ==========================================================================
// templatized tree class member function definitions
// ==========================================================================
// Last modified on 7/21/12; 8/5/12; 8/6/12; 11/24/12
// ==========================================================================

#include "math/basic_math.h"
#include "general/outputfuncs.h"

// Initialization, constructor and destructor methods:

template <class T> void tree<T>::allocate_member_objects() 
{
}

template <class T> void tree<T>::initialize_member_objects() 
{
   destroy_treenodes_flag=true;
   ID=-1;
   next_unused_treenode_ID=-1;
   curr_treenodes_map_ptr=NULL;
   root_node_ptr=NULL;

   treenodes_map_map_ptr=NULL;
   leaf_treenodes_map_ptr=NULL;
}

template <class T> void tree<T>::generate_treenode_maps() 
{
//   std::cout << "inside tree::generate_treenode_maps()" << std::endl;
   treenodes_map_map_ptr=new std::map<int,TREENODES_MAP*>;
   leaf_treenodes_map_ptr=new TREENODES_MAP();
}

template <class T> tree<T>::tree(bool generate_treenode_maps_flag)
{
   initialize_member_objects();
   allocate_member_objects();
   if (generate_treenode_maps_flag) generate_treenode_maps();
}

template <class T> tree<T>::tree(int ID,bool generate_treenode_maps_flag)
{
   initialize_member_objects();
   allocate_member_objects();
   this->ID=ID;

   if (generate_treenode_maps_flag) generate_treenode_maps();
}

// Copy constructor:

template <class T> tree<T>::tree(const tree<T>& t)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(t);
}

template <class T> void tree<T>::docopy(const tree<T>& t)
{
}	

// Overload = operator:

template <class T> tree<T>& tree<T>::operator= (const tree<T>& t)
{
   if (this==&t) return *this;
   docopy(t);
   return *this;
}

template <class T> tree<T>::~tree()
{
//   std::cout << "inside tree destructor" << std::endl;
//   if (destroy_treenodes_flag) recursively_destroy_treenodes();
   if (destroy_treenodes_flag) purge_nodes();
   delete treenodes_map_map_ptr;
   delete leaf_treenodes_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,tree<T>& t)
{
   outstream << std::endl;
   outstream << "tree ID = " << t.get_ID() << std::endl;
   outstream << "min_level = " << t.get_min_level() << std::endl;
   outstream << "max_level = " << t.get_max_level() << std::endl;
   outstream << "n_nodes = size() = " << t.size() << std::endl;
//   outstream << "recursive node count = " << t.recursively_count_nodes() 
//             << std::endl;
   outstream << "n_leaf_nodes = " << t.get_n_leaf_nodes() << std::endl;
   outstream << std::endl;

   for (int level=t.get_min_level(); level <= t.get_max_level(); level++)
   {
      std::cout << "level = " << level << std::endl;
      std::vector<treenode<T>* > nodes_on_level=t.retrieve_nodes_on_level(
         level);

// FAKE FAKE:  Weds July 18, 2012 at 8:14 pm
// Comment out next for loop

      for (int c=0; c<nodes_on_level.size(); c++)
      {
         outstream << "........................................" << std::endl;
         outstream << "Column = " << c << std::endl;
         outstream << "Treenode->get_data_ptr() = " 
                   << nodes_on_level[c]->get_data_ptr()
                   << std::endl;
//         outstream << *(nodes_on_level[c]) << std::endl;
      } // loop over index c labeling nodes on current level

   } // loop over level 

   return outstream;
}

// ==========================================================================
// Treenode insertion, retrieval & destruction member functions
// ==========================================================================

// Boolean member function treenode_exists() returns true if the input
// treenode already exists within the current tree.

template <class T> bool tree<T>::treenode_exists(
   int level,treenode<T>* treenode_ptr)
{
   TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(level);
   if (treenodes_map_ptr==NULL) return false;

   treenode_iter=treenodes_map_ptr->find(treenode_ptr->get_ID());
   if (treenode_iter==treenodes_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

// --------------------------------------------------------------------------
template <class T> treenode<T>* tree<T>::get_treenode_ptr(int level,int ID)
{
   TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(level);
   if (treenodes_map_ptr==NULL) return NULL;
   
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
// Member function get_next_unused_treenode_ID()

template<class T> int tree<T>::get_next_unused_treenode_ID()
{
   next_unused_treenode_ID++;
   return next_unused_treenode_ID;
}

// ---------------------------------------------------------------------
// Member function generate_new_treenode()

template<class T> treenode<T>* tree<T>::generate_new_treenode(int level)
{
//    std::cout << "inside tree::generate_new_treenode() #1" << std::endl;
   return generate_new_treenode(get_next_unused_treenode_ID(),level);
}

template<class T> treenode<T>* tree<T>::generate_new_treenode(int ID,int level)
{
//    std::cout << "inside tree::generate_new_treenode() #2" << std::endl;

   treenode<T>* treenode_ptr=new treenode<T>(ID,level);
   insert_new_treenode(level,treenode_ptr);
   return treenode_ptr;
}

template<class T> void tree<T>::insert_new_treenode(
   int level,treenode<T>* treenode_ptr)
{
//   std::cout << "inside tree::insert_new_treenode()" << std::endl;

   treenode_ptr->set_level(level);   

   if (treenodes_map_map_ptr==NULL) return;

   TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(level);
   if (treenodes_map_map_ptr != NULL && treenodes_map_ptr==NULL)
   {
      treenodes_map_ptr=new TREENODES_MAP();
      (*treenodes_map_map_ptr)[level]=treenodes_map_ptr;
   }

   int ID=treenode_ptr->get_ID();
   (*treenodes_map_ptr)[ID]=treenode_ptr;
}

// ---------------------------------------------------------------------
template<class T> treenode<T>* tree<T>::generate_new_treenode(
   int ID,int level,TREENODES_MAP* treenodes_map_ptr)
{
//   std::cout << "inside tree::generate_new_treenode() #3" << std::endl;

   treenode<T>* treenode_ptr=new treenode<T>(ID,level);
   (*treenodes_map_ptr)[ID]=treenode_ptr;
   return treenode_ptr;
}

// ---------------------------------------------------------------------
// Member function purge_nodes loops over all treenodes as a function
// of level and column.  It deletes each treenode and clears the
// *treenodes_map_map_ptr member object.

template<class T> void tree<T>::purge_nodes() 
{
//   std::cout << "inside tree::purge_nodes()" << std::endl;

   std::vector<treenode<T>* > treenodes_to_delete;

   int n_levels=get_n_levels();
   for (int l=0; l<n_levels; l++)
   {
      TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(l);
      if (treenodes_map_ptr==NULL) continue;
   
      for (treenode_iter=treenodes_map_ptr->begin(); treenode_iter != 
              treenodes_map_ptr->end(); treenode_iter++)
      {
         treenode<T>* curr_node_ptr=treenode_iter->second;
         treenodes_to_delete.push_back(curr_node_ptr);
      }
   } // loop over index l labeling levels

   int n_treenodes=treenodes_to_delete.size();
   for (int n=0; n<n_treenodes; n++)
   {
      delete treenodes_to_delete[n];
   }

   for (int l=0; l<n_levels; l++)
   {
      delete get_treenodes_map_ptr(l);
   }
   treenodes_map_map_ptr->clear();
}
   
// ==========================================================================
// Tree level member functions
// ==========================================================================

template <class T> int tree<T>::get_n_levels() 
{
   if (treenodes_map_map_ptr==NULL) 
   {
      return -1;
   }
   else
   {
      return treenodes_map_map_ptr->size();
   }
}

template <class T> int tree<T>::get_min_level() 
{
   if (treenodes_map_map_ptr==NULL) return -1;
   int min_level=1000000;
   for (treenodes_map_iter=treenodes_map_map_ptr->begin(); 
        treenodes_map_iter != treenodes_map_map_ptr->end(); 
        treenodes_map_iter++)
   {
      int curr_level=treenodes_map_iter->first;
      min_level=basic_math::min(min_level,curr_level);
   }
   return min_level;
}

template <class T> int tree<T>::get_max_level() 
{
   if (treenodes_map_map_ptr==NULL) return -1;
   int max_level=-1;

   for (treenodes_map_iter=treenodes_map_map_ptr->begin(); 
        treenodes_map_iter != treenodes_map_map_ptr->end(); 
        treenodes_map_iter++)
   {
      int curr_level=treenodes_map_iter->first;
      max_level=basic_math::max(max_level,curr_level);
   }
   return max_level;
}

// ---------------------------------------------------------------------
// Member function retrieve_nodes_on_level() is generally deprecated.
// Use reset_curr_treenodes_map_ptr() and get_next_treenode_ptr()
// instead.

template <class T> std::vector<treenode<T>* > 
tree<T>::retrieve_nodes_on_level(int level)
{
   std::vector<treenode<T>* > nodes_on_level;
   TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(level);
   if (treenodes_map_ptr==NULL) return nodes_on_level;

   for (treenode_iter=treenodes_map_ptr->begin(); treenode_iter != 
           treenodes_map_ptr->end(); treenode_iter++)
   {
      treenode<T>* curr_node_ptr=treenode_iter->second;
      if (curr_node_ptr->get_level()==level)
      {
         nodes_on_level.push_back(curr_node_ptr);
      }
   }
   return nodes_on_level;
}

// ---------------------------------------------------------------------
template <class T> int tree<T>::number_nodes_on_level(int level)
{
//   std::cout << "inside tree::number_nodes_on_level(), level = "
//             << level << std::endl;
   TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(level);
   if (treenodes_map_ptr==NULL) 
   {
      return 0;
   }
   else
   {
      return treenodes_map_ptr->size();
   }
}

// ---------------------------------------------------------------------
template <class T> int tree<T>::size()
{
//   std::cout << "inside tree::size()" << std::endl;
   int n_nodes=0;
//   std::cout << "get_n_levels() = " << get_n_levels() << std::endl;
   for (int l=get_min_level(); l<=get_max_level(); l++)
   {
      n_nodes += number_nodes_on_level(l);
//      std::cout << "l = " << l << " n_nodes = " << n_nodes << std::endl;
   }
   return n_nodes;
}

// ==========================================================================
// Same-level treenode iteration member functions
// ==========================================================================

template <class T> treenode<T>* tree<T>::reset_curr_treenodes_map_ptr(
   int level)
{
   curr_treenodes_map_ptr=get_treenodes_map_ptr(level);
   if (curr_treenodes_map_ptr==NULL) return NULL;
   curr_treenode_iter=curr_treenodes_map_ptr->begin();
   return curr_treenode_iter->second;
}

template <class T> treenode<T>* tree<T>::get_next_treenode_ptr()
{
   curr_treenode_iter++;
   if (curr_treenode_iter==curr_treenodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return curr_treenode_iter->second;
   }
}

// ==========================================================================
// Leaf treenode member functions
// ==========================================================================

template <class T> void tree<T>::pushback_leaf_node(
   treenode<T>* leaf_treenode_ptr)
{
   int leaf_ID=leaf_treenode_ptr->get_ID();
   (*leaf_treenodes_map_ptr)[leaf_ID]=leaf_treenode_ptr;
}

template <class T> treenode<T>* tree<T>::reset_leafnode_ptr()
{
//   std::cout << "inside tree::reset_leafnode_ptr()" << std::endl;
   
   leafnode_iter=leaf_treenodes_map_ptr->begin();
   return leafnode_iter->second;
}

template <class T> treenode<T>* tree<T>::get_next_leafnode_ptr()
{
//   std::cout << "inside tree::get_next_leafnode_ptr()" << std::endl;
//   std::cout << "leaf_treenodes_map_ptr = " << leaf_treenodes_map_ptr
//             << std::endl;

   leafnode_iter++;
   if (leafnode_iter==leaf_treenodes_map_ptr->end())
   {
//      std::cout << "returning null" << std::endl;
      return NULL;
   }
   else
   {
//      std::cout << "returning leafnode_iter->second = "
//                << leafnode_iter->second << std::endl;
      return leafnode_iter->second;
   }
}

// ==========================================================================
// Recursive tree member functions
// ==========================================================================

// Member function recursively_count_nodes()

template <class T> int tree<T>::recursively_count_nodes()
{
//   std::cout << "inside tree::recursively_count_nodes()" << std::endl;

   if (get_rootnode_ptr()==NULL)
   {
      std::cout << "Error in tree::recursively_count_nodes()!" << std::endl;
      std::cout << "rootnode_ptr = NULL" << std::endl;
      return -1;
   }

   int curr_level=basic_math::max(0,get_max_level());
   int n_nodes=0;
   recursively_count_nodes(curr_level,get_rootnode_ptr(),n_nodes);
   return n_nodes;
}

// ---------------------------------------------------------------------
template <class T> void tree<T>::recursively_count_nodes(
   int curr_level,treenode<T>* input_treenode_ptr,int& n_nodes)
{
//   std::cout << "inside tree::recursively_count_nodes() #2, curr_level = "
//             << curr_level << " n_nodes = " << n_nodes << std::endl;

   std::vector<treenode<T>* > children_node_ptrs=
      input_treenode_ptr->getChildrenNodePtrs();

// Recursively call count_nodes() on all children nodes of
// *input_treenode_ptr (which do NOT equal input_treenode_ptr!):

   for (int c=0; c<children_node_ptrs.size(); c++)
   {
      if (children_node_ptrs[c] != input_treenode_ptr)
      {
         recursively_count_nodes(
            curr_level-1,children_node_ptrs[c],n_nodes);
      }
   }
   n_nodes=n_nodes+1;
//   std::cout << "  tree_ID = " << get_ID()
//             << " n_treenodes = " << n_nodes
//             << " treenode_ID = " << input_treenode_ptr->get_ID()
//             << std::endl;
}

// ---------------------------------------------------------------------
// Member function recursively_count_levels()

template <class T> int tree<T>::recursively_count_levels()
{
   std::cout << "inside tree::recursively_count_levels()" << std::endl;
   
   if (get_rootnode_ptr()==NULL)
   {
      std::cout << "Error in tree::recursively_count_levels()!" << std::endl;
      std::cout << "rootnode_ptr = NULL" << std::endl;
      return -1;
   }

   int n_levels=-1;
   if (get_max_level() >= 0 && get_min_level() >= 0)
   {
      n_levels=get_max_level()-get_min_level()+1;
   }
   else
   {
      int curr_level=0;
      int lowest_level=0;
      recursively_count_levels(curr_level,get_rootnode_ptr(),lowest_level);
      n_levels=-lowest_level;
   }
   return n_levels;
}

// ---------------------------------------------------------------------
template <class T> void tree<T>::recursively_count_levels(
   int curr_level,treenode<T>* input_treenode_ptr,
   int& lowest_level)
{
//   std::cout << "inside tree::recursively_count_levels(), curr_level = "
//             << curr_level << std::endl;

   std::vector<treenode<T>* > children_node_ptrs=
      input_treenode_ptr->getChildrenNodePtrs();

// Recursively call generate_tree() on all children nodes of
// *input_treenode_ptr (which do NOT equal input_treenode_ptr!):

   for (int c=0; c<children_node_ptrs.size(); c++)
   {
      if (children_node_ptrs[c] != input_treenode_ptr)
      {
         recursively_count_levels(
            curr_level-1,children_node_ptrs[c],lowest_level);
      }
   }
   lowest_level=basic_math::min(lowest_level,curr_level);
}

// ---------------------------------------------------------------------
// Member function recursively_generate_tree() takes in some existing
// treenode which we assume initially corresponds to the root of some
// precalculated set of nodes.  

template <class T> void tree<T>::recursively_generate_tree()
{
   if (get_rootnode_ptr()==NULL)
   {
      std::cout << "Error in tree::recursively_generate_tree()!" << std::endl;
      std::cout << "rootnode_ptr = NULL" << std::endl;
      return;
   }

   int n_levels=recursively_count_levels();
   recursively_generate_tree(n_levels,get_rootnode_ptr());
}

// ---------------------------------------------------------------------
template <class T> void tree<T>::recursively_generate_tree(
   int curr_level,treenode<T>* input_treenode_ptr)
{
   std::vector<treenode<T>* > children_node_ptrs=
      input_treenode_ptr->getChildrenNodePtrs();

// Recursively call generate_tree() on all children nodes of
// *input_treenode_ptr (which do NOT equal input_treenode_ptr!):

   for (int c=0; c<children_node_ptrs.size(); c++)
   {
      if (children_node_ptrs[c] != input_treenode_ptr)
      {
         recursively_generate_tree(curr_level-1,children_node_ptrs[c]);
      }
   }
   
   insert_new_treenode(curr_level,input_treenode_ptr);
}

// ---------------------------------------------------------------------
// Member function recursively_destroy_treenodes() 

template <class T> void tree<T>::recursively_destroy_treenodes()
{
   std::cout << "inside tree::recursively_destroy_treenodes()" 
             << std::endl;
   
   if (get_rootnode_ptr()==NULL)
   {
      std::cout << "Error in tree::recursively_destroy_treenodes()!" 
                << std::endl;
      std::cout << "rootnode_ptr = NULL" << std::endl;
      return;
   }

   int n_levels=recursively_count_levels();
   std::cout << "n_levels = " << n_levels << std::endl;

   int map_nodes_sum=0;
   for (int l=0; l<n_levels; l++)
   {
      if (get_treenodes_map_ptr(l) != NULL)
      {
         map_nodes_sum += get_treenodes_map_ptr(l)->size();
      }
   }

   int n_deleted_nodes=0;
   std::cout << "Before call to rdt" << std::endl;
//   std::cout << "rootnode = " << get_rootnode_ptr() << std::endl;
   std::cout << "*rootnode_ptr = " << *get_rootnode_ptr() << std::endl;
   std::cout << "*this = " << *this << std::endl;
   
   recursively_destroy_treenodes(n_levels,get_rootnode_ptr(),n_deleted_nodes);
   std::cout << "After call to rdt" << std::endl;

   if (n_deleted_nodes != map_nodes_sum)
   {
      std::cout << "n_deleted_nodes = " << n_deleted_nodes 
                << " map_nodes_sum = " << map_nodes_sum 
                << std::endl;
      outputfunc::enter_continue_char();
   }

   for (int l=0; l<n_levels; l++)
   {
      delete get_treenodes_map_ptr(l);
   }
   if (treenodes_map_map_ptr != NULL)
      treenodes_map_map_ptr->clear();
}

// ---------------------------------------------------------------------
template <class T> void tree<T>::recursively_destroy_treenodes(
   int curr_level,treenode<T>* input_treenode_ptr,
   int& n_deleted_nodes)
{
   std::cout << "n_deleted_nodes = " << n_deleted_nodes << std::endl;
   
   std::vector<treenode<T>* > children_node_ptrs=
      input_treenode_ptr->getChildrenNodePtrs();

   std::cout << "children_node.size() = " << children_node_ptrs.size()
             << std::endl;

// Recursively call destroy_tree() on all children nodes of
// *input_treenode_ptr (which do NOT equal input_treenode_ptr!):

   for (int c=0; c<children_node_ptrs.size(); c++)
   {
      if (children_node_ptrs[c] != input_treenode_ptr)
      {
         recursively_destroy_treenodes(
            curr_level-1,children_node_ptrs[c],n_deleted_nodes);
      }
   }
   
   delete input_treenode_ptr;
   n_deleted_nodes++;
}


