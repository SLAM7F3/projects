// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 11/26/16; 11/27/16; 11/28/16; 11/29/16
// ==========================================================================

#include <string>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "machine_learning/reinforce.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
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
   debug_flag = false;
   cum_time_counter = 0;

// This particular set of hyperparameters yields perfect reinforcement
// learning for an agent not to place any of its pieces into already
// occupied cells within a 4x4 grid !

   batch_size = 1;	// Perform parameter update after this many episodes
   base_learning_rate = 1E-4;  //
   learning_rate = base_learning_rate;
   mu = 0.9;		// Coefficient for momentum solver type
   lambda = 0.0;	// L2 regularization coefficient (better than 1E-3)
   gamma = 0.5;	// Discount factor for reward
   rmsprop_decay_rate = 0.85;
   running_reward = -1000;
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
   hardwired_output_action = -1;

   p_action = new genvector(n_actions);
   pcum_action = new genvector(n_actions);

// Biases for all network layers:

   for(int l = 0; l < n_layers; l++)
   {
      genvector *curr_biases = new genvector(layer_dims[l]);
      biases.push_back(curr_biases);
      genvector *curr_old_biases = new genvector(layer_dims[l]);
      old_biases.push_back(curr_old_biases);

      genvector *curr_nabla_biases = new genvector(layer_dims[l]);
      nabla_biases.push_back(curr_nabla_biases);
      genvector *curr_delta_nabla_biases = new genvector(layer_dims[l]);
      delta_nabla_biases.push_back(curr_delta_nabla_biases);

// Initialize bias for each network node in layers 1, 2, ... to be
// zero.  Recall input layer has no biases:

      for(int i = 0; i < layer_dims[l]; i++)
      {
         curr_biases->put(i, 0);
         curr_old_biases->put(i, curr_biases->get(i));
      } // loop over index i labeling node in current layer
   } // loop over index l labeling network layers

// Weights link layer l with layer l+1:

// FAKE FAKE:  Tues Nov 29 at 5:15 am
// Should really only instantiate genmatrices which are needed depending
// upon selected solver type

   for(int l = 0; l < n_layers; l++)
   {
      genvector *curr_rmsprop_biases = new genvector(layer_dims[l]);
      curr_rmsprop_biases->clear_values();
      rmsprop_biases_cache.push_back(curr_rmsprop_biases);

      genvector *curr_rms_biases_denom = new genvector(layer_dims[l]);
      curr_rms_biases_denom->clear_values();
      rms_biases_denom.push_back(curr_rms_biases_denom);
   }
    
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      genmatrix *curr_old_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      genmatrix *curr_velocity = new genmatrix(
         layer_dims[l+1], layer_dims[l]);  // used for momentum solver type
      genmatrix *prev_velocity = new genmatrix(
         layer_dims[l+1], layer_dims[l]);  // used for Nesterov solver type

      genmatrix *curr_adam_m = new genmatrix(
         layer_dims[l+1], layer_dims[l]);  // used for ADAM solver type
      genmatrix *curr_adam_v = new genmatrix(
         layer_dims[l+1], layer_dims[l]);  // used for ADAM solver type

      int mdim = layer_dims[l+1];
      int ndim = layer_dims[l];
      cout << "l = " << l << " mdim = " << mdim << " ndim = " << ndim
           << endl;
      
      weights.push_back(curr_weights);
      old_weights.push_back(curr_old_weights);
      genmatrix *curr_weights_transpose = new genmatrix(
         layer_dims[l], layer_dims[l+1]);
      weights_transpose.push_back(curr_weights_transpose);

      curr_velocity->clear_values();
      velocities.push_back(curr_velocity);
      prev_velocity->clear_values();
      prev_velocities.push_back(prev_velocity);

      curr_adam_m->clear_values();
      adam_m.push_back(curr_adam_m);
      curr_adam_v->clear_values();
      adam_v.push_back(curr_adam_v);

      genmatrix *curr_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_nabla_weights->clear_values();
      nabla_weights.push_back(curr_nabla_weights);

      genmatrix *curr_delta_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_delta_nabla_weights->clear_values();
      delta_nabla_weights.push_back(curr_delta_nabla_weights);

      genmatrix *curr_rmsprop_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_rmsprop_weights->clear_values();
      rmsprop_weights_cache.push_back(curr_rmsprop_weights);

      genmatrix *curr_rms_weights_denom = 
         new genmatrix(layer_dims[l+1], layer_dims[l]);
      curr_rms_weights_denom->clear_values();
      rms_weights_denom.push_back(curr_rms_weights_denom);

// Xavier initialize weights connecting network layers l and l+1 to be
// gaussian random vars distributed according to N(0,1/sqrt(n_in)):

      for(int i = 0; i < layer_dims[l+1]; i++)
      {
         for(int j = 0; j < layer_dims[l]; j++)
         {
            curr_weights->put(i, j, nrfunc::gasdev() / sqrt(layer_dims[l]) );
            curr_old_weights->put(i, j, curr_weights->get(i, j));
         } // loop over index j labeling node in next layer
      } // loop over index i labeling node in current layer

   } // loop over index l labeling neural net layers

   snapshots_subdir="";

// Q learning variable initialization:

   environment_ptr = NULL;
   replay_memory_full_flag = false;
   replay_memory_index = 0;
   epsilon = 1.000001;
   solver_type = RMSPROP;

   beta1 = beta2 = 0;
   curr_beta1_pow = curr_beta2_pow = 1;
}

// ---------------------------------------------------------------------
void reinforce::allocate_member_objects()
{
   y = new genvector(Tmax);
   reward = new genvector(Tmax);
   discounted_reward = new genvector(Tmax);

   s_curr = new genmatrix(replay_memory_capacity, layer_dims.front());
   a_curr = new genvector(replay_memory_capacity);
   r_curr = new genvector(replay_memory_capacity);
   s_next = new genmatrix(replay_memory_capacity, layer_dims.front());
   terminal_state = new genvector(replay_memory_capacity);

   curr_s_sample = new genvector(layer_dims.front());
   next_s_sample = new genvector(layer_dims.front());
   prev_afterstate_ptr = new genvector(layer_dims.front());
}		       

// ---------------------------------------------------------------------
reinforce::reinforce(const vector<int>& n_nodes_per_layer, int Tmax)
{
   this->Tmax = Tmax;
   this->replay_memory_capacity = 1;
   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();
}

reinforce::reinforce(const vector<int>& n_nodes_per_layer, int Tmax,
                     int batch_size, int replay_memory_capacity,
                     int solver_type)
{
   this->Tmax = Tmax;
   this->batch_size = batch_size;
   this->replay_memory_capacity = replay_memory_capacity;
   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();
   this->solver_type = solver_type;
}

// ---------------------------------------------------------------------
reinforce::reinforce()
{
   Tmax = 1;
   allocate_member_objects();
   import_snapshot();
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

   for(unsigned int l = 0; l < biases.size(); l++)
   {
      delete biases[l];
      delete old_biases[l];
      delete nabla_biases[l];
      delete delta_nabla_biases[l];
   }

   for(unsigned int l = 0; l < weights.size(); l++)
   {
      delete weights[l];
      delete old_weights[l];
      delete weights_transpose[l];
      delete nabla_weights[l];
      delete delta_nabla_weights[l];
      delete rmsprop_weights_cache[l];
      delete rms_biases_denom[l];
      delete rms_weights_denom[l];
      delete velocities[l];
      delete prev_velocities[l];
      delete adam_m[l];
      delete adam_v[l];
   }

   delete p_action;
   delete pcum_action;
   delete y;
   delete reward;
   delete discounted_reward;

   delete s_curr;
   delete a_curr;
   delete r_curr;
   delete s_next;
   delete terminal_state;

   delete curr_s_sample;
   delete next_s_sample;
   delete prev_afterstate_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const reinforce& R)
{
   outstream << endl;
   outstream << "batch_size = " << R.batch_size << " base_learning_rate = "
             << R.base_learning_rate << endl;
   outstream << "gamma = " << R.gamma << endl;
   return outstream;
}


