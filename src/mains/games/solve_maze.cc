// ==========================================================================
// Program SOLVE_MAZE
// ==========================================================================
// Last updated on 11/5/16; 11/9/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "machine_learning/environment.h"
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
//   int n_grid_size = 3;
   maze curr_maze(n_grid_size);


   int Din = curr_maze.get_occupancy_state()->get_mdim(); // Input dim
   int Dout = 4;			// Output dim
   int Tmax = 20 * 16;

   int H1 = 4 * Din;
//   int H1 = 8 * Din;
//   int H1 = 16 * Din;
   int H2 = 0;
//   int H2 = 1 * Dout;

   string extrainfo="H1="+stringfunc::number_to_string(H1);
   if(H2 > 0)
   {
      "; H2="+stringfunc::number_to_string(H2);
   }

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   if(H2 > 0)
   {
      layer_dims.push_back(H2);
   }
   layer_dims.push_back(Dout);

   reinforce* reinforce_agent_ptr = new reinforce(layer_dims, Tmax);

// Gamma = discount factor for reward:

   reinforce_agent_ptr->set_gamma(0.90);
   reinforce_agent_ptr->set_batch_size(30);   
   reinforce_agent_ptr->set_rmsprop_decay_rate(0.85);

//   reinforce_agent_ptr->set_base_learning_rate(1E-3);
   reinforce_agent_ptr->set_base_learning_rate(3E-4);
//   reinforce_agent_ptr->set_base_learning_rate(1E-4);
   double min_learning_rate = 3E-5;

   int n_max_episodes = 1 * 1000 * 1000;
   int n_update = 1000;
   int n_summarize = 1000 * 1000;

   int n_losses = 0;
   int n_wins = 0;
   double curr_reward=-999;

// Periodically decrease learning rate down to some minimal floor
// value:

   int n_episodes_period = 1000 * 1000;

// Initialize Deep Q replay memory:

   curr_maze.generate_maze();
   curr_maze.DrawMaze();
   curr_maze.reset_game();
   reinforce_agent_ptr->initialize_episode();   

   environment game_world(environment::MAZE);
   game_world.set_maze(&curr_maze);
   reinforce_agent_ptr->set_environment(&game_world);

   reinforce_agent_ptr->initialize_replay_memory();

   cout << "Before start of training loop inside solve_maze" << endl;

   exit(-1);

   bool generate_new_maze = false;
   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes)
   {
      if(generate_new_maze)
      {
         curr_maze.generate_maze();
         generate_new_maze = false;
      }
      curr_maze.reset_game();

      reinforce_agent_ptr->initialize_episode();

      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_summarize, n_max_episodes);

/*      
      if(curr_episode_number > 0 && curr_episode_number%n_episodes_period == 0)
      {
         double curr_learning_rate = reinforce_agent_ptr->get_learning_rate();
         if(curr_learning_rate > min_learning_rate)
         {
            reinforce_agent_ptr->set_learning_rate(0.8 * curr_learning_rate);
            n_episodes_period *= 1.2;
         }
      }
*/

// -----------------------------------------------------------------------
// Current episode starts here:

//      cout << "************  Start of Game " << curr_episode_number
//           << " ***********" << endl;
//      curr_maze.print_occupancy_grid();

//      int Nmax = 2;
      int Nmax = 4;
      double alpha = -(Nmax + 1) / (Nmax - 1);

      while(!curr_maze.get_game_over())
      {

// Agent move:

// Given turtle's current position within maze, first determine which
// output actions are legal:

         genvector* curr_legal_actions = 
            curr_maze.compute_legal_turtle_actions();

//         bool enforce_constraints_flag = false;
         bool enforce_constraints_flag = true;
         reinforce_agent_ptr->compute_action_probs(
            curr_maze.get_occupancy_state(), enforce_constraints_flag,
            curr_legal_actions);
         int output_action = reinforce_agent_ptr->
            get_candidate_current_action();
         reinforce_agent_ptr->set_current_action(output_action);

         bool erase_turtle_path = true;
//         bool erase_turtle_path = false;
         int legal_move = curr_maze.move_turtle(
            output_action, erase_turtle_path);
         if(legal_move < 0)
         {
            cout << "output_action = " << output_action
                 << " legal_move = " << legal_move 
                 << " n_turtle_moves = " << curr_maze.get_n_turtle_moves()
                 << endl;
            outputfunc::enter_continue_char();
         }

// Do not allow turtle to oscillate indefinitely:

         if(curr_maze.get_n_turtle_moves() > 
            Nmax * curr_maze.get_solution_path_moves())
         {
            legal_move = -1;
         }

// Step the environment and then retrieve new reward measurements:

         curr_reward = 0;
         if(legal_move < 0)
         {
            curr_reward = -1;
            curr_maze.set_game_over(true);
         }
         else if (curr_maze.get_game_over())
         {
            int n_soln_steps = curr_maze.get_n_soln_steps();
            int n_turtle_steps = curr_maze.get_n_turtle_moves();
            curr_reward = 1 - sqr(n_turtle_steps - n_soln_steps);
         }
         
         reinforce_agent_ptr->record_reward_for_action(curr_reward);

//         curr_maze.print_occupancy_grid();
//         curr_maze.print_turtle_path_history();

      } // !game_over while loop
// -----------------------------------------------------------------------

      if(curr_maze.get_maze_solved() && nearly_equal(curr_reward,1))
      {
         n_wins++;
         generate_new_maze = true;
      }
      else
      {
         n_losses++;
      }
      
      reinforce_agent_ptr->update_weights();
      reinforce_agent_ptr->update_running_reward(extrainfo);

      reinforce_agent_ptr->increment_episode_number();
      int n_episodes = curr_episode_number + 1;
      if(curr_episode_number > 10 && curr_episode_number % n_update == 0)
      {
         double loss_frac = double(n_losses) / n_episodes;
         double win_frac = double(n_wins) / n_episodes;
         cout << "n_episodes = " << n_episodes 
              << " n_losses = " << n_losses
              << " n_wins = " << n_wins
              << endl;
         cout << "  loss_frac = " << loss_frac
              << "  win_frac = " << win_frac
              << endl;
         cout << "  n_turtle_moves = "
              << curr_maze.get_n_turtle_moves() << endl;
         cout << "  n_soln_steps = "
              << curr_maze.get_n_soln_steps() << endl;
         curr_maze.print_turtle_path_history();

         double curr_n_turns_ratio = double(
            curr_maze.get_n_soln_steps()) / curr_maze.get_n_turtle_moves();
         reinforce_agent_ptr->append_n_episode_turns_frac(curr_n_turns_ratio);
         reinforce_agent_ptr->snapshot_running_reward();
      }

      if(curr_episode_number > 10 && curr_episode_number % n_summarize == 0)
      {
         reinforce_agent_ptr->compute_weight_distributions();
         reinforce_agent_ptr->plot_loss_history(extrainfo);
         reinforce_agent_ptr->plot_reward_history(extrainfo, -1, 1);
         reinforce_agent_ptr->plot_turns_history(extrainfo);
      }

//      outputfunc::enter_continue_char();
   } // n_episodes < n_max_episodes while loop

   delete reinforce_agent_ptr;
}



