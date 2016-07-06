// ==========================================================================
// Program VOLTAGE_KALMAN implements 1D voltage example of kalman filtering
// from http://bilgin.esme.org/BitsBytes/KalmanFilterforDummies.aspx
// ==========================================================================
// Last updated on 8/25/15
// ==========================================================================

#include <iostream>
#include <math.h>
#include "filter/kalman.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
  std::set_new_handler(sysfunc::out_of_memory);
  cout.precision(10);

  vector<double> voltage_measurements;
  voltage_measurements.push_back(0.39);
  voltage_measurements.push_back(0.50);
  voltage_measurements.push_back(0.48);
  voltage_measurements.push_back(0.29);
  voltage_measurements.push_back(0.25);
  voltage_measurements.push_back(0.32);
  voltage_measurements.push_back(0.34);
  voltage_measurements.push_back(0.48);
  voltage_measurements.push_back(0.41);
  voltage_measurements.push_back(0.45);

  int dim = 1;
  kalman K(dim);

  K.set_dt(0.001);	// sec

  double x = 0;
  K.initialize_state_vector(x);
  double Sigma_xx = 1;
  K.initialize_covariance_matrix(Sigma_xx);

  K.initialize_transition_matrix(dim);
//  K.initialize_control_matrix();
//  K.initialize_control_vector();

//  double noise_xx = 0.1;
//  K.set_environment_noise_matrix(noise_xx);

  double sigma_noise = 0.1;
  K.set_sensor_noise_matrix(sigma_noise);

//  int n_steps = 3;
  int n_steps = 10;
  for(int n = 0; n <= n_steps; n++)
  {
     cout << "---------------------------------------------" << endl;

     K.predict_next_state();
     K.set_next_measurements(voltage_measurements[n]);
     K.correct_next_state();
     cout << K << endl;     

     K.increment_time_counter();
  } // loop over index n labeling time steps
}
