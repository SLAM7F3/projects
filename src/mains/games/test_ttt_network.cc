// ==========================================================================
// Program TEST_TTT_NETWORK imports a neural network trained via
// reinforcement learning in program TTT.  It then uses the trained
// network to play against a human opponent.
// ==========================================================================
// Last updated on 10/28/16; 10/29/16; 10/30/16; 10/31/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "machine_learning/reinforce.h"
#include "games/tictac3d.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

//   nrfunc::init_time_based_seed();

   int nsize = 4;
//   int n_zlevels = 1;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);

   ttt_ptr->reset_board_state();
   reinforce* reinforce_agent_ptr = new reinforce();
   reinforce_agent_ptr->initialize_episode();

   ttt_ptr->display_board_state();

   while(!ttt_ptr->get_game_over())
   {

// Human move:

      int human_value = -1;

      ttt_ptr->enter_player_move(human_value);
      ttt_ptr->display_board_state();
      if(ttt_ptr->check_player_win(human_value) > 0) break;
      if(ttt_ptr->check_filled_board()) break;

// Agent move:

      int agent_value = 1;
//      int depth = 0;
//      int depth = 1;
//      int depth = 2;
      int depth = 3;
      int best_move = ttt_ptr->get_recursive_minimax_move(agent_value,depth);

//      ttt_ptr->max_move(agent_value, best_xyz);
//      cout << "max move best_xyz = " << best_xyz << endl;
//      ttt_ptr->minimax_move(agent_value, best_xyz);
//      cout << "minimax move best_xyz = " << best_xyz << endl;

      ttt_ptr->set_player_move(best_move, agent_value);
      ttt_ptr->record_latest_move(agent_value, best_move);

/*
      reinforce_agent_ptr->compute_unrenorm_action_probs(
         ttt_ptr->get_board_state_ptr());
      reinforce_agent_ptr->renormalize_action_distribution();
         
      int output_action = reinforce_agent_ptr->get_candidate_current_action();
      int pz = output_action / (nsize * nsize);
      int py = (output_action - nsize * nsize * pz) / nsize;
      int px = (output_action - nsize * nsize * pz - nsize * py);
      reinforce_agent_ptr->set_current_action(output_action);

      ttt_ptr->set_player_move(px, py, pz, agent_value);
*/


      ttt_ptr->display_board_state();
      ttt_ptr->increment_n_agent_turns();

      if(ttt_ptr->check_player_win(agent_value) > 0) break;
      if(ttt_ptr->check_filled_board()) break;

   } // !game_over while loop

   ttt_ptr->print_winning_pattern();

   delete ttt_ptr;
   delete reinforce_agent_ptr;
}



