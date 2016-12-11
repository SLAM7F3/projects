// ==========================================================================
// Program QBREAK solves the BreakOut atari game via deep Q-learning.
// ==========================================================================
// Last updated on 12/10/16; 12/11/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "games/breakout.h"
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "machine_learning/reinforce.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

int main(int argc, char** argv) 
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();
//   long s = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> s;
//   nrfunc::init_default_seed(s);

// Instantiate Breakout ALE game:

//   int n_screen_states = 1;
   int n_screen_states = 2;
//   int n_screen_states = 3;
   breakout *breakout_ptr = new breakout(n_screen_states);
   int n_actions = breakout_ptr->get_n_actions();

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::BREAKOUT);
   game_world.set_breakout(breakout_ptr);

   bool use_big_states_flag = false;
   if(n_screen_states > 1)
   {
      use_big_states_flag = true;
   }
   game_world.set_use_big_states_flag(use_big_states_flag);
   game_world.set_frame_skip(3);

// Set neural network architecture parameters:

   int Din = game_world.get_curr_state()->get_mdim();   // Input layer dim
   cout << "Din = " << Din << endl;
   int Dout = n_actions;
   int n_max_episodes = 3 * 1000;
   int Tmax = n_max_episodes;

//   int H1 = 256;
   int H1 = 128;
//   int H1 = 64;
//   int H1 = 24;
//   int H1 = 32;
//   int H1 = 64;

//   int H2 = 0;
//   int H2 = 8;
//   int H2 = 16;
   int H2 = 32;
//   int H2 = 64;

   int H3 = 0;
//   int H3 = 8;

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

//   int replay_memory_capacity = 5 * 1000;
//   int replay_memory_capacity = 10 * 1000;
   int replay_memory_capacity = 15 * 1000;
//   int replay_memory_capacity = 20 * 1000;
   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, Tmax, 1, replay_memory_capacity,
//      reinforce::SGD);
//      reinforce::MOMENTUM);
//      reinforce::NESTEROV);
      reinforce::RMSPROP);
//      reinforce::ADAM);

//   reinforce_agent_ptr->set_debug_flag(true);
   reinforce_agent_ptr->set_environment(&game_world);

// Initialize output subdirectory within an experiments folder:

   string experiments_subdir="./experiments/breakout/";
   filefunc::dircreate(experiments_subdir);

   int expt_number;
   cout << "Enter experiment number:" << endl;
   cin >> expt_number;
   string output_subdir=experiments_subdir+
      "expt"+stringfunc::integer_to_string(expt_number,3)+"/";
   filefunc::dircreate(output_subdir);

   string weights_subdir = output_subdir+"zeroth_layer_weights/";
   filefunc::dircreate(weights_subdir);
   string screen_exports_subdir = output_subdir+"screen_exports/";
   filefunc::dircreate(screen_exports_subdir);
   breakout_ptr->set_screen_exports_subdir(screen_exports_subdir);

//   reinforce_agent_ptr->set_Nd(10);  // # samples to be drawn from replay mem
   reinforce_agent_ptr->set_Nd(16);  // # samples to be drawn from replay mem
//   reinforce_agent_ptr->set_Nd(32);  // # samples to be drawn from replay mem
   reinforce_agent_ptr->set_gamma(0.99); // discount reward factor
//   reinforce_agent_ptr->set_gamma(0.95); // discount reward factor
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);
//   reinforce_agent_ptr->set_rmsprop_decay_rate(0.95);

//   reinforce_agent_ptr->set_base_learning_rate(3E-3);
//   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_base_learning_rate(3E-4);  
//   reinforce_agent_ptr->set_base_learning_rate(2.5E-4);  
//   reinforce_agent_ptr->set_base_learning_rate(1E-4);

   reinforce_agent_ptr->set_epsilon_time_constant(200);
   double min_epsilon = 0.10;
   reinforce_agent_ptr->set_min_epsilon(min_epsilon);
   
// Periodically decrease learning rate down to some minimal floor
// value:

   double min_learning_rate = 
      0.1 * reinforce_agent_ptr->get_base_learning_rate();

   int n_lr_episodes_period = 1 * 1000;

//   int nn_update_frame_period = 100;
   int nn_update_frame_period = 500;
//   int nn_update_frame_period = 1000000;
   
//   int old_weights_period = 10; 
   int old_weights_period = 32;
//   int old_weights_period = 100;
//   int old_weights_period = 320;

