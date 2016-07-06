// ==========================================================================
// Header file for templatized Tensor class 
// ==========================================================================
// Last modified on 11/26/10; 3/28/14; 4/4/14; 4/5/14
// ==========================================================================

#ifndef T_TENSOR_H
#define T_TENSOR_H

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <new>
#include <string>
#include <vector>
#include "math/constants.h"

template <class A>
class Tensor
{

  public:

// Initialization, constructor and destructor functions:

// On 1/18/02, Tara Dennis strongly recommended avoiding void
// constructors.  She said that the C++ philosophy is different than
// that of C.  Objects should not be declared before they are defined.
// And once we are finished working with an object, it should
// immediately be deleted.  Moreover, basic member variables within a
// base class should be allocated and deleted within that class and
// generally not within derived classes.  Since derived class
// constructors first call base class constructors, Tara recommended
// that there should generally be some base class constructor with the
// same interface as any derived class constructor; the derived class
// constructor should then explicitly call the base class constructor
// using the : syntax...

   Tensor(unsigned int rnk,const std::vector<unsigned int>& d);
   Tensor();
   Tensor(int m);
   Tensor(int m,int n);
   Tensor(int m,int n,int p);
   Tensor(int m,int n,int p,int q);
   Tensor(const Tensor<A>& T);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~Tensor();

   Tensor<A>& operator= (const Tensor<A>& T);

   bool operator== (const Tensor<A>& T) const;
   bool operator!= (const Tensor<A>& T) const;

// On 11/12/03, we learned from Andrew Bradley that friend functions
// which are not part of a templatized class should NOT use the same
// dummy template parameters as those which are used elsewhere within
// the class.

   template<class B>
   friend std::ostream& operator<< (std::ostream& outstream, Tensor<B>& T);

// ---------------------------------------------------------------------
// Set & get methods:

   unsigned int get_rank() const;
   unsigned int get_dimproduct() const;
   unsigned int get_dim(int r) const;
   unsigned int get_Indices(int r) const;

// Member functions:

   bool check_index_validity(unsigned int index) const;

   void put(const std::vector<int>& entry,A value);
   void put(int index,A value);
   void put(int m,int n,A value);
   void put(int m,int n,int p,A value);
   void put(int m,int n,int p,int q,A value);

   void increment(const std::vector<unsigned int>& entry,A value);
   void increment(unsigned int index,A value);
   void increment(int m,int n,A value);
   void increment(int m,int n,int p,A value);
   void increment(int m,int n,int p,int q,A value);

   A get(const std::vector<unsigned int>& entry) const;
   A get(unsigned int index) const;
   A get(int m,int n) const;
   A get(int m,int n,int p) const;
   A get(int m,int n,int p,int q) const;
   A* get_e_ptr();
   const A* get_e_ptr() const;

   void clear_values();
   void initialize_values(A value);
   void minmax_values(A& minvalue,A& maxvalue) const;
   void minmax_values(
      A& minvalue,A floor_value,A& maxvalue,A ceiling_value) const;
   A minimum_value() const;
   A maximum_value() const;

   A sum_values() const;
   bool is_finite() const;
   bool compare_rank_and_dims(const Tensor<A>& T) const;
   bool nearly_equal(const Tensor<A>& T,double TINY=1.0E-5) const;

// Tensor manipulation methods:

   Tensor<A> outerproduct(const Tensor<A>& T) const;
   Tensor<A> outerproduct(const Tensor<A>& S,const Tensor<A>& T) const;
   Tensor<A> outerproduct(
      const Tensor<A>& R,const Tensor<A>& S,const Tensor<A>& T) const;
   Tensor<A> contract(unsigned int i,unsigned int j);
   Tensor<A> contract_adjacent_pairs(unsigned int i);

   void operator+= (const Tensor<A>& T);
   void operator-= (const Tensor<A>& T);
   void operator*= (A a);
   void operator/= (A a);

// Friend functions:

   template<class B>
   friend Tensor<B> operator+ (const Tensor<B>& X,const Tensor<B>& Y);
   template<class B>
   friend Tensor<B> operator- (const Tensor<B>& X,const Tensor<B>& Y);
   template<class B>
   friend Tensor<B> operator- (const Tensor<B>& X);

// For reasons which we don't understand as of 7/6/06, we are to
// declare an overloading of operator* here in Tensor.h and supply the
// definition in Tensor.cc.  So we are forced to provide the
// definition along with the declaration below within the Template
// class declaration...

/*
   template<class B>
   friend Tensor<A> operator* (B b,const Tensor<A>& X);
   template <class B>
   friend Tensor<A> operator* (const Tensor<A>& X,B b);
*/

/*
   template <class B>
   friend Tensor<A> operator* (B b,const Tensor<A>& X)
      {
         Tensor<A> Z(X.get_rank(),X.dim);
         for (int i=0; i<Z.get_dimproduct(); i++)
         {
            Z.put(i,b*X.get(i));
         }
         return Z;
      }

   template <class B>
   friend Tensor<A> operator* (const Tensor<A>& X,B b)
      {
         return b*X;
      }
*/

   friend Tensor<A> operator* (double b,const Tensor<A>& X)
      {
         Tensor<A> Z(X.get_rank(),X.dim);
         for (unsigned int i=0; i<Z.get_dimproduct(); i++)
         {
            Z.put(i,b*X.get(i));
         }
         return Z;
      }

   friend Tensor<A> operator* (const Tensor<A>& X,double b)
      {
         return b*X;
      }

  protected:

