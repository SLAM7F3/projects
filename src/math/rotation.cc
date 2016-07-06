// ==========================================================================
// Rotation class member function definitions
// ==========================================================================
// Last modified on 1/29/12; 2/29/12; 3/23/12
// =========================================================================

#include <string>
#include <vector>
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "math/mathfuncs.h"
#include "math/rotation.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void rotation::allocate_member_objects()
{
//   cout << "inside rotation::allocate_member_objects()" << endl;
   
   Rx_ptr=new genmatrix(3,3);
   Ry_ptr=new genmatrix(3,3);
   Rz_ptr=new genmatrix(3,3);
}		       

void rotation::initialize_member_objects()
{
}

rotation::rotation():
   genmatrix(3,3)
{
//   cout << "inside rotation constructor #1" << endl;
   allocate_member_objects();
   initialize_member_objects();
   identity();
}

rotation::rotation(const genmatrix& R):
   genmatrix(R)
{
   allocate_member_objects();
   initialize_member_objects();
}

// This next constructor takes in three rotation angles and constructs
// the matrix product Rz(thetaz)*Ry(thetay)*Rx(thetax):

rotation::rotation(double thetax,double thetay,double thetaz):
   genmatrix(3,3)
{
//   cout << "inside rotation(thetax,thetay,thetaz) constructor" << endl;
   
   allocate_member_objects();
   initialize_member_objects();

   generate_Rx(thetax);
   generate_Ry(thetay);
   generate_Rz(thetaz);

//   cout << "*Rx_ptr = " << *Rx_ptr << endl;
//   cout << "*Ry_ptr = " << *Ry_ptr << endl;
//   cout << "*Rz_ptr = " << *Rz_ptr << endl;

   *this = (*Rz_ptr) * (*Ry_ptr) * (*Rx_ptr);
//   cout << "*this = " << *this << endl;
}

// ---------------------------------------------------------------------
// This next constructor builds the 3x3 matrix corresponding to a
// right handed rotation through angle alpha about the axis_direction
// direction vector:

rotation::rotation(const threevector& axis_direction,double alpha):
   genmatrix(3,3)
{
   allocate_member_objects();
   initialize_member_objects();

   threevector n_hat(axis_direction.unitvector());
   double theta=asin(n_hat.dot(z_hat));
   double phi=atan2(n_hat.dot(y_hat),n_hat.dot(x_hat));

   double cosphi=cos(phi);
   double sinphi=sin(phi);
   double costheta=cos(theta);
   double sintheta=sin(theta);

   rotation R;
   
   R.put(0,0,sinphi);
   R.put(0,1,sintheta*cosphi);
   R.put(0,2,costheta*cosphi);
   R.put(1,0,-cosphi);
   R.put(1,1,sintheta*sinphi);
   R.put(1,2,costheta*sinphi);
   R.put(2,0,0);
   R.put(2,1,-costheta);
   R.put(2,2,sintheta);

   generate_Rz(alpha);
   *this=R * (*Rz_ptr) * R.transpose();
}

// ---------------------------------------------------------------------
// This next constructor takes in Uhat, Vhat and What which are
// assumed to form an orthonormal triad.  It enters these 3-vectors
// into the columns of the current rotation matrix.

rotation::rotation(
   const threevector& Uhat,const threevector& Vhat,
   const threevector& What): genmatrix(3,3)
{
   put_column(0,Uhat);
   put_column(1,Vhat);
   put_column(2,What);
}

// ---------------------------------------------------------------------
// Copy constructor

rotation::rotation(const rotation& R):
   genmatrix(3,3)
{
//   cout << "inside rotation copy constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();

   docopy(R);
}

rotation::~rotation()
{
   delete Rx_ptr;
   delete Ry_ptr;
   delete Rz_ptr;
}

// ---------------------------------------------------------------------
void rotation::docopy(const rotation& R)
{
//   cout << "inside rotation::docopy(), input R = " << R << endl;
   genmatrix::docopy(R);
}	

// Overload = operator:

rotation& rotation::operator= (const rotation& R)
{
//   cout << "inside rotation::operator=, input R = " << R << endl;
   if (this==&R) return *this;
   genmatrix::operator=(R);
   docopy(R);

   return *this;
}

