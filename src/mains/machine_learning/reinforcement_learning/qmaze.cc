// ==========================================================================
// Program QMAZE
// ==========================================================================
// Last updated on 11/11/16
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
//   nrfunc::init_time_based_seed();

   int n_grid_size = 2;
   int n_maze_states = 2;
   int n_turtle_states = sqr(n_grid_size);
   int n_states = n_maze_states * n_turtle_states;
//   int n_grid_size = 3;
//   int n_states = 14;
   int n_actions = 4;

   genmatrix Q(n_states, n_actions);

// Initialize Q matrix to random values ranging over interval [-1,1]:

   for(unsigned int i = 0; i < Q.get_mdim(); i++)
   {
      for(unsigned int j = 0; j < Q.get_ndim(); j++)
      {
         Q.put(i, j, 2 * (nrfunc::ran1() - 0.5) );
      }
   }


   maze curr_maze(n_grid_size);

   int Din = curr_maze.get_occupancy_state()->get_mdim(); // Input dim
   int Dout = 4;			// Output dim
   int Tmax = 20 * 16;

   int H1 = 1 * Din;
   int H2 = 0;

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
//   reinforce_agent_ptr->set_debug_flag(true);

// Gamma = discount factor for reward:

   reinforce_agent_ptr->set_gamma(0.90);

// Q-learning rate:
   double alpha = 0.5;

   int n_max_episodes = 1 * 1000 * 1000;
   int n_update = 10;
   int n_summarize = 1000 * 1000;

   int n_losses = 0;
   int n_wins = 0;
   double curr_reward=-999;


   environment game_world(environment::MAZE);
   game_world.set_maze(&curr_maze);
   reinforce_agent_ptr->set_environment(&game_world);

   vector<double> turn_ratios;

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes)
   {
      game_world.start_new_episode();
      reinforce_agent_ptr->initialize_episode();

      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, n_summarize, n_max_episodes);

// -----------------------------------------------------------------------
// Current episode starts here:

//      cout << "************  Start of Game " << curr_episode_number
//           << " ***********" << endl;
      curr_maze.print_occupancy_grid();
//      curr_maze.print_solution_path();

      double reward;
      genvector* next_s;
      while(!curr_maze.get_game_over())
      {
         genvector *curr_s = game_world.get_curr_state();
         int curr_a = reinforce_agent_ptr->select_action_for_curr_state();
         string curr_state_action_str = 
            game_world.get_state_action_string(curr_s, curr_a);

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
            
// Retrieve row from Q corresponding to next state.  Then compute max
// value within this row over all actions:

            double max_Q = NEGATIVEINFINITY;
            for(int a = 0; a < n_actions; a++)
            {
               string next_state_action_str = 
                  game_world.get_state_action_string(next_s, curr_a);
               double next_Q = reinforce_agent_ptr->get_Qmap_value(
                  next_state_action_str);
               max_Q = basic_math::max(next_Q, max_Q);
            }
//         cout << "max_Q = " << max_Q << endl;

         
            double old_q = reinforce_agent_ptr->get_Qmap_value(
               curr_state_action_str);
            double new_q = reward + reinforce_agent_ptr->get_gamma() * max_Q;
            double avg_q = (1 - alpha) * old_q + alpha * new_q;
            reinforce_agent_ptr->set_Qmap_value(curr_state_action_str, avg_q);

//         cout << "old_q = " << old_q << " new_q = " << new_q << endl;

// check for terminal state


/*
            if(curr_maze.get_n_turtle_moves() > 
               5 * curr_maze.get_n_soln_steps())
            {
               curr_maze.set_game_over(true);
            }
*/
         } // curr_a is legal action conditional
         

      } // !game_over while loop
// -----------------------------------------------------------------------

/*
      curr_maze.print_occupancy_grid();
      curr_maze.print_turtle_path_history();

      cout << "  n_soln_steps = "
           << curr_maze.get_n_soln_steps() << endl;
      cout << "  n_turtle_moves = "
           << curr_maze.get_n_turtle_moves() << endl;
      cout << "reward = " << reward << endl;
      outputfunc::enter_continue_char();
*/

      reinforce_agent_ptr->increment_episode_number();

      double curr_n_turns_ratio = double(
         curr_maze.get_n_soln_steps()) / curr_maze.get_n_turtle_moves();
      turn_ratios.push_back(curr_n_turns_ratio);
      if(turn_ratios.size() > 1000)
      {
         double mu, sigma;
         mathfunc::mean_and_std_dev(turn_ratios, mu, sigma);

         double median, quartile_width;
         mathfunc::median_value_and_quartile_width(
            turn_ratios, median, quartile_width);


         cout << "turn ratio = " << mu << " +/- " << sigma 
              << "   median = " << median << " quartile_width = "
              << quartile_width << endl;
         cout << "      epsilon = " << reinforce_agent_ptr->get_epsilon()
              << endl;
         turn_ratios.clear();
      }

//      outputfunc::enter_continue_char();
   } // n_episodes < n_max_episodes while loop

   delete reinforce_agent_ptr;
}



