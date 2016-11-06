// ==========================================================================
// Header file for maze class 
// ==========================================================================
// Last modified on 11/5/16; 11/6/16
// ==========================================================================

#ifndef MAZE_H
#define MAZE_H

#include <map>
#include <stack>
#include <vector>
#include "math/genmatrix.h"
#include "math/ltduple.h"

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
   std::vector<int> get_cell_neighbors(int p);
   std::vector<int> get_unvisited_neighbors(int p);
   int n_visited_cells() const;
   void init_grid();
   void print_grid() const;
   void print_visited_cells() const;
   void print_visited_cell_stack() const;
   void set_solution_path();
   const std::vector<int>& get_solution_path() const;
   void print_solution_path() const;
   void generate_maze();

   void DrawLine(unsigned char* img, int x1, int y1, int x2, int y2,
                 int R, int G, int B);
   void RenderMaze(unsigned char* img);
   void SaveBMP(std::string FileName, const void* RawBGRImage, 
                int Width, int Height);
   void DrawMaze();
   void generate_occupancy_grid();
   void print_occupancy_grid() const;
   
  private: 

   int n_size, n_cells;
   int n_directions, nbits;
   int ImageSize;
   bool game_over;
   std::vector<int> direction;
   genmatrix *grid_ptr;

   std::vector<bool> visited_cell;
	// independent int = cell ID; dependent bool = visited flag

   std::vector<bool> deadend_cell;
	// independent int = cell ID; dependent bool = dead end cell

   std::vector<int> visited_cell_stack;

   std::vector<DUPLE> cell_decomposition;
// independent int: p
// dependent triple: px, py

   std::vector<int> soln_path;

   genmatrix *occupancy_grid;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const maze& T);

   int get_cell(int px, int py);
   void set_game_over(bool flag);
   std::string get_cell_bitstr(int px, int py);
   int get_direction_from_p_to_q(int p, int q);
   void remove_wall(int p, int curr_dir);
   

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


