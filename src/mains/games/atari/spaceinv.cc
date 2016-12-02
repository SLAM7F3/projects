// ==========================================================================
// Program SPACEINV
// ==========================================================================
// Last updated on 12/1/16; 12/2/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "general/filefuncs.h"
#include "games/spaceinv.h"
#include "numrec/nrfuncs.h"
#include "video/videofuncs.h"

int main(int argc, char** argv) 
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   
   spaceinv *spaceinv_ptr = new spaceinv();

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
      while (!spaceinv_ptr->get_ale().game_over()) {

         int curr_frame_number = spaceinv_ptr->get_ale().
            getEpisodeFrameNumber();
         if(curr_frame_number > spaceinv_ptr->get_min_episode_framenumber())
         {
            if(curr_frame_number % spaceinv_ptr->get_frame_skip() == 0)
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
         spaceinv_ptr->get_max_score_per_episode();

      cout << " Final episode frame number = " 
           << spaceinv_ptr->get_ale().getEpisodeFrameNumber() << endl;
      spaceinv_ptr->get_ale().reset_game();
   } // loop over episodes
   
/*
   double mu, sigma;
   mathfunc::mean_and_std_dev(z_differences, mu, sigma);
   cout << "z_difference = " << mu << " +/- " << sigma << endl;

   mathfunc::mean_and_std_dev(scores, mu, sigma);
   double median, qw;
   mathfunc::median_value_and_quartile_width(scores, median, qw);

   cout << "score = " << mu << " +/- " << sigma << endl;
   cout << "median = " << median << " quartile width = " << qw << endl;
   cout << "max score = " << mathfunc::maximal_value(scores) << endl;

*/


   

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

