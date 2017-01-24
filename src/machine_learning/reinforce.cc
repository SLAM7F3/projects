// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 1/17/17; 1/18/17; 1/23/17; 1/24/17
// ==========================================================================

#include <string>
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
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
#include "time/timefuncs.h"
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
   expt_number = -1;
   perm_symmetrize_weights_and_biases = false;
   debug_flag = false;
   batch_size = 1;	// Perform parameter update after this many episodes
   mu = 0.9;		// Coefficient for momentum solver type
   lambda = 0.0;	// L2 regularization coefficient 
   gamma = 0.5;	// Discount factor for reward
   rmsprop_decay_rate = 0.85;
   rmsprop_denom_const = 1E-5;
   episode_number = 0;
   curr_epoch = 0.0;

   n_layers = n_nodes_per_layer.size();
   for(int l = 0; l < n_layers; l++)
   {
      layer_dims.push_back(n_nodes_per_layer[l]);
   }
   n_actions = layer_dims.back();

   hardwired_output_action = -1;
   n_backprops = 0;

// Only instantiate genmatrices which are needed depending upon
// selected solver type:
    
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_velocity = NULL;
      if(solver_type == MOMENTUM || solver_type == NESTEROV)
      {
         curr_velocity = new genmatrix(
            layer_dims[l+1], layer_dims[l]);  // used for momentum solver type
      }
      genmatrix *prev_velocity = NULL;
      if(solver_type == NESTEROV)
      {
         prev_velocity = new genmatrix(
            layer_dims[l+1], layer_dims[l]);  // used for Nesterov solver type
      }
      genmatrix *curr_adam_m = NULL;
      genmatrix *curr_adam_v = NULL;
      if(solver_type == ADAM)
      {
         curr_adam_m = new genmatrix(
            layer_dims[l+1], layer_dims[l]);  // used for ADAM solver type
         curr_adam_v = new genmatrix(
            layer_dims[l+1], layer_dims[l]);  // used for ADAM solver type
      }

      int mdim = layer_dims[l+1];
      int ndim = layer_dims[l];
      cout << "l = " << l << " mdim = " << mdim << " ndim = " << ndim
           << endl;
      
      if(curr_velocity != NULL)
      {
         curr_velocity->clear_values();
         velocities.push_back(curr_velocity);
      }
      if(prev_velocity != NULL)
      {
         prev_velocity->clear_values();
         prev_velocities.push_back(prev_velocity);
      }
      if(curr_adam_m != NULL)
      {
         curr_adam_m->clear_values();
         adam_m.push_back(curr_adam_m);
         curr_adam_v->clear_values();
         adam_v.push_back(curr_adam_v);
      }
   } // loop over index l labeling neural net layers

   snapshots_subdir="";

// Q learning variable initialization:

   environment_ptr = NULL;
   replay_memory_full_flag = false;
   eval_memory_full_flag = false;
   eval_memory_index = 0;
   replay_memory_index = 0;
   epsilon = 1.000001;
   epsilon_tau = -1;

   beta1 = beta2 = 0;
   curr_beta1_pow = curr_beta2_pow = 1;

// P learning variable initialization:

   max_mean_KL_divergence = 1E-4;
}

// ---------------------------------------------------------------------
void reinforce::instantiate_weights_and_biases()
{
   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         genvector *curr_biases = new genvector(layer_dims[l]);
         biases.push_back(curr_biases);
         genvector *curr_old_biases = new genvector(layer_dims[l]);
         old_biases.push_back(curr_old_biases);
      }  
   } // include_biases conditional

// Weights link layer l with layer l+1:
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = new genmatrix(layer_dims[l+1], layer_dims[l]);
      weights.push_back(curr_weights);
      genmatrix *curr_old_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      old_weights.push_back(curr_old_weights);
   } // loop over index l labeling neural net layers
   n_weights = count_weights();
}

// ---------------------------------------------------------------------
void reinforce::instantiate_training_variables()
{
   vector<double> dummy_dist;

   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         nabla_biases.push_back(new genvector(layer_dims[l]));
         delta_nabla_biases.push_back(new genvector(layer_dims[l]));
         if(perm_symmetrize_weights_and_biases)
         {
            permuted_biases.push_back(new genvector(layer_dims[l]));
            sym_biases.push_back(new genvector(layer_dims[l]));
         }

         if(solver_type == RMSPROP)
         {
            genvector *curr_rmsprop_biases = new genvector(layer_dims[l]);
            curr_rmsprop_biases->clear_values();
            rmsprop_biases_cache.push_back(curr_rmsprop_biases);
            
            genvector *curr_rmsprop_biases_denom = 
               new genvector(layer_dims[l]);
            curr_rmsprop_biases_denom->clear_values();
            rmsprop_biases_denom.push_back(curr_rmsprop_biases_denom);
         }

         bias_01.push_back(dummy_dist);
         bias_05.push_back(dummy_dist);
         bias_10.push_back(dummy_dist);
         bias_25.push_back(dummy_dist);
         bias_35.push_back(dummy_dist);
         bias_50.push_back(dummy_dist);
         bias_65.push_back(dummy_dist);
         bias_75.push_back(dummy_dist);
         bias_90.push_back(dummy_dist);
         bias_95.push_back(dummy_dist);
         bias_99.push_back(dummy_dist);
      } // loop over index l labeling layers
   } // include_bias_terms conditional

// Weights link layer l with layer l+1:
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      weights_transpose.push_back(
         new genmatrix(layer_dims[l], layer_dims[l+1]));

      if(perm_symmetrize_weights_and_biases)
      {
         permuted_weights.push_back(
            new genmatrix(layer_dims[l+1], layer_dims[l]));
         sym_weights.push_back(
            new genmatrix(layer_dims[l+1], layer_dims[l]));
      }

      nabla_weights.push_back(
         new genmatrix(layer_dims[l+1], layer_dims[l]));
      delta_nabla_weights.push_back(
         new genmatrix(layer_dims[l+1], layer_dims[l]));

      if(solver_type == RMSPROP)
      {
         genmatrix *curr_rmsprop_weights = new genmatrix(
            layer_dims[l+1], layer_dims[l]);
         curr_rmsprop_weights->clear_values();
         rmsprop_weights_cache.push_back(curr_rmsprop_weights);
         
         genmatrix *curr_rmsprop_weights_denom = 
            new genmatrix(layer_dims[l+1], layer_dims[l]);
         curr_rmsprop_weights_denom->clear_values();
         rmsprop_weights_denom.push_back(curr_rmsprop_weights_denom);
      }

//      log10_lr_mean_abs_nabla_weight_ratios.push_back(dummy_dist);

      weight_01.push_back(dummy_dist);
      weight_05.push_back(dummy_dist);
      weight_10.push_back(dummy_dist);
      weight_25.push_back(dummy_dist);
      weight_35.push_back(dummy_dist);
      weight_50.push_back(dummy_dist);
      weight_65.push_back(dummy_dist);
      weight_75.push_back(dummy_dist);
      weight_90.push_back(dummy_dist);
      weight_95.push_back(dummy_dist);
      weight_99.push_back(dummy_dist);

      weight_1.push_back(dummy_dist);
      weight_2.push_back(dummy_dist);
      weight_3.push_back(dummy_dist);
      weight_4.push_back(dummy_dist);
      weight_5.push_back(dummy_dist);
      weight_6.push_back(dummy_dist);
      weight_7.push_back(dummy_dist);
      weight_8.push_back(dummy_dist);
      weight_9.push_back(dummy_dist);
   } // loop over index l labeling neural net layers
}

// ---------------------------------------------------------------------
void reinforce::initialize_weights_and_biases()
{
   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         genvector *curr_biases = biases[l];
         genvector *curr_old_biases = old_biases[l];

// Initialize bias for each network node in layers 1, 2, ... to be
// gaussian random var distributed according to N(0,1).  Recall input
// layer has no biases:

         for(int i = 0; i < layer_dims[l]; i++)
         {
            if(l == 0)
            {
               curr_biases->put(i, 0);
            }
            else
            {

// On 1/24/17, we empirically observed that initializing ALL biases to
// zero appears to yield noticeably better PMAZE results (for leaky
// ReLU nonlinearities) than initializing biases either by
// nrfunc::ran1() or nrfunc::gasdev() !!!

               curr_biases->put(i, 0);
//               curr_biases->put(i, nrfunc::ran1());
//               curr_biases->put(i, nrfunc::gasdev());
            }
            curr_old_biases->put(i, curr_biases->get(i));
         } // loop over index i labeling node in current layer

         if(perm_symmetrize_weights_and_biases)
         {
//            environment_ptr->permutation_symmetrize_biases(
//               curr_biases, permuted_biases[l], sym_biases[l]);
         }
      }  // loop over index l labeling layers
   } // include_bias_terms conditional

   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = weights[l];
      genmatrix *curr_old_weights = old_weights[l];

// Xavier initialize weights connecting network layers l and l+1 to be
// gaussian random vars distributed according to N(0,1/sqrt(n_in)):

      for(int i = 0; i < layer_dims[l+1]; i++)
      {
         for(int j = 0; j < layer_dims[l]; j++)
         {
            curr_weights->put(
               i, j, sqrt(2.0) * nrfunc::gasdev() / sqrt(layer_dims[l]) );
            curr_old_weights->put(i, j, curr_weights->get(i, j));
         } // loop over index j labeling node in next layer
      } // loop over index i labeling node in current layer

      if(perm_symmetrize_weights_and_biases)
      {
//         environment_ptr->permutation_symmetrize_weights(
//            curr_weights, permuted_weights[l], sym_weights[l]);
      }
   } // loop over index l labeling neural net layers
}

// ---------------------------------------------------------------------
void reinforce::allocate_member_objects()
{
   for(int l = 0; l < n_layers; l++)
   {
      genvector *curr_Z_Prime = new genvector(layer_dims[l]);
      genvector *curr_gamma = new genvector(layer_dims[l]);
      genvector *curr_beta = new genvector(layer_dims[l]);
      genvector *curr_A_Prime = new genvector(layer_dims[l]);
      genvector *curr_Delta_Prime = new genvector(layer_dims[l]);
      Z_Prime.push_back(curr_Z_Prime);
      gammas.push_back(curr_gamma);
      betas.push_back(curr_beta);
      A_Prime.push_back(curr_A_Prime);
      Delta_Prime.push_back(curr_Delta_Prime);
   } // loop over index l labeling neural network layers

   s_curr = new genmatrix(replay_memory_capacity, layer_dims.front());
   a_curr = new genvector(replay_memory_capacity);
   r_curr = new genvector(replay_memory_capacity);
   terminal_state = new genvector(replay_memory_capacity);
   curr_s_sample = new genvector(layer_dims.front());

   s_next = NULL;
   s_eval = NULL;
   next_s_sample = NULL;
   pi_curr = NULL;
   curr_pi_sample = NULL;
   next_pi_sample = NULL;
   prev_afterstate_ptr = NULL;

   if (learning_type == QLEARNING)
   {
      s_eval = new genmatrix(eval_memory_capacity, layer_dims.front());
      next_s_sample = new genvector(layer_dims.front());
      s_next = new genmatrix(replay_memory_capacity, layer_dims.front());
   }
   else if(learning_type == PLEARNING)
   {
      pi_curr = new genmatrix(replay_memory_capacity, layer_dims.back());
      curr_pi_sample = new genvector(layer_dims.back());
      next_pi_sample = new genvector(layer_dims.back());
   }
   else if (learning_type == VLEARNING)
   {
      prev_afterstate_ptr = new genvector(layer_dims.front());
   }
}		       

// ---------------------------------------------------------------------
reinforce::reinforce(bool include_biases, const vector<int>& n_nodes_per_layer)
{
   this->include_biases = include_biases;
   this->replay_memory_capacity = 1;
   this->eval_memory_capacity = 1;
   initialize_member_objects(n_nodes_per_layer);
   instantiate_weights_and_biases();
   instantiate_training_variables();
   initialize_weights_and_biases();
   allocate_member_objects();

   machinelearning_func::set_leaky_ReLU_small_slope(0.01);
}

// ---------------------------------------------------------------------
// Reinforce class constructor

