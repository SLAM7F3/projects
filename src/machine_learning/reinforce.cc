// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 12/20/16; 12/21/16; 12/24/16; 12/26/16
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
   include_bias_terms = false;
   debug_flag = false;

// This particular set of hyperparameters yields perfect reinforcement
// learning for an agent not to place any of its pieces into already
// occupied cells within a 4x4 grid !

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
      genvector *curr_Z_Prime = new genvector(layer_dims.back());
      genvector *curr_gamma = new genvector(layer_dims.back());
      genvector *curr_beta = new genvector(layer_dims.back());
      genvector *curr_A_Prime = new genvector(layer_dims.back());
      genvector *curr_Delta_Prime = new genvector(layer_dims.back());
      Z_Prime.push_back(curr_Z_Prime);
      gammas.push_back(curr_gamma);
      betas.push_back(curr_beta);
      A_Prime.push_back(curr_A_Prime);
      Delta_Prime.push_back(curr_Delta_Prime);
   }

   n_weights = 0;
   n_weights = count_weights();
   n_actions = layer_dims.back();
   hardwired_output_action = -1;
   n_backprops = 0;

// Biases for all network layers:

   if(include_bias_terms)
   {
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

         vector<double> dummy_dist;
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
      } // loop over index l labeling network layers

      for(int l = 0; l < n_layers; l++)
      {
         genvector *curr_rmsprop_biases = new genvector(layer_dims[l]);
         curr_rmsprop_biases->clear_values();
         rmsprop_biases_cache.push_back(curr_rmsprop_biases);
         
         genvector *curr_rms_biases_denom = new genvector(layer_dims[l]);
         curr_rms_biases_denom->clear_values();
         rms_biases_denom.push_back(curr_rms_biases_denom);
      }
   } // include_bias_terms conditional

// Weights link layer l with layer l+1:

// Only instantiate genmatrices which are needed depending upon
// selected solver type:
    
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      genmatrix *curr_old_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
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
      
      weights.push_back(curr_weights);
      old_weights.push_back(curr_old_weights);
      genmatrix *curr_weights_transpose = new genmatrix(
         layer_dims[l], layer_dims[l+1]);
      weights_transpose.push_back(curr_weights_transpose);

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
      
      genmatrix *curr_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_nabla_weights->clear_values();
      nabla_weights.push_back(curr_nabla_weights);

      genmatrix *curr_delta_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_delta_nabla_weights->clear_values();
      delta_nabla_weights.push_back(curr_delta_nabla_weights);

      if(solver_type == RMSPROP)
      {
         genmatrix *curr_rmsprop_weights = new genmatrix(
            layer_dims[l+1], layer_dims[l]);
         curr_rmsprop_weights->clear_values();
         rmsprop_weights_cache.push_back(curr_rmsprop_weights);

         genmatrix *curr_rms_weights_denom = 
            new genmatrix(layer_dims[l+1], layer_dims[l]);
         curr_rms_weights_denom->clear_values();
         rms_weights_denom.push_back(curr_rms_weights_denom);
      }

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

      vector<double> dummy_dist;
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
}

// ---------------------------------------------------------------------
void reinforce::allocate_member_objects()
{

   s_curr = new genmatrix(replay_memory_capacity, layer_dims.front());
   a_curr = new genvector(replay_memory_capacity);
   r_curr = new genvector(replay_memory_capacity);
   s_next = new genmatrix(replay_memory_capacity, layer_dims.front());
   terminal_state = new genvector(replay_memory_capacity);

   if (learning_type == QLEARNING)
   {
      s_eval = new genmatrix(eval_memory_capacity, layer_dims.front());
      prob_a = NULL;
   }
   else if(learning_type == PLEARNING)
   {
      s_eval = NULL;
      prob_a = new genvector(replay_memory_capacity);      
   }

   curr_s_sample = new genvector(layer_dims.front());
   next_s_sample = new genvector(layer_dims.front());
   prev_afterstate_ptr = new genvector(layer_dims.front());
}		       

// ---------------------------------------------------------------------
reinforce::reinforce(const vector<int>& n_nodes_per_layer)
{
   this->replay_memory_capacity = 1;
   this->eval_memory_capacity = 1;
   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();
}

reinforce::reinforce(const vector<int>& n_nodes_per_layer, 
                     int replay_memory_capacity, int solver_type)
{
   this->replay_memory_capacity = replay_memory_capacity;
   this->eval_memory_capacity = -1;
   this->solver_type = solver_type;
   this->learning_type = PLEARNING;

   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();
   this->batch_size = -1;
}

