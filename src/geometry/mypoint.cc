// ==========================================================================
// Mypoint class member function definitions
// ==========================================================================
// Last modified on 7/29/06; 8/5/06; 1/29/12
// ==========================================================================

#include "math/constant_vectors.h"
#include "geometry/mypoint.h"
#include "math/rotation.h"

using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

mypoint::mypoint(void)
{
   pnt=Zero_vector;
}

mypoint::mypoint(const threevector& v)
{
   pnt=v;
}

// Copy constructor:

mypoint::mypoint(const mypoint& p)
{
   docopy(p);
}

mypoint::~mypoint()
{
}

// ---------------------------------------------------------------------
void mypoint::docopy(const mypoint& p)
{
   pnt=p.pnt;
}	

// Overload = operator:

mypoint& mypoint::operator= (const mypoint& p)
{
   if (this==&p) return *this;
   docopy(p);
   return *this;
}

mypoint& mypoint::operator= (const threevector& v)
{
   pnt=v;
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const mypoint& p)
{
   outstream << endl;
   outstream << p.pnt << endl;
   return(outstream);
}

// ==========================================================================
void mypoint::translate(const threevector& rvec)
{
   pnt += rvec;
}

// ---------------------------------------------------------------------
void mypoint::scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   threevector dp(pnt-scale_origin);
   dp.put(0,dp.get(0)*scalefactor.get(0));
   dp.put(1,dp.get(1)*scalefactor.get(1));
   dp.put(2,dp.get(2)*scalefactor.get(2));
   pnt=scale_origin+dp;
}

// ---------------------------------------------------------------------
void mypoint::rotate(const rotation& R)
{
   pnt=R*pnt;
}

// ---------------------------------------------------------------------
void mypoint::rotate(const threevector& rotation_origin,
                     double thetax,double thetay,double thetaz)
{
   rotation R(thetax,thetay,thetaz);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
// This overloaded version of member function rotate performs a right
// handed rotation of the current point object through angle alpha
// about the axis_direction direction vector:

void mypoint::rotate(
   const threevector& rotation_origin,const threevector& axis_direction,
   double alpha)
{
   rotation R(axis_direction,alpha);
   rotate(rotation_origin,R);
}

// ---------------------------------------------------------------------
void mypoint::rotate(const threevector& rotation_origin,const rotation& R)
{
   threevector dp(pnt-rotation_origin);
   dp=R*dp;
   pnt=rotation_origin+dp;
}

// ---------------------------------------------------------------------
// Member function delta_scale computes the change in the current
// point's position induced by the input scaling vector scalefactor
// wrt the origin point scale_origin:

threevector mypoint::delta_scale(
   const threevector& scale_origin,const threevector& scalefactor)
{
   threevector dp(pnt-scale_origin);
   for (int i=0; i<3; i++)
   {
      dp.put(i,dp.get(i)*scalefactor.get(i));
   }
   return threevector((scale_origin+dp)-pnt);
}

// ---------------------------------------------------------------------
// Member function delta_rotation computes the change in the current
// point's position induced by a rotation about the axis_direction
// direction vector through infinitesimal angle dtheta:

threevector mypoint::delta_rotation(
   const threevector& rotation_origin,const threevector& axis_direction,
   double dtheta)
{
   threevector dp(pnt-rotation_origin);
   threevector n_hat(axis_direction.unitvector());
   return threevector(sin(dtheta)*n_hat.cross(dp)
      +(cos(dtheta)-1)*(dp.dot(n_hat)*n_hat-dp));
}

// ---------------------------------------------------------------------
// Overload - operator:
          
mypoint operator- (const mypoint& X)
{
   return mypoint(-X.pnt);
}

