// ==========================================================================
// Program TEST_TTT_NETWORK imports a neural network trained via
// reinforcement learning in program TTT.  It then uses the trained
// network to play against a human opponent.
// ==========================================================================
// Last updated on 10/5/16; 10/18/16; 10/27/16; 10/28/16
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

   int nsize = 4;
//   int n_zlevels = 1;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int Dout = nsize * nsize * n_zlevels;  // Output dimensionality

   ttt_ptr->reset_board_state();
   reinforce* reinforce_ptr = new reinforce();
   reinforce_ptr->initialize_episode();

   ttt_ptr->display_board_state();

   while(!ttt_ptr->get_game_over())
   {

// Human move:

      int human_value = -1;
      ttt_ptr->enter_player_move(human_value);
      ttt_ptr->display_board_state();
      if(ttt_ptr->check_player_win(human_value) > 0) break;

// Agent move:

      int agent_value = 1;

      reinforce_ptr->compute_unrenorm_action_probs(
         ttt_ptr->get_board_state_ptr());
      bool reasonable_action_prob_distribution_flag = 
         reinforce_ptr->renormalize_action_distribution();
         
      int output_action = -99;
      int px, py, pz;
      bool legal_move = false;

      while(!legal_move)
      {
         if(reasonable_action_prob_distribution_flag)
         {
            output_action = reinforce_ptr->get_candidate_current_action();
         }
         else
         {
            output_action = nrfunc::ran1() * Dout;
         }
            
         pz = output_action / (nsize * nsize);
         py = (output_action - nsize * nsize * pz) / nsize;
         px = (output_action - nsize * nsize * pz - nsize * py);
         legal_move = ttt_ptr->legal_player_move(px, py, pz);

         reasonable_action_prob_distribution_flag = 
            reinforce_ptr->zero_p_action(output_action);
      } // !legal_move conditional
      reinforce_ptr->set_current_action(output_action);

      ttt_ptr->set_player_move(px, py, pz, agent_value);
      ttt_ptr->display_board_state();
      ttt_ptr->increment_n_agent_turns();

//      ttt_ptr->get_random_legal_player_move(agent_value);

      if(ttt_ptr->check_player_win(agent_value) > 0) break;
   } // !game_over while loop

   ttt_ptr->print_winning_pattern();

   delete ttt_ptr;
   delete reinforce_ptr;
}