// ---------------------------------------------------------------------
void reinforce::hardwire_output_action(int a)
{
   hardwired_output_action = a;
}

// ---------------------------------------------------------------------
// Member function policy_forward returns the output of the network
// given an input set of values.  See "Forward & backward propagation
// for one episode of reinforcement learning" notes dated 10/18/2016.

void reinforce::policy_forward(int t, bool enforce_constraints_flag,
                               genvector *legal_actions)
{
//   cout << "inside policy_forward(), t = " << t << endl;
   a[0]->put_column(t, *x_input);
 
   for(int l = 0; l < n_layers-2; l++)
   {
      z[l+1]->matrix_column_mult(*weights[l], *a[l], t);
      machinelearning_func::ReLU(t, *z[l+1], *a[l+1]);
   }

   z[n_layers-1]->matrix_column_mult(*weights[n_layers-2], *a[n_layers-2], t);

   if(!enforce_constraints_flag)
   {
      machinelearning_func::softmax(t, *z[n_layers-1], *a[n_layers-1]);
   }
   else
   {
      if(hardwired_output_action >= 0)
      {
         machinelearning_func::hardwire_output_action(
            t, hardwired_output_action, *a[n_layers-1]);
         hardwired_output_action = -1;
      }
      else
      {
         if(legal_actions != NULL)
         {
            machinelearning_func::constrained_softmax(
               t, *legal_actions, *z[n_layers-1], *a[n_layers-1]);
         }
         else
         {
            machinelearning_func::constrained_softmax(
               t, *x_input, *z[n_layers-1], *a[n_layers-1]);
         }
         
      } 
   } // enforce_constraints_flag conditional
}

// ---------------------------------------------------------------------
void reinforce::get_softmax_action_probs(int t)
{
   *p_action = a[n_layers-1]->get_column(t);
   compute_cumulative_action_dist();
}

// ---------------------------------------------------------------------
double reinforce::compute_cross_entropy_loss(int t) 
{
   get_softmax_action_probs(t);
   int a_star = y->get(t);
   double curr_prob = p_action->get(a_star);
   double curr_loss = 15;
   double exp_neg_15 = 3.05902321E-7;
   if(curr_prob > exp_neg_15)
   {
      curr_loss = -log(curr_prob);
   }
   return curr_loss;
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

// Standardize discounted rewards to be unit normal to help control
// the gradient estimator variance:

   double mean = 0;
   for(int t = 0; t < T; t++)
   {
      mean += discounted_reward->get(t);
   }
   mean /= T;

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
      sigma = sqrt(sigmasqr);
   }

   if(sigma > 0)
   {
      for(int t = 0; t < T; t++)
      {
         double curr_reward = discounted_reward->get(t) / sigma;
         discounted_reward->put(t, curr_reward);
//      cout << "t = " << t << " discounted_reward = "
//           << discounted_reward->get(t) << endl;
      }
   } // sigma > 0 conditional
   
}

// ---------------------------------------------------------------------
// See "Forward & backward propagation for one episode of
// reinforcement learning" notes dated 10/18/2016.

