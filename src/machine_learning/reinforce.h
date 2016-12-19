// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 12/12/16; 12/13/16; 12/15/16; 12/19/16
// ==========================================================================

#ifndef REINFORCE_H
#define REINFORCE_H

#include <deque>
#include <map>
#include <iostream>
#include <vector>

#include "machine_learning/environment.h"

class environment;
class genmatrix;
class genvector;

class reinforce
{
   
  public:

   typedef enum{
      SGD = 0,
      RMSPROP = 1,
      MOMENTUM = 2,
      NESTEROV = 3,
      ADAM = 4
   } solver_t;

   typedef std::map<std::string, double > Q_MAP;
// independent string: string rep for state + action
// dependent double: Q(s,a)

// Initialization, constructor and destructor functions:

   reinforce(const std::vector<int>& n_nodes_per_layer);
   reinforce(const std::vector<int>& n_nodes_per_layer,
             int batch_size, int replay_memory_capacity, 
             int eval_memory_capacity, int solver_type = SGD);
   reinforce();
   ~reinforce();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const reinforce& R);

// Set & get member functions:

   void set_include_bias_terms(bool flag);
   bool get_include_bias_terms() const;
   void set_environment(environment* e_ptr);
   void set_debug_flag(bool flag);
   bool get_debug_flag() const;
   int get_episode_number() const;
   int increment_episode_number();
   void append_n_episode_turns_frac(double curr_n_turns_frac);
   void set_base_learning_rate(double rate);
   double get_base_learning_rate() const;
   void set_learning_rate(double rate);
   double get_learning_rate() const;
   void set_batch_size(double bsize);
   int get_batch_size() const;
   void set_lambda(double lambda);
   void set_Nd(int Nd);
   void set_gamma(double gamma);
   double get_gamma() const;
   void set_rmsprop_decay_rate(double rate);
   void set_ADAM_params(double beta1, double beta2);

   void initialize_episode();
   void snapshot_running_reward();
   void snapshot_cumulative_reward(double cum_reward);
   void accumulate_reward(double curr_reward);
   void append_n_episode_frames(int n_frames);
   void append_epsilon();

// Monitoring network training methods:

   void summarize_parameters(std::string params_filename);
   int count_weights();
   void print_biases();
   void print_weights();
   void plot_zeroth_layer_weights(std::string output_subdir);
   void plot_zeroth_layer_weights(int nx, int ny, std::string output_subdir);
   void compute_bias_distributions();
   void compute_weight_distributions();
   void store_quasirandom_weight_values();

   std::string init_subtitle();
   void plot_loss_history(std::string output_subdir, std::string extrainfo);
   void plot_avg_maxQ_history(
      std::string output_subdir, std::string extrainfo);
   void plot_reward_history(std::string output_subdir, std::string extrainfo,
      bool plot_cumulative_reward = false);
   void plot_reward_history(
      std::string output_subdir, std::string extrainfo, 
      double min_reward, double max_reward,
      const std::vector<double>& reward_snapshots);
   void plot_turns_history(std::string output_subdir, std::string extrainfo);
   void plot_frames_history(std::string output_subdir, std::string extrainfo);
   void plot_epsilon_history(std::string output_subdir, std::string extrainfo);
   void plot_Qmap_score_history(std::string output_subdir, 
                                std::string subtitle, std::string extrainfo);
   void plot_log10_loss_history(
      std::string output_subdir, std::string extrainfo);
   void plot_bias_distributions(
      std::string output_subdir, std::string extrainfo);
   void plot_weight_distributions(
      std::string output_subdir, std::string extrainfo);
   void plot_quasirandom_weight_values(
      std::string output_subdir, std::string extrainfo);
   void generate_summary_plots(
      std::string output_subdir, std::string extrainfo);

   void create_snapshots_subdir(std::string output_subdir);
   void export_snapshot(std::string output_subdir);
   void import_snapshot();

