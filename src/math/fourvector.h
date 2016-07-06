// ==========================================================================
// Header file for fourvector class 
// ==========================================================================
// Last modified on 6/13/06; 6/29/07; 12/20/11
// ==========================================================================

#ifndef FOURVECTOR_H
#define FOURVECTOR_H

#include "math/genvector.h"

class threevector;

class fourvector:public genvector
{

  public:

   typedef double value_type;

// ---------------------------------------------------------------------
// Constructor function
// ---------------------------------------------------------------------

   fourvector();
   fourvector(double x,double y,double z);
   fourvector(double x,double y,double z,double p);
   fourvector(const threevector& r);
   fourvector(const threevector& r,double p);
   fourvector(const genvector& v);
   fourvector(const fourvector& v);

   virtual ~fourvector();
   fourvector& operator= (const fourvector& v);
   fourvector& operator= (const genvector& v);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const fourvector& X);
 
// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   value_type operator[] (int n) const;
   fourvector unitvector() const;

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   friend fourvector operator+ (const fourvector& X,const fourvector& Y);
   friend fourvector operator- (const fourvector& X,const fourvector& Y);
   friend fourvector operator- (const fourvector& X);
   friend fourvector operator* (double a,const fourvector& X);
   friend fourvector operator* (const fourvector& X,double a);
   friend fourvector operator/ (const fourvector& X,double a);
   friend fourvector operator* (const genmatrix& A,const fourvector& X);
   friend fourvector operator* (const fourvector& X,const genmatrix& A);

  private: 

   void docopy(const fourvector& v);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// We define operator[] for kdtree searching purposes:

inline fourvector::value_type fourvector::operator[] (int n) const
{
   return get(n);
}

#endif  // math/fourvector.h






