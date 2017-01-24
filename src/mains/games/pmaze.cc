// ==========================================================================
// Program PMAZE solves a maze via policy gradient learning
// ==========================================================================
// Last updated on 1/11/17; 1/18/17; 1/23/17; 1/24/17
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "machine_learning/machinelearningfuncs.h"
#include "games/maze.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "machine_learning/reinforce.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory); 

   timefunc::initialize_timeofday_clock();
//   long seed = nrfunc::init_time_based_seed();
   long seed = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> seed;
   nrfunc::init_default_seed(seed);

   int n_grid_size = 2;
   cout << "Enter grid size:" << endl;
   cin >> n_grid_size;
   int n_actions = 4;

// Construct one particular maze:

   maze curr_maze(n_grid_size);
   curr_maze.generate_maze();
   curr_maze.solve_maze_backwards();
   curr_maze.generate_all_turtle_states();

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::MAZE);
   game_world.set_maze(&curr_maze);

// Set neural network architecture parameters:

   int Din = curr_maze.get_occupancy_state()->get_mdim(); // Input dim
   int Dout = n_actions;

//   int H1 = 8;
//   int H1 = 10;
//   int H1 = 20;
   int H1 = 32;     // 10x10 grids

//   int H2 = 0;
//   int H2 = 8;
//   int H2 = 10;
   int H2 = 16;    // 10x10 grids

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

//   int n_rollouts = 10;
//   int n_rollouts = 32;
   int n_rollouts = 100;
//   int n_rollouts = 1000;
   bool include_biases = true;
   int replay_memory_capacity = n_rollouts;
   reinforce* reinforce_agent_ptr = new reinforce(
      include_biases, layer_dims, replay_memory_capacity, reinforce::RMSPROP);

   reinforce_agent_ptr->set_environment(&game_world);
//   reinforce_agent_ptr->set_lambda(0);
   reinforce_agent_ptr->set_lambda(1E-2);
   machinelearning_func::set_leaky_ReLU_small_slope(0.01); 

   curr_maze.set_qmap_ptr(reinforce_agent_ptr->get_qmap_ptr());

// Initialize output subdirectory within an experiments folder:

   int output_counter = 0;
   string experiments_subdir="./experiments/mazes/";
   filefunc::dircreate(experiments_subdir);

   int expt_number;
   cout << "Enter experiment number:" << endl;
   cin >> expt_number;
   string output_subdir=experiments_subdir+
      "expt"+stringfunc::integer_to_string(expt_number,3)+"/";
   filefunc::dircreate(output_subdir);
   reinforce_agent_ptr->set_output_subdir(output_subdir);

//   reinforce_agent_ptr->set_gamma(0.9);  // reward discount factor
   reinforce_agent_ptr->set_gamma(0.95);  // reward discount factor
//   reinforce_agent_ptr->set_gamma(0.99);  // reward discount factor
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);
//   reinforce_agent_ptr->set_base_learning_rate(1E-2);
   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_max_mean_KL_divergence(1E32);

   int n_max_episodes = 1 * 1000 * 1000;
   int n_lr_episodes_period = 1000 * 1000;

//   int n_update = 100;
   int n_update = 1000;
   int n_progress = 100 * 1000;
   double Qmap_score = -1;

   string basename = "maze";
   bool display_qmap_flag = true;
   reinforce_agent_ptr->compute_deep_Qvalues();
//   reinforce_agent_ptr->print_Qmap();
   curr_maze.compute_max_Qmap();
   curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                      display_qmap_flag);

   string subtitle="Nsize="+stringfunc::number_to_string(n_grid_size);
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

// Generate text file summary of parameter values:

   reinforce_agent_ptr->summarize_parameters();
   ofstream params_stream;
   filefunc::appendfile(
      reinforce_agent_ptr->get_params_filename(), params_stream);

   params_stream << "n_actions = " << n_actions << endl;
   params_stream << "Leaky ReLU small slope = "
                 << machinelearning_func::get_leaky_ReLU_small_slope() << endl;
   params_stream << "Random seed = " << seed << endl;
   filefunc::closefile(reinforce_agent_ptr->get_params_filename(), 
                       params_stream);

