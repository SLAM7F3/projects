// ==========================================================================
// Program PBREAK solves the BreakOut atari game via policy gradients
// ==========================================================================
// Last updated on 12/21/16
// ==========================================================================

// Note: On 12/17/16, we learned the hard and painful way that left
// and right actions continue to move the paddle even after it's hit a
// sidewall.  So the paddle's position can have values exceeding the
// visible part of the game board.  In these cases, the paddle appears
// to be "pinned" to the side wall.  

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "games/breakout.h"
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "machine_learning/machinelearningfuncs.h"
#include "numrec/nrfuncs.h"
#include "machine_learning/reinforce.h"
#include "general/sysfuncs.h"
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
   std::set_new_handler(sysfunc::out_of_memory);      

   timefunc::initialize_timeofday_clock();
   long seed = nrfunc::init_time_based_seed();
//   long seed = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> seed;
//   nrfunc::init_default_seed(seed);

// Instantiate Breakout ALE game:

   int n_screen_states = 1;
   breakout *breakout_ptr = new breakout(n_screen_states);
   int n_actions = breakout_ptr->get_n_actions();

   breakout_ptr->set_compute_difference_flag(true);
   
// Disable ALE's random responsiveness to input actions:

   breakout_ptr->get_ale().setFloat("repeat_action_probability",0);

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::BREAKOUT);
   game_world.set_breakout(breakout_ptr);

   bool use_big_states_flag = false;
   game_world.set_use_big_states_flag(use_big_states_flag);

// Set neural network architecture parameters:

   int Din = game_world.get_curr_state()->get_mdim();   // Input layer dim
   cout << "Din = " << Din << endl;
   int Dout = n_actions;

//   int H1 = 8;
//   int H1 = 16;
//   int H1 = 32;
   int H1 = 64;
//   int H1 = 128;

//   int H2 = 0;
//   int H2 = 8;
//   int H2 = 16;
//   int H2 = 32;
   int H2 = 64;
//   int H2 = 128;

   int H3 = 0;
//   int H3 = 16;
//   int H3 = 32;
//   int H3 = 64;

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
   int n_max_epochs = 3000;

   int replay_memory_capacity = 4 * 1000;
//   int replay_memory_capacity = 10 * 1000;
   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, replay_memory_capacity, reinforce::RMSPROP);

   reinforce_agent_ptr->set_environment(&game_world);
//   reinforce_agent_ptr->set_lambda(0.0);
   reinforce_agent_ptr->set_lambda(1E-2);
//   reinforce_agent_ptr->set_lambda(1E-3);
//   machinelearning_func::set_leaky_ReLU_small_slope(0.00); 
   machinelearning_func::set_leaky_ReLU_small_slope(0.01); 

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

   reinforce_agent_ptr->set_gamma(0.99); // discount reward factor
//   reinforce_agent_ptr->set_gamma(0.95); // discount reward factor
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);
//   reinforce_agent_ptr->set_rmsprop_decay_rate(0.95);

   reinforce_agent_ptr->set_base_learning_rate(3E-3);
//   reinforce_agent_ptr->set_base_learning_rate(1E-3);
//   reinforce_agent_ptr->set_base_learning_rate(3E-4);  

// Periodically decrease learning rate down to some minimal floor
// value:

   double min_learning_rate = 
      0.1 * reinforce_agent_ptr->get_base_learning_rate();

   int n_lr_episodes_period = 1 * 1000;
//    int n_snapshot = 500;
   int n_episode_update = 100;

   string extrainfo="H1="+stringfunc::number_to_string(H1);
   if(H2 > 0)
   {
      extrainfo += ";H2="+stringfunc::number_to_string(H2);
   }
   if(H3 > 0)
   {
      extrainfo += ";H3="+stringfunc::number_to_string(H3);
   }

   double total_loss = -1;

   bool export_frames_flag = false;
//   bool export_frames_flag = true;

   // Set vector of minimal legal actions:

   ActionVect minimal_actions;
   if(n_actions >= 3)
   {
      minimal_actions.push_back(PLAYER_A_NOOP);  
   }
   minimal_actions.push_back(PLAYER_A_RIGHT);
   minimal_actions.push_back(PLAYER_A_LEFT);

