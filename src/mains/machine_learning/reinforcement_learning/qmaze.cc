// ==========================================================================
// Program QMAZE utilizes non-deep Q learning to iteratively solve for
// directions to follow within randomly generated mazes.  For a
// particular maze with size n_size, the total number of possible
// "turtle" states = n_size**2 * n_directions.  QMAZE works with an
// explicit table Q(s,a) whose values are initially randomly chosen
// from [-1,1].  The turtle is started at some random cell inside the
// maze.  It takes argmax_a Q(s,a) as the direction to move.  If the
// turtle crashes into a wall, the episode terminates and the
// reinforcement agent receives a -1 reward.  If the turtle moves to a
// legal cell, the reinforcement agent receives a 0 reward.  And if
// the turtle reaches the final maze cell, the episode terminates with
// a +1 reward.  Q(s,a) is updated after each terminal move according
// to the Bellman equation.  It eventually converges to a function for
// which the turtle never crashes into a wall and for which the turtle
// optimally finds it way through the maze starting at any cell
// location.

// QMAZE periodically outputs PNG images showing the current values
// for Q(s,a) as arrows superposed on the grid cell locations.  It
// also exports a plot of the ratio of correct Q(s,a) directions
// to the total number of cells as a function of episode number.  This
// ratio should approach one as Q learning proceeds.
// ==========================================================================
// Last updated on 11/11/16; 11/12/16
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
   cout << "Enter grid size:" << endl;
   cin >> n_grid_size;
   int n_actions = 4;

// Construct one particular maze:

   maze curr_maze(n_grid_size);
   curr_maze.generate_maze();
   curr_maze.DrawMaze(0, "./", "empty_maze", false);
   curr_maze.solve_maze_backwards();
   curr_maze.generate_all_turtle_states();

// Construct environment which acts as interface between reinforcement
// agent and particular game:

   environment game_world(environment::MAZE);
   game_world.set_maze(&curr_maze);

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

// Construct reinforcement learning agent:

   reinforce* reinforce_agent_ptr = new reinforce(layer_dims, Tmax);
//   reinforce_agent_ptr->set_debug_flag(true);
   reinforce_agent_ptr->set_environment(&game_world);
   reinforce_agent_ptr->init_random_Qmap();
   curr_maze.set_qmap_ptr(reinforce_agent_ptr->get_qmap_ptr());

   int output_counter = 0;
   string output_subdir = "./output_solns";
   string basename = "maze";
   bool display_qmap_flag = true;
   curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                      display_qmap_flag);

// Gamma = discount factor for reward:

   reinforce_agent_ptr->set_gamma(0.8);

// Q-learning rate:

   double alpha = 0.75;
//   double alpha = 1.0;

   int n_max_episodes = basic_math::max(10000, 5 * sqr(sqr(n_grid_size)));
   int n_summarize = 500;
   double Qmap_score = -1;

   while(reinforce_agent_ptr->get_episode_number() < n_max_episodes &&
         Qmap_score < 0.999999)
   {
      bool random_turtle_start = true;
//      bool random_turtle_start = false;
      curr_maze.reset_game(random_turtle_start);
      reinforce_agent_ptr->initialize_episode();

      int curr_episode_number = reinforce_agent_ptr->get_episode_number();
      outputfunc::update_progress_and_remaining_time(
         curr_episode_number, 5 * n_summarize, n_max_episodes);

// -----------------------------------------------------------------------
// Current episode starts here:

//      cout << "************  Start of Game " << curr_episode_number
//           << " ***********" << endl;
//      curr_maze.print_occupancy_grid();
//      curr_maze.print_occupancy_state();
//      curr_maze.print_solution_path();

      double reward, max_Q;
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
            max_Q = 0;
            curr_maze.set_game_over(true);
         }
         else
         {
            next_s = game_world.compute_next_state(curr_a);
            reward = curr_maze.compute_turtle_reward();

// Retrieve row from Q corresponding to next state.  Then compute max
// value within this row over all actions:

            max_Q = NEGATIVEINFINITY;
            for(int a = 0; a < n_actions; a++)
            {
               string next_state_action_str = 
                  game_world.get_state_action_string(next_s, a);
               double next_Q = reinforce_agent_ptr->get_Q_value(
                  next_state_action_str);
               max_Q = basic_math::max(next_Q, max_Q);
            }
         } // curr_a is legal action conditional

         double old_q = reinforce_agent_ptr->get_Q_value(
            curr_state_action_str);
         double new_q = reward + reinforce_agent_ptr->get_gamma() * max_Q;
         double avg_q = (1 - alpha) * old_q + alpha * new_q;
         reinforce_agent_ptr->set_Q_value(curr_state_action_str, avg_q);

//         cout << "reward = " << reward << " max_Q = " << max_Q << endl;
//         cout << "old_q = " << old_q << " new_q = " << new_q 
//              << " avg_q = " << avg_q << endl;

      } // !game_over while loop
// -----------------------------------------------------------------------

//      cout << "game over" << endl;
//      reinforce_agent_ptr->print_Qmap();

      reinforce_agent_ptr->increment_episode_number();

      if(curr_episode_number % n_summarize == 0)
      {
         Qmap_score = curr_maze.score_max_Qmap();
         reinforce_agent_ptr->push_back_Qmap_score(Qmap_score);
         cout << "Qmap_score = " << Qmap_score << endl;
         curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                            display_qmap_flag);
         cout << endl;
      }
   } // n_episodes < n_max_episodes while loop

   curr_maze.set_qmap_ptr(reinforce_agent_ptr->get_qmap_ptr());
   curr_maze.DrawMaze(output_counter++, output_subdir, basename,
                      display_qmap_flag);
   reinforce_agent_ptr->plot_Qmap_score_history("");

   delete reinforce_agent_ptr;
}



