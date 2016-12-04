// ==========================================================================
// Program QSPACE solves the Space Invaders atari game via deep Q-learning.
// ==========================================================================
// Last updated on 12/1/16; 12/2/16; 12/3/16; 12/4/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "machine_learning/reinforce.h"
#include "games/spaceinv.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

int main(int argc, char** argv) 
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();
//   long s = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> s;
//   nrfunc::init_default_seed(s);

// Instantiate Space Invaders ALE game:

   spaceinv *spaceinv_ptr = new spaceinv();
   int n_actions = 6;

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::SPACEINV);
   game_world.set_spaceinv(spaceinv_ptr);

// Set neural network architecture parameters:

   int Din = spaceinv_ptr->get_curr_state()->get_mdim(); // Input dim
   cout << "Din = " << Din << endl;
   int Dout = n_actions;
   int n_max_episodes = 50 * 1000;
   int Tmax = n_max_episodes;

   int H1 = 16;
   int H2 = 8;
   int H3 = 0;

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

   cout << "Din * H1 + H1 * H2 + H2 * Dout = "
        << Din * H1 + H1 * H2 + H2 * Dout << endl;

// Construct reinforcement learning agent:

   int batch_size = 5;
//   int replay_memory_capacity = batch_size * 100;
   int replay_memory_capacity = 50;
   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, Tmax, batch_size, replay_memory_capacity,
//      reinforce::SGD);
//      reinforce::MOMENTUM);
//      reinforce::NESTEROV);
      reinforce::RMSPROP);
//      reinforce::ADAM);

   reinforce_agent_ptr->set_debug_flag(true);
   reinforce_agent_ptr->set_environment(&game_world);

// Initialize output subdirectory within an experiments folder:

   string experiments_subdir="./experiments/spaceinv/";
   filefunc::dircreate(experiments_subdir);

   int expt_number;
   cout << "Enter experiment number:" << endl;
   cin >> expt_number;
   string output_subdir=experiments_subdir+
      "expt"+stringfunc::integer_to_string(expt_number,3)+"/";
   filefunc::dircreate(output_subdir);

// Gamma = discount factor for reward:

   reinforce_agent_ptr->set_gamma(0.95);
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);
   reinforce_agent_ptr->set_base_learning_rate(1E-3);
//   reinforce_agent_ptr->set_base_learning_rate(3E-4);  
//   reinforce_agent_ptr->set_base_learning_rate(1E-4);

// Periodically decrease learning rate down to some minimal floor
// value:

   double min_learning_rate = 
      0.1 * reinforce_agent_ptr->get_base_learning_rate();

   int n_episodes_period = 1 * 1000;
   int old_weights_period = 10; 
//   int old_weights_period = 32;  

//   double min_epsilon = 0.01;	
   double min_epsilon = 0.025;
//   double min_epsilon = 0.05; 

   int n_anneal_steps = 5;
   int n_update = 2;
   int n_summarize = 4;
   int n_snapshot = 10;

   string subtitle=
      "old weights T="+stringfunc::number_to_string(old_weights_period)
      +";min eps="+stringfunc::number_to_string(min_epsilon);
   string extrainfo="H1="+stringfunc::number_to_string(H1);
   if(H2 > 0)
   {
      extrainfo += ";H2="+stringfunc::number_to_string(H2);
   }
   if(H3 > 0)
   {
      extrainfo += ";H3="+stringfunc::number_to_string(H3);
   }

   int update_old_weights_counter = 0;
   double total_loss = -1;

//   bool export_frames_flag = false;
   bool export_frames_flag = true;

   // Get the vector of minimal legal actions
   ActionVect minimal_actions = spaceinv_ptr->get_ale().getMinimalActionSet();

// ==========================================================================
// Reinforcement training loop starts here

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes)
   {
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      cout << "Starting episode " << curr_episode_number << endl;
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_update, n_max_episodes);

      bool random_start = false;
      game_world.start_new_episode(random_start);
      reinforce_agent_ptr->initialize_episode();

      if(curr_episode_number > 0 && curr_episode_number%n_episodes_period == 0)
      {
         double curr_learning_rate = reinforce_agent_ptr->get_learning_rate();
         if(curr_learning_rate > min_learning_rate)
         {
            reinforce_agent_ptr->set_learning_rate(0.8 * curr_learning_rate);
            n_episodes_period *= 1.2;
         }
      }

