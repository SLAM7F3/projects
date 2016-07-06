// ==========================================================================
// MOTIONFUNC stand-alone methods
// ==========================================================================
// Last modified on 3/4/04; 7/20/06; 1/29/12
// ==========================================================================

#include "math/basic_math.h"
#include "math/genmatrix.h"
#include "space/motionfuncs.h"
#include "geometry/mypoint.h"
#include "math/rotation.h"
#include "space/satellitefuncs.h"
#include "math/statevector.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

namespace motionfunc
{

// Method range_vector_in_body_IGES_basis returns the components of
// the range vector in the nominally oriented body IGES basis:

   void range_vector_in_body_IGES_basis(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,threevector& range_vector)
      {
         threevector xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0;
         satellitefunc::nominal_body_IGES_direction_vectors(
            satellite_statevector,radar_statevector,currmotion,
            xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0);
         range_vector=threevector(
            xhat_body_IGES0.get(1),yhat_body_IGES0.get(1),
            zhat_body_IGES0.get(1));
      }

// ---------------------------------------------------------------------
// Method Ushomirsky_LHS_matrix takes in a roll angle (in degs) for
// the satellite wireframe model in image i.  It returns the 2x2
// matrix which relates rolldot and pitchdot to cross range scale
// factor phi and maneuvering image plane azimuthal angle phi.  See
// Ushormisky's notes "Incorporating s/pdot & rdot calculation" (dated
// 10/4/02) for derivation of this matrix.  The overloaded version of
// this method also returns the partial derivative of the LHS matrix
// with respect to roll.

   void Ushomirsky_LHS_matrix(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,double roll,genmatrix& LHS)
      {
         genmatrix dLHS_droll(2,2);
         Ushomirsky_LHS_matrix(
            satellite_statevector,radar_statevector,currmotion,roll,
            LHS,dLHS_droll);
      }

   void Ushomirsky_LHS_matrix(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,double roll,genmatrix& LHS,genmatrix& dLHS_droll)
      {
// Calculate nominal body axes in image frame coordinates which are
// defined by sun-sync motion:

         threevector xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0;
         satellitefunc::nominal_body_IGES_direction_vectors(
            satellite_statevector,radar_statevector,currmotion,
            xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0);

         double r=roll*PI/180;	// radians
         LHS.put(0,0,-cos(r)*xhat_body_IGES0.get(0)
                 -sin(r)*yhat_body_IGES0.get(0));
         LHS.put(0,1,zhat_body_IGES0.get(0));
         LHS.put(1,0,-cos(r)*xhat_body_IGES0.get(2)
                 -sin(r)*yhat_body_IGES0.get(2));
         LHS.put(1,1,zhat_body_IGES0.get(2));

         dLHS_droll.put(0,0,sin(r)*xhat_body_IGES0.get(0)
                        -cos(r)*yhat_body_IGES0.get(0));
         dLHS_droll.put(0,1,0);
         dLHS_droll.put(1,0,sin(r)*xhat_body_IGES0.get(2)
                        -cos(r)*yhat_body_IGES0.get(2));
         dLHS_droll.put(1,1,0);
      }

// ---------------------------------------------------------------------
// Method abcs_from_roll_pitch_and_derivs takes in physical satellite
// motion variables roll, pitch and their time derivatives.  It
// returns the corresponding non-physical wireframe parameters alpha,
// beta, gamma and scale factor s.

   void abcs_from_roll_pitch_and_derivs(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,const mypoint& xIGES_hat,
      const mypoint& yIGES_hat,const mypoint& zIGES_hat,
      double roll,double pitch,double roll_dot,double pitch_dot,
      double& thetaIGES_x,double& thetaIGES_y,double& thetaIGES_z,double& s)
      {
         rotation Rmaneuver_body;
         satellitefunc::roll_pitch_maneuver_body_rot(
            roll,pitch,Rmaneuver_body);

         double phi;
         s_and_phi_from_roll_pitch_and_derivs(
            satellite_statevector,radar_statevector,currmotion,
            roll,roll_dot,pitch_dot,s,phi);
   
         rotation Gimageframe;
         phi_rotation_matrix(phi,Gimageframe);
         rotation R_imageframe;
         body_to_imageframe_rotation(
            satellite_statevector,radar_statevector,currmotion,R_imageframe);
         rotation Gbody=R_imageframe.transpose()*Gimageframe*R_imageframe;

         rotation K_body=Gbody.transpose()*Rmaneuver_body;
         rotation K_imageframe=R_imageframe*K_body*R_imageframe.transpose();

         satellitefunc::IGES_angles(
            K_imageframe,satellitefunc::IGES_to_imageframe_rot(
               xIGES_hat.get_pnt(),yIGES_hat.get_pnt(),zIGES_hat.get_pnt()),
            thetaIGES_x,thetaIGES_y,thetaIGES_z);
      }

// ---------------------------------------------------------------------
// Method function s_and_phi_from_roll_pitch_and_derivs computes the
// scalefactor s and maneuvering image plane azimuthal angle phi given
// satellite and radar statevector information, the roll for RH (in
// degs) along with its pitchdot and rolldot time derivatives (in
// degs/sec).  It returns scalefactor s which equals the range factor
// by which model is re-scaled.  It also returns azimuthal angle phi
// by which qhat_stable must be rotataed about rhat in order to become
// qhat_maneuvering.