reinforce::reinforce(const vector<int>& n_nodes_per_layer, 
                     int batch_size, int replay_memory_capacity,
                     int eval_memory_capacity, int solver_type)
{
   this->replay_memory_capacity = replay_memory_capacity;
   this->eval_memory_capacity = eval_memory_capacity;
   this->solver_type = solver_type;
   this->learning_type = QLEARNING;

   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();
   this->batch_size = batch_size;
}

// ---------------------------------------------------------------------
reinforce::reinforce()
{
   allocate_member_objects();
   import_snapshot();
}

// ---------------------------------------------------------------------
reinforce::~reinforce()
{
   for(int l = 0; l < n_layers; l++)
   {
      delete Z_Prime[l];
      delete gammas[l];
      delete betas[l];
      delete A_Prime[l];
      delete Delta_Prime[l];
   }

   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         delete biases[l];
         delete old_biases[l];
         delete nabla_biases[l];
         delete delta_nabla_biases[l];
         delete rms_biases_denom[l];
      }
   } // include_bias_terms flag

   for(int l = 0; l < n_layers - 1; l++)
   {
      delete weights[l];
      delete old_weights[l];
      delete weights_transpose[l];
      delete nabla_weights[l];
      delete delta_nabla_weights[l];
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
         delete rms_weights_denom[l];
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
   delete prob_a;
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
void reinforce::snapshot_cumulative_reward(double cum_reward)
{
   cumulative_reward_snapshots.push_back(cum_reward);
}

// ---------------------------------------------------------------------
void reinforce::append_n_frames_per_episode(int n_frames)
{
   n_frames_per_episode.push_back(n_frames);
}

void reinforce::append_epsilon()
{
   epsilon_values.push_back(epsilon);
}

// ==========================================================================
// Monitoring network training methods
// ==========================================================================

// Member function summarize_parameters() exports most parameters and
// hyperparameters used for Deep Reinforcement Learning to a specified
// text file for book-keeping purposes.

void reinforce::summarize_parameters(string params_filename)
{
   ofstream params_stream;
   filefunc::openfile(params_filename, params_stream);

   params_stream << timefunc::getcurrdate() << endl;
   params_stream << "Neural net params:" << endl;
   params_stream << "   n_layers = " << n_layers << endl;
   for(int l = 0; l < n_layers; l++)
   {
      params_stream << "   layer = " << l << " n_nodes = " 
                    << layer_dims[l] << endl;
   }
   params_stream << "   n_weights = " << count_weights() << " (FC)" 
                 << endl;
 
   params_stream << "base_learning_rate = " << base_learning_rate 
                 << "; batch_size = " << batch_size
                 << endl;
   if(learning_type == QLEARNING)
   {
      params_stream << "Q learning" << endl;
   }
   else if(learning_type == PLEARNING)
   {
      params_stream << "P learning" << endl;
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
   params_stream << "Discount factor gamma = " << gamma 
                 << endl;
   if(epsilon_tau > 0)
   {
      params_stream << "Epsilon time constant = " << epsilon_tau << endl;
   }
   params_stream << "minimum epsilon = " << min_epsilon << endl;
         
   filefunc::closefile(params_filename, params_stream);
}

// ---------------------------------------------------------------------
// Member function count_weights() sums up the total number of weights
// among all network layers assuming the network is fully connected.

int reinforce::count_weights()
{
   if(n_weights == 0)
   {
      for(int l = 0; l < n_layers - 1; l++)
      {
//         n_weights += weights[l]->get_mdim() * weights[l]->get_ndim();
         n_weights += layer_dims[l] * layer_dims[l+1];
      }
   }
   
   return n_weights;
}

void reinforce::print_biases()
{
   if(!include_bias_terms) return;
   
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
   int n_zeroth_layer_pixels = weights[0]->get_ndim();
   int ncols = sqrt(double(n_zeroth_layer_pixels));
   int nrows = ncols;
   plot_zeroth_layer_weights(ncols, nrows, output_subdir);
}

void reinforce::plot_zeroth_layer_weights(
   int ncols, int nrows, string output_subdir)
{
//   cout << "inside reinforce::plot_zeroth_layer_weights()" << endl;
//   cout << "n_rows = " << nrows << " n_cols = " << ncols << endl;

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

      string output_filename=output_subdir + 
         "weights_"+stringfunc::integer_to_string(n,3)+".png";
      tr_ptr->write_curr_frame(output_filename);
      string banner="Exported "+output_filename;
      outputfunc::write_banner(banner);

      delete tr_ptr;
      delete enlarged_wtwoDarray_ptr;
      delete wtwoDarray_ptr;

   } // loop over index n labeling weight images

   string script_filename=output_subdir + "view_zeroth_layer_weights";
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
      title += "; nweights="+stringfunc::number_to_string(n_weights);
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
// Generate metafile plot of max Q distribution for evaluation states
// versus epoch.

void reinforce::plot_maxQ_history(string output_subdir, string extrainfo,
                                  bool epoch_indep_var)
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

/*
// Temporally smooth noisy avg maxQ values:

   double sigma = 10;
   if(avg_max_eval_Qvalues.size() > 100)
   {
      sigma += log10(avg_max_eval_Qvalues.size())/log10(2.0);
   }
   
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 1.5);

   vector<double> smoothed_avg_max_eval_Qvalues;
   if(gaussian_size < int(avg_max_eval_Qvalues.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);

      bool wrap_around_input_values = false;
      filterfunc::brute_force_filter(
         avg_max_eval_Qvalues, h, smoothed_avg_max_eval_Qvalues, 
         wrap_around_input_values);
      curr_metafile.write_curve(
         0, xmax, smoothed_avg_max_eval_Qvalues, colorfunc::blue);
   }
*/
 
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of running reward sum versus time step samples.

void reinforce::plot_reward_history(
   string output_subdir, string extrainfo, bool epoch_indep_var,
   bool plot_cumulative_reward)
{
   double max_reward = mathfunc::maximal_value(running_reward_snapshots);
   if(plot_cumulative_reward)
   {
      max_reward = mathfunc::maximal_value(cumulative_reward_snapshots);
      if(max_reward < 1) max_reward = 1;
      plot_reward_history(output_subdir, extrainfo, 0, max_reward,
                          cumulative_reward_snapshots, epoch_indep_var);
   }
   else
   {
      max_reward = mathfunc::maximal_value(running_reward_snapshots);
      if(max_reward < 1) max_reward = 1;
      plot_reward_history(output_subdir, extrainfo, 0, max_reward,
                          running_reward_snapshots, epoch_indep_var);
   }
}

void reinforce::plot_reward_history(
   string output_subdir, string extrainfo, 
   double min_reward, double max_reward, 
   const vector<double>& reward_snapshots,
   bool epoch_indep_var)
{
   if(reward_snapshots.size() < 5) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"reward_history";
   string title="Running reward sum; bsize="+
      stringfunc::number_to_string(batch_size);
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
      title += "; nweights="+stringfunc::number_to_string(n_weights);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label = "Episode";
   double xmax = episode_number;
   if(epoch_indep_var)
   {
      x_label="Epoch";
      xmax = curr_epoch;
   }
   string y_label="Running reward sum";

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, min_reward, max_reward);
   curr_metafile.set_subtitle(subtitle);
   double ytic = 0.1 * (max_reward - min_reward);
   curr_metafile.set_ytic(ytic);
   curr_metafile.set_ysubtic(0.5 * ytic);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, xmax, reward_snapshots);

// Temporally smooth noisy reward values:

   double sigma = 10;
   if(reward_snapshots.size() > 100)
   {
      sigma += log10(reward_snapshots.size())/log10(2.0);
   }

   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

   vector<double> smoothed_reward_snapshots;
   if(gaussian_size < int(reward_snapshots.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);
      
      bool wrap_around_input_values = false;
      filterfunc::brute_force_filter(
         reward_snapshots, h, smoothed_reward_snapshots, 
         wrap_around_input_values);

      curr_metafile.set_thickness(3);
      curr_metafile.write_curve(
         0, xmax, smoothed_reward_snapshots, colorfunc::blue);
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
      title += "; nweights="+stringfunc::number_to_string(n_weights);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label="Episode";
   string y_label="Number of AI + agent turns";
   double xmax = episode_number;
   double min_turn_frac = 0;
   double max_turn_frac = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax,
      min_turn_frac, max_turn_frac);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, xmax, n_episode_turns_frac);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy turns fraction values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

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
         0, xmax, smoothed_n_episode_turns_frac, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of total number of ALE frames versus episode number.

