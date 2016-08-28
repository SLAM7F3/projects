// ==========================================================================
// Program 3D_TTT
// ==========================================================================
// Last updated on 8/28/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "games/tictac3d.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   tictac3d* ttt_ptr = new tictac3d();

   ttt_ptr->display_board_state();

   delete ttt_ptr;
}



