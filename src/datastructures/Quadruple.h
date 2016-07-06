// ==========================================================================
// Header file for templatized Quadruple class that generalizes STL's
// pair class.
// ==========================================================================
// Last modified on 12/14/05; 12/29/12
// ==========================================================================

#ifndef T_QUADRUPLE_H
#define T_QUADRUPLE_H

#include <iostream>

template <class A,class B,class C,class D>
struct Quadruple
{
      A first;
      B second;
      C third;
      D fourth;
      
      Quadruple();
      Quadruple(A a,B b);
      Quadruple(A a,B b,C c,D d);
      void docopy(const Quadruple<A,B,C,D>& q);
      Quadruple<A,B,C,D>& operator= (const Quadruple<A,B,C,D>& q);
      bool operator== (const Quadruple<A,B,C,D>& q) const;
      bool operator!= (const Quadruple<A,B,C,D>& q) const;

      template <class A1,class B1,class C1,class D1>
      friend std::ostream& operator<< 
      (std::ostream& outstream,const Quadruple<A1,B1,C1,D1>& q);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

template <class A,class B,class C,class D>
inline Quadruple<A,B,C,D>::Quadruple()
{
//   first=second=0;
//   third=0;
//   fourth=0;
}

template <class A,class B,class C,class D>
inline Quadruple<A,B,C,D>::Quadruple(A a,B b)
{
   first=a;
   second=b;
//   third=0;
//   fourth=0;
}

template <class A,class B,class C,class D>
inline Quadruple<A,B,C,D>::Quadruple(A a,B b,C c,D d)
{
   first=a;
   second=b;
   third=c;
   fourth=d;
}

template <class A,class B,class C,class D>
inline void Quadruple<A,B,C,D>::docopy(const Quadruple<A,B,C,D>& q)
{
   first=q.first;
   second=q.second;
   third=q.third;
   fourth=q.fourth;
}

// ---------------------------------------------------------------------
// Overload = operator:

template <class A,class B,class C,class D>
inline Quadruple<A,B,C,D>& Quadruple<A,B,C,D>::operator= 
(const Quadruple<A,B,C,D>& q)
{
   if (this==&q) return *this;
   docopy(q);
   return *this;
}

// ---------------------------------------------------------------------
// Overload == operator:

template <class A,class B,class C,class D>
inline bool Quadruple<A,B,C,D>::operator== (const Quadruple<A,B,C,D>& q) const
{
   return (q.first==first && q.second==second && q.third==third &&
           q.fourth==fourth);
}

// ---------------------------------------------------------------------
// Overload != operator:

template <class A,class B,class C,class D>
inline bool Quadruple<A,B,C,D>::operator!= (const Quadruple<A,B,C,D>& q) const
{
   return !(q==*this);
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class A,class B,class C,class D> std::ostream& operator<< 
(std::ostream& outstream,const Quadruple<A,B,C,D>& q)
{
   outstream << std::endl;
   outstream << "(first,second,third,fourth) = (" 
             << q.first << "," << q.second << "," 
             << q.third << "," << q.fourth << ")" << std::endl;
   return outstream;
}

#endif  // T_Quadruple.h



