// ==========================================================================
// Program TEST_TTT_NETWORK imports a neural network trained via
// reinforcement learning in program TTT.  It then uses the trained
// network to play against a human opponent.
// ==========================================================================
// Last updated on 11/1/16; 11/2/16; 11/3/16; 11/4/16
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

   nrfunc::init_time_based_seed();

//   int nsize = 3;
   int nsize = 4;
//   int nsize = 5;
//   int n_zlevels = 1;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   ttt_ptr->reset_board_state();

//   bool human_move_first = true;
   bool human_move_first = false;
   if(human_move_first)
   {
//      ttt_ptr->set_recursive_depth(1);	 // machine plays defensively
      ttt_ptr->set_recursive_depth(3);
   }
   else
   {
//      ttt_ptr->set_recursive_depth(0);	 // machine plays offensively
      ttt_ptr->set_recursive_depth(2);	 // machine plays offensively
   }
   
   reinforce* reinforce_agent_ptr = new reinforce();
   reinforce_agent_ptr->initialize_episode();

   while(!ttt_ptr->get_game_over())
   {

// Human move:

      if(human_move_first && ttt_ptr->get_n_agent_turns() == 0)
      {
         ttt_ptr->display_board_state();
      }

      if(human_move_first || ttt_ptr->get_n_agent_turns() > 0)
      {
         int human_value = -1;   // "X"

         int a_winning_opponent_action = 
            ttt_ptr->check_opponent_win_on_next_turn(human_value);

         ttt_ptr->enter_player_move(human_value);
         ttt_ptr->display_board_state();
         ttt_ptr->increment_n_human_turns();
         if(ttt_ptr->check_player_win(human_value) > 0)
         {
            ttt_ptr->set_game_over(true);
            break;
         }
         if(ttt_ptr->check_filled_board()) break;
      }

// Agent move:

      int agent_value = 1;   // "O"
//      ttt_ptr->display_minimax_scores(agent_value);

      int best_move = ttt_ptr->imminent_win_or_loss(agent_value);
      if(best_move < 0)
      {
         best_move = ttt_ptr->get_recursive_minimax_move(agent_value);
      }

      ttt_ptr->set_player_move(best_move, agent_value);
      ttt_ptr->record_latest_move(agent_value, best_move);

/*
      reinforce_agent_ptr->compute_unrenorm_action_probs(
         ttt_ptr->get_board_state_ptr());
      reinforce_agent_ptr->renormalize_action_distribution();
         
      int output_action = reinforce_agent_ptr->get_candidate_current_action();
      reinforce_agent_ptr->set_current_action(output_action);

      ttt_ptr->set_player_move(output_action, agent_value);
*/

      ttt_ptr->display_board_state();
      ttt_ptr->increment_n_agent_turns();

      if(ttt_ptr->check_player_win(agent_value) > 0)
      {
         ttt_ptr->set_game_over(true);
         break;
      }

      if(ttt_ptr->check_filled_board()) break;

// Periodically diminish relative differences between intrinsic cell
// prizes as game progresses:

      int n_completed_turns = ttt_ptr->get_n_completed_turns();
      cout << "n_completed_turns = " << n_completed_turns << endl;
      if(n_completed_turns == 1)
      {
         ttt_ptr->adjust_intrinsic_cell_prizes();
      }
      else if (n_completed_turns == 2)
      {
         ttt_ptr->adjust_intrinsic_cell_prizes();
      }
      else if (n_completed_turns == 3)
      {
         ttt_ptr->adjust_intrinsic_cell_prizes();
      }

   } // !game_over while loop

   ttt_ptr->print_winning_pattern();

   delete ttt_ptr;
   delete reinforce_agent_ptr;
}



