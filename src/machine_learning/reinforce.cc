// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/4/16; 10/5/16; 10/11/16; 10/12/16
// ==========================================================================

#include <string>
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "numrec/nrfuncs.h"
#include "machine_learning/reinforce.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void reinforce::initialize_member_objects()
{
   Din = 4 * 4;		// Input dimensionality
   Dout = 4 * 4;	// Output dimensionality
   H = 200;		// Number of hidden layer neurons
   Tmax = 64;	 	// Maximum number of time steps per episode
//   Tmax = 1000;	 	// Maximum number of time steps per episode

   batch_size = 10;	// Perform parameter update after this many episodes
   learning_rate = 0.001;	
   gamma = 0.99;	// Discount factor for reward
   decay_rate = 0.99;	// Decay factor for RMSProp leaky sum of grad**2
   
   running_reward = 0;
   reward_sum = 0;
   episode_number = 0;
}		       

void reinforce::allocate_member_objects()
{
   W1_ptr = new genmatrix(H, Din);
   dW1_ptr = new genmatrix(H, Din);
   dW1_buffer_ptr = new genmatrix(H, Din);
   sqr_dW1_buffer_ptr = new genmatrix(H, Din);

   W2_ptr = new genmatrix(Dout, H);
   dW2_ptr = new genmatrix(Dout, H);
   dW2_buffer_ptr = new genmatrix(Dout, H);
   sqr_dW2_buffer_ptr = new genmatrix(Dout, H);

   rmsprop1_ptr = new genmatrix(H, Din);
   sqrt_rmsprop1_ptr = new genmatrix(H, Din);
   rmsprop2_ptr = new genmatrix(Dout, H);
   sqrt_rmsprop2_ptr = new genmatrix(Dout, H);

   h_ptr = new genvector(H);
   logp_ptr = new genvector(Dout);
   dlogp_ptr = new genvector(Dout);
   p_ptr = new genvector(Dout);

   episode_x_ptr = new genmatrix(Tmax, Din);
   episode_h_ptr = new genmatrix(Tmax, H);
   episode_dlogp_ptr = new genmatrix(Tmax, Dout);
   episode_reward_ptr = new genvector(Tmax);
   discounted_episode_reward_ptr = new genvector(Tmax);
}		       

void reinforce::print_matrices()
{
   cout << "*W1_ptr = " << *W1_ptr << endl;
   cout << "*W2_ptr = " << *W2_ptr << endl;

   cout << "*dW1_ptr = " << *dW1_ptr << endl;
   cout << "*dW2_ptr = " << *dW2_ptr << endl;

   cout << "*rmsprop1_ptr = " << *rmsprop1_ptr << endl;
   cout << "*rmsprop2_ptr = " << *rmsprop2_ptr << endl;
}		       

// ---------------------------------------------------------------------
reinforce::reinforce()
{
   initialize_member_objects();
   allocate_member_objects();

// Clear cumulative matrices:

   dW1_buffer_ptr->clear_matrix_values();
   dW2_buffer_ptr->clear_matrix_values();
   rmsprop1_ptr->clear_matrix_values();
   rmsprop2_ptr->clear_matrix_values();

   xavier_init_weight_matrices();
}

// Copy constructor:

reinforce::reinforce(const reinforce& R)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
reinforce::~reinforce()
{
   delete W1_ptr;
   delete dW1_ptr;
   delete dW1_buffer_ptr;
   delete sqr_dW1_buffer_ptr;

   delete W2_ptr;
   delete dW2_ptr;
   delete dW2_buffer_ptr;
   delete sqr_dW2_buffer_ptr;

   delete rmsprop1_ptr;
   delete sqrt_rmsprop1_ptr;
   delete rmsprop2_ptr;
   delete sqrt_rmsprop2_ptr;

   delete h_ptr;
   delete logp_ptr;
   delete dlogp_ptr;
   delete p_ptr;

   delete episode_x_ptr;
   delete episode_h_ptr;
   delete episode_dlogp_ptr;
   delete episode_reward_ptr;
   delete discounted_episode_reward_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const reinforce& R)
{
   outstream << endl;
   outstream << "Din = " << R.Din << " Dout = " << R.Dout << endl;
   outstream << "H = " << R.H << endl;
   outstream << "batch_size = " << R.batch_size << " learning_rate = "
             << R.learning_rate << endl;
   outstream << "gamma = " << R.gamma << endl;
   return outstream;
}

