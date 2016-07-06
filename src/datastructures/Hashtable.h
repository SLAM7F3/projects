// ==========================================================================
// Header file for templatized hashtable class
// ==========================================================================
// Last modified on 6/19/07; 1/14/10; 11/9/11; 4/5/14
// ==========================================================================

#ifndef T_HASHTABLE_H
#define T_HASHTABLE_H

#include <fstream>
#include "datastructures/Linkedlist.h"

template <class T>
class Hashtable
{

  public:
   
// Initialization, constructor and destructor functions:

   Hashtable(int c);
   Hashtable(const Hashtable<T>& h);
   void purge_all_entries();
   void copy_hashtable(
      Hashtable<T> const *input_hashtable_ptr,
      Hashtable<T>*& output_hashtable_ptr);
   virtual ~Hashtable();
   Hashtable<T>& operator= (const Hashtable<T>& h);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Hashtable<T1>& h);

   std::vector<T>* convert_hashtable_to_vector() const;
   void display_dereferenced_contents() const;
   void display_dereferenced_contents(std::ostream& hashstream) const;
   void print_collision_table();

// Set and get member functions:

   void set_table_capacity(unsigned int c);
   unsigned int size() const;
   unsigned int get_table_capacity() const;

   Linkedlist<T>* get_list_ptr(int n);
   const Linkedlist<T>* get_list_ptr(int n) const;

// Key insertion, retrieval and deletion member functions:

   Mynode<T>* insert_key(int key,T data);
   Mynode<T>* insert_key(int key,int location,T data);
   Mynode<T>* update_key(int key,T data);
   Mynode<T>* retrieve_key(int key);
   const Mynode<T>* retrieve_key(int key) const;
   Mynode<T>* retrieve_key(int key,int& location);
   const Mynode<T>* retrieve_key(int key,int& location) const;
   Mynode<T>* increment_key(int key);
   void delete_key(int key);
   void delete_node(int location,Mynode<T>* node_ptr);
   void decrement_nkeys_in_table();
   Mynode<T>* data_in_hashtable(const T& data) const;
   bool overlapping_table(Hashtable<T> const *table2_ptr) const;

// Data manipulation member functions:

   int explicitly_count_entries() const;
   T max_data_value() const;
   T min_data_value() const;
//   double generate_data_histogram(
//      std::string metafilename_str,double cumulative_percentile=0.1) const;
   void shift_data_values(T alpha);
   void rescale_data_values(T alpha);

  private: 

   static const int multiplier;
   static const int addend;
   static const int modulus;
   
   unsigned int nkeys_in_table;
   unsigned int capacity;  // Number of linked lists within hashtable array

// Hash table contains an array of pointers to linked lists:

   Linkedlist<T> **list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Hashtable<T>& h);

   int compute_location(int key) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

template <class T> inline void Hashtable<T>::initialize_member_objects() 
{
   nkeys_in_table=0;
}

// ---------------------------------------------------------------------
template <class T> inline void Hashtable<T>::docopy(const Hashtable<T>& h)
{
   nkeys_in_table=h.nkeys_in_table;
   for (unsigned int n=0; n<capacity; n++)
   {
      if (h.list_ptr[n] != NULL)
      {
         *(list_ptr[n])=*(h.list_ptr[n]);
      }
   }
}	

// ---------------------------------------------------------------------
// Overload = operator:

template <class T> inline Hashtable<T>& Hashtable<T>::operator= 
(const Hashtable<T>& h)
{
   if (this==&h) return *this;
   docopy(h);
   return *this;
}

// ---------------------------------------------------------------------
template <class T> inline void Hashtable<T>::set_table_capacity(unsigned int c)
{
   capacity=c;
}

// ---------------------------------------------------------------------
template <class T> inline unsigned int Hashtable<T>::size() const
{
   return nkeys_in_table;
}

// ---------------------------------------------------------------------
template <class T> inline unsigned int Hashtable<T>::get_table_capacity() const
{
   return capacity;
}

// ---------------------------------------------------------------------
template <class T> inline Linkedlist<T>* Hashtable<T>::get_list_ptr(int n) 
{
   return list_ptr[n];
}

template <class T> inline const Linkedlist<T>* Hashtable<T>::get_list_ptr(int n) const
{
   return list_ptr[n];
}

// ---------------------------------------------------------------------
// Member function compute_location takes in an integer key and
// returns a non-negative valued location within the linkedlist array
// where the key should be stored.  This method implements the hash
// function.

template <class T> inline int Hashtable<T>::compute_location(int key) const
{
   
// Guarantee non-negative valued integer by working with an unsigned int!

   unsigned int randomint=((multiplier*key)+addend)%modulus;
   return static_cast<int>((randomint%capacity));
}

// ---------------------------------------------------------------------
// Member function retrieve_key returns a pointer to the node
// containing key as its independent variable if it exists within the
// hash table.  Otherwise, this method returns a NULL pointer.

template <class T> inline Mynode<T>* Hashtable<T>::retrieve_key(int key) 
{
   int location;
   return retrieve_key(key,location);
}

template <class T> inline const Mynode<T>* Hashtable<T>::retrieve_key(int key)
   const
{
   int location;
   return retrieve_key(key,location);
}

// ---------------------------------------------------------------------
// Member function increment_key assumes that the hashtable's data
// corresponds to primitive integers, floats or doubles.  If the input
// key corresponds to an existing node, this method increments its
// datum value by one.  Otherwise, it creates the node with a unit
// valued datum.

template <class T> inline Mynode<T>* Hashtable<T>::increment_key(int key)
{
   Mynode<T>* currnode_ptr=retrieve_key(key);
   if (currnode_ptr==NULL)
   {
      currnode_ptr=insert_key(key,1);
   }
   else
   {
      currnode_ptr->set_data(currnode_ptr->get_data()+1);
   }
   return currnode_ptr;
}

// ---------------------------------------------------------------------
template <class T> inline void Hashtable<T>::decrement_nkeys_in_table() 
{
   nkeys_in_table--;
}

#include "Hashtable.cc"

#endif  // T_datastructures/Hashtable.h