// Q learning methods

   bool get_replay_memory_full() const;
   void copy_weights_onto_old_weights();
   int get_random_action() const;
   int get_random_legal_action() const;

   void set_epsilon(double eps);
   double get_epsilon() const;
   void set_min_epsilon(double min_eps);
   void set_epsilon_time_constant(double tau);
   double exponentially_decay_epsilon(double t, double tstart);
   double linearly_decay_epsilon(double t, double tstart, double tstop);

   int select_action_for_curr_state();
   int select_legal_action_for_curr_state();
   void compute_deep_Qvalues();
   void Q_forward_propagate(
      genvector* s_input, bool use_old_weights_flag = false);
   int compute_argmax_Q();
   int compute_legal_argmax_Q();
   int store_curr_state_into_replay_memory(const genvector& curr_s);
   void store_arsprime_into_replay_memory(
      int d, int curr_a, double curr_r,
      const genvector& next_s, bool terminal_state_flag);
   void store_final_arsprime_into_replay_memory(
      int d, int curr_a, double curr_r);
   double update_neural_network(bool verbose_flag = false);
   bool get_replay_memory_entry(
      int d, genvector& curr_s, int& curr_a, double& curr_r,
      genvector& next_s);
   bool store_curr_state_into_eval_memory(const genvector& curr_s);
   void compute_avg_max_eval_Qvalues();

   double compute_target(double curr_r, genvector* next_s, 
                         bool terminal_state_flag);
   double Q_backward_propagate(int d, int Nd, bool verbose_flag = false);

   void set_Q_value(std::string state_action_str, double Qvalue);
   double get_Q_value(std::string state_action_str);
   void init_random_Qmap();
   void print_Qmap();
   void push_back_Qmap_score(double score);
   void push_back_log10_loss(double log10_loss);
   Q_MAP* get_qmap_ptr();
   const Q_MAP* get_qmap_ptr() const;

// Value function learning methods

   void set_n_actions(int n);
   double compute_value(
      genvector* curr_afterstate, bool use_old_weights_flag = false);
   int V_forward_propagate_afterstates(int player_value, double& Vstar);
   int select_legal_action_for_curr_state(int player_value, double& Vstar);
   double compute_target(
      int curr_a, int player_value, double curr_r, bool terminal_state_flag);
   double get_prev_afterstate_curr_value();

  private:

   bool include_bias_terms;
   bool debug_flag;
   int solver_type;
   int n_layers, n_actions, n_weights;
   std::vector<int> layer_dims;
   environment* environment_ptr;
   
   std::deque<double> T_values;  // Holds latest T values
   int batch_size;  	// Perform parameter update after this many episodes
   double base_learning_rate;
   double learning_rate;
   double mu;	  	// Damping coeff for momentum solver type
   double lambda;	// L2 regularization coefficient
   double gamma;	// Discount factor for reward
   double rmsprop_decay_rate; // Decay factor for RMSProp leaky sum of grad**2
   double rmsprop_denom_const;  // const added to denom in RMSProp

   std::vector<genvector*> biases, old_biases;
//	Bias STL vectors are nonzero for layers 1 thru n_layers-1
   std::vector<genvector*> nabla_biases, delta_nabla_biases;

   std::vector<genmatrix*> weights, weights_transpose;
//	Weight STL vectors connect layer pairs {0,1}, {1,2}, ... , 
//      {n_layers-2, n_layers-1}
   std::vector<genmatrix*> old_weights;
   std::vector<genmatrix*> nabla_weights, delta_nabla_weights;
   std::vector<genmatrix*> velocities, prev_velocities;
   std::vector<genmatrix*> adam_m, adam_v;

   std::vector<genvector*> rmsprop_biases_cache;
   std::vector<genvector*> rms_biases_denom;
   std::vector<genmatrix*> rmsprop_weights_cache;
   std::vector<genmatrix*> rms_weights_denom;

// STL vector index ranges over layers l = 0, 1, ..., n_layers
// row index ranges over lth layer nodes j = 0, 1, ... n_nodes_in_lth_layer

// Node weighted inputs:

   std::vector<genvector*> Z_Prime;
   std::vector<genvector*> gammas, betas;  // Batch normalization parameters

// Node activation outputs:

   std::vector<genvector*> A_Prime;    // n_actions x 1
   int hardwired_output_action;

// Node errors:

   std::vector<genvector*> Delta_Prime; // n_actions x 1 
   
// Episode datastructures:

   std::vector<double> time_samples;
   std::vector<double> loss_values;
   std::vector<double> n_episode_turns_frac;
   std::vector<double> n_episode_frames;
   std::vector<double> epsilon_values;
   std::vector<double> Qmap_scores;
   std::vector<double> log10_losses;
   std::vector<double> avg_max_eval_Qvalues;
   std::vector<std::vector<double> > weight_01, weight_05, weight_10;
   std::vector<std::vector<double> > weight_25, weight_35, weight_50;
   std::vector<std::vector<double> > weight_65, weight_75, weight_90;
   std::vector<std::vector<double> > weight_95, weight_99;
   std::vector<std::vector<double> > bias_01, bias_05, bias_10, bias_25;
   std::vector<std::vector<double>  >bias_35, bias_50, bias_65, bias_75;
   std::vector<std::vector<double> > bias_90, bias_95, bias_99;

