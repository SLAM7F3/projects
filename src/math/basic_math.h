// ==========================================================================
// Header file for some extremely basic math functions.  All the
// primitive methods within this file are inlined for speed.  So there
// does NOT exist a "basic_math.cc" file corresponding to this
// "basic_math.h" file.
// ==========================================================================
// Last updated on 3/21/08; 9/5/08; 8/19/16; 12/8/16
// ==========================================================================

#ifndef BASIC_MATH_H
#define BASIC_MATH_H

#include <iomanip>
#include <iostream>

// Note added on 6/23/05: Shouldn't we just include <math> rather than
// <math.h>

#include <math.h>

#include <stdlib.h>

// ==========================================================================

namespace basic_math
{
   
// Declarations:

   int round(double x);

// Definitions:

   inline bool is_int(double x,double TINY=1E-10)
      {
//         const double TINY=1E-10;
         return (fabs(x-round(x)) < TINY);
      }

// ---------------------------------------------------------------------
// In Aug 03, we discovered that the "standard" C floor function
// definitely exhibits platform dependence.  In particular, we found
// on one machine that floor(3.000000000)=2 while on another
// floor(3.000000000)=3.  To eliminate this source of uncertainty, we
// are forced to supply our own truncation method.

// Mytruncate(x) returns greatest integer less than x (even for x < 0).

   inline int mytruncate(double x)
      {
         const double TINY=1E-14;
         return round(floor(x+TINY));
      }

// ---------------------------------------------------------------------
// Method phase_to_canonical_interval adds or subtracts multiples of
// (phase_max-phase_min) until the input phase value lies within the
// specified interval [phase_min,phase_max].  Note that the output
// phase value is in radians [degs] if the input phase value is in
// radians [degs].

   inline double phase_to_canonical_interval(
      double phase,double phase_min,double phase_max)
      {
         double phase_interval=phase_max-phase_min;
         int n=basic_math::round(floor((phase-phase_min)/phase_interval));
         phase -= phase_interval*n;
         return phase;
      }

// ---------------------------------------------------------------------
// As of Aug 03, we believe that this round method should be robust
// against platform dependence in the standard C floor function.
// Suppose x=3.00000001.  Then floor(x) = 3.000000000 or perhaps
// 2.99999999.  xlo=int(floor(x)) = 3 or 2.  In the first case, x-xlo
// is very small, and xlo=3 should be returned.  In the second case,
// x-xlo is nearly unity, and xlo+1=3 should also be returned.  

   inline int round(double x)
      {
         int xlo=int(floor(x));

// On 10/2/01, we discovered that the GROUP 93 SGI machine SCOOBY did
// not properly round fractions ending with 0.5.  So we made the
// following ugly change in order to port our A2AU codes from KERMIE
// to SCOOBY:

         if (x-xlo >= 0.499999999)
//   if (x-xlo >= 0.5)
         {
            return xlo+1;
         }
         else
         {
            return xlo;
         }
      }

/*
  inline int round(double x)
  {
  if (x > 0)
  {
  return int(x+0.5);
  }
  else
  {
  return int(x-0.5);
  }
  }
*/

// ---------------------------------------------------------------------
// Minimum and maximum macros:
// ---------------------------------------------------------------------

   template <class T> inline T max(T x,T y)
   {
      if (x > y) 
      {
         return x;
      }
      else
      {
         return y;
      }
   }

   template <class T> inline T max(T x,T y,T z)
   {
      return max(x,max(y,z));
   }

   template <class T> inline T max(T w,T x,T y,T z)
   {
      return max(w,max(x,y,z));
   }

   template <class T> inline T max(T w,T x,T y,T z,T a)
   {
      return max(a,max(w,x,y,z));
   }

   template <class T> inline T max(T w,T x,T y,T z,T a, T b)
   {
      return max(b,max(w,x,y,z,a));
   }

   template <class T> inline T min(T x,T y)
   {
      if (x < y)
      {
         return x;
      }
      else
      {
         return y;
      }
   }

   template <class T> inline T min(T x,T y,T z)
   {
      return min(x,min(y,z));
   }

   template <class T> inline T min(T w,T x,T y,T z)
   {
      return min(w,min(x,y,z));
   }

   template <class T> inline T min(T w,T x,T y,T z,T a)
   {
      return min(a,min(w,x,y,z));
   }

   template <class T> inline T min(T w,T x,T y,T z,T a, T b)
   {
      return min(b,min(w,x,y,z,a));
   }

} // basic_math namespace

// ==========================================================================

int modulo(int i,int n);
bool nearly_equal(double a,double b,double TINY=1.0E-5);

// ==========================================================================
// Inlined methods
// ==========================================================================

inline bool is_even(int n)
{
   return(modulo(n,2)==0);
}

inline bool is_odd(int n)
{
   return(modulo(n+1,2)==0);
}

// ---------------------------------------------------------------------
// Method modulo returns i mod n provided n > 0.  Unlike C++'s
// built in remainder function, the output for any positive, negative
// or zero valued integer i is nonnegative.

inline int modulo(int i,int n)
{
   if (n <= 0)
   {
      std::cout << "Error inside modulo method in basicmath.h !" << std::endl;
      std::cout << "i = " << i << " n = " << n << std::endl;
      std::cout << "n must be positive!" << std::endl;
      return 0;
   }
   else
   {
      if (i >= 0)
      {
         return i % n;
      }
      else
      {
         return n - abs(i) % n;
      }
   }
}

/*
// ---------------------------------------------------------------------
// Method myfinite checks whether the input variable x is either
// infinite or non-a-number (NAN).  It also checks whether the
// magnitude of x is exceeds POSITIVEINFINITY.  If any of these cases
// holds, myfinite returns false.  Otherwise, x is deemed to be a
// legitimate finite number, and myfinite returns true:

inline bool myfinite(double x)
{
   return true;
//   return ((finite(x) != 0) && fabs(x) < 10*POSITIVEINFINITY);
}
*/

inline int sgn(double x)
{
   if (x==0)
   {
      return 0;
   }
   else if (x < 0)
   {
      return -1;
   }
   else
   {
      return 1;
   }
}

// ---------------------------------------------------------------------
// Method nearly_equal is intended to remove system dependence for
// determining equality between two doubles:

inline bool nearly_equal(double a,double b,double TINY)
{
//   std::cout << "inside basic_math::nearly_equal()" << std::endl;
//   std::cout << "a = " << a << " b = " << b << std::endl;
//   std::cout << "fabs(a-b) = " << fabs(a-b) << std::endl;
   return (fabs(a-b) < TINY);
}

// ---------------------------------------------------------------------
template <class T> inline T sqr(T x)
{
   return x*x;
}

// ---------------------------------------------------------------------
// Method trunclog returns the closest smallest power of 10 to abs(x):
// e.g. trunclog(0)=0; trunclog(0.2)=0.1, trunclog(-0.2)=0.1;
// trunclog(-2.2)=1

inline double trunclog(double x)
{
   if (x==0)
   {
      return(0);
   }
   else
   {
      double a=fabs(x);
      double y=floor(log10(a));
      return(pow(10,y));
   }
}
 
#endif  // basic_math.h

