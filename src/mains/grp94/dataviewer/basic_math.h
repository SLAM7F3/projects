// ==========================================================================
// Header file for some extremely basic math functions.  All the
// primitive methods within this file are inlined for speed.  So there
// does NOT exist a "basic_math.cc" file corresponding to this
// "basic_math.h" file.
// ==========================================================================
// Last updated on 3/11/04
// ==========================================================================

#ifndef BASIC_MATH_H
#define BASIC_MATH_H

#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdlib.h>

// Minimum and maximum macros:

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

bool is_even(int n);
bool is_odd(int n);
int max(int x,int y);
double max(int x,double y);
double max(double x,int y);
double max(double x,double y);
int max(int x,int y,int z);
double max(double x,double y,double z);
double max(double w,double x,double y,double z);
int min(int x,int y);
double min(int x,double y);
double min(double x,int y);
double min(double x,double y);
int min(int x,int y,int z);
double min(double x,double y,double z);
double min(double w,double x,double y,double z);
int modulo(int i,int n);
bool myfinite(double x);
int sgn(double x);
int sqr(int n);
long sqr(long n);
float sqr(float x);
double sqr(double x);
double trunclog(double x);

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline bool is_even(int n)
{
   return(modulo(n,2)==0);
}

// ---------------------------------------------------------------------
inline bool is_odd(int n)
{
   return(modulo(n+1,2)==0);
}

// ---------------------------------------------------------------------
inline int max(int x,int y)
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

inline double max(double x,int y)
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

inline double max(int x,double y)
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

inline double max(double x,double y)
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

inline int max(int x,int y,int z)
{
   return max(x,max(y,z));
}

inline double max(double x,double y,double z)
{
   return max(x,max(y,z));
}

inline double max(double w,double x,double y,double z)
{
   return max(w,max(x,y,z));
}

// ---------------------------------------------------------------------
inline int min(int x,int y)
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

inline double min(int x,double y)
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

inline double min(double x,int y)
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

inline double min(double x,double y)
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

inline int min(int x,int y,int z)
{
   return min(x,min(y,z));
}

inline double min(double x,double y,double z)
{
   return min(x,min(y,z));
}

inline double min(double w,double x,double y,double z)
{
   return min(w,min(x,y,z));
}

// ---------------------------------------------------------------------
// Method modulo takes returns i mod n provided n > 0.  Unlike
// C++'s built in remainder function, integer i can assume any
// positive, negative or zero value...

inline int modulo(int i,int n)
{
   int j;
   
   if (n < 0)
   {
      std::cout << "Error inside modulo method in basicmath.h !" << std::endl;
      std::cout << "i = " << i << " n = " << n << std::endl;
      return 0;
   }
   else
   {
      if (i >= 0)
      {
         return i%n;
      }
      else
      {
         j=i+n*(int(abs(i)/n)+1);
         return j%n;
      }
   }
}

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
inline float sqr(float x)
{
   return x*x;
}

inline int sqr(int n)
{
   return n*n;
}

inline long sqr(long x)
{
   return x*x;
}

inline double sqr(double x)
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

