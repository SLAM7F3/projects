// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 10/4/16; 10/5/16; 10/11/16; 10/12/16
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

   reinforce();
   reinforce(const reinforce& R);
   ~reinforce();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const reinforce& R);

   int get_episode_number() const;
   void increment_episode_number();

   void print_matrices();

   void initialize_episode();
   void compute_current_action(
      genvector* input_state_ptr, genvector* output_action_ptr);
   void record_reward_for_action(double curr_reward);
   void update_weights(bool episode_finished_flag);

  private:

   int H;   		// Number of hidden layer neurons
   int Din;		// Input dimensionality 
   int Dout;		// Output dimensionality 
   int curr_timestep;
   int T;		// number of time steps in current episode
   int Tmax;
   int batch_size;  	// Perform parameter update after this many episodes
   double learning_rate;
   double gamma;	// Discount factor for reward
   double decay_rate;	// Decay factor for RMSProp leaky sum of grad**2

   genmatrix *W1_ptr;        // H x Din
   genmatrix *dW1_ptr;	     // H x Din
   genmatrix *dW1_buffer_ptr;// H x Din  Update buffer for grad sums over batch
   genmatrix *sqr_dW1_buffer_ptr;   // H x Din
   genmatrix *rmsprop1_ptr;  // H x Din
   genmatrix *sqrt_rmsprop1_ptr;  // H x Din

   genmatrix *W2_ptr;        // Dout x H
   genmatrix *dW2_ptr;       // Dout x H
   genmatrix *dW2_buffer_ptr;// Dout x H Update buffer for grad sums over batch
   genmatrix *sqr_dW2_buffer_ptr;   // Dout x H
   genmatrix *rmsprop2_ptr;  // Dout x H
   genmatrix *sqrt_rmsprop2_ptr;  // Dout x H

   genvector *h_ptr;         // H x 1
   genvector *logp_ptr;      // Dout x 1
   genvector *dlogp_ptr;     // Dout x 1
   genvector *p_ptr;         // Dout x 1  Prob of taking action
   genvector *pcum_ptr;      // Dout x 1  Cumulative prob of taking action
   genvector *action_ptr;    // Dout x 1  Just pointer to pre-existing output

   genmatrix *episode_x_ptr;   	// T x Din
   genmatrix *episode_h_ptr;    // T x H
   genmatrix *episode_dlogp_ptr;// T x Dout
   genvector *episode_reward_ptr;  // T x 1
   genvector *discounted_episode_reward_ptr;  // T x 1

   double running_reward;
   double reward_sum;
   int episode_number;

   void xavier_init_weight_matrices();
   void discount_rewards();
   void policy_forward(genvector* x_ptr);
   void policy_backward();


   void allocate_member_objects();
   void initialize_member_objects();
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


