// ==========================================================================
// Program TRAIN_TTT_NETWORK 
// ==========================================================================
// Last updated on 11/3/16; 11/4/16; 11/7/16; 11/25/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "machine_learning/environment.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "machine_learning/reinforce.h"
#include "general/stringfuncs.h"
#include "games/tictac3d.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
void compute_minimax_move(
   bool AI_moves_first, tictac3d* ttt_ptr, int AI_value)
{
   int best_move = ttt_ptr->imminent_win_or_loss(AI_value);
   if(best_move < 0)
   {
      best_move = ttt_ptr->get_recursive_minimax_move(AI_value);
   }
   ttt_ptr->set_player_move(best_move, AI_value);

// Periodically diminish relative differences between intrinsic cell
// prizes as game progresses:

   int n_completed_turns = ttt_ptr->get_n_completed_turns();
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
}

// ==========================================================================
int main (int argc, char* argv[])
{
   timefunc::initialize_timeofday_clock();
//   nrfunc::init_time_based_seed();
   long s = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> s;
   nrfunc::init_default_seed(s);

   int nsize = 4;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_actions = nsize * nsize * n_zlevels;
   int n_max_turns = n_actions;

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::TTT);
   game_world.set_tictac3d(ttt_ptr);

   int Din = nsize * nsize * n_zlevels;	// Input dimensionality
   int Dout = n_actions;		// Output dimensionality

//   int H1 = 1 * 64;	// 
//   int H1 = 3 * 64;	//  
   int H1 = 5 * 64;	//  = 320
//   int H1 = 7 * 64;	//  

   int H2 = 0;
//   int H2 = 1 * 64;
//   int H2 = 3 * 64;
//   int H2 = 5 * 64;

   string extrainfo="H1="+stringfunc::number_to_string(H1);
   if(H2 > 0)
   {
      extrainfo += "; H2="+stringfunc::number_to_string(H2);
   }
   extrainfo += "; zlevels="+stringfunc::number_to_string(n_zlevels);

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   if(H2 > 0)
   {
      layer_dims.push_back(H2);
   }
   layer_dims.push_back(Dout);

// Construct reinforcement learning agent:

   int replay_memory_capacity = 10 * sqr(n_max_turns);  
   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, n_max_turns, replay_memory_capacity);
//   reinforce_agent_ptr->set_debug_flag(true);
   reinforce_agent_ptr->set_environment(&game_world);

// Gamma = discount factor for reward:

//   double gamma = 0.99;
   double gamma = 0.95;
//   double gamma = 0.9;
   reinforce_agent_ptr->set_gamma(gamma);  

//   reinforce_agent_ptr->set_batch_size(10);   
//   reinforce_agent_ptr->set_batch_size(3);   
   reinforce_agent_ptr->set_batch_size(1);  

   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);

//   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_base_learning_rate(3E-4);
//   reinforce_agent_ptr->set_base_learning_rate(1E-4);
   double min_learning_rate = 1E-4;

   int n_max_episodes = 1 * 1000000;
   if(n_zlevels > 1)
   {
      n_max_episodes = 20 * 1000000;
   }

   int n_update = 2000;
   int n_summarize = 10000;

   int n_losses = 0;
   int n_stalemates = 0;
   int n_wins = 0;
   int n_recent_losses = 0;
   int n_recent_stalemates = 0;
   int n_recent_wins = 0;
   double curr_reward = -999;
   double win_reward = 1;
   double stalemate_reward = 0.25;
   double lose_reward = -1;

// Periodically decrease learning rate down to some minimal floor
// value:

   int n_episodes_period = 100 * 1000;

   int old_weights_period = 10; // Seems optimal for n_grid_size = 8
//   int old_weights_period = 32;  

   double min_epsilon = 0.01;	// Seems optimal for n_grid_size = 8
//   double min_epsilon = 0.025;
//   double min_epsilon = 0.05; 
//   double min_epsilon = 0.1; 

   int AI_value = -1;     // "X" pieces
   int agent_value = 1;   // "O" pieces
