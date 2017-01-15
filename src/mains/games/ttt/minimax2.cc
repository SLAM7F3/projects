// ==========================================================================
// Program MINIMAX allows a minimax AI to play again a minimax agent.
// It exports afterstate-action pairs for both players to an output
// text file for later supervised learning policy training purposes.
// ==========================================================================
// Last updated on 11/2/16; 11/3/16; 11/4/16; 1/15/17
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "games/tictac3d.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

   int nsize = 4;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   ttt_ptr->reset_board_state();

//   bool AI_move_first = true;
   bool AI_move_first = false;
//   ttt_ptr->set_recursive_depth(0);  
//   ttt_ptr->set_recursive_depth(1);  // machine plays defensively 
   ttt_ptr->set_recursive_depth(2);  // machine plays offensively
//   ttt_ptr->set_recursive_depth(3);  // machine plays defensively (very slowly)

   int n_games = 5 * 1000;
   for(int g = 0; g < n_games; g++)
   {
      cout << "************************************************************" 
           << endl;
      cout << "Starting game " << g << endl;
      outputfunc::update_progress_and_remaining_time(g, 10, n_games);

      ttt_ptr->reset_board_state();
      while(!ttt_ptr->get_game_over())
      {

// AI move:

         if(AI_move_first || ttt_ptr->get_n_agent_turns() > 0)
         {
            int AI_value = -1;   // "X"

            int best_move = ttt_ptr->imminent_win_or_loss(AI_value);
            if(best_move < 0)
            {
               best_move = ttt_ptr->get_recursive_minimax_move(AI_value);
            }

            ttt_ptr->set_player_move(best_move, AI_value);
            ttt_ptr->record_latest_move(AI_value, best_move);
            ttt_ptr->record_afterstate_action(AI_value, best_move);

//            ttt_ptr->display_board_state();
            ttt_ptr->increment_n_AI_turns();
            if(ttt_ptr->check_player_win(AI_value) > 0)
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
         ttt_ptr->record_afterstate_action(agent_value, best_move);

//         ttt_ptr->display_board_state();
         ttt_ptr->increment_n_agent_turns();

         if(ttt_ptr->check_player_win(agent_value) > 0)
         {
            ttt_ptr->set_game_over(true);
            break;
         }

         if(ttt_ptr->check_filled_board()) break;

         int n_completed_turns = ttt_ptr->get_n_completed_turns();
//         cout << "n_completed_turns = " << n_completed_turns << endl;
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

//      ttt_ptr->print_winning_pattern();
      cout << "n_AI_turns = " << ttt_ptr->get_n_AI_turns() << endl;
      cout << "n_agent_turns = " << ttt_ptr->get_n_agent_turns() << endl;
      cout << "n_completed_turns = " << ttt_ptr->get_n_completed_turns() 
           << endl;
      cout << "Number of recorded afterstate-action pairs = "
           << ttt_ptr->get_n_afterstate_action_pairs() << endl;

   } // loop over index g labeling games
   
   string output_filename="afterstate_action_pairs.txt";
   ttt_ptr->export_recorded_afterstate_action_pairs(output_filename);

   delete ttt_ptr;
}