rotation& rotation::operator= (const genmatrix& R)
{
//   cout << "inside rotation::operator= (GM R), R = " << R << endl;
   if (this==&R) return *this;
   genmatrix::operator=(R);
   docopy(R);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const rotation& R)
{
   outstream << endl;
   for (unsigned int i=0; i<R.get_mdim(); i++)
   {
      for (unsigned int j=0; j<R.get_ndim(); j++)
      {
         outstream << R.get(i,j) << "\t";
      }
      outstream << endl;
   }
   return outstream;
}

// ==========================================================================
// Rotation generation member functions
// ==========================================================================

void rotation::generate_Rx(double thetax) const
{
//   cout << "inside rotation::generate_Rx(), thetax = " 
//        << thetax*180/PI << endl;
   Rx_ptr->identity();

   double costhetax=cos(thetax);
   double sinthetax=sin(thetax);
   Rx_ptr->put(1,1,costhetax);
   Rx_ptr->put(1,2,-sinthetax);
   Rx_ptr->put(2,1,sinthetax);
   Rx_ptr->put(2,2,costhetax);
//   cout << "*Rx_ptr = " << *Rx_ptr << endl;
}

void rotation::generate_Ry(double thetay) const
{
   Ry_ptr->identity();

   double costhetay=cos(thetay);
   double sinthetay=sin(thetay);
   Ry_ptr->put(0,0,costhetay);
   Ry_ptr->put(0,2,sinthetay);
   Ry_ptr->put(2,0,-sinthetay);
   Ry_ptr->put(2,2,costhetay);
}

void rotation::generate_Rz(double thetaz) const
{
   Rz_ptr->identity();
   
   double costhetaz=cos(thetaz);
   double sinthetaz=sin(thetaz);
   Rz_ptr->put(0,0,costhetaz);
   Rz_ptr->put(0,1,-sinthetaz);
   Rz_ptr->put(1,0,sinthetaz);
   Rz_ptr->put(1,1,costhetaz);
}

// ---------------------------------------------------------------------
// Member function rotation_from_az_el_roll returns the 3x3 matrix
// corresponding to a right-handed rotation about z_hat by az followed
// by a LEFT-handed rotation about y_hat' by el (so that positive
// elevations lift vectors up out of the xy plane) followed by a
// right-handed roll about x_hat''.  All input angles are assumed to
// be in radians.

rotation rotation::rotation_from_az_el_roll(
   double az,double el,double roll,bool enforce_el_limits_flag)
{
//   cout << "inside rotation::rotation_from_az_el_roll()" << endl;
//   cout << "enforce_el_limits_flag = " << enforce_el_limits_flag << endl;

   if (enforce_el_limits_flag && (el < -PI/2 || el > PI/2))
   {
      cout << "Trouble in rotation::rotation_from_az_el_roll!" << endl;
      cout << "elevation angle = " << el*180/PI << " degs";
      cout << " should not lie outside interval [-90,90]" << endl;
      el=basic_math::min(PI/2,el);
      el=basic_math::max(-PI/2,el);
   }

   generate_Rx(roll);
   generate_Ry(-el);
   generate_Rz(az);

   return (*Rz_ptr) * (*Ry_ptr) * (*Rx_ptr);
}

// ---------------------------------------------------------------------
// Member function rotation_about_nhat_by_theta returns the matrix
// which performs a finite rotation about an arbitrary axis labeled by
// input direction vector n_hat.
   
rotation& rotation::rotation_about_nhat_by_theta(
   double theta,const threevector& n_hat)
{
   double costheta=cos(theta);
   double sintheta=sin(theta);

   double n0sqr=sqr(n_hat.get(0));
   double n1sqr=sqr(n_hat.get(1));
   double n2sqr=sqr(n_hat.get(2));
   double n0n1=n_hat.get(0)*n_hat.get(1);
   double n1n2=n_hat.get(1)*n_hat.get(2);
   double n2n0=n_hat.get(2)*n_hat.get(0);
         
   identity();

   put(0,0,n0sqr+costheta*(n1sqr+n2sqr));
   put(1,1,n1sqr+costheta*(n0sqr+n2sqr));
   put(2,2,n2sqr+costheta*(n0sqr+n1sqr));

   put(0,1,(1-costheta)*n0n1-sintheta*n_hat.get(2));
   put(1,0,(1-costheta)*n0n1+sintheta*n_hat.get(2));

   put(1,2,(1-costheta)*n1n2-sintheta*n_hat.get(0));
   put(2,1,(1-costheta)*n1n2+sintheta*n_hat.get(0));

   put(0,2,(1-costheta)*n2n0+sintheta*n_hat.get(1));
   put(2,0,(1-costheta)*n2n0-sintheta*n_hat.get(1));

//         cout << "R = " << R << endl;
//         cout << "det = " <<  R.determinant() << endl;

   return *this;
}

