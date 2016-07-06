// ==========================================================================
// Header file for twovector class 
// ==========================================================================
// Last modified on 12/30/05; 7/6/06; 6/29/07; 2/10/08
// ==========================================================================

#ifndef TWOVECTOR_H
#define TWOVECTOR_H

#include "math/genvector.h"

class threevector;

class twovector:public genvector
{

  public:

   typedef double value_type;

// ---------------------------------------------------------------------
// Constructor function
// ---------------------------------------------------------------------

   twovector();
   twovector(double x,double y);
   twovector(const threevector& v);
   twovector(const genvector& v);
   twovector(const tensor& T);
   twovector(const twovector& v);
   virtual ~twovector();
   twovector& operator= (const twovector& v);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const twovector& X);
 
// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   value_type operator[] (int n) const;
   twovector unitvector() const;

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   friend twovector operator+ (const twovector& X,const twovector& Y);
   friend twovector operator- (const twovector& X,const twovector& Y);
   friend twovector operator- (const twovector& X);
   friend twovector operator* (double a,const twovector& X);
   friend twovector operator* (const twovector& X,double a);
   friend twovector operator/ (const twovector& X,double a);
   friend twovector operator* (const genmatrix& A,const twovector& X);
   friend twovector operator* (const twovector& X,const genmatrix& A);

  private: 

   void docopy(const twovector& v);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// We define operator[] for kdtree searching purposes:

inline twovector::value_type twovector::operator[] (int n) const
{
   return get(n);
}

#endif  // math/twovector.h





