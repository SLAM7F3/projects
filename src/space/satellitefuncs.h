// =========================================================================
// Header file for stand-alone satellite functions.
// =========================================================================
// Last modified on 7/28/06; 7/31/06; 1/29/12
// =========================================================================

#ifndef SATELLITEFUNCS_H
#define SATELLITEFUNCS_H

#include "space/motionfuncs.h"

class mypoint;
class statevector;
class rotation;
class threevector;

namespace satellitefunc
{
  
// =====================================================================
// Pass initialization methods
// =====================================================================

   int select_starting_image(
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber);

// =====================================================================
// Image frame initialization methods
// =====================================================================

   void compute_range_unitvector(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,threevector& rhat);
   void compute_angular_velocities(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      threevector& omega_orbital,threevector& omega_spin_earthstable);
   void compute_nominal_crossrange_vector(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& curr_qvec);
   void compute_qhat_rhat_phat_in_ECI_coords(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& qhat,threevector& rhat,threevector& phat);

   void construct_imageframe_rotation_matrix(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      rotation& R0_rotate);

   void construct_imageframe_rotation_matrix(
      const threevector& what_0,const threevector& shat_0,
      const threevector& Rhat_0,rotation& R0_rotate);

   rotation* XELIAS_imageframe_to_model_rotation_matrix(
      const rotation& R0_rotate);
   void compute_imageframe_direction_vectors(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& rhat_0,threevector& vhat_0,threevector& Rhat_0,
      threevector& that_0,threevector& shat_0,threevector& what_0);

// =====================================================================
// Target elevation member functions
// =====================================================================

   void compute_target_range_az_el(
      const threevector& radar_posn,const threevector& satellite_posn,
      double& range,double& azimuth,double& elevation);
   void compute_target_range_az_el(
      const statevector& radar_statevector,
      const statevector& satellite_statevector,
      double& range,double& azimuth,double& elevation);
   void compute_target_range_az_el(
      const statevector& radar_statevector,
      const statevector& satellite_statevector,
      double& range,double& azimuth,double& elevation,double& elevation_dot);
   void compute_target_range_az_el(
      const statevector& radar_statevector,
      const statevector& satellite_statevector,
      double& range,double& azimuth,double& elevation,
      double& azimuth_dot_mag,double& elevation_dot);

// =====================================================================
// Basis transformation methods
// =====================================================================

   threevector imageframe_to_ECI(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      const threevector& V_imageframe);
   threevector ECI_to_imageframe(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      const threevector& V_ECI);
   rotation IGES_to_imageframe_rot(
      const threevector& xIGES_hat,const threevector& yIGES_hat,
      const threevector& zIGES_hat);

// =====================================================================
// Nominal wireframe setup methods
// =====================================================================

   void nominal_body_IGES_direction_vectors(
      const statevector& satellite_statevector,
      threevector& xhat_body_IGES_ECI,threevector& yhat_body_IGES_ECI,
      threevector& zhat_body_IGES_ECI);
   void nominal_body_IGES_direction_vectors(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& xhat_body_IGES0,threevector& yhat_body_IGES0,
      threevector& zhat_body_IGES0);
   void panelnormal_towards_sun_rotation(
      const threevector& panelnormal,const threevector& e_sun0,
      const rotation& R0_rotate,rotation& Rsolar0);
   double maximize_solar_flux(
      double sigma_start,double sigma_stop,double dsigma,
      const threevector& panelnormal,const threevector& e_sun0,
      const rotation& R0_rotate,rotation& Rsolar0);
   void nominal_panel_IGES_direction_vectors(
      const statevector& satellite_statevector,
      const statevector& radar_statevector,
      const threevector& sun_direction_ECI,
      motionfunc::Motion_type nominal_spacecraft_motion_type,
      threevector& xhat_panel_IGES0,threevector& yhat_panel_IGES0,
      threevector& zhat_panel_IGES0);

// =====================================================================
// Motion determination methods
// =====================================================================

   rotation IGES_rotation(
      const mypoint& xIGES_hat,const mypoint& yIGES_hat,
      const mypoint& zIGES_hat,double thetaIGES_x,double thetaIGES_y,
      double thetaIGES_z);
   rotation IGES_rotation(
      double thetaIGES_x,double thetaIGES_y,double thetaIGES_z,
      const rotation& RIGES);
   void IGES_angles(
      const rotation& Ktotal,const rotation& RIGES,
      double& thetaIGES_x,double& thetaIGES_y,double& thetaIGES_z);

   void compute_roll_pitch_matrices(
      double roll,double pitch,rotation& Rroll,rotation& Rpitch);
   void compute_roll_pitch_dot_matrices(
      double roll,double rolldot,double pitch,double pitchdot,
      rotation& Rroll_dot,rotation& Rpitch_dot);

   void roll_pitch_maneuver_body_rot(
      double roll,double pitch,rotation& Rmaneuver_body);

   void compute_telescope_direction(
      const statevector& satellite_statevector,
      double roll,double pitch,threevector& That_ECI);
   void compute_telescope_direction_and_derivative(
      const statevector& satellite_statevector,
      double roll,double rolldot,double pitch,double pitchdot,
      threevector& That_ECI,threevector& That_dot_ECI);
}

#endif // satellitefuncs.h