// ==========================================================================

void reinforce::xavier_init_weight_matrices()
{
   for(unsigned int py = 0; py < W1_ptr->get_ndim(); py++)
   {
      for(unsigned int px = 0; px < W1_ptr->get_mdim(); px++)
      {
         W1_ptr->put(px,py,nrfunc::gasdev() / sqrt(Din) );
      } // loop over px
   } // loop over py 

   for(unsigned int py = 0; py < W2_ptr->get_ndim(); py++)
   {
      for(unsigned int px = 0; px < W2_ptr->get_mdim(); px++)
      {
         W2_ptr->put(px,py,nrfunc::gasdev() / sqrt(H) );
      } // loop over px
   } // loop over py 

//   cout << "W1 = " << *W1_ptr << endl;
//   cout << "W2 = " << *W2_ptr << endl;
}

// ---------------------------------------------------------------------
void reinforce::discount_rewards()
{
   double running_add = 0;

   for(int t = T-1; t >= 0; t--)
   {
      running_add = gamma * running_add + episode_reward_ptr->get(t);
      discounted_episode_reward_ptr->put(t, running_add);
   }
}

// ---------------------------------------------------------------------
void reinforce::policy_forward(genvector* x_ptr)
{
   *h_ptr = (*W1_ptr) * (*x_ptr);
   machinelearning_func::ReLU(*h_ptr);
      
   *logp_ptr = (*W2_ptr) * (*h_ptr);
   machinelearning_func::sigmoid(*logp_ptr, *p_ptr);
}

// ---------------------------------------------------------------------
void reinforce::policy_backward()
{
   *dW2_ptr =  episode_dlogp_ptr->transpose() * (*episode_h_ptr);

   genmatrix *dh_ptr = new genmatrix(H,T);
   *dh_ptr = W2_ptr->transpose() * episode_dlogp_ptr->transpose();
   machinelearning_func::ReLU(*dh_ptr);

   *dW1_ptr = (*dh_ptr) * (*episode_x_ptr);

   delete dh_ptr;
}

// ---------------------------------------------------------------------
void reinforce::initialize_episode()
{
   episode_x_ptr->clear_values();
   episode_h_ptr->clear_values();
   episode_dlogp_ptr->clear_values();

   curr_timestep = 0;
}

// ---------------------------------------------------------------------
void reinforce::compute_current_action(
   genvector* input_state_ptr, genvector* output_action_ptr)
{
   policy_forward(input_state_ptr);
   
// Sample an action from returned probability:
   for(int d = 0; d < Dout; d++)
   {
      double action_prob = p_ptr->get(d);
      double y;	 // fake label
      if(nrfunc::ran1() < action_prob)
      {
         output_action_ptr->put(d, 1);
         y = 1;
      }
      else
      {
         output_action_ptr->put(d, 0);
         y = 0;
      }

// Logistic regresssion loss function:

// L_i = Sum_j [ y_ij log(sigma(f_j)) + (1 - y_ij) * log(1-sigma(f_j)) ]

//  dL_i/df_j = y_ij - sigma(f_j)

      dlogp_ptr->put(d, y - action_prob);
   } // loop over index d 

// Record various intermediates needed later for backpropagation:
          
   episode_x_ptr->put_row(curr_timestep, *input_state_ptr);
   episode_h_ptr->put_row(curr_timestep, *h_ptr);
   episode_dlogp_ptr->put_row(curr_timestep, *dlogp_ptr);
   
// Step the environment and then retrieve new reward measurements
}

// ---------------------------------------------------------------------
void reinforce::record_reward_for_action(double curr_reward)
{
   reward_sum += curr_reward;
   episode_reward_ptr->put(curr_timestep, curr_reward);
   curr_timestep++;
}

