// ==========================================================================
// Program TTT
// ==========================================================================
// Last updated on 10/24/16; 10/25/16; 10/26/16; 10/27/16
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

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();
//    nrfunc::init_time_based_seed();

   int nsize = 4;
//   int n_zlevels = 1;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_max_turns = nsize * nsize * n_zlevels;

   int Din = nsize * nsize * n_zlevels;	// Input dimensionality
   int Dout = nsize * nsize * n_zlevels;// Output dimensionality
   int Tmax = nsize * nsize * n_zlevels;

//   int H1 = 32;
//   int H1 = 64;
//    int H1 = 128;
//   int H1 = 256;
//   int H1 = 300;
   int H1 = 5 * 64;	//  = 320
//   int H1 = 700;

//   int H2 = 32;
   int H2 = 64;
//   int H2 = 80;
//   int H2 = 100;
//   int H2 = 128;
//   int H2 = 200;
//   int H2 = 300;

//   int H3 = 32;
//   int H3 = 64;
//   int H3 = 100;
//   int H3 = 128;
//   int H3 = 256;

   string extrainfo="H1="+stringfunc::number_to_string(H1)+
      "; H2="+stringfunc::number_to_string(H2)+
//      "; H3="+stringfunc::number_to_string(H3)+
      "; zlevels="+stringfunc::number_to_string(n_zlevels);

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   layer_dims.push_back(H2);
//   layer_dims.push_back(H3);
   layer_dims.push_back(Dout);

   reinforce* reinforce_ptr = new reinforce(layer_dims, Tmax);

// Gamma = discount factor for reward:

//   reinforce_ptr->set_gamma(0.4);
//   reinforce_ptr->set_gamma(0.3);
//   reinforce_ptr->set_gamma(0.33);
   reinforce_ptr->set_gamma(0.25);  // best gamma value as of Weds Oct 26
//   reinforce_ptr->set_gamma(0.1);

//   reinforce_ptr->set_lambda(0.01);// 0 = best lambda value as of Tues Oct 25
//   reinforce_ptr->set_lambda(0.001);
//   reinforce_ptr->set_lambda(0.0001);

   reinforce_ptr->set_batch_size(30);   // Best value as of Tues Oct 25
//   reinforce_ptr->set_batch_size(100);

   reinforce_ptr->set_rmsprop_decay_rate(0.85);
//   reinforce_ptr->set_rmsprop_decay_rate(0.9);

//   reinforce_ptr->set_base_learning_rate(2E-3);
   reinforce_ptr->set_base_learning_rate(10E-4);
//   reinforce_ptr->set_base_learning_rate(3E-4);
//   reinforce_ptr->set_base_learning_rate(1E-4);
//   reinforce_ptr->set_base_learning_rate(3E-5);
   double min_learning_rate = 0.5E-4;
//   double min_learning_rate = 3E-5;

//   int n_max_episodes = 1 * 1000000;
//   int n_max_episodes = 2 * 1000000;
//   int n_max_episodes = 3 * 1000000;
//  int n_max_episodes = 4 * 1000000;
   int n_max_episodes = 10 * 1000000;
//  int n_max_episodes = 15 * 1000000;
   int n_update = 1000;
//   int n_update = 50000;

   int n_losses = 0;
   int n_stalemates = 0;
   int n_wins = 0;
   double curr_reward = -999;
   double max_reward = 1;
   double min_reward = -1;

// Periodically decrease learning rate down to some minimal floor
// value:

   int n_episodes_period = 100 * 1000;
//      int n_episodes_period = 150 * 1000;
//      int n_episodes_period = 200 * 1000;
//      int n_episodes_period = 250 * 1000;

   int AI_value = -1;
   int agent_value = 1;

   while(reinforce_ptr->get_episode_number() < n_max_episodes)
   {
      ttt_ptr->reset_board_state();
      reinforce_ptr->initialize_episode();

      int curr_episode_number = reinforce_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, 10000, n_max_episodes);
      
      if(curr_episode_number > 0 && curr_episode_number%n_episodes_period == 0)
      {
         double curr_learning_rate = reinforce_ptr->get_learning_rate();
         if(curr_learning_rate > min_learning_rate)
         {
            reinforce_ptr->set_learning_rate(0.8 * curr_learning_rate);
            n_episodes_period *= 1.2;
         }
      }

