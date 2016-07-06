// ==========================================================================
// Header file for vector_union_find class
// ==========================================================================
// Last modified on 7/25/12; 7/28/12; 4/3/14
// ==========================================================================

#ifndef VECTOR_UNIONFIND_H
#define VECTOR_UNIONFIND_H

#include <map>
#include <vector>
#include "datastructures/Triple.h"

class vector_union_find
{

  public:

   typedef Triple<int,int,void*> NODE;
   typedef std::vector<NODE> NODES_VECTOR;

// independent int var: node_ID
// dependent int var 1: parent_ID
// dependent int var 2: rank
// dependent void* var 3: pointer to some data object

   typedef std::map<void*,int> DATA_FOR_SET_MAP;

// independent void* var: pointer to some data object
// dependent int var: data label

   vector_union_find();
   vector_union_find(const vector_union_find& uf);
   ~vector_union_find();

   vector_union_find& operator= (const vector_union_find& uf);
   friend std::ostream& operator<< 
      (std::ostream& outstream,vector_union_find& uf);

// Set and get member functions:

   unsigned int get_n_nodes() const;
   void set_parent_ID(int node_ID,int parent_ID);
   int get_parent_ID(int node_ID);
   void set_rank(int node_ID,int r);
   int get_rank(int node_ID);

   void set_data_ptr(int node_ID,void* data_ptr);
   void set_data_label(void* data_ptr,int data_label);
   void set_data_label(int node_ID,int data_label);
   void* get_data_ptr(int node_ID);
   void* get_root_data_ptr(int node_ID);

// Union-find algorithm member functions:

   void initializeNodes(int n);
   void MakeSet(int node_ID);
   void CreateNode(int node_ID,int parent_node_ID);
   int Find(int node_ID);
   int Link(int node1_ID,int node2_ID);
   void purgeNodes();

  private: 

   NODES_VECTOR* nodes_vector_ptr;

// *data_for_set_map_ptr is only to be used for destroying all data
// objects specified by void* pointers!  It serves no purpose other
// than memory management.

   DATA_FOR_SET_MAP* data_for_set_map_ptr; 
   DATA_FOR_SET_MAP::iterator data_for_set_map_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const vector_union_find& uf);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int vector_union_find::get_n_nodes() const
{
   return nodes_vector_ptr->size();
}

inline void vector_union_find::set_parent_ID(int node_ID,int parent_ID)
{
   nodes_vector_ptr->at(node_ID).first=parent_ID;
}

inline int vector_union_find::get_parent_ID(int node_ID)
{
   return nodes_vector_ptr->at(node_ID).first;
}

inline void vector_union_find::set_rank(int node_ID,int rank)
{
   nodes_vector_ptr->at(node_ID).second=rank;
}

inline int vector_union_find::get_rank(int node_ID)
{
   return nodes_vector_ptr->at(node_ID).second;
}


#endif  // vector_union_find.h
