// ==========================================================================
// Header file for templatized Mynode class
// ==========================================================================
// Last modified on 6/7/04; 6/19/07
// ==========================================================================

#ifndef T_MYNODE_H
#define T_MYNODE_H

#include "pool/objpool.h"
#include "datastructures/datapoint.h"

template <class T>
class Mynode: public ObjectPool< Mynode<T> >
{

  public:

// Initialization, constructor and destructor functions:

   Mynode();
   Mynode(const T& d);
   Mynode(const Mynode<T>& node);
   virtual ~Mynode();
   Mynode<T>& operator= (const Mynode<T>& node);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Mynode<T1>& node);

// Set and get member functions:
   
   void set_ID(int id);
   void set_order(double o);
   void set_data(T const & d);
   void set_nextptr(Mynode<T>* nextptr);
   void set_prevptr(Mynode<T>* prevptr);

   int get_ID() const;
   double get_order() const;
   T& get_data(); 
   const T& get_data() const; 
   T* get_data_ptr() ;
   Mynode<T>* const get_nextptr() const;
   Mynode<T>* const get_prevptr() const;

  protected:

   int ID;
   double order;

   T data;		// Arbitrary data object
   Mynode<T>* next_ptr;
   Mynode<T>* prev_ptr;

   void initialize_member_objects();
   void docopy(const Mynode<T>& node);
};

typedef Mynode<datapoint> mynode;

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline void Mynode<T>::set_ID(int id)
{
   ID=id;
}

template <class T> inline void Mynode<T>::set_order(double o)
{
   order=o;
}

template <class T> inline void Mynode<T>::set_nextptr(Mynode<T>* nextptr)
{
   next_ptr=nextptr;
}

template <class T> inline void Mynode<T>::set_prevptr(Mynode<T>* prevptr)
{
   prev_ptr=prevptr;
}

template <class T> inline void Mynode<T>::set_data(T const & d)
{
   data=d;
}

template <class T> inline int Mynode<T>::get_ID() const
{
   return ID;
}

template <class T> inline double Mynode<T>::get_order() const
{
   return order;
}

template <class T> inline T& Mynode<T>::get_data() 
{
   return data;
}

template <class T> inline const T& Mynode<T>::get_data() const
{
   return data;
}

template <class T> inline T* Mynode<T>::get_data_ptr() 
{
   return &data;
}

template <class T> inline Mynode<T>* const Mynode<T>::get_nextptr() const
{
   return next_ptr;
}

template <class T> inline Mynode<T>* const Mynode<T>::get_prevptr() const
{
   return prev_ptr;
}

template <class T> inline void Mynode<T>::initialize_member_objects()
{
   ID=0;
   order=0;
   prev_ptr=NULL;
   next_ptr=NULL;
}

#include "datastructures/Mynode.cc"

#endif  // T_datastructures/Mynode.h



