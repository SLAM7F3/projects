// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 10/4/16; 10/5/16; 10/11/16
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

   void clear_matrices();
   void print_matrices();
   void xavier_init_weight_matrices();
   void compute_discounted_rewards();
   void policy_forward();
   void policy_backward();

   void initialize_episode();
   void process_timestep(
      genvector* input_state_ptr, double curr_reward, 
      bool episode_finished_flag);

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
   
   genmatrix *W1_ptr;        // H x Din
   genmatrix *dW1_ptr;	     // H x Din
   genmatrix *W2_ptr;        // Dout x H
   genmatrix *dW2_ptr;       // Dout x H
   genmatrix *grad1_ptr;     // H x Din
   genmatrix *grad2_ptr;     // Dout x H
   genmatrix *rmsprop1_ptr;  // H x Din
   genmatrix *rmsprop2_ptr;  // Dout x H

   genvector *x_ptr;	     // Din x 1
   genvector *h_ptr;         // H x 1
   genvector *logp_ptr;      // Dout x 1
   genvector *dlogp_ptr;     // Dout x 1
   genvector *p_ptr;         // Dout x 1
   genvector *action_ptr;    // Dout x 1

   genmatrix *episode_x_ptr;   	// T x Din
   genmatrix *episode_h_ptr;    // T x H
   genmatrix *episode_dlogp_ptr;// T x Dout

   std::vector<double> rewards, discounted_rewards;
   double running_reward;
   double reward_sum;
   int episode_number;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

/*
inline void reinforce::set_n_zlevels(int n)
{
   n_zlevels = n;
}
*/


#endif  // reinforce.h