void reinforce::plot_frames_history(string output_subdir, string extrainfo,
                                   bool epoch_indep_var)
{
   if(n_frames_per_episode.size() < 5) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"frames_history";
   string title="Number of ALE frames per episode; bsize="+
      stringfunc::number_to_string(batch_size);
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
      title += "; nweights="+stringfunc::number_to_string(n_weights);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label = "Episode";
   double xmax = episode_number;
   if(epoch_indep_var)
   {
      xmax = curr_epoch;
      x_label="Epoch";
   }
   string y_label="Number of ALE frames";
   double min_frames = 0;
   double max_frames = mathfunc::maximal_value(n_frames_per_episode);

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, min_frames, max_frames);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, xmax, n_frames_per_episode);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy frames fraction values:

   double sigma = 10;
   if(n_frames_per_episode.size() > 100)
   {
      sigma += log10(n_frames_per_episode.size())/log10(2.0);
   }
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

   if(gaussian_size < int(n_frames_per_episode.size())) 
   {
      vector<double> h;
      h.reserve(gaussian_size);
      filterfunc::gaussian_filter(dx, sigma, h);

      bool wrap_around_input_values = false;
      vector<double> smoothed_n_frames_per_episode;
      filterfunc::brute_force_filter(
         n_frames_per_episode, h, smoothed_n_frames_per_episode, 
         wrap_around_input_values);

      curr_metafile.write_curve(
         0, xmax, smoothed_n_frames_per_episode, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of epsilon vs episode number

void reinforce::plot_epsilon_history(string output_subdir, string extrainfo,
                                     bool epoch_indep_var)
{
   if(epsilon_values.size() < 5) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"epsilon_history";
   string title="Epsilon vs episode; bsize="+
      stringfunc::number_to_string(batch_size);
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
      title += "; nweights="+stringfunc::number_to_string(n_weights);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label = "Episode";
   double xmax = episode_number;
   if(epoch_indep_var)
   {
      x_label="Epoch";
      xmax = curr_epoch;
   }
   string y_label="Epsilon";

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, 0, 1);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, xmax, epsilon_values);
   curr_metafile.set_thickness(3);
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of learning rate vs episode number

void reinforce::plot_lr_history(string output_subdir, string extrainfo,
                                bool epoch_indep_var)
{
   if(epsilon_values.size() < 5) return;

   metafile curr_metafile;
   string meta_filename=output_subdir+"lr_history";
   string title="Learning rate vs episode";
   if(lambda > 1E-5)
   {
      title += "; lambda="+stringfunc::number_to_string(lambda);
      title += "; nweights="+stringfunc::number_to_string(n_weights);
   }

   string subtitle=init_subtitle();
   subtitle += " "+extrainfo;
   string x_label = "Episode";
   double xmax = episode_number;
   if(epoch_indep_var)
   {
      x_label="Epoch";
      xmax = curr_epoch;
   }
   string y_label="Learning rate";

   double max_lr = mathfunc::maximal_value(learning_rate);
   double min_lr = mathfunc::minimal_value(learning_rate);

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, min_lr, max_lr);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(0, xmax, learning_rate);
   curr_metafile.set_thickness(3);
   
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

   double xmax = episode_number;
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
   curr_metafile.write_curve(0, xmax, Qmap_scores);
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
         0, xmax, smoothed_Qmap_scores, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of log10(total loss) versus episode number.

void reinforce::plot_log10_loss_history(string output_subdir, string extrainfo,
                                        bool epoch_indep_var)
{
   if(log10_losses.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir + "/log10_losses_history";

   string title="Log10(total loss)";
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
   string y_label="Log10(Total loss)";

   double max_score = mathfunc::maximal_value(log10_losses)+0.5;
   double min_score = mathfunc::minimal_value(log10_losses)-0.5;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, min_score, max_score);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.5);
   curr_metafile.set_ysubtic(0.25);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.set_thickness(2);
   curr_metafile.write_curve(0, xmax, log10_losses);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy log10(loss) scores:

   double sigma = 10;
   if(log10_losses.size() > 100)
   {
      sigma += log10(log10_losses.size())/log10(2.0);
   }
   
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
         0, xmax, smoothed_log10_losses, colorfunc::blue);
   }
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of bias distributions versus episode number.

