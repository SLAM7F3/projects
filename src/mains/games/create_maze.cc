// ==========================================================================
// Program MAKEMAZE
// ==========================================================================
// Last updated on 11/5/16; 11/6/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "games/maze.h"
#include "numrec/nrfuncs.h"
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

   int nsize = 5;
   cout << "Enter maze linear size:" << endl;
   cin >> nsize;
   maze curr_maze(nsize);

   curr_maze.generate_maze();
   curr_maze.initialize_occupancy_grid();

   curr_maze.DrawMaze();
   curr_maze.print_solution_path();

   curr_maze.reset_game();
   curr_maze.print_occupancy_grid();
}



