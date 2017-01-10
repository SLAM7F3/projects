// ==========================================================================
// Program QSPACE solves the Space Invaders atari game via deep Q-learning.
// ==========================================================================
// Last updated on 12/9/16; 12/10/16; 12/13/16; 1/10/17
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "machine_learning/machinelearningfuncs.h"
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
   using std::ofstream;
   using std::string;
   using std::vector;
   
   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();
//   long s = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> s;
//   nrfunc::init_default_seed(s);

// Instantiate Space Invaders ALE game:

//   int n_screen_states = 1;
   int n_screen_states = 2;
//   int n_screen_states = 3;
   spaceinv *spaceinv_ptr = new spaceinv(n_screen_states);
   int n_actions = spaceinv_ptr->get_n_actions();

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::SPACEINV);
   game_world.set_spaceinv(spaceinv_ptr);

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

   int H1 = 256;
//   int H1 = 128;
//   int H1 = 24;
//   int H1 = 64;

   int H2 = 0;
//   int H2 = 8;
//   int H2 = 16;
//   int H2 = 32;
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

   int nframes_per_epoch = 50 * 1000;
   int n_max_epochs = 2000;
   
   int replay_memory_capacity = nframes_per_epoch * 4;
//   int replay_memory_capacity = nframes_per_epoch * 8;
   int eval_memory_capacity = basic_math::min(
      int(0.1 * replay_memory_capacity), 20000);

   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, 1, replay_memory_capacity, eval_memory_capacity,
      reinforce::RMSPROP);
   reinforce_agent_ptr->set_environment(&game_world);
   reinforce_agent_ptr->set_lambda(1E-2);
//   reinforce_agent_ptr->set_lambda(1E-3);
//   machinelearning_func::set_leaky_ReLU_small_slope(0.00); 
   machinelearning_func::set_leaky_ReLU_small_slope(0.01); 

// Initialize output subdirectory within an experiments folder:

   string experiments_subdir="./experiments/spaceinv/";
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
   spaceinv_ptr->set_screen_exports_subdir(screen_exports_subdir);

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
// value:<

   double min_learning_rate = 
      0.1 * reinforce_agent_ptr->get_base_learning_rate();

   int n_lr_episodes_period = 1 * 1000;

   int nn_update_frame_period = 5000;
   
//   int old_weights_period = 10; 
   int old_weights_period = 32;
//   int old_weights_period = 100;
//   int old_weights_period = 320;

   int n_update = 5;
   int n_summarize = 5;
   int n_snapshot = 50;

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

   bool export_frames_flag = false;
//   bool export_frames_flag = true;

   // Get the vector of minimal legal actions
   ActionVect minimal_actions = spaceinv_ptr->get_ale().getMinimalActionSet();

   string params_filename = output_subdir + "params.dat";
   reinforce_agent_ptr->summarize_parameters(params_filename);
   ofstream params_stream;
   filefunc::appendfile(params_filename, params_stream);
   params_stream << "Learning rate decrease period = " 
                 << n_lr_episodes_period << " episodes" << endl;
   params_stream << "Old weights period = " << old_weights_period << endl;

   params_stream << "Use big states flag = " << use_big_states_flag << endl;
   params_stream << "Frame skip = " << game_world.get_frame_skip() << endl;
   params_stream << "1 big state = n_screen_states = "
                 << spaceinv_ptr->get_n_screen_states() << endl;
   params_stream << "nn_update_frame_period = "
                 << nn_update_frame_period << endl;
   filefunc::closefile(params_filename, params_stream);

