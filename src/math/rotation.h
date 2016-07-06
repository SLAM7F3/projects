// ==========================================================================
// Header file for rotation class 
// ==========================================================================
// Last modified on 8/18/09; 8/3/10; 1/29/12; 3/15/13
// ==========================================================================

#ifndef ROTATION_H
#define ROTATION_H

#include "math/genmatrix.h"
#include "math/fourvector.h"
#include "math/threevector.h"

class rotation:public genmatrix
{

  public:

// Initialization, allocation and construction functions

   rotation();
   rotation(const genmatrix& R);
   rotation(double thetax,double thetay,double thetaz);
   rotation(const threevector& axis_direction,double alpha);
   rotation(const threevector& Uhat,const threevector& Vhat,
            const threevector& What);
   rotation(const rotation& R);
   ~rotation();

   rotation& operator= (const rotation& R);
   rotation& operator= (const genmatrix& R);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const rotation& R); 

// Rotation generation member functions:

   rotation rotation_from_az_el_roll(
      double az,double el,double roll,bool enforce_el_limits_flag=true);
   rotation& rotation_corresponding_to_quaternion(const fourvector& q);

   rotation& rotation_about_nhat_by_theta(
      double theta,const threevector& n_hat);

   rotation& rotation_taking_u_to_v(
      const threevector& u,const threevector& v,double TINY=1.0E-5);
   rotation& rotation_taking_pqr_to_uvw(
      const threevector& p,const threevector& q,const threevector& r,
      const threevector& u,const threevector& v,const threevector& w);
   void rotation_between_ray_bundles(
      const std::vector<threevector>& ray_bundle_final,
      const std::vector<threevector>& ray_bundle_init);
      
// OSG convention conversion member functions:

   fourvector quat_corresponding_to_OSG_quat(const fourvector& q_OSG);
   rotation rotation_corresponding_to_OSG_quat(const fourvector& q_OSG);
   rotation OSG_rotation_corresponding_to_rotation();
   fourvector OSG_quat_corresponding_to_rotation();

// Rotation decomposition member functions:

   fourvector quaternion_corresponding_to_rotation();
   void az_el_roll_from_rotation(double& az,double& el,double& roll);
   bool rotation_sanity_check() const;

// Derivative member functions:

   rotation dRdx(double thetax) const;
   rotation dRdy(double thetay) const;
   rotation dRdz(double thetaz) const;

   rotation dRdaz(double az,double el,double roll);
   rotation dRdel(double az,double el,double roll);
   rotation dRdroll(double az,double el,double roll);

// Friend methods:

   friend rotation operator* (const rotation& A,const rotation& B);
   friend rotation operator* (const genmatrix& A,const rotation& B);
   friend rotation operator* (const rotation& A,const genmatrix& B);
   friend threevector operator* (const rotation& A,const threevector& V);

  protected:

  private: 

   genmatrix* Rx_ptr;
   genmatrix* Ry_ptr;
   genmatrix* Rz_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const rotation& r);

   void generate_Rx(double thetax) const;
   void generate_Ry(double thetay) const;
   void generate_Rz(double thetaz) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif  // math/rotation.h




