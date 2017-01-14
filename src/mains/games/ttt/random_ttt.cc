// ==========================================================================
// Program RANDOM_TTT scores random play of TTT.
// ==========================================================================
// Last updated on 1/10/17; 1/11/17; 1/13/17; 1/14/17
// ==========================================================================

#include <iostream>
#include <string>
#include <unistd.h>		// needed for getpid()
#include <vector>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "machine_learning/machinelearningfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "machine_learning/reinforce.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "games/tictac3d.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   long seed = nrfunc::init_time_based_seed();
//   long seed = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> seed;
//   nrfunc::init_default_seed(seed);

   int nsize = 4;
//   int n_zlevels = 1;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_actions = nsize * nsize * n_zlevels;
   int n_max_turns = n_actions;

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::TTT);
   game_world.set_tictac3d(ttt_ptr);

// Set neural network architecture parameters:

   int Din = nsize * nsize * n_zlevels;	// Input dimensionality
   int Dout = n_actions;		// Output dimensionality

//   int H1 = 32;
   int H1 = 64;
//   int H1 = 128;

   int H2 = 32;
//   int H2 = 64;

   int H3 = 0;
   
   string extrainfo="H1="+stringfunc::number_to_string(H1);
   if(H2 > 0)
   {
      extrainfo += "; H2="+stringfunc::number_to_string(H2);
   }
   if(H3 > 0)
   {
      extrainfo += "; H3="+stringfunc::number_to_string(H3);
   }
   extrainfo += "; zlevels="+stringfunc::number_to_string(n_zlevels);
   extrainfo += "; Q-learning";

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   if(H2 > 0)
   {
      layer_dims.push_back(H2);
   }
   if(H3 > 0)
   {
      layer_dims.push_back(H3);
   }
   layer_dims.push_back(Dout);

// Construct reinforcement learning agent:

   int replay_memory_capacity = 10 * 1000;
   int eval_memory_capacity = basic_math::min(
      int(0.1 * replay_memory_capacity), 20000);

   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, 1, replay_memory_capacity, eval_memory_capacity,
      reinforce::RMSPROP);
   reinforce_agent_ptr->set_environment(&game_world);
   reinforce_agent_ptr->set_lambda(1E-2);
   machinelearning_func::set_leaky_ReLU_small_slope(0.01); 

// Initialize output subdirectory within an experiments folder:

   string experiments_subdir="./experiments/";
   filefunc::dircreate(experiments_subdir);

   int expt_number;
   cout << "Enter experiment number:" << endl;
   cin >> expt_number;
   string output_subdir=experiments_subdir+
      "expt"+stringfunc::integer_to_string(expt_number,3)+"/";
   filefunc::dircreate(output_subdir);

   reinforce_agent_ptr->set_expt_number(expt_number);
   reinforce_agent_ptr->set_Nd(32);
   reinforce_agent_ptr->set_gamma(0.95);  // reward discount factor
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);  

//   reinforce_agent_ptr->set_base_learning_rate(1E-5);   // too large
//   reinforce_agent_ptr->set_base_learning_rate(3E-6);   // too large
//   reinforce_agent_ptr->set_base_learning_rate(2E-6);  //  ?
   reinforce_agent_ptr->set_base_learning_rate(1E-6);  //  OK

//   int n_max_episodes = 200 * 1000;
//   int n_max_episodes = 300 * 1000;
   int n_max_episodes = 400 * 1000;

   int n_update = 2000;
   int n_progress = 4000;
//    int n_snapshot = 20000;

   int n_illegal_moves = 0;
   int n_losses = 0;
   int n_stalemates = 0;
   int n_wins = 0;
   int n_recent_illegal_moves = 0;
   int n_recent_losses = 0;
   int n_recent_stalemates = 0;
   int n_recent_wins = 0;

   double win_reward = 1;
   double stalemate_reward = 0.25;
   double lose_reward = -1;
   double illegal_reward = -2;

// Periodically decrease learning rate down to some minimal floor
// value:

   int n_lr_episodes_period = 150 * 1000;
   int old_weights_period = 8 * replay_memory_capacity;

   double min_epsilon = 0.025;
//   double min_epsilon = 0.10;
   reinforce_agent_ptr->set_min_epsilon(min_epsilon);
   double starting_episode_linear_eps_decay = 1000;
   double stopping_episode_linear_eps_decay = 0.75 * n_max_episodes;

   int AI_value = -1;     // "X" pieces
   int agent_value = 1;   // "O" pieces
