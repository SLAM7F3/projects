// ==========================================================================
// Program 3D_TTT
// ==========================================================================
// Last updated on 8/28/16; 8/29/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
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
   tictac3d* ttt_ptr = new tictac3d();

   bool game_over = false;
   while(!game_over)
   {
//       ttt_ptr->randomize_board_state();
      ttt_ptr->display_board_state();
      
      ttt_ptr->enter_human_move();

      if(ttt_ptr->check_player_win(1))
      {
         game_over = true;
      }
      
      if(ttt_ptr->check_player_win(-1))
      {
         game_over = true;
      }

   } // loop over iter index
   delete ttt_ptr;
}