reinforce::reinforce(
   bool include_biases, const vector<int>& n_nodes_per_layer, 
   int replay_memory_capacity, int solver_type)
{
   this->include_biases = include_biases;
   this->replay_memory_capacity = replay_memory_capacity;
   this->eval_memory_capacity = -1;
   this->solver_type = solver_type;
   this->learning_type = PLEARNING;

   initialize_member_objects(n_nodes_per_layer);
   instantiate_weights_and_biases();
   instantiate_training_variables();
   initialize_weights_and_biases();
   allocate_member_objects();

   this->batch_size = -1;
   machinelearning_func::set_leaky_ReLU_small_slope(0.01);
}

// ---------------------------------------------------------------------
// Reinforce class constructor

reinforce::reinforce(
   bool include_biases, const vector<int>& n_nodes_per_layer, 
   int batch_size, int replay_memory_capacity,
   int eval_memory_capacity, int solver_type)
{
   this->include_biases = include_biases;
   this->replay_memory_capacity = replay_memory_capacity;
   this->eval_memory_capacity = eval_memory_capacity;
   this->solver_type = solver_type;
   this->learning_type = QLEARNING;

   initialize_member_objects(n_nodes_per_layer);
   instantiate_weights_and_biases();
   instantiate_training_variables();
   initialize_weights_and_biases();
   allocate_member_objects();

   this->batch_size = batch_size;
   machinelearning_func::set_leaky_ReLU_small_slope(0.01);
}

// ---------------------------------------------------------------------
// Reinforce class constructor

reinforce::reinforce(string snapshot_filename)
{
   import_snapshot(snapshot_filename);
   allocate_member_objects();

//   extrainfo = layer_label = "";
   machinelearning_func::set_leaky_ReLU_small_slope(0.01);
}

// ---------------------------------------------------------------------
void reinforce::delete_weights_and_biases()
{
   if(include_biases)
   {
      for(unsigned int l = 0; l < biases.size(); l++)
      {
         delete biases[l];
         delete old_biases[l];
      }
   } 
   biases.clear();
   old_biases.clear();

   for(unsigned int l = 0; l < weights.size(); l++)
   {
      delete weights[l];
      delete old_weights[l];
   }
   weights.clear();
   old_weights.clear();
}

// ---------------------------------------------------------------------
reinforce::~reinforce()
{
   delete_weights_and_biases();

   for(int l = 0; l < n_layers; l++)
   {
      delete Z_Prime[l];
      delete gammas[l];
      delete betas[l];
      delete A_Prime[l];
      delete Delta_Prime[l];
   }

   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         delete nabla_biases[l];
         delete delta_nabla_biases[l];

         if(perm_symmetrize_weights_and_biases)
         {
            delete permuted_biases[l];
            delete sym_biases[l];
         }
         if(solver_type == RMSPROP)
         {
            delete rmsprop_biases_cache[l];
            delete rmsprop_biases_denom[l];
         }
      }
   } // include_biases flag

   for(int l = 0; l < n_layers - 1; l++)
   {
      delete weights_transpose[l];
      delete nabla_weights[l];
      delete delta_nabla_weights[l];

      if(perm_symmetrize_weights_and_biases)
      {
         delete permuted_weights[l];
         delete sym_weights[l];
      }
      if(velocities.size() > 0)
      {
         delete velocities[l];
      }
      if(prev_velocities.size() > 0)
      {
         delete prev_velocities[l];
      }
      if(rmsprop_weights_cache.size() > 0)
      {
         delete rmsprop_weights_cache[l];
         delete rmsprop_weights_denom[l];
      }
      if(adam_m.size() > 0)
      {
         delete adam_m[l];
         delete adam_v[l];
      }
   }

   delete s_curr;
   delete s_eval;
   delete a_curr;
   delete r_curr;
   delete s_next;
   delete terminal_state;
   delete pi_curr;

   delete curr_s_sample;
   delete next_s_sample;
   delete curr_pi_sample;
   delete next_pi_sample;
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
void reinforce::initialize_episode()
{
   for(int l = 0; l < n_layers; l++)
   {
      Z_Prime[l]->clear_values();
      A_Prime[l]->clear_values();
      Delta_Prime[l]->clear_values();
   }
}

// ---------------------------------------------------------------------
void reinforce::update_cumulative_reward(double cum_reward)
{
   cumulative_reward_snapshots.push_back(cum_reward);
}

void reinforce::update_n_frames_per_episode(int n_frames)
{
   n_frames_per_episode.push_back(n_frames);
}

void reinforce::update_episode_history()
{
   episode_history.push_back(get_episode_number());
}

void reinforce::update_epoch_history()
{
   epoch_history.push_back(get_curr_epoch());
}

void reinforce::update_epsilon()
{
   epsilon_values.push_back(epsilon);
}

// ==========================================================================
// Monitoring network training methods
// ==========================================================================

// Member function summarize_parameters() exports most parameters and
// hyperparameters used for Deep Reinforcement Learning to a specified
// text file for book-keeping purposes.

void reinforce::summarize_parameters()
{
   params_filename = output_subdir + "params.dat";
   ofstream params_stream;
   filefunc::openfile(params_filename, params_stream);

   params_stream << "Experiment " << expt_number << endl;
   params_stream << timefunc::getcurrdate() << endl;
   params_stream << "output_subdir = " << output_subdir << endl;
   params_stream << "Neural net params:" << endl;
   params_stream << "   n_layers = " << n_layers << endl;
   for(int l = 0; l < n_layers; l++)
   {
      params_stream << "   layer = " << l << " n_nodes = " 
                    << layer_dims[l] << endl;
   }
   params_stream << "   n_weights = " << n_weights << " (FC)" << endl;
   params_stream << "include_biases = " << include_biases << endl;

   params_stream << "base_learning_rate = " << base_learning_rate << endl;
   params_stream << "batch_size = " << batch_size << endl;
   params_stream << "perm_symmetrize_weights_and_biases = " 
                 << perm_symmetrize_weights_and_biases << endl;

   if(learning_type == QLEARNING)
   {
      params_stream << "Q learning" << endl;
   }
   else if(learning_type == PLEARNING)
   {
      params_stream << "P learning" << endl;
      params_stream << "max mean KL divergence between pi(curr) and pi(next) = " 
                    << max_mean_KL_divergence << endl;
      if(epsilon_tau > 0)
      {
         params_stream << "Epsilon time constant = " << epsilon_tau << endl;
      }
      params_stream << "minimum epsilon = " << min_epsilon << endl;
   }
   else if(learning_type == VLEARNING)
   {
      params_stream << "V learning" << endl;
   }

   if(solver_type == SGD)
   {
      params_stream << "solver type = SGD" << endl;
   }
   else if(solver_type == RMSPROP)
   {
      params_stream << "solver type = RMSPROP" << endl;
      params_stream << "   rmsprop_decay_rate = " << rmsprop_decay_rate
                    << endl;
      params_stream << "   rmsprop_denom_const = " << rmsprop_denom_const
                    << endl;
   }
   else if(solver_type == MOMENTUM)
   {
      params_stream << "solver type = MOMENTUM" << endl;
      params_stream << "  mu = " << mu << endl;
   }
   else if(solver_type == NESTEROV)
   {
      params_stream << "solver type = NESTEROV" << endl;
      params_stream << "   beta1 = " << beta1 << " beta2 = " << beta2 << endl;
   }
   else if(solver_type == ADAM)
   {
      params_stream << "solver type = ADAM" << endl;
   }
   params_stream << "L2 regularization lambda coeff = " << lambda << endl;
   params_stream << "Replay memory capacity = " << replay_memory_capacity
                 << endl;
   params_stream << "Number random samples drawn from replay memory Nd = "
                 << Nd << endl;
   params_stream << "Discount factor gamma = " << gamma << endl;
         
   filefunc::closefile(params_filename, params_stream);
}

// ---------------------------------------------------------------------
// Member function count_weights() sums up the total number of weights
// among all network layers assuming the network is fully connected.

int reinforce::count_weights()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      n_weights += layer_dims[l] * layer_dims[l+1];
   }
   return n_weights;
}

// ---------------------------------------------------------------------
void reinforce::print_biases()
{
   if(!include_biases) return;
   
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

void reinforce::plot_zeroth_layer_weights()
{
   int n_zeroth_layer_pixels = weights[0]->get_ndim();
   int ncols = sqrt(double(n_zeroth_layer_pixels));
   int nrows = ncols;
   plot_zeroth_layer_weights(ncols, nrows);
}

void reinforce::plot_zeroth_layer_weights(int ncols, int nrows)
{
//   cout << "inside reinforce::plot_zeroth_layer_weights()" << endl;
//   cout << "n_rows = " << nrows << " n_cols = " << ncols << endl;

   string weights_subdir = output_subdir+"zeroth_layer_weights/";
   filefunc::dircreate(weights_subdir);

   int n_zeroth_layer_weights = weights[0]->get_mdim();
   int n_zeroth_layer_pixels = weights[0]->get_ndim();
   
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

// Renormalize image weight values to range from 0 to 255:

   for(int n = 0; n < n_zeroth_layer_weights; n++)
   {
      twoDarray* wtwoDarray_ptr = new twoDarray(ncols, nrows);
      environment_ptr->append_wtwoDarray(wtwoDarray_ptr);

      for(int p = 0; p < n_zeroth_layer_pixels; p++)
      {
         int pu = p % ncols;
         int pv = p / ncols;
         double curr_weight_val = 255 * 
            (weights[0]->get(n,p) - min_weight_val) / 
            (max_weight_val - min_weight_val);
         wtwoDarray_ptr->put(pu, pv, curr_weight_val);
      }

      int magnify_factor = 16;
      int mag_nrows = magnify_factor * nrows;
      int mag_ncols = magnify_factor * ncols;
//      cout << "mag_nrows = " << mag_nrows
//           << " mag_ncols = " << mag_ncols << endl;

// On 12/7/16, we discovered that the next twoDarray constructor line
// can fail for reasons we don't understand...

      twoDarray* enlarged_wtwoDarray_ptr = new twoDarray(mag_ncols, mag_nrows);

      for(unsigned int row = 0; row < enlarged_wtwoDarray_ptr->get_ndim(); 
          row++)
      {
         int pv = row / magnify_factor;
         for(unsigned int col = 0; col < enlarged_wtwoDarray_ptr->get_mdim(); 
             col++)
         {
            int pu = col / magnify_factor;
            
            double curr_weight_val = wtwoDarray_ptr->get(pu, pv);
            enlarged_wtwoDarray_ptr->put(col, row, curr_weight_val);
         }
      }

      int n_channels = 3;
      texture_rectangle* tr_ptr = new texture_rectangle(
         mag_ncols, mag_nrows, 1, n_channels, NULL);

      tr_ptr->convert_single_twoDarray_to_three_channels(
         enlarged_wtwoDarray_ptr, true);

      double hue_min =270;
      tr_ptr->convert_grey_values_to_hues(hue_min);

      string output_filename=weights_subdir + 
         "weights_"+stringfunc::integer_to_string(n,3)+".png";
      tr_ptr->write_curr_frame(output_filename);
      string banner="Exported "+output_filename;
      outputfunc::write_banner(banner);

      delete tr_ptr;
      delete enlarged_wtwoDarray_ptr;
      delete wtwoDarray_ptr;

   } // loop over index n labeling weight images

   string script_filename=weights_subdir + "view_zeroth_layer_weights";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);
   script_stream << "view weights_000.png" << endl;
   script_stream << "view weights_001.png" << endl;
   script_stream << "view weights_002.png" << endl;
   script_stream << "view weights_003.png" << endl;
   script_stream << "view weights_004.png" << endl;
   script_stream << "view weights_005.png" << endl;
   script_stream << "view weights_006.png" << endl;
   script_stream << "view weights_007.png" << endl;
   script_stream << "view weights_008.png" << endl;
   script_stream << "view weights_009.png" << endl;
   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
}

