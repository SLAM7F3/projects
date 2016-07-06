// ==========================================================================
// Program CANNON_KALMAN implements 1D voltage example of kalman filtering
// from http://bilgin.esme.org/BitsBytes/KalmanFilterforDummies.aspx
// ==========================================================================
// Last updated on 8/26/15
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

  int dim = 4;
  kalman K(dim);

  K.set_dt(1);	// sec

  double x = 0;
  double v_x = 100 * cos(PI/4);
  double y = 500;
  double v_y = 100 * sin(PI/4);
  K.initialize_state_vector(x, v_x, y, v_y);

  x = 0;
  v_x = 100 * cos(PI/4);
  y = 0;
  v_y = 100 * sin(PI/4);
  K.initialize_true_vector(x, v_x, y, v_y);

  double Sigma_xx = 1;
  double Sigma_vxvx = 1;
  double Sigma_yy = 1;
  double Sigma_vyvy = 1;
  K.initialize_covariance_matrix(Sigma_xx, Sigma_vxvx, Sigma_yy, Sigma_vyvy);

  K.initialize_transition_matrix(dim);
  K.initialize_control_matrix(dim);
  K.initialize_control_vector(dim);

  double sigma_noise = 0.2;
  K.set_sensor_noise_matrix(sigma_noise, sigma_noise, sigma_noise, sigma_noise);

//  int n_steps = 3;
//  int n_steps = 10;
  int n_steps = 13;
  for(int n = 0; n <= n_steps; n++)
  {
     cout << "---------------------------------------------" << endl;

     K.predict_next_state();

     K.true_next_state();
     K.simulate_next_measurements();
//     K.set_next_measurements(voltage_measurements[n]);

     K.correct_next_state();
     cout << K << endl;     

     K.increment_time_counter();
  } // loop over index n labeling time steps
}