void reinforce::plot_bias_distributions(string output_subdir, string extrainfo,
                                        bool epoch_indep_var)
{
   for(unsigned int l = 1; l < bias_50.size(); l++)
   {
      metafile curr_metafile;
      string meta_filename=output_subdir + "/bias_dists_"+
         stringfunc::number_to_string(l);

      string title="Bias dists for layer "+stringfunc::number_to_string(l);
      title += "; lambda="+stringfunc::number_to_string(lambda);
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

      curr_metafile.write_curve(
         0, xmax, bias_01[l], colorfunc::get_color(0));
      curr_metafile.write_curve(
         0, xmax, bias_05[l], colorfunc::get_color(1));
      curr_metafile.write_curve(
         0, xmax, bias_10[l], colorfunc::get_color(2));
      curr_metafile.write_curve(
         0, xmax, bias_25[l], colorfunc::get_color(3));
      curr_metafile.write_curve(
         0, xmax, bias_35[l], colorfunc::get_color(4));
      curr_metafile.write_curve(
         0, xmax, bias_50[l], colorfunc::get_color(5));
      curr_metafile.write_curve(
         0, xmax, bias_65[l], colorfunc::get_color(6));
      curr_metafile.write_curve(
         0, xmax, bias_75[l], colorfunc::get_color(7));
      curr_metafile.write_curve(
         0, xmax, bias_90[l], colorfunc::get_color(8));
      curr_metafile.write_curve(
         0, xmax, bias_95[l], colorfunc::get_color(9));
      curr_metafile.write_curve(
         0, xmax, bias_99[l], colorfunc::get_color(10));

      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
// Generate metafile plot of weight distributions versus episode number.

void reinforce::plot_weight_distributions(
   string output_subdir, string extrainfo, bool epoch_indep_var)
{
   string script_filename=output_subdir + "view_weight_dists";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 0; l < weight_50.size(); l++)
   {
      metafile curr_metafile;
      string basename="weight_dists_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;

      string title="Weight dists for layer "+stringfunc::number_to_string(l);
      title += "; lambda="+stringfunc::number_to_string(lambda);
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

      curr_metafile.write_curve(
         0, xmax, weight_01[l], colorfunc::get_color(0));
      curr_metafile.write_curve(
         0, xmax, weight_05[l], colorfunc::get_color(1));
      curr_metafile.write_curve(
         0, xmax, weight_10[l], colorfunc::get_color(2));
      curr_metafile.write_curve(
         0, xmax, weight_25[l], colorfunc::get_color(3));
      curr_metafile.write_curve(
         0, xmax, weight_35[l], colorfunc::get_color(4));
      curr_metafile.write_curve(
         0, xmax, weight_50[l], colorfunc::get_color(5));
      curr_metafile.write_curve(
         0, xmax, weight_65[l], colorfunc::get_color(6));
      curr_metafile.write_curve(
         0, xmax, weight_75[l], colorfunc::get_color(7));
      curr_metafile.write_curve(
         0, xmax, weight_90[l], colorfunc::get_color(8));
      curr_metafile.write_curve(
         0, xmax, weight_95[l], colorfunc::get_color(9));
      curr_metafile.write_curve(
         0, xmax, weight_99[l], colorfunc::get_color(10));

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
   string output_subdir, string extrainfo, bool epoch_indep_var)
{
   string script_filename=output_subdir + "view_weight_values";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 0; l < weight_50.size(); l++)
   {
      metafile curr_metafile;
      string basename="weight_values_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;
      string title="Quasi random weight values for layer "
         +stringfunc::number_to_string(l);
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

      curr_metafile.write_curve(
         0, xmax, weight_1[l], colorfunc::get_color(0));
      curr_metafile.write_curve(
         0, xmax, weight_2[l], colorfunc::get_color(1));
      curr_metafile.write_curve(
         0, xmax, weight_3[l], colorfunc::get_color(2));
      curr_metafile.write_curve(
         0, xmax, weight_4[l], colorfunc::get_color(3));
      curr_metafile.write_curve(
         0, xmax, weight_5[l], colorfunc::get_color(4));
      curr_metafile.write_curve(
         0, xmax, weight_6[l], colorfunc::get_color(5));
      curr_metafile.write_curve(
         0, xmax, weight_7[l], colorfunc::get_color(6));
      curr_metafile.write_curve(
         0, xmax, weight_8[l], colorfunc::get_color(7));
      curr_metafile.write_curve(
         0, xmax, weight_9[l], colorfunc::get_color(8));

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

void reinforce::generate_summary_plots(string output_subdir, string extrainfo,
                                       bool epoch_indep_var)
{
   if(get_include_bias_terms()){
      plot_bias_distributions(output_subdir, extrainfo, epoch_indep_var);
   }

   plot_maxQ_history(output_subdir, extrainfo, epoch_indep_var);
   plot_weight_distributions(output_subdir, extrainfo, epoch_indep_var);
   plot_quasirandom_weight_values(output_subdir, extrainfo, epoch_indep_var);
   bool plot_cumulative_reward = true;
   plot_reward_history(output_subdir, extrainfo,plot_cumulative_reward,
                       epoch_indep_var);
   plot_epsilon_history(output_subdir, extrainfo, epoch_indep_var);
   plot_lr_history(output_subdir, extrainfo, epoch_indep_var);
   plot_frames_history(output_subdir, extrainfo, epoch_indep_var);
   plot_log10_loss_history(output_subdir, extrainfo, epoch_indep_var);
}

// ---------------------------------------------------------------------
// Member fucntion generate_view_metrics_script creates an executable
// script which displays reward, nframes/episode, max Q and epsilon
// history metafile outputs.

void reinforce::generate_view_metrics_script(
   string output_subdir, bool Qmap_score_flag)
{
   string script_filename=output_subdir + "view_metrics";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);
   script_stream << "view log10_losses_history.jpg" << endl;
   script_stream << "view lr_history.jpg" << endl;

   if(Qmap_score_flag)
   {
      script_stream << "view Qmap_score_history.jpg" << endl;
   }
   else
   {
      script_stream << "view reward_history.jpg" << endl;
      script_stream << "view frames_history.jpg" << endl;
   }

   if(learning_type == QLEARNING)
   {
      script_stream << "view maxQ_history.jpg" << endl;
      script_stream << "view epsilon_history.jpg" << endl;
   }
   
   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);

   cout << "script_filename = " << script_filename << endl;
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
   instream >> lambda;
   instream >> gamma;
   instream >> rmsprop_decay_rate;

   cout << "batch_size = " << batch_size << endl;
   cout << "base_learning_rate = " << base_learning_rate << endl;
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
// General learning methods
// ==========================================================================

// Member function clear_delta_nablas() initializes "batch" weight and
// bias gradients to zero.

void reinforce::clear_delta_nablas()
{
   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         delta_nabla_biases[l]->clear_values();
      }
   }

   for(int l = 0; l < n_layers - 1; l++)
   {
      delta_nabla_weights[l]->clear_values();
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
   if(include_bias_terms)
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
 
   for(int l = 0; l < n_layers-1; l++)
   {
      if(use_old_weights_flag)
      {
         if(include_bias_terms)
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
         if(include_bias_terms)
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

int reinforce::compute_legal_argmax_Q()
{
   double Qstar = NEGATIVEINFINITY;
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
   }
   else
   {
      replay_memory_full_flag = true;
//      cout << "REPLAY MEMORY IS NOW FULL" << endl;
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

void reinforce::store_action_prob_into_replay_memory(int d, double prob)
{
   prob_a->put(d, prob);
}

// ---------------------------------------------------------------------
// Member function get_replay_memory_entry()

bool reinforce::get_replay_memory_entry(
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

//   avg_max_eval_Qvalues.push_back(mathfunc::mean(max_Qvalues));
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
// Member function update_neural_network() takes in Nd = number of
// random samples to be drawn from replay memory.

double reinforce::update_neural_network(bool verbose_flag)
{
//   cout << "inside update_neural_network()" << endl;

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
      if(include_bias_terms)
      {
         update_biases_cache(rmsprop_decay_rate);
      }
      update_rmsprop_cache(rmsprop_decay_rate);
   }
   else if (solver_type == ADAM)
   {
      if(include_bias_terms)
      {
         update_biases_cache(beta2);
      }
      update_rmsprop_cache(beta2);
   }
   
// Update weights and biases for each network layer by their nabla
// values averaged over the current mini-batch:

   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         if(solver_type == SGD)
         {
            *biases[l] -= learning_rate.back() * (*nabla_biases[l]);
         }
         else if (solver_type == RMSPROP)
         {
            rms_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
            rms_biases_denom[l]->hadamard_sum(rmsprop_denom_const);
            nabla_biases[l]->hadamard_ratio(*rms_biases_denom[l]);
            *biases[l] -= learning_rate.back() * (*nabla_biases[l]);
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
         
            rms_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
            const double TINY = 1E-8;
            rms_biases_denom[l]->hadamard_sum(TINY);
            adam_m_biases[l]->hadamard_ratio(*rms_biases_denom[l]);
            *biases[l] -= learning_rate.back() * (*adam_m_biases[l]);
            */

         }
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
      } // loop over index l labeling network layers
   } // include_bias_terms_flag
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      if (solver_type == SGD)
      {
         *weights[l] -= learning_rate.back() * (*nabla_weights[l]);
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
         rms_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         rms_weights_denom[l]->hadamard_sum(rmsprop_denom_const);
         nabla_weights[l]->hadamard_division(*rms_weights_denom[l]);
         *weights[l] -= learning_rate.back() * (*nabla_weights[l]);
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
         *weights[l] -= learning_rate.back() * (*adam_m[l]);
      }
      
      if(verbose_flag)
      {
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
         
//         cout << " mean_abs_adam_m = " << mean_abs_adam_m
//              << " lr * mean_abs_adam_m = " 
//              << learning_rate.back() * mean_abs_adam_m
//              << endl;

      } // verbose_flag conditional
      
      if(include_bias_terms) nabla_biases[l]->clear_values();
      nabla_weights[l]->clear_values();
   }
//   cout << endl;
//   print_weights();
//   outputfunc::enter_continue_char();

   return total_loss;
}

// ---------------------------------------------------------------------
// Member function compute_curr_loss() assumes that a forward
// propagation has recently been performed.  It returns the current
// contribution to the loss function for Q-learning.

double reinforce::compute_curr_loss(int curr_a, double target_value)
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

// Add L2 regularization term's contribution to loss function:

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
      curr_loss += (lambda / n_weights) * sqrd_weight_sum;
   }
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
//            Delta_Prime[prev_layer]->put(j, 0);
            Delta_Prime[prev_layer]->put(
               j, machinelearning_func::get_leaky_ReLU_small_slope() * 
               Delta_Prime[prev_layer]->get(j));
         }
      } // loop over j index