   void s_and_phi_from_roll_pitch_and_derivs(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,Motion_type currmotion,
      double roll,double roll_dot,double pitch_dot,double& s,double& phi)
      {
         genmatrix LHS(2,2);
         Ushomirsky_LHS_matrix(
            satellite_statevector,radar_statevector,currmotion,roll,LHS);
         twovector rates(pitch_dot*PI/180,roll_dot*PI/180);

// See Ushomirsky's notes entitled "Angular velocity of the panel wrt
// ECI for RH", dated 2/19/03 for the derivation of the following
// correction terms:

         threevector omega_orbital,omega_spin_earthstable;
         satellitefunc::compute_angular_velocities(
            satellite_statevector,radar_statevector,omega_orbital,
            omega_spin_earthstable);
         double omega_es=omega_spin_earthstable.magnitude();

// Calculate nominal body axes in image frame coordinates which are
// defined by sun-sync motion:

         threevector xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0;
         satellitefunc::nominal_body_IGES_direction_vectors(
            satellite_statevector,radar_statevector,currmotion,
            xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0);

         double correction_term0=(cos(roll*PI/180)-1)*xhat_body_IGES0.get(0)
            +sin(roll*PI/180)*yhat_body_IGES0.get(0);
         double correction_term1=(cos(roll*PI/180)-1)*xhat_body_IGES0.get(2)
            +sin(roll*PI/180)*yhat_body_IGES0.get(2);

         twovector correction(correction_term0,correction_term1);
         twovector transformed_rates=LHS*rates-omega_es*correction;

         threevector qvec_nominal;
         satellitefunc::compute_nominal_crossrange_vector(
            satellite_statevector,radar_statevector,currmotion,qvec_nominal);
         double q=qvec_nominal.magnitude();
         transformed_rates *= 1.0/q;

         s=sqrt(sqr(transformed_rates.get(0))+sqr(transformed_rates.get(1)+1));
         double sinphi=transformed_rates.get(0)/s;
         double cosphi=(transformed_rates.get(1)+1)/s;
         phi=atan2(sinphi,cosphi);
         phi=basic_math::phase_to_canonical_interval(phi,-PI,PI);
      }

/*
// Old version prior to Ushomirsky's Feb 19, 2003 correction:

   void s_and_phi_from_roll_pitch_and_derivs(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,
      double roll,double roll_dot,double pitch_dot,double& s,double& phi)
      {
         genmatrix LHS(2,2);
         Ushomirsky_LHS_matrix(
            satellite_statevector,radar_statevector,currmotion,roll,LHS);

// See Ushomirsky's notes entitled "Angular velocity of the panel wrt
// ECI for RH", dated 2/19/03 for the derivation of the following
// correction terms:

         threevector omega_orbital,omega_spin_earthstable;
         satellitefunc::compute_angular_velocities(
            satellite_statevector,radar_statevector,omega_orbital,
            omega_spin_earthstable);
         double omega_es=omega_spin_earthstable.magnitude();
         double correction_term0=(cos(roll*PI/180)-1)*xhat_body_IGES0.get(0)
            -sin(roll*PI/180)*yhat_body_IGES0.get(0);
         double correction_term1=(cos(roll*PI/180)-1)*xhat_body_IGES0.get(2)
            -sin(roll*PI/180)*yhat_body_IGES0.get(2);


         threevector qvec_nominal;
         satellitefunc::compute_nominal_crossrange_vector(
            satellite_statevector,radar_statevector,currmotion,qvec_nominal);
         double q=qvec_nominal.magnitude();
         LHS *= 1.0/q;
         twovector rates(pitch_dot*PI/180,roll_dot*PI/180);
         twovector transformed_rates=LHS*rates;

         s=sqrt(sqr(transformed_rates.get(0))+sqr(transformed_rates.get(1)+1));
         double sinphi=transformed_rates.get(0)/s;
         double cosphi=(transformed_rates.get(1)+1)/s;
         phi=atan2(sinphi,cosphi);
         phi=mathfunc::phase_to_canonical_interval(phi,-PI,PI);
      }
*/

// ---------------------------------------------------------------------
   void Kabc_from_roll_pitch_phi(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,double roll,double pitch,double phi)
      {
         rotation Rmaneuver_body;
         satellitefunc::roll_pitch_maneuver_body_rot(
            roll,pitch,Rmaneuver_body);

         rotation Gimageframe;
         phi_rotation_matrix(phi,Gimageframe);
         rotation R_imageframe;
         body_to_imageframe_rotation(
            satellite_statevector,radar_statevector,currmotion,R_imageframe);
      }


// ---------------------------------------------------------------------
// Method body_to_imageframe_rotation returns the rotation matrix
// which transfroms from the RH body frame (spanned by the body IGES
// axes) to the image frame (spanned by the nominal qhat, rhat and
// phat direction vectors).  This matrix is a minor variant of
// R0_rotate calculated in rhsatellite::construct_rotation_matrices().