// ---------------------------------------------------------------------
// Member function rotation_taking_u_to_v takes in vectors u and v
// which are NOT assumed to be normalized.  Following our notes
// "Finite rotation about an arbitrary axis" dated 12/15/04, we
// compute a rotation matrix R(n_hat,theta) which maps u to v.  This
// rotation is NOT unique.  It can be multiplied by another other
// rotation that spins about v after u is mapped to v.

rotation& rotation::rotation_taking_u_to_v(
   const threevector& u,const threevector& v,double TINY)
{
//   cout << "inside rotation::rotation_taking_u_to_v()" << endl;
   
   threevector u_hat(u.unitvector());
   threevector v_hat(v.unitvector());
         
   double dotproduct=u_hat.dot(v_hat);
   if (::nearly_equal(dotproduct,1.0,TINY))
   {
      identity();
      return *this;
   }
   else if (::nearly_equal(dotproduct,-1.0))
   {
      cout << "Error in rotation::rotation_taking_u_to_v()" << endl;
      cout << "Insufficient information in order to flip u to v ..."
           << endl;
      exit(-1);
   }
   
   threevector cross_product(u_hat.cross(v_hat));
   threevector n_hat(cross_product.unitvector());

// On 6/28/05, we double-checked that we can compute angle theta
// without loss of generality using sintheta_mag which is guaranteed
// to be positive!

   double costheta=u_hat.dot(v_hat);
   double sintheta_mag=cross_product.magnitude();
   double theta=atan2(sintheta_mag,costheta);
//   double sintheta=sin(theta);

   return rotation_about_nhat_by_theta(theta,n_hat);
}

// ---------------------------------------------------------------------
// Member function rotation_taking_pqr_to_uvw takes input orthonormal basis
// vectors (p,q,r) and output orthonormal basis vectors (u,v,w).  It
// returns the 3x3 rotation matrix which maps p->u, q->v and r->w.

rotation& rotation::rotation_taking_pqr_to_uvw(
   const threevector& p,const threevector& q,const threevector& r,
   const threevector& u,const threevector& v,const threevector& w)
{
   rotation Ruvw,Rpqr;
         
   Ruvw.put(0,0,u.get(0));
   Ruvw.put(1,0,u.get(1));
   Ruvw.put(2,0,u.get(2));

   Ruvw.put(0,1,v.get(0));
   Ruvw.put(1,1,v.get(1));
   Ruvw.put(2,1,v.get(2));

   Ruvw.put(0,2,w.get(0));
   Ruvw.put(1,2,w.get(1));
   Ruvw.put(2,2,w.get(2));
         

   Rpqr.put(0,0,p.get(0));
   Rpqr.put(1,0,p.get(1));
   Rpqr.put(2,0,p.get(2));

   Rpqr.put(0,1,q.get(0));
   Rpqr.put(1,1,q.get(1));
   Rpqr.put(2,1,q.get(2));

   Rpqr.put(0,2,r.get(0));
   Rpqr.put(1,2,r.get(1));
   Rpqr.put(2,2,r.get(2));
         
   *this=Ruvw * Rpqr.transpose();
   return *this;
}

// -------------------------------------------------------------------------
// Member function rotation_between_ray_bundles() implements the ray
// approach described in section 8 of "Minimal solutions for panoramic
// stitching" by Brown, Hartley and Nister to estimate the relative
// rotation between two sets of corresponding ray bundles.  The
// returned rotation (approximately) transforms ray_bundle_init to
// ray_bundle_final.

void rotation::rotation_between_ray_bundles(
   const vector<threevector>& ray_bundle_final,
   const vector<threevector>& ray_bundle_init)
{
//   cout << "inside rotation::rotation_between_ray_bundles()" << endl;

   genmatrix Correlation(3,3);
   Correlation.clear_values();
   for (unsigned int r=0; r<ray_bundle_final.size(); r++)
   {
      Correlation += ray_bundle_final[r].outerproduct(ray_bundle_init[r]);
   } // loop over index r labeling rays

   genmatrix U(3,3),W(3,3),V(3,3);
   Correlation.sorted_singular_value_decomposition(U,W,V);
   double determinant=(U*V.transpose()).determinant();
   double s=sgn( determinant );
   W.clear_values();
   W.identity();
   W.put(2,2,s);
   *this=U*W*V.transpose();
}

