// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 10/17/16; 10/18/16; 10/19/16; 10/22/16
// ==========================================================================

#ifndef REINFORCE_H
#define REINFORCE_H

#include <deque>
#include <map>
#include <iostream>
#include <vector>

class genmatrix;
class genvector;

class reinforce
{
   
  public:

// Initialization, constructor and destructor functions:

   reinforce(const std::vector<int>& n_nodes_per_layer, int Tmax);
   ~reinforce();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const reinforce& R);

   int get_episode_number() const;
   void increment_episode_number();
   void append_n_episode_turns_frac(double curr_n_turns_frac);
   void set_learning_rate(double rate);

   void initialize_episode();
   int compute_current_action(genvector* input_state_ptr);
   void record_reward_for_action(double curr_reward);
   void update_weights(bool episode_finished_flag);
   void update_running_reward();

   void print_weights();
   void compute_weight_distributions();
   void plot_loss_history(std::string extrainfo);
   void plot_reward_history(std::string extrainfo);
   void plot_turns_history(std::string extrainfo);

  private:

   int n_layers, n_actions;
   std::vector<int> layer_dims;

   int cum_time_counter;  // Cumulative time step counter
   int curr_timestep;
   int T;		// number of time steps in current episode
   std::deque<double> T_values;  // Holds latest T values
   int Tmax;
   int batch_size;  	// Perform parameter update after this many episodes
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

// Node errors:
   std::vector<genmatrix*> delta_prime;

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

   void policy_forward(int t, genvector& x_input);
   void get_softmax_action_probs(int t) const;
   double compute_loss(int t) const;
   void discount_rewards();
   void policy_backward();

   void allocate_member_objects();
   void initialize_member_objects(const std::vector<int>& n_nodes_per_layer);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int reinforce::get_episode_number() const
{
   return episode_number;
}

inline void reinforce::increment_episode_number() 
{
   episode_number++;
}

inline void reinforce::append_n_episode_turns_frac(double frac)
{
   n_episode_turns_frac.push_back(frac);
}

inline void reinforce::set_learning_rate(double rate)
{
   learning_rate = rate;
}

#endif  // reinforce.h


