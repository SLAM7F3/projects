// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/5/16; 10/11/16; 10/12/16; 10/18/16
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

void reinforce::initialize_member_objects(const vector<int>& n_nodes_per_layer)
{
   batch_size = 5;	// Perform parameter update after this many episodes
   learning_rate = 0.001;	
   lambda = 0.001;	// L2 regularization coefficient
   gamma = 0.99;	// Discount factor for reward
   decay_rate = 0.99;	// Decay factor for RMSProp leaky sum of grad**2
   
   running_reward = 0;
   reward_sum = 0;
   episode_number = 0;

   n_layers = n_nodes_per_layer.size();

   for(int l = 0; l < n_layers; l++)
   {
      layer_dims.push_back(n_nodes_per_layer[l]);
      genmatrix *curr_z = new genmatrix(layer_dims.back(), Tmax);
      genmatrix *curr_a = new genmatrix(layer_dims.back(), Tmax);
      genmatrix *curr_delta_prime = new genmatrix(layer_dims.back(), Tmax);
      z.push_back(curr_z);
      a.push_back(curr_a);
      delta_prime.push_back(curr_delta_prime);
   }
   n_actions = layer_dims.back();

   p_action = new genvector(n_actions);
   pcum_action = new genvector(n_actions);

// Weights link layer l with layer l+1:
    
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      weights.push_back(curr_weights);
      genmatrix *curr_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      nabla_weights.push_back(curr_nabla_weights);
      genmatrix *curr_delta_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      delta_nabla_weights.push_back(curr_delta_nabla_weights);
      genmatrix *curr_rmsprop_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_rmsprop_weights->clear_values();
      rmsprop_weights_cache.push_back(curr_rmsprop_weights);

// Xavier initialize weights connecting network layers l and l+1 to be
// gaussian random vars distributed according to N(0,1/sqrt(n_in)):

      for(int i = 0; i < layer_dims[l+1]; i++)
      {
         for(int j = 0; j < layer_dims[l]; j++)
         {
            curr_weights->put(i, j, nrfunc::gasdev() / sqrt(layer_dims[l]) );
         } // loop over index j labeling node in next layer
      } // loop over index i labeling node in current layer

   } // loop over index l labeling neural net layers
}

void reinforce::allocate_member_objects()
{
   y = new genvector(Tmax);
   reward = new genvector(Tmax);
   discounted_reward = new genvector(Tmax);
}		       

// ---------------------------------------------------------------------
reinforce::reinforce(const vector<int>& n_nodes_per_layer, int Tmax)
{
   this->Tmax = Tmax;
   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();
}

