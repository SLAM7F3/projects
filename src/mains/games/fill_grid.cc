// ==========================================================================
// Program FILL_GRID
// ==========================================================================
// Last updated on 10/12/16
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
//   tictac3d* ttt_ptr = new tictac3d(2,1);
   tictac3d* ttt_ptr = new tictac3d(3,1);
//    tictac3d* ttt_ptr = new tictac3d(4,1);

   reinforce* reinforce_ptr = new reinforce();
   cout << "*reinforce_ptr = " << *reinforce_ptr << endl;
   reinforce_ptr->initialize_episode();

   int n_max_episodes = 1 * 1000000;
//   int n_max_episodes = 10 * 1000000;
   int n_episodes = 0;
   int n_losses = 0;
   int n_wins = 0;

   while(n_episodes < n_max_episodes)
   {
      ttt_ptr->reset_board_state();

      while(true)
      {
//         ttt_ptr->display_board_state();
         ttt_ptr->get_random_legal_AI_move();
         if(ttt_ptr->get_game_over()) break;
//         ttt_ptr->display_board_state();
//       usleep(250 * 1000);

         bool print_flag = false;
//         bool print_flag = true;
         ttt_ptr->get_random_agent_move(print_flag);
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
      
//      cout << "Final score = " << ttt_ptr->get_score() << endl;
//      cout << "GAME OVER" << endl << endl;
      n_episodes++;
      if(n_episodes % 100 == 0)
      {
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_losses = " << n_losses
              << " n_wins = " << n_wins
              << " win_frac = " << win_frac
              << endl;
      }

//      outputfunc::enter_continue_char();
   } // infinite while loop

   double win_frac = double(n_wins) / n_episodes;
   cout << "n_episodes = " << n_episodes 
        << " n_losses = " << n_losses
        << " n_wins = " << n_wins
        << " win_frac = " << win_frac
        << endl;

   delete ttt_ptr;
   delete reinforce_ptr;
}