//   bool AI_moves_first = true;
   bool AI_moves_first = false;
//   bool periodically_switch_starting_player = false;
   bool periodically_switch_starting_player = true;

   int update_old_weights_counter = 0;
   double total_loss = -1;

// Generate text file summary of parameter values:

   string params_filename = output_subdir + "params.dat";
   reinforce_agent_ptr->summarize_parameters(params_filename);
   ofstream params_stream;
   filefunc::appendfile(params_filename, params_stream);

   params_stream << "n_actions = " << n_actions << endl;
   params_stream << "Leaky ReLU small slope = "
                 << machinelearning_func::get_leaky_ReLU_small_slope() << endl;
   params_stream << "Learning rate decrease period = " 
                 << n_lr_episodes_period << " episodes" << endl;
   params_stream << "Old weights period = " << old_weights_period
                 << " episodes" << endl;
   params_stream << "n_max_episodes = " << n_max_episodes << endl;
   params_stream << "Frame skip = " << game_world.get_frame_skip() << endl;
   params_stream << "Starting episode for linear epsilon decay = "
                 << starting_episode_linear_eps_decay << endl;
   params_stream << "Stopping episode for linear epsilon decay = "
                 << stopping_episode_linear_eps_decay << endl;
   params_stream << "Periodically switch starting player = "
                 << periodically_switch_starting_player << endl;
   params_stream << "Random seed = " << seed << endl;
   params_stream << "Process ID = " << getpid() << endl;
   filefunc::closefile(params_filename, params_stream);

// Import previously trained TTT network to guide AI play:

   reinforce* reinforce_AI_ptr = NULL;
//   reinforce* reinforce_AI_ptr = new reinforce();

// ==========================================================================
// Reinforcement training loop begins here

   bool eval_memory_full_flag = false;
   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes)
   {
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_progress, n_max_episodes);

      game_world.start_new_episode();
      reinforce_agent_ptr->initialize_episode();

// Randomly switch starting player:

      if(nrfunc::ran1() < 0.5)
      {
         AI_moves_first = true;
      }
      else
      {
         AI_moves_first = false;
      }

// -----------------------------------------------------------------------
// Current episode starts here:

//      cout << "************  Start of episode " << curr_episode_number
//           << " for expt " << expt_number << " ***********" << endl;

      int d = -1;
      double curr_reward;
      genvector* next_s;
      double cum_reward = 0;

      while(!game_world.get_game_over())
      {

// AI move:

         if((AI_moves_first && ttt_ptr->get_n_AI_turns() == 0) || 
            ttt_ptr->get_n_agent_turns() > 0)
         {
            ttt_ptr->get_random_legal_player_move(AI_value);

            ttt_ptr->increment_n_AI_turns();
//            ttt_ptr->display_board_state();
//            outputfunc::enter_continue_char();

            if(ttt_ptr->check_player_win(AI_value) > 0)
            {
               game_world.set_game_over(true);
               curr_reward = lose_reward; // Agent loses!
//               cout << "AI wins" << endl;
               break;
            }
         
            if(ttt_ptr->check_filled_board())
            {
               game_world.set_game_over(true);
               curr_reward = stalemate_reward; // Entire board filled
               break;
            }
         } // AI_moves_first || timestep > 0 conditional

// Agent move:

         ttt_ptr->get_random_legal_player_move(agent_value);

         ttt_ptr->increment_n_agent_turns();
//         ttt_ptr->display_board_state();
//         outputfunc::enter_continue_char();

         if(ttt_ptr->check_player_win(agent_value) > 0)
         {
            game_world.set_game_over(true);
            curr_reward = win_reward; // Agent wins!
//            cout << "Agent wins" << endl;
            break;
         }
         
         if(ttt_ptr->check_filled_board())
         {
            game_world.set_game_over(true);
            curr_reward = stalemate_reward; // Entire board filled
            break;
         }
      } // !game_over while loop
// -----------------------------------------------------------------------

