// ==========================================================================
// Program 3D_TTT
// ==========================================================================
// Last updated on 10/4/16; 10/5/16; 10/18/16; 10/27/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "machine_learning/reinforce.h"
#include "games/tictac3d.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();

   int nsize = 4;
//   int n_zlevels = 1;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);

//   reinforce* reinforce_ptr = new reinforce(layer_dims, Tmax);
//   reinforce_ptr->initialize_episode();

   while(!ttt_ptr->get_game_over())
   {

// Human move:

      ttt_ptr->display_board_state();

      int human_value = -1;
      ttt_ptr->enter_player_move(human_value);
      ttt_ptr->check_player_win(human_value);
      
// Agent move:

      int agent_value = 1;
      ttt_ptr->display_board_state();
      ttt_ptr->get_random_legal_player_move(agent_value);
      ttt_ptr->check_player_win(agent_value);

   } // !game_over while loop
   cout << "Final score = " << ttt_ptr->get_score() << endl;
   cout << "GAME OVER" << endl << endl;

   delete ttt_ptr;
//    delete reinforce_ptr;
}



