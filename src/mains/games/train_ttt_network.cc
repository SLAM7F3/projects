// ==========================================================================
// Program TRAIN_TTT_NETWORK 
// ==========================================================================
// Last updated on 10/27/16; 10/28/16; 10/29/16; 10/30/16
// ==========================================================================

#include <iostream>
#include <string>
#include <unistd.h>     // Needed for sleep() and usleep()
#include <vector>
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
void compute_AI_move(tictac3d* ttt_ptr, reinforce* reinforce_AI_ptr, 
                     int AI_value)
{
   reinforce_AI_ptr->compute_unrenorm_action_probs(
      ttt_ptr->get_inverse_board_state_ptr());
   reinforce_AI_ptr->renormalize_action_distribution();

   int nsize = ttt_ptr->get_n_size();         
   int output_action = reinforce_AI_ptr->get_candidate_current_action();
   int pz = output_action / (nsize * nsize);
   int py = (output_action - nsize * nsize * pz) / nsize;
   int px = (output_action - nsize * nsize * pz - nsize * py);

   bool legal_move = ttt_ptr->legal_player_move(px, py, pz);
   if(!legal_move)
   {
      cout << "legal AI move = " << legal_move << endl;
   }
   
   reinforce_AI_ptr->set_current_action(output_action);
   ttt_ptr->set_player_move(px, py, pz, AI_value);
}

// ==========================================================================
int main (int argc, char* argv[])
{
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

//   int n_zlevels = 1;
   int n_zlevels = 4;

   int nsize = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_max_turns = nsize * nsize * n_zlevels;

   int Din = nsize * nsize * n_zlevels;	// Input dimensionality
   int Dout = nsize * nsize * n_zlevels;// Output dimensionality
   int Tmax = nsize * nsize * n_zlevels;

//   int H1 = 1 * 64;	// 
//   int H1 = 3 * 64;	//  
   int H1 = 5 * 64;	//  = 320
//   int H1 = 7 * 64;	//  

   int H2 = 0;
//   int H2 = 1 * 64;
//   int H2 = 3 * 64;
//   int H2 = 5 * 64;

   if(n_zlevels == 4)
   {
      H1 = 5 * 64;
      H2 = 1 * 64;

//      H1 = 3 * 64;
//      H2 = 3 * 64;

//      H1 = 1 * 64;
//      H2 = 5 * 64;
   }
   
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

   reinforce* reinforce_agent_ptr = new reinforce(layer_dims, Tmax);

// Gamma = discount factor for reward:

   double gamma = 0.99;
//   double gamma = 0.95;
//   double gamma = 0.9;
//   double gamma = 0.5;
   reinforce_agent_ptr->set_gamma(gamma);  

//   reinforce_agent_ptr->set_gamma(0.25);  // best gamma value as of Weds Oct 26

   reinforce_agent_ptr->set_batch_size(30);   // Best value as of Tues Oct 25
//   reinforce_agent_ptr->set_batch_size(60);   
//   reinforce_agent_ptr->set_batch_size(120);  

   reinforce_agent_ptr->set_rmsprop_decay_rate(0.85);

//   double base_learning_rate = 0.001 + nrfunc::ran1() * 0.004;
//   reinforce_agent_ptr->set_base_learning_rate(base_learning_rate);

//   reinforce_agent_ptr->set_base_learning_rate(3E-3);
   //   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_base_learning_rate(3E-4);
   //   reinforce_agent_ptr->set_base_learning_rate(1E-4);

   double min_learning_rate = 0.5E-4;
//   double min_learning_rate = 3E-5;

//  int n_max_episodes = 1 * 1000000;
//  int n_max_episodes = 2 * 1000000;
   int n_max_episodes = 5 * 1000000;
//   int n_max_episodes = 10 * 1000000;
   int n_update = 25000;
   int n_summarize = 50000;

   int n_losses = 0;
   int n_stalemates = 0;
   int n_wins = 0;
   int n_recent_losses = 0;
   int n_recent_stalemates = 0;
   int n_recent_wins = 0;
   double curr_reward = -999;
   double win_reward = 1;
   double stalemate_reward = 0.25;
   double lose_reward = -2;
//   double lose_reward = -1;

// Periodically decrease learning rate down to some minimal floor
// value:

   int n_episodes_period = 100 * 1000;
//      int n_episodes_period = 150 * 1000;

   bool AI_moves_first = true;
   int AI_value = -1;
   int agent_value = 1;
//   bool periodically_switch_starting_player = false;
   bool periodically_switch_starting_player = true;
   bool periodically_switch_player_values = false;

// Import previously trained TTT network to guide AI play:

//   reinforce* reinforce_AI_ptr = NULL;
   reinforce* reinforce_AI_ptr = new reinforce();

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes)
   {
      ttt_ptr->reset_board_state();
      reinforce_agent_ptr->initialize_episode();

      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_summarize, n_max_episodes);

      if(curr_episode_number > 0 && curr_episode_number%n_episodes_period == 0)
      {
         double curr_learning_rate = reinforce_agent_ptr->get_learning_rate();
         if(curr_learning_rate > min_learning_rate)
         {
            reinforce_agent_ptr->set_learning_rate(0.8 * curr_learning_rate);
            n_episodes_period *= 1.2;
         }
      }

      if(periodically_switch_player_values)
      {
         if(curr_episode_number % 10000 == 0)
         {
            AI_value *= -1;
            agent_value *= -1;
         }
      }

      if(periodically_switch_starting_player)
      {
         AI_moves_first = !AI_moves_first;
      }

