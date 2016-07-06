

#ifndef T_SMALLTREE_H
#define T_SMALLTREE_H

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "datastructures/treenode.h"

template <class T>
class tree
{

  public:

   typedef std::map<int,treenode<T>* > TREENODES_MAP;

// Independent var: treenode integer ID
// Dependent var: pointer to treenode

   typedef std::map<int,TREENODES_MAP*> TREENODES_MAP_MAP;

// Independent variable = level
// Dependent variable = TREENODES_MAP* for specified level

// Initialization, constructor and destructor functions:

   tree(bool generate_treenode_maps_flag);
   tree(int ID,bool generate_treenode_maps_flag);
   tree(const tree<T>& t);
   tree<T>& operator= (const tree<T>& t);
   virtual ~tree();

   template <class T1>
      friend std::ostream& operator<< (
         std::ostream& outstream,tree<T1>& t);

// Set and get member functions:

   void set_destroy_treenodes_flag(bool flag)
   {
      destroy_treenodes_flag=flag;
   }

   int get_ID() const
   {
      return ID;
   }
   
   TREENODES_MAP* get_treenodes_map_ptr(int level)
   {
      if (treenodes_map_map_ptr==NULL) return NULL;
      treenodes_map_iter=treenodes_map_map_ptr->find(level);
      if (treenodes_map_iter==treenodes_map_map_ptr->end())
      {
         return NULL;
      }
      else
      {
         return treenodes_map_iter->second;
      }
   }

   const TREENODES_MAP* get_treenodes_map_ptr(int level) const
   {
      if (treenodes_map_map_ptr==NULL) return NULL;
      treenodes_map_iter=treenodes_map_map_ptr->find(level);
      if (treenodes_map_iter==treenodes_map_map_ptr->end())
      {
         return NULL;
      }
      else
      {
         return treenodes_map_iter->second;
      }
   }
   
   TREENODES_MAP* get_or_create_treenodes_map_ptr(int level)
   {
      TREENODES_MAP* treenodes_map_ptr=get_treenodes_map_ptr(level);
      if (treenodes_map_map_ptr != NULL && treenodes_map_ptr==NULL)
      {
         treenodes_map_ptr=new TREENODES_MAP();
         (*treenodes_map_map_ptr)[level]=treenodes_map_ptr;
      }
      return treenodes_map_ptr;
   }

   void set_rootnode_ptr(treenode<T>* rootnode_ptr)
   {
      root_node_ptr=rootnode_ptr;
   }
   
   treenode<T>* get_rootnode_ptr()
   {
      return root_node_ptr;
   }
   
// Treenode insertion, retrieval & destruction member functions:
   
   bool treenode_exists(int level,treenode<T>* treenode_ptr);
   treenode<T>* get_treenode_ptr(int level,int ID) ;
   int get_next_unused_treenode_ID();
   treenode<T>* generate_new_treenode(int level);
   treenode<T>* generate_new_treenode(int ID,int level);
   void insert_new_treenode(int level,treenode<T>* treenode_ptr);
   treenode<T>* generate_new_treenode(
      int ID,int level,TREENODES_MAP* treenodes_map_ptr);
   void purge_nodes();

// Recursive tree member functions:

   int recursively_count_nodes();
   void recursively_count_nodes(
      int curr_level,treenode<T>* input_treenode_ptr,int& n_nodes);
   int recursively_count_levels();
   void recursively_count_levels(
      int curr_level,treenode<T>* input_treenode_ptr,int& lowest_level);
   void recursively_generate_tree();
   void recursively_generate_tree(
      int curr_level,treenode<T>* input_treenode_ptr);
   void recursively_destroy_treenodes();
   void recursively_destroy_treenodes(
      int curr_level,treenode<T>* input_treenode_ptr,int& n_deleted_nodes);

// Tree level member functions:

   int get_n_levels();
   int get_min_level();
   int get_max_level();
   void reset_min_level_to_zero();

   std::vector<treenode<T>* > retrieve_nodes_on_level(int level);
   int number_nodes_on_level(int level);
   int size();

// Same-level treenode iteration member functions:

   treenode<T>* reset_curr_treenodes_map_ptr(int level);
   treenode<T>* get_next_treenode_ptr();

// Leaf treenode member functions:

   int get_n_leaf_nodes() const
   {
      if (leaf_treenodes_map_ptr==NULL)
      {
         return -1;
      }
      else
      {
         return leaf_treenodes_map_ptr->size();
      }
   }
   void pushback_leaf_node(treenode<T>* treenode_ptr);
   treenode<T>* reset_leafnode_ptr();
   treenode<T>* get_next_leafnode_ptr();

  private: 

   TREENODES_MAP_MAP* treenodes_map_map_ptr;
   typename TREENODES_MAP_MAP::iterator treenodes_map_iter;

   typename TREENODES_MAP::iterator treenode_iter;

   TREENODES_MAP* curr_treenodes_map_ptr;
   typename TREENODES_MAP::iterator curr_treenode_iter;

   TREENODES_MAP* leaf_treenodes_map_ptr;
   typename TREENODES_MAP::iterator leafnode_iter;

   bool destroy_treenodes_flag;
   int ID,next_unused_treenode_ID;
   treenode<T>* root_node_ptr;

   void allocate_member_objects();
   void generate_treenode_maps();
   void initialize_member_objects();
   void docopy(const tree<T>& t);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

#include "tree.cc"

#endif  // T_datastructures/tree.h






