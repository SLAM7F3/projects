// ==========================================================================
// Header file for reinforce class 
// ==========================================================================
// Last modified on 10/4/16
// ==========================================================================

#ifndef REINFORCE_H
#define REINFORCE_H

#include <map>
#include <iostream>
#include <vector>

class genmatrix;

class reinforce
{
   
  public:

// Initialization, constructor and destructor functions:

   reinforce();
   reinforce(const reinforce& R);
   ~reinforce();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const reinforce& R);

   void xavier_init_weight_matrices();

  private: 

   int Din;		// Input dimensionality 
   int Dout;		// Output dimensionality 
   int H;   		// Number of hidden layer neurons
   int batch_size;  	// Perform parameter update after this many episodes
   double learning_rate;
   double gamma;	// Discount factor for reward
   
   genmatrix *W1_ptr, *W2_ptr;
   genmatrix *grad1_ptr, *grad2_ptr;
   genmatrix *rmsprop1_ptr, *rmsprop2_ptr;

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