// ==========================================================================
// Rotation decomposition member functions
// ==========================================================================

// Member function rotation_corresponding_to_quaterion performs the
// inverse operation to quaternion_corresponding_to_rotation.

// Note added on 1/20/09: The 3x3 matrix returned by this method is a
// general orthogonal matrix which may have determinant = -1!

// Recall q and -q both yield the same 3x3 rotation matrix!

rotation& rotation::rotation_corresponding_to_quaternion(const fourvector& q)
{
   double q0=q.get(0);
   double q1=q.get(1);
   double q2=q.get(2);
   double q3=q.get(3);

   identity();
         
   put(0,0,1-2*(q2*q2+q3*q3));
   put(0,1,2*(q1*q2-q0*q3));
   put(0,2,2*(q0*q2+q1*q3));

   put(1,0,2*(q1*q2+q0*q3));
   put(1,1,1-2*(q1*q1+q3*q3));
   put(1,2,2*(q2*q3-q0*q1));

   put(2,0,2*(q1*q3-q0*q2));
   put(2,1,2*(q0*q1+q2*q3));
   put(2,2,1-2*(q1*q1+q2*q2));

   return *this;
}

// ---------------------------------------------------------------------
// Member function quaternion_corresponding_to_rotation returns
// fourvector q containing quaternion coords corresponding to the
// current rotation.  See "Answers.com: conversion between quaternions
// and Euler angles".

// Algorithm described in
// http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation and
// implemented on 1/20/09:

// Recall q and -q both map to R !

fourvector rotation::quaternion_corresponding_to_rotation()
{
//   cout << "inside rotation::quaternion_corresponding_to_rotation()" << endl;
//   cout << "det = " << determinant() << endl;

/*
   double d0=get(0,0);
   double d1=get(1,1);
   double d2=get(2,2);
//   cout << "d0 = " << d0 << " d1 = " << d1 << " d2 = " << d2 << endl;

   int u,v,w;
   if (fabs(d0) > fabs(d1) && fabs(d0) > fabs(d2))
   {
      u=0;
      v=1;
      w=2;
   }
   else if (fabs(d1) > fabs(d0) && fabs(d1) > fabs(d2))
   {
      u=1;
      v=2;
      w=0;
   }
   else
   {
      u=2;
      v=0;
      w=1;
   }
//   cout << "u = " << u << " v = " << v << " w = " << w << endl;

   double r=sqrt(1+get(u,u)-get(v,v)-get(w,w));
   double q[4];
   q[0]=(get(w,v)-get(v,w))/(2*r);
   q[u+1]=r/2;
   q[v+1]=(get(u,v)+get(v,u))/(2*r);
   q[w+1]=(get(w,u)+get(u,w))/(2*r);

   fourvector quat(q[0],q[1],q[2],q[3]);
//   cout << "quat = " << quat << endl;

   return quat;
*/

//   cout << "R(quat) = " 
//        << rotation_corresponding_to_quaternion(quat) << endl;

// Older Answers.com algorithm for computing q from R whose absolute
// sign is ambiguous due to double cover of SO(3) by SU(2)

   double chi;
   threevector nhat;
   mathfunc::decompose_orthogonal_matrix(*this,chi,nhat);
//   cout << "nhat = " << nhat << " chi = " << chi*180/PI << endl;
   
//   rotation Rnew;
//   mathfunc::construct_orthogonal_matrix(1,nhat,chi,Rnew);
//   cout << "*this - Rnew = " << *this-Rnew << endl;
   
   fourvector q;
   q.put(0,cos(0.5*chi));
   q.put(1,sin(0.5*chi)*nhat.get(0));
   q.put(2,sin(0.5*chi)*nhat.get(1));
   q.put(3,sin(0.5*chi)*nhat.get(2));
//   cout << "q = " << q << endl;

   return q;
}

// ---------------------------------------------------------------------
// Member function az_el_roll_from_rotation recovers the azimuth,
// elevation and roll angles in radians from 3x3 rotation matrix R.
// See "Answers.com: conversion between quaternions and Euler angles".
// Note that our az = answer.com's psi, our el = - answer.com's theta
// and our roll = answer.com's phi.