// ==========================================================================
// Reinforcement training loop starts here

   int cum_framenumber = 0;
   bool eval_memory_full_flag = false;

   while(reinforce_agent_ptr->get_curr_epoch() < n_max_epochs)
   {
      double curr_epoch = cum_framenumber / nframes_per_epoch;
      reinforce_agent_ptr->set_curr_epoch(curr_epoch);
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();

      bool random_start = false;
      game_world.start_new_episode(random_start);
      reinforce_agent_ptr->initialize_episode();

      if(curr_episode_number > 0 && 
         curr_episode_number%n_lr_episodes_period == 0)
      {
         reinforce_agent_ptr->decrease_learning_rate();
      }

// -----------------------------------------------------------------------
// Current episode starts here:

      cout << "************  Start of Game " << curr_episode_number
           << " ***********" << endl;

      int d = -1, n_state_updates = 0;
      int prev_a = 0;
      double cum_reward = 0;

// On 12/16/16, we discovered the hard way that the Arcade Learning
// Environment's getEpisodeFrameNumber() method does NOT always return
// contiguous increasing integers!  So we no longer use the following
// line:

      int curr_episode_framenumber = 0;  // since start of current episode

      while(!game_world.get_game_over())
      {
         bool state_updated_flag = false;

         curr_episode_framenumber++;
         cum_framenumber++;
         curr_epoch = double(cum_framenumber) / nframes_per_epoch;
         reinforce_agent_ptr->set_curr_epoch(curr_epoch);

         if(curr_episode_framenumber > 
            game_world.get_min_episode_framenumber())
         {
            if(cum_framenumber % game_world.get_frame_skip() == 0)
            {
               state_updated_flag = true;
               n_state_updates++;

               if(use_big_states_flag)
               {
                  spaceinv_ptr->crop_pool_curr_frame(export_frames_flag);
               }
               else
               {
                  spaceinv_ptr->crop_pool_difference_curr_frame(
                     export_frames_flag);
               }
            } // curr_frame_number % frame_skip == 0 conditional
         } // curr_frame_number > min_episode_framenumber

         if(state_updated_flag && n_state_updates > 2)
         {
            genvector* curr_s = NULL;
            if(use_big_states_flag)
            {
               curr_s = spaceinv_ptr->get_curr_big_state();
            }
            else
            {
               curr_s = game_world.get_curr_state();
            }

            d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
               *curr_s);
         }

         int curr_a = prev_a;
         if(state_updated_flag)
         {
            curr_a = reinforce_agent_ptr->select_action_for_curr_state();
            prev_a = curr_a;
         }
         Action a = minimal_actions[curr_a];
         double curr_reward = spaceinv_ptr->get_ale().act(a);
         double renorm_reward = curr_reward / 10.0;
         cum_reward += curr_reward;

         if(game_world.get_game_over())
         {

// Experiment with penalizing agent for dying (bad results so far)

//            const double death_penalty = 0;
//            const double death_penalty = 15;
//            renorm_reward -= death_penalty;

            reinforce_agent_ptr->store_final_arsprime_into_replay_memory(
               d, curr_a, renorm_reward);

            cout << "episode = " << curr_episode_number
                 << " frame = " << curr_episode_framenumber
                 << " curr_reward = " << curr_reward
                 << " renorm_reward = " << renorm_reward << endl;
         }
         else if (n_state_updates > 2)
         {
            genvector* next_s = game_world.compute_next_state(a);
            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, renorm_reward, *next_s, 
               game_world.get_game_over());
         }

      if(reinforce_agent_ptr->get_replay_memory_full() &&
         curr_episode_number % nn_update_frame_period == 0)
      {
         bool verbose_flag = false;
         if(curr_episode_number % 100 == 0)
         {
            verbose_flag = true;
         }
         total_loss = reinforce_agent_ptr->update_Q_network(verbose_flag);
      }

// Periodically save an episode's worth of screens to output
// subdirectory:

         bool export_RGB_screens_flag = false;
         if(curr_episode_number% 75 == 0) export_RGB_screens_flag = true;
         if(curr_frame_number < 110) export_RGB_screens_flag = false;
         if(export_RGB_screens_flag)
         {
            string curr_screen_filename="screen_"+
               stringfunc::integer_to_string(curr_episode_number,5)+"_"+
               stringfunc::integer_to_string(curr_episode_framenumber,5)
               +".png";
            spaceinv_ptr->save_screen(curr_screen_filename);
         }

      } // game_over while loop

// -----------------------------------------------------------------------

      cout << "Episode finished" << endl;
      cout << "  epoch = " << curr_epoch 
           << "  cum_frame = " << cum_framenumber << endl;
      cout << "  cum_reward = " << cum_reward 
           << "  epsilon = " << reinforce_agent_ptr->get_epsilon() 
           << "  n_backprops = " 
           << reinforce_agent_ptr->get_n_backprops() << endl;

      reinforce_agent_ptr->update_episode_history();
      reinforce_agent_ptr->update_epoch_history();
      reinforce_agent_ptr->update_n_frames_per_episode(
         curr_episode_framenumber);
      reinforce_agent_ptr->update_cumulative_reward(cum_reward);
      reinforce_agent_ptr->update_epsilon();

// Periodically copy current weights into old weights:

      update_old_weights_counter++;
      if(update_old_weights_counter%old_weights_period == 0)
      {
         reinforce_agent_ptr->copy_weights_onto_old_weights();
         update_old_weights_counter = 1;
      }

// Expontentially decay epsilon:

      reinforce_agent_ptr->exponentially_decay_epsilon(
         curr_episode_number, 30);

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
         reinforce_agent_ptr->store_quasirandom_weight_values();
         reinforce_agent_ptr->snapshot_cumulative_reward(cum_reward);
         reinforce_agent_ptr->generate_summary_plots(output_subdir, extrainfo);
      }

      if(curr_episode_number > 0 && curr_episode_number % n_snapshot == 0)
      {
         reinforce_agent_ptr->export_snapshot(output_subdir);

// Export trained weights in neural network's zeroth layer as
// greyscale images to output_subdir

         int n_reduced_xdim = spaceinv_ptr->get_n_reduced_xdim();
         int n_reduced_ydim = spaceinv_ptr->get_n_reduced_ydim();
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
   delete spaceinv_ptr;
}

