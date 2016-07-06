// ==========================================================================
// Header file for templatized Triple structure that generalizes STL's
// pair class.
// ==========================================================================
// Last modified on 12/16/05; 7/17/06; 7/18/06; 1/5/07
// ==========================================================================

#ifndef T_TRIPLE_H
#define T_TRIPLE_H

#include <iostream>
#include "math/basic_math.h"

template <class A,class B,class C>
struct Triple
{
      public:

      A first;
      B second;
      C third;
      
      Triple();
      Triple(A a,B b,C c);

      Triple<A,B,C>& operator= (const Triple<A,B,C>& T);

      template<class E,class F,class G>
      friend Triple<E,F,G> operator+ (
         const Triple<E,F,G>& S,const Triple<E,F,G>& T);
      template<class E,class F,class G>
      friend Triple<E,F,G> operator- (
         const Triple<E,F,G>& S,const Triple<E,F,G>& T);
      template<class E,class F,class G>
      friend Triple<E,F,G> operator* (
         double a,const Triple<E,F,G>& T);
      template<class E,class F,class G>
      friend Triple<E,F,G> operator* (
         const Triple<E,F,G>& T,double a);
      template<class E,class F,class G>
      friend Triple<E,F,G> operator/ (
         const Triple<E,F,G>& T,double a);

      bool operator== (const Triple<A,B,C>& T) const;
      bool operator!= (const Triple<A,B,C>& T) const;
      bool nearly_equal(const Triple<A,B,C>& T) const;

      template <class A1,class B1,class C1>
      friend std::ostream& operator<< 
      (std::ostream& outstream,const Triple<A1,B1,C1>& T);

      private:

      void docopy(const Triple<A,B,C>& T);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

template <class A,class B,class C>
inline Triple<A,B,C>::Triple()
{
   first=0;
   second=0;
   third=0;
}

template <class A,class B,class C>
inline Triple<A,B,C>::Triple(A a,B b,C c)
{
   first=a;
   second=b;
   third=c;
}

// ---------------------------------------------------------------------
template <class A,class B,class C>
inline void Triple<A,B,C>::docopy(const Triple<A,B,C>& T)
{
   first=T.first;
   second=T.second;
   third=T.third;
}

// ---------------------------------------------------------------------
// Overload = operator:

template <class A,class B,class C>
inline Triple<A,B,C>& Triple<A,B,C>::operator= (const Triple<A,B,C>& T)
{
   if (this==&T) return *this;
   docopy(T);
   return *this;
}

// ---------------------------------------------------------------------
// Overload + operator:

template <class A,class B,class C>
inline Triple<A,B,C> operator+ (
   const Triple<A,B,C>& S,const Triple<A,B,C>& T)
{
   return Triple<A,B,C>(S.first+T.first,S.second+T.second,S.third+T.third);
}

// ---------------------------------------------------------------------
// Overload - operator:

template <class A,class B,class C>
inline Triple<A,B,C> operator- (
   const Triple<A,B,C>& S,const Triple<A,B,C>& T)
{
   return Triple<A,B,C>(S.first-T.first,S.second-T.second,S.third-T.third);
}

// ---------------------------------------------------------------------
// Overload * operator:

template <class A,class B,class C>
inline Triple<A,B,C> operator* (double a,const Triple<A,B,C>& T)
{
   return Triple<A,B,C>(a*T.first,a*T.second,a*T.third);
}

template <class A,class B,class C>
inline Triple<A,B,C> operator* (const Triple<A,B,C>& T,double a)
{
   return Triple<A,B,C>(a*T.first,a*T.second,a*T.third);
}
// ---------------------------------------------------------------------
// Overload / operator:

template <class A,class B,class C>
inline Triple<A,B,C> operator/ (const Triple<A,B,C>& T,double a)
{
   return Triple<A,B,C>(T.first/a,T.second/a,T.third/a);
}

// ---------------------------------------------------------------------
// Overload == operator:

template <class A,class B,class C>
inline bool Triple<A,B,C>::operator== (const Triple<A,B,C>& T) const
{
   return (T.first==first && T.second==second && T.third==third);
}

// ---------------------------------------------------------------------
// Overload != operator:

template <class A,class B,class C>
inline bool Triple<A,B,C>::operator!= (const Triple<A,B,C>& T) const
{
   return !(T==*this);
}

// ---------------------------------------------------------------------
// nearly_equal operator:

template <class A,class B,class C>
inline bool Triple<A,B,C>::nearly_equal(const Triple<A,B,C>& T) const
{
   return (::nearly_equal(T.first,first) && 
           ::nearly_equal(T.second,second) && 
           ::nearly_equal(T.third,third));
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class A,class B,class C> inline std::ostream& operator<< 
(std::ostream& outstream,const Triple<A,B,C>& T)
{
   outstream << "(" << T.first << "," << T.second << "," << T.third
             << ")" << std::endl;
   return outstream;
}


#endif  // T_datastructures/Triple.h



