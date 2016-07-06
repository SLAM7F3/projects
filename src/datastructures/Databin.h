// ==========================================================================
// Header file for templatized Databin class.  This class provides
// any-valued vector field objects.  Its independent variable is its
// XYZ world-space posn member.  Its dependent variable is its
// templatized object. 
// ==========================================================================
// Last updated on 7/27/05; 7/2/07
// ==========================================================================

#ifndef T_DATABIN_H
#define T_DATABIN_H

#include <iostream>
#include <vector>
#include "math/threevector.h"

template <class T>
class Databin
{

  public:

   typedef double value_type; // needed for KDTree library calls
   
   Databin(int id,const threevector& p);
   Databin(int id,const threevector& p,const T& d);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Databin<T1>& D);

// Set and get member functions:

   int get_ID() const;
   const threevector& get_posn() const;
   void set_data(const T& d);
   const T& get_data() const;
   T* get_data_ptr();
   const T* get_data_ptr() const;
   value_type operator[] (int n) const;

  private:
   
   int ID;
   threevector posn;
   T data;  // Arbitrary data object
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class T> inline int Databin<T>::get_ID() const
{
   return ID;
}

template <class T> inline const threevector& Databin<T>::get_posn() const
{
   return posn;
}

template <class T> inline void Databin<T>::set_data(const T& d)
{
   data=d;
}

template <class T> inline const T& Databin<T>::get_data() const
{
   return data;
}

template <class T> inline T* Databin<T>::get_data_ptr() 
{
   return &data;
}

template <class T> inline const T* Databin<T>::get_data_ptr() const 
{
   return &data;
}

template <class T> inline typename Databin<T>::value_type 
  Databin<T>::operator[] (int n) const
{
   return posn.e[n];
}

#include "datastructures/Databin.cc"

#endif  // T_DATABIN_H



