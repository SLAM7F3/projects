/* ***************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare,
 *  Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence 
 *  Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * ***************************************************************************
 *  sharedLibraryInterfaceExample.cpp 
 *
 *  Sample code for running an agent with the shared library interface. 
 ************************************************************************** */

#include <iostream>
#include <SDL.h>
#include <ale_interface.hpp>
#include "general/filefuncs.h"
#include "video/videofuncs.h"

using namespace std;

int main(int argc, char** argv) {
   if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
      return 1;
   }

   ALEInterface ale;

/*
enum Action {
    PLAYER_A_NOOP           = 0,
    PLAYER_A_FIRE           = 1,
    PLAYER_A_UP             = 2,
    PLAYER_A_RIGHT          = 3,
    PLAYER_A_LEFT           = 4,
    PLAYER_A_DOWN           = 5,
    PLAYER_A_UPRIGHT        = 6,
    PLAYER_A_UPLEFT         = 7,
    PLAYER_A_DOWNRIGHT      = 8,
    PLAYER_A_DOWNLEFT       = 9,
    PLAYER_A_UPFIRE         = 10,
    PLAYER_A_RIGHTFIRE      = 11,
    PLAYER_A_LEFTFIRE       = 12,
    PLAYER_A_DOWNFIRE       = 13,
    PLAYER_A_UPRIGHTFIRE    = 14,
    PLAYER_A_UPLEFTFIRE     = 15,
    PLAYER_A_DOWNRIGHTFIRE  = 16,
    PLAYER_A_DOWNLEFTFIRE   = 17,
}
*/
 
// Breakout: 0, 1, 3, 4
// Space invaders: 0, 1, 3, 4, 11, 12
// Enduro: 0, 1, 3, 4, 5, 8, 9, 11, 12
// Ms pacman: 0, 2, 3, 4, 5, 6, 7, 8, 9

  // Get & Set the desired settings
   ale.setInt("random_seed", 123);
   //The default is already 0.25, this is just an example
   ale.setFloat("repeat_action_probability", 0.25);

   ale.setBool("display_screen", false);
   ale.setBool("display_screen", true);
//   ale.setBool("sound", true);


   // Load the ROM file. (Also resets the system for new settings to
   // take effect.)
   ale.loadROM(argv[1]);

   // Get the vector of legal actions
   ActionVect legal_actions = ale.getLegalActionSet();
   ActionVect minimal_actions = ale.getMinimalActionSet();
 
   for(unsigned int a = 0; a < minimal_actions.size(); a++)
   {
      cout << "a = " << a << " minimal_action = " << minimal_actions[a] 
           << endl;
   }

   ALEScreen curr_screen = ale.getScreen();
   int width = curr_screen.width();
   int height = curr_screen.height();
   cout << "curr_screen.width = " << width
        << " curr_screen.height = " << height << endl;


   int n_pixels = curr_screen.width() * curr_screen.height();
   vector<unsigned char> grayscale_output_buffer;
   grayscale_output_buffer.reserve(n_pixels);

   string output_subdir="./cropped_frames/";
   filefunc::dircreate(output_subdir);

// Screen ROI for space invaders:

   int min_px = 22;
   int max_px = 138;
   int min_py = 30;
   int max_py = 192;

   // Play episodes
   int n_episodes = 10;

   for (int episode = 0; episode< n_episodes; episode++) {
      cout << "Starting episode " << episode << endl;
      float totalReward = 0;
      while (!ale.game_over()) {

         int curr_frame_number = ale.getEpisodeFrameNumber();
//         cout << "Epsiode frame number = " << ale.getEpisodeFrameNumber()
//              << endl;
         grayscale_output_buffer.clear();
         ale.getScreenGrayscale(grayscale_output_buffer);

         if(curr_frame_number % 3 == 0)
         {
            vector<vector<unsigned char > > byte_array;
            for(int py = min_py; py < max_py; py++)
            {
               vector<unsigned char> curr_byte_row;
               for(int px = min_px; px <  max_px; px++)
               {
                  int curr_pixel = px + py * width;
                  curr_byte_row.push_back(grayscale_output_buffer[
                                             curr_pixel]);
               }
               byte_array.push_back(curr_byte_row);
            }

            string output_frame = output_subdir+"frame_"+
               stringfunc::integer_to_string(curr_frame_number,5)+
               ".png";
            videofunc::write_8bit_greyscale_pngfile(byte_array, output_frame);
         } // curr_frame_number % 3 == 0 conditional
         

         Action a = legal_actions[rand() % legal_actions.size()];
         // Apply the action and get the resulting reward
         float reward = ale.act(a);
         totalReward += reward;
         cout << "frame = " << curr_frame_number
              << " curr_reward = " << reward
              << " total_reward = " << totalReward << endl;
      }
      cout << "Episode " << episode << " ended with score: " 
           << totalReward << endl;
      ale.reset_game();
   }

   return 0;
}


// Space invaders:  
// 20 <= px <= 140
// 30 <= py <= 193