void reinforce::policy_backward()
{
//   cout << "inside policy_backward()"  << endl;

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

//   if(debug_flag)
//   {
//      cout << "*delta_prime[curr_layer] = " << *delta_prime[curr_layer]
//           << endl;
//   }
 
   for(curr_layer = n_layers-1; curr_layer >= 1; curr_layer--)
   {
      int prev_layer = curr_layer - 1;

//      if(debug_flag)
//      {
//         cout << "prev_layer = " << prev_layer 
//              << " curr_layer = " << curr_layer << endl;
//      }

// Eqn BP2A:

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

      weights_transpose[prev_layer]->matrix_transpose(*weights[prev_layer]);
      delta_prime[prev_layer]->matrix_mult(
         *weights_transpose[prev_layer], *delta_prime[curr_layer]);

//      if(debug_flag)
//      {
//         cout << "*delta_prime[prev_layer] = " << *delta_prime[prev_layer]
//              << endl;
//      }

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

         delta_nabla_weights[prev_layer]->accumulate_outerprod(
            delta_prime[curr_layer]->get_column(t),
            a[prev_layer]->get_column(t));

/*

// As of 11/4/16, we comment out L2-regularization for deep
// reinforcement learning:

         const double TINY = 1E-8;<
         if(lambda > TINY)
         {
            *delta_nabla_weights[prev_layer] += 
               2 * lambda * (*weights[prev_layer]);
         }
*/

      } // loop over index t 

      if(debug_flag)
      {
         cout << "prev_layer = " << prev_layer << endl;
         cout << "*delta_nabla_weights[prev_layer] = " 
              << *delta_nabla_weights[prev_layer]
              << endl;
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
// Member function compute_action_probs() imports an input
// state vector.  

void reinforce::compute_action_probs(
   genvector *x_input, bool enforce_constraints_flag, genvector* legal_actions)
{
//   cout << "curr_timestep = " << curr_timestep << endl;
   this->x_input = x_input;
   policy_forward(curr_timestep, enforce_constraints_flag, legal_actions);
   get_softmax_action_probs(curr_timestep);  // n_actions x 1

   compute_cumulative_action_dist();
}

// ---------------------------------------------------------------------
// Member function normalize_action_distribution()
// renormalizes action probabilities and computes their cumulative
// probability distribution.

// Note as of 11/4/16, we believe that explicit prob dist
// renormalization no longer needs to be performed !!!

void reinforce::renormalize_action_distribution()
{
//   cout << "inside renormalize_action_dist()" << endl;
   double denom = 0;
   for(int a = 0; a < n_actions; a++)
   {
      denom += p_action->get(a);
   }
//    print_p_action();

/*
   const double TINY=1E-100;
   if(denom < TINY)
   {
      cout << "denom = " << denom << endl;
      redistribute_action_probs();
   }
   else
*/
   {
      for(int a = 0; a < n_actions; a++)
      {
         p_action->put(a, p_action->get(a) / denom);
      }
   }

   compute_cumulative_action_dist();
}

// ---------------------------------------------------------------------
void reinforce::compute_cumulative_action_dist()
{
//   cout << "inside compute_cum_action_dist()" << endl;
   double pcum = 0;
   for(int a = 0; a < n_actions; a++)
   {
      pcum += p_action->get(a);
      pcum_action->put(a, pcum);
   }
//   cout << "pcum = " << pcum << endl;

   if(pcum < 0.999 || pcum > 1.001)
   {
      cout << "Danger in compute_cum_action_dist(): pcum = " << pcum << endl;
      print_p_action();
      redistribute_action_probs();
   }   
}

// ---------------------------------------------------------------------
void reinforce::redistribute_action_probs()
{
   int n_unoccupied_cells = 0;
   for(unsigned int i = 0; i < x_input->get_mdim(); i++)
   {
      if(nearly_equal(x_input->get(i), 0))
      {
         n_unoccupied_cells++;
      }
   }

   p_action->clear_values();
   for(unsigned int i = 0; i < x_input->get_mdim(); i++)
   {
      if(nearly_equal(x_input->get(i), 0))
      {
         p_action->put(i, 1.0 / n_unoccupied_cells);
      }
   }
   compute_cumulative_action_dist();
}

// ---------------------------------------------------------------------
void reinforce::print_p_action() const
{ 
  for(int a = 0; a < n_actions; a++)
   {
      cout << "a = " << a << " p_action = " << p_action->get(a)
           << " p_cum = " << pcum_action->get(a)
           << endl;
   }
}

// ---------------------------------------------------------------------
// Member function get_candidate_current_action() returns the index
// for the action which is probabilistically selected based upon the
// current softmax action distribution.

int reinforce::get_candidate_current_action()
{

// Generate uniformly-distributed random variable.  Use it to
// inversely sample cumulative probability distribution to set action
// a_star:

//   if(debug_flag) print_p_action();

   int a_star = 0;
   double q = nrfunc::ran1();
   for(int a = 0; a < n_actions; a++)
   {
      double plo, phi;
      if(a == 0)
      {
         plo = 0;
         phi = pcum_action->get(a);
      }
      else if (a == n_actions - 1)
      {
         plo = pcum_action->get(a - 1);
         phi = 1.01;
      }
      else
      {
         plo = pcum_action->get(a - 1);
         phi = pcum_action->get(a);
      }

      if(q >= plo && q < phi)
      {
         a_star = a;
      }
   }
   return a_star;
}

// ---------------------------------------------------------------------
void reinforce::set_current_action(int a_star)
{
   y->put(curr_timestep, a_star);
}

// ---------------------------------------------------------------------
void reinforce::snapshot_running_reward()
{
   running_reward_snapshots.push_back(running_reward);
}

// ---------------------------------------------------------------------
void reinforce::record_reward_for_action(double curr_reward)
{
   reward_sum += curr_reward;
   reward->put(curr_timestep, curr_reward);
}

// ---------------------------------------------------------------------
void reinforce::increment_time_counters()
{
   cum_time_counter++;
   curr_timestep++;
   T++;
}

// ---------------------------------------------------------------------
void reinforce::periodically_snapshot_loss_value()
{
   if(cum_time_counter > 0 && cum_time_counter%3000 == 0)
   {
      time_samples.push_back(cum_time_counter);
      loss_values.push_back(compute_cross_entropy_loss(curr_timestep));
   }
}

// ---------------------------------------------------------------------
void reinforce::update_T_values()
{
//   cout << "inside update_T_values()" << endl;

   T_values.push_back(T);
   if(T_values.size() > 1000)
   {
      T_values.pop_front();
   }
}

// ---------------------------------------------------------------------
void reinforce::update_weights()
{
//   cout << "inside update_weights()" << endl;

   update_T_values();

// Compute the discounted reward backwards through time:

   discount_rewards();

   policy_backward();

// Accumulate weights' gradients for each network layer:

   double inverse_T = 1.0 / T;
   for(int l = 0; l < n_layers - 1; l++)
   {
      nabla_weights[l]->matrix_increment(inverse_T, *delta_nabla_weights[l]);
   }
   
// Perform RMSprop parameter update every batch_size episodes:

   if(episode_number > 0 && episode_number % batch_size == 0)
   {
      for(int l = 0; l < n_layers - 1; l++)
      {
         for(unsigned int i=0; i < rmsprop_weights_cache[l]->get_mdim(); i++) 
         {
            for(unsigned int j=0; j<rmsprop_weights_cache[l]->get_ndim(); j++)
            {
               double curr_val = 
                  rmsprop_decay_rate * rmsprop_weights_cache[l]->get(i,j)
                  + (1 - rmsprop_decay_rate) * sqr(nabla_weights[l]->get(i,j));
               rmsprop_weights_cache[l]->put(i,j,curr_val);
            } // loop over index j labeling columns
         } // loop over index i labeling rows
      }

// Update weights and biases for each network layer by their nabla
// values averaged over the current mini-batch:

      const double eps = 1E-5;
      for(int l = 0; l < n_layers - 1; l++)
      {
         rms_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         rms_weights_denom[l]->hadamard_sum(eps);
         nabla_weights[l]->hadamard_division(*rms_weights_denom[l]);

         *weights[l] -= learning_rate * (*nabla_weights[l]);

         if(debug_flag)
         {
            int mdim = nabla_weights[l]->get_mdim();
            int ndim = nabla_weights[l]->get_ndim();

            vector<double> curr_nabla_weights;
            vector<double> curr_nabla_weight_ratios;
            for(int r = 0; r < mdim; r++)
            {
               for(int c = 0; c < ndim; c++)
               {
                  curr_nabla_weights.push_back(
                     fabs(nabla_weights[l]->get(c,r)));
//               cout << "r = " << r << " c = " << c
//                    << " curr_nabla_weight = " 
//                    << curr_nabla_weights.back() << endl;
                  double denom = weights[l]->get(c,r);
                  if(fabs(denom) > 1E-10)
                  {
                     curr_nabla_weight_ratios.push_back(
                        fabs(nabla_weights[l]->get(c,r) / denom ));
                  }
               }
            }
            double mean_abs_nabla_weight = mathfunc::mean(curr_nabla_weights);
            double mean_abs_nabla_weight_ratio = mathfunc::mean(
               curr_nabla_weight_ratios);
            
//         double median_abs_nabla_weight = mathfunc::median_value(
//            curr_nabla_weights);
//         double median_abs_nabla_weight_ratio = mathfunc::median_value(
//            curr_nabla_weight_ratios);
            cout << "layer l = " << l
                 << " mean |nabla weight| = " 
                 << mean_abs_nabla_weight 
//              << " median |nabla weight| = " 
//              << median_abs_nabla_weight  
                 << " mean |nabla weight| ratio = " 
                 << mean_abs_nabla_weight_ratio  << endl;

            cout << " lr * mean weight ratio = " 
                 << learning_rate * mean_abs_nabla_weight 
                 << " lr * mean weight ratio = " 
                 << learning_rate * mean_abs_nabla_weight_ratio << endl;
         } // debug_flag conditional
         
         nabla_weights[l]->clear_values();
      }
//       cout << endl;
//      print_weights();
   } // episode % batch_size == 0 conditional
}

// ---------------------------------------------------------------------
void reinforce::update_running_reward(string extrainfo)
{
   if(episode_number == 0)
   {
      running_reward = reward_sum;
   }
   else
   {
      running_reward = 0.95 * running_reward + 0.05 * reward_sum;
   }
   reward_sum = 0;

   bool print_flag = false;
   if(episode_number > 0 && episode_number % 5000 == 0) print_flag = true;
   if(print_flag)
   {
      double mu_T, sigma_T;
      mathfunc::mean_and_std_dev(T_values, mu_T, sigma_T);
      cout << "base learning rate="+stringfunc::scinumber_to_string(
         base_learning_rate,2)
           << " learning_rate="+stringfunc::scinumber_to_string(
              learning_rate, 2);
      cout << " gamma="+stringfunc::number_to_string(gamma,3)
           << " rms_decay="+stringfunc::number_to_string(rmsprop_decay_rate,3)
           << endl;
      cout << "batch_size = " << batch_size << " lambda = " << lambda << endl;
      cout << extrainfo << endl;
      cout << "episode_number = " << episode_number << endl;
      cout << "  T = " << mu_T << " +/- " << sigma_T << endl;
      cout << "  Running reward mean = " << running_reward << endl;
   }
}

// ==========================================================================
// Monitoring network training methods
// ==========================================================================

int reinforce::count_weights()
{
   int n_weights = 0;
   for(int l = 0; l < n_layers - 1; l++)
   {
      n_weights += weights[l]->get_mdim() * weights[l]->get_ndim();
   }
   return n_weights;
}

void reinforce::print_biases()
{
   for(int l = 0; l < n_layers; l++)
   {
      cout << "layer = " << l << endl;
      cout << "biases[l].mdim = " << biases[l]->get_mdim()
           << endl;
      cout << "biases[l] = " << *biases[l] << endl;
   }
}

void reinforce::print_weights()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      cout << "layer = " << l << endl;
      cout << "weights[l].mdim = " << weights[l]->get_mdim()
           << " weights[l].ndim = " << weights[l]->get_ndim()
           << endl;
      cout << "weights[l] = " << *weights[l] << endl;
   }
}

// ---------------------------------------------------------------------
// Member function plot_zeroth_layer_weights() renormalizes copies of
// all weights within the zeroth layer to range between 0 and 1.  It
// then exports the renormalized weight values as PNG images to the
// specified output subdirectory.

