// ==========================================================================
// Header file for templatized linkedlist class
// ==========================================================================
// Last modified on 9/23/05; 1/2/07; 1/6/07; 6/19/07; 4/4/14
// ==========================================================================

#ifndef T_LINKEDLIST_H
#define T_LINKEDLIST_H

#include <fstream>
#include <string>
#include <vector>
#include "datastructures/datapoint.h"

template <class T> class Mynode;

template <class T>
class Linkedlist
{
  public:
   
// Initialization, constructor and destructor functions:

   void allocate_member_objects();
   Linkedlist();
   Linkedlist(const Linkedlist<T>& l);
   Linkedlist<T>& operator= (const Linkedlist<T>& l);
   void purge_all_nodes(bool destructor_flag=false);
   virtual ~Linkedlist();

   template <class T1>
      friend std::ostream& operator<< (
         std::ostream& outstream,const Linkedlist<T1>& l);

// Set and get member functions:

   void set_start_ptr(Mynode<T>* startptr);
   void set_stop_ptr(Mynode<T>* stopptr);
   unsigned int size() const;
   const Mynode<T>* get_start_ptr() const;
   const Mynode<T>* get_stop_ptr() const;
   Mynode<T>* get_start_ptr();
   Mynode<T>* get_stop_ptr();
   const Mynode<T>* get_node_ahead(const Mynode<T>* currnode_ptr,int n) const;
   const Mynode<T>* get_node_behind(
      const Mynode<T>* currnode_ptr,int n) const;
   Mynode<T>* get_node_ahead(Mynode<T>* currnode_ptr,int n);
   Mynode<T>* get_node_behind(Mynode<T>* currnode_ptr,int n);

// Data retrieval member functions:

   Mynode<T>* data_in_list(const T& data) const;
   T get_data(int n) const;

// Node manipulation member functions:
   
   void insert_node_into_list(
      const int posn,const double ord,Mynode<T>* newnode_ptr);
   Mynode<T>* create_and_insert_node(
      const int posn,const double ord,T& data);
   Mynode<T>* create_and_insert_node(
      const int posn,const double ord,const T& data);

   Mynode<T>* append_node(T& data);
   Mynode<T>* append_node(const T& data);

   Mynode<T>* get_node(int posn) const;
   Mynode<T>* get_ID_node(int ID) const;
   void delete_node(int posn);
   void delete_node(Mynode<T>* node_ptr);
   void sort_nodes(double var[],bool decreasing_func);
   void sort_nodes(Mynode<T>* node_ptr[]);

   void concatenate(Linkedlist<T>* taillist_ptr);
   void concatenate_wo_duplication(Linkedlist<T>* taillist_ptr);
   std::vector<T>* convert_list_to_vector();
   void convert_vector_to_list(const std::vector<T>& V);

   bool overlapping_list(Linkedlist<T> const *list2_ptr) const;

  private: 

   int n_nodes,unique_ID_counter;
   Mynode<T>* start_ptr;
   Mynode<T>* stop_ptr;

   void initialize_member_objects();
   void docopy(const Linkedlist<T>& l);
};

typedef Linkedlist<datapoint> linkedlist;

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline void Linkedlist<T>::set_start_ptr(
Mynode<T>* startptr)
{
   start_ptr=startptr;
}

template <class T> inline void Linkedlist<T>::set_stop_ptr(Mynode<T>* stopptr)
{
   stop_ptr=stopptr;
}

template <class T> inline unsigned int Linkedlist<T>::size() const
{
   return n_nodes;
}

template <class T> inline const Mynode<T>* Linkedlist<T>::get_start_ptr() 
   const
{
   return start_ptr;
}

template <class T> inline const Mynode<T>* Linkedlist<T>::get_stop_ptr() const
{
   return stop_ptr;
}

template <class T> inline Mynode<T>* Linkedlist<T>::get_start_ptr()
{
   return start_ptr;
}

