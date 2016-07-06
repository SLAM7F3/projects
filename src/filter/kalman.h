// ==========================================================================
// Header file for kalman class
// ==========================================================================
// Last updated on 8/25/15; 8/26/15
// ==========================================================================

#ifndef KALMAN_H
#define KALMAN_H

#include "math/genmatrix.h"
#include "math/genvector.h"

class kalman
{
  public:

// Initialization, constructor and destructor functions:

   kalman(int mdim);
   kalman(const kalman& K);
   ~kalman();
   kalman& operator= (const kalman& K);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const kalman& K);
 
// Set and get methods:

   int get_mdim() const;
   void set_dt(double dt);

   void initialize_state_vector(double x);
   void initialize_state_vector(double x, double v);
   void initialize_state_vector(double x, double v_x, double y, double v_y);

   void initialize_true_vector(double x);
   void initialize_true_vector(double x, double v);
   void initialize_true_vector(double x, double v_x, double y, double v_y);

   void initialize_covariance_matrix(double Sigma_xx);
   void initialize_covariance_matrix(
      double Sigma_xx, double Sigma_xv, double Sigma_vv);
   void initialize_covariance_matrix(
      double Sigma_xx, double Sigma_vxvx, double Sigma_yy, double Sigma_vyvy);

   void initialize_transition_matrix(int dim);
   void initialize_control_matrix(int dim);
   void initialize_control_vector(int dim);

   void set_next_measurements(double measured_x);
   void set_next_measurements(double measured_x, double measured_v);
   void set_process_noise_matrix(double noise_xx);
   void set_process_noise_matrix(
      double noise_xx, double noise_xv, double noise_vv);
   void set_sensor_noise_matrix(double noise_xx);
   void set_sensor_noise_matrix(
      double noise_xx, double noise_xv, double noise_vv);
   void set_sensor_noise_matrix(
      double noise_xx, double noise_vxvx, double noise_yy, double noise_vyvy);

   void true_next_state();
   void simulate_next_measurements();

   void predict_next_state();
   genvector* get_Xpredict_ptr();
   const genvector* get_Xpredict_ptr() const;
   void correct_next_state();
   void increment_time_counter();

  private: 

   int mdim;
   int time_counter;
   double dt;		// Time step 
   genvector* Xtrue_ptr;// True system state vector (if actually known)
   genvector* X_ptr;	// System state vector
   genmatrix* P_ptr;	// System covariance matrix estimates accuracy
			//  of *X_ptr
   genmatrix* F_ptr;	// State transition model matrix
   
   genvector* U_ptr;	// External control vector
   genmatrix* B_ptr;	// External control matrix
   genmatrix* Q_ptr;	// Covariance matrix for process noise 
			//  (includes physical effects not explicitly modeled
 			//   within *F_ptr)

   genvector* Xpredict_ptr;	// Predicted state vector at next timestep
   genmatrix* Ppredict_ptr;	// Predicted covar matrix at next timestep

   genvector* Z_ptr;	// Measurement vector
   genmatrix* H_ptr;	// Sensor model matrix
   genmatrix* R_ptr;	// Covariance matrix for sensor noise

   genmatrix* Kprime_ptr;  // Kalman gain matrix

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const kalman& K);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int kalman::get_mdim() const
{
   return mdim;
}


inline void kalman::set_dt(double dt)
{
   this->dt = dt;
}

inline genvector* kalman::get_Xpredict_ptr()
{
   return Xpredict_ptr;
}

inline const genvector* kalman::get_Xpredict_ptr() const
{
   return Xpredict_ptr;
}


# endif // kalman.h











