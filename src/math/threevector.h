// ==========================================================================
// Header file for threevector class 
// ==========================================================================
// Last modified on 9/27/11; 12/20/12; 5/8/13
// ==========================================================================

#ifndef THREEVECTOR_H
#define THREEVECTOR_H

#include <osg/Vec3>
#include <osg/Vec4>
#include "math/genvector.h"
#include "math/threematrix.h"
#include "math/twovector.h"

class fourvector;

class threevector:public genvector
{

  public:

   typedef double value_type;

// ---------------------------------------------------------------------
// Constructor function
// ---------------------------------------------------------------------

   threevector();
   threevector(double x,double y,double z=0);
   threevector(const twovector& v);
   threevector(const twovector& v,double z);
   threevector(const osg::Vec3& v);
   threevector(const osg::Vec4& v);
   threevector(const threevector& v);
   threevector(const fourvector& f);
   virtual ~threevector();
   threevector& operator= (const threevector& v);
   threevector& operator= (double v);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const threevector& X);
   std::ostream& write_to_textstream(std::ostream& textstream);

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   value_type operator[] (int n) const;
   threevector unitvector() const;
   double sqrd_magnitude() const;
   double dot(const threevector& X) const;
   void cross(const threevector& X,const threevector& Y);
   threevector cross(const threevector& Y) const;
   twovector xy_projection() const;
//   void scale(const threevector& X);

   threematrix outerproduct(const threevector& Y) const;
   genmatrix* generate_antisymmetric_matrix() const;

   void operator+= (const threevector& X);
   void operator-= (const threevector& X);
   void operator*= (double a);
   void operator/= (double a);

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   friend threevector operator+ (const threevector& X,const threevector& Y);
   friend threevector operator- (const threevector& X,const threevector& Y);

   friend threevector operator- (const threevector& X);
   friend threevector operator* (int a,const threevector& X);
   friend threevector operator* (double a,const threevector& X);
//   friend threevector operator* (const threevector& X,int a);
   friend threevector operator* (const threevector& X,double a);
   friend threevector operator/ (const threevector& X,double a);
   friend threevector operator* (const genmatrix& A,const threevector& X);
   friend threevector operator* (const threevector& X,const genmatrix& A);

  private: 

   void docopy(const threevector& v);

};


// ==========================================================================
// Inlined methods:
// ==========================================================================

// We define operator[] for kdtree searching purposes:

inline threevector::value_type threevector::operator[] (int n) const
{
   return get(n);
}

inline double threevector::sqrd_magnitude() const
{
//   return sqr(e[0])+sqr(e[1])+sqr(e[2]);
   return e[0]*e[0]+e[1]*e[1]+e[2]*e[2];
}

inline double threevector::dot(const threevector& X) const
{
   return e[0]*X.get(0)+e[1]*X.get(1)+e[2]*X.get(2);
}

/*
// Member function scale multiplies the values of each component
// within the current threevector object by the corresponding
// component within input threevector X:

inline void threevector::scale(const threevector& X)
{
   put(0,get(0)*X.get(0));
   put(1,get(1)*X.get(1));
   put(2,get(2)*X.get(2));
}
*/

// Overload +=, -=, *= and /= operators:

inline void threevector::operator+= (const threevector& X)
{
   put(0,get(0)+X.get(0));
   put(1,get(1)+X.get(1));
   put(2,get(2)+X.get(2));
}

inline void threevector::operator-= (const threevector& X)
{
   put(0,get(0)-X.get(0));
   put(1,get(1)-X.get(1));
   put(2,get(2)-X.get(2));
}

inline void threevector::operator*= (double a)
{
   put(0,a*get(0));
   put(1,a*get(1));
   put(2,a*get(2));
}

inline void threevector::operator/= (double a)
{
   put(0,get(0)/a);
   put(1,get(1)/a);
   put(2,get(2)/a);
}

inline std::ostream& threevector::write_to_textstream(
   std::ostream& textstream)
{
   textstream << get(0) << " " << get(1) << " " << get(2) << std::endl;
   return textstream;
}

#endif  // math/threevector.h