// Current episode starts here:

//      AI_value *= -1;
//      agent_value *= -1;

      while(true)
      {

// AI move:

         ttt_ptr->get_random_legal_player_move(AI_value);
         ttt_ptr->increment_n_AI_turns();
         if(ttt_ptr->check_player_win(AI_value) > 0)
         {
            curr_reward = -1; // Agent loses!
            reinforce_ptr->record_reward_for_action(curr_reward);
            break;
         }
         
         if(ttt_ptr->get_game_over())
         {
            curr_reward = 0; // Entire 3D board is filled - stalemate
            reinforce_ptr->record_reward_for_action(curr_reward);
            break;
         }

// Agent move:

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
            legal_move = ttt_ptr->legal_agent_move(px, py, pz);

            reasonable_action_prob_distribution_flag = 
               reinforce_ptr->zero_p_action(output_action);
         } // !legal_move conditional
         reinforce_ptr->set_current_action(output_action);

         ttt_ptr->increment_n_agent_turns();

// Step the environment and then retrieve new reward measurements:

         curr_reward = ttt_ptr->set_agent_move(px, py, pz);

         if(ttt_ptr->check_player_win(agent_value) > 0)
         {
            curr_reward = max_reward;	 // Agent wins!
         }
         
         reinforce_ptr->record_reward_for_action(curr_reward);
//          cout << "curr_reward = " << curr_reward << endl;

//          ttt_ptr->display_board_state();
         if(ttt_ptr->get_game_over())
         {
            break;
         }
      } // !game_over while loop

      if(ttt_ptr->get_n_empty_cells() == 0)
      {
         n_stalemates++;
      }
      else if(nearly_equal(curr_reward, min_reward))
      {
         n_losses++;
      }
      else if (nearly_equal(curr_reward, max_reward))
      {
         n_wins++;
      }

      bool episode_finished_flag = true;
      reinforce_ptr->update_weights(episode_finished_flag);
      reinforce_ptr->update_running_reward(extrainfo);
      
      reinforce_ptr->increment_episode_number();
      int n_episodes = curr_episode_number;
      if(curr_episode_number > 10 && curr_episode_number % n_update == 0)
      {
         cout << "n_filled_cells = " << ttt_ptr->get_n_filled_cells()
              << endl;
//          ttt_ptr->display_board_state();
         
         double loss_frac = double(n_losses) / n_episodes;
         double stalemate_frac = double(n_stalemates) / n_episodes;
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_losses = " << n_losses
              << " n_stalemates = " << n_stalemates
              << " n_wins = " << n_wins
              << endl;
         cout << " loss_frac = " << loss_frac
              << " stalemate_frac = " << stalemate_frac
              << " win_frac = " << win_frac
              << endl;
      }

      if(curr_episode_number > 10 && curr_episode_number % 1000 == 0)
      {
         double curr_n_turns_frac = double(
            ttt_ptr->get_n_AI_turns() + ttt_ptr->get_n_agent_turns()) / 
            n_max_turns;
         reinforce_ptr->append_n_episode_turns_frac(curr_n_turns_frac);
         reinforce_ptr->snapshot_running_reward();
         reinforce_ptr->export_snapshot();

         double loss_frac = double(n_losses) / n_episodes;
         double stalemate_frac = double(n_stalemates) / n_episodes;
         double win_frac = double(n_wins) / n_episodes;
         ttt_ptr->append_game_loss_frac(loss_frac);
         ttt_ptr->append_game_stalemate_frac(stalemate_frac);
         ttt_ptr->append_game_win_frac(win_frac);
      }

      if(curr_episode_number > 10 && curr_episode_number % 10000 == 0)
      {
         reinforce_ptr->compute_weight_distributions();
         reinforce_ptr->plot_loss_history(extrainfo);
         reinforce_ptr->plot_reward_history(extrainfo, min_reward, max_reward);
         reinforce_ptr->plot_turns_history(extrainfo);
         ttt_ptr->plot_game_frac_histories(curr_episode_number, extrainfo);
      }
   } // n_episodes < n_max_episodes while loop

   delete ttt_ptr;
   delete reinforce_ptr;
}



