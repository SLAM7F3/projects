// ==========================================================================
// Program SOLVE_MAZE
// ==========================================================================
// Last updated on 12/13/16; 12/14/16; 12/15/16; 12/20/16
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
   long seed = nrfunc::init_time_based_seed();
//   long seed = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> seed;
//   nrfunc::init_default_seed(seed);

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
   int H1 = 10;
//   int H1 = 12;

//   int H2 = 0;
//   int H2 = 8;
   int H2 = 10;
//   int H2 = 12;

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

// Construct reinforcement learning agent:

   int replay_memory_capacity = 10 * sqr(n_grid_size);
//   int replay_memory_capacity = 25 * sqr(n_grid_size);
   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, 1, replay_memory_capacity,
//      reinforce::SGD);
//      reinforce::MOMENTUM);
//      reinforce::NESTEROV);
      reinforce::RMSPROP);
//      reinforce::ADAM);

//   const double beta1 = 0.0;
//   const double beta1 = 1E-12;// OK
//   const double beta1 = 1E-9;  // OK
   const double beta1 = 1E-8;  // OK
//   const double beta1 = 1E-7;  // bad
//   const double beta1 = 1E-6;   // bad

//   const double beta2 = 0.5;   // OK
   const double beta2 = 0.90;  // OK
//   const double beta2 = 0.99;   /bad
   reinforce_agent_ptr->set_ADAM_params(beta1, beta2);

//   reinforce_agent_ptr->set_debug_flag(true);
   reinforce_agent_ptr->set_environment(&game_world);
   reinforce_agent_ptr->set_lambda(0);
//   reinforce_agent_ptr->set_lambda(1E-2);
//   reinforce_agent_ptr->set_lambda(1E-4);
   machinelearning_func::set_leaky_ReLU_small_slope(0.00); 
//   machinelearning_func::set_leaky_ReLU_small_slope(0.01); 

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

   reinforce_agent_ptr->set_Nd(32);
//   reinforce_agent_ptr->set_Nd(64);
   reinforce_agent_ptr->set_gamma(0.95);  // reward discount factor
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);
//   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_base_learning_rate(3E-4);  
//   reinforce_agent_ptr->set_base_learning_rate(1E-5);

// Periodically decrease learning rate down to some minimal floor
// value:

   double min_learning_rate = 
      0.1 * reinforce_agent_ptr->get_base_learning_rate();

   int n_max_episodes = 1 * 1000 * 1000;
   int n_lr_episodes_period = 100 * 1000;
   int old_weights_period = 10; // Seems optimal for n_grid_size = 8
//   int old_weights_period = 32;  

   reinforce_agent_ptr->set_epsilon_time_constant(20000 * n_grid_size / 6.0);
   double min_epsilon = 0.10;
   reinforce_agent_ptr->set_min_epsilon(min_epsilon);

   int n_update = 500;
   int n_progress = 25000;
   double Qmap_score = -1;

   string basename = "maze";
   bool display_qmap_flag = true;
   reinforce_agent_ptr->compute_deep_Qvalues();
//   reinforce_agent_ptr->print_Qmap();
   curr_maze.compute_max_Qmap();
   curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                      display_qmap_flag);

   string subtitle="Nsize="+stringfunc::number_to_string(n_grid_size)
      +";old weights T="+stringfunc::number_to_string(old_weights_period)
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

// Generate text file summary of parameter values:

   string params_filename = output_subdir + "params.dat";
   reinforce_agent_ptr->summarize_parameters(params_filename);
   ofstream params_stream;
   filefunc::appendfile(params_filename, params_stream);

   params_stream << "n_actions = " << n_actions << endl;
   params_stream << "Leaky ReLU small slope = "
                 << machinelearning_func::get_leaky_ReLU_small_slope() << endl;
   params_stream << "Old weights period = " << old_weights_period
                 << " episodes" << endl;
   params_stream << "Random seed = " << seed << endl;
   filefunc::closefile(params_filename, params_stream);