// -----------------------------------------------------------------------
// Current episode starts here:

      cout << "************  Start of Game " << curr_episode_number
           << " ***********" << endl;

      int d = -1, n_state_updates = 0;
      int prev_a = 0;
      double cum_reward = 0;
      double renorm_cum_reward = 0;
      genvector* curr_diff_s = NULL;
      while(!game_world.get_game_over())
      {
         bool state_updated_flag = false;
         int curr_frame_number = game_world.get_episode_framenumber();
         
         if(curr_frame_number > game_world.get_min_episode_framenumber())
         {
            if(curr_frame_number % game_world.get_frame_skip() == 0)
            {
               spaceinv_ptr->crop_pool_difference_curr_frame(
                  export_frames_flag);
               state_updated_flag = true;
               n_state_updates++;
               curr_diff_s = game_world.get_curr_state();
            } // curr_frame_number % frame_skip == 0 conditional
         } // curr_frame_number > min_episode_framenumber

         if(state_updated_flag && n_state_updates > 2)
         {
            d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
               *curr_diff_s);
         }

         int curr_a = prev_a;
         if(state_updated_flag)
         {
            curr_a = reinforce_agent_ptr->select_action_for_curr_state();
            prev_a = curr_a;
         }
         Action a = minimal_actions[curr_a];
         double curr_reward = spaceinv_ptr->get_ale().act(a);
         double renorm_reward = curr_reward /
            game_world.get_max_score_per_episode();
         cum_reward += curr_reward;

         reinforce_agent_ptr->record_reward_for_action(curr_reward);
         reinforce_agent_ptr->increment_time_counters();

         if(!state_updated_flag && !game_world.get_game_over()) continue;

         game_world.compute_next_state(a);

         if(game_world.get_game_over())
         {
            reinforce_agent_ptr->store_final_arsprime_into_replay_memory(
               d, curr_a, renorm_reward);
         }
         else if (n_state_updates > 2)
         {
            genvector *next_diff_s = spaceinv_ptr->get_next_state();
            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, renorm_reward, *next_diff_s, 
               game_world.get_game_over());
         }
      } // game_over while loop

// -----------------------------------------------------------------------

      cout << "Episode finished" << endl;
      cout << "cum_reward = " << cum_reward << endl;

      reinforce_agent_ptr->increment_episode_number();
      reinforce_agent_ptr->update_T_values();
      reinforce_agent_ptr->update_running_reward(extrainfo, n_update);

// Periodically copy current weights into old weights:

      update_old_weights_counter++;
      if(update_old_weights_counter%old_weights_period == 0)
      {
         reinforce_agent_ptr->copy_weights_onto_old_weights();
         update_old_weights_counter = 1;
      }

      if(reinforce_agent_ptr->get_replay_memory_full() && 
         curr_episode_number % reinforce_agent_ptr->get_batch_size() == 0)
      {
         total_loss = reinforce_agent_ptr->update_neural_network();
      }

// Periodically anneal epsilon:

      if(curr_episode_number > 0 && curr_episode_number % n_anneal_steps == 0)
      {
//         double decay_factor = 0.99; 
         double decay_factor = 0.95;
//         double decay_factor = 0.90; 
         reinforce_agent_ptr->anneal_epsilon(decay_factor, min_epsilon);
      }

// Periodically write status info to text console:

      if(curr_episode_number >= n_update - 1 && 
         curr_episode_number % n_update == 0)
      {
         cout << "Q-learning" << endl;
         cout << "  Total loss = " << total_loss << endl;
         if(total_loss > 0)
         {
            cout << " log10(total_loss) = " << log10(total_loss) << endl;
            reinforce_agent_ptr->push_back_log10_loss(log10(total_loss));
         }

         if(reinforce_agent_ptr->get_include_bias_terms()){
           reinforce_agent_ptr->compute_bias_distributions();
         }
         reinforce_agent_ptr->compute_weight_distributions();
      }

      if(curr_episode_number >= n_summarize - 1 && 
         curr_episode_number % n_summarize == 0)
      {
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->plot_weight_distributions(
            output_subdir, extrainfo);
         reinforce_agent_ptr->snapshot_running_reward();
         reinforce_agent_ptr->plot_reward_history(
            output_subdir, extrainfo, 0, 1);
         reinforce_agent_ptr->plot_frames_history(output_subdir, extrainfo);
         reinforce_agent_ptr->plot_log10_loss_history(
            output_subdir, extrainfo);
      }

      if(curr_episode_number > 0 && curr_episode_number % n_snapshot == 0)
      {
         reinforce_agent_ptr->export_snapshot(output_subdir);
      }
      
   } // n_episodes < n_max_episodes while loop

// Reinforcement training loop ends here
// ==========================================================================

   outputfunc::print_elapsed_time();
   cout << "Final episode number = "
        << reinforce_agent_ptr->get_episode_number() << endl;
   cout << "N_weights = " << reinforce_agent_ptr->count_weights()
        << endl;

// Export trained weights in neural network's zeroth layer as
// greyscale images to output_subdir

   string weights_subdir = output_subdir+"zeroth_layer_weights/";
   filefunc::dircreate(weights_subdir);
   reinforce_agent_ptr->plot_zeroth_layer_weights(weights_subdir);


   delete reinforce_agent_ptr;
   delete spaceinv_ptr;
}