// Accumulate bias & weight gradients over batch of episodes:

// Eqn BP3:

      if(include_bias_terms)
      {
         *delta_nabla_biases[curr_layer] = *Delta_Prime[curr_layer];
      }

// Eqn BP4:

      delta_nabla_weights[prev_layer]->accumulate_outerprod(
         *Delta_Prime[curr_layer], *A_Prime[prev_layer]);

// Add L2 regularization contribution to delta_nabla_weights.  No such
// regularization contribution is conventionally added to
// delta_nabla_biases:

      const double TINY = 1E-8;
      if(lambda > TINY)
      {
         *delta_nabla_weights[prev_layer] += 
            2 * (lambda / n_weights) * (*weights[prev_layer]);
      }
   } // loop over curr_layer index

// Accumulate biases' and weights' gradients for each network layer:

   if(include_bias_terms)
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

   double curr_loss = compute_curr_loss(curr_a, target_value);

// On 12/26/16, we explicitly spot-checked backpropagated
// loss-function derivatives with their numerically calculated
// counterparts.  To very good approximation, the two precisely
// matched...

/*
// Numerically spot-check loss derivatives wrt a few random
// weights:

   if(nrfunc::ran1() < 1E-4)
   {
      const double eps = 1E-6;
      for(unsigned int l = 0; l < weights.size(); l++)
      {
         int row = nrfunc::ran1() * weights[l]->get_mdim();
         int col = nrfunc::ran1() * weights[l]->get_ndim();
         double orig_weight = weights[l]->get(row, col);
         weights[l]->put(row, col, orig_weight + eps);
         Q_forward_propagate(curr_s_sample);
         double pos_loss = compute_curr_loss(curr_a, target_value);
         weights[l]->put(row, col, orig_weight - eps);
         Q_forward_propagate(curr_s_sample);      
         double neg_loss = compute_curr_loss(curr_a, target_value);
         double curr_deriv = (pos_loss - neg_loss) / (2 * eps);
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
         
         weights[l]->put(row, col, orig_weight);
      } // loop over index l labeling network layers
   }
*/

   curr_loss *= inverse_Nd;
   return curr_loss;
}

