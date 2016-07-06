// ==========================================================================
// kalman class member function definitions
// ==========================================================================
// Last modified on 8/25/15; 8/26/15
// ==========================================================================

#include <iostream>
#include "filter/kalman.h"
#include "numrec/nrfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void kalman::allocate_member_objects()
{
   Xtrue_ptr = new genvector(mdim);
   X_ptr = new genvector(mdim);
   P_ptr = new genmatrix(mdim, mdim);
   F_ptr = new genmatrix(mdim, mdim);

   U_ptr = new genvector(mdim);
   B_ptr = new genmatrix(mdim, mdim);
   Q_ptr = new genmatrix(mdim, mdim);

   Xpredict_ptr = new genvector(mdim);
   Ppredict_ptr = new genmatrix(mdim, mdim);

   Z_ptr = new genvector(mdim);
   H_ptr = new genmatrix(mdim, mdim);
   R_ptr = new genmatrix(mdim, mdim);

   Kprime_ptr = new genmatrix(mdim, mdim);
}

void kalman::initialize_member_objects()
{
   time_counter = 0;

   Xtrue_ptr->clear_values();
   X_ptr->clear_values();
   P_ptr->clear_values();
   F_ptr->identity();
   U_ptr->clear_values();
   B_ptr->clear_values();
   Q_ptr->clear_values();
   Xpredict_ptr->clear_values();
   Ppredict_ptr->clear_values();

   Z_ptr->clear_values();
   H_ptr->identity();
   R_ptr->clear_values();
   Kprime_ptr->clear_values();
}

kalman::kalman(int mdim)
{
   this->mdim = mdim;

   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

kalman::kalman(const kalman& K)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(K);
}

kalman::~kalman()
{
   delete Xtrue_ptr;
   delete X_ptr;
   delete P_ptr;
   delete F_ptr;

   delete U_ptr;
   delete B_ptr;
   delete Q_ptr;

   delete Z_ptr;
   delete H_ptr;
   delete R_ptr;
   delete Kprime_ptr;
}

// ---------------------------------------------------------------------
void kalman::docopy(const kalman& K)
{
}
   
// Overload = operator:

kalman& kalman::operator= (const kalman& K)
{
   if (this==&K) return *this;
   docopy(K);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const kalman& K)
{
   outstream.precision(6);
//   outstream.setf(ios::scientific);

//   outstream << "mdim = " << K.mdim << endl;
//   outstream << "dt = " << K.dt << endl;
   outstream << "time counter = " << K.time_counter 
             << " t = " << K.time_counter * K.dt << endl;
//   outstream << "Transition matrix F = " << *K.F_ptr << endl;
   
   double Xtrue_sum = 0;
   for(int d = 0; d < K.mdim; d++)
   {
      Xtrue_sum += fabs( K.Xtrue_ptr->get(d) );
   }
   
   if(!nearly_equal(Xtrue_sum,0))
   {
      outstream << "Xtrue = " << *K.Xtrue_ptr << endl;
   }
   outstream << "Xpredict = " << *K.Xpredict_ptr << endl;
   outstream << "Ppredict = " << *K.Ppredict_ptr << endl;
   outstream << "Xmeasured = " << *K.Z_ptr << endl;


   outstream << "Current Kalman gain = " << *K.Kprime_ptr << endl;
   outstream << "Current state vector X = " << *K.X_ptr << endl;
   outstream << "Current state covariance matrix P = " << *K.P_ptr << endl;


   if(!nearly_equal(Xtrue_sum,0))
   {
      for(int d = 0; d < K.mdim; d++)
      {
         double frac_error = (K.X_ptr->get(d) - K.Xtrue_ptr->get(d)) / 
            K.Xtrue_ptr->get(d);
         outstream << "d = " << d 
                   << " (Filtered - true) statevector error = " 
                   << 100 * frac_error << " % " << endl;
      }
      outstream << endl;
   }

   outstream << endl;
   return outstream;
}

// ==========================================================================
// ==========================================================================

void kalman::initialize_state_vector(double x)
{
   X_ptr->put(0,x);
}

void kalman::initialize_state_vector(double x, double v)
{
   X_ptr->put(0,x);
   X_ptr->put(1,v);
}

void kalman::initialize_state_vector(double x, double v_x, double y, double v_y)
{
   X_ptr->put(0,x);
   X_ptr->put(1,v_x);
   X_ptr->put(2,y);
   X_ptr->put(3,v_y);
}

void kalman::initialize_true_vector(double x, double v_x, double y, double v_y)
{
   Xtrue_ptr->put(0,x);
   Xtrue_ptr->put(1,v_x);
   Xtrue_ptr->put(2,y);
   Xtrue_ptr->put(3,v_y);
}

// ---------------------------------------------------------------------
void kalman::initialize_covariance_matrix(double Sigma_xx)
{
   P_ptr->put(0,0,Sigma_xx);
}

void kalman::initialize_covariance_matrix(
   double Sigma_xx, double Sigma_xv, double Sigma_vv)
{
   P_ptr->put(0,0,Sigma_xx);
   P_ptr->put(0,1,Sigma_xv);
   P_ptr->put(1,0,Sigma_xv);
   P_ptr->put(1,1,Sigma_vv);
}

void kalman::initialize_covariance_matrix(
   double Sigma_xx, double Sigma_vxvx, double Sigma_yy, double Sigma_vyvy)
{
   P_ptr->put(0,0,Sigma_xx);
   P_ptr->put(1,1,Sigma_vxvx);
   P_ptr->put(2,2,Sigma_yy);
   P_ptr->put(3,3,Sigma_vyvy);
}

