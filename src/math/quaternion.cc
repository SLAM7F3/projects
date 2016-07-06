// ==========================================================================
// Quaternion class member function definitions
// ==========================================================================
// Last modified on 9/21/11; 9/22/11; 9/23/11; 3/29/14
// ==========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "math/mathfuncs.h"
#include "math/quaternion.h"
#include "numrec/nrfuncs.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

quaternion::quaternion()
{
   s=0;
   r=Zero_vector;
}

quaternion::quaternion(double s,const threevector& r)
{
   this->s=s;
   this->r=r;
}

// Copy constructor

quaternion::quaternion(const quaternion& q)
{
   docopy(q);
}

quaternion::~quaternion()
{
}

// ---------------------------------------------------------------------
void quaternion::docopy(const quaternion& q)
{
   s=q.s;
   r=q.r;
}

// Overload = operator:

quaternion& quaternion::operator= (const quaternion& q)
{
   if (this==&q) return *this;
   docopy(q);
   return *this;
}

// Overload << operator

ostream& operator<< (ostream& outstream,const quaternion& q)
{
   outstream << q.s << " , "
             << q.r.get(0) << " , "
             << q.r.get(1) << " , "
             << q.r.get(2) ;
   return outstream;
}

// ---------------------------------------------------------------------
quaternion quaternion::conjugate() const
{
   return quaternion(s,-r);
}

// ---------------------------------------------------------------------
double quaternion::dot(const quaternion& q)
{
   double term1=s*q.get_s();
   double term2=r.dot(q.get_r());
   return term1+term2;
}

// ---------------------------------------------------------------------
double quaternion::sqrd_magnitude() const
{
   double sqrd_magnitude=s*s+r.sqrd_magnitude();
   return sqrd_magnitude;
}

double quaternion::magnitude() const
{
   return sqrt(sqrd_magnitude());
}

// ---------------------------------------------------------------------
void quaternion::identity()
{
   s=1;
   r=Zero_vector;
}

// ---------------------------------------------------------------------
quaternion quaternion::inverse()
{
   double curr_mag=magnitude();
   if (nearly_equal(curr_mag,0))
   {
      cout << "Error in quaternion::inverse()!" << endl;
      cout << "magnitude = " << magnitude() << endl;
      exit(-1);
   }

   quaternion conj=*this;
   conj.set_r(-get_r());
   
   quaternion inverse=conj/sqr(curr_mag);
   return inverse;
}

// ---------------------------------------------------------------------
// Member function generate_random_quaternion() returns a unit
// quaternion with random scalar and three-vector entries.

void quaternion::generate_random_unit_quaternion()
{
   double alpha=(360*PI/180)*nrfunc::ran1();
   double beta=(360*PI/180)*nrfunc::ran1();
   double gamma=(360*PI/180)*nrfunc::ran1();
   double cos_alpha=cos(alpha);
   double sin_alpha=sin(alpha);
   double cos_beta=cos(beta);
   double sin_beta=sin(beta);
   double cos_gamma=cos(gamma);
   double sin_gamma=sin(gamma);
   
   s=cos_alpha*cos_beta*cos_gamma;
   r=threevector(cos_alpha*cos_beta*sin_gamma,cos_alpha*sin_beta,sin_alpha);
}

// ---------------------------------------------------------------------
// Member function convert_to_fourvector()

fourvector quaternion::convert_to_fourvector()
{
   return fourvector(s,r.get(0),r.get(1),r.get(2));
}

// ==========================================================================
// Rotation conversion member functions
// ==========================================================================

// Member function rotation() takes in rotation R and returns a
// quaternion representing this SO(3) member.  Recall the negative of the
// quaternion also represents the same SO(3) rotation.

quaternion quaternion::generate_from_rotation(const rotation& R)
{
   double theta;
   threevector n_hat;
   mathfunc::decompose_orthogonal_matrix(R,theta,n_hat);

   s=cos(0.5*theta);
   r=sin(0.5*theta)*n_hat;
   return *this;
}

// ---------------------------------------------------------------------
rotation quaternion::convert_to_rotation()
{
//   cout << "inside quaternion::recover_rotation_params()" << endl;
   
   double cos_theta=sqr(s)-r.dot(r);
   double sin_theta=2*s*r.magnitude();
   double theta=atan2(sin_theta,cos_theta);
//   cout << "theta " << theta*180/PI << endl;
   threevector n_hat=r/sin(0.5*theta);
//   cout << "n_hat = " << n_hat << endl;
//   cout << "n_hat.magnitude = " << n_hat.magnitude() << endl;

   return rotation(n_hat,theta);
}

// ---------------------------------------------------------------------
// Overload + operator:

quaternion operator+ (const quaternion& q,const quaternion& p)
{
   quaternion sum;
   sum.set_s(q.get_s()+p.get_s());
   sum.set_r(q.get_r()+p.get_r());
   return sum;
}

// ---------------------------------------------------------------------
// Overload - operator:

quaternion operator- (const quaternion& q,const quaternion& p)
{
   quaternion difference;
   difference.set_s(q.get_s()-p.get_s());
   difference.set_r(q.get_r()-p.get_r());
   return difference;
}

// ---------------------------------------------------------------------
// Overload - operator:

quaternion operator- (const quaternion& q)
{
   quaternion negative;
   negative.set_s(-q.get_s());
   negative.set_r(-q.get_r());
   return negative;
}

// ---------------------------------------------------------------------
// Overload * operator:

quaternion operator* (double a,const quaternion& q)
{
   double s=q.get_s()*a;
   threevector r=q.get_r()*a;
   return quaternion(s,r);
}


quaternion operator* (const quaternion& q,double a)
{
   double s=q.get_s()*a;
   threevector r=q.get_r()*a;
   return quaternion(s,r);
}

// Overload / operator:

quaternion operator/ (const quaternion& q,double a)
{
   double s=q.get_s()/a;
   threevector r=q.get_r()/a;
   return quaternion(s,r);
}

// Overload * operator:

quaternion operator* (const quaternion& q,const quaternion& p)
{
   double s_qp=q.get_s()*p.get_s()-q.get_r().dot(p.get_r());
   threevector r_qp=q.get_s()*p.get_r()+p.get_s()*q.get_r()+
      q.get_r().cross(p.get_r());
   return quaternion(s_qp,r_qp);
}