//   bool AI_moves_first = true;
   bool AI_moves_first = false;
   bool periodically_switch_starting_player = false;
//   bool periodically_switch_starting_player = true;

// Initialize Deep Q replay memory:

   game_world.start_new_episode();

   reinforce_agent_ptr->initialize_replay_memory();
   int update_old_weights_counter = 0;
   double total_loss = -1;

// Import previously trained TTT network to guide AI play:

   reinforce* reinforce_AI_ptr = NULL;
//   reinforce* reinforce_AI_ptr = new reinforce();

// ==========================================================================
// Reinforcement training loop begins here

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes)
   {
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_summarize, n_max_episodes);

      ttt_ptr->reset_board_state();
      reinforce_agent_ptr->initialize_episode();

// Decrease learning rate as training proceeds:

      if(curr_episode_number > 0 && curr_episode_number%n_episodes_period == 0)
      {
         double curr_learning_rate = reinforce_agent_ptr->get_learning_rate();
         if(curr_learning_rate > min_learning_rate)
         {
            reinforce_agent_ptr->set_learning_rate(0.8 * curr_learning_rate);
            n_episodes_period *= 1.2;
         }

         if(periodically_switch_starting_player)
         {
            AI_moves_first = !AI_moves_first;
         }
      }

      if(AI_moves_first)
      {
         ttt_ptr->set_recursive_depth(0);   // AI plays stupidly
//         ttt_ptr->set_recursive_depth(2);   // AI plays offensively
      }
      else
      {
         ttt_ptr->set_recursive_depth(0);   // AI plays stupidly
//         ttt_ptr->set_recursive_depth(1);   // AI plays defensively
      }

// -----------------------------------------------------------------------
// Current episode starts here:

//      cout << "************  Start of Game " << curr_episode_number
//           << " ***********" << endl;

      double reward;
      genvector* next_s;
      while(!ttt_ptr->get_game_over())
      {
         int curr_timestep = reinforce_agent_ptr->get_curr_timestep();

// AI move:

         if((AI_moves_first && curr_timestep == 0) || curr_timestep > 0)
         {
//            ttt_ptr->get_random_legal_player_move(AI_value);
            compute_minimax_move(AI_moves_first, ttt_ptr, AI_value);
            ttt_ptr->increment_n_AI_turns();
//            ttt_ptr->display_board_state();

            if(ttt_ptr->check_player_win(AI_value) > 0)
            {
               ttt_ptr->set_game_over(true);
               curr_reward = lose_reward; // Agent loses!
               reinforce_agent_ptr->record_reward_for_action(curr_reward);
               break;
            }
         
            if(ttt_ptr->check_filled_board())
            {
               curr_reward = stalemate_reward; // Entire board filled
               reinforce_agent_ptr->record_reward_for_action(curr_reward);
               break;
            }
         } // AI_moves_first || timestep > 0 conditional

// Agent move:

/*
// Expt on Fri Nov 4 at 9:30 am with Delsey's idea about "check" in 3D TTT:

         int a_winning_opponent_action = 
            ttt_ptr->check_opponent_win_on_next_turn(agent_value);
         if(a_winning_opponent_action >= 0)
         {
            reinforce_agent_ptr->hardwire_output_action(
               a_winning_opponent_action);
            output_action = a_winning_opponent_action;
         }
         else
         {
*/

         genvector *curr_s = game_world.get_curr_state();
         int d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
            *curr_s);
         int curr_a = reinforce_agent_ptr->select_action_for_curr_state();

         if(!game_world.is_legal_action(curr_a))
         {
            next_s = NULL;
            reward = -1;
            curr_maze.set_game_over(true);
         }
         else
         {
            next_s = game_world.compute_next_state(curr_a);
            reward = curr_maze.compute_turtle_reward();
         } // curr_a is legal action conditional

//         cout << " reward = " << reward 
//              << " game over = " << curr_maze.get_game_over() << endl;




         reinforce_agent_ptr->set_current_action(output_action);
         ttt_ptr->set_player_move(output_action, agent_value);

