// ==========================================================================
// Header file for quaternion class based upon appendix A in "Relative
// Orientation Revisited" by B.K. Horn dated 15 March 1990.
// ==========================================================================
// Last modified on 9/21/11; 9/22/11; 9/23/11
// ==========================================================================

#ifndef QUATERNION_H
#define QUATERNION_H

#include "math/fourvector.h"
#include "math/rotation.h"
#include "math/threevector.h"

class quaternion
{

  public:

// ---------------------------------------------------------------------
// Constructor function
// ---------------------------------------------------------------------

   quaternion();
   quaternion(double s,const threevector& r);
   quaternion(const quaternion& q);
   virtual ~quaternion();
   quaternion& operator= (const quaternion& q);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const quaternion& q);

// Set & get methods

   void set_s(double s);
   double get_s() const;
   void set_r(const threevector& r);
   threevector& get_r();
   const threevector& get_r() const;

   quaternion conjugate() const;
   double dot(const quaternion& q);
   double sqrd_magnitude() const;
   double magnitude() const; 
   void identity();
   quaternion inverse();

   void generate_random_unit_quaternion();
   fourvector convert_to_fourvector();

// Rotation conversion member functions:

   quaternion generate_from_rotation(const rotation& R);
   rotation convert_to_rotation();

// Friend functions:

   friend quaternion operator+ (const quaternion& q,const quaternion& p);
   friend quaternion operator- (const quaternion& q,const quaternion& p);
   friend quaternion operator- (const quaternion& q);
   friend quaternion operator* (double a,const quaternion& q);
   friend quaternion operator* (const quaternion& q,double a);
   friend quaternion operator/ (const quaternion& q,double a);
   friend quaternion operator* (const quaternion& q,const quaternion& p);

  private: 

   double s;
   threevector r;

   void docopy(const quaternion& v);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void quaternion::set_s(double s)
{
   this->s=s;
}

inline double quaternion::get_s() const
{
   return s;
}

inline void quaternion::set_r(const threevector& r)
{
   this->r=r;
}

inline threevector& quaternion::get_r() 
{
   return r;
}

inline const threevector& quaternion::get_r() const
{
   return r;
}



#endif  // math/quaternion.h