// ---------------------------------------------------------------------
void reinforce::numerically_check_derivs()
{

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
   *A_Prime[0] = *s_input;
 
   for(int l = 0; l < n_layers-1; l++)
   {
      if(include_bias_terms)
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
// Member function get_P_action_for_curr_state() returns integer index
// a for the action which is stochastically sampled from the
// probability distribution encoded in the P-network's final layer.
// It also returns the probability associated with the sampled action.

int reinforce::get_P_action_for_curr_state(double& prob_a)
{
   genvector* curr_s = environment_ptr->get_curr_state();
   P_forward_propagate(curr_s);

   double ran_val = nrfunc::ran1();

   double cum_p = 0;
   for(unsigned int a = 0; a < A_Prime[n_layers-1]->get_mdim(); a++)
   {
      prob_a = A_Prime[n_layers-1]->get(a);
      if(ran_val >= cum_p && ran_val <= cum_p + prob_a)
      {
         return a;
      }
      else
      {
         cum_p += prob_a;
      }
   }

   cout << "Error in reinforce::get_P_action_for_curr_state()" << endl;
   cout << "Should not have reached this point" << endl;
   exit(-1);
   return -1;
}

// ---------------------------------------------------------------------
// Member function P_backward_propagate()

double reinforce::P_backward_propagate(int d, int Nd, bool verbose_flag)
{
   clear_delta_nablas();

// Calculate target for curr transition sample:

   int curr_a;
   double curr_advantage;
   get_replay_memory_entry(
      d, *curr_s_sample, curr_a, curr_advantage, *next_s_sample);
   double action_prob = prob_a->get(d);

// First need to perform forward propagation for *curr_s_sample in
// order to repopulate linear z inputs and nonlinear a outputs for
// each node in the neural network:

   Q_forward_propagate(curr_s_sample);

   double curr_loss = - curr_advantage * log(action_prob);

   int curr_layer = n_layers - 1;

// Eqn BP1:

   for(int j = 0; j < layer_dims[curr_layer]; j++)
   {
      double curr_activation = A_Prime[n_layers-1]->get(j);
      if(j == curr_a)
      {
         curr_activation -= 1.0;
      }
      Delta_Prime[curr_layer]->put(j, curr_advantage * curr_activation);
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

      if(include_bias_terms)
      {
         *delta_nabla_biases[curr_layer] = *Delta_Prime[curr_layer];
      }

// Eqn BP4:

      delta_nabla_weights[prev_layer]->accumulate_outerprod(
         *Delta_Prime[curr_layer], *A_Prime[prev_layer]);

      const double TINY = 1E-8;
      if(lambda > TINY)
      {
//         if(include_bias_terms)
//         {
//            *delta_nabla_biases[curr_layer] += 
//               2 * lambda * (*biases[curr_layer]);
         //        }
         *delta_nabla_weights[prev_layer] += 
            2 * (lambda / n_weights) * (*weights[prev_layer]);
      }
   } // loop over curr_layer index

// Accumulate biases' and weights' gradients for each network layer:

   double inverse_Nd = 1.0 / Nd;
   if(include_bias_terms)
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

// Add L2 regularization term's contribution to current loss:

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
      curr_loss += (lambda / n_weights) * sqrd_weight_sum;
   }

   curr_loss *= inverse_Nd;
   return curr_loss;
}

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
      double curr_loss = P_backward_propagate(
         d, replay_memory_capacity, verbose_flag);
      total_loss += curr_loss;
   } // loop over index j labeling replay memory samples
