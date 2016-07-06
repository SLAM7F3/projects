// =========================================================================
// Header file for stand-alone container functions.
// =========================================================================
// Last modified on 12/15/05
// =========================================================================

#ifndef CONTAINERFUNCS_H
#define CONTAINERFUNCS_H

#include <vector>
#include "datastructures/datapoint.h"

template <class T> class Mynode;
typedef Mynode<datapoint> mynode;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;

namespace containerfunc
{

// STL vector methods:

   template <class T> int locate_vector_index(
      const std::vector<T>& V,const T& value);
   template <class T> void remove_duplicate_entries(std::vector<T>& V);

// Linked list methods:

   void scale_variable_values(linkedlist* list_ptr,int varnumber,double a);
   void scale_node_func_values(linkedlist* list_ptr,double a);

/*
//   void find_max_min_vars(linkedlist* list_ptr);
   void find_max_min_func_values(linkedlist* list_ptr);
   void find_max_min_func_values(
      linkedlist* list_ptr,double var_lo,double var_hi);
   void find_max_min_func_values(linkedlist* list_ptr,int func_number);
   void find_max_min_func_values(
      linkedlist* list_ptr,int func_number,double var_lo,double var_hi);
   void find_max_min_func_values(
      linkedlist* list_ptr,double var_maximum[],double var_minimum[]);
   void find_max_min_func_values(
      linkedlist* list_ptr,int func_number,
      double var_maximum[],double var_minimum[]);
   void find_max_min_func_values(
      linkedlist* list_ptr,int func_number,double var_lo,double var_hi,
      double var_maximum[],double var_minimum[]);
   void find_max_min_values_of_func_plus_errors(linkedlist* list_ptr);
*/

   void sort_nodes_by_indep_var(
      linkedlist* list_ptr,bool decreasing_func=false);
   void sort_nodes_by_depend_var(
      linkedlist* list_ptr,int d,bool decreasing_func=false);

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Method locate_vector_index performs a brute-force search over the
// values stored within input STL vector V.  It returns the index of
// the first entry in V which matches the input value.  If no match is
// found, this method return -1.  This specialized method should only
// really be used for vectors of integers where operator== is
// well-defined.

   template <class T> 
      inline int locate_vector_index(const std::vector<T>& V,const T& value)
   {
      for (unsigned int n=0; n<V.size(); n++)
      {
         if (V[n]==value) return n;
      }
      return -1;
   }

   template <class T> 
      inline void remove_duplicate_entries(std::vector<T>& V)
      {
         Linkedlist<T>* taillist_ptr=new Linkedlist<T>;
         taillist_ptr->convert_vector_to_list(V);

         Linkedlist<T>* headlist_ptr=new Linkedlist<T>;
         headlist_ptr->concatenate_wo_duplication(taillist_ptr);
         std::vector<T>* V_ptr=headlist_ptr->convert_list_to_vector();
         delete taillist_ptr;
         delete headlist_ptr;

         V.clear();
         for (unsigned int n=0; n<V_ptr->size(); n++)
         {
            V.push_back((*V_ptr)[n]);
         }
         delete V_ptr;
      }
   
}



#endif // datastructures/containerfuncs.h