// Fraction of zero-reward (S,A,R,S') states to NOT include within
// replay memory:

   const double discard_0_reward_frac = 0.95;  

   int n_update = 10;
   int n_summarize = 10;
   int n_snapshot = 100;

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
   ActionVect minimal_actions = breakout_ptr->get_ale().getMinimalActionSet();

   string params_filename = output_subdir + "params.dat";
   reinforce_agent_ptr->summarize_parameters(params_filename);
   ofstream params_stream;
   filefunc::appendfile(params_filename, params_stream);
   params_stream << "Learning rate decrease period = " 
                 << n_lr_episodes_period << " episodes" << endl;
   params_stream << "Old weights period = " << old_weights_period << endl;
   params_stream << "Discard zero reward frac = " 
                 << discard_0_reward_frac << endl;
   params_stream << "Use big states flag = " << use_big_states_flag << endl;
   params_stream << "Frame skip = " << game_world.get_frame_skip() << endl;
   params_stream << "1 big state = n_screen_states = "
                 << breakout_ptr->get_n_screen_states() << endl;
   params_stream << "nn_update_frame_period = "
                 << nn_update_frame_period << endl;
   filefunc::closefile(params_filename, params_stream);

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

      if(curr_episode_number > 0 && curr_episode_number%n_lr_episodes_period 
         == 0)
      {
         double curr_learning_rate = reinforce_agent_ptr->get_learning_rate();
         if(curr_learning_rate > min_learning_rate)
         {
            reinforce_agent_ptr->set_learning_rate(0.8 * curr_learning_rate);
            n_lr_episodes_period *= 1.2;
         }
      }

// -----------------------------------------------------------------------
// Current episode starts here:

      cout << "************  Start of Game " << curr_episode_number
           << " ***********" << endl;

      int prev_nlives = breakout_ptr->get_ale().lives();
      int d = -1, n_state_updates = 0;
      int prev_a = 0;
      double cum_reward = 0;

      while(!game_world.get_game_over())
      {
         bool state_updated_flag = false;
         int curr_frame_number = game_world.get_episode_framenumber();
//         cout << "curr_frame_number = " << curr_frame_number
//              << " n_lives = " << breakout_ptr->get_ale().lives() << endl;
         
         if(curr_frame_number > game_world.get_min_episode_framenumber())
         {
            if(curr_frame_number % game_world.get_frame_skip() == 0)
            {
               state_updated_flag = true;
               n_state_updates++;

               if(use_big_states_flag)
               {
                  breakout_ptr->crop_pool_curr_frame(export_frames_flag);
               }
               else
               {
                  breakout_ptr->crop_pool_difference_curr_frame(
                     export_frames_flag);
               }
            } // curr_frame_number % frame_skip == 0 conditional
         } // curr_frame_number > min_episode_framenumber

         if(state_updated_flag && n_state_updates > 2)
         {
            genvector* curr_s = NULL;
            if(use_big_states_flag)
            {
               curr_s = breakout_ptr->get_curr_big_state();
            }
            else
            {
               curr_s = game_world.get_curr_state();
            }

            d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
               *curr_s);
            if(d%1000 == 0) cout << "d = " << d << endl;
         }

         int curr_a = prev_a;
         if(state_updated_flag)
         {
            curr_a = reinforce_agent_ptr->select_action_for_curr_state();
            prev_a = curr_a;
         }
         Action a = minimal_actions[curr_a];
         double curr_reward = breakout_ptr->get_ale().act(a);
         cum_reward += curr_reward;

         double renorm_reward = 0;
         if(curr_reward > 0)
         {
            renorm_reward = 1;
         }

// Penalize agent whenever it misses the ball:

         int curr_nlives = breakout_ptr->get_ale().lives();
         if(curr_nlives < prev_nlives)
         {
            renorm_reward = -1;
            prev_nlives = curr_nlives;
         }

// Experiment with rewarding agent for living (bad results so far)

         const double live_timestep_reward = 0.0;