// -----------------------------------------------------------------------
// Current episode starts here:

      while(!ttt_ptr->get_game_over())
      {

// AI move:
         if((AI_moves_first && reinforce_agent_ptr->get_curr_timestep() == 0)
            || reinforce_agent_ptr->get_curr_timestep() > 0)
         {
            if(reinforce_AI_ptr != NULL && nrfunc::ran1() > 0.025)
            {
               compute_AI_move(ttt_ptr, reinforce_AI_ptr, AI_value);
            }
            else
            {
               ttt_ptr->get_random_legal_player_move(AI_value);
            }
            ttt_ptr->increment_n_AI_turns();

//            ttt_ptr->display_board_state();
//            ttt_ptr->display_p_action(reinforce_agent_ptr->get_p_action());

            if(ttt_ptr->check_player_win(AI_value) > 0)
            {
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

         if(nrfunc::ran1() > 0.025)
         {
            reinforce_agent_ptr->compute_unrenorm_action_probs(
               ttt_ptr->get_board_state_ptr());
            reinforce_agent_ptr->renormalize_action_distribution();
//         ttt_ptr->display_p_action(reinforce_agent_ptr->get_p_action());

            int output_action = 
               reinforce_agent_ptr->get_candidate_current_action();
            int pz = output_action / (nsize * nsize);
            int py = (output_action - nsize * nsize * pz) / nsize;
            int px = (output_action - nsize * nsize * pz - nsize * py);

            reinforce_agent_ptr->set_current_action(output_action);
            ttt_ptr->set_player_move(px, py, pz, agent_value);
         }
         else
         {
            ttt_ptr->get_random_legal_player_move(agent_value);
         }
         ttt_ptr->increment_n_agent_turns();

//         ttt_ptr->display_board_state();

// Step the environment and then retrieve new reward measurements:

         curr_reward = 0;
         if(ttt_ptr->check_player_win(agent_value) > 0)
         {
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

      bool episode_finished_flag = true;
      reinforce_agent_ptr->update_weights(episode_finished_flag);
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
         else
         {
            cout << "AI = O    agent = X" << endl;
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

   delete ttt_ptr;
   delete reinforce_agent_ptr;
   delete reinforce_AI_ptr;
}