// ==========================================================================
// Reinforcement training loop starts here

   int cum_framenumber = 0;      

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes &&
         Qmap_score < 0.999999)
   {
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
//      outputfunc::update_progress_and_remaining_time(
//         curr_episode_number, n_progress, n_max_episodes);

      bool random_turtle_start = true;
      game_world.start_new_episode(random_turtle_start);
      reinforce_agent_ptr->initialize_episode();

// Periodically decrease learning rate:

      if(curr_episode_number > 0 && 
         curr_episode_number%n_lr_episodes_period == 0)
      {
         reinforce_agent_ptr->decrease_learning_rate();
      }

// -----------------------------------------------------------------------
// Current episode starts here:

//      cout << "************  Start of episode " << curr_episode_number
//           << " ***********" << endl;
//      curr_maze.print_occupancy_grid();
//      curr_maze.print_occupancy_state();

      int d = -98;
      double reward;
      double cum_reward = 0;

      while(!game_world.get_game_over())
      {
         cum_framenumber++;

         genvector *curr_s = game_world.get_curr_state();
         d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
            *curr_s);

         genvector *curr_pi = reinforce_agent_ptr->get_curr_s_sample();
         reinforce_agent_ptr->compute_pi_given_state(curr_s, curr_pi);
         reinforce_agent_ptr->store_curr_pi_into_replay_memory(d, curr_pi);
         
         double ran_value = nrfunc::ran1();
         double action_prob;
         int curr_a = reinforce_agent_ptr->get_P_action_given_pi(
            curr_pi, ran_value, action_prob);

         if(!game_world.is_legal_action(curr_a))
         {

// Penalize agent whenever it crashes into a wall:

            reward = -1;
            game_world.set_game_over(true);
         }
         else
         {
            game_world.compute_next_state(curr_a);
            reward = curr_maze.compute_turtle_reward();
         } // curr_a is legal action conditional

//         cout << "   reward = " << reward 
//              << " game over = " << game_world.get_game_over() << endl;
         cum_reward += reward;

         if(d >= 0)
         {
            reinforce_agent_ptr->store_ar_into_replay_memory(
               d, curr_a, reward, game_world.get_game_over());
         } // d >= 0 conditional

// Update P-network when replay memory becomes completely full:

         if(cum_framenumber % replay_memory_capacity == 0)
         {
            bool verbose_flag = false;
            if(reinforce_agent_ptr->get_n_backprops() % (100 * 1000) == 0)
            {
//               verbose_flag = true;
            }
            total_loss = reinforce_agent_ptr->update_P_network(verbose_flag);

            reinforce_agent_ptr->take_KL_divergence_constrained_step();
            reinforce_agent_ptr->clear_replay_memory();
         }

      } // game_over while loop

// -----------------------------------------------------------------------
    
      if(total_loss > 0)
      {
         reinforce_agent_ptr->push_back_log10_loss(log10(total_loss));
      }

      if(curr_episode_number%n_update == 0 && curr_episode_number > 0)
      {
         cout << "Episode number = " << curr_episode_number 
              << endl;

         reinforce_agent_ptr->update_episode_history();
         reinforce_agent_ptr->update_cumulative_reward(cum_reward);

         if(reinforce_agent_ptr->get_include_biases()){
           reinforce_agent_ptr->compute_bias_distributions();
         }
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->compute_deep_Qvalues();
//          reinforce_agent_ptr->print_Qmap();

         curr_maze.compute_max_Qmap();
         Qmap_score = curr_maze.score_max_Qmap();
         cout << "  Qmap_score = " << Qmap_score << endl;
         reinforce_agent_ptr->push_back_Qmap_score(Qmap_score);
         cout << "  n_backprops = " 
              << reinforce_agent_ptr->get_n_backprops() << endl;

         curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                            display_qmap_flag);
      }

      if(curr_episode_number > 0 && curr_episode_number % 100 == 0)
      {
         reinforce_agent_ptr->store_quasirandom_weight_values();
      }

      if(curr_episode_number > 0 && curr_episode_number % n_progress == 0)
      {
         reinforce_agent_ptr->push_back_learning_rate(
            reinforce_agent_ptr->get_learning_rate());
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->plot_Qmap_score_history();
         bool epoch_indep_var = false;
         reinforce_agent_ptr->generate_summary_plots(epoch_indep_var);
         reinforce_agent_ptr->generate_view_metrics_script(true, false);
      }
      reinforce_agent_ptr->increment_episode_number();
   } // n_episodes < n_max_episodes while loop

// Reinforcement training loop ends here
// ==========================================================================

   outputfunc::print_elapsed_time();
   cout << "Final episode number = "
        << reinforce_agent_ptr->get_episode_number() << endl;
   cout << "N_weights = " << reinforce_agent_ptr->count_weights()
        << endl;

   int n_final_mazes = 10;
   for(int i = 0; i < n_final_mazes; i++)
   {
      curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                         display_qmap_flag);
   }

// Generate metafiles for Qmap and loss function histories:

   reinforce_agent_ptr->push_back_learning_rate(
      reinforce_agent_ptr->get_learning_rate());
   reinforce_agent_ptr->plot_Qmap_score_history();
   bool epoch_indep_var = false;
   reinforce_agent_ptr->generate_summary_plots(epoch_indep_var);
   reinforce_agent_ptr->generate_view_metrics_script(true, false);

// Export trained weights in neural network's zeroth layer as
// greyscale images to output_subdir

   reinforce_agent_ptr->plot_zeroth_layer_weights();

//   curr_maze.DisplayTrainedZerothLayerWeights(weights_subdir);

   delete reinforce_agent_ptr;
}



