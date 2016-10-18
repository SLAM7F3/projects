// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 10/11/16; 10/12/16; 10/17/16; 10/18/16
// ==========================================================================

#ifndef REINFORCE_H
#define REINFORCE_H

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

   void initialize_episode();
   int compute_current_action(genvector* input_state_ptr);
   void record_reward_for_action(double curr_reward);
   void update_weights(bool episode_finished_flag);

  private:

   int n_layers, n_actions;
   std::vector<int> layer_dims;

   int curr_timestep;
   int T;		// number of time steps in current episode
   int Tmax;
   int batch_size;  	// Perform parameter update after this many episodes
   double learning_rate;
   double lambda;	// L2 regularization coefficient
   double gamma;	// Discount factor for reward
   double rmsprop_decay_rate; // Decay factor for RMSProp leaky sum of grad**2

   std::vector<genmatrix*> weights, nabla_weights, delta_nabla_weights;
//	Weight STL vectors connect layer pairs {0,1}, {1,2}, ... , 
//      {n_layers-2, n_layers-1}

   std::vector<genmatrix*> rmsprop_weights_cache;

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

   genvector *y; // T x 1 (holds index for action taken at t = 1, 2, ... T)
   genvector *reward;  // T x 1
   genvector *discounted_reward;  // T x 1
   double running_reward;
   double reward_sum;
   int episode_number;

   void policy_forward(int t, genvector& x_input);
   void get_softmax_action_probs(int t) const;
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

#endif  // reinforce.h