//         double live_timestep_reward = 50.0 / 2000.0;
//         renorm_reward += live_timestep_reward;

         reinforce_agent_ptr->record_reward_for_action(curr_reward);
         reinforce_agent_ptr->increment_time_counters();

         if(!nearly_equal(renorm_reward, live_timestep_reward))
         {
            cout << "episode = " << curr_episode_number
                 << " frame = " << curr_frame_number
                 << " curr_reward = " << curr_reward
                 << " renorm_reward = " << renorm_reward << endl;
         }

         if(game_world.get_game_over())
         {

// Experiment with penalizing agent for dying (bad results so far)

//            const double death_penalty = 0;
//            const double death_penalty = 15;
//            renorm_reward -= death_penalty;

            reinforce_agent_ptr->store_final_arsprime_into_replay_memory(
               d, curr_a, renorm_reward);

            cout << "episode = " << curr_episode_number
                 << " frame = " << curr_frame_number
                 << " curr_reward = " << curr_reward
                 << " renorm_reward = " << renorm_reward << endl;
         }
         else if (n_state_updates > 2)
         {
            genvector* next_s = game_world.compute_next_state(a);

// As of 6:45 am on Mon Dec 5, we experiment with discarding some
// fraction of zero reward states in order to increase chances of
// agent seeing non-zero reward states without having to perform a
// huge number of expensive backpropagations:

            if(nearly_equal(renorm_reward, live_timestep_reward))
            {
               if(nrfunc::ran1() > 1 - discard_0_reward_frac) continue;
            }

            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, renorm_reward, *next_s, 
               game_world.get_game_over());
         }

      if(reinforce_agent_ptr->get_replay_memory_full() &&
         curr_episode_number % nn_update_frame_period == 0)
      {
         bool verbose_flag = true;
         total_loss = reinforce_agent_ptr->update_neural_network(
            verbose_flag);
      }

// Periodically save an episode's worth of screens to output
// subdirectory:

         bool export_RGB_screens_flag = false;
         if(curr_episode_number% 200 == 0) export_RGB_screens_flag = true;
         if(curr_frame_number < 110) export_RGB_screens_flag = false;
         if(export_RGB_screens_flag)
         {
            string curr_screen_filename="screen_"+
               stringfunc::integer_to_string(curr_episode_number,5)+"_"+
               stringfunc::integer_to_string(curr_frame_number,5)+".png";
            breakout_ptr->save_screen(curr_screen_filename);
         }

      } // game_over while loop

// -----------------------------------------------------------------------

      cout << "Episode finished" << endl;
      cout << "  cum_reward = " << cum_reward << endl;
      cout << "  epsilon = " << reinforce_agent_ptr->get_epsilon() << endl;

//       breakout_ptr->mu_and_sigma_for_pooled_zvalues();

      reinforce_agent_ptr->update_T_values();
      reinforce_agent_ptr->update_running_reward(n_update);
      reinforce_agent_ptr->append_n_episode_frames(
         game_world.get_episode_framenumber());
      reinforce_agent_ptr->append_epsilon();
      reinforce_agent_ptr->increment_episode_number();      

// Periodically copy current weights into old weights:

      update_old_weights_counter++;
      if(update_old_weights_counter%old_weights_period == 0)
      {
         reinforce_agent_ptr->copy_weights_onto_old_weights();
         update_old_weights_counter = 1;
      }

      if(reinforce_agent_ptr->get_replay_memory_full())
      {
         bool verbose_flag = true;
         total_loss = reinforce_agent_ptr->update_neural_network(
            verbose_flag);
      }

// Expontentially decay epsilon:

      reinforce_agent_ptr->exponentially_decay_epsilon(
         curr_episode_number, 40);

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
         reinforce_agent_ptr->plot_reward_history(output_subdir, extrainfo);
         reinforce_agent_ptr->plot_epsilon_history(output_subdir, extrainfo);
         reinforce_agent_ptr->plot_frames_history(output_subdir, extrainfo);
         reinforce_agent_ptr->plot_log10_loss_history(
            output_subdir, extrainfo);
      }

      if(curr_episode_number > 0 && curr_episode_number % n_snapshot == 0)
      {
         reinforce_agent_ptr->export_snapshot(output_subdir);

// Export trained weights in neural network's zeroth layer as
// greyscale images to output_subdir

         int n_reduced_xdim = breakout_ptr->get_n_reduced_xdim();
         int n_reduced_ydim = breakout_ptr->get_n_reduced_ydim();
         if(use_big_states_flag)
         {
            n_reduced_ydim *= n_screen_states;
         }
         reinforce_agent_ptr->plot_zeroth_layer_weights(
            n_reduced_xdim, n_reduced_ydim, weights_subdir);
      }

   } // n_episodes < n_max_episodes while loop

// Reinforcement training loop ends here
// ==========================================================================

   outputfunc::print_elapsed_time();
   cout << "Final episode number = "
        << reinforce_agent_ptr->get_episode_number() << endl;
   cout << "N_weights = " << reinforce_agent_ptr->count_weights()
        << endl;

   delete reinforce_agent_ptr;
   delete breakout_ptr;
}

