// ==========================================================================
// Program QSPACE solves the Space Invaders atari game via deep Q-learning.
// ==========================================================================
// Last updated on 12/1/16; 12/2/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "games/spaceinv.h"
#include "numrec/nrfuncs.h"
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

   int Din = curr_maze.get_occupancy_state()->get_mdim(); // Input dim
   int Dout = n_actions;
   int Tmax = 1;

   int H1 = 64;
   int H2 = 32;
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


   bool export_frames_flag = false;
//   bool export_frames_flag = true;

   int n_episodes = 100;

   // Get the vector of legal actions
   ActionVect legal_actions = spaceinv_ptr->get_ale().getLegalActionSet();
   ActionVect minimal_actions = spaceinv_ptr->get_ale().getMinimalActionSet();

   for(int episode = 0; episode< n_episodes; episode++) 
   {
      cout << "Starting episode " << episode << endl;
      float cumReward = 0;
      while (!game_world.get_game_over()) {

         int curr_frame_number = game_world.get_episode_framenumber();
         if(curr_frame_number > game_world.get_min_episode_framenumber())
         {
            if(curr_frame_number % game_world.get_frame_skip() == 0)
            {
               spaceinv_ptr->crop_pool_difference_curr_frame(
                  export_frames_flag);
            } // curr_frame_number % frame_skip == 0 conditional
         } // curr_frame_number > min_episode_framenumber
         
         Action a = legal_actions[rand() % legal_actions.size()];
         // Apply the action and get the resulting reward
         cumReward += spaceinv_ptr->get_ale().act(a);
      } // !game_over conditional

      cout << "Episode " << episode << " ended with score: " 
           << cumReward << endl;
//       scores.push_back(cumReward);
      double renorm_reward = cumReward / 
         game_world.get_max_score_per_episode();

      cout << " Final episode frame number = " 
           << game_world.get_episode_framenumber() << endl;
      spaceinv_ptr->get_ale().reset_game();
   } // loop over episodes

   

// z_difference = 0.937752 +/- 11.0019
// z_difference = 0.936237 +/- 10.9916
// z_difference = 0.951648 +/- 11.0808
// z_difference = 0.944325 +/- 11.0395

// score = 147.6 +/- 98.1898
// score = 169.6 +/- 115.999
// score = 160.55 +/- 95.4513
// median = 155 quartile width = 55
// median = 135 quartile width = 52.5
// max score = 605
// max score = 640
// max score = 660

   delete spaceinv_ptr;
}

