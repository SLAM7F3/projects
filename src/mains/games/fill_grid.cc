// ==========================================================================
// Program FILL_GRID
// ==========================================================================
// Last updated on 10/12/16; 10/18/16
// ==========================================================================

#include <iostream>
#include <string>
#include <unistd.h>     // Needed for sleep() and usleep()
#include <vector>
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "machine_learning/reinforce.h"
#include "games/tictac3d.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();

//   int nsize = 3;
   int nsize = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize,1);

   int Din = nsize * nsize;	// Input dimensionality
   int H = 100;			// Number of hidden layer neurons
   int Dout = nsize * nsize;	// Output dimensionality
   int Tmax = 64;

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H);
   layer_dims.push_back(Dout);
   reinforce* reinforce_ptr = new reinforce(layer_dims, Tmax);

   int n_max_episodes = 1 * 1000000;
   int n_losses = 0;
   int n_wins = 0;

   while(reinforce_ptr->get_episode_number() < n_max_episodes)
   {
      ttt_ptr->reset_board_state();
      reinforce_ptr->initialize_episode();

      while(true)
      {
         ttt_ptr->get_random_legal_AI_move();
//         ttt_ptr->display_board_state();
         if(ttt_ptr->get_game_over()) break;
//       usleep(250 * 1000);

         int output_action = reinforce_ptr->compute_current_action(
            ttt_ptr->get_board_state_ptr());
         cout << "output_action = " << output_action << endl;

// Step the environment and then retrieve new reward measurements:

         bool print_flag = false;
//         bool print_flag = true;
         ttt_ptr->get_random_agent_move(print_flag);
//         ttt_ptr->display_board_state();
         if(ttt_ptr->get_game_over()) break;
      } // !game_over while loop

      
      if(ttt_ptr->get_score() == -1)
      {
         n_losses++;
      }
      else
      {
         n_wins++;
      }

//      ttt_ptr->display_board_state();
//      cout << "*board_state_ptr = " 
//           << *(ttt_ptr->get_board_state_ptr()) << endl;
//      outputfunc::enter_continue_char();
      
//      cout << "Final score = " << ttt_ptr->get_score() << endl;
//      cout << "GAME OVER" << endl << endl;


      reinforce_ptr->increment_episode_number();
      int n_episodes = reinforce_ptr->get_episode_number();
      if(n_episodes % 100 == 0)
      {
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_losses = " << n_losses
              << " n_wins = " << n_wins
              << " win_frac = " << win_frac
              << endl;
      }

   } // n_episodes < n_max_episodes while loop

   int n_episodes = reinforce_ptr->get_episode_number();
   double win_frac = double(n_wins) / n_episodes;
   cout << "n_episodes = " << n_episodes 
        << " n_losses = " << n_losses
        << " n_wins = " << n_wins
        << " win_frac = " << win_frac
        << endl;

   delete ttt_ptr;
   delete reinforce_ptr;
}



