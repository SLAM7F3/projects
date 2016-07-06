// ==========================================================================
// Header file for map_unionfind class
// ==========================================================================
// Last modified on 7/8/13; 7/10/13; 7/11/13; 8/10/13
// ==========================================================================

#ifndef MAP_UNIONFIND_H
#define MAP_UNIONFIND_H

#include <map>
#include <set>
#include "math/ltduple.h"
#include "datastructures/Triple.h"

class map_unionfind
{

  public:

// DUPLE.first = image number
// DUPLE.second = feature number

   typedef Triple<DUPLE,int,void*> NODE;

// NODE DUPLE var 1 = parent ID
// NODE int var 2 = rank
// NODE void* var 3 = pointer to some data object

   typedef std::map<DUPLE,NODE*> NODES_MAP;

// independent DUPLE = node_ID formed from image number and feature label
// dependent NODE* = pointer to node labeled by node_ID

   typedef std::map<void*,int> DATA_FOR_SET_MAP;

// independent void* var: pointer to some data object
// dependent int var: data label

   map_unionfind();
   map_unionfind(const map_unionfind& uf);
   ~map_unionfind();

   map_unionfind& operator= (const map_unionfind& uf);
   friend std::ostream& operator<< 
      (std::ostream& outstream,map_unionfind& uf);

// Set and get member functions:

   NODE* get_node_ptr(DUPLE node_ID);
   NODES_MAP* get_nodes_map_ptr();
   const NODES_MAP* get_nodes_map_ptr() const;

   DUPLE form_node_ID(int frame_ID,int feature_ID) const;
   
   int get_n_nodes() const;
   void set_parent_ID(DUPLE node_ID,DUPLE parent_ID);
   DUPLE get_parent_ID(DUPLE node_ID);
   void set_rank(DUPLE node_ID,int r);
   int get_rank(DUPLE node_ID);

   bool set_data_ptr(DUPLE node_ID,void* data_ptr);
   void set_data_label(void* data_ptr,int data_label);
   void set_data_label(DUPLE node_ID,int data_label);
   void* get_data_ptr(DUPLE node_ID);
   void* get_root_data_ptr(DUPLE node_ID);

// Union-find algorithm member functions:

   NODE* MakeSet(DUPLE node_ID);
   NODE* CreateNode(DUPLE node_ID,DUPLE parent_node_ID);
   bool ElementOf(DUPLE node_ID);
   DUPLE Find(DUPLE node_ID);
   DUPLE Link(DUPLE node1_ID,DUPLE node2_ID);
   void purgeNodes();

  private: 

   NODES_MAP* nodes_map_ptr;
   NODES_MAP::iterator node_iter;

// *data_for_set_map_ptr is only to be used for destroying all data
// objects specified by void* pointers!  It serves no purpose other
// than memory management.

   DATA_FOR_SET_MAP* data_for_set_map_ptr; 
   DATA_FOR_SET_MAP::iterator data_for_set_map_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const map_unionfind& uf);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline map_unionfind::NODES_MAP* map_unionfind::get_nodes_map_ptr()
{
   return nodes_map_ptr;
}

inline const map_unionfind::NODES_MAP* 
map_unionfind::get_nodes_map_ptr() const
{
   return nodes_map_ptr;
}

#endif  // map_unionfind.h