   void body_to_imageframe_rotation(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,rotation& R_imageframe)
      {
// First calculate nominal body axes in screen coordinates which are
// defined by sun-sync motion.  Recall that the RH body obeys earth
// stable motion!

         threevector xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0;
         satellitefunc::nominal_body_IGES_direction_vectors(
            satellite_statevector,radar_statevector,currmotion,
            xhat_body_IGES0,yhat_body_IGES0,zhat_body_IGES0);

         R_imageframe.put(0,0,xhat_body_IGES0.get(0));
         R_imageframe.put(1,0,xhat_body_IGES0.get(1));
         R_imageframe.put(2,0,xhat_body_IGES0.get(2));
         R_imageframe.put(0,1,yhat_body_IGES0.get(0));
         R_imageframe.put(1,1,yhat_body_IGES0.get(1));
         R_imageframe.put(2,1,yhat_body_IGES0.get(2));
         R_imageframe.put(0,2,zhat_body_IGES0.get(0));
         R_imageframe.put(1,2,zhat_body_IGES0.get(1));
         R_imageframe.put(2,2,zhat_body_IGES0.get(2));
      }

// ---------------------------------------------------------------------
// Method phi_rotation_matrix computes the rotation matrix about the
// range direction vector which relates the stable and maneuvering
// image planes.  This matrix is returned in the imageframe basis
// (spanned by the nominal motion vectors qhat,rhat and phat).

   void phi_rotation_matrix(double phi,rotation& Gimageframe)
      {
         double cosphi=cos(phi);
         double sinphi=sin(phi);
         Gimageframe.put(0,0,cosphi);
         Gimageframe.put(0,2,sinphi);
         Gimageframe.put(2,0,-sinphi);
         Gimageframe.put(2,2,cosphi);
         Gimageframe.put(1,1,1);
      }

// ---------------------------------------------------------------------
// Method angle_between_stable_and_maneuvering_imageplanes implements
// Greg Ushomirsky's method for calculating the azimuthal angle phi
// between the stable, sun-sync cross range direction vector and the
// maneuvering cross range direction vector.  It takes in matix Ktotal
// which represents the "alpha,beta,gamma" wireframe rotation in the
// image frame basis (spanned by sun-sync direction vectors
// qhat,rhat,phat).  It also takes in roll and pitch in degrees as
// returned by satellite::calculate_raw_pitch_roll_values().  This
// method then returns the azimuthal angle between the stable and
// maneuvering image planes in radians.

   double angle_between_stable_and_maneuvering_imageplanes(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,const rotation& Ktotal,double roll,double pitch)
      {
         rotation R_imageframe;
         body_to_imageframe_rotation(
            satellite_statevector,radar_statevector,currmotion,R_imageframe);
         rotation Ktotal_body=R_imageframe.transpose()*Ktotal*R_imageframe;

         rotation Rmaneuver_body;
         satellitefunc::roll_pitch_maneuver_body_rot(
            roll,pitch,Rmaneuver_body);
   
         rotation Gbody=Rmaneuver_body*Ktotal_body.transpose();
         rotation Gimageframe=R_imageframe*Gbody*R_imageframe.transpose();
         double cosphi=Gimageframe.get(0,0);
         double sinphi=Gimageframe.get(0,2);
         double phi=atan2(sinphi,cosphi);

//   cout << "Gimageframe = " << Gimageframe << endl; cout << "phi = "
//   << phi*180/PI << endl;
         return phi;
      }

// ---------------------------------------------------------------------
// Methods allocate_partial_derivs_arrays and
// destroy_partial_deeriv_arrays dynamically creates and destroys an
// array of 5x4 genmatrices to hold partial derivatives of phi, roll,
// pitch, rolldot and pitchdot with respect to alpha, beta, gamma and
// s for each image in the current pass.

   void allocate_partial_deriv_arrays(
      int istart,int number_of_images,genmatrix* partials_ptr[])
      {
         for (int i=istart; i<number_of_images; i++)
         {
            partials_ptr[i]=new genmatrix(5,4);
         }
      }

   void delete_partial_deriv_arrays(
      int istart,int number_of_images,genmatrix* partials_ptr[])
      {
         for (int i=istart; i<number_of_images; i++)
         {
            delete partials_ptr[i];
         }
      }

} // motionfunc namespace





