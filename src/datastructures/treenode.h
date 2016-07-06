// ==========================================================================
// Header file for templatized treenode class
// ==========================================================================
// Last modified on 7/16/12; 7/18/12; 7/28/12
// ==========================================================================

#ifndef T_SMALLTREENODE_H
#define T_SMALLTREENODE_H

#include <map>
#include "pool/objpool.h"

template <class T>
class treenode: public ObjectPool< treenode<T> >
{

  public:

   typedef std::map<int,treenode<T>* > TREENODES_MAP;
// Indep var: treenode integer ID
// Depend var: pointer to treenode

// Initialization, constructor and destructor functions:

   treenode();
   treenode(int id,int level);
   treenode(const treenode<T>& node);
   virtual ~treenode();
   treenode<T>& operator= (const treenode<T>& node);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream, treenode<T1>& node);

// Set and get member functions:

   void set_previously_visited_flag(bool flag);
   bool get_previously_visited_flag() const;
   void set_ID(int id);
   int get_ID() const;
   void set_level(int l);
   int get_level() const;
   void set_rank(int r);
   int get_rank() const;
   void set_data_ptr(T const & d);
   T& get_data_ptr(); 
   const T& get_data_ptr() const; 

// Parent member functions:

   void set_parent_node_ptr(treenode<T>* parent_node_ptr);
   treenode<T>* get_parent_node_ptr();
   const treenode<T>* get_parent_node_ptr() const;

// Children member functions:

   bool isChild(treenode<T>* child_node_ptr);
   void addChild(treenode<T>* child_node_ptr);
   void deleteChild(treenode<T>* child_node_ptr);
   int getNumChildren() const;
   bool is_leaf_flag() const;
   int get_n_siblings() const;

   std::vector<treenode<T>* > getChildrenNodePtrs();
   TREENODES_MAP* get_child_nodes_map_ptr()
   {
      return child_nodes_map_ptr;
   }

   const TREENODES_MAP* get_child_nodes_map_ptr() const
   {
      return child_nodes_map_ptr;
   }

   treenode<T>* reset_child_treenode_ptr();
   treenode<T>* get_next_child_treenode_ptr();
   treenode<T>* get_rootnode_ptr();


  private:

   bool previously_visited_flag;
   int ID,level,rank;

// As of 7/4/12, we assume the data_ptr member is a pointer to a
// dynamically instantiated object.  When we destroy this treenode, we
// will call the destructor for the data object:

   T data_ptr;		

   treenode<T>* parent_node_ptr;
   TREENODES_MAP* child_nodes_map_ptr;
   typename std::map<int,treenode<T>* >::iterator child_treenode_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const treenode<T>& node);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline void treenode<T>::set_previously_visited_flag(
   bool flag)
{
   previously_visited_flag=flag;
}

template <class T> inline bool treenode<T>::get_previously_visited_flag() 
const
{
   return previously_visited_flag;
}

template <class T> inline void treenode<T>::set_ID(int id)
{
   ID=id;
}

template <class T> inline int treenode<T>::get_ID() const
{
   return ID;
}

template <class T> inline void treenode<T>::set_level(int l)
{
   level=l;
}

template <class T> inline int treenode<T>::get_level() const
{
   return level;
}

template <class T> inline void treenode<T>::set_rank(int r)
{
   rank=r;
}

template <class T> inline int treenode<T>::get_rank() const
{
   return rank;
}


template <class T> inline void treenode<T>::set_data_ptr(T const & d)
{
   data_ptr=d;
}

template <class T> inline T& treenode<T>::get_data_ptr() 
{
   return data_ptr;
}

template <class T> inline const T& treenode<T>::get_data_ptr() const
{
   return data_ptr;
}

#include "datastructures/treenode.cc"

#endif  // T_datastructures/treenode.h



