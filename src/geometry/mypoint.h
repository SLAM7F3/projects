// ==========================================================================
// Header file for mypoint class
// ==========================================================================
// Last modified on 4/13/06; 8/5/06; 1/29/12
// ==========================================================================

#ifndef MYPOINT_H
#define MYPOINT_H

#include "math/threevector.h"

class rotation;

class mypoint
{

  public:

// Constructor functions

   mypoint(void);
   mypoint(const threevector& v);
   mypoint(const mypoint& p);
   ~mypoint();
   mypoint& operator= (const mypoint& p);
   mypoint& operator= (const threevector& v);
   friend std::ostream& operator<< (std::ostream& outstream,const mypoint& p);

// Set and get member functions:

   void set_pnt(const threevector& p);
   const threevector& get_pnt() const;
   threevector get_center() const;

   void translate(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);
   void rotate(
      const threevector& rotation_origin,const threevector& axis_direction,
      double alpha);
   void rotate(const threevector& rotation_origin,const rotation& R);
   threevector delta_scale(
      const threevector& scale_origin,const threevector& scalefactor);
   threevector delta_rotation(
      const threevector& rotation_origin,const threevector& axis_direction,
      double dtheta);
   friend mypoint operator- (const mypoint& X);

  private: 

   threevector pnt;
   void docopy(const mypoint& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void mypoint::set_pnt(const threevector& p)
{
   pnt=p;
}

inline const threevector& mypoint::get_pnt() const
{
   return pnt;
}

inline threevector mypoint::get_center() const
{
   return threevector(pnt);
}

#endif  // mypoint.h