// On 10/3/11, we explicitly confirmed the decomposition:

// 		R = Rz(0,0,az) * Ry(0,-el,0) * Rx(roll,0,0)

void rotation::az_el_roll_from_rotation(
   double& az,double& el,double& roll)
{
//   cout << "inside rotation::az_el_roll_from_rotation()" << endl;

   fourvector q=quaternion_corresponding_to_rotation();
//   cout << "q = " << q << endl;
   mathfunc::az_el_roll_corresponding_to_quaternion(q,az,el,roll);

//   cout << "az = " << az*180/PI 
//        << " el = " << el*180/PI << " roll = " << roll*180/PI << endl;
}

// ---------------------------------------------------------------------
bool rotation::rotation_sanity_check() const
{
//   cout << "inside rotation::rotation_sanity_check()" << endl;
   bool valid_rotation=true;

// Important note: On 10/28/08, we realized the painful way that the
// nearly_equal() member function of Tensor() can be confused with the
// nearly_equal basic_math method.  To force the latter to be called,
// we must explicitly call ::nearly_equal().

   if (!::nearly_equal(determinant(),1))
   {
      valid_rotation=false;
   }
   
   genmatrix identity(3,3);
   identity=(*this)*(*this).transpose();

   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         if (i==j)
         {
            if (!::nearly_equal(identity.get(i,j),1)) 
            {
               valid_rotation=false;
            }
         }
         else
         {
            if (!::nearly_equal(identity.get(i,j),0))
            { 
               valid_rotation=false;
            }
         }
      }
   }
   if (!valid_rotation)
   {
      string banner=
         "Current matrix R does NOT correspond to a genuine rotation!";
      outputfunc::write_big_banner(banner);
   }
   return valid_rotation;
}

// ==========================================================================
// OSG conventions conversion member functions
// ==========================================================================

// Member function rotation_corresponding_to_OSG_quat() takes in a
// fourvector and swaps its first and fourth components.  Recall that
// OSG's quaternion has the structure (q_x, q_y, q_z, q_w) whereas our
// quaternions have the structure (q_w, q_x, q_y, q_z).  So this
// method converts from OSG's quaternion convention to ours and
// vice-versa.

fourvector rotation::quat_corresponding_to_OSG_quat(const fourvector& q_OSG)
{
   fourvector q=q_OSG;
   q.put(0,q_OSG.get(3));
   q.put(3,q_OSG.get(0));
   return q;
}

// ---------------------------------------------------------------------
// Member function rotation_corresponding_to_OSG_quat() takes in a
// fourvector representing a quaternion according to OSG's (q_x, q_y,
// q_z, q_w) convention.  It returns a 3x3 rotation matrix in our
// x_hat, y_hat, z_hat basis (which differs from OSG's rotation matrix
// convention by a transpose).

rotation rotation::rotation_corresponding_to_OSG_quat(const fourvector& q_OSG)
{
   fourvector q=quat_corresponding_to_OSG_quat(q_OSG);
   rotation R;
   R=R.rotation_corresponding_to_quaternion(q);
   return R;
}

// ---------------------------------------------------------------------
// Member function OSG_rotation_corresponding_rotation()

rotation rotation::OSG_rotation_corresponding_to_rotation()
{
   return this->transpose();
}

// ---------------------------------------------------------------------
// Member function OSG_quat_corresponding_to_rotation() returns a
// fourvector representing a quaternion in OSG's (q_x, q_y, q_z, q_w)
// convention corresponding to the current rotation object.

fourvector rotation::OSG_quat_corresponding_to_rotation()
{
   fourvector q=quaternion_corresponding_to_rotation();
   fourvector q_OSG=quat_corresponding_to_OSG_quat(q);
//   cout << "q_OSG = " << q_OSG << endl;
   return q_OSG;
}

// ==========================================================================
// Rotation derivative member functions
// ==========================================================================

// Note: First derivatives of 3x3 rotation matrices are NOT members of
// SO(3).  Their determinants equal 0!  Nevertheless, we choose to
// return derivative information within 3x3 rotation matrix output.