// ---------------------------------------------------------------------
reinforce::~reinforce()
{
   for(unsigned int l = 0; l < z.size(); l++)
   {
      delete z[l];
      delete a[l];
      delete delta_prime[l];
   }

   delete p_action;
   delete pcum_action;
   delete y;
   delete reward;
   delete discounted_reward;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const reinforce& R)
{
   outstream << endl;
   outstream << "batch_size = " << R.batch_size << " learning_rate = "
             << R.learning_rate << endl;
   outstream << "gamma = " << R.gamma << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Member function policy_forward returns the output of the network
// given an input set of values.

void reinforce::policy_forward(int t, genvector& x_input)
{
   a[0]->put_column(t, x_input);
//    cout << "inside policy_forward, x_input = " << *x_input << endl;

   for(int l = 0; l < n_layers-1; l++)
   {
//      cout << "l = " << l << endl;
      genmatrix* curr_weights = weights[l];
      genvector z_lplus1_t(*curr_weights * a[l]->get_column(t));
      genvector a_lplus1_t(z_lplus1_t.get_mdim());

      z[l+1]->put_column(t, z_lplus1_t);
//      cout << "z[l+1, t] = " << z[l+1]->get_column(t) << endl;

// Perform soft-max classification on final-layer's weighted inputs:

      if(l == n_layers - 2)
      {
         machinelearning_func::softmax(z_lplus1_t, a_lplus1_t);
      }
      else // perform ReLU on hidden layer's weight inputs
      {
         machinelearning_func::ReLU(z_lplus1_t, a_lplus1_t);
      }
//       cout << "a[l+1] = " << a[l+1]->get_column(t) << endl;
      a[l+1]->put_column(t, a_lplus1_t);
   }
}

// ---------------------------------------------------------------------
void reinforce::get_softmax_action_probs(int t) const
{
   *p_action = a[n_layers-1]->get_column(t);
}

// ---------------------------------------------------------------------
void reinforce::discount_rewards()
{
   double running_add = 0;

   for(int t = T-1; t >= 0; t--)
   {
      running_add = gamma * running_add + reward->get(t);
      discounted_reward->put(t, running_add);
   }
}

// ---------------------------------------------------------------------
void reinforce::policy_backward()
{

// Initialize "episode" weight gradients to zero:

   for(int l = 0; l < n_layers - 1; l++)
   {
      delta_nabla_weights[l]->clear_values();
   }

   for(int t = 0; t < T; t++)
   {
      
// Eqn BP1:

      int curr_layer = n_layers - 1;
      for(int j = 0; j < layer_dims[curr_layer]; j++)
      {
         double curr_activation = a[curr_layer]->get(j, t);
         if(j == y->get(t)) curr_activation -= 1.0;

// Modulate the gradient with advantage (Policy Gradient magic happens
// right here):

         delta_prime[curr_layer]->put(
            j, t, discounted_reward->get(t) * curr_activation);
      }
   } // loop over index t
   
   for(int curr_layer = n_layers-1; curr_layer >= 1; curr_layer--)
   {
      int prev_layer = curr_layer - 1;

// Eqn BP2A:

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

      *delta_prime[prev_layer] = weights[prev_layer]->transpose() * 
         (*delta_prime[curr_layer]);

// Eqn BP2B:
      for(int j = 0; j < layer_dims[prev_layer]; j++)
      {
         for(int t = 0; t < T; t++)
         {
            if(z[prev_layer]->get(j, t) < 0)
            {
               delta_prime[prev_layer]->put(j, t, 0);
            }
         } // loop over t index
      } // loop over j index

// Accumulate weight gradients over episode:

      for(int t = 0; t < T; t++)
      {
// Eqn BP4:
         *delta_nabla_weights[prev_layer] += 
            delta_prime[curr_layer]->get_column(t).outerproduct(
               a[prev_layer]->get_column(t)) +
            2 * lambda * (*weights[prev_layer]);
      }

   } // loop over curr_layer index
} 

// ---------------------------------------------------------------------
void reinforce::initialize_episode()
{
   for(unsigned int i = 0; i < z.size(); i++)
   {
      z[i]->clear_values();
      a[i]->clear_values();
   }

   for(unsigned int i = 0; i < delta_prime.size(); i++)
   {
      delta_prime[i]->clear_values();
   }

   for(int t = 0; t < Tmax; t++)
   {
      y->put(t, 0);
      reward->put(t, 0);
      discounted_reward->put(t, 0);
   }
   
   curr_timestep = 0;
}

// ---------------------------------------------------------------------
// Member function compute_current_action() imports an input state
// vector.  It returns the index for the action which is
// probabilistically selected based upon the current softmax action
// distribution.

int reinforce::compute_current_action(genvector* input_state_ptr)
{
   policy_forward(curr_timestep, *input_state_ptr);
   get_softmax_action_probs(curr_timestep);  // n_actions x 1

// Renormalize action probabilities:

   double denom = 0;
   for(int a = 0; a < n_actions; a++)
   {
      denom += p_action->get(a);
   }

   double pcum = 0;
   for(int a = 0; a < n_actions; a++)
   {
      p_action->put(a, p_action->get(a) / denom);
      pcum += p_action->get(a);
      pcum_action->put(a, pcum);
//      cout << "a = " << a 
//           << " p_action = " << p_action->get(a) 
//           << " pcum_action = " << pcum_action->get(a) << endl;
   }

// Generate uniformly-distributed random variable.  Use it to
// inversely sample cumulative probability distribution to set action
// a_star:

   int a_star = 0;
   double q = nrfunc::ran1();
   for(int a = 0; a < n_actions - 1; a++)
   {
      if(q >= pcum_action->get(a) && q < pcum_action->get(a+1))
      {
         a_star = a + 1;
         break;
      }
   }

   y->put(curr_timestep, a_star);

   return a_star;
}

// ---------------------------------------------------------------------
void reinforce::record_reward_for_action(double curr_reward)
{
   reward_sum += curr_reward;
   reward->put(curr_timestep, curr_reward);
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
      mean += discounted_reward->get(t);
   }
   mean /= T;
      
   double sigmasqr = 0;
   for(int t = 0; t < T; t++)
   {
      double curr_reward = discounted_reward->get(t) - mean;
      discounted_reward->put(t, curr_reward);
      sigmasqr += sqr(curr_reward);
   }
   sigmasqr /= T;
   double sigma = sqrt(sigmasqr);

   for(int t = 0; t < T; t++)
   {
      double curr_reward = discounted_reward->get(t) / sigma;
      discounted_reward->put(t, curr_reward);
   }

   policy_backward();

/*      

// Accumulate dW1 and dW2 gradients over batch:

   *dW0_buffer_ptr += *dW0_ptr;
   *dW1_buffer_ptr += *dW1_ptr;
            
// Perform RMSprop parameter update every batch_size episodes:

   episode_number++;
   if(episode_number % batch_size == 0)
   {
      sqr_dW0_buffer_ptr->elementwise_power(*dW0_buffer_ptr, 2);
      sqr_dW1_buffer_ptr->elementwise_power(*dW1_buffer_ptr, 2);

//  MeanSquare(w,t) = 0.9 MeanSquare(w,t-1) + 0.1 (dE(t)/dw)**2

      *rmsprop0_ptr = decay_rate * (*rmsprop0_ptr) +
         (1 - decay_rate) * (*sqr_dW0_buffer_ptr);
      *rmsprop1_ptr = decay_rate * (*rmsprop1_ptr) +
         (1 - decay_rate) * (*sqr_dW1_buffer_ptr);

// Dividing gradient by sqrt(MeanSquare(w,t)) significantly improves learning:

      sqrt_rmsprop0_ptr->elementwise_power(*rmsprop0_ptr, 0.5);
      sqrt_rmsprop1_ptr->elementwise_power(*rmsprop1_ptr, 0.5);

      const double TINY = 1E-5;
      for(unsigned int py = 0; py < W0_ptr->get_mdim(); py++)
      {
         for(unsigned int px = 0; px < W0_ptr->get_ndim(); px++)
         {
            double term1 = W0_ptr->get(px, py);
            double denom = sqrt_rmsprop0_ptr->get(px, py) + TINY;
            double term2 = learning_rate * rmsprop0_ptr->get(px,py)/denom;
            W0_ptr->put(px, py, term1 + term2);
         }
      }

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

      dW0_buffer_ptr->clear_values();
      dW1_buffer_ptr->clear_values();
   } // episode % batch_size == 0 conditional
*/

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

