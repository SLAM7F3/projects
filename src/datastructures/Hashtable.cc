// ==========================================================================
// Templatized Hashtable class member function definitions
// ==========================================================================
// Last modified on 7/8/05; 2/23/06; 1/14/10; 4/5/14
// ==========================================================================

#include <iostream>
#include "datastructures/Mynode.h"

// The following parameters which enter into our random hash function
// are supposed to be appropriate for a 32-bit machine.  See footnote
// on page 486 of Chapter 9 in "C++: An introduction to data
// structures" by Larry Nyhoff.

template <class T> const int Hashtable<T>::multiplier=25173;
template <class T> const int Hashtable<T>::addend=13849;
template <class T> const int Hashtable<T>::modulus=65536;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

template <class T> void Hashtable<T>::allocate_member_objects() 
{
   list_ptr=new Linkedlist<T>*[capacity];

   for (unsigned int i=0; i<capacity; i++)
   {
      list_ptr[i]=new Linkedlist<T>();
   }
}

template <class T> Hashtable<T>::Hashtable(int c)
{
   capacity=c;
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

template <class T> Hashtable<T>::Hashtable(const Hashtable<T>& h)
{
   capacity=h.capacity;
   allocate_member_objects();
   initialize_member_objects();
   docopy(h);
}

// ---------------------------------------------------------------------
// Member function purge_all_entries resets all of the linked lists to
// their initial empty states.  But it does not destroy the
// dynamically generated lists.  The destructor, on the other hand,
// deletes all the lists.

template <class T> void Hashtable<T>::purge_all_entries()
{
   for (unsigned int i=0; i<capacity; i++)
   {
      list_ptr[i]->purge_all_nodes();
   }
   nkeys_in_table=0;
}

// ---------------------------------------------------------------------
// Member function copy_hashtable takes in an input and an output
// hashtable pointer.  If the output hashtable pointer is NULL, this
// method dynamically generates the output hashtable and copies the
// input hashtable's contents onto it.  If the output hashtable
// already exists, this method copies the input hashtable's contents
// onto it.

template <class T> void Hashtable<T>::copy_hashtable(
   Hashtable<T> const *input_hashtable_ptr,
   Hashtable<T>*& output_hashtable_ptr)
{
   if (input_hashtable_ptr != NULL)
   {
      if (output_hashtable_ptr==NULL)
      {
         output_hashtable_ptr=new Hashtable<T>(*input_hashtable_ptr);
      }
      else
      {
         *output_hashtable_ptr=*input_hashtable_ptr;
      }
   }
}

// ---------------------------------------------------------------------
template <class T> Hashtable<T>::~Hashtable()
{
   for (unsigned int i=0; i<capacity; i++)
   {
      delete list_ptr[i];
   }
   delete [] list_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Hashtable<T>& h)
{
   outstream << std::endl;
   outstream << "Number of keys in hash table = "
             << h.nkeys_in_table << std::endl;
   for (unsigned int n=0; n<h.capacity; n++)
   {
      if (h.list_ptr[n]->size() > 0)
      {
         outstream << "Linked list number = " << n << std::endl;
         outstream << *(h.list_ptr[n]) << std::endl;
      }
   }
   return outstream;
}

// ---------------------------------------------------------------------
// Member function convert_hashtable_to_vector loops over all entries
// within the hashtable and copies pointers to its linked list's
// nodes' data within an output STL vector.

template <class T> std::vector<T>*
Hashtable<T>::convert_hashtable_to_vector() const
{
   std::vector<T>* V_ptr=new std::vector<T>;
   V_ptr->reserve(size());
   for (unsigned int i=0; i<capacity; i++)
   {
      Mynode<T>* currnode_ptr=list_ptr[i]->get_start_ptr();
      while (currnode_ptr != NULL)
      {
         V_ptr->push_back(currnode_ptr->get_data());
         currnode_ptr=currnode_ptr->get_nextptr();
      }
   } // loop over index i labeling linked list in hashtable
   return V_ptr;
}

// ---------------------------------------------------------------------
// Member function display_dereferenced_contents systematically visits
// every non-empty node within the hashtable.  Each node is assumed to
// contain a pointer to some piece of data.  This method dereferences
// and prints out the data to output filestream hashstream.  To print
// output to stdout, simply chant "display_derefenced_contents(cout)".

template <class T> void Hashtable<T>::display_dereferenced_contents(
   std::ostream& hashstream) const
{
   for (unsigned int i=0; i<capacity; i++)
   {
      Mynode<T>* currnode_ptr=list_ptr[i]->get_start_ptr();
      while(currnode_ptr != NULL)
      {
         hashstream << (currnode_ptr->get_data()) << std::endl;
//         hashstream << *(currnode_ptr->get_data()) << std::endl;
         currnode_ptr=currnode_ptr->get_nextptr();
      }
   } // loop over index i labeling linked list in hash table
}

// ---------------------------------------------------------------------
template <class T> void Hashtable<T>::print_collision_table()
{
   std::cout << "Hashtable capacity = " << capacity << std::endl;
   for (unsigned int n=0; n<capacity; n++)
   {
      int n_nodes=list_ptr[n]->size();
//      if (n_nodes > 1)
      {
         std::cout << "List n = " << n << " has "
                   << list_ptr[n]->size() << " nodes" << std::endl;
      }
   }
}

// ==========================================================================
// Key insertion, retrieval and deletion member functions
// ==========================================================================

// Member function insert_key computes the location of input integer
// key within the current Hashtable object.  It inserts the key into
// the table and returns a pointer to the node which was dynamically
// generated to hold the key.

template <class T> Mynode<T>* Hashtable<T>::insert_key(int key,T data)
{
   int location=compute_location(key);
   list_ptr[location]->append_node(data);
   list_ptr[location]->get_stop_ptr()->set_ID(key);
   nkeys_in_table++;
   return list_ptr[location]->get_stop_ptr();
}

template <class T> Mynode<T>* Hashtable<T>::insert_key(
   int key,int location,T data)
{
   list_ptr[location]->append_node(data);
   list_ptr[location]->get_stop_ptr()->set_ID(key);
   nkeys_in_table++;
   return list_ptr[location]->get_stop_ptr();
}

// ---------------------------------------------------------------------
// Member function update_key overwrites the data contents of a
// hashtable entry if it exists.  Otherwise, it inserts the data into
// the hashtable.

template <class T> Mynode<T>* Hashtable<T>::update_key(int key,T data)
{
   Mynode<T>* currnode_ptr=retrieve_key(key);
   if (currnode_ptr==NULL)
   {
      currnode_ptr=insert_key(key,data);
   }
   else
   {
      currnode_ptr->set_data(data);
   }
   return currnode_ptr;
}

// ---------------------------------------------------------------------
// Member function retrieve_key returns a pointer to the node
// containing key as its independent variable if it exists within the
// hash table.  Otherwise, this method returns a NULL pointer.

template <class T> Mynode<T>* Hashtable<T>::retrieve_key(
   int key,int& location)
{
   location=compute_location(key);
   Mynode<T>* currnode_ptr=list_ptr[location]->get_start_ptr();

   while (currnode_ptr != NULL)
   {
      if (currnode_ptr->get_ID()==key) return currnode_ptr;
      currnode_ptr=currnode_ptr->get_nextptr();
   }
   return currnode_ptr;
}

template <class T> const Mynode<T>* Hashtable<T>::retrieve_key(
   int key,int& location) const
{
   location=compute_location(key);
   Mynode<T>* currnode_ptr=list_ptr[location]->get_start_ptr();

   while (currnode_ptr != NULL)
   {
      if (currnode_ptr->get_ID()==key) return currnode_ptr;
      currnode_ptr=currnode_ptr->get_nextptr();
   }
   return currnode_ptr;
}

// ---------------------------------------------------------------------
template <class T> void Hashtable<T>::delete_key(int key)
{
   int location;
   Mynode<T>* keynode_ptr=retrieve_key(key,location);
   if (keynode_ptr != NULL)
   {
      list_ptr[location]->delete_node(keynode_ptr);
      nkeys_in_table--;
   }
}

// ---------------------------------------------------------------------
template <class T> void Hashtable<T>::delete_node(
   int location,Mynode<T>* node_ptr)
{
   if (node_ptr != NULL)
   {
      list_ptr[location]->delete_node(node_ptr);
      nkeys_in_table--;
   }
}

// ---------------------------------------------------------------------
// Member function data_in_hashtable systematically visits every
// non-empty node within the hashtable and compares its data with the
// input data.  If a match is found, this method returns a pointer to
// the node containing the matching data.  Otherwise, it returns NULL.

template <class T> Mynode<T>* Hashtable<T>::data_in_hashtable(
   const T& data) const
{
   Mynode<T>* datanode_ptr=NULL;
   for (unsigned int i=0; i<capacity; i++)
   {
      datanode_ptr=list_ptr[i]->data_in_list(data);
      if (datanode_ptr != NULL) break;
   } 
   return datanode_ptr;
}

// ---------------------------------------------------------------------
// Boolean member function overlapping_table loops over every linked
// list within the current hashtable as well as in input hashtable
// *table2_ptr.  It returns true if the address of any node in the
// first hashtable matches that in the second.  This method can be
// used to search for memory clobbers.

template <class T> bool Hashtable<T>::overlapping_table(
   Hashtable<T> const *table2_ptr) const
{
   bool overlap_flag=false;
   for (unsigned int i=0; i<capacity; i++)
   {
      for (unsigned int j=0; j<table2_ptr->get_table_capacity(); j++)
      {
         if (list_ptr[i]->overlapping_list(table2_ptr->get_list_ptr(j)))
         {
            overlap_flag=true;
         }
      } // loop over index j
   } // loop over index i
   return overlap_flag;
}

// ==========================================================================
// Data manipulation member functions
// ==========================================================================

// Member function explicitly_count_entries systematically counts
// every non-empty node within the hashtable.  

template <class T> int Hashtable<T>::explicitly_count_entries() const
{
   int n_entries=0;
   for (unsigned int i=0; i<capacity; i++)
   {
      Mynode<T>* currnode_ptr=list_ptr[i]->get_start_ptr();
      while(currnode_ptr != NULL)
      {
         n_entries++;
         currnode_ptr=currnode_ptr->get_nextptr();
      }
   } // loop over index i labeling linked list in hash table
   return n_entries;
}

// ---------------------------------------------------------------------
// Member functions max_data_value and min_data_value scan through the
// hashtable and returns the maximum data value.  These methods are
// intended to be used for primitive data types such as integers,
// floats and doubles where operator> is well-definied.

template <class T> T Hashtable<T>::max_data_value() const
{
   bool first_value=true;
   T max_value;
   for (unsigned int i=0; i<capacity; i++)
   {
      Mynode<T>* currnode_ptr=list_ptr[i]->get_start_ptr();
      while(currnode_ptr != NULL)
      {
         T curr_value=currnode_ptr->get_data();
         if (first_value)
         {
            first_value=false;
            max_value=curr_value;
         }
         else
         {
            if (curr_value > max_value) max_value=curr_value;
         }
         currnode_ptr=currnode_ptr->get_nextptr();
      }
   } // loop over index i labeling linked list in hash table
   return max_value;
}

template <class T> T Hashtable<T>::min_data_value() const
{
   bool first_value=true;
   T min_value;
   for (unsigned int i=0; i<capacity; i++)
   {
      Mynode<T>* currnode_ptr=list_ptr[i]->get_start_ptr();
      while(currnode_ptr != NULL)
      {
         T curr_value=currnode_ptr->get_data();
         if (first_value)
         {
            first_value=false;
            min_value=curr_value;
         }
         else
         {
            if (curr_value < min_value) min_value=curr_value;
         }
         currnode_ptr=currnode_ptr->get_nextptr();
      }
   } // loop over index i labeling linked list in hash table
   return min_value;
}

// Note added on 5/22/05: Method generate_data_histogram appears to be
// the only one within Hashtable.cc which makes any reference to
// libmath.a In order to disentangle libdatastructures.a from
// libmath.a, we comment out this method below which does not appear
// to be called by any other method.

/*

// ---------------------------------------------------------------------
// Member function generate_data_histogram creates a frequency
// histogram for its data members (which are assumed to be primitive
// integers, floats, doubles, etc).  Metafile output is generated by
// this method.

template <class T> double Hashtable<T>::generate_data_histogram(
   std::string metafilename_str,double cumulative_percentile) const
{
// First scan through hashtable entries and fill data_value array:

   int n_entry=0;
   double* data_value=new double[2*nkeys_in_table];

   for (unsigned int n=0; n<capacity; n++)
   {
      if (list_ptr[n]->size() > 0)
      {
         Mynode<T>* currnode_ptr=list_ptr[n]->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            data_value[n_entry++]=static_cast<double>(
               currnode_ptr->get_data());
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }
   } // loop over index n labeling hashtable's linked lists

   prob_distribution prob(n_entry,data_value,100);
   prob.xmin=-0.1;
//   prob.xmax=100;
//   prob.xtic=20;
//   prob.xsubtic=10;
   prob.freq_histogram=true;
   prob.densityfilenamestr=metafilename_str;
   prob.xlabel="Data Value";
//   prob.write_density_dist();
   delete [] data_value;

   return prob.find_x_corresponding_to_pcum(cumulative_percentile);
}
*/

// ---------------------------------------------------------------------
// Member function shift_data_values add input alpha to all data
// entries within the hashtable.  This method is intended to be
// applied to Hashtables containing only primitive integer, float and
// double types.

template <class T> void Hashtable<T>::shift_data_values(T alpha) 
{
   for (unsigned int n=0; n<capacity; n++)
   {
      if (list_ptr[n]->size() > 0)
      {
         Mynode<T>* currnode_ptr=list_ptr[n]->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            currnode_ptr->set_data(alpha+currnode_ptr->get_data());
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }
   } // loop over index n labeling hashtable's linked lists
}

// ---------------------------------------------------------------------
// Member function rescale_data_values multiplies all data entries
// within the hashtable by input scale factor alpha.  This method is
// intended to be applied to Hashtables containing only primitive
// integer, float and double types.

template <class T> void Hashtable<T>::rescale_data_values(T alpha) 
{
   for (unsigned int n=0; n<capacity; n++)
   {
      if (list_ptr[n]->size() > 0)
      {
         Mynode<T>* currnode_ptr=list_ptr[n]->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            currnode_ptr->set_data(alpha*currnode_ptr->get_data());
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }
   } // loop over index n labeling hashtable's linked lists
}





