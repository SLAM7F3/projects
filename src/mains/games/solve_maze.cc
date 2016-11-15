// ==========================================================================
// Program SOLVE_MAZE
// ==========================================================================
// Last updated on 11/5/16; 11/9/16; 11/13/16; 11/14/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "games/maze.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "machine_learning/reinforce.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

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

   int Din = curr_maze.get_occupancy_state()->get_mdim(); // Input dim
   int Dout = n_actions;
   int Tmax = 1;

   int H1 = 8;
//   int H1 = 10;
//   int H1 = 12;
//   int H1 = 16;
//   int H1 = 20;
//   int H1 = 32;

//   int H2 = 0;
   int H2 = 8;
//   int H2 = 10;
//   int H2 = 12;
//   int H2 = 4;

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

//   int replay_memory_capacity = 3 * sqr(n_grid_size);
   int replay_memory_capacity = 5 * sqr(n_grid_size);
   reinforce* reinforce_agent_ptr = new reinforce(
      layer_dims, Tmax, replay_memory_capacity);
//   reinforce_agent_ptr->set_debug_flag(true);
   reinforce_agent_ptr->set_environment(&game_world);
   curr_maze.set_qmap_ptr(reinforce_agent_ptr->get_qmap_ptr());

   int output_counter = 0;
   string experiments_subdir="./experiments/mazes/";
   filefunc::dircreate(experiments_subdir);

   int expt_number;
   cout << "Enter experiment number:" << endl;
   cin >> expt_number;
   string output_subdir=experiments_subdir+
      "expt"+stringfunc::integer_to_string(expt_number,3)+"/";
   filefunc::dircreate(output_subdir);

   string basename = "maze";
   bool display_qmap_flag = true;
   curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                      display_qmap_flag);

// Gamma = discount factor for reward:

   reinforce_agent_ptr->set_gamma(0.95);
   reinforce_agent_ptr->set_batch_size(1);
//   reinforce_agent_ptr->set_batch_size(3);
//   reinforce_agent_ptr->set_batch_size(10);   
//   reinforce_agent_ptr->set_batch_size(30);
//   reinforce_agent_ptr->set_rmsprop_decay_rate(0.95);
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.90);
//   reinforce_agent_ptr->set_rmsprop_decay_rate(0.85);
//   reinforce_agent_ptr->set_rmsprop_decay_rate(0.75);

//   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_base_learning_rate(3E-4);  
		// optimal for n_grid_size = 7
//   reinforce_agent_ptr->set_base_learning_rate(1E-4);
//   reinforce_agent_ptr->set_base_learning_rate(3E-5);
//   reinforce_agent_ptr->set_base_learning_rate(1E-5);
   double min_learning_rate = 1E-4;

   int n_max_episodes = 100 * 1000 * 1000;
   int n_anneal_steps = 1000;
   int n_update = 1000;
   int n_summarize = 20000;
   double Qmap_score = -1;

// Periodically decrease learning rate down to some minimal floor
// value:

   int n_episodes_period = 100 * 1000;
   //   int old_weights_period = 10;
   int old_weights_period = 30;   // Optimal for n_grid_size = 7 
//   int old_weights_period = 100;
   //   int old_weights_period = 300;
   //   int old_weights_period = 1000;

//   double min_epsilon = 0.05;
   double min_epsilon = 0.1; // Optimal for n_grid_size = 7, 10
//   double min_epsilon = 0.15;

// Initialize Deep Q replay memory:

   game_world.start_new_episode();

   reinforce_agent_ptr->initialize_replay_memory();
   int update_old_weights_counter = 0;
   double total_loss = -1;

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes &&
         Qmap_score < 0.999999)
   {
      bool random_turtle_start = true;
      curr_maze.reset_game(random_turtle_start);
      reinforce_agent_ptr->initialize_episode();

      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, 5 * n_summarize, n_max_episodes);

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

