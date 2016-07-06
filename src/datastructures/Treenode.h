// ==========================================================================
// Header file for templatized Treenode class
// ==========================================================================
// Last modified on 11/7/06; 11/8/06
// ==========================================================================

#ifndef T_TREENODE_H
#define T_TREENODE_H

#include "datastructures/Linkedlist.h"
#include "pool/objpool.h"
#include "math/threevector.h"

template <class T>
class Treenode: public ObjectPool< Treenode<T> >
{

  public:

// Initialization, constructor and destructor functions:

   Treenode();
   Treenode(int id,int l,int cc);
   Treenode(const Treenode<T>& node);
   virtual ~Treenode();
   Treenode<T>& operator= (const Treenode<T>& node);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Treenode<T1>& node);

// Set and get member functions:
   
   void set_ID(int id);
   int get_ID() const;
   void set_level(int l);
   int get_level() const;
   void set_column(int c);
   int get_column() const;
   int get_own_child_index() const;
   std::vector<int>& get_total_indices();
   const std::vector<int>& get_total_indices() const;

   void set_data(T const & d);
   T& get_data(); 
   const T& get_data() const; 

   void set_posn(const threevector& p);
   threevector& get_posn();
   const threevector& get_posn() const;

   Linkedlist< Treenode<T>* >* get_Parents_list_ptr();
   Linkedlist< Treenode<T>* >* get_Children_list_ptr();

// Parents & children manipulation member functions:

   int getNumParents() const;
   int getNumChildren() const;
   Treenode<T>* addChild(int id);

  private:

   int ID,level,column,own_child_index,offspring_counter;
   T data;		// Arbitrary data object
   std::vector<int> total_indices; // Essentially holds nodepath info
   threevector posn;

   Linkedlist< Treenode<T>* >* Parents_list_ptr;
   Linkedlist< Treenode<T>* >* Children_list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Treenode<T>& node);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline void Treenode<T>::set_ID(int id)
{
   ID=id;
}

template <class T> inline int Treenode<T>::get_ID() const
{
   return ID;
}

template <class T> inline void Treenode<T>::set_level(int l)
{
   level=l;
}

template <class T> inline int Treenode<T>::get_level() const
{
   return level;
}

template <class T> inline void Treenode<T>::set_column(int c)
{
   column=c;
}

template <class T> inline int Treenode<T>::get_column() const
{
   return column;
}

template <class T> inline int Treenode<T>::get_own_child_index() const
{
   return own_child_index;
}

template <class T> inline std::vector<int>& Treenode<T>::get_total_indices() 
{
   return total_indices;
}

template <class T> inline const std::vector<int>& 
Treenode<T>::get_total_indices() const
{
   return total_indices;
}

template <class T> inline void Treenode<T>::set_data(T const & d)
{
   data=d;
}

template <class T> inline T& Treenode<T>::get_data() 
{
   return data;
}

template <class T> inline const T& Treenode<T>::get_data() const
{
   return data;
}

template <class T> inline void Treenode<T>::set_posn(const threevector& p)
{
   posn=p;
}

template <class T> inline threevector& Treenode<T>::get_posn()
{
   return posn;
}

template <class T> inline const threevector& Treenode<T>::get_posn() const
{
   return posn;
}

template <class T> inline Linkedlist< Treenode<T>* >* 
Treenode<T>::get_Parents_list_ptr()
{
   return Parents_list_ptr;
}

template <class T> inline Linkedlist< Treenode<T>* >* 
Treenode<T>::get_Children_list_ptr()
{
   return Children_list_ptr;
}

#include "datastructures/Treenode.cc"

#endif  // T_datastructures/Treenode.h