   A* e;
   void docopy(const Tensor<A>& T);

  private: 

   unsigned int rank,dimproduct;
   std::vector<unsigned int> dim;
   std::vector<unsigned int> Indices;
   
   void initialize_member_objects(int r);
   void allocate_member_objects();

   unsigned int indices_to_index(const std::vector<unsigned int>& entry) 
      const; 
   void index_to_indices(unsigned int index);
};

typedef Tensor<double> tensor;

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get methods:

template <class A> inline unsigned int Tensor<A>::get_rank() const
{
   return rank;
}

template <class A> inline unsigned int Tensor<A>::get_dimproduct() const
{
   return dimproduct;
}

template <class A> inline unsigned int Tensor<A>::get_dim(int r) const
{
   return dim[r];
}

template <class A> inline unsigned int Tensor<A>::get_Indices(int r) const
{
   return Indices[r];
}

template <class A> inline void Tensor<A>::initialize_member_objects(int r)
{
   rank=r;
   e=NULL;
}

// Put member functions:

template <class A> inline void Tensor<A>::put(
   const std::vector<int>& entry,A value)
{
   e[indices_to_index(entry)]=value;
}

template <class A> inline void Tensor<A>::put(int index,A value)
{
#ifdef DEBUGFLAG
   if (check_index_validity(index))
#endif
   {
      e[index]=value;
   }
}

template <class A> inline void Tensor<A>::put(int m,int n,A value)
{
   this->e[dim[1]*m+n]=value;
}

template <class A> inline void Tensor<A>::put(int m,int n,int p,A value)
{
   this->e[dim[1]*dim[2]*m+dim[2]*n+p]=value;
}

template <class A> inline void Tensor<A>::put(int m,int n,int p,int q,A value)
{
   this->e[dim[1]*dim[2]*dim[3]*m+dim[2]*dim[3]*n+dim[3]*p+q]=value;
}

// Increment member functions:

template <class A> inline void Tensor<A>::increment(
   const std::vector<unsigned int>& entry,A value)
{
   e[indices_to_index(entry)] += value;
}

template <class A> inline void Tensor<A>::increment(unsigned int index,A value)
{
#ifdef DEBUGFLAG
   if (check_index_validity(index))
#endif
   {
      e[index] += value;
   }
}

template <class A> inline void Tensor<A>::increment(int m,int n,A value)
{
   this->e[dim[1]*m+n] += value;
}

template <class A> inline void Tensor<A>::increment(int m,int n,int p,A value)
{
   this->e[dim[1]*dim[2]*m+dim[2]*n+p] += value;
}

template <class A> inline void Tensor<A>::increment(
   int m,int n,int p,int q,A value)
{
   this->e[dim[1]*dim[2]*dim[3]*m+dim[2]*dim[3]*n+dim[3]*p+q] += value;
}

// Get member functions:

template <class A> inline A Tensor<A>::get(
   const std::vector<unsigned int>& entry) const
{
   return e[indices_to_index(entry)];
}

template <class A> inline A Tensor<A>::get(unsigned int index) const
{
#ifdef DEBUGFLAG
   if (check_index_validity(index))
#endif
   {
      return e[index];
   }
#ifdef DEBUGFLAG
   else
   {
      return NEGATIVEINFINITY;
   }
#endif
}

template <class A> inline A Tensor<A>::get(int m,int n) const
{
   return this->e[dim[1]*m+n];
}

template <class A> inline A Tensor<A>::get(int m,int n,int p) const
{
   return this->e[dim[1]*dim[2]*m+dim[2]*n+p];
}

template <class A> inline A Tensor<A>::get(int m,int n,int p,int q) const
{
   return this->e[dim[1]*dim[2]*dim[3]*m+dim[2]*dim[3]*n+dim[3]*p+q];
}

template <class A> inline A* Tensor<A>::get_e_ptr() 
{
   return this->e;
}

template <class A> inline const A* Tensor<A>::get_e_ptr() const
{
   return this->e;
}

// The following deprecated function destroys the structure of any
// template type (e.g. complex) which is not a primitive:

//template <class A> inline void Tensor<A>::clear_values()
//{
//   int number_of_bytes=dimproduct*sizeof(*e);
//   memset(e,0,number_of_bytes);
//}

// We have to be willing to explicitly hardwire in clear_values for
// each possible tensor type:

template <> inline void Tensor<int>::clear_values()
{
   unsigned int number_of_bytes=dimproduct*sizeof(*e);
   memset(e,0,number_of_bytes);
}

template <> inline void Tensor<float>::clear_values()
{
   unsigned int number_of_bytes=dimproduct*sizeof(*e);
   memset(e,0,number_of_bytes);
}

template <> inline void Tensor<double>::clear_values()
{
   unsigned int number_of_bytes=dimproduct*sizeof(*e);
   memset(e,0,number_of_bytes);
}

template <> inline void Tensor<std::pair<double,double> >::clear_values()
{
   unsigned int number_of_bytes=dimproduct*sizeof(*e);
   memset(e,0,number_of_bytes);
}

template <> inline void 
Tensor<std::vector<std::pair<int,int> >* >::clear_values()
{
   for (unsigned int i=0; i<dimproduct; i++)
   {
      e[i]=NULL;
   }
}

template <> inline void 
Tensor<std::vector<std::pair<double,int> >* >::clear_values()
{
   for (unsigned int i=0; i<dimproduct; i++)
   {
      e[i]=NULL;
   }
}

#include "Tensor.cc"

#endif  // T_math/Tensor.h











