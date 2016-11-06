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
   maze curr_maze(nsize);

   curr_maze.generate_maze();
   curr_maze.DrawMaze();
   curr_maze.print_solution_path();
}