// ---------------------------------------------------------------------
void reinforce::compute_bias_distributions()
{
   for(int l = 0; l < n_layers; l++)
   {
      vector<double> bias_values;
      for(unsigned int r = 0; r < biases[l]->get_mdim(); r++)
      {
         bias_values.push_back(biases[l]->get(r));
      }

      double blo = mathfunc::minimal_value(bias_values);
      double bhi = mathfunc::maximal_value(bias_values);
      if(nearly_equal(blo,bhi))
      {
         bias_01[l].push_back(blo);
         bias_05[l].push_back(blo);
         bias_10[l].push_back(blo);
         bias_25[l].push_back(blo);
         bias_35[l].push_back(blo);
         bias_50[l].push_back(blo);
         bias_65[l].push_back(blo);
         bias_75[l].push_back(blo);
         bias_90[l].push_back(blo);
         bias_95[l].push_back(blo);
         bias_99[l].push_back(blo);
      }
      else
      {
         int nbins = 500;
         prob_distribution prob_biases(nbins, blo, bhi, bias_values);
         bias_01[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.01));
         bias_05[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.05));
         bias_10[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.10));
         bias_25[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.25));
         bias_35[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.35));
         bias_50[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.50));
         bias_65[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.65));
         bias_75[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.75));
         bias_90[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.90));
         bias_95[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.95));
         bias_99[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.99));
      }
   } // loop over index l labeling network layers
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

      weight_01[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.01));
      weight_05[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.05));
      weight_10[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.10));
      weight_25[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.25));
      weight_35[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.35));
      weight_50[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.50));
      weight_65[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.65));
      weight_75[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.75));
      weight_90[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.90));
      weight_95[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.95));
      weight_99[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.99));
      
//      cout << "layer = " << l
//           << " wlo = " << wlo
//           << " w_05 = " << weight_05[l].back()
//           << " w_25 = " << weight_25[l].back();
//      cout << "   w_50 = " << weight_50[l].back()
//           << " w_75 = " << weight_75[l].back()
//           << " w_95 = " << weight_95[l].back()
//           << " whi = " << whi
//           << endl;
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
void reinforce::store_quasirandom_weight_values()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      vector<double> weight_values;
      int n_weights_for_layer = weights[l]->get_mdim() * 
         weights[l]->get_ndim();
      int weight_skip = n_weights_for_layer / 9;
      for(unsigned int i = 1; i <= 9; i++)
      {
         int weight_index = i * weight_skip;
         int r = weight_index / weights[l]->get_ndim();
         int c = weight_index % weights[l]->get_ndim();
         double curr_w = weights[l]->get(r,c);

         if(i == 1)
         {
            weight_1[l].push_back(curr_w);
         }
         else if (i == 2)
         {
            weight_2[l].push_back(curr_w);
         }
         else if (i == 3)
         {
            weight_3[l].push_back(curr_w);
         }
         else if (i == 4)
         {
            weight_4[l].push_back(curr_w);
         }
         else if (i == 5)
         {
            weight_5[l].push_back(curr_w);
         }
         else if (i == 6)
         {
            weight_6[l].push_back(curr_w);
         }
         else if (i == 7)
         {
            weight_7[l].push_back(curr_w);
         }
         else if (i == 8)
         {
            weight_8[l].push_back(curr_w);
         }
         else if (i == 9)
         {
            weight_9[l].push_back(curr_w);
         }
      }
   } // loop over index l labeling network layers
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
// Generate metafile plot of input STL vector of values plotted
// against either epoch or episode independent variables.  The total
// number of independent and dependent variables do NOT need to be
// equal.  

void reinforce::generate_metafile_plot(
   const vector<double>& values,
   string metafile_basename, string title,
   string y_label, string extrainfo, bool epoch_indep_var,
   bool plot_smoothed_values_flag, bool zero_min_value_flag)
{
   if(values.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+metafile_basename;
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
   }
   title += "; nweights="+stringfunc::number_to_string(n_weights);

   string subtitle=init_subtitle() + " " + extrainfo;
   string x_label="Episode";
   double xmax = get_episode_number();
   if(epoch_indep_var)
   {
      x_label="Epoch";
      xmax = get_curr_epoch();
   }
   double min_value = mathfunc::minimal_value(values);
   if(zero_min_value_flag)
   {
      min_value = 0;
   }
   double max_value = mathfunc::maximal_value(values);

   if(nearly_equal(min_value, max_value))
   {
      double avg_value = 0.5 * (min_value + max_value);
      min_value = 0.5 * avg_value;
      max_value = 1.5 * avg_value;
   }
   
   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, 
      min_value, max_value);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.openmetafile();
   curr_metafile.write_header();

   if(epoch_indep_var)
   {
      curr_metafile.write_curve(epoch_history, values);
   }
   else
   {
      curr_metafile.write_curve(episode_history, values);
   }
   curr_metafile.set_thickness(3);

// Temporally smooth noisy input values:

   if(plot_smoothed_values_flag)
   {
      int n_values = values.size();
      double sigma = 10;
      if(n_values > 100)
      {
         sigma += log10(values.size())/log10(2.0);
      }
      double dx = 1;
      int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

      if(gaussian_size < n_values)
      {
         vector<double> h;
         h.reserve(gaussian_size);
         filterfunc::gaussian_filter(dx, sigma, h);

         bool wrap_around_input_values = false;
         vector<double> smoothed_values;
         filterfunc::brute_force_filter(
            values, h, smoothed_values, wrap_around_input_values);
         if(epoch_indep_var)
         {
            curr_metafile.write_curve(
               epoch_history, smoothed_values, colorfunc::blue);
         }
         else
         {
            curr_metafile.write_curve(
               episode_history, smoothed_values, colorfunc::blue);
         }
      }
   } // plot_smoothed_values_flag conditional
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of loss values versus time step samples.