void reinforce::plot_zeroth_layer_weights(string output_subdir)
{
   int n_zeroth_layer_weights = weights[0]->get_mdim();
   int n_zeroth_layer_pixels = weights[0]->get_ndim();
   int nx = sqrt(double(n_zeroth_layer_pixels));
   int ny = nx;
   
   double min_weight_val = POSITIVEINFINITY;
   double max_weight_val = NEGATIVEINFINITY;
   for(int n = 0; n < n_zeroth_layer_weights; n++)
   {
      for(int p = 0; p < n_zeroth_layer_pixels; p++)
      {
         min_weight_val = basic_math::min(
            min_weight_val, weights[0]->get(n, p));
         max_weight_val = basic_math::max(
            max_weight_val, weights[0]->get(n, p));
      }
   } // loop over index n labeling weight images

//   cout << "min_weight_val = " << min_weight_val
//        << " max_weight_val = " << max_weight_val << endl;

// Renormalize image weight values to range from 0 to 255:

   vector<twoDarray*> weight_image_ptrs;
   
   for(int n = 0; n < n_zeroth_layer_weights; n++)
   {
      twoDarray* wtwoDarray_ptr = new twoDarray(nx, ny);
      environment_ptr->append_wtwoDarray(wtwoDarray_ptr);

      for(int p = 0; p < n_zeroth_layer_pixels; p++)
      {
         int pu = p % ny;
         int pv = p / ny;
         double curr_weight_val = 255 * 
            (weights[0]->get(n,p) - min_weight_val) / 
            (max_weight_val - min_weight_val);
         wtwoDarray_ptr->put(pu, pv, curr_weight_val);
      }

      int magnify_factor = 15;
      twoDarray* enlarged_wtwoDarray_ptr = new twoDarray(
         magnify_factor * nx, magnify_factor * ny);

      for(unsigned int py = 0; py < enlarged_wtwoDarray_ptr->get_mdim(); py++)
      {
         int pv = py / magnify_factor;
         for(unsigned int px = 0; px < enlarged_wtwoDarray_ptr->get_ndim(); 
             px++)
         {
            int pu = px / magnify_factor;
            double curr_weight_val = wtwoDarray_ptr->get(pu, pv);
            enlarged_wtwoDarray_ptr->put(px, py, curr_weight_val);
         }
      }

      int n_channels = 3;
      texture_rectangle* tr_ptr = new texture_rectangle(
         magnify_factor * nx, magnify_factor * ny, 1, n_channels, NULL);
      tr_ptr->convert_single_twoDarray_to_three_channels(
         enlarged_wtwoDarray_ptr, true);

      double hue_min =270;
      tr_ptr->convert_grey_values_to_hues(hue_min);

      string output_filename=output_subdir + 
         "weights_"+stringfunc::integer_to_string(n,3)+".png";
      tr_ptr->write_curr_frame(output_filename);
      string banner="Exported "+output_filename;
      outputfunc::write_banner(banner);

      delete tr_ptr;
//      delete wtwoDarray_ptr;
      delete enlarged_wtwoDarray_ptr;

   } // loop over index n labeling weight images
}

// ---------------------------------------------------------------------
void reinforce::compute_weight_distributions()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      vector<double> weight_values;
      for(unsigned int r = 0; r < weights[l]->get_mdim(); r++)
      {
         for(unsigned int c = 0; c < weights[l]->get_ndim(); c++)
         {
            weight_values.push_back(weights[l]->get(r,c));
         }
      }
      int nbins = 500;
      double wlo = mathfunc::minimal_value(weight_values);
      double whi = mathfunc::maximal_value(weight_values);
      prob_distribution prob_weights(nbins, wlo, whi, weight_values);

      double w_05 = prob_weights.find_x_corresponding_to_pcum(0.05);
      double w_25 = prob_weights.find_x_corresponding_to_pcum(0.25);
      double w_median = prob_weights.median();
      double w_75 = prob_weights.find_x_corresponding_to_pcum(0.75);
      double w_95 = prob_weights.find_x_corresponding_to_pcum(0.95);
      cout << "layer = " << l
           << " wlo = " << wlo
           << " w_05 = " << w_05
           << " w_25 = " << w_25 << endl;
      cout << "   w_50 = " << w_median
           << " w_75 = " << w_75
           << " w_95 = " << w_95 
           << " whi = " << whi
           << endl;
   }
}

// ---------------------------------------------------------------------
string reinforce::init_subtitle()
{
   string subtitle=
      "blr="+stringfunc::scinumber_to_string(base_learning_rate,2)+
      "; gamma="+stringfunc::scinumber_to_string(gamma,2)+
      "; ";
   if(solver_type == SGD)
   {
      subtitle += "SGD";
   }
   else if(solver_type == RMSPROP)
   {
      subtitle += "RMSPROP;";
      subtitle += " decay="+stringfunc::scinumber_to_string(
         rmsprop_decay_rate,2);
   }
   else if(solver_type == MOMENTUM)
   {
      subtitle += "MOMENTUM";
   }
   else if(solver_type == NESTEROV)
   {
      subtitle += "NESTEROV";
   }
   else if(solver_type == ADAM)
   {
      subtitle += "ADAM";
      subtitle += " b1="+stringfunc::scinumber_to_string(beta1,2);
      subtitle += " b2="+stringfunc::scinumber_to_string(beta2,2);
   }
   return subtitle;
}

// ---------------------------------------------------------------------
// Generate metafile plot of loss values versus time step samples.

void reinforce::plot_loss_history(string output_subdir, string extrainfo)
{
   if(loss_values.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"loss_history";
   string title="Loss function; bsize="+
      stringfunc::number_to_string(batch_size);
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
   }
   
   string subtitle=init_subtitle();
   subtitle += ";"+extrainfo;
   string x_label="Time step";
   string y_label="Loss";

   double extremal_min_loss = mathfunc::minimal_value(loss_values);
   double extremal_max_loss = mathfunc::maximal_value(loss_values);
   double min_loss, max_loss;

   if(nearly_equal(extremal_min_loss, extremal_max_loss))
   {
      max_loss = extremal_max_loss + 1;
   }
   else
   {
      mathfunc::lo_hi_values(loss_values, 0.025, 0.975, min_loss, max_loss);
   }
   min_loss = 0;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, time_samples.back(),
      min_loss, max_loss);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.5);
   curr_metafile.set_ysubtic(0.25);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(time_samples, loss_values, colorfunc::red);

// Temporally smooth noisy loss values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

   if(gaussian_size < int(loss_values.size()))
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);

      bool wrap_around_input_values = false;
      vector<double> smoothed_loss_values;
      filterfunc::brute_force_filter(
         loss_values, h, smoothed_loss_values, wrap_around_input_values);

      curr_metafile.set_thickness(3);
      curr_metafile.write_curve(time_samples, smoothed_loss_values, 
                                colorfunc::blue);
   }

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of running reward sum versus time step samples.

void reinforce::plot_reward_history(
   std::string output_subdir, std::string extrainfo, 
   double min_reward, double max_reward)
{
   if(running_reward_snapshots.size() < 5) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"reward_history";
   string title="Running reward sum; bsize="+
      stringfunc::number_to_string(batch_size);
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label="Episode number";
   string y_label="Running reward sum";

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, episode_number,
      min_reward, max_reward);
   curr_metafile.set_subtitle(subtitle);
   double ytic = 0.1 * (max_reward - min_reward);
   curr_metafile.set_ytic(ytic);
   curr_metafile.set_ysubtic(0.5 * ytic);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, episode_number, running_reward_snapshots);

// Temporally smooth noisy loss values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

   vector<double> smoothed_reward_snapshots;
   if(gaussian_size < int(running_reward_snapshots.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);
      
      bool wrap_around_input_values = false;
      filterfunc::brute_force_filter(
         running_reward_snapshots, h, smoothed_reward_snapshots, 
         wrap_around_input_values);

      curr_metafile.set_thickness(3);
      curr_metafile.write_curve(
         0, episode_number, smoothed_reward_snapshots, colorfunc::blue);
   }

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of total number of turns versus episode number.

