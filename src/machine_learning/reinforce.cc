// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/11/16; 10/12/16; 10/18/16; 10/19/16
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
   batch_size = 10;	// Perform parameter update after this many episodes
//   batch_size = 5;	// Perform parameter update after this many episodes
   learning_rate = 1E-3;  // Better than 1E-4
//   learning_rate = 1E-4;
   lambda = 0.0;	// L2 regularization coefficient (better than 1E-3)
//   lambda = 0.001;	// L2 regularization coefficient
//   gamma = 0.99;	// Discount factor for reward
   gamma = 0.90;	// Discount factor for reward
   rmsprop_decay_rate = 0.95; 
//   rmsprop_decay_rate = 0.99; 
   
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
// given an input set of values.  See "Forward & backward propagation
// for one episode of reinforcement learning" notes dated 10/18/2016.

void reinforce::policy_forward(int t, genvector& x_input)
{
   a[0]->put_column(t, x_input);
//   cout << "inside policy_forward, x_input = " << x_input << endl;

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
      a[l+1]->put_column(t, a_lplus1_t);
//      cout << "a[l+1] = " << a[l+1]->get_column(t) << endl;
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
//      cout << "t = " << t << " discounted_reward = "
//           << running_add << endl;
   }
}

// ---------------------------------------------------------------------
// See "Forward & backward propagation for one episode of
// reinforcement learning" notes dated 10/18/2016.

void reinforce::policy_backward()
{

// Initialize "episode" weight gradients to zero:

   for(int l = 0; l < n_layers - 1; l++)
   {
      delta_nabla_weights[l]->clear_values();
   }

   int curr_layer = n_layers - 1;
   for(int t = 0; t < T; t++)
   {
      
// Eqn BP1:

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
//   cout << "*delta_prime[curr_layer] = " << *delta_prime[curr_layer]
//        << endl;
   
   for(curr_layer = n_layers-1; curr_layer >= 1; curr_layer--)
   {
      int prev_layer = curr_layer - 1;

//      cout << "prev_layer = " << prev_layer 
//           << " curr_layer = " << curr_layer << endl;

// Eqn BP2A:

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

      *delta_prime[prev_layer] = weights[prev_layer]->transpose() * 
         (*delta_prime[curr_layer]);

//      cout << "*delta_prime[prev_layer] = " << *delta_prime[prev_layer]
//           << endl;

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
   T = 0;
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
//   cout << "curr_timestep = " << curr_timestep 
//        << " curr_reward = " << curr_reward << endl;
   curr_timestep++;
   T++;
}

// ---------------------------------------------------------------------
void reinforce::update_weights(bool episode_finished_flag)
{
   if(!episode_finished_flag) return;

//   cout << "inside reinforce::update_weights()" << endl;

   T_values.push_back(T);
   if(T_values.size() > 1000)
   {
      T_values.pop_front();
   }
   
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
//   cout << "mean = " << mean << endl;

   double sigma = 1;
   if(T > 1)
   {
      double sigmasqr = 0;
      for(int t = 0; t < T; t++)
      {
         double curr_reward = discounted_reward->get(t) - mean;
         discounted_reward->put(t, curr_reward);
         sigmasqr += sqr(curr_reward);
      }
      sigmasqr /= T;
//      cout << "sigmasqr = " << sigmasqr << endl;
      sigma = sqrt(sigmasqr);
   }
   
   for(int t = 0; t < T; t++)
   {
      double curr_reward = discounted_reward->get(t) / sigma;
      discounted_reward->put(t, curr_reward);
//      cout << "t = " << t << " discounted_reward = "
//           << discounted_reward->get(t) << endl;
   }

   policy_backward();

// Accumulate weights' gradients for each network layer:

//   cout << "T = " << T << endl;
   for(int l = 0; l < n_layers - 1; l++)
   {
      *nabla_weights[l] += *delta_nabla_weights[l] / T;
//      cout << "l = " << l << " nabla_weights[l] = " << *nabla_weights[l]
//           << endl;

      *rmsprop_weights_cache[l] = 
         rmsprop_decay_rate * (*rmsprop_weights_cache[l])
         + (1 - rmsprop_decay_rate) * nabla_weights[l]->hadamard_power(2);
   }
   
// Perform RMSprop parameter update every batch_size episodes:

   if(episode_number % batch_size == 0)
   {
      for(int l = 0; l < n_layers - 1; l++)
      {
         *rmsprop_weights_cache[l] = 
            rmsprop_decay_rate * (*rmsprop_weights_cache[l])
            + (1 - rmsprop_decay_rate) * nabla_weights[l]->hadamard_power(2);
//         cout << "l = " << l << " rmsprop_weights_cache[l] = "
//              << *rmsprop_weights_cache[l] << endl;
      }

// Update weights and biases for eacy network layer by their nabla
// values averaged over the current mini-batch:

      const double epsilon = 1E-5;
      for(int l = 0; l < n_layers - 1; l++)
      {
         genmatrix denom = rmsprop_weights_cache[l]->hadamard_power(0.5);
         denom.hadamard_sum(epsilon);
//         cout << "l = " << l << " denom = " << denom << endl;

         *weights[l] -= learning_rate * 
            nabla_weights[l]->hadamard_division(denom);
         nabla_weights[l]->clear_values();
      }

//       print_weights();
   } // episode % batch_size == 0 conditional

   if(running_reward == 0)
   {
      running_reward = reward_sum;
   }
   else
   {
      running_reward = 0.90 * running_reward + 0.10 * reward_sum;
   }
//    cout << "Episode reward total was " << reward_sum << endl;
   reward_sum = 0;

   bool print_flag = false;
   if(episode_number % 10000 == 0) print_flag = true;
   if(print_flag)
   {
      double mu_T, sigma_T;
      mathfunc::mean_and_std_dev(T_values, mu_T, sigma_T);
      cout << "episode_number = " << episode_number << endl;
      cout << "  T = " << mu_T << " +/- " << sigma_T << endl;
      cout << "  Running reward mean = " << running_reward << endl;
   }
}

// ---------------------------------------------------------------------
void reinforce::print_weights()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      cout << "layer = " << l << endl;
      cout << "weights[l] = " << *weights[l] << endl;
   }
}

