// =========================================================================
// Header file for stand-alone satellite motion methods.
// =========================================================================
// Last modified on 6/13/03; 7/20/06; 1/29/12
// =========================================================================

#ifndef MOTIONFUNCS_H
#define MOTIONFUNCS_H

class mypoint;
class rotation;
class statevector;
class threevector;
class genmatrix;

namespace motionfunc
{
   enum Motion_type
   {
      earthstable,sunsync
   };

   enum Imagery_motion_type
   {
      general_ypr,prosol,dynamic_motion
   };

   void range_vector_in_body_IGES_basis(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,threevector& range_vector);
   void Ushomirsky_LHS_matrix(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,double roll,genmatrix& LHS);
   void Ushomirsky_LHS_matrix(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,double roll,
      genmatrix& LHS,genmatrix& dLHS_droll);

   void abcs_from_roll_pitch_and_derivs(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,const mypoint& xIGES_hat,
      const mypoint& yIGES_hat,const mypoint& zIGES_hat,
      double roll,double pitch,double roll_dot,double pitch_dot,
      double& thetaIGES_x,double& thetaIGES_y,double& thetaIGES_z,double& s);
   
   void s_and_phi_from_roll_pitch_and_derivs(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,
      double roll,double roll_dot,double pitch_dot,double& s,double& phi);
   void Kabc_from_roll_pitch_phi(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,double roll,double pitch,double phi);
   void body_to_imageframe_rotation(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,rotation& R_imageframe);
   void phi_rotation_matrix(double phi,rotation& Gimageframe);
   double angle_between_stable_and_maneuvering_imageplanes(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      Motion_type currmotion,const rotation& Ktotal,double roll,double pitch);

   void allocate_partial_deriv_arrays(
      int istart,int number_of_images,genmatrix* partials_ptr[]);
   void delete_partial_deriv_arrays(
      int istart,int number_of_images,genmatrix* partials_ptr[]);
}

#endif // motionfuncs.h



