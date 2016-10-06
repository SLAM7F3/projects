// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/4/16; 10/5/16
// ==========================================================================

#include <string>
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "numrec/nrfuncs.h"
#include "machine_learning/reinforce.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void reinforce::initialize_member_objects()
{
   Din = 4 * 4;		// Input dimensionality
   Dout = 4 * 4;	// Output dimensionality
   H = 200;		// Number of hidden layer neurons

   batch_size = 10;	// Perform parameter update after this many episodes
   learning_rate = 0.001;	
   gamma = 0.99;	// Discount factor for reward
}		       

void reinforce::allocate_member_objects()
{
   W1_ptr = new genmatrix(H, Din);
   W2_ptr = new genmatrix(Dout, H);
   grad1_ptr = new genmatrix(H, Din);
   grad2_ptr = new genmatrix(Dout, H);
   rmsprop1_ptr = new genmatrix(H, Din);
   rmsprop2_ptr = new genmatrix(Dout, H);

   h_ptr = new genvector(H);
   logp_ptr = new genvector(Dout);
   p_ptr = new genvector(Dout);
}		       

void reinforce::clear_matrices()
{
   W1_ptr->clear_matrix_values();
   W2_ptr->clear_matrix_values();
   grad1_ptr->clear_matrix_values();
   grad2_ptr->clear_matrix_values();
   rmsprop1_ptr->clear_matrix_values();
   rmsprop2_ptr->clear_matrix_values();
}		       

void reinforce::print_matrices()
{
   cout << "*W1_ptr = " << *W1_ptr << endl;
   cout << "*W2_ptr = " << *W2_ptr << endl;

   cout << "*grad1_ptr = " << *grad1_ptr << endl;
   cout << "*grad2_ptr = " << *grad2_ptr << endl;

   cout << "*rmsprop1_ptr = " << *rmsprop1_ptr << endl;
   cout << "*rmsprop2_ptr = " << *rmsprop2_ptr << endl;
}		       

// ---------------------------------------------------------------------
reinforce::reinforce()
{
   initialize_member_objects();
   allocate_member_objects();
   clear_matrices();
}

// Copy constructor:

reinforce::reinforce(const reinforce& R)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
reinforce::~reinforce()
{
   delete W1_ptr;
   delete W2_ptr;
   delete grad1_ptr;
   delete grad2_ptr;
   delete rmsprop1_ptr;
   delete rmsprop2_ptr;

   delete h_ptr;
   delete logp_ptr;
   delete p_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const reinforce& R)
{
   outstream << endl;
   outstream << "Din = " << R.Din << " Dout = " << R.Dout << endl;
   outstream << "H = " << R.H << endl;
   outstream << "batch_size = " << R.batch_size << " learning_rate = "
             << R.learning_rate << endl;
   outstream << "gamma = " << R.gamma << endl;
   return outstream;
}

// ==========================================================================

void reinforce::xavier_init_weight_matrices()
{
   for(unsigned int py = 0; py < W1_ptr->get_ndim(); py++)
   {
      for(unsigned int px = 0; px < W1_ptr->get_mdim(); px++)
      {
         W1_ptr->put(px,py,nrfunc::gasdev() / sqrt(Din) );
      } // loop over px
   } // loop over py 

   for(unsigned int py = 0; py < W2_ptr->get_ndim(); py++)
   {
      for(unsigned int px = 0; px < W2_ptr->get_mdim(); px++)
      {
         W2_ptr->put(px,py,nrfunc::gasdev() / sqrt(H) );
      } // loop over px
   } // loop over py 

//   cout << "W1 = " << *W1_ptr << endl;
//   cout << "W2 = " << *W2_ptr << endl;
}

// ---------------------------------------------------------------------
void reinforce::compute_discounted_rewards()
{
   double running_add = 0;
   int nsteps = rewards.size();

   for(int t = nsteps - 1; t >= 0; t--)
   {
      running_add = running_add * gamma + rewards[t];
      discounted_rewards.push_back(running_add);
   }
}

// ---------------------------------------------------------------------
void reinforce::policy_forward(genvector* in_state_ptr)
{
   *h_ptr = (*W1_ptr) * (*in_state_ptr);
   
// Apply ReLU thresholding to hidden state activations:

   for(int i = 0; i < H; i++)
   {
      if(h_ptr->get(i) < 0) h_ptr->put(i,0);
   }
   
   *logp_ptr = (*W2_ptr) * (*h_ptr);
   machinelearning_func::sigmoid(*logp_ptr, *p_ptr);
}

// ---------------------------------------------------------------------
// eph is an array of intermediate hidden states

/*
void reinforce::policy_backward(genvector* eph, genvector* epdlogp)
{
//   dW2 = eph.transpose dotproduct epdlogp


}
*/