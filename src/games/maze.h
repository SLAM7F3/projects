// ==========================================================================
// Header file for maze class 
// ==========================================================================
// Last modified on 11/5/16
// ==========================================================================

#ifndef MAZE_H
#define MAZE_H

#include <map>
#include <vector>
#include "math/genmatrix.h"
#include "math/ltduple.h"
#include "datastructures/Stack.h"

class maze
{
   
  public:

// Initialization, constructor and destructor functions:

   maze(int n_size);
   maze(const maze& C);
   ~maze();
//   maze operator= (const maze& C);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const maze& C);

   DUPLE getDirection(int curr_dir);
   bool IsDirValid(int px, int py, int curr_dir);
   int get_neighbor(int p, int curr_dir);
   void generate_maze();



  private: 

   int n_size, n_cells;
   bool game_over;
   std::vector<int> direction;
   genmatrix *grid_ptr, *visited_ptr;
   Stack<int> mstack;

   std::vector<DUPLE> cell_decomposition;
// independent int: p
// dependent triple: px, py

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const maze& T);

   int get_cell(int px, int py);
   void set_game_over(bool flag);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int maze::get_cell(int px, int py)
{
   return n_size * py + px;
}

inline void maze::set_game_over(bool flag)
{
   game_over = flag;
}


#endif  // maze.h

