// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 10/4/16; 10/5/16
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
   void policy_forward(genvector* in_state_ptr);

  private:

   int H;   		// Number of hidden layer neurons
   int Din;		// Input dimensionality 
   int Dout;		// Output dimensionality 
   int batch_size;  	// Perform parameter update after this many episodes
   double learning_rate;
   double gamma;	// Discount factor for reward
   
   genmatrix *W1_ptr;        // H x Din
   genmatrix *W2_ptr;        // Dout x H
   genmatrix *grad1_ptr;     // H x Din
   genmatrix *grad2_ptr;     // Dout x H
   genmatrix *rmsprop1_ptr;  // H x Din
   genmatrix *rmsprop2_ptr;  // Dout x H

   genvector *h_ptr;         // H x 1
   genvector *logp_ptr;      // Dout x 1
   genvector *p_ptr;         // Dout x 1

   std::vector<double> rewards, discounted_rewards;

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