// Generate text file summary of parameter values:

   string params_filename = output_subdir + "params.dat";
   reinforce_agent_ptr->summarize_parameters(params_filename);
   ofstream params_stream;
   filefunc::appendfile(params_filename, params_stream);

   params_stream << "n_actions = " << n_actions << endl;
   params_stream << "Leaky ReLU small slope = "
                 << machinelearning_func::get_leaky_ReLU_small_slope() << endl;
//   params_stream << "Learning rate decrease period = " 
//                 << n_lr_episodes_period << " episodes" << endl;
   params_stream << "nframes / epoch = " << nframes_per_epoch << endl;
   params_stream << "n_max_epochs = " << n_max_epochs << endl;
   params_stream << "Random seed = " << seed << endl;
   filefunc::closefile(params_filename, params_stream);

// ==========================================================================
// Reinforcement training loop starts here

   const int n_fire_ball_frames = 2;
   int cum_framenumber = 0;

   while(reinforce_agent_ptr->get_curr_epoch() < n_max_epochs)
   {
      double curr_epoch = cum_framenumber / nframes_per_epoch;
      reinforce_agent_ptr->set_curr_epoch(curr_epoch);
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();

      bool random_start = false;
      game_world.start_new_episode(random_start);
      reinforce_agent_ptr->initialize_episode();

/*

// Periodically decrease learning rate:

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
*/

// -----------------------------------------------------------------------
// Current episode starts here:

      cout << "************  Start of episode " << curr_episode_number
           << " ***********" << endl;

      int d = -1, n_state_updates = 0;
      int prev_a = 0;
      int curr_a = -1;
      Action a;
      double prob_a;
      int n_prev_lives = -1;
      double cum_reward = 0;

// As of 12/17/16, we reset paddle's starting position to be at the
// gameboard's horizontal center:

      breakout_ptr->set_paddle_x(
         breakout_ptr->get_default_starting_paddle_x());
      int n_recenter_paddle_frames = 
         breakout_ptr->get_default_starting_paddle_x()  
         - breakout_ptr->get_center_paddle_x();

// On 12/16/16, we discovered the hard way that the Arcade Learning
// Environment's getEpisodeFrameNumber() method does NOT always return
// contiguous increasing integers!  So we no longer use the following
// line:

//  int curr_frame_number = game_world.get_episode_framenumber();

      int curr_episode_framenumber = 0;  // since start of current episode
      int curr_life_framenumber = 0;  // n_frames since start of current life

      while(!game_world.get_game_over())
      {
         int n_curr_lives = breakout_ptr->get_ale().lives();
         bool state_updated_flag = false;
         bool zero_input_state = false;

         curr_episode_framenumber++;
         curr_life_framenumber++;
         cum_framenumber++;
         curr_epoch = double(cum_framenumber) / nframes_per_epoch;
         reinforce_agent_ptr->set_curr_epoch(curr_epoch);

         if(breakout_ptr->crop_pool_difference_curr_frame(export_frames_flag))
         {
            state_updated_flag = true;
            n_state_updates++;
         }

         if(state_updated_flag && n_state_updates > 2)
         {
            genvector* curr_s = game_world.get_curr_state();

// If curr_s == 0, do NOT store it into the replay memory:

            if(curr_s->magnitude() <= 0)
            {
               zero_input_state = true;
//               cout << " Zero input state detected" << endl;
            }
            else
            {
               d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
                  *curr_s);
            }
         } // state_updated_flag && n_state_updates > 2 conditional

// First reposition paddle so that it starts at screen's horizontal center:

         if(curr_life_framenumber < n_recenter_paddle_frames)
         {
            a = PLAYER_A_LEFT;  // move paddle towards center
         }

// Next fire ball:

         else if(curr_life_framenumber < 
                 n_recenter_paddle_frames + n_fire_ball_frames)
         {
            a = PLAYER_A_FIRE;  // fire ball
         }

// Now start playing game:

         else
         {
            curr_a = prev_a;
            if(state_updated_flag)
            {
               curr_a = reinforce_agent_ptr->get_P_action_for_curr_state(
                  prob_a);
               prev_a = curr_a;
            }
            a = minimal_actions[curr_a];
         }

// As of 12/18/16 we do not not allow the paddle to move beyond the
// right or left walls:
         if(a == PLAYER_A_RIGHT)
         {
            if(!breakout_ptr->increment_paddle_x())
            {
               a = PLAYER_A_NOOP;
            }
         }
         else if (a == PLAYER_A_LEFT)
         {
            if(!breakout_ptr->decrement_paddle_x())
            {
               a = PLAYER_A_NOOP;
            }
         }

         double curr_reward = breakout_ptr->get_ale().act(a);
         cum_reward += curr_reward;

         double renorm_reward = 0;
         if(curr_reward > 0)
         {
            renorm_reward = 1;
         }

         if(n_curr_lives != n_prev_lives)
         {

/*
// Penalize agent whenever it misses the ball:

            if(n_prev_lives > 0)
            {
               renorm_reward = -1;
            }
*/

            n_prev_lives = n_curr_lives;
            curr_life_framenumber = 0;
//            cout << "n_curr_lives = " << n_curr_lives
//                 << " n_prev_lives = " << n_prev_lives
//                 << " renorm_reward = " << renorm_reward << endl;
         }

         if(d >= 0 && game_world.get_game_over())
         {
            reinforce_agent_ptr->store_final_arsprime_into_replay_memory(
               d, curr_a, renorm_reward);
            reinforce_agent_ptr->store_action_prob_into_replay_memory(
               d, prob_a);
         
         }
         else if (d >= 0 && state_updated_flag && n_state_updates > 2 && 
                  curr_a >= 0 && !zero_input_state)
         {
            genvector* next_s = game_world.compute_next_state(a);
            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, renorm_reward, *next_s, 
               game_world.get_game_over());
            reinforce_agent_ptr->store_action_prob_into_replay_memory(
               d, prob_a);
         }