// ---------------------------------------------------------------------
void kalman::initialize_transition_matrix(int dim)
{
   if(dim == 1)	// Voltage example problem
   {
      F_ptr->put(0,0,1);
   }
   else if(dim == 2)
   {
      F_ptr->put(0,0,1);
      F_ptr->put(0,1,dt);
   
      F_ptr->put(1,0,0);
      F_ptr->put(1,1,1);
   }
   else if(dim ==4)	// Cannon ball example problem
   {
      F_ptr->put(0,1,dt);
      F_ptr->put(2,3,dt);
   }
}

// ---------------------------------------------------------------------
void kalman::initialize_control_matrix(int dim)
{
   if(dim == 2)
   {
      B_ptr->put(0,0,0.5 * sqr(dt));
      B_ptr->put(0,1,0);
      B_ptr->put(1,0,0);
      B_ptr->put(1,1,dt);
   }
   else if (dim == 4)	// Cannon ball example problem
   {
      B_ptr->put(2,2,1);
      B_ptr->put(3,3,1);
   }
}

// ---------------------------------------------------------------------
void kalman::initialize_control_vector(int dim)
{
   if(dim == 2)
   {
//   double a = 0;	// acceleration
      double a = 0.5;	// acceleration
   
      U_ptr->put(0,a);
      U_ptr->put(1,a);
   }
   else if (dim == 4)
   {
      double g = 9.8;	// m/sec**2
      U_ptr->put(2,-0.5 * g * sqr(dt));
      U_ptr->put(3,-g * dt);
   }
}

// ---------------------------------------------------------------------
void kalman::set_next_measurements(double measured_x)
{
   Z_ptr->put(0,measured_x);
}

void kalman::set_next_measurements(double measured_x, double measured_v)
{
   Z_ptr->put(0,measured_x);
   Z_ptr->put(1,measured_v);
}

// ---------------------------------------------------------------------
void kalman::set_process_noise_matrix(double noise_xx)
{
   Q_ptr->put(0,0,noise_xx);
}

void kalman::set_process_noise_matrix(
   double noise_xx, double noise_xv, double noise_vv)
{
   Q_ptr->put(0,0,noise_xx);
   Q_ptr->put(0,1,noise_xv);
   Q_ptr->put(1,0,noise_xv);
   Q_ptr->put(1,1,noise_vv);
}

// ---------------------------------------------------------------------
void kalman::set_sensor_noise_matrix(double noise_xx)
{
   R_ptr->put(0,0,noise_xx);
}

void kalman::set_sensor_noise_matrix(
   double noise_xx, double noise_xv, double noise_vv)
{
   R_ptr->put(0,0,noise_xx);
   R_ptr->put(0,1,noise_xv);
   R_ptr->put(1,0,noise_xv);
   R_ptr->put(1,1,noise_vv);
}

void kalman::set_sensor_noise_matrix(
   double noise_xx, double noise_vxvx, double noise_yy, double noise_vyvy)
{
   R_ptr->put(0,0,noise_xx);
   R_ptr->put(1,1,noise_vxvx);
   R_ptr->put(2,2,noise_yy);
   R_ptr->put(3,3,noise_vyvy);
}

// ---------------------------------------------------------------------
// Member function predict_next_state() computes the state vector at
// the next time step based solely upon propagating forward the state
// vector at the previous time step in the presence of state vector
// uncertainty:

void kalman::predict_next_state()
{
   *Xpredict_ptr = (*F_ptr) * (*X_ptr) + (*B_ptr) * (*U_ptr);
   *Ppredict_ptr = (*F_ptr) * (*P_ptr) * (F_ptr->transpose()) + (*Q_ptr);
}

// ---------------------------------------------------------------------
// Member function true_next_state() propagates the true state vector
// to the next time step assuming its value is known at the current
// time step in the limit of zero noise.

void kalman::true_next_state()
{
   *Xtrue_ptr = (*F_ptr) * (*Xtrue_ptr) + (*B_ptr) * (*U_ptr);
}

void kalman::simulate_next_measurements()
{
   double mu = 0;
   for(int d = 0; d < mdim; d++)
   {
      double simulated_noise = mu + R_ptr->get(d,d) * nrfunc::gasdev();
      double simulated_measurement = Xtrue_ptr->get(d) + simulated_noise;
      Z_ptr->put(d,simulated_measurement);
   }
}

// ---------------------------------------------------------------------
// Member function correct_next_state() computes the correction to the
// predicted state vector based upon noisy sensor measurements.  The
// sensor correction is multiplied by the Kalman gain for the current
// time step and added to the predicted state vector to yield the
// optimal estimate for the state vector at the next time step.

void kalman::correct_next_state()
{
   genmatrix denom(mdim, mdim), denom_inverse(mdim, mdim);

// Compute Kalman gain:

   denom = (*H_ptr) * (*Ppredict_ptr) * H_ptr->transpose() + (*R_ptr);
   denom.inverse(denom_inverse);
//   cout << "denom_inverse = " << denom_inverse << endl;

   *Kprime_ptr = (*Ppredict_ptr) * H_ptr->transpose() * denom_inverse;

// Correct state at next time step:

   (*X_ptr) = (*Xpredict_ptr) + (*Kprime_ptr) * 
      (*Z_ptr - (*H_ptr) * (*Xpredict_ptr));
   (*P_ptr) = (*Ppredict_ptr) - (*Kprime_ptr) * (*H_ptr) * (*Ppredict_ptr);
}

void kalman::increment_time_counter()
{
   time_counter++;
}
