// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 11/4/16; 11/8/16; 11/9/16; 11/10/16
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

// Initialization, constructor and destructor functions:

   reinforce(const std::vector<int>& n_nodes_per_layer, int Tmax);
   reinforce();
   ~reinforce();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const reinforce& R);

   void set_environment(environment* e_ptr);
   void set_debug_flag(bool flag);
   bool get_debug_flag() const;
   int get_curr_timestep() const;
   int get_episode_number() const;
   int increment_episode_number();
   void append_n_episode_turns_frac(double curr_n_turns_frac);
   void set_base_learning_rate(double rate);
   void set_learning_rate(double rate);
   double get_learning_rate() const;
   void set_batch_size(double bsize);
   int get_batch_size() const;
   void set_lambda(double lambda);
   void set_gamma(double gamma);
   void set_rmsprop_decay_rate(double rate);
   genvector* get_p_action();

   void hardwire_output_action(int a);

   void initialize_episode();
   void compute_action_probs(
      genvector* x_input, bool enforce_constraints_flag,
      genvector* legal_actions = NULL);
   void renormalize_action_distribution();

   void redistribute_action_probs();
   void print_p_action() const;
   int get_candidate_current_action();
   void set_current_action(int output_action);
   void periodically_snapshot_loss_value();
   void snapshot_running_reward();
   void record_reward_for_action(double curr_reward);
   void update_weights();
   void update_running_reward(std::string extrainfo);

// Monitoring network training methods:

   void print_weights();
   void compute_weight_distributions();
   std::string init_subtitle();
   void plot_loss_history(std::string extrainfo);
   void plot_reward_history(
      std::string extrainfo, double min_reward, double max_reward);
   void plot_turns_history(std::string extrainfo);

   void create_snapshots_subdir();
   void export_snapshot();
   void import_snapshot();

// Q learning methods

   void initialize_replay_memory();
   int get_random_legal_action() const;
   double anneal_epsilon();
   int select_legal_action_for_curr_state();

   void Q_forward_propagate(genvector& s_input);
   int compute_argmax_Q();

   int store_curr_state_into_replay_memory(const genvector& curr_s);
   void store_arsprime_into_replay_memory(
      int d, int curr_a, double curr_r,
      const genvector& next_s, bool terminal_state_flag);

   void update_Q_network();

   bool get_memory_replay_entry(
      int d, genvector& curr_s, int& curr_a, double& curr_r,
      genvector& next_s);
   double max_Q(genvector& next_s);
   double compute_target(int d);

  private:

   bool debug_flag;
   int n_layers, n_actions;
   std::vector<int> layer_dims;
   environment* environment_ptr;
   
   int cum_time_counter;  // Cumulative time step counter
   int curr_timestep;
   int T;		// number of time steps in current episode
   std::deque<double> T_values;  // Holds latest T values
   int Tmax;  	        
   int batch_size;  	// Perform parameter update after this many episodes
   double base_learning_rate;
   double learning_rate;
   double lambda;	// L2 regularization coefficient
   double gamma;	// Discount factor for reward
   double rmsprop_decay_rate; // Decay factor for RMSProp leaky sum of grad**2

   std::vector<genmatrix*> weights, weights_transpose;
   std::vector<genmatrix*> nabla_weights, delta_nabla_weights;
//	Weight STL vectors connect layer pairs {0,1}, {1,2}, ... , 
//      {n_layers-2, n_layers-1}

   std::vector<genmatrix*> rmsprop_weights_cache;
   std::vector<genmatrix*> rms_denom;

// STL vector index ranges over layers l = 0, 1, ..., n_layers
// row index ranges over lth layer nodes j = 0, 1, ... n_nodes_in_lth_layer
// column index ranges over t = 0, 1, ... T

// Node weighted inputs:

   std::vector<genmatrix*> z;

// Node activation outputs:
   std::vector<genmatrix*> a;
   int hardwired_output_action;

// Node errors:
   std::vector<genmatrix*> delta_prime; // n_actions x T

   genvector *x_input;          // Din x 1
   genvector *p_action;		// n_actions x 1  
   genvector *pcum_action;	// n_actions x 1  

// Episode datastructures:

   std::vector<double> time_samples;
   std::vector<double> loss_values;
   std::vector<double> n_episode_turns_frac;

   genvector *y; // T x 1 (holds index for action taken at t = 1, 2, ... T)
   genvector *reward;  // T x 1
   genvector *discounted_reward;  // T x 1
   double running_reward;
   double reward_sum;
   std::vector<double> running_reward_snapshots;
   int episode_number;

   std::string snapshots_subdir;

// Q learning variables:

   int replay_memory_index;  // 0 <= replay_memory_index < Tmax
   double epsilon;	// Select random action with probability epsilon
   genmatrix *s_curr;  // T x Din
   genvector *a_curr;  // T x 1  (Holds indices for actions)
   genvector *r_curr;  // T x 1  (Holds rewards)
   genmatrix *s_next;  // T x Din
   genvector *terminal_state;   // T x 1

   genvector *curr_s_sample, *next_s_sample;  // Din x 1 

   void policy_forward(int t, bool enforce_constraints_flag,
      genvector *legal_actions = NULL);
   void get_softmax_action_probs(int t);
   void compute_cumulative_action_dist();
   double compute_cross_entropy_loss(int t);
   void discount_rewards();
   void policy_backward();

   void allocate_member_objects();
   void initialize_member_objects(const std::vector<int>& n_nodes_per_layer);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

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

inline int reinforce::get_curr_timestep() const
{
   return curr_timestep;
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

inline void reinforce::set_gamma(double gamma)
{
   this->gamma=gamma;
}

inline void reinforce::set_rmsprop_decay_rate(double rate)
{
   rmsprop_decay_rate = rate;
}

inline genvector* reinforce::get_p_action()
{
   return p_action;
}


#endif  // reinforce.h