// ==========================================================================
// Reinforcement training loop starts here
      
   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes &&
         Qmap_score < 0.999999)
   {
      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_progress, n_max_episodes);

      bool random_turtle_start = true;
      game_world.start_new_episode(random_turtle_start);
      reinforce_agent_ptr->initialize_episode();

      if(curr_episode_number > 0 && 
         curr_episode_number%n_lr_episodes_period == 0)
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

//      cout << "************  Start of episode " << curr_episode_number
//           << " ***********" << endl;
//      curr_maze.print_occupancy_grid();
//      curr_maze.print_occupancy_state();

      double reward;
      genvector* next_s;
      while(!game_world.get_game_over())
      {
         genvector *curr_s = game_world.get_curr_state();
         int d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
            *curr_s);
         int curr_a = reinforce_agent_ptr->select_action_for_curr_state();
//         cout << "   d = " << d << " curr_a = " << curr_a << endl;

         if(!game_world.is_legal_action(curr_a))
         {
            next_s = NULL;
            reward = -1;
            game_world.set_game_over(true);
         }
         else
         {
            next_s = game_world.compute_next_state(curr_a);
            reward = curr_maze.compute_turtle_reward();
         } // curr_a is legal action conditional

//         cout << "   reward = " << reward 
//              << " game over = " << game_world.get_game_over() << endl;

         if(d >= 0 && game_world.get_game_over())
         {
            reinforce_agent_ptr->store_final_arsprime_into_replay_memory(
               d, curr_a, reward);
         }
         else
         {
            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, reward, *next_s, game_world.get_game_over());
         }
      } // game_over while loop

// -----------------------------------------------------------------------
    
      reinforce_agent_ptr->append_n_frames_per_episode(
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
         total_loss = reinforce_agent_ptr->update_neural_network();
      }

// Exponentially decay epsilon:

      reinforce_agent_ptr->exponentially_decay_epsilon(
         curr_episode_number, 0);

      if(curr_episode_number%n_update == 0 && curr_episode_number > 0)
      {
         cout << "Episode number = " << curr_episode_number 
              << " epsilon = " << reinforce_agent_ptr->get_epsilon()
              << endl;

         if(reinforce_agent_ptr->get_include_bias_terms()){
           reinforce_agent_ptr->compute_bias_distributions();
         }
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->compute_deep_Qvalues();
//          reinforce_agent_ptr->print_Qmap();

         curr_maze.compute_max_Qmap();
         Qmap_score = curr_maze.score_max_Qmap();
         cout << "  Qmap_score = " << Qmap_score << endl;
         reinforce_agent_ptr->push_back_Qmap_score(Qmap_score);

         if(total_loss > 0)
         {
            cout << "  Total loss = " << total_loss 
                 << " log10(total_loss) = " << log10(total_loss) << endl;
            reinforce_agent_ptr->push_back_log10_loss(log10(total_loss));
         }

         curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                            display_qmap_flag);
      }

      if(curr_episode_number > 0 && curr_episode_number % 100 == 0)
      {
         reinforce_agent_ptr->store_quasirandom_weight_values();
      }

      if(curr_episode_number > 0 && curr_episode_number % n_progress == 0)
      {
         reinforce_agent_ptr->compute_weight_distributions();
         bool epoch_indep_var = false;
         reinforce_agent_ptr->generate_summary_plots(
            output_subdir, extrainfo, epoch_indep_var);
         reinforce_agent_ptr->plot_Qmap_score_history(
            output_subdir, subtitle, extrainfo);
      }

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

   reinforce_agent_ptr->generate_summary_plots(output_subdir, extrainfo);
   reinforce_agent_ptr->plot_Qmap_score_history(
      output_subdir, subtitle, extrainfo);

// Export trained weights in neural network's zeroth layer as
// greyscale images to output_subdir

   string weights_subdir = output_subdir+"zeroth_layer_weights/";
   filefunc::dircreate(weights_subdir);
   reinforce_agent_ptr->plot_zeroth_layer_weights(weights_subdir);

   curr_maze.DisplayTrainedZerothLayerWeights(weights_subdir);

   delete reinforce_agent_ptr;
}



