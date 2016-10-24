// ==========================================================================
// Program FILL_GRID
// ==========================================================================
// Last updated on 10/18/16; 10/22/16; 10/23/16; 10/24/16
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
//   int n_zlevels = 2;
//   int n_zlevels = 3;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_max_turns = nsize * nsize * n_zlevels;

   int Din = nsize * nsize * n_zlevels;	// Input dimensionality
   int Dout = nsize * nsize * n_zlevels;// Output dimensionality
   int Tmax = nsize * nsize * n_zlevels;

   int H1 = 128;
//   int H1 = 256;
//   int H1 = 300;
//   int H1 = 700;

//   int H2 = 32;
   int H2 = 64;
//   int H2 = 80;
//   int H2 = 100;
//   int H2 = 128;
//   int H2 = 300;

   int H3 = 32;
//   int H3 = 64;

   string extrainfo="H1="+stringfunc::number_to_string(H1)+
      "; H2="+stringfunc::number_to_string(H2)+
      "; H3="+stringfunc::number_to_string(H3)+
      "; zlevels="+stringfunc::number_to_string(n_zlevels);

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   layer_dims.push_back(H2);
   layer_dims.push_back(H3);
   layer_dims.push_back(Dout);

   reinforce* reinforce_ptr = new reinforce(layer_dims, Tmax);
//    reinforce_ptr->set_learning_rate(3E-4);
   reinforce_ptr->set_learning_rate(1E-4);
//   reinforce_ptr->set_learning_rate(3E-5);

//   int n_max_episodes = 1 * 1000000;
//   int n_max_episodes = 2 * 1000000;
//   int n_max_episodes = 3 * 1000000;
//  int n_max_episodes = 4 * 1000000;
//  int n_max_episodes = 10 * 1000000;
  int n_max_episodes = 15 * 1000000;
   int n_update = 0.01 * n_max_episodes;
   int n_losses = 0;
   int n_wins = 0;
   double curr_reward;

   while(reinforce_ptr->get_episode_number() < n_max_episodes)
   {
      ttt_ptr->reset_board_state();
      reinforce_ptr->initialize_episode();

      int curr_episode_number = reinforce_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         reinforce_ptr->get_episode_number(), 50000, n_max_episodes);
      
      while(true)
      {
         ttt_ptr->get_random_legal_AI_move();
         ttt_ptr->increment_n_AI_turns();
//          ttt_ptr->display_board_state();
         if(ttt_ptr->get_game_over())
         {
            curr_reward = 1;
            reinforce_ptr->record_reward_for_action(curr_reward);
            break;
         }

         int output_action = reinforce_ptr->compute_current_action(
            ttt_ptr->get_board_state_ptr());
//          cout << "output_action = " << output_action << endl;
         ttt_ptr->increment_n_agent_turns();

// Step the environment and then retrieve new reward measurements:

         int pz = output_action / (nsize * nsize);
         int py = (output_action - nsize * nsize * pz) / nsize;
         int px = (output_action - nsize * nsize * pz - nsize * py);

         // cout << "px = " << px << " py = " << py << endl;

         curr_reward = ttt_ptr->set_agent_move(px, py, pz);
         reinforce_ptr->record_reward_for_action(curr_reward);
//          cout << "curr_reward = " << curr_reward << endl;

//         bool print_flag = false;
//         bool print_flag = true;
//         ttt_ptr->get_random_agent_move(print_flag);

//          ttt_ptr->display_board_state();
         if(ttt_ptr->get_game_over())
         {
            break;
         }
      } // !game_over while loop


      if(ttt_ptr->get_score() == -1)
      {
         n_losses++;
      }
      else
      {
         n_wins++;
      }

      bool episode_finished_flag = true;
      reinforce_ptr->update_weights(episode_finished_flag);
      reinforce_ptr->update_running_reward(extrainfo);
      
      reinforce_ptr->increment_episode_number();
      int n_episodes = reinforce_ptr->get_episode_number();
      if(curr_episode_number % n_update == 0)
      {
         cout << "n_filled_cells = " << ttt_ptr->get_n_filled_cells()
              << endl;
//          ttt_ptr->display_board_state();
         
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_losses = " << n_losses
              << " n_wins = " << n_wins
              << " win_frac = " << win_frac
              << endl;
      }

      if(curr_episode_number % 2000 == 0)
      {
         double curr_n_turns_frac = double(
            ttt_ptr->get_n_AI_turns() + ttt_ptr->get_n_agent_turns()) / 
            n_max_turns;
         reinforce_ptr->append_n_episode_turns_frac(curr_n_turns_frac);
      }

      if(reinforce_ptr->get_episode_number() % 50000 == 0)
      {
         reinforce_ptr->compute_weight_distributions();
         reinforce_ptr->plot_loss_history(extrainfo);
         reinforce_ptr->plot_reward_history(extrainfo);
         reinforce_ptr->plot_turns_history(extrainfo);
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