//      cout << "************  Start of Game " << curr_episode_number
//           << " ***********" << endl;
//      curr_maze.print_occupancy_grid();
//      curr_maze.print_occupancy_state();

      double reward;
      genvector* next_s;
      while(!curr_maze.get_game_over())
      {
         genvector *curr_s = game_world.get_curr_state();
         int d = reinforce_agent_ptr->store_curr_state_into_replay_memory(
            *curr_s);

         int curr_a;

// Experiment with increasing epsilon for random actions if current
// turtle cell is problematic:

         if(curr_maze.get_n_problem_cells() < 0.5 * curr_maze.get_n_cells())
         {
            int turtle_p = curr_maze.get_turtle_cell();
            if(curr_maze.is_problem_cell(turtle_p) < 0)
            {
               double orig_eps = reinforce_agent_ptr->get_epsilon();
               reinforce_agent_ptr->set_epsilon(
                  basic_math::max(0.5, orig_eps));
               curr_a = reinforce_agent_ptr->select_action_for_curr_state();
               reinforce_agent_ptr->set_epsilon(orig_eps);
            }
         }
         else
         {
            curr_a = reinforce_agent_ptr->select_action_for_curr_state();
         }
         


         if(!game_world.is_legal_action(curr_a))
         {
            next_s = NULL;
            reward = -1;
            curr_maze.set_game_over(true);
         }
         else
         {
            next_s = game_world.compute_next_state(curr_a);
            reward = curr_maze.compute_turtle_reward();
         } // curr_a is legal action conditional

//         cout << " reward = " << reward 
//              << " game over = " << curr_maze.get_game_over() << endl;

         if(curr_maze.get_game_over())
         {
            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, reward, *curr_s, curr_maze.get_game_over());
         }
         else
         {
            reinforce_agent_ptr->store_arsprime_into_replay_memory(
               d, curr_a, reward, *next_s, curr_maze.get_game_over());
         }
      } // game_over while loop

// -----------------------------------------------------------------------

      reinforce_agent_ptr->increment_episode_number();

      update_old_weights_counter++;
      if(update_old_weights_counter%old_weights_period == 0)
      {
         reinforce_agent_ptr->copy_weights_onto_old_weights();
         update_old_weights_counter = 1;
      }

      if(curr_episode_number > 0 && curr_episode_number % 
         reinforce_agent_ptr->get_batch_size() == 0)
      {
         total_loss = reinforce_agent_ptr->update_Q_network();
      }

      if(curr_episode_number > 0 && curr_episode_number % n_anneal_steps == 0)
      {
//         double decay_factor = 0.995;
         double decay_factor = 0.99;
         reinforce_agent_ptr->anneal_epsilon(decay_factor, min_epsilon);
      }

      if(curr_episode_number%n_update == 0)
      {
         cout << "Episode number = " << curr_episode_number 
              << " epsilon = " << reinforce_agent_ptr->get_epsilon()
              << endl;

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
   } // n_episodes < n_max_episodes while loop

   outputfunc::print_elapsed_time();
   cout << "Final episode number = "
        << reinforce_agent_ptr->get_episode_number() << endl;
   cout << "N_weights = " << reinforce_agent_ptr->count_weights()
        << endl;

   curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                      display_qmap_flag);
   string subtitle="Nsize="+stringfunc::number_to_string(n_grid_size)
      +";old weights T="+stringfunc::number_to_string(old_weights_period)
      +";min eps="+stringfunc::number_to_string(min_epsilon);

   string extrainfo="H1="+stringfunc::number_to_string(H1);
   if(H2 > 0)
   {
      ";H2="+stringfunc::number_to_string(H2);
   }
   if(H3 > 0)
   {
      ";H3="+stringfunc::number_to_string(H3);
   }

   reinforce_agent_ptr->plot_Qmap_score_history(
      output_subdir, subtitle, extrainfo);
   reinforce_agent_ptr->plot_log10_loss_history(
      output_subdir, subtitle, extrainfo);
//   reinforce_agent_ptr->print_Qmap();

   delete reinforce_agent_ptr;
}