//      ttt_ptr->display_board_state();
//      outputfunc::enter_continue_char();

      int n_episodes = 1 + curr_episode_number;
      double illegal_frac, loss_frac, win_frac, stalemate_frac;
      if(nearly_equal(curr_reward, illegal_reward))
      {
         n_illegal_moves++;
         n_recent_illegal_moves++;
         illegal_frac = double(n_illegal_moves) / n_episodes;
      }
      else if(nearly_equal(curr_reward, lose_reward))
      {
         n_losses++;
         n_recent_losses++;
         loss_frac = double(n_losses) / n_episodes;
      }
      else if (nearly_equal(curr_reward, win_reward))
      {
         n_wins++;
         n_recent_wins++;
         win_frac = double(n_wins) / n_episodes;
      }
      else if(ttt_ptr->get_n_empty_cells() == 0)
      {
         n_stalemates++;
         n_recent_stalemates++;
         stalemate_frac = double(n_stalemates) / n_episodes;
      }

      if(curr_episode_number % 100 == 0)
      {
         cout << "*** Episode " << curr_episode_number 
              << " of expt " << expt_number << " finished ***" << endl;
         cout << "  cum_reward = " << cum_reward 
              << "  epsilon = " << reinforce_agent_ptr->get_epsilon() 
              << "  n_backprops = " 
              << reinforce_agent_ptr->get_n_backprops() << endl;
         cout << "  n_illegal_moves = " << n_illegal_moves
              << "  n_losses = " << n_losses
              << "  n_wins = " << n_wins 
              << "  n_stalemates = " << n_stalemates << endl;
         cout << "  Fractions: win = " << win_frac 
              << " loss = " << loss_frac
              << " stalemate = " << stalemate_frac << endl;
      }

// Periodically write status info to text console:

      if(curr_episode_number > 0 && curr_episode_number % n_update == 0)
      {
         cout << "Episode number = " << curr_episode_number 
              << " epsilon = " << reinforce_agent_ptr->get_epsilon()
              << endl;

         ttt_ptr->display_board_state();
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

         int n_episodes = curr_episode_number + 1;         
         double recent_illegal_frac = double(n_recent_illegal_moves) 
            / n_update;
         double recent_loss_frac = double(n_recent_losses) / n_update;
         double recent_stalemate_frac = double(n_recent_stalemates) / n_update;
         double recent_win_frac = double(n_recent_wins) / n_update;
         double illegal_frac = double(n_illegal_moves) / n_episodes;
         double loss_frac = double(n_losses) / n_episodes;
         double stalemate_frac = double(n_stalemates) / n_episodes;
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_recent_illegal_moves = " << n_recent_illegal_moves
              << " n_recent_losses = " << n_recent_losses
              << " n_recent_stalemates = " << n_recent_stalemates
              << " n_recent_wins = " << n_recent_wins
              << endl;
         cout << "recent illegal frac = " << recent_illegal_frac
              << " recent loss frac = " << recent_loss_frac
              << " recent stalemate frac = " << recent_stalemate_frac 
              << " recent win frac = " << recent_win_frac << endl;
         cout << " illegal_frac = " << illegal_frac
              << " loss_frac = " << loss_frac
              << " stalemate_frac = " << stalemate_frac
              << " win_frac = " << win_frac
              << endl;
         n_recent_illegal_moves = n_recent_losses = n_recent_stalemates 
            = n_recent_wins = 0;

         double curr_n_turns_frac = double(
            ttt_ptr->get_n_AI_turns() + ttt_ptr->get_n_agent_turns()) / 
            n_max_turns;
         reinforce_agent_ptr->append_n_episode_turns_frac(curr_n_turns_frac);

         ttt_ptr->append_game_illegal_frac(illegal_frac);
         ttt_ptr->append_game_loss_frac(loss_frac);
         ttt_ptr->append_game_stalemate_frac(stalemate_frac);
         ttt_ptr->append_game_win_frac(win_frac);
      }

      if(curr_episode_number > 0 && curr_episode_number % n_progress == 0)
      {
         ttt_ptr->plot_game_frac_histories(
            output_subdir, curr_episode_number, extrainfo);
      }

      reinforce_agent_ptr->increment_episode_number();
   } // n_episodes < n_max_episodes while loop

// Reinforcement training loop ends here
// ==========================================================================

   outputfunc::print_elapsed_time();
   cout << "Final episode number = "
        << reinforce_agent_ptr->get_episode_number() << endl;

   delete ttt_ptr;
   delete reinforce_agent_ptr;
   delete reinforce_AI_ptr;
}
