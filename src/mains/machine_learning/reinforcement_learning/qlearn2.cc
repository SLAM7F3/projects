// ==========================================================================
// Program QLEARN2 simulates 16 states for a 2x2 maze.  It computes
// Q(s,a) as a lookup table for this simple game.
// ==========================================================================
// Last updated on 11/8/16; 11/10/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/genmatrix.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

//   nrfunc::init_time_based_seed();

   double gamma = 0.8;
//   cout << "Enter discounter factor gamma:" << endl;
//   cin >> gamma;

   double alpha = 1.0;
//   cout << "Enter learning rate alpha:" << endl;
//   cin >> alpha;

// Environment reward matrix:  row = state; column = action

   int n_states = 16;
   int n_actions = 4;
   
// Action:  0 --> up, 1 --> right, 2 --> down, 3 --> left

   genmatrix R(n_states, n_actions);
   genmatrix Q(n_states, n_actions);

   vector<int> starting_states, winning_states;
   
   starting_states.push_back(0);
   starting_states.push_back(4);
   starting_states.push_back(8);
   starting_states.push_back(12);

   winning_states.push_back(2);
   winning_states.push_back(6);
   winning_states.push_back(10);
   winning_states.push_back(14);

   genmatrix NextState(n_states, n_actions);
   
// Set fixed reward matrix values:

   R.initialize_values(-1);
   NextState.initialize_values(-1);

   R.put(0, 2, 0);
   R.put(1, 2, 1);
   R.put(3, 0, 0);
   R.put(3, 1, 1);

// Action:  0 --> up, 1 --> right, 2 --> down, 3 --> left

   NextState.put(0, 2, 3);
   NextState.put(1, 2, 2);
   NextState.put(3, 0, 0);
   NextState.put(3, 1, 2);

   R.put(4, 1, 0);
   R.put(4, 2, 0);
   R.put(5, 3, 0);
   R.put(7, 0, 0);
   R.put(7, 1, 1);

   NextState.put(4, 1, 5);
   NextState.put(4, 2, 7);
   NextState.put(5, 3, 4);
   NextState.put(7, 0, 4);
   NextState.put(7, 1, 6);

   R.put(8, 1, 0);
   R.put(8, 2, 0);
   R.put(9, 3, 0);
   R.put(9, 2, 1);
   R.put(11, 0, 0);

// Action:  0 --> up, 1 --> right, 2 --> down, 3 --> left

   NextState.put(8, 1, 9);
   NextState.put(8, 2, 11);
   NextState.put(9, 3, 8);
   NextState.put(9, 2, 10);
   NextState.put(11, 0, 8);

   R.put(12, 1, 0);
   R.put(13, 3, 0);   
   R.put(13, 2, 1);   
   R.put(15, 1, 1);   
   
   NextState.put(12, 1, 13);
   NextState.put(13, 3, 12);
   NextState.put(13, 2, 14);
   NextState.put(15, 1, 14);

   cout << "R = " << R << endl;

// Initialize Q matrix to zero:
//   Q.clear_values();

// Initialize Q matrix to random values ranging over interval [-1,1]:

   for(unsigned int i = 0; i < Q.get_mdim(); i++)
   {
      for(unsigned int j = 0; j < Q.get_ndim(); j++)
      {
         Q.put(i, j, 2 * (nrfunc::ran1() - 0.5) );
      }
   }
   cout << "Initial random Q = " << Q << endl;

   int n_episodes = 5;
//   int n_episodes = 10000;
   cout << "Enter number of episodes to iterate over:" << endl;
   cin >> n_episodes;

   for(int n = 0; n < n_episodes; n++)
   {
      int curr_state = -1;
      bool init_OK_state = false;
      while(!init_OK_state)
      {
         curr_state = mathfunc::getRandomInteger(n_states);

         init_OK_state = true;
         for(unsigned int g = 0; g < winning_states.size(); g++)
         {
            if(curr_state == winning_states[g])
            {
               init_OK_state = false;
               break;
            }
         }
      }

      cout << "************  Start of Game " << n
           << " ***********" << endl;

      bool game_over = false;
      do
      {
         int curr_action = mathfunc::getRandomInteger(n_actions);
         cout << "curr_action = " << curr_action << endl;
         int next_state = NextState.get(curr_state, curr_action);
//         cout << "next_state = " << next_state << endl;

         double max_Q = 0;
         if(next_state >= 0)
         {

// Retrieve row from Q corresponding to next state.  Then compute max
// value within this row over all actions:

            max_Q = NEGATIVEINFINITY;
            for(int a = 0; a < n_actions; a++)
            {
               double curr_Q = Q.get(next_state, a);
               max_Q = basic_math::max(curr_Q, max_Q);
            }

         }

         double reward = R.get(curr_state, curr_action);
         cout << "reward = " << reward << " max_Q = " << max_Q << endl;

         double old_q = Q.get(curr_state, curr_action);
         double new_q = reward + gamma * max_Q;
         double avg_q = (1 - alpha) * old_q + alpha * new_q;
         Q.put(curr_state, curr_action, avg_q);

         cout << "old_q = " << old_q << " new_q = " << new_q 
              << " avg_q = " << avg_q << endl;

         curr_state = next_state;
         for(unsigned int g = 0; g < winning_states.size(); g++)
         {
            if(curr_state < 0 || curr_state == winning_states[g])
            {
               game_over = true;
               break;
            }
         }
      }
      while(!game_over);
      
   } // loop over index n labeling episodes

   cout << "Q = " << Q << endl;

// Renormalize Q entries so that maximum value = 1:

   double max_q_entry = Q.max_element_value();
   Q = Q / max_q_entry;
   cout << "Renormalized Q = " << Q << endl;
}


