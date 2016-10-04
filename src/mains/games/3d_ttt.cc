// ==========================================================================
// Program 3D_TTT
// ==========================================================================
// Last updated on 8/28/16; 8/29/16; 9/12/16; 10/4/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "games/reinforce.h"
#include "games/tictac3d.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();
//   tictac3d* ttt_ptr = new tictac3d(4,4);
//   tictac3d* ttt_ptr = new tictac3d(2,1);
   tictac3d* ttt_ptr = new tictac3d(4,1);

   reinforce* reinforce_ptr = new reinforce();
   cout << "*reinforce_ptr = " << *reinforce_ptr << endl;
   reinforce_ptr->xavier_init_weight_matrices();

   while(!ttt_ptr->get_game_over())
   {
//       ttt_ptr->randomize_board_state();
      ttt_ptr->display_board_state();
      
      ttt_ptr->enter_human_move();

/*
      if(ttt_ptr->check_player_win(1))
      {
         game_over = true;
      }
      
      if(ttt_ptr->check_player_win(-1))
      {
         game_over = true;
      }
*/

   } // !game_over while loop
   cout << "Final score = " << ttt_ptr->get_score() << endl;
   cout << "GAME OVER" << endl << endl;

   delete ttt_ptr;
   delete reinforce_ptr;
}



