// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/19/16; 10/20/16; 10/22/16; 10/24/16
// ==========================================================================

// random comment

#include <string>
#include "filter/filterfuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "plot/metafile.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "machine_learning/reinforce.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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
   cum_time_counter = 0;

// This particular set of hyperparameters yields perfect reinforcement
// learning for an agent not to place any of its pieces into already
// occupied cells within a 4x4 grid !

   batch_size = 10;	// Perform parameter update after this many episodes
//   batch_size = 5;	// Perform parameter update after this many episodes
//   learning_rate = 3E-3;  // Seems worse than 1E-4
//   learning_rate = 1E-3;  // Terrible for 2 z-levels

//   learning_rate = 5.196E-4;  
//   learning_rate = 3E-4;  // Much better than 1E-4 for 1 z-level!!!
//   learning_rate = 1.732E-4;  
   learning_rate = 1E-4;  //
   lambda = 0.0;	// L2 regularization coefficient (better than 1E-3)
//   lambda = 0.001;	// L2 regularization coefficient
//   gamma = 0.99;	// Discount factor for reward
   gamma = 0.5;	// Discount factor for reward
//   rmsprop_decay_rate = 0.75;
//   rmsprop_decay_rate = 0.8;
   rmsprop_decay_rate = 0.85;
//   rmsprop_decay_rate = 0.9;
   // rmsprop_decay_rate = 0.95; 
//   rmsprop_decay_rate = 0.99; 
   
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

   p_action = new genvector(n_actions);
   pcum_action = new genvector(n_actions);

// Weights link layer l with layer l+1:
    
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      weights.push_back(curr_weights);
      genmatrix *curr_weights_transpose = new genmatrix(
         layer_dims[l], layer_dims[l+1]);
      weights_transpose.push_back(curr_weights_transpose);

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
      genmatrix *curr_rms_denom = 
         new genmatrix(layer_dims[l+1], layer_dims[l]);
      rms_denom.push_back(curr_rms_denom);

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

   for(unsigned int l = 0; l < weights.size(); l++)
   {
      delete weights[l];
      delete weights_transpose[l];
      delete nabla_weights[l];
      delete delta_nabla_weights[l];
      delete rmsprop_weights_cache[l];
      delete rms_denom[l];
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

   for(int l = 0; l < n_layers-2; l++)
   {
      z[l+1]->matrix_column_mult(*weights[l], *a[l], t);
      machinelearning_func::ReLU(t, *z[l+1], *a[l+1]);
   }

   z[n_layers-1]->matrix_column_mult(*weights[n_layers-2], *a[n_layers-2], t);
   machinelearning_func::softmax(t, *z[n_layers-1], *a[n_layers-1]);
}

// ---------------------------------------------------------------------
void reinforce::get_softmax_action_probs(int t) const
{
   *p_action = a[n_layers-1]->get_column(t);
}

// ---------------------------------------------------------------------
double reinforce::compute_loss(int t) const
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
//      cout << "t = " << t << " discounted_reward = "
//           << running_add << endl;
   }

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

      weights_transpose[prev_layer]->matrix_transpose(*weights[prev_layer]);
      delta_prime[prev_layer]->matrix_mult(
         *weights_transpose[prev_layer], *delta_prime[curr_layer]);
//         weights[prev_layer]->transpose(), *delta_prime[curr_layer]);

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

         delta_nabla_weights[prev_layer]->accumulate_outerprod(
            delta_prime[curr_layer]->get_column(t),
            a[prev_layer]->get_column(t));

//         *delta_nabla_weights[prev_layer] += 
//            delta_prime[curr_layer]->get_column(t).outerproduct(
//               a[prev_layer]->get_column(t));

         const double TINY = 1E-8;
         if(lambda > TINY)
         {
            *delta_nabla_weights[prev_layer] += 
               2 * lambda * (*weights[prev_layer]);
         }
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

   if(cum_time_counter > 0 && cum_time_counter%20000 == 0)
   {
      time_samples.push_back(cum_time_counter);
      loss_values.push_back(compute_loss(curr_timestep));
      running_reward_snapshots.push_back(running_reward);
   }
   return a_star;
}