// ---------------------------------------------------------------------
void reinforce::update_weights(bool episode_finished_flag)
{
   if(!episode_finished_flag) return;

// Compute the discounted reward backwards through time:

   discount_rewards();

// Standardize discounted rewards to be unit normal to help control
// the gradient estimator variance:

   double mean = 0;
   for(int t = 0; t < T; t++)
   {
      mean += discounted_episode_reward_ptr->get(t);
   }
   mean /= T;
      
   double sigmasqr = 0;
   for(int t = 0; t < T; t++)
   {
      double curr_reward = discounted_episode_reward_ptr->get(t) - mean;
      discounted_episode_reward_ptr->put(t, curr_reward);
      sigmasqr += sqr(curr_reward);
   }
   sigmasqr /= T;
   double sigma = sqrt(sigmasqr);

   for(int t = 0; t < T; t++)
   {
      double curr_reward = discounted_episode_reward_ptr->get(t) / sigma;
      discounted_episode_reward_ptr->put(t, curr_reward);
   }
      
// Modulate the gradient with advantage (Policy Gradient magic happens
// right here):

   genvector curr_row(Dout);
   for(int t = 0; t < T; t++)
   {
      episode_dlogp_ptr->get_row(t, curr_row);
      curr_row *= discounted_episode_reward_ptr->get(t);
      episode_dlogp_ptr->put_row(t, curr_row);
   }

   policy_backward();

// Accumulate dW1 and dW2 gradients over batch:

   *dW1_buffer_ptr += *dW1_ptr;
   *dW2_buffer_ptr += *dW2_ptr;
            
// Perform RMSprop parameter update every batch_size episodes:

   episode_number++;
   if(episode_number % batch_size == 0)
   {
      sqr_dW1_buffer_ptr->elementwise_power(*dW1_buffer_ptr, 2);
      sqr_dW2_buffer_ptr->elementwise_power(*dW2_buffer_ptr, 2);

//  MeanSquare(w,t) = 0.9 MeanSquare(w,t-1) + 0.1 (dE(t)/dw)**2

      *rmsprop1_ptr = decay_rate * (*rmsprop1_ptr) +
         (1 - decay_rate) * (*sqr_dW1_buffer_ptr);
      *rmsprop2_ptr = decay_rate * (*rmsprop2_ptr) +
         (1 - decay_rate) * (*sqr_dW2_buffer_ptr);

// Dividing gradient by sqrt(MeanSquare(w,t)) significantly improves learning:

      sqrt_rmsprop1_ptr->elementwise_power(*rmsprop1_ptr, 0.5);
      sqrt_rmsprop2_ptr->elementwise_power(*rmsprop2_ptr, 0.5);

      const double TINY = 1E-5;
      for(unsigned int py = 0; py < W1_ptr->get_mdim(); py++)
      {
         for(unsigned int px = 0; px < W1_ptr->get_ndim(); px++)
         {
            double term1 = W1_ptr->get(px, py);
            double denom = sqrt_rmsprop1_ptr->get(px, py) + TINY;
            double term2 = learning_rate * rmsprop1_ptr->get(px,py)/denom;
            W1_ptr->put(px, py, term1 + term2);
         }
      }

      for(unsigned int py = 0; py < W2_ptr->get_mdim(); py++)
      {
         for(unsigned int px = 0; px < W2_ptr->get_ndim(); px++)
         {
            double term1 = W2_ptr->get(px, py);
            double denom = sqrt_rmsprop2_ptr->get(px, py) + TINY;
            double term2 = learning_rate * rmsprop2_ptr->get(px,py)/denom;
            W2_ptr->put(px, py, term1 + term2);
         }
      }

      dW1_buffer_ptr->clear_values();
      dW2_buffer_ptr->clear_values();
   } // episode % batch_size == 0 conditional

   if(running_reward == 0)
   {
      running_reward = reward_sum;
   }
   else
   {
      running_reward = 0.99 * running_reward + 0.01 * reward_sum;
   }
      
   cout << "Episode reward total was " << reward_sum << endl;
   cout << "Running reward mean = " << running_reward << endl;
   reward_sum = 0;
}