void reinforce::plot_loss_history(string extrainfo)
{
   string metafile_basename = "loss_history";
   string title = "Loss function";
   string y_label="Loss";
   bool epoch_indep_var = false;
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = false;

   generate_metafile_plot(
      loss_values, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of average discounted eventual rewards for
// P-learning versus epoch.

void reinforce::plot_avg_discounted_eventual_reward(
   string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "eventual_rewards_history";
   string title = "Average eventual reward";
   string y_label="Average eventual reward";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = false;

   generate_metafile_plot(
      avg_discounted_eventual_rewards, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of max Q distribution for evaluation states
// versus epoch.

void reinforce::plot_maxQ_history(string extrainfo, bool epoch_indep_var)
{
   if(max_eval_Qvalues_50.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir + "/maxQ_history";

   string title="Max Q percentiles for "+stringfunc::number_to_string(
      eval_memory_capacity)+" evaluation states";
   title += ";lambda="+stringfunc::scinumber_to_string(lambda,2);
   title += "; nweights="+stringfunc::number_to_string(n_weights);

   string subtitle=init_subtitle();
   subtitle += ";"+extrainfo;
   string x_label = "Episode";
   double xmax = episode_number;
   if(epoch_indep_var)
   {
      x_label="Epoch";
      xmax = curr_epoch;
   }
   string y_label="Max Q percentiles";

   double min_Q = mathfunc::minimal_value(max_eval_Qvalues_10) - 0.5;
   double max_Q = mathfunc::maximal_value(max_eval_Qvalues_90) + 0.5;
   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, min_Q, max_Q);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.set_thickness(2);

   curr_metafile.write_curve(0, xmax, max_eval_Qvalues_10,
                             colorfunc::get_color(0));
//   curr_metafile.write_curve(0, xmax, max_eval_Qvalues_25,
//                             colorfunc::get_color(1));
//   curr_metafile.write_curve(0, xmax, max_eval_Qvalues_75,
//                             colorfunc::get_color(3));
   curr_metafile.write_curve(0, xmax, max_eval_Qvalues_90,
                             colorfunc::get_color(4));
   curr_metafile.write_curve(0, xmax, max_eval_Qvalues_50,
                             colorfunc::get_color(2));
   curr_metafile.set_thickness(3);
 
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of reward versus time.

void reinforce::plot_reward_history(
   string extrainfo, bool epoch_indep_var, bool plot_cumulative_reward)
{
   if(plot_cumulative_reward)
   {
      string metafile_basename = "reward_history";
      string title = "Cumulative reward";
      string ylabel = "Cumulative reward";
      bool plot_smoothed_values_flag = true;
      bool zero_min_value_flag = false;
      
      generate_metafile_plot(
         cumulative_reward_snapshots, metafile_basename, 
         title, ylabel, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
         zero_min_value_flag);
   }
   else
   {
      string metafile_basename = "reward_history";
      string title = "Running reward";
      string ylabel = "Running reward";
      bool plot_smoothed_values_flag = true;
      bool zero_min_value_flag = false;
      
      generate_metafile_plot(
         running_reward_snapshots, metafile_basename, 
         title, ylabel, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
         zero_min_value_flag);
   }
}

// ---------------------------------------------------------------------
// Generate metafile plot of total number of turns versus episode number.

void reinforce::plot_turns_history(string extrainfo)
{
   string metafile_basename = "turns_history";
   string title = "Number of AI and agent turns";
   string y_label="Number of AI + agent turns";
   bool epoch_indep_var = false;
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = true;

   generate_metafile_plot(
      n_episode_turns_frac, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of total number of ALE frames per episode number.

void reinforce::plot_frames_history(string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "frames_history";
   string title = "Number of ALE frames per episode";
   string y_label="Number of ALE frames per episode";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = true;

   generate_metafile_plot(
      n_frames_per_episode, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of total number of episodes vs epoch.

void reinforce::plot_episode_number_history(
   string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "n_episodes";
   string title = "Number of episodes";
   string y_label="Number of episodes";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = true;

   generate_metafile_plot(
      episode_history, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of epsilon vs episode number

void reinforce::plot_epsilon_history(string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "epsilon_history";
   string title = "Epsilon";
   string y_label="Epsilon";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = true;

   generate_metafile_plot(
      epsilon_values, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of learning rate vs episode number

void reinforce::plot_lr_history(string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "lr_history";
   string title = "Learning rate";
   string y_label="Learning rate";
   bool plot_smoothed_values_flag = false;
   bool zero_min_value_flag = false;

   generate_metafile_plot(
      learning_rate, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of Qmap score versus episode number.

void reinforce::plot_Qmap_score_history(string extrainfo)
{
   string metafile_basename = "Qmap_score_history";
   string title = "Qmap_score";
   string y_label="Qmap_score";
   bool epoch_indep_var = false;
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = true;

   generate_metafile_plot(
      Qmap_scores, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of log10(total loss) versus episode number.

void reinforce::plot_log10_loss_history(string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "log10_losses_history";
   string title = "Log10(total loss)";
   string y_label="Log10(total loss)";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = false;

   generate_metafile_plot(
      log10_losses, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of log10(lr * mean_abs_nabla_weight_ratios)
// versus episode number.

void reinforce::plot_log10_lr_mean_abs_nabla_weight_ratios(
   string extrainfo,bool epoch_indep_var)
{
   string metafile_basename = "lr_nabla_weight_ratios";
   string title="learning rate * <|nabla_weight_ratio|>";
   string y_label="log10(learning rate * <|nabla_weight_ratio|>)";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = false;

   generate_metafile_plot(
      log10_lr_mean_abs_nabla_weight_ratios, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of mean KL divergence vs episode number

void reinforce::plot_KL_divergence_history(
   string extrainfo, bool epoch_indep_var)
{
   string metafile_basename = "KL_history";
   string title = "Mean KL divergence";
   string y_label="log10(mean KL divergence between pi_next and pi_curr)";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = false;

   generate_metafile_plot(
      log10_mean_KL_divergences, metafile_basename, 
      title, y_label, extrainfo, epoch_indep_var, plot_smoothed_values_flag,
      zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of action0 probabilities versus frame for a
// particular episode.

void reinforce::plot_prob_action_0(string extrainfo)
{
   if(prob_action_0.size() <= 1) return;

   metafile curr_metafile;
   string prob_subdir = output_subdir + "action_probs/";
   filefunc::dircreate(prob_subdir);
   string meta_filename=prob_subdir + "/prob_action0_"+
      stringfunc::integer_to_string(episode_number, 4);

   string title="Probability of action 0";
   title += ";lambda="+stringfunc::scinumber_to_string(lambda,2);
   title += "; nweights="+stringfunc::number_to_string(n_weights);

   double median_prob_action0, qw_prob_action0;
   mathfunc::median_value_and_quartile_width(
      prob_action_0, median_prob_action0, qw_prob_action0);
   string subtitle="Median prob action_0 = "+
      stringfunc::number_to_string(median_prob_action0) + " +/- "+
      stringfunc::number_to_string(qw_prob_action0);
   string x_label = "Frame number for episode "+stringfunc::number_to_string(
      episode_number);
   double xmax = prob_action_0.size();
   string y_label="probability of action 0";

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, 0, 1);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.25);
   curr_metafile.set_ysubtic(0.125);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.set_thickness(2);
   curr_metafile.write_curve(0, xmax, prob_action_0);
   curr_metafile.set_thickness(3);
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of bias distributions versus episode number.

bool reinforce::plot_bias_distributions(string extrainfo, bool epoch_indep_var)
{
   if(bias_01[0].size() < 3) return false;

   string script_filename=output_subdir + "view_bias_dists";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 1; l < bias_50.size(); l++)
   {
      metafile curr_metafile;
      string basename="bias_dists_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;

      string title="Bias dists for layer "+stringfunc::number_to_string(l);
      title += "; lambda="+stringfunc::number_to_string(lambda);
      title += "; nweights="+stringfunc::number_to_string(n_weights);

      string subtitle=init_subtitle();
      subtitle += ";"+extrainfo;
      string x_label="Episode";
      double xmax = get_episode_number();
      if(epoch_indep_var)
      {
         x_label="Epoch";
         xmax = get_curr_epoch();
      }
      string y_label="Bias distributions";

      double max_bias = NEGATIVEINFINITY;
      double min_bias = POSITIVEINFINITY; 
      max_bias = basic_math::max(
         max_bias, mathfunc::maximal_value(bias_99[l]));
      min_bias = basic_math::min(
         min_bias, mathfunc::minimal_value(bias_01[l]));

      curr_metafile.set_parameters(
         meta_filename, title, x_label, y_label, 0, xmax, min_bias, max_bias);
      curr_metafile.set_subtitle(subtitle);
      curr_metafile.openmetafile();
      curr_metafile.write_header();
      curr_metafile.set_thickness(2);

      if(epoch_indep_var)
      {
         curr_metafile.write_curve(
            epoch_history, bias_01[l], colorfunc::get_color(0));
         curr_metafile.write_curve(
            epoch_history, bias_05[l], colorfunc::get_color(1));
         curr_metafile.write_curve(
            epoch_history, bias_10[l], colorfunc::get_color(2));
         curr_metafile.write_curve(
            epoch_history, bias_25[l], colorfunc::get_color(3));
         curr_metafile.write_curve(
            epoch_history, bias_35[l], colorfunc::get_color(4));
         curr_metafile.write_curve(
            epoch_history, bias_50[l], colorfunc::get_color(5));
         curr_metafile.write_curve(
            epoch_history, bias_65[l], colorfunc::get_color(6));
         curr_metafile.write_curve(
            epoch_history, bias_75[l], colorfunc::get_color(7));
         curr_metafile.write_curve(
            epoch_history, bias_90[l], colorfunc::get_color(8));
         curr_metafile.write_curve(
            epoch_history, bias_95[l], colorfunc::get_color(9));
         curr_metafile.write_curve(
            epoch_history, bias_99[l], colorfunc::get_color(10));
      }
      else
      {
         curr_metafile.write_curve(
            episode_history, bias_01[l], colorfunc::get_color(0));
         curr_metafile.write_curve(
            episode_history, bias_05[l], colorfunc::get_color(1));
         curr_metafile.write_curve(
            episode_history, bias_10[l], colorfunc::get_color(2));
         curr_metafile.write_curve(
            episode_history, bias_25[l], colorfunc::get_color(3));
         curr_metafile.write_curve(
            episode_history, bias_35[l], colorfunc::get_color(4));
         curr_metafile.write_curve(
            episode_history, bias_50[l], colorfunc::get_color(5));
         curr_metafile.write_curve(
            episode_history, bias_65[l], colorfunc::get_color(6));
         curr_metafile.write_curve(
            episode_history, bias_75[l], colorfunc::get_color(7));
         curr_metafile.write_curve(
            episode_history, bias_90[l], colorfunc::get_color(8));
         curr_metafile.write_curve(
            episode_history, bias_95[l], colorfunc::get_color(9));
         curr_metafile.write_curve(
            episode_history, bias_99[l], colorfunc::get_color(10));
      }
      
      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);
      string jpg_filename=basename+".jpg";
      script_stream << "view "+jpg_filename << endl;
   } // loop over index l labeling network layers

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
   return true;
}

// ---------------------------------------------------------------------
// Generate metafile plot of weight distributions versus episode number.

void reinforce::plot_weight_distributions(
   string extrainfo, bool epoch_indep_var)
{
   string script_filename=output_subdir + "view_weight_dists";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 0; l < weight_50.size(); l++)
   {
      if(weight_01[l].size() < 3) continue;

      metafile curr_metafile;
      string basename="weight_dists_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;

      string title="Weight dists for layer "+stringfunc::number_to_string(l);
      title += "; lambda="+stringfunc::number_to_string(lambda);
      title += "; nweights="+stringfunc::number_to_string(n_weights);

      string subtitle=init_subtitle();
      subtitle += ";"+extrainfo;
      string x_label="Episode";
      double xmax = get_episode_number();
      if(epoch_indep_var)
      {
         x_label="Epoch";
         xmax = get_curr_epoch();
      }
      string y_label="Weight distributions";

      double max_weight = NEGATIVEINFINITY;
      double min_weight = POSITIVEINFINITY; 
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_99[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_01[l]));

      curr_metafile.set_parameters(
         meta_filename, title, x_label, y_label, 0, xmax, 
         min_weight, max_weight);
      curr_metafile.set_subtitle(subtitle);
      curr_metafile.openmetafile();
      curr_metafile.write_header();
      curr_metafile.set_thickness(2);

      if(epoch_indep_var)
      {
         curr_metafile.write_curve(
            epoch_history, weight_01[l], colorfunc::get_color(0));
         curr_metafile.write_curve(
            epoch_history, weight_05[l], colorfunc::get_color(1));
         curr_metafile.write_curve(
            epoch_history, weight_10[l], colorfunc::get_color(2));
         curr_metafile.write_curve(
            epoch_history, weight_25[l], colorfunc::get_color(3));
         curr_metafile.write_curve(
            epoch_history, weight_35[l], colorfunc::get_color(4));
         curr_metafile.write_curve(
            epoch_history, weight_50[l], colorfunc::get_color(5));
         curr_metafile.write_curve(
            epoch_history, weight_65[l], colorfunc::get_color(6));
         curr_metafile.write_curve(
            epoch_history, weight_75[l], colorfunc::get_color(7));
         curr_metafile.write_curve(
            epoch_history, weight_90[l], colorfunc::get_color(8));
         curr_metafile.write_curve(
            epoch_history, weight_95[l], colorfunc::get_color(9));
         curr_metafile.write_curve(
            epoch_history, weight_99[l], colorfunc::get_color(10));
      }
      else
      {
         curr_metafile.write_curve(
            episode_history, weight_01[l], colorfunc::get_color(0));
         curr_metafile.write_curve(
            episode_history, weight_05[l], colorfunc::get_color(1));
         curr_metafile.write_curve(
            episode_history, weight_10[l], colorfunc::get_color(2));
         curr_metafile.write_curve(
            episode_history, weight_25[l], colorfunc::get_color(3));
         curr_metafile.write_curve(
            episode_history, weight_35[l], colorfunc::get_color(4));
         curr_metafile.write_curve(
            episode_history, weight_50[l], colorfunc::get_color(5));
         curr_metafile.write_curve(
            episode_history, weight_65[l], colorfunc::get_color(6));
         curr_metafile.write_curve(
            episode_history, weight_75[l], colorfunc::get_color(7));
         curr_metafile.write_curve(
            episode_history, weight_90[l], colorfunc::get_color(8));
         curr_metafile.write_curve(
            episode_history, weight_95[l], colorfunc::get_color(9));
         curr_metafile.write_curve(
            episode_history, weight_99[l], colorfunc::get_color(10));
      }
      
      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);
      string jpg_filename=basename+".jpg";
      script_stream << "view "+jpg_filename << endl;
   } // loop over index l labeling network layers

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
}

// ---------------------------------------------------------------------
// Generate metafile plot of quasi-random weight values versus episode number.

void reinforce::plot_quasirandom_weight_values(
   string extrainfo, bool epoch_indep_var)
{
   string script_filename=output_subdir + "view_weight_values";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 0; l < weight_50.size(); l++)
   {
      if(weight_1[l].size() < 3) continue;

      metafile curr_metafile;
      string basename="weight_values_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;
      string title="Quasi random weight values for layer "
         +stringfunc::number_to_string(l);
      title += ";lambda="+stringfunc::scinumber_to_string(lambda,2);
      title += "; nweights="+stringfunc::number_to_string(n_weights);

      string subtitle=init_subtitle();
      subtitle += ";"+extrainfo;
      string x_label="Episode";
      double xmax = get_episode_number();
      if(epoch_indep_var)
      {
         x_label="Epoch";
         xmax = get_curr_epoch();
      }
      string y_label="Weight values";

      double max_weight = NEGATIVEINFINITY;
      double min_weight = POSITIVEINFINITY; 
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_1[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_2[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_3[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_4[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_5[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_6[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_7[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_8[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_9[l]));

      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_1[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_2[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_3[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_4[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_5[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_6[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_7[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_8[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_9[l]));

      curr_metafile.set_parameters(
         meta_filename, title, x_label, y_label, 0, xmax,
         min_weight, max_weight);
      curr_metafile.set_subtitle(subtitle);
      curr_metafile.openmetafile();
      curr_metafile.write_header();
      curr_metafile.set_thickness(2);

      if(epoch_indep_var)
      {
         curr_metafile.write_curve(
            epoch_history, weight_1[l], colorfunc::get_color(0));
         curr_metafile.write_curve(
            epoch_history, weight_2[l], colorfunc::get_color(1));
         curr_metafile.write_curve(
            epoch_history, weight_3[l], colorfunc::get_color(2));
         curr_metafile.write_curve(
            epoch_history, weight_4[l], colorfunc::get_color(3));
         curr_metafile.write_curve(
            epoch_history, weight_5[l], colorfunc::get_color(4));
         curr_metafile.write_curve(
            epoch_history, weight_6[l], colorfunc::get_color(5));
         curr_metafile.write_curve(
            epoch_history, weight_7[l], colorfunc::get_color(6));
         curr_metafile.write_curve(
            epoch_history, weight_8[l], colorfunc::get_color(7));
         curr_metafile.write_curve(
            epoch_history, weight_9[l], colorfunc::get_color(8));
      }
      else
      {
         curr_metafile.write_curve(
            episode_history, weight_1[l], colorfunc::get_color(0));
         curr_metafile.write_curve(
            episode_history, weight_2[l], colorfunc::get_color(1));
         curr_metafile.write_curve(
            episode_history, weight_3[l], colorfunc::get_color(2));
         curr_metafile.write_curve(
            episode_history, weight_4[l], colorfunc::get_color(3));
         curr_metafile.write_curve(
            episode_history, weight_5[l], colorfunc::get_color(4));
         curr_metafile.write_curve(
            episode_history, weight_6[l], colorfunc::get_color(5));
         curr_metafile.write_curve(
            episode_history, weight_7[l], colorfunc::get_color(6));
         curr_metafile.write_curve(
            episode_history, weight_8[l], colorfunc::get_color(7));
         curr_metafile.write_curve(
            episode_history, weight_9[l], colorfunc::get_color(8));
      }

      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      string jpg_filename=basename+".jpg";
      script_stream << "view "+jpg_filename << endl;
   } // loop over index l labeling network layers

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
}

// ---------------------------------------------------------------------
// Member function generate_summary_plots() outputs metafile plots of
// weight distributions, rewards, epsilon, nframes/episode and loss
// histories.

void reinforce::generate_summary_plots(string extrainfo, bool epoch_indep_var)
{
   plot_episode_number_history(extrainfo, epoch_indep_var);
   plot_frames_history(extrainfo, epoch_indep_var);
   bool plot_cumulative_reward = true;
   plot_reward_history(extrainfo, epoch_indep_var, plot_cumulative_reward);
   plot_log10_loss_history(extrainfo, epoch_indep_var);
   plot_lr_history(extrainfo, epoch_indep_var);
   plot_log10_lr_mean_abs_nabla_weight_ratios(extrainfo, epoch_indep_var);

   if(get_include_biases()){
      plot_bias_distributions(extrainfo, epoch_indep_var);
   }
   plot_weight_distributions(extrainfo, epoch_indep_var);
   plot_quasirandom_weight_values(extrainfo, epoch_indep_var);

   if (learning_type == QLEARNING)
   {
      plot_maxQ_history(extrainfo, epoch_indep_var);
      plot_epsilon_history(extrainfo, epoch_indep_var);
   }
   else if (learning_type == PLEARNING)
   {
      plot_avg_discounted_eventual_reward(extrainfo, epoch_indep_var);
      plot_prob_action_0(extrainfo);
      plot_KL_divergence_history(extrainfo, epoch_indep_var);
   }
}

// ---------------------------------------------------------------------
// Member function generate_view_metrics_script creates an executable
// script which displays reward, nframes/episode, max Q and epsilon
// history metafile outputs.

void reinforce::generate_view_metrics_script(bool maze_flag, bool atari_flag)
{
   string script_filename=output_subdir + "view_metrics";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   script_stream << "view log10_losses_history.jpg" << endl;
   script_stream << "view lr_history.jpg" << endl;
   script_stream << "view lr_nabla_weight_ratios.jpg" << endl;

   if(environment_ptr->get_world_type() != environment::TTT)
   {
      script_stream << "view n_episodes.jpg" << endl;
   }
   else
   {
      script_stream << "view game_performance.jpg" << endl;
   }
   
   if(maze_flag)  
   {
      script_stream << "view Qmap_score_history.jpg" << endl;
   }
   
   if(!maze_flag)
   {
      script_stream << "view reward_history.jpg" << endl;
   }

   if(atari_flag)// breakout, pong
   {
      script_stream << "view frames_history.jpg" << endl;
      script_stream << "view paddle_X.jpg" << endl;
      script_stream << "view paddle_Y.jpg" << endl;
   }

   if(learning_type == QLEARNING)
   {
      script_stream << "view maxQ_history.jpg" << endl;
      script_stream << "view epsilon_history.jpg" << endl;

   }
   else if(learning_type == PLEARNING)
   {
      script_stream << "view eventual_rewards_history.jpg" << endl;
      script_stream << "view KL_history.jpg" << endl;
   }
   
   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);

   cout << "script_filename = " << script_filename << endl;
}

// ---------------------------------------------------------------------
// Member function create_snapshots_subdir()

void reinforce::create_snapshots_subdir()
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

void reinforce::export_snapshot()
{
   if(snapshots_subdir.size() == 0) create_snapshots_subdir();
   
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
   outstream << lambda << endl;
   outstream << gamma << endl;
   outstream << rmsprop_decay_rate << endl;

//   cout << "batch_size = " << batch_size << endl;
//   cout << "base_learning_rate = " << base_learning_rate << endl;
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

void reinforce::import_snapshot(string snapshot_filename)
{
   cout << "inside reinforce::import_snapshot()" << endl;

   ifstream instream;
   filefunc::openfile(snapshot_filename,instream);

   instream >> expt_number;    		// Added early on Mon Jan 23
   instream >> output_subdir;  		// Added early on Mon Jan 23
   instream >> include_biases;		// Added early on Mon Jan 23
   instream >> n_layers;

   for(int i = 0; i < n_layers; i++)
   {
      int curr_layer_dim;
      instream >> curr_layer_dim;
      layer_dims.push_back(curr_layer_dim);
   }
   n_actions = layer_dims.back();

   delete_weights_and_biases();
   instantiate_weights_and_biases();

   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         int mdim;
         instream >> mdim;

         for(int row = 0; row < mdim; row++)
         {
            double curr_bias;
            instream >> curr_bias;
            biases[l]->put(row, curr_bias);
         }
      }
   } // include_biases conditional

   for(int l = 0; l < n_layers-1; l++)
   {
      int mdim, ndim;
      instream >> mdim;
      instream >> ndim;

      for(int row = 0; row < mdim; row++)
      {
         for(int col = 0; col < ndim; col++)
         {
            double curr_weight;
            instream >> curr_weight;
            weights[l]->put(row, col, curr_weight);
         }
      }
//      cout << "weights[l] = " << *weights[l] << endl;
   } // loop over index l labeling weight matrices
   
   filefunc::closefile(snapshot_filename,instream);
   cout << "Imported trained neural network from " << snapshot_filename 
        << endl;
}

// ==========================================================================
// General learning methods
// ==========================================================================

// Member function clear_delta_nablas() initializes "batch" weight and
// bias gradients to zero.

void reinforce::clear_delta_nablas()
{
   if(include_biases)
   {
      for(unsigned int l = 0; l < delta_nabla_biases.size(); l++)
      {
         delta_nabla_biases[l]->clear_values();
      }
   }

   for(unsigned int l = 0; l < delta_nabla_weights.size(); l++)
   {
      delta_nabla_weights[l]->clear_values();
   }

   for(unsigned int l = 0; l < Delta_Prime.size(); l++)
   {
      Delta_Prime[l]->clear_values();
   }
}

// ---------------------------------------------------------------------
void reinforce::clear_nablas()
{
   if(include_biases)
   {
      for(unsigned int l = 0; l < nabla_biases.size(); l++)
      {
         nabla_biases[l]->clear_values();
      }
   }

   for(unsigned int l = 0; l < nabla_weights.size(); l++)
   {
      nabla_weights[l]->clear_values();
   }
}

// ---------------------------------------------------------------------
// Member function update_weights_and_biases()

void reinforce::update_weights_and_biases(double lr)
{
   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         *biases[l] -= lr * (*nabla_biases[l]);
      } // loop over index l labeling network layers
   } // include_biases_flag
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      *weights[l] -= lr * (*nabla_weights[l]);
   }
}

// ---------------------------------------------------------------------
// Member function decrease_learning_rate decreases the learning rate
// down to some minimal floor value.

void reinforce::decrease_learning_rate()
{
   double curr_learning_rate = get_learning_rate();
   double min_learning_rate = 0.1 * get_base_learning_rate();

   if(curr_learning_rate > min_learning_rate)
   {
      push_back_learning_rate(0.8 * curr_learning_rate);
   }
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
   if(include_biases)
   {
      for(unsigned int l = 0; l < biases.size(); l++)
      {
         *old_biases[l] = *biases[l];
      }
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
// Member function 

void reinforce::set_epsilon(double eps)
{
   epsilon = eps;
}

double reinforce::get_epsilon() const
{
   return epsilon;
}

void reinforce::set_min_epsilon(double min_eps)
{
   min_epsilon = min_eps;
}

void reinforce::set_epsilon_time_constant(double tau)
{
   epsilon_tau = tau;
}

double reinforce::exponentially_decay_epsilon(double t, double tstart)
{
   if(t < tstart)
   {
      epsilon = 1;
   }
   else
   {
      epsilon = min_epsilon + (1 - min_epsilon) * 
         exp(- (t - tstart) / epsilon_tau);
   }
   return epsilon;
}

double reinforce::linearly_decay_epsilon(double t, double tstart, double tstop)
{
   if(t < tstart)
   {
      epsilon = 1;
   }
   else if (t > tstop)
   {
      epsilon = min_epsilon;
   }
   else
   {
      epsilon = 1 + (min_epsilon - 1) * (t - tstart) / (tstop - tstart);
   }
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
      double Qmax;
      return compute_legal_argmax_Q(Qmax);
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

   for(unsigned int s = 0; s < curr_states.size(); s++)
   {
      Q_forward_propagate(curr_states[s]);
      for(unsigned int i = 0; i < Z_Prime[n_layers-1]->get_mdim(); i++)
      {
         string state_action_str = environment_ptr->
            get_state_action_string(curr_state_strings[s], i);
         double Qvalue = A_Prime[n_layers-1]->get(i);
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
   *A_Prime[0] = *s_input;
//   cout << "inside reinforce::Q_forward_propagate()" << endl;

   for(int l = 0; l < n_layers-1; l++)
   {
      if(use_old_weights_flag)
      {
         if(include_biases)
         {
            Z_Prime[l+1]->matrix_vector_mult_sum(
               *old_weights[l], *A_Prime[l], *old_biases[l+1]);
         }
         else
         {
            Z_Prime[l+1]->matrix_vector_mult(*old_weights[l], *A_Prime[l]);
         }
      }
      else
      {
         if(include_biases)
         {
            Z_Prime[l+1]->matrix_vector_mult_sum(
               *weights[l], *A_Prime[l], *biases[l+1]);
         }
         else
         {
            Z_Prime[l+1]->matrix_vector_mult(*weights[l], *A_Prime[l]);
         }
      } // use_old_weights_flag conditional

//      machinelearning_func::batch_normalization(
//         *Z_Prime[l+1], *gammas[l+1], *betas[l+1]);

      if(l == n_layers - 2)
      {
         for(unsigned int i = 0; i < Z_Prime[l+1]->get_mdim(); i++)
         {
            *A_Prime[l+1] = *Z_Prime[l+1];
         }
      }
      else
      {
         machinelearning_func::leaky_ReLU(*Z_Prime[l+1], *A_Prime[l+1]);
      }
   }
}

// ---------------------------------------------------------------------
// Member function compute_argmax_Q() returns a = argmax_a' Q(s,a').

int reinforce::compute_argmax_Q()
{
   double Qstar = NEGATIVEINFINITY;
   int curr_a = -1;
   for(unsigned int i = 0; i < A_Prime[n_layers-1]->get_mdim(); i++)
   {
      double curr_activation = A_Prime[n_layers-1]->get(i);

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
int reinforce::compute_legal_argmax_Q(double& Qstar)
{
   Qstar = NEGATIVEINFINITY;
   return compute_legal_argextremum_Q(true, Qstar);
}

int reinforce::compute_legal_argmin_Q(double& Qstar)
{
   Qstar = POSITIVEINFINITY;
   return compute_legal_argextremum_Q(false, Qstar);
}

int reinforce::compute_legal_argextremum_Q(bool max_flag, double& Qstar)
{
   int curr_a = -1;
   for(unsigned int i = 0; i < A_Prime[n_layers-1]->get_mdim(); i++)
   {
      if(!environment_ptr->is_legal_action(i)) continue;

      double curr_activation = A_Prime[n_layers-1]->get(i);
   
// As of 11/17/16, we experiment with randomly breaking "ties" between
// Q value scores:

      if(nearly_equal(curr_activation, Qstar))
      {
         if(nrfunc::ran1() > 0.5)
         {
            Qstar = curr_activation;
            curr_a = i;
         }
         continue;
      }
      if(max_flag)
      {
         if(curr_activation > Qstar)
         {
            Qstar = curr_activation;
            curr_a = i;
         }
      }
      else
      {
         if(curr_activation < Qstar)
         {
            Qstar = curr_activation;
            curr_a = i;
         }
      }
   }
   return curr_a;
}

// ---------------------------------------------------------------------
// Member function store_curr_state_into_replay_memory()

int reinforce::store_curr_state_into_replay_memory(const genvector& curr_s)
{
//    cout << "inside store_curr_state_into_replay_memory()" << endl;
   int d = -999;
   if(replay_memory_index < replay_memory_capacity)
   {
      d = replay_memory_index;
      replay_memory_index++;
   }
   else
   {
      replay_memory_full_flag = true;
      replay_memory_index = 0;
      d = replay_memory_index;
   }
//   cout << "d = " << d << endl;
//   cout << "s_curr = " << s_curr << endl;
//   cout << "s_curr: mdim = " << s_curr->get_mdim()
//        << " ndim = " << s_curr->get_ndim() << endl;
//   cout << "curr_s = " << curr_s << endl;
   
   s_curr->put_row(d, curr_s);
   return d;
}

// ---------------------------------------------------------------------
// Member function store_arsprime_into_replay_memory()

void reinforce::store_arsprime_into_replay_memory(
   int d, int curr_a, double curr_r,
   const genvector& next_s, bool terminal_state_flag)
{
//   cout << "inside reinforce::store_arsprime_into_replay_mem()" << endl;

   a_curr->put(d, curr_a);
   r_curr->put(d, curr_r);
   s_next->put_row(d, next_s);
   terminal_state->put(d, terminal_state_flag);

//   genvector curr_s(next_s);
//   s_curr->get_row(d, curr_s);
//   cout << "|next_s - curr_s| = " << (next_s - curr_s).magnitude()
//        << endl;
//   cout << "curr_r = " << curr_r << endl;
}

void reinforce::store_final_arsprime_into_replay_memory(
   int d, int curr_a, double curr_r)
{
   a_curr->put(d, curr_a);
   r_curr->put(d, curr_r);
   s_next->copy_row(d, *s_curr);
   terminal_state->put(d, true);
//   cout << "curr_r = " << curr_r << endl;
}

// ---------------------------------------------------------------------
// Member function store_ar_into_replay_memory()

void reinforce::store_ar_into_replay_memory(int d, int curr_a, double curr_r,
                                            bool terminal_state_flag)
{
//   cout << "inside reinforce::store_ar_into_replay_mem()" << endl;

   a_curr->put(d, curr_a);
   r_curr->put(d, curr_r);
   terminal_state->put(d, terminal_state_flag);
}

// ---------------------------------------------------------------------
// Member function get_replay_memory_entry()

bool reinforce::get_replay_memory_entry(
   int d, genvector& curr_s, int& curr_a, double& curr_r)
{
   s_curr->get_row(d, curr_s);
   double a_val = a_curr->get(d);
   curr_a = int(a_val);
   curr_r = r_curr->get(d);

   bool terminal_state_flag = (terminal_state->get(d) > 0);
   return terminal_state_flag;

}

bool reinforce::get_replay_memory_entry(
   int d, genvector& curr_s, int& curr_a, double& curr_r, genvector& next_s)
{
   s_next->get_row(d, next_s);
   return get_replay_memory_entry(d, curr_s, curr_a, curr_r);
}


// ---------------------------------------------------------------------
// Boolean member function store_curr_state_into_eval_memory() returns
// true if the evaluation memory is full.

bool reinforce::store_curr_state_into_eval_memory(const genvector& curr_s)
{
//   cout << "inside store_curr_state_into_eval_memory()" << endl;
//   cout << "eval_memory_index = " << eval_memory_index
//        << " eval_memory_capacity = " << eval_memory_capacity
//        << endl;
   
   if(eval_memory_index < eval_memory_capacity)
   {
      s_eval->put_row(eval_memory_index, curr_s);
      eval_memory_index++;
   }
   else
   {
      eval_memory_full_flag = true;
   }

   return eval_memory_full_flag;
}

// ---------------------------------------------------------------------
// Member function compute_max_eval_Qvalues_distribution() retrieves
// every randomly generated state stored within evaluation memory
// genmatrix s_eval.  It computes the maximum Q value for each
// evaluation state.  This method returns the average of the maximum Q
// values which serves as a metric for Q learning.

void reinforce::compute_max_eval_Qvalues_distribution()
{
//   cout << "inside compute_max_eval_Qvalues_distribution()" << endl;

   if(!eval_memory_full_flag) return;

   bool use_old_weights_flag = false;
   genvector eval_s(s_eval->get_ndim());

   vector<double> max_Qvalues;
   for(int d = 0; d < eval_memory_capacity; d++)
   {
      s_eval->get_row(d, eval_s);

      Q_forward_propagate(&eval_s, use_old_weights_flag);
      double Qmax = A_Prime[n_layers-1]->get(0);
      for(unsigned int j = 0; j < A_Prime[n_layers-1]->get_mdim(); j++)
      {
         Qmax = basic_math::max(Qmax, A_Prime[n_layers-1]->get(j));
      }
      max_Qvalues.push_back(Qmax);
   } // loop over index d labeling evaluation states

   vector<double> percentile_fracs;
   percentile_fracs.push_back(0.10);
   percentile_fracs.push_back(0.25);
   percentile_fracs.push_back(0.50);
   percentile_fracs.push_back(0.75);
   percentile_fracs.push_back(0.90);

   vector<double> values;
   mathfunc::percentile_values(max_Qvalues, percentile_fracs, values);

   max_eval_Qvalues_10.push_back(values[0]);
   max_eval_Qvalues_25.push_back(values[1]);
   max_eval_Qvalues_50.push_back(values[2]);
   max_eval_Qvalues_75.push_back(values[3]);
   max_eval_Qvalues_90.push_back(values[4]);
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
      double Qmax = A_Prime[n_layers-1]->get(0);
      for(unsigned int j = 0; j < A_Prime[n_layers-1]->get_mdim(); j++)
      {
         Qmax = basic_math::max(Qmax, A_Prime[n_layers-1]->get(j));
      }
      return curr_r + gamma * Qmax;
   }
}

// ---------------------------------------------------------------------
// Member function update_rmsprop_biases_cache()

void reinforce::update_rmsprop_biases_cache(double decay_rate)
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

void reinforce::update_rmsprop_weights_cache(double decay_rate)
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
// Member function update_Q_network() takes in Nd = number of
// random samples to be drawn from replay memory.

double reinforce::update_Q_network(bool verbose_flag)
{
//   cout << "inside update_Q_network()" << endl;

   vector<int> d_samples = mathfunc::random_sequence(
      replay_memory_capacity, Nd);
   double total_loss = 0;
   for(unsigned int j = 0; j < d_samples.size(); j++)
   {
      int curr_d = d_samples[j];
      double curr_loss = Q_backward_propagate(curr_d, Nd, verbose_flag);
      if(curr_loss >= 0) total_loss += curr_loss;
   } // loop over index j labeling replay memory samples
//   cout << "total_loss = " << total_loss << endl;

   if(solver_type == RMSPROP)
   {
      if(include_biases)
      {
         update_rmsprop_biases_cache(rmsprop_decay_rate);
      }
      update_rmsprop_weights_cache(rmsprop_decay_rate);
   }
   else if (solver_type == ADAM)
   {
      if(include_biases)
      {
         update_rmsprop_biases_cache(beta2);
      }
      update_rmsprop_weights_cache(beta2);
   }
   
// Update weights and biases for each network layer by their nabla
// values averaged over the current mini-batch:

   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         if(solver_type == SGD)
         {
            *biases[l] -= get_learning_rate() * (*nabla_biases[l]);
         }
         else if (solver_type == RMSPROP)
         {
            rmsprop_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
            rmsprop_biases_denom[l]->hadamard_sum(rmsprop_denom_const);
            nabla_biases[l]->hadamard_ratio(*rmsprop_biases_denom[l]);
            *biases[l] -= get_learning_rate() * (*nabla_biases[l]);
         }
         else if (solver_type == ADAM)
         {

/*
// FAKE FAKE:  ADAM solver for biases needs fixing
// Weds Nov 30 at 4:11 am

            *adam_m_biases[l] = beta1 * (*adam_m[l]) + 
               (1 - beta1) * (*nabla_biases[l]);
            curr_beta1_pow *= beta1;
            *adam_m_biases[l] /= (1 - curr_beta1_pow);

            curr_beta2_pow *= beta2;
            *rmsprop_biases_cache[l] /= (1 - curr_beta2_pow);
         
            rmsprop_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
            const double TINY = 1E-8;
            rmsprop_biases_denom[l]->hadamard_sum(TINY);
            adam_m_biases[l]->hadamard_ratio(*rmsprop_biases_denom[l]);
            *biases[l] -= learning_rate.back() * (*adam_m_biases[l]);
            */

         }
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
      } // loop over index l labeling network layers
   } // include_biases_flag
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      if (solver_type == SGD)
      {
         *weights[l] -= get_learning_rate() * (*nabla_weights[l]);
      }
      else if (solver_type == MOMENTUM)
      {
         *velocities[l] = mu * (*velocities[l])
            - learning_rate.back() * (*nabla_weights[l]);
         *weights[l] += *velocities[l];
      }
      else if (solver_type == NESTEROV)
      {
         *prev_velocities[l] = *velocities[l];
         *velocities[l] = mu * (*velocities[l])
            - learning_rate.back() * (*nabla_weights[l]);
         *weights[l] += -mu * (*prev_velocities[l]) +
            (1 + mu) * (*velocities[l]);
      }
      else if(solver_type == RMSPROP)
      {
         rmsprop_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         rmsprop_weights_denom[l]->hadamard_sum(rmsprop_denom_const);
         nabla_weights[l]->hadamard_division(*rmsprop_weights_denom[l]);
         *weights[l] -= get_learning_rate() * (*nabla_weights[l]);
      }
      else if(solver_type == ADAM)
      {
         *adam_m[l] = beta1 * (*adam_m[l]) + (1 - beta1) * (*nabla_weights[l]);
         curr_beta1_pow *= beta1;
         *adam_m[l] /= (1 - curr_beta1_pow);

         curr_beta2_pow *= beta2;
         *rmsprop_weights_cache[l] /= (1 - curr_beta2_pow);
         
         rmsprop_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         const double TINY = 1E-8;
         rmsprop_weights_denom[l]->hadamard_sum(TINY);
         adam_m[l]->hadamard_division(*rmsprop_weights_denom[l]);
         *weights[l] -= learning_rate.back() * (*adam_m[l]);
      }
      
// Record average |nabla_weight / weight| to monitor network learning:

      int mdim = nabla_weights[l]->get_mdim();
      int ndim = nabla_weights[l]->get_ndim();
         
      vector<double> curr_nabla_weights;
      vector<double> curr_nabla_weight_ratios;
//         vector<double> curr_adam_ms;

      for(int r = 0; r < mdim; r++)
      {
         for(int c = 0; c < ndim; c++)
         {
            curr_nabla_weights.push_back(fabs(nabla_weights[l]->get(r,c)));
//               curr_adam_ms.push_back(fabs(adam_m[l]->get(r,c)));
//               cout << "r = " << r << " c = " << c
//                    << " curr_nabla_weight = " 
//                    << curr_nabla_weights.back() << endl;
            double denom = weights[l]->get(r,c);
            if(fabs(denom) > 1E-10)
            {
               curr_nabla_weight_ratios.push_back(
                  fabs(nabla_weights[l]->get(r,c) / denom ));
            }
         }
      }
      double mean_abs_nabla_weight = mathfunc::mean(curr_nabla_weights);
      double mean_abs_nabla_weight_ratio = mathfunc::mean(
         curr_nabla_weight_ratios);
//         double mean_abs_adam_m = mathfunc::mean(curr_adam_ms);
      log10_lr_mean_abs_nabla_weight_ratios.push_back(
         log10(learning_rate.back() * mean_abs_nabla_weight_ratio));

      if(verbose_flag)
      {
         cout << "layer l = " << l
              << " mean |nabla weight| = " 
              << mean_abs_nabla_weight 
              << " mean |nabla weight/weight| = " 
              << mean_abs_nabla_weight_ratio  << endl;
            
         cout << "lr = " << learning_rate.back() 
              << " lr * <|nabla_weight|> = " 
              << learning_rate.back() * mean_abs_nabla_weight 
              << "  lr * <|nabla_weight/weight|> = " 
              << learning_rate.back() * mean_abs_nabla_weight_ratio 
              << " log10(lr * <|nabla_weight/weight/>) = "
              << log10_lr_mean_abs_nabla_weight_ratios.back()
              << endl;

//         cout << " mean_abs_adam_m = " << mean_abs_adam_m
//              << " lr * mean_abs_adam_m = " 
//              << learning_rate.back() * mean_abs_adam_m
//              << endl;
      } // verbose_flag conditional
      
      if(include_biases) nabla_biases[l]->clear_values();
      nabla_weights[l]->clear_values();
   } // loop over index l labeling network layers
   
   return total_loss;
}

// ---------------------------------------------------------------------
// Member function L2_loss_contribution() adds the L2 regularization
// term's contribution to the loss function.

double reinforce::L2_loss_contribution()
{
   double L2_loss = 0;

   const double TINY = 1E-8;
   if(lambda > TINY)
   {
      double sqrd_weight_sum = 0;
      for(int l = 0; l < n_layers - 1; l++)
      {
         for(unsigned int r = 0; r < weights[l]->get_mdim(); r++)
         {
            for(unsigned int c = 0; c < weights[l]->get_ndim(); c++)
            {
               sqrd_weight_sum += sqr(weights[l]->get(r,c));
            }
         }
      } // loop over index l labeling network layers
      L2_loss += (lambda / n_weights) * sqrd_weight_sum;
   }
   return L2_loss;
}

// ---------------------------------------------------------------------
// Member function compute_curr_Q_loss() assumes that a forward
// propagation has recently been performed.  It returns the current
// contribution to the loss function for Q-learning.

double reinforce::compute_curr_Q_loss(int curr_a, double target_value)
{
   int curr_layer = n_layers - 1;
   double curr_loss = -1;
   for(int j = 0; j < layer_dims[curr_layer]; j++)
   {
      double curr_Q = A_Prime[curr_layer]->get(j);
      double curr_activation = 0;
      if(j == curr_a)
      {
         curr_activation = curr_Q - target_value;
         curr_loss = 0.5 * sqr(curr_activation);
      }
      Delta_Prime[curr_layer]->put(j, curr_activation);
   }

   if(curr_loss <= 0)
   {
      cout << "Current loss = " << curr_loss << endl;
      cout << "  curr_s_sample.mag = " << curr_s_sample->magnitude()
           << endl;
      for(int l = 0; l < n_layers; l++)
      {
         cout << "l = " << l
              << " A_Prime.mag = " << A_Prime[l]->magnitude()
              << endl;
      }

      for(int j = 0; j < layer_dims[curr_layer]; j++)
      {
         double curr_Q = A_Prime[curr_layer]->get(j);
         cout << "   j = " << j << " curr_Q = " << curr_Q 
              << " target = " << target_value
              << " curr_a = " << curr_a 
              << endl;
      }
   }

   curr_loss += L2_loss_contribution();
   return curr_loss;
}

// ---------------------------------------------------------------------
// Member function Q_backward_propagate()

double reinforce::Q_backward_propagate(int d, int Nd, bool verbose_flag)
{
   clear_delta_nablas();

// Calculate target for curr transition sample:

   int curr_a;
   double curr_r;
   bool terminal_state_flag = get_replay_memory_entry(
      d, *curr_s_sample, curr_a, curr_r, *next_s_sample);
   double target_value = 
      compute_target(curr_r, next_s_sample, terminal_state_flag);
   double inverse_Nd = 1.0 / Nd;

   if(verbose_flag)
   {
      cout << "inside Q_backward_propagate()" << endl;
      cout << "  curr_r = " << curr_r << " target_value = " << target_value
           << endl;
   }

// First need to perform forward propagation for *curr_s_sample in
// order to repopulate linear z inputs and nonlinear a outputs for
// each node in the neural network:

   Q_forward_propagate(curr_s_sample);

   int curr_layer = n_layers - 1;

// Eqn BP1:

   for(int j = 0; j < layer_dims[curr_layer]; j++)
   {
      double curr_Q = A_Prime[curr_layer]->get(j);
      double curr_activation = 0;
      if(j == curr_a)
      {
         curr_activation = curr_Q - target_value;
      }
      Delta_Prime[curr_layer]->put(j, curr_activation);
   }

   for(curr_layer = n_layers-1; curr_layer >= 1; curr_layer--)
   {

// Don't bother backpropagating if current layer has zero content:
//      if(Delta_Prime[curr_layer]->magnitude() <= 0) break;

      int prev_layer = curr_layer - 1;

// Eqn BP2A:

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

      weights_transpose[prev_layer]->matrix_transpose(*weights[prev_layer]);
      Delta_Prime[prev_layer]->matrix_vector_mult(
         *weights_transpose[prev_layer], *Delta_Prime[curr_layer]);

// Eqn BP2B (Leaky ReLU):
      for(int j = 0; j < layer_dims[prev_layer]; j++)
      {
         if(Z_Prime[prev_layer]->get(j) < 0)
         {
            Delta_Prime[prev_layer]->put(
               j, machinelearning_func::get_leaky_ReLU_small_slope() * 
               Delta_Prime[prev_layer]->get(j));
         }
      } // loop over j index

// Accumulate bias & weight gradients over batch of episodes:

// Eqn BP3:

      if(include_biases)
      {
         *delta_nabla_biases[curr_layer] = *Delta_Prime[curr_layer];
      }

// Eqn BP4:

      delta_nabla_weights[prev_layer]->compute_outerprod(
         *Delta_Prime[curr_layer], *A_Prime[prev_layer]);

// Add L2 regularization contribution to delta_nabla_weights.  No such
// regularization contribution is conventionally added to
// delta_nabla_biases:

      const double TINY = 1E-8;
      if(lambda > TINY)
      {
//         *delta_nabla_weights[prev_layer] += 
//            2 * (lambda / n_weights) * (*weights[prev_layer]);
         delta_nabla_weights[prev_layer]->matrix_increment(
            2 * lambda / n_weights, *weights[prev_layer]);
      }
   } // loop over curr_layer index

// Accumulate biases' and weights' gradients for each network layer:

   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         nabla_biases[l]->vector_increment(inverse_Nd, *delta_nabla_biases[l]);
      }
   }
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      nabla_weights[l]->matrix_increment(inverse_Nd, *delta_nabla_weights[l]);
   }

   n_backprops++;

   double curr_loss = compute_curr_Q_loss(curr_a, target_value);

// On 12/26/16, we explicitly spot-checked backpropagated
// loss-function derivatives with their numerically calculated
// counterparts.  To very good approximation, the two precisely
// matched...

/*
// Numerically spot-check loss derivatives wrt a few random
// weights:

   if(nrfunc::ran1() < 1E-4)
   {
      numerically_check_Q_derivs(curr_a, target_value);
   }
*/

   curr_loss *= inverse_Nd;
   return curr_loss;
}

// ---------------------------------------------------------------------
void reinforce::numerically_check_Q_derivs(int curr_a, double target_value)
{
   const double eps = 1E-6;
   for(unsigned int l = 0; l < weights.size(); l++)
   {
      int row = nrfunc::ran1() * weights[l]->get_mdim();
      int col = nrfunc::ran1() * weights[l]->get_ndim();
      double orig_weight = weights[l]->get(row, col);
      weights[l]->put(row, col, orig_weight + eps);
      Q_forward_propagate(curr_s_sample);
      double pos_loss = compute_curr_Q_loss(curr_a, target_value);
      weights[l]->put(row, col, orig_weight - eps);
      Q_forward_propagate(curr_s_sample);      
      double neg_loss = compute_curr_Q_loss(curr_a, target_value);
      double curr_deriv = (pos_loss - neg_loss) / (2 * eps);
      weights[l]->put(row, col, orig_weight);

      cout.precision(12);
      cout << "l = " << l << " row = " << row << " col = " << col << endl;
      cout << "  pos_loss = " << pos_loss << " neg_loss = " << neg_loss
           << endl;
      cout << " curr_deriv = " << curr_deriv 
           << " delta_nabla_weight = " 
           << delta_nabla_weights[l]->get(row, col)
           << endl;

      if(fabs(curr_deriv) > 1E-10)
      {
         double ratio = 
            delta_nabla_weights[l]->get(row, col) / curr_deriv;
         cout << "  delta_nabla_weight / curr_deriv = " << ratio << endl;
      }
         

   } // loop over index l labeling network layers
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
   return A_Prime[n_layers-1]->get(0);
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

// ==========================================================================
// Policy gradient learning methods
// ==========================================================================

// Member function P_forward_propagate() performs a feedforward pass
// for the input state s to get predicted probabilities for all
// actions.

void reinforce::P_forward_propagate(genvector* s_input)
{
//   cout << "inside P_forward_propagate()" << endl;
//   cout << "s_input = " << s_input << endl;
   
   *A_Prime[0] = *s_input;
   for(int l = 0; l < n_layers-1; l++)
   {
      if(include_biases)
      {
         Z_Prime[l+1]->matrix_vector_mult_sum(
            *weights[l], *A_Prime[l], *biases[l+1]);
      }
      else
      {
         Z_Prime[l+1]->matrix_vector_mult(*weights[l], *A_Prime[l]);
      }

//      machinelearning_func::batch_normalization(
//         *Z_Prime[l+1], *gammas[l+1], *betas[l+1]);

      if(l == n_layers - 2)
      {
         machinelearning_func::softmax(*Z_Prime[l+1], *A_Prime[l+1]);
      }
      else
      {
         machinelearning_func::leaky_ReLU(*Z_Prime[l+1], *A_Prime[l+1]);
      }
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
// Member function compute_curr_pi_given_state() performs a forward
// pass of the P-network for the input state.  It stores the ouput
// softmax action probabilities within member genvector
// *curr_pi_sample.

void reinforce::compute_pi_given_state(
   genvector* s_input, genvector* pi_output)
{
//   cout << "inside reinforce::compute_pi_given_state()" << endl;
   P_forward_propagate(s_input);

   for(unsigned int a = 0; a < A_Prime[n_layers-1]->get_mdim(); a++)
   {
      pi_output->put(a, A_Prime[n_layers-1]->get(a));
   }
}

// ---------------------------------------------------------------------
// Member function store_curr_pi_into_replay_memory()

void reinforce::store_curr_pi_into_replay_memory(int d, genvector *curr_pi)
{
   pi_curr->put_row(d, *curr_pi);
}

void reinforce::get_curr_pi_from_replay_memory(int d)
{
   pi_curr->get_row(d, *curr_pi_sample);
}

// ---------------------------------------------------------------------
void reinforce::compute_next_pi_given_replay_index(int d)
{
//   cout << "inside reinforce::compute_curr_pi_given_replay_index()" << endl;

   s_curr->get_row(d, *curr_s_sample);
   compute_pi_given_state(curr_s_sample, next_pi_sample);
}

// ---------------------------------------------------------------------
// Member function
// compute_mean_KL_divergence_between_curr_and_next_pi() loops over
// every state within the replay memory.  For each state, it retrieves
// curr_pi which we assume was calculated using the "current" set of
// P-network weights.  This method subsequently computes next_pi using
// the "next" set of P-network weights.  The KL divergence between
// curr_pi and next_pi is calculated over all states in the replay
// memory.  This method returns the average KL divergence.

double reinforce::compute_mean_KL_divergence_between_curr_and_next_pi()
{
   if(!replay_memory_full_flag)
   {
      return -1;
   }
   
   double mean_KL_divergence = 0;
   for(int d = 0; d < replay_memory_capacity; d++)
   {
      get_curr_pi_from_replay_memory(d);
      compute_next_pi_given_replay_index(d);
      mean_KL_divergence += 
         mathfunc::KL_divergence(curr_pi_sample, next_pi_sample);
   }
   mean_KL_divergence /= replay_memory_capacity;
   return mean_KL_divergence;
}

// ---------------------------------------------------------------------
// Member function get_P_action_for_curr_state() returns integer index
// a for the action which is stochastically sampled from the
// probability distribution encoded in the P-network's final layer.
// It also returns the probability associated with the sampled action.

int reinforce::get_P_action_given_pi(
   genvector *curr_pi, double ran_val, double& action_prob)
{
//   cout << "inside reinforce::get_P_action_given_pi()" << endl;

   double cum_p = 0;
   for(unsigned int a = 0; a < curr_pi->get_mdim(); a++)
   {
      action_prob = curr_pi->get(a);
//      cout << "a = " << a 
//           << " action_prob = " << action_prob
//           << " ran_val = " << ran_val 
//           << endl;

      if(ran_val >= cum_p && ran_val <= cum_p + action_prob)
      {
         return a;
      }
      else
      {
         cum_p += action_prob;
      }
   }

   cout << "Error in reinforce::get_P_action_given_pi()" << endl;
   cout << "Should not have reached this point" << endl;
   exit(-1);
   return -1;
}

// ---------------------------------------------------------------------
// Member function compute_curr_P_loss

double reinforce::compute_curr_P_loss(int d, double action_prob)
{
   double curr_advantage = get_advantage(d);
   double curr_loss = curr_advantage * -log(action_prob);
   curr_loss += L2_loss_contribution();
   return curr_loss;
}

// ---------------------------------------------------------------------
// Member function P_backward_propagate()

double reinforce::P_backward_propagate(int d, int Nd, bool verbose_flag)
{
//   cout << "inside P_backward_propagate, d = " << d << endl;

   double curr_advantage = get_advantage(d);
   if(nearly_equal(curr_advantage, 0)) return 0;

   clear_delta_nablas();

// Calculate target for curr transition sample:

   int stored_a;
   double curr_R, action_prob = -1;
   get_replay_memory_entry(d, *curr_s_sample, stored_a, curr_R);

// First need to perform forward propagation for *curr_s_sample in
// order to repopulate linear z inputs and nonlinear a outputs for
// each node in the neural network:

   P_forward_propagate(curr_s_sample);

// Eqn BP1:

   int curr_layer = n_layers - 1;
   for(int j = 0; j < layer_dims[curr_layer]; j++)
   {
      double curr_activation = A_Prime[curr_layer]->get(j);
      if(j == stored_a)
      {
         action_prob = curr_activation;
         curr_activation -= 1.0;
      }
      Delta_Prime[curr_layer]->put(j, curr_advantage * curr_activation);
   }

   for(curr_layer = n_layers - 1; curr_layer >= 1; curr_layer--)
   {

// Don't bother backpropagating if current layer has zero content:
//      if(Delta_Prime[curr_layer]->magnitude() <= 0) 
//      {
//         cout << "mag = 0" << endl;
//         break;
//      }

      int prev_layer = curr_layer - 1;

// Eqn BP2A:

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

      weights_transpose[prev_layer]->matrix_transpose(*weights[prev_layer]);
      Delta_Prime[prev_layer]->matrix_vector_mult(
         *weights_transpose[prev_layer], *Delta_Prime[curr_layer]);

// Eqn BP2B (Leaky ReLU):
      for(int j = 0; j < layer_dims[prev_layer]; j++)
      {
         if(Z_Prime[prev_layer]->get(j) < 0)
         {
            Delta_Prime[prev_layer]->put(
               j, machinelearning_func::get_leaky_ReLU_small_slope() * 
               Delta_Prime[prev_layer]->get(j));
         }
      } // loop over j index

// Accumulate bias & weight gradients over batch of episodes:

// Eqn BP3:

      if(include_biases)
      {
         *delta_nabla_biases[curr_layer] = *Delta_Prime[curr_layer];
      }

// Eqn BP4:

      delta_nabla_weights[prev_layer]->compute_outerprod(
         *Delta_Prime[curr_layer], *A_Prime[prev_layer]);

// Add L2 regularization contribution to delta_nabla_weights.  No such
// regularization contribution is conventionally added to
// delta_nabla_biases:

      const double TINY = 1E-8;
      if(lambda > TINY)
      {
//         *delta_nabla_weights[prev_layer] += 
//            2 * (lambda / n_weights) * (*weights[prev_layer]);
         delta_nabla_weights[prev_layer]->matrix_increment(
            2 * lambda / n_weights, *weights[prev_layer]);
      }
   } // loop over curr_layer index

// Accumulate biases' and weights' gradients for each network layer:

   double inverse_Nd = 1.0 / Nd;
   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         nabla_biases[l]->vector_increment(inverse_Nd, *delta_nabla_biases[l]);
      }
   }
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      nabla_weights[l]->matrix_increment(inverse_Nd, *delta_nabla_weights[l]);
   }

   n_backprops++;

// On 12/27/16, we numerically spot-checked loss function
// derivatives wrt random neural network weights.  We empirically
// found that the ratio of numerically derived to backpropagated
// derivatives lay within the interval [0.99, 1.01] provided that the
// |pos_loss - neg_loss| > 1E-13 and eps = 1E-6. Derivatives with
// smaller magnitudes effectively equal 0.  So their ratio becomes
// noisier.

//   if(nrfunc::ran1() < 1E-3)
//   {
//      numerically_check_P_derivs(d, ran_value);
//   }

// Note added on 1/2/17: We need to move this curr_loss computation
// OUT of this method.  The loss should be calculated only after any
// potential weight/bias undoing steps have been performed.

   double curr_loss = compute_curr_P_loss(d, action_prob);
   curr_loss *= inverse_Nd;
   return curr_loss;
}

/*
// ---------------------------------------------------------------------
void reinforce::numerically_check_P_derivs(int d, double ran_value)
{
//   cout << "inside numerically_check_P_derivs, d = " << d << endl;
   const double eps = 1E-6;
   double action_prob;
   for(unsigned int l = 0; l < weights.size(); l++)
   {
      int row = nrfunc::ran1() * weights[l]->get_mdim();
      int col = nrfunc::ran1() * weights[l]->get_ndim();
      double orig_weight = weights[l]->get(row, col);

      weights[l]->put(row, col, orig_weight + eps);
      int curr_a_pos = 
         get_P_action_for_curr_state(ran_value, curr_s_sample);
      double pos_loss = compute_curr_P_loss(d, action_prob);

      weights[l]->put(row, col, orig_weight - eps);
      int curr_a_neg = 
         get_P_action_for_curr_state(ran_value, curr_s_sample);
      double neg_loss = compute_curr_P_loss(d, action_prob);
      double curr_deriv = (pos_loss - neg_loss) / (2 * eps);
      
      weights[l]->put(row, col, orig_weight);
      cout.precision(12);
      cout << "l = " << l << " row = " << row << " col = " << col 
           << " orig_weight = " << orig_weight << endl;
      cout << "  curr_a_pos = " << curr_a_pos 
           << " curr_a_neg = " << curr_a_neg << endl;
      cout << "  pos_loss = " << pos_loss << " neg_loss = " << neg_loss
           << endl;
      cout << "  pos - neg loss = " << pos_loss - neg_loss << endl;
      cout << "  curr_deriv = " << curr_deriv 
           << " delta_nabla_weight = " 
           << delta_nabla_weights[l]->get(row, col)
           << endl;

      if(fabs(curr_deriv) > 1E-10)
      {
         double ratio = 
            delta_nabla_weights[l]->get(row, col) / curr_deriv;
         cout << "  delta_nabla_weight / curr_deriv = " << ratio << endl;

         if(ratio < 0.99 || ratio > 1.01) 
         {
            cout << endl;
            outputfunc::enter_continue_char();
         }
      }
         
   } // loop over index l labeling network layers
}
*/

// ---------------------------------------------------------------------
// Member function update_P_network() 

double reinforce::update_P_network(bool verbose_flag)
{
//   cout << "inside update_P_network()" << endl;

   compute_renormalized_discounted_eventual_rewards();

// Compute total loss = -sum_i advantage_i * log(action prob_i)

   double total_loss = 0;
   for(int d = 0; d < replay_memory_capacity; d++)
   {
//      outputfunc::update_progress_fraction(d, 2500, replay_memory_capacity);
      double curr_loss = P_backward_propagate(
         d, replay_memory_capacity, verbose_flag);
      total_loss += curr_loss;
   } // loop over index j labeling replay memory samples
//   cout << endl;
//   cout << "total_loss = " << total_loss << endl;

   if(solver_type == RMSPROP)
   {
      if(include_biases)
      {
         update_rmsprop_biases_cache(rmsprop_decay_rate);
      }
      update_rmsprop_weights_cache(rmsprop_decay_rate);
   }
   
   if(include_biases)
   {
      for(int l = 0; l < n_layers; l++)
      {
         if (solver_type == SGD)
         {
         }
         else if (solver_type == RMSPROP)
         {
            rmsprop_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
            rmsprop_biases_denom[l]->hadamard_sum(rmsprop_denom_const);
            nabla_biases[l]->hadamard_ratio(*rmsprop_biases_denom[l]);
         }
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
      } // loop over index l labeling network layers
   } // include_biases_flag
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      if (solver_type == SGD)
      {
      }
      else if(solver_type == RMSPROP)
      {
         rmsprop_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         rmsprop_weights_denom[l]->hadamard_sum(rmsprop_denom_const);
         nabla_weights[l]->hadamard_division(*rmsprop_weights_denom[l]);
      }
      
      int mdim = nabla_weights[l]->get_mdim();
      int ndim = nabla_weights[l]->get_ndim();
         
      vector<double> curr_nabla_weights;
      vector<double> curr_nabla_weight_ratios;
      for(int r = 0; r < mdim; r++)
      {
         for(int c = 0; c < ndim; c++)
         {
            curr_nabla_weights.push_back(fabs(nabla_weights[l]->get(r,c)));
            double denom = weights[l]->get(r,c);
            if(fabs(denom) > 1E-10)
            {
               curr_nabla_weight_ratios.push_back(
                  fabs(nabla_weights[l]->get(r,c) / denom ));
            }
         }
      }
      double mean_abs_nabla_weight = mathfunc::mean(curr_nabla_weights);
      double mean_abs_nabla_weight_ratio = mathfunc::mean(
         curr_nabla_weight_ratios);
      log10_lr_mean_abs_nabla_weight_ratios.push_back(
         log10(learning_rate.back() * mean_abs_nabla_weight_ratio));

      if(verbose_flag)
      {
         cout << "layer l = " << l
              << " mean |nabla weight| = " 
              << mean_abs_nabla_weight 
              << " mean |nabla weight/weight| = " 
              << mean_abs_nabla_weight_ratio  << endl;
         
         cout << "lr = " << learning_rate.back()
              << " lr * <|nabla_weight|> = " 
              << learning_rate.back() * mean_abs_nabla_weight 
              << "  lr * <|nabla_weight/weight|> = " 
              << learning_rate.back() * mean_abs_nabla_weight_ratio << endl;
      } // verbose_flag conditional
   } // loop over index l labeling network layers

   return total_loss;
}

// ---------------------------------------------------------------------
//  Member function take_KL_divergence_constrained_step() updates the
//  P-network's weights and biases based upon the current learning
//  rate.  It then evaluates the mean KL divergence between the
//  current and next pi outputs.  If the mean KL divergence exceeds
//  member variable max_mean_KL_divergence, we cut the learning rate
//  by a factor of two and compute a new, smaller set of delta weights
//  and biases.  We iteratively continue this stepping process until
//  either the mean KL divergence bound is respected or a maximum
//  number of undo steps has been performed.

int reinforce::take_KL_divergence_constrained_step()
{
   double lr = get_learning_rate();
   update_weights_and_biases(lr);

   int n_undos = 0;
   int max_n_undos = 6;
   double mean_KL_divergence = 1;
   do
   {
      mean_KL_divergence = 
         compute_mean_KL_divergence_between_curr_and_next_pi(); 

      if(mean_KL_divergence > max_mean_KL_divergence)
      {
         lr *= 0.5;
         update_weights_and_biases(-lr);
         n_undos++;
         cout << "n_undos = " << n_undos << " *****************"
              << endl;
      }
   }
   while(mean_KL_divergence > max_mean_KL_divergence &&
         n_undos < max_n_undos);

   if(mean_KL_divergence > 1E-20)
   {
      log10_mean_KL_divergences.push_back(log10(mean_KL_divergence));
   }

   clear_nablas();
   return n_undos;
}

// ---------------------------------------------------------------------
// Member function compute_renormalized_discounted_eventual_rewards()
// replaces the reward received at every time step with a renormalized
// discounted eventual reward.

void reinforce::compute_renormalized_discounted_eventual_rewards()
{
   double next_R = 0;
   vector<double> discounted_eventual_rewards;
   for(int d = replay_memory_capacity - 1; d >= 0; d--)
   {
      int curr_a;
      double curr_r;
      bool terminal_state = get_replay_memory_entry(
         d, *curr_s_sample, curr_a, curr_r);

      if(terminal_state)
      {
         next_R = 0;
      }

      double curr_R = curr_r + gamma * next_R;

//      cout << "d = " << d 
//           << " curr_r = " << curr_r
//           << " curr_R = " << curr_R 
//           << " next_R = " << next_R 
//           << " terminal_state = " << terminal_state
//           << endl;

      discounted_eventual_rewards.push_back(curr_R);
      r_curr->put(d, curr_R);
      next_R = curr_R;
   } // loop over index d labeling replay memory entries

// Compute mean and standard deviation of discounted eventual rewards
// over entire episode:

   double eventual_threshold = 1.0;
   if(avg_discounted_eventual_rewards.size() > 10)
   {
      eventual_threshold = 10.0 / avg_discounted_eventual_rewards.size();
   }
   if(nrfunc::ran1() < eventual_threshold)
   {
      mathfunc::mean_and_std_dev(discounted_eventual_rewards, mu_R, sigma_R);
      avg_discounted_eventual_rewards.push_back(mu_R);
//      cout << "mu_R = " << mu_R << endl;
   }

// Make sure sigma_R doesn't equal 0!
   if(sigma_R < 1E-6)
   {
      sigma_R = 1;
   }
}

// ---------------------------------------------------------------------
double reinforce::get_advantage(int d) const
{
   double curr_R = r_curr->get(d);

// Renormalize advantage estimator values entering into RL loss
// function so that they have zero mean and unit standard deviation:

   double curr_advantage = (curr_R - mu_R) / sigma_R;
   
   if(environment_ptr->get_world_type() == 0) // maze
   {
      if(curr_R > 0)
      {
         curr_advantage = 1;
      }
      else
      {
         curr_advantage = -1;
      }
   }
   return curr_advantage;   
}

// ---------------------------------------------------------------------
void reinforce::clear_replay_memory()
{
   s_curr->clear_values();
   a_curr->clear_values();
   r_curr->clear_values();

   terminal_state->clear_values();

   if(s_next != NULL)
   {
      s_next->clear_values();
   }

   if(pi_curr != NULL)
   {
      pi_curr->clear_values();
   }
}




