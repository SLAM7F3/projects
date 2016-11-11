// ==========================================================================
// Program CREATE_MAZE
// ==========================================================================
// Last updated on 11/5/16; 11/6/16; 11/11/16
// ==========================================================================

#include <iostream>
#include <map>
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
   using std::map;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();

   typedef std::map<string, bool > MAZE_STRINGS_MAP;
// independent string: "TWE" representation for maze
// dependent bool: dummy variable

   MAZE_STRINGS_MAP maze_strings_map;
   MAZE_STRINGS_MAP::iterator maze_strings_iter;

   int nsize = 5;
   cout << "Enter maze linear size:" << endl;
   cin >> nsize;

// Number of distinct mazes as a function of nsize:

// nsize = 2	n_mazes = 2
// nsize = 3	n_mazes = 14
// nsize = 4	n_mazes = 322
// nsize = 5	n_mazes >= 23968

   maze curr_maze(nsize);

   int n_trials = 100 * pow(nsize, 5);
   for(int trial = 0; trial < n_trials; trial++)
   {
      curr_maze.generate_maze();

      bool zero_mean_flag = false;
//   bool zero_mean_flag = true;
      curr_maze.initialize_occupancy_grid(zero_mean_flag);

//      curr_maze.DrawMaze();
//      curr_maze.print_solution_path();

      curr_maze.reset_game(zero_mean_flag);
//      curr_maze.print_occupancy_grid();

      string occup_state_str = curr_maze.occupancy_state_to_string();

      maze_strings_iter = maze_strings_map.find(occup_state_str);
      if(maze_strings_iter == maze_strings_map.end())
      {
         maze_strings_map[occup_state_str] = true;
      }
   } // loop over trial index
   
   cout << "maze_strings_map.size() = " << maze_strings_map.size()
        << endl;

}