void reinforce::plot_turns_history(string output_subdir, string extrainfo)
{
   if(n_episode_turns_frac.size() < 5) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"turns_history";
   string title="Number of AI and agent turns; bsize="+
      stringfunc::number_to_string(batch_size);
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label="Episode number";
   string y_label="Number of AI + agent turns";

   double min_turn_frac = 0;
   double max_turn_frac = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, episode_number,
      min_turn_frac, max_turn_frac);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, episode_number, n_episode_turns_frac);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy turns fraction values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

   vector<double> smoothed_n_episode_turns_frac;
   if(gaussian_size < int(n_episode_turns_frac.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);

      bool wrap_around_input_values = false;
      vector<double> smoothed_n_episode_turns_frac;
      filterfunc::brute_force_filter(
         n_episode_turns_frac, h, smoothed_n_episode_turns_frac, 
         wrap_around_input_values);

      curr_metafile.write_curve(
         0, episode_number, smoothed_n_episode_turns_frac, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of Qmap score versus episode number.

void reinforce::plot_Qmap_score_history(string output_subdir, 
                                        string subtitle,string extrainfo)
{
   if(Qmap_scores.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir + "/Qmap_score_history";
   string title="Qmap score";
   title += ";learning rate="+stringfunc::scinumber_to_string(
      base_learning_rate,2);
   title += ";bsize="+stringfunc::number_to_string(batch_size);

//   string subtitle=init_subtitle();
   subtitle += ";"+extrainfo;
   string x_label="Episode number";
   string y_label="Qmap score";

   double min_score = 0;
   double max_score = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, episode_number,
      min_score, max_score);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.set_thickness(2);
   curr_metafile.write_curve(0, episode_number, Qmap_scores);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy Qmap scores:

   double sigma = 5;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);
   cout << "gaussian_size = " << gaussian_size << endl;
   cout << "Qmap_scores.size() = " << Qmap_scores.size() << endl;

   vector<double> smoothed_Qmap_scores;
   if(gaussian_size < int(Qmap_scores.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);

      bool wrap_around_input_values = false;
      filterfunc::brute_force_filter(
         n_episode_turns_frac, h, smoothed_Qmap_scores, 
         wrap_around_input_values);
      curr_metafile.write_curve(
         0, episode_number, smoothed_Qmap_scores, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of log10(total loss) versus episode number.

void reinforce::plot_log10_loss_history(string output_subdir, string extrainfo)
{
   if(log10_losses.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir + "/log10_losses_history";

   string title="Log10(total loss)";
   title += ";blr="+stringfunc::scinumber_to_string(base_learning_rate,2);
   title += ";bsize="+stringfunc::number_to_string(batch_size);

   string subtitle=init_subtitle();
   subtitle += ";"+extrainfo;
   string x_label="Episode number";
   string y_label="Log10(Total loss)";

   double max_score = mathfunc::maximal_value(log10_losses)+0.5;
   double min_score = mathfunc::minimal_value(log10_losses)-0.5;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, episode_number,
      min_score, max_score);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.5);
   curr_metafile.set_ysubtic(0.25);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.set_thickness(2);
   curr_metafile.write_curve(0, episode_number, log10_losses);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy log10(Total Loss) scores:

   double sigma = 3;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 1.5);
   cout << "gaussian_size = " << gaussian_size << endl;

   vector<double> smoothed_log10_losses;
   if(gaussian_size < int(log10_losses.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);

      bool wrap_around_input_values = false;
      filterfunc::brute_force_filter(
         log10_losses, h, smoothed_log10_losses, wrap_around_input_values);
      curr_metafile.write_curve(
         0, episode_number, smoothed_log10_losses, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Member function create_snapshots_subdir()

void reinforce::create_snapshots_subdir(string output_subdir)
{
   Clock clock;
   clock.set_time_based_on_local_computer_clock();
   string timestamp_str = clock.YYYY_MM_DD_H_M_S("_","_",false,0);
   string timestamp_substr = timestamp_str.substr(0,16);

   snapshots_subdir = output_subdir+"snapshots/"+timestamp_substr+"/";
   filefunc::dircreate(snapshots_subdir);
}

// ---------------------------------------------------------------------
// Member function export_snapshot()

void reinforce::export_snapshot(string output_subdir)
{
   if(snapshots_subdir.size() == 0) create_snapshots_subdir(output_subdir);
   
   string snapshot_filename=snapshots_subdir+"snapshot_"+
      stringfunc::integer_to_string(episode_number, 5)+".binary";
   ofstream outstream;
   
//   cout << "inside reinforce::export_snapshot()" << endl;
//   cout << "n_layers = " << n_layers << " n_actions = " << n_actions
//        << endl;

   filefunc::open_binaryfile(snapshot_filename,outstream);
   outstream << n_layers << endl;
   outstream << n_actions << endl;
   for(unsigned int i = 0; i < layer_dims.size(); i++)
   {
      outstream << layer_dims[i] << endl;
//      cout << "i = " << i << " layer_dim = " << layer_dims[i] << endl;
   }
   
   outstream << batch_size << endl;
   outstream << base_learning_rate << endl;
   outstream << learning_rate << endl;
   outstream << lambda << endl;
   outstream << gamma << endl;
   outstream << rmsprop_decay_rate << endl;

//   cout << "batch_size = " << batch_size << endl;
//   cout << "base_learning_rate = " << base_learning_rate << endl;
//   cout << "learning_rate = " << learning_rate << endl;
//   cout << "lambda = " << lambda << endl;
//   cout << "gamma = " << gamma << endl;
//   cout << "rmsprop_decay_rate = " << rmsprop_decay_rate << endl;

   for(unsigned int l = 0; l < weights.size(); l++)
   {
      genmatrix* curr_weights_ptr = weights[l];
      outstream << curr_weights_ptr->get_mdim() << endl;
      outstream << curr_weights_ptr->get_ndim() << endl;
//      cout << "l = " << l << " mdim = " << curr_weights_ptr->get_mdim()
//           << " ndim = " << curr_weights_ptr->get_ndim() << endl;
//      cout << "*curr_weights_ptr = " << *curr_weights_ptr << endl;

      for(unsigned int row = 0; row < curr_weights_ptr->get_mdim(); row++)
      {
         for(unsigned int col = 0; col < curr_weights_ptr->get_ndim(); col++)
         {
            outstream << curr_weights_ptr->get(row,col) << endl;
//            cout << "row = " << row << " col = " << col
//                 << " weights[l] = " << curr_weights_ptr->get(row,col) << endl;
         }
      }
   } // loop over index l labeling weight matrices
   filefunc::closefile(snapshot_filename,outstream);
   cout << "Exported " << snapshot_filename << endl;
}

// ---------------------------------------------------------------------
// Member function import_snapshot()

void reinforce::import_snapshot()
{
   cout << "inside reinforce::import_snapshot()" << endl;

   string snapshots_subdir = "./snapshots/";
   filefunc::dircreate(snapshots_subdir);

   string snapshot_filename=snapshots_subdir+"snapshot.binary";
   ifstream instream;
   
   filefunc::open_binaryfile(snapshot_filename,instream);
   instream >> n_layers;
   instream >> n_actions;
   cout << "n_layers = " << n_layers << " n_actions = " << n_actions 
        << endl;

   vector<int> n_nodes_per_layer;
   for(int i = 0; i < n_layers; i++)
   {
      int curr_layer_dim;
      instream >> curr_layer_dim;
      n_nodes_per_layer.push_back(curr_layer_dim);
      cout << "i = " << i << " n_nodes_per_layer = " << n_nodes_per_layer[i] 
           << endl;
   }

   initialize_member_objects(n_nodes_per_layer);

   instream >> batch_size;
   instream >> base_learning_rate;
   instream >> learning_rate;
   instream >> lambda;
   instream >> gamma;
   instream >> rmsprop_decay_rate;

   cout << "batch_size = " << batch_size << endl;
   cout << "base_learning_rate = " << base_learning_rate << endl;
   cout << "learning_rate = " << learning_rate << endl;
   cout << "lambda = " << lambda << endl;
   cout << "gamma = " << gamma << endl;
   cout << "rmsprop_decay_rate = " << rmsprop_decay_rate << endl;

   for(int l = 0; l < n_layers-1; l++)
   {
      int mdim, ndim;
      instream >> mdim;
      instream >> ndim;

//      cout << "l = " << l << " mdim = " << mdim << " ndim = " << ndim
//           << endl;

      for(int row = 0; row < mdim; row++)
      {
         for(int col = 0; col < ndim; col++)
         {
            double curr_weight;
            instream >> curr_weight;
//            cout << "l = " << l << " mdim = " << mdim << " ndim = " << ndim
//                 << " col = " << col << " row = " << row << endl;
//            cout << "  curr_weight = " << curr_weight << endl;
//            cout << "  weights[l].mdim = " << weights[l]->get_mdim()
//                 << " weights[l].ndim = " << weights[l]->get_ndim()
//                 << endl;
            weights[l]->put(row, col, curr_weight);
         }
      }
//      cout << "weights[l] = " << *weights[l] << endl;
   } // loop over index l labeling weight matrices
   
   filefunc::closefile(snapshot_filename,instream);
   cout << "Imported " << snapshot_filename << endl;
}

// ==========================================================================
// Q learning methods
// ==========================================================================

// Member function copy_weights_onto_old_weights() copies biases onto
// old_biases and weights onto old_weights.  According to David
// Silver's "Deep Reinforcement Learning" notes, using
// frequently-updated weights within the target can lead to neural
// network oscillations and instabilities.  So we should instead
// compute targets using old weights and only periodically update them
// by calling this method.

void reinforce::copy_weights_onto_old_weights() 
{
   for(unsigned int l = 0; l < biases.size(); l++)
   {
      *old_biases[l] = *biases[l];
   }
   for(unsigned int l = 0; l < weights.size(); l++)
   {
      *old_weights[l] = *weights[l];
   }
}

// ---------------------------------------------------------------------
// Member function get_random_legal_action()

int reinforce::get_random_action() const
{
   int curr_a = mathfunc::getRandomInteger(n_actions);
   return curr_a;
}

int reinforce::get_random_legal_action() const
{
   int curr_a = -1;
   bool legal_action = false;
   while(!legal_action)
   {
      curr_a = mathfunc::getRandomInteger(n_actions);
      legal_action = environment_ptr->is_legal_action(curr_a);
   }
   return curr_a;
}

// ---------------------------------------------------------------------
// Member function anneal_epsilon()

double reinforce::anneal_epsilon(double decay_factor, double min_epsilon)
{
   if(epsilon < min_epsilon)
   {
      epsilon = min_epsilon;
   }
   else
   {
      epsilon *= decay_factor;
   }
//    cout << "epsilon = " << epsilon << endl;
   return epsilon;
}

void reinforce::set_epsilon(double eps)
{
   epsilon = eps;
}

double reinforce::get_epsilon() const
{
   return epsilon;
}

// ---------------------------------------------------------------------
// Member function select_action_for_curr_state()

int reinforce::select_action_for_curr_state()
{
   if(nrfunc::ran1() < epsilon)
   {
      return get_random_action();
   }
   else
   {
      genvector* curr_s = environment_ptr->get_curr_state();
      Q_forward_propagate(curr_s);
      return compute_argmax_Q();
   }
}

int reinforce::select_legal_action_for_curr_state()
{
   if(nrfunc::ran1() < epsilon)
   {
      return get_random_legal_action();
   }
   else
   {
      genvector* curr_s = environment_ptr->get_curr_state();
      Q_forward_propagate(curr_s);
      return compute_legal_argmax_Q();
   }
}

// ---------------------------------------------------------------------
// Member function compute_deep_Qvalues() retrieves all possible
// states in both genvector* and string form from the environment.
// Looping over each possible state, it forward propagates the input
// state through the neural network and extracts Q values
// corresponding to each possible action.  The independent
// state-action variables are encoded into a string.  The dependent Q
// values are stored as functions of the state-action strings within
// STL map member qmap.

void reinforce::compute_deep_Qvalues()
{
   vector<genvector*> curr_states = environment_ptr->get_all_curr_states();
   vector<string> curr_state_strings = 
      environment_ptr->get_all_curr_state_strings();

   int t = 0;
   for(unsigned int s = 0; s < curr_states.size(); s++)
   {
      Q_forward_propagate(curr_states[s]);
      for(unsigned int i = 0; i < z[n_layers-1]->get_mdim(); i++)
      {
         string state_action_str = environment_ptr->
            get_state_action_string(curr_state_strings[s], i);
         double Qvalue = a[n_layers-1]->get(i, t);
         set_Q_value(state_action_str, Qvalue);
//         cout << "s = " << s << " a = " << i << " Qvalue = " << Qvalue
//              << endl;
      } // loop over index i labeling actions
   } // loop over index s labeling input states
}

// ---------------------------------------------------------------------
// Member function Q_forward_propagate() performs a feedforward pass
// for the input state s to get predicted Q-values for all actions.

void reinforce::Q_forward_propagate(
   genvector* s_input, bool use_old_weights_flag)
{
   int t = 0;
   a[0]->put_column(t, *s_input);
 
   for(int l = 0; l < n_layers-2; l++)
   {
      if(use_old_weights_flag)
      {
         z[l+1]->matrix_column_mult_sum(
            *old_weights[l], *a[l], *old_biases[l+1],t);
      }
      else
      {
         z[l+1]->matrix_column_mult_sum(*weights[l], *a[l], *biases[l+1], t);
      }
      machinelearning_func::ReLU(t, *z[l+1], *a[l+1]);
   }

   if(use_old_weights_flag)
   {
      z[n_layers-1]->matrix_column_mult_sum(
         *old_weights[n_layers-2], *a[n_layers-2], *old_biases[n_layers-1], t);
   }
   else
   {
      z[n_layers-1]->matrix_column_mult_sum(
         *weights[n_layers-2], *a[n_layers-2], *biases[n_layers-1], t);
   }

   for(unsigned int i = 0; i < z[n_layers-1]->get_mdim(); i++)
   {
      a[n_layers-1]->put(i, t, z[n_layers-1]->get(i,t));
   }
}

// ---------------------------------------------------------------------
// Member function compute_argmax_Q() returns a = argmax_a' Q(s,a').

int reinforce::compute_argmax_Q()
{
   double Qstar = NEGATIVEINFINITY;
   int t = 0;
   int curr_a = -1;
   for(unsigned int i = 0; i < a[n_layers-1]->get_mdim(); i++)
   {
      double curr_activation = a[n_layers-1]->get(i,t);

// As of 11/17/16, we experiment with randomly breaking "ties" between
// Q value scores:

      if(nearly_equal(curr_activation, Qstar))
      {
         if(nrfunc::ran1() > 0.5)
         {
            Qstar = curr_activation;
            curr_a = i;
         }
      }
      else if(curr_activation > Qstar)
      {
         Qstar = curr_activation;
         curr_a = i;
      }
   }
   return curr_a;
}

int reinforce::compute_legal_argmax_Q()
{
   double Qstar = NEGATIVEINFINITY;
   int t = 0;
   int curr_a = -1;
   for(unsigned int i = 0; i < a[n_layers-1]->get_mdim(); i++)
   {
      if(!environment_ptr->is_legal_action(i)) continue;

      double curr_activation = a[n_layers-1]->get(i,t);
   
// As of 11/17/16, we experiment with randomly breaking "ties" between
// Q value scores:

      if(nearly_equal(curr_activation, Qstar))
      {
         if(nrfunc::ran1() > 0.5)
         {
            Qstar = curr_activation;
            curr_a = i;
         }
      }
      else if(curr_activation > Qstar)
      {
         Qstar = curr_activation;
         curr_a = i;
      }
   }
   return curr_a;
}

// ---------------------------------------------------------------------
// Member function store_curr_state_into_replay_memory()

int reinforce::store_curr_state_into_replay_memory(const genvector& curr_s)
{
//   cout << "inside store_curr_state_into_replay_memory()" << endl;
   int d = -1;
   if(replay_memory_index < replay_memory_capacity)
   {
      d = replay_memory_index;
      replay_memory_index++;
//      double fill_frac = double(replay_memory_index) / replay_memory_capacity;
//      cout << "Replay memory fill frac = " << fill_frac << endl;
   }
   else
   {
      replay_memory_full_flag = true;
//      cout << "REPLAY MEMORY IS NOW FULL" << endl;
      replay_memory_index = 0;
      d = replay_memory_index;
   }
//   cout << " d = " << d << endl;
//   cout << "replay_memory_index = " << replay_memory_index << endl;

   s_curr->put_row(d, curr_s);
   return d;
}

// ---------------------------------------------------------------------
// Member function store_arsprime_into_replay_memory()

void reinforce::store_arsprime_into_replay_memory(
   int d, int curr_a, double curr_r,
   const genvector& next_s, bool terminal_state_flag)
{
   a_curr->put(d, curr_a);
   r_curr->put(d, curr_r);
   s_next->put_row(d, next_s);
   terminal_state->put(d, terminal_state_flag);
}

// ---------------------------------------------------------------------
// Member function get_memory_replay_entry()

bool reinforce::get_memory_replay_entry(
   int d, genvector& curr_s, int& curr_a, double& curr_r, genvector& next_s)
{
   s_curr->get_row(d, curr_s);
   double a_val = a_curr->get(d);
   curr_a = int(a_val);
   curr_r = r_curr->get(d);
   
   s_next->get_row(d, next_s);
   bool terminal_state_flag = (terminal_state->get(d) > 0);
   return terminal_state_flag;
}

// ---------------------------------------------------------------------
// Member function compute_target()

double reinforce::compute_target(double curr_r, genvector* next_s,
                                 bool terminal_state_flag)
{
   if(terminal_state_flag)
   {
      return curr_r;
   }
   else
   {
      bool use_old_weights_flag = true;
      Q_forward_propagate(next_s, use_old_weights_flag);
      double Qmax = a[n_layers-1]->get(0,0);
      for(unsigned int j = 0; j < a[n_layers-1]->get_mdim(); j++)
      {
         Qmax = basic_math::max(Qmax, a[n_layers-1]->get(j,0));
      }
      return curr_r + gamma * Qmax;
   }
}

// ---------------------------------------------------------------------
// Member function update_biases_cache()

void reinforce::update_biases_cache(double decay_rate)
{
   for(int l = 0; l < n_layers; l++)
   {
      for(unsigned int i=0; i < rmsprop_biases_cache[l]->get_mdim(); i++) 
      {
         double curr_val = decay_rate * rmsprop_biases_cache[l]->get(i)
            + (1 - decay_rate) * sqr(nabla_biases[l]->get(i));
         rmsprop_biases_cache[l]->put(i,curr_val);
      } // loop over index i
   } // loop over index l labeling network layers
}

void reinforce::update_rmsprop_cache(double decay_rate)
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      for(unsigned int i=0; i < rmsprop_weights_cache[l]->get_mdim(); i++) 
      {
         for(unsigned int j=0; j<rmsprop_weights_cache[l]->get_ndim(); j++)
         {
            double curr_val = 
               decay_rate * rmsprop_weights_cache[l]->get(i,j)
               + (1 - decay_rate) * sqr(nabla_weights[l]->get(i,j));
            rmsprop_weights_cache[l]->put(i,j,curr_val);
         } // loop over index j labeling columns
      } // loop over index i labeling rows
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
// Member function update_neural_network()

double reinforce::update_neural_network()
{
//   cout << "inside update_neural_network()" << endl;
//   cout << "episode_number = " << get_episode_number() << endl;

// Nd = Number of random samples to be drawn from replay memory:

   int Nd = 0.1 * replay_memory_capacity; 
   Nd = basic_math::min(Nd, 32);

   vector<int> d_samples = mathfunc::random_sequence(
      replay_memory_capacity, Nd);
   double total_loss = 0;
   for(unsigned int j = 0; j < d_samples.size(); j++)
   {
      int curr_d = d_samples[j];
      double curr_loss = Q_backward_propagate(curr_d, Nd);
      if(curr_loss >= 0) total_loss += curr_loss;
   } // loop over index j labeling replay memory samples
//   cout << "total_loss = " << total_loss << endl;

   if(solver_type == RMSPROP)
   {
      update_biases_cache(rmsprop_decay_rate);
      update_rmsprop_cache(rmsprop_decay_rate);
   }
   else if (solver_type == ADAM)
   {
      update_biases_cache(beta2);
      update_rmsprop_cache(beta2);
   }
   
// Update weights and biases for each network layer by their nabla
// values averaged over the current mini-batch:

   for(int l = 0; l < n_layers; l++)
   {
      if(solver_type == SGD)
      {
         *biases[l] -= learning_rate * (*nabla_biases[l]);
      }
      else if (solver_type == RMSPROP)
      {
         rms_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
         const double TINY = 1E-5;
         rms_biases_denom[l]->hadamard_sum(TINY);
         nabla_biases[l]->hadamard_ratio(*rms_biases_denom[l]);
         *biases[l] -= learning_rate * (*nabla_biases[l]);
      }
      else if (solver_type == ADAM)
      {
      }
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
   }

   for(int l = 0; l < n_layers - 1; l++)
   {
      if (solver_type == SGD)
      {
         *weights[l] -= learning_rate * (*nabla_weights[l]);
      }
      else if (solver_type == MOMENTUM)
      {
         *velocities[l] = mu * (*velocities[l])
            - learning_rate * (*nabla_weights[l]);
         *weights[l] += *velocities[l];
      }
      else if (solver_type == NESTEROV)
      {
         *prev_velocities[l] = *velocities[l];
         *velocities[l] = mu * (*velocities[l])
            - learning_rate * (*nabla_weights[l]);
         *weights[l] += -mu * (*prev_velocities[l]) +
            (1 + mu) * (*velocities[l]);
      }
      else if(solver_type == RMSPROP)
      {
         rms_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         const double TINY = 1E-5;
         rms_weights_denom[l]->hadamard_sum(TINY);
         nabla_weights[l]->hadamard_division(*rms_weights_denom[l]);
         *weights[l] -= learning_rate * (*nabla_weights[l]);
      }
      else if(solver_type == ADAM)
      {
         *adam_m[l] = beta1 * (*adam_m[l]) + (1 - beta1) * (*nabla_weights[l]);
         curr_beta1_pow *= beta1;
         *adam_m[l] /= (1 - curr_beta1_pow);

         curr_beta2_pow *= beta2;
         *rmsprop_weights_cache[l] /= (1 - curr_beta2_pow);
         
         rms_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         const double TINY = 1E-8;
         rms_weights_denom[l]->hadamard_sum(TINY);
         adam_m[l]->hadamard_division(*rms_weights_denom[l]);
         *weights[l] -= learning_rate * (*adam_m[l]);
      }
      
//      debug_flag = true;
      if(debug_flag)
      {
         int mdim = nabla_weights[l]->get_mdim();
         int ndim = nabla_weights[l]->get_ndim();
         
         vector<double> curr_nabla_weights;
         vector<double> curr_nabla_weight_ratios;
         vector<double> curr_adam_ms;
         
         for(int r = 0; r < mdim; r++)
         {
            for(int c = 0; c < ndim; c++)
            {
               curr_nabla_weights.push_back(
                  fabs(nabla_weights[l]->get(c,r)));
               curr_adam_ms.push_back(fabs(adam_m[l]->get(c,r)));
//               cout << "r = " << r << " c = " << c
//                    << " curr_nabla_weight = " 
//                    << curr_nabla_weights.back() << endl;
               double denom = weights[l]->get(c,r);
               if(fabs(denom) > 1E-10)
               {
                  curr_nabla_weight_ratios.push_back(
                     fabs(nabla_weights[l]->get(c,r) / denom ));
               }
            }
         }
         double mean_abs_nabla_weight = mathfunc::mean(curr_nabla_weights);
         double mean_abs_nabla_weight_ratio = mathfunc::mean(
            curr_nabla_weight_ratios);
         double mean_abs_adam_m = mathfunc::mean(curr_adam_ms);
            
         cout << "layer l = " << l
              << " mean |nabla weight| = " 
              << mean_abs_nabla_weight 
              << " mean |nabla weight| ratio = " 
              << mean_abs_nabla_weight_ratio  << endl;
         
         cout << " lr * mean_abs_nabla_weight = " 
              << learning_rate * mean_abs_nabla_weight 
              << "  lr * mean_abs_nabla_weight_ratio = " 
              << learning_rate * mean_abs_nabla_weight_ratio << endl;

         cout << " mean_abs_adam_m = " << mean_abs_adam_m
              << " lr * mean_abs_adam_m = " << learning_rate * mean_abs_adam_m
              << endl;
         
      } // debug_flag conditional
      
      nabla_biases[l]->clear_values();
      nabla_weights[l]->clear_values();
   }
//   cout << endl;
//   print_weights();
//   outputfunc::enter_continue_char();

   return total_loss;
}

// ---------------------------------------------------------------------
// Member function Q_backward_propagate()

double reinforce::Q_backward_propagate(int d, int Nd)
{
   int t = 0;

   // Initialize "batch" weight gradients to zero:

   for(int l = 0; l < n_layers; l++)
   {
      delta_nabla_biases[l]->clear_values();
   }

   for(int l = 0; l < n_layers - 1; l++)
   {
      delta_nabla_weights[l]->clear_values();
   }

// Calculate target for curr transition sample:

   int curr_a;
   double curr_r;
   bool terminal_state_flag = get_memory_replay_entry(
      d, *curr_s_sample, curr_a, curr_r, *next_s_sample);
   double target_value = 
      compute_target(curr_r, next_s_sample, terminal_state_flag);

// First need to perform forward propagation for *curr_s_sample in
// order to repopulate linear z inputs and nonlinear a outputs for
// each node in the neural network:

   Q_forward_propagate(curr_s_sample);

   int curr_layer = n_layers - 1;

// Eqn BP1:

   double curr_loss = -1;
   for(int j = 0; j < layer_dims[curr_layer]; j++)
   {
      double curr_Q = a[curr_layer]->get(j, t);

      double curr_activation = 0;
      if(j == curr_a)
      {
         curr_activation = curr_Q - target_value;
         curr_loss = 0.5 * sqr(curr_activation);
      }

      delta_prime[curr_layer]->put(j, t, curr_activation);
   }

   for(curr_layer = n_layers-1; curr_layer >= 1; curr_layer--)
   {
      int prev_layer = curr_layer - 1;

// Eqn BP2A:

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

      weights_transpose[prev_layer]->matrix_transpose(*weights[prev_layer]);
      delta_prime[prev_layer]->matrix_mult(
         *weights_transpose[prev_layer], *delta_prime[curr_layer]);

// Eqn BP2B (ReLU):
      for(int j = 0; j < layer_dims[prev_layer]; j++)
      {
         if(z[prev_layer]->get(j, t) < 0)
         {
            delta_prime[prev_layer]->put(j, t, 0);
         }
      } // loop over j index

// Accumulate bias & weight gradients over batch of episodes:

// Eqn BP3:

      delta_nabla_biases[curr_layer]->copy_matrix_column(
         *delta_prime[curr_layer], t);

// Eqn BP4:

      delta_nabla_weights[prev_layer]->accumulate_outerprod(
         delta_prime[curr_layer]->get_column(t), a[prev_layer]->get_column(t));

   } // loop over curr_layer index

// Accumulate biases' and weights' gradients for each network layer:

   double inverse_Nd = 1.0 / Nd;
   for(int l = 0; l < n_layers; l++)
   {
      nabla_biases[l]->vector_increment(inverse_Nd, *delta_nabla_biases[l]);
   }
   for(int l = 0; l < n_layers - 1; l++)
   {
      nabla_weights[l]->matrix_increment(inverse_Nd, *delta_nabla_weights[l]);
   }

   curr_loss *= inverse_Nd;
   return curr_loss;
}

// ---------------------------------------------------------------------
void reinforce::set_Q_value(string state_action_str, double Qvalue)
{
   qmap_iter = qmap.find(state_action_str);
   if(qmap_iter == qmap.end())
   {
      qmap[state_action_str] = Qvalue;
   }
   else
   {
      qmap_iter->second = Qvalue;
   }
}

// ---------------------------------------------------------------------
double reinforce::get_Q_value(string state_action_str)
{
   qmap_iter = qmap.find(state_action_str);
   if(qmap_iter == qmap.end())
   {
      return NEGATIVEINFINITY;
   }
   else
   {
      return qmap_iter->second;
   }
}

// ---------------------------------------------------------------------
// Member function init_random_Qmap initializes matrix Q(s,a) with
// random values ranging over interval [-1,1].

void reinforce::init_random_Qmap()
{
   vector<string> curr_state_strings = 
      environment_ptr->get_all_curr_state_strings();
   for(unsigned int s = 0; s < curr_state_strings.size(); s++)
   {
      for(int a = 0; a < n_actions; a++)
      {
         string curr_state_action_str = curr_state_strings[s] + 
            stringfunc::number_to_string(a);
         double Qval = 2 * (nrfunc::ran1() - 0.5);
         set_Q_value(curr_state_action_str, Qval);
      } // loop over index a labeling actions
   } // loop over index s labeling state strings
}

// ---------------------------------------------------------------------
// Member function print_Qmap 

void reinforce::print_Qmap()
{
   for(qmap_iter = qmap.begin(); qmap_iter != qmap.end(); qmap_iter++)
   {
      cout << qmap_iter->first << "  " << qmap_iter->second << endl;
   }
}

// ==========================================================================
// Value function learning methods
// ==========================================================================

void reinforce::set_n_actions(int n)
{
   n_actions = n;
}

// ---------------------------------------------------------------------
// Member function compute_value() performs a forward-pass on the
// input state.  It returns the scalar value output from the neural
// network.

double reinforce::compute_value(
   genvector* curr_afterstate, bool use_old_weights_flag)
{
   Q_forward_propagate(curr_afterstate, use_old_weights_flag);
   return a[n_layers-1]->get(0,0);
}

// ---------------------------------------------------------------------
// Member function V_forward_propagate_afterstates() performs a
// feedforward pass for all legal afterstates to the current state.
// It returns the index for the action which yields the best
// afterstate value Vstar, The best afterstate is also saved within
// member *prev_afterstate_ptr.

int reinforce::V_forward_propagate_afterstates(
   int player_value, double& Vstar)
{
   vector<genvector*>* afterstate_ptrs = environment_ptr->get_all_afterstates(
      player_value);

   Vstar = POSITIVEINFINITY;
   if(player_value > 0)
   {
      Vstar = NEGATIVEINFINITY;
   }

   int best_a = -1;
   for(unsigned int curr_a = 0; curr_a < afterstate_ptrs->size(); curr_a++)
   {
      genvector* curr_afterstate = afterstate_ptrs->at(curr_a);

// Recall illegal afterstates are signaled by -999 sentinel values in
// all coordinates of their genvectors:

      if(nearly_equal(curr_afterstate->get(0), -999)) continue;

      double curr_activation = compute_value(curr_afterstate);
      if(player_value > 0 && curr_activation > Vstar)
      {
         Vstar = curr_activation;
         best_a = curr_a;
      }
      else if(player_value < 0 && curr_activation < Vstar)
      {
         Vstar = curr_activation;
         best_a = curr_a;
      }
   } // loop over index curr_a labeling actions which lead to afterstates

   *prev_afterstate_ptr = *afterstate_ptrs->at(best_a);
   return best_a;
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// select_legal_action_for_curr_state() implements epsilon-greedy
// exploration.  For epsilon percentage of calls to this method, some
// random, legal afterstate is selected and its V-value is evaluated
// via the neural network.  Otherwise, the V-values for all
// afterstates are evaluated via the network.  The one with the
// extremal value consistent within input player_value is returned
// within output Vstar along with the index for the action which
// yields the afterstate.

int reinforce::select_legal_action_for_curr_state(
   int player_value, double& Vstar)
{
   vector<genvector*>* afterstate_ptrs = 
      environment_ptr->get_all_afterstates(player_value);

   int curr_a = -1;
   if(nrfunc::ran1() < epsilon)
   {
      curr_a = get_random_legal_action();
      *prev_afterstate_ptr = *afterstate_ptrs->at(curr_a);
      Vstar = compute_value(prev_afterstate_ptr);
   }
   else
   {
      curr_a = V_forward_propagate_afterstates(player_value, Vstar);
   }

   return curr_a;
}

// ---------------------------------------------------------------------
double reinforce::get_prev_afterstate_curr_value()
{
   return compute_value(prev_afterstate_ptr);
}