// Store histories for 9 "quasi-random" weights for each layer within
// following vectors of vectors:

   std::vector<std::vector<double> > weight_1, weight_2, weight_3;
   std::vector<std::vector<double> > weight_4, weight_5, weight_6;
   std::vector<std::vector<double> > weight_7, weight_8, weight_9;

   double reward_sum;
   std::vector<double> running_reward_snapshots;
   std::vector<double> cumulative_reward_snapshots;
   int episode_number;

   std::string snapshots_subdir;

// Q learning variables:

   bool replay_memory_full_flag;
   int replay_memory_capacity;
   int replay_memory_index; // 0 <=replay_memory_index < replay_memory_capacity

   int eval_memory_capacity;
   int eval_memory_index; // 0 <= eval_memory_index < eval_memory_capacity

   int Nd;  // Number of random samples to be drawn from replay memory
   double epsilon;	// Select random action with probability epsilon
   double epsilon_decay_factor;
   double min_epsilon;  // Minimal value for annealed epsilon
   double epsilon_tau; // Exponential time constant for epsilon

   genmatrix *s_eval;  // eval_memory_capacity x Din

   genmatrix *s_curr;  // replay_memory_capacity x Din
   genvector *a_curr;  // replay_memory_capacity x 1 (Holds action indices)
   genvector *r_curr;  // replay_memory_capacity x 1  (Holds rewards)
   genmatrix *s_next;  // replay_memory_capacity x Din
   genvector *terminal_state;   // replay_memory_capacity x 1

   genvector *curr_s_sample, *next_s_sample;  // Din x 1 

   Q_MAP qmap;
   Q_MAP::iterator qmap_iter;

// V learning variables:

   genvector *prev_afterstate_ptr; // Din x 1

// ADAM solver variables:

   double beta1, beta2;
   double curr_beta1_pow, curr_beta2_pow;

   void compute_cumulative_action_dist();
   void update_biases_cache(double decay_rate);
   void update_rmsprop_cache(double decay_rate);

   void allocate_member_objects();
   void initialize_member_objects(const std::vector<int>& n_nodes_per_layer);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void reinforce::set_include_bias_terms(bool flag)
{
   include_bias_terms = flag;
}

inline bool reinforce::get_include_bias_terms() const
{
   return include_bias_terms;
}

inline void reinforce::set_environment(environment* e_ptr)
{
   environment_ptr = e_ptr;
}

inline void reinforce::set_debug_flag(bool flag)
{
   debug_flag = flag;
}

inline bool reinforce::get_debug_flag() const
{
   return debug_flag;
}

inline int reinforce::get_episode_number() const
{
   return episode_number;
}

inline int reinforce::increment_episode_number() 
{
   episode_number++;
   return episode_number;
}

inline void reinforce::append_n_episode_turns_frac(double frac)
{
   n_episode_turns_frac.push_back(frac);
}

inline void reinforce::set_base_learning_rate(double rate)
{
   base_learning_rate = rate;
   learning_rate = rate;
}

inline double reinforce::get_base_learning_rate() const
{
   return base_learning_rate;
}

inline void reinforce::set_learning_rate(double rate)
{
   learning_rate = rate;
}

inline double reinforce::get_learning_rate() const
{
   return learning_rate;
}

inline void reinforce::set_batch_size(double bsize)
{
   batch_size = bsize;
}

inline int reinforce::get_batch_size() const
{
   return batch_size;
}

inline void reinforce::set_lambda(double lambda)
{
   this->lambda = lambda;
}

inline void reinforce::set_Nd(int Nd)
{
   this->Nd = Nd;
}

inline void reinforce::set_gamma(double gamma)
{
   this->gamma=gamma;
}

inline double reinforce::get_gamma() const
{
   return gamma;
}

inline void reinforce::set_rmsprop_decay_rate(double rate)
{
   rmsprop_decay_rate = rate;
}

inline reinforce::Q_MAP* reinforce::get_qmap_ptr()
{
   return &qmap;
}

inline const reinforce::Q_MAP* reinforce::get_qmap_ptr() const
{
   return &qmap;
}

inline void reinforce::push_back_Qmap_score(double score)
{
   Qmap_scores.push_back(score);
}

inline void reinforce::push_back_log10_loss(double log10_loss)
{
   log10_losses.push_back(log10_loss);
}

inline bool reinforce::get_replay_memory_full() const
{
   return replay_memory_full_flag;
}

inline void reinforce::set_ADAM_params(double beta1, double beta2)
{
   this->beta1 = beta1;
   this->beta2 = beta2;
}


#endif  // reinforce.h


