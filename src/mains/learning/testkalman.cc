// ==========================================================================
// Program TESTKALMAN
// ==========================================================================
// Last updated on 8/25/15; 8/30/15
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

  int dim = 2;
  kalman K(dim);

  K.set_dt(0.1);	// sec

  double x = 0;
  double v = 1;
  K.initialize_state_vector(x, v);
  double Sigma_xx = 0.2;
  double Sigma_xv = 0.05;
  double Sigma_vv = 0.1;
  K.initialize_covariance_matrix(Sigma_xx, Sigma_xv, Sigma_vv);

  K.initialize_transition_matrix(dim);
  K.initialize_control_matrix(dim);
  K.initialize_control_vector(dim);

  double noise_xx = 0.1;
  double noise_xv = 0.02;
  double noise_vv = 0.05;
  K.set_process_noise_matrix(noise_xx, noise_xv, noise_vv);

  noise_xx = 0.5;
  noise_xv = 0.25;
  noise_vv = 0.5;
  K.set_sensor_noise_matrix(noise_xx, noise_xv, noise_vv);

  int n_steps = 10;
  for(int n = 0; n <= n_steps; n++)
  {
     cout << "---------------------------------------------" << endl;
     cout << K << endl;     
     K.predict_next_state();
     genvector* Xpredict_ptr = K.get_Xpredict_ptr();
     cout << "Xpredict = " << Xpredict_ptr->get(0)
          << " Vpredict = " << Xpredict_ptr->get(1)
          << endl;

     K.set_next_measurements(Xpredict_ptr->get(0),Xpredict_ptr->get(1));
     K.correct_next_state();
  } // loop over index n labeling time steps
  



}