template <class T> inline Mynode<T>* Linkedlist<T>::get_stop_ptr()
{
   return stop_ptr;
}

// Methods get_node_ahead and get_node_behind take in a pointer to
// some current node within the linked list along with an integer
// nsteps.  They return pointers to the nodes which are ahead/behind
// the current node by nsteps nodes.  Both methods wrap around at the
// beginning and end of the linked list.

template <class T> inline Mynode<T>* Linkedlist<T>::get_node_ahead(
   Mynode<T>* currnode_ptr,int nsteps)
{
   Mynode<T>* ahead_node_ptr=currnode_ptr;
   for (int n=0; n<nsteps; n++)
   {
      ahead_node_ptr=ahead_node_ptr->get_nextptr();
      if (ahead_node_ptr==NULL) ahead_node_ptr=start_ptr;
   }
   return ahead_node_ptr;
}

template <class T> inline const Mynode<T>* Linkedlist<T>::get_node_ahead(
   const Mynode<T>* currnode_ptr,int nsteps) const
{
   const Mynode<T>* ahead_node_ptr=currnode_ptr;
   for (int n=0; n<nsteps; n++)
   {
      ahead_node_ptr=ahead_node_ptr->get_nextptr();
      if (ahead_node_ptr==NULL) ahead_node_ptr=start_ptr;
   }
   return ahead_node_ptr;
}

template <class T> inline Mynode<T>* Linkedlist<T>::get_node_behind(
   Mynode<T>* currnode_ptr,int nsteps)
{
   Mynode<T>* behind_node_ptr=currnode_ptr;
   for (int n=0; n<nsteps; n++)
   {
      behind_node_ptr=behind_node_ptr->get_prevptr();
      if (behind_node_ptr==NULL) behind_node_ptr=stop_ptr;
   }
   return behind_node_ptr;
}

template <class T> inline const Mynode<T>* Linkedlist<T>::get_node_behind(
   const Mynode<T>* currnode_ptr,int nsteps) const
{
   const Mynode<T>* behind_node_ptr=currnode_ptr;
   for (int n=0; n<nsteps; n++)
   {
      behind_node_ptr=behind_node_ptr->get_prevptr();
      if (behind_node_ptr==NULL) behind_node_ptr=stop_ptr;
   }
   return behind_node_ptr;
}

// ---------------------------------------------------------------------
// Member function create_and_insert_node dynamically generates a new
// node and inserts it into the linked list between nodes posn and
// posn+1.  

template <class T> inline Mynode<T>* Linkedlist<T>::create_and_insert_node(
   const int posn,const double ord,T& data)
{
   Mynode<T>* newnode_ptr=new Mynode<T>(data);
   insert_node_into_list(posn,ord,newnode_ptr);
   return newnode_ptr;
}

template <class T> inline Mynode<T>* Linkedlist<T>::create_and_insert_node(
   const int posn,const double ord,const T& data)
{
   Mynode<T>* newnode_ptr=new Mynode<T>(data);
   insert_node_into_list(posn,ord,newnode_ptr);
   return newnode_ptr;
}

// ---------------------------------------------------------------------
// Member function append_node tacks on a node to the end of the
// current linked list object:

template <class T> inline Mynode<T>* Linkedlist<T>::append_node(T& data)
{
   Mynode<T>* newnode_ptr=new Mynode<T>(data);
   insert_node_into_list(n_nodes,static_cast<double>(n_nodes),newnode_ptr);
   return newnode_ptr;
}

template <class T> inline Mynode<T>* Linkedlist<T>::append_node(const T& data)
{
   Mynode<T> *newnode_ptr=new Mynode<T>(data);
   insert_node_into_list(n_nodes,static_cast<double>(n_nodes),newnode_ptr);
   return newnode_ptr;
}

template <class T> inline void Linkedlist<T>::delete_node(int posn)
{
   delete_node(get_node(posn));
}

#include "Linkedlist.cc"

#endif  // T_datastructures/Linkedlist.h






