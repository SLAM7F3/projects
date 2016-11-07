// ==========================================================================
// Program QLEARN1 implements the "painless Q learning tutorial"
// presented in
// http://mnemstudio.org/path-finding-q-learning-tutorial.htm.  See
// also "Technical Note: Q Learning by Watkins and Dayan, Machine
// Learning, 8, 279-292 (1992).
// ==========================================================================
// Last updated on 11/7/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/genmatrix.h"
#include "math/mathfuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   double gamma = 0.8;
   cout << "Enter discounter factor gamma:" << endl;
   cin >> gamma;

   double alpha = 1.0;
   cout << "Enter learning rate alpha:" << endl;
   cin >> alpha;

   int goal_state = 5;

// Environment reward matrix:  row = state; column = action

   int n_states = 6;
   int n_actions = n_states;
   
   genmatrix Q(n_states, n_actions);
   genmatrix R(n_states, n_actions);

// Set fixed reward matrix values:

   R.initialize_values(-1);
   R.put(0,4,0);
   R.put(1,3,0);
   R.put(2,3,0);
   R.put(3,1,0);
   R.put(3,2,0); 
   R.put(3,4,0);
   R.put(4,0,0);
   R.put(4,3,0);
   R.put(5,1,0);
   R.put(5,4,0);

   R.put(1,5,100);
   R.put(4,5,100);
   R.put(5,5,100);
   cout << "R = " << R << endl;

// Initialize Q matrix to zero:

   Q.clear_values();

   int n_episodes = 1000;
   for(int n = 0; n < n_episodes; n++)
   {
      int curr_state = mathfunc::getRandomInteger(n_states);

      do
      {
         int curr_action = -1;
         bool legal_action = false;
         do
         {
            curr_action = mathfunc::getRandomInteger(n_actions);
            if(R.get(curr_state, curr_action) >= 0)
            {
               legal_action = true;
            }
         }
         while(!legal_action);
   
         int next_state = curr_action;
         
// Retrieve row from Q corresponding to next state.  Then compute max
// value within this row over all actions:

         double max_Q = NEGATIVEINFINITY;
         for(int a = 0; a < n_actions; a++)
         {
            double curr_Q = Q.get(next_state, a);
            max_Q = basic_math::max(curr_Q, max_Q);
         }

         double old_q = Q.get(curr_state, curr_action);
         double new_q = R.get(curr_state, curr_action) + gamma * max_Q;
         double avg_q = (1 - alpha) * old_q + alpha * new_q;
         Q.put(curr_state, curr_action, avg_q);

         curr_state = next_state;
      }
      while(curr_state != goal_state);
   } // loop over index n labeling episodes

// Renormalize Q entries so that maximum value = 1:

   double max_q_entry = Q.max_element_value();
   Q = Q / max_q_entry;
   cout << "Renormalized Q = " << Q << endl;
}

