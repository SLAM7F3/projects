// ==========================================================================
// Header file for some extremely basic math functions
// ==========================================================================
// Last updated on 3/1/04
// ==========================================================================

#ifndef BASIC_MATH_H
#define BASIC_MATH_H

// Minimum and maximum macros:

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

bool is_even(int n);
bool is_odd(int n);
int modulo(int i,int n);
int sqr(int n);
long sqr(long n);
float sqr(float x);
double sqr(double x);

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
inline int sqr(int n)
{
   return n*n;
}

inline float sqr(float x)
{
   return x*x;
}

inline long sqr(long x)
{
   return x*x;
}

inline double sqr(double x)
{
   return x*x;
}

#endif  // basic_math.h

