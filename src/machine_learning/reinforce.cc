// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/4/16; 10/5/16; 10/11/16
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
   Tmax = 1000;	 	// Maximum number of time steps per episode

   batch_size = 10;	// Perform parameter update after this many episodes
   learning_rate = 0.001;	
   gamma = 0.99;	// Discount factor for reward

   running_reward = 0;
   reward_sum = 0;
   episode_number = 0;
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
   dlogp_ptr = new genvector(Dout);
   p_ptr = new genvector(Dout);
   action_ptr = new genvector(Dout);

   episode_x_ptr = new genmatrix(Tmax, Din);
   episode_h_ptr = new genmatrix(Tmax, H);
   episode_dlogp_ptr = new genmatrix(Tmax, Dout);
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
   delete dW1_ptr;
   delete W2_ptr;
   delete dW2_ptr;
   delete grad1_ptr;
   delete grad2_ptr;
   delete rmsprop1_ptr;
   delete rmsprop2_ptr;

//    delete x_ptr;
   delete h_ptr;
   delete logp_ptr;
   delete dlogp_ptr;
   delete p_ptr;
   delete action_ptr;

   delete episode_x_ptr;
   delete episode_h_ptr;
   delete episode_dlogp_ptr;
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
void reinforce::policy_forward()
{
   *h_ptr = (*W1_ptr) * (*x_ptr);
   
// Apply ReLU thresholding to hidden state activations:

   for(int i = 0; i < H; i++)
   {
      if(h_ptr->get(i) < 0) h_ptr->put(i,0);
   }
   
   *logp_ptr = (*W2_ptr) * (*h_ptr);
   machinelearning_func::sigmoid(*logp_ptr, *p_ptr);
}

// ---------------------------------------------------------------------
void reinforce::policy_backward()
{
   *dW2_ptr =  episode_dlogp_ptr->transpose() * (*episode_h_ptr);

   genmatrix *dh_ptr = new genmatrix(H,T);
   *dh_ptr = W2_ptr->transpose() * episode_dlogp_ptr->transpose();

// Apply ReLU thresholding::

   for(int i = 0; i < H; i++)
   {
      if(dh_ptr->get(i) < 0) dh_ptr->put(i,0);
   }

   *dW1_ptr = (*dh_ptr) * (*episode_x_ptr);

   delete dh_ptr;
}

// ---------------------------------------------------------------------
void reinforce::initialize_episode()
{
   episode_x_ptr->clear_values();
   episode_h_ptr->clear_values();
   episode_dlogp_ptr->clear_values();

   curr_timestep = 0;
}

// ---------------------------------------------------------------------
void reinforce::process_timestep(
   genvector* input_state_ptr, double curr_reward, bool episode_finished_flag)
{
   x_ptr = input_state_ptr;
   policy_forward();
   
   for(int d = 0; d < Dout; d++)
   {
      double action_prob = p_ptr->get(d);
      double y;	 // fake label
      if(nrfunc::ran1() < action_prob)
      {
         action_ptr->put(d, 1);
         y = 1;
      }
      else
      {
         action_ptr->put(d, 0);
         y = 0;
      }
      dlogp_ptr->put(d, y - action_prob);
   } // loop over index d 
          
   episode_x_ptr->put_row(curr_timestep, *input_state_ptr);
   episode_h_ptr->put_row(curr_timestep, *h_ptr);
   episode_dlogp_ptr->put_row(curr_timestep, *dlogp_ptr);
   
   reward_sum += curr_reward;

   if (episode_finished_flag)
   {
   } // episode_finished_flag
   

}