//   cout << "total_loss = " << total_loss << endl;

   if(solver_type == RMSPROP)
   {
      if(include_bias_terms)
      {
         update_biases_cache(rmsprop_decay_rate);
      }
      update_rmsprop_cache(rmsprop_decay_rate);
   }
   
// Update weights and biases for each network layer by their nabla
// values averaged over the current mini-batch:

   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         if (solver_type == RMSPROP)
         {
            rms_biases_denom[l]->hadamard_sqrt(*rmsprop_biases_cache[l]);
            rms_biases_denom[l]->hadamard_sum(rmsprop_denom_const);
            nabla_biases[l]->hadamard_ratio(*rms_biases_denom[l]);
            *biases[l] -= learning_rate.back() * (*nabla_biases[l]);
         }
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
      } // loop over index l labeling network layers
   } // include_bias_terms_flag
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      if(solver_type == RMSPROP)
      {
         rms_weights_denom[l]->hadamard_sqrt(*rmsprop_weights_cache[l]);
         rms_weights_denom[l]->hadamard_sum(rmsprop_denom_const);
         nabla_weights[l]->hadamard_division(*rms_weights_denom[l]);
         *weights[l] -= learning_rate.back() * (*nabla_weights[l]);
      }
      
      if(verbose_flag)
      {
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
      
      if(include_bias_terms) nabla_biases[l]->clear_values();
      nabla_weights[l]->clear_values();
   }

   return total_loss;
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
         d, *curr_s_sample, curr_a, curr_r, *next_s_sample);

      if(terminal_state)
      {
         next_R = 0;
      }
      double curr_R = curr_r + gamma * next_R;
      discounted_eventual_rewards.push_back(curr_R);
      r_curr->put(d, curr_R);
      next_R = curr_R;
   } // loop over index d labeling replay memory entries

// Renormalize discounted eventual rewards so that they have zero mean
// and unit standard deviation:

   double mu, sigma;
   mathfunc::mean_and_std_dev(discounted_eventual_rewards, mu, sigma);
   
   for(int d = 0; d < replay_memory_capacity; d++)
   {
      r_curr->put(d, (r_curr->get(d) - mu) / sigma);
   }
}