//         ttt_ptr->increment_n_agent_turns();

//          ttt_ptr->display_board_state();

// Step the environment and then retrieve new reward measurements:

         curr_reward = 0;
         if(ttt_ptr->check_player_win(agent_value) > 0)
         {
            ttt_ptr->set_game_over(true);
            curr_reward = win_reward;	 // Agent wins!
         }
         else if (ttt_ptr->check_filled_board())
         {
            curr_reward = stalemate_reward;
         }

         reinforce_agent_ptr->record_reward_for_action(curr_reward);
//          cout << "curr_reward = " << curr_reward << endl;
      } // !game_over while loop
// -----------------------------------------------------------------------

      if(curr_episode_number > 0 && curr_episode_number% n_update == 0)
      {
         ttt_ptr->display_board_state();
//         outputfunc::enter_continue_char();
//          cout << "GAME OVER" << endl;
      }
      
      if(nearly_equal(curr_reward, lose_reward))
      {
         n_losses++;
         n_recent_losses++;
      }
      else if (nearly_equal(curr_reward, win_reward))
      {
         n_wins++;
         n_recent_wins++;
      }
      else if(ttt_ptr->get_n_empty_cells() == 0)
      {
         n_stalemates++;
         n_recent_stalemates++;
      }

      reinforce_agent_ptr->update_weights();
      reinforce_agent_ptr->update_running_reward(extrainfo);
      
      reinforce_agent_ptr->increment_episode_number();
      int n_episodes = curr_episode_number + 1;
      if(curr_episode_number >= n_update - 1 && 
         curr_episode_number % n_update == 0)
      {
         if(AI_value == -1)
         {
            cout << "AI = X    agent = O" << endl;
         }
         if(AI_moves_first)
         {
            cout << "AI moves first " << endl;
         }
         else
         {
            cout << "Agent moves first" << endl;
         }
         
         double recent_loss_frac = double(n_recent_losses) / n_update;
         double recent_stalemate_frac = double(n_recent_stalemates) / n_update;
         double recent_win_frac = double(n_recent_wins) / n_update;
         double loss_frac = double(n_losses) / n_episodes;
         double stalemate_frac = double(n_stalemates) / n_episodes;
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_recent_losses = " << n_recent_losses
              << " n_recent_stalemates = " << n_recent_stalemates
              << " n_recent_wins = " << n_recent_wins
              << endl;
         cout << " recent loss frac = " << recent_loss_frac
              << " recent stalemate frac = " << recent_stalemate_frac 
              << " recent win frac = " << recent_win_frac << endl;
         cout << " loss_frac = " << loss_frac
              << " stalemate_frac = " << stalemate_frac
              << " win_frac = " << win_frac
              << endl;
         n_recent_losses = n_recent_stalemates = n_recent_wins = 0;

         double curr_n_turns_frac = double(
            ttt_ptr->get_n_AI_turns() + ttt_ptr->get_n_agent_turns()) / 
            n_max_turns;
         reinforce_agent_ptr->append_n_episode_turns_frac(curr_n_turns_frac);
         reinforce_agent_ptr->snapshot_running_reward();

         ttt_ptr->append_game_loss_frac(loss_frac);
         ttt_ptr->append_game_stalemate_frac(stalemate_frac);
         ttt_ptr->append_game_win_frac(win_frac);
      }

      if(curr_episode_number >= n_summarize - 1 && 
         curr_episode_number % n_summarize == 0)
      {
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->plot_loss_history(extrainfo);
         reinforce_agent_ptr->plot_reward_history(
            extrainfo, lose_reward, win_reward);
         reinforce_agent_ptr->plot_turns_history(extrainfo);
         reinforce_agent_ptr->export_snapshot();
         ttt_ptr->plot_game_frac_histories(curr_episode_number, extrainfo);
      }

   } // n_episodes < n_max_episodes while loop

// Reinforcement training loop ends here
// ==========================================================================

   delete ttt_ptr;
   delete reinforce_agent_ptr;
   delete reinforce_AI_ptr;
}
