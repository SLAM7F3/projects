// ==========================================================================
// Program TICTAC2 attempts to train a 2D tic-tac-toe program.


//			       tictac2

// ==========================================================================
// Last updated on 8/23/15; 8/24/15
// ==========================================================================

#include  <string>
#include  <vector>

#include "numrec/nrfuncs.h"
#include "games/tictac2d.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   nrfunc::init_time_based_seed();
   tictac2d tictac;
   tictac.initialize_weights();

   tictac.import_board_scores_map();

   int n_games = 10;
   cout << "Enter number of games to play:" << endl;
   cin >> n_games;

   for(int iter = 0; iter < n_games; iter++)
   {
      cout << "------------------------------------------ " << endl;
      cout << "Game " << iter << endl;
      cout << "------------------------------------------ " << endl;

//      tictac.print_board();
      do
      {
         tictac.make_next_move();
//         tictac.print_board();
         tictac.reset_curr_player();
      }
      while(!tictac.is_game_over());
//      tictac.print_corridor_weights();
      tictac.print_board_history();
//      tictac.score_board_history();

      tictac.reset_game_params();
   } // loop over iter index counting games

//   tictac.print_board_scores_map();
//   tictac.export_board_scores_map();
//   tictac.train_weights();
//   tictac.print_final_weights();
   cout << "n_games = " << n_games << endl;
}