// Update P-network when replay memory becomes completely full:

         if(cum_framenumber % replay_memory_capacity == 0)
         {
            bool verbose_flag = false;
            if(curr_episode_number % 10 == 0)
            {
               verbose_flag = true;
            }
            total_loss = reinforce_agent_ptr->update_P_network(verbose_flag);
         }

// Periodically save an episode's worth of screens to output
// subdirectory:

         bool export_RGB_screens_flag = false;
         if(curr_episode_number % 1000 == 0) export_RGB_screens_flag = true;

         if(export_RGB_screens_flag)
         {
            string curr_screen_filename="screen_"+
               stringfunc::integer_to_string(curr_episode_number,5)+"_"+
               stringfunc::integer_to_string(curr_episode_framenumber,5)+
               ".png";
            breakout_ptr->save_screen(
               curr_episode_number, curr_screen_filename);
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

//       breakout_ptr->mu_and_sigma_for_pooled_zvalues();

      reinforce_agent_ptr->append_n_frames_per_episode(
         curr_episode_framenumber);
      reinforce_agent_ptr->snapshot_cumulative_reward(cum_reward);
      reinforce_agent_ptr->increment_episode_number();      

      if(total_loss > 0)
      {
         reinforce_agent_ptr->push_back_log10_loss(log10(total_loss));
      }
      
// Periodically export status info:

      if(curr_episode_number >= n_episode_update - 1 && 
         curr_episode_number % n_episode_update == 0)
      {
         outputfunc::print_elapsed_time();
         if(reinforce_agent_ptr->get_include_bias_terms()){
            reinforce_agent_ptr->compute_bias_distributions();
         }
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->store_quasirandom_weight_values();
         reinforce_agent_ptr->generate_summary_plots(output_subdir, extrainfo);

// Export trained weights in neural network's zeroth layer as
// colored images to output_subdir

         int n_reduced_xdim = breakout_ptr->get_n_reduced_xdim();
         int n_reduced_ydim = breakout_ptr->get_n_reduced_ydim();
         if(use_big_states_flag)
         {
            n_reduced_ydim *= n_screen_states;
         }
         reinforce_agent_ptr->plot_zeroth_layer_weights(
            n_reduced_xdim, n_reduced_ydim, weights_subdir);
      }

/*
      if(curr_episode_number > 0 && curr_episode_number % n_snapshot == 0)
      {
         n_snapshot *= 2;
         reinforce_agent_ptr->export_snapshot(output_subdir);
      }
*/

   } // curr_epoch < n_max_epochs while loop

// Reinforcement training loop ends here
// ==========================================================================

   delete reinforce_agent_ptr;
   delete breakout_ptr;
}