// ---------------------------------------------------------------------
void reinforce::record_reward_for_action(double curr_reward)
{
   reward_sum += curr_reward;
   reward->put(curr_timestep, curr_reward);
//   cout << "curr_timestep = " << curr_timestep 
//        << " curr_reward = " << curr_reward << endl;

   cum_time_counter++;
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
//         *rmsprop_weights_cache[l] = 
//            rmsprop_decay_rate * (*rmsprop_weights_cache[l])
//            + (1 - rmsprop_decay_rate) * nabla_weights[l]->hadamard_power(2);

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

// Update weights and biases for eacy network layer by their nabla
// values averaged over the current mini-batch:

      const double epsilon = 1E-5;
      for(int l = 0; l < n_layers - 1; l++)
      {
         rms_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         rms_denom[l]->hadamard_sum(epsilon);
         nabla_weights[l]->hadamard_division(*rms_denom[l]);

         *weights[l] -= learning_rate * (*nabla_weights[l]);
         nabla_weights[l]->clear_values();
      }
//       print_weights();
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
   if(episode_number > 0 && episode_number % 10000 == 0) print_flag = true;
   if(print_flag)
   {
      double mu_T, sigma_T;
      mathfunc::mean_and_std_dev(T_values, mu_T, sigma_T);
      cout << "learning rate="+stringfunc::number_to_string(learning_rate,5)
           << " gamma="+stringfunc::number_to_string(gamma,3)
           << " rms_decay="+stringfunc::number_to_string(rmsprop_decay_rate,3)
           << endl;
      cout << "batch_size = " << batch_size << endl;
      cout << extrainfo << endl;
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
      "learning rate="+stringfunc::number_to_string(learning_rate,5)+
      "; gamma="+stringfunc::number_to_string(gamma,3)+
      "; rms_decay="+stringfunc::number_to_string(rmsprop_decay_rate,3);
   return subtitle;
}

// ---------------------------------------------------------------------
// Generate metafile plot of loss values versus time step samples.

void reinforce::plot_loss_history(std::string extrainfo)
{

// Temporally smooth noisy loss values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx);
   vector<double> h;
   h.reserve(gaussian_size);
   filterfunc::gaussian_filter(dx, sigma, h);

   bool wrap_around_input_values = false;
   vector<double> smoothed_loss_values;
   filterfunc::brute_force_filter(
      loss_values, h, smoothed_loss_values, wrap_around_input_values);

   metafile curr_metafile;
   string meta_filename="loss_history";
   string title="Loss vs RMSprop model training; bsize="+
      stringfunc::number_to_string(batch_size);
   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label="Time step";
   string y_label="Loss";

   double min_loss, max_loss;
   mathfunc::lo_hi_values(
      loss_values, 0.025, 0.975, min_loss, max_loss);
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

   curr_metafile.set_thickness(3);
   curr_metafile.write_curve(time_samples, smoothed_loss_values, 
                             colorfunc::blue);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of running reward sum versus time step samples.

void reinforce::plot_reward_history(std::string extrainfo)
{

// Temporally smooth noisy loss values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx);
   vector<double> h;
   h.reserve(gaussian_size);
   filterfunc::gaussian_filter(dx, sigma, h);

   bool wrap_around_input_values = false;
   vector<double> smoothed_reward_snapshots;
   filterfunc::brute_force_filter(
      running_reward_snapshots, h, smoothed_reward_snapshots, 
      wrap_around_input_values);

   metafile curr_metafile;
   string meta_filename="reward_history";
   string title="Running reward sum vs RMSprop model training; bsize="+
      stringfunc::number_to_string(batch_size);
   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label="Time step";
   string y_label="Running reward sum";

   double min_reward = -1;
   double max_reward = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, time_samples.back(),
      min_reward, max_reward);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(time_samples, running_reward_snapshots);
   curr_metafile.set_thickness(3);
   curr_metafile.write_curve(
      time_samples, smoothed_reward_snapshots, colorfunc::blue);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of total number of turns versus episode number.

void reinforce::plot_turns_history(std::string extrainfo)
{

// Temporally smooth noisy turns fraction values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx);
   vector<double> h;
   h.reserve(gaussian_size);
   filterfunc::gaussian_filter(dx, sigma, h);

   bool wrap_around_input_values = false;
   vector<double> smoothed_n_episode_turns_frac;
   filterfunc::brute_force_filter(
      n_episode_turns_frac, h, smoothed_n_episode_turns_frac, 
      wrap_around_input_values);

   metafile curr_metafile;
   string meta_filename="turns_history";
   string title="Number of AI and agent turns vs episode; bsize="+
      stringfunc::number_to_string(batch_size);
   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label="Episode";
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
   curr_metafile.write_curve(
      0, episode_number, smoothed_n_episode_turns_frac, colorfunc::blue);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