rotation rotation::dRdx(double thetax) const
{
//   cout << "inside rotation::dRdx, thetax = " << thetax << endl;
   
   rotation dRdx;

   double costhetax=cos(thetax);
   double sinthetax=sin(thetax);
   dRdx.put(1,1,-sinthetax);
   dRdx.put(1,2,-costhetax);
   dRdx.put(2,1,costhetax);
   dRdx.put(2,2,-sinthetax);
   dRdx.put(0,0,0);

//   cout << "dRdx = " << dRdx << endl;
   return dRdx;
}

rotation rotation::dRdy(double thetay) const
{
   rotation dRdy;

   double costhetay=cos(thetay);
   double sinthetay=sin(thetay);
   dRdy.put(0,0,-sinthetay);
   dRdy.put(0,2,costhetay);
   dRdy.put(2,0,-costhetay);
   dRdy.put(2,2,-sinthetay);
   dRdy.put(1,1,0);
   return dRdy;
}

rotation rotation::dRdz(double thetaz) const
{
   rotation dRdz;

   double costhetaz=cos(thetaz);
   double sinthetaz=sin(thetaz);
   dRdz.put(0,0,-sinthetaz);
   dRdz.put(0,1,-costhetaz);
   dRdz.put(1,0,costhetaz);
   dRdz.put(1,1,-sinthetaz);
   dRdz.put(2,2,0);
   return dRdz;
}

rotation rotation::dRdaz(double az,double el,double roll)
{
//   cout << "inside rotation::dRdaz()" << endl;
   generate_Ry(-el);
   generate_Rx(roll);
   rotation dRdaz = dRdz(az) * (*Ry_ptr) * (*Rx_ptr);
//   cout << "dRdaz = " << dRdaz << endl;
   return dRdaz;
}

rotation rotation::dRdel(double az,double el,double roll)
{
   generate_Rz(az);
   generate_Rx(roll);
   rotation dRdel = - (*Rz_ptr) * dRdy(-el) * (*Rx_ptr);
   return dRdel;
}

rotation rotation::dRdroll(double az,double el,double roll)
{
//   cout << "inside rotation::dRdroll, roll = " << roll << endl;
   generate_Rz(az);
   generate_Ry(-el);
   rotation dRdroll = (*Rz_ptr) * (*Ry_ptr) * dRdx(roll);
   return dRdroll;
}


// ==========================================================================
// Note: Keyword friend should appear in class declaration file and not 
// within class member function definition file.  Friendly functions should
// not be declared as member functions of a class.  So their definition syntax
// is not 

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list)
// ==========================================================================

// ==========================================================================
// Friend methods
// ==========================================================================


// ==========================================================================
// Overload * operator for multiplying a rotation by another rotation
// ==========================================================================

rotation operator* (const rotation& A,const rotation& B)
{
   rotation C;
   C.clear_values();
	 
   for (unsigned int i=0; i<A.get_mdim(); i++)
   {
      for (unsigned int j=0; j<B.get_ndim(); j++)
      {
         for (unsigned int k=0; k<A.get_ndim(); k++)
         {
            C.put(i,j,C.get(i,j)+A.get(i,k)*B.get(k,j));
         }
      }
   }
   return C;
}

// Note added on Monday Sep 26, 2011:

// Following overloads over operator* only work if A [B] is a 3x3
// genmatrix !!!

rotation operator* (const genmatrix& A,const rotation& B)
{
   rotation C;
   C.clear_values();
	 
   for (unsigned int i=0; i<A.get_mdim(); i++)
   {
      for (unsigned int j=0; j<B.get_ndim(); j++)
      {
         for (unsigned int k=0; k<A.get_ndim(); k++)
         {
            C.put(i,j,C.get(i,j)+A.get(i,k)*B.get(k,j));
         }
      }
   }
   return C;
}

rotation operator* (const rotation& A,const genmatrix& B)
{
   rotation C;
   C.clear_values();
	 
   for (unsigned int i=0; i<A.get_mdim(); i++)
   {
      for (unsigned int j=0; j<B.get_ndim(); j++)
      {
         for (unsigned int k=0; k<A.get_ndim(); k++)
         {
            C.put(i,j,C.get(i,j)+A.get(i,k)*B.get(k,j));
         }
      }
   }
   return C;
}

threevector operator* (const rotation& A,const threevector& V)
{
   threevector W;
	 
   for (unsigned int i=0; i<A.get_mdim(); i++)
   {
      for (unsigned int j=0; j<A.get_ndim(); j++)
      {
         W.put(i,W.get(i)+A.get(i,j)*V.get(j));
      }
   }
   return W;
}
