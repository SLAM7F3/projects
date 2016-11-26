// ==========================================================================
// Header file for environment class 
// ==========================================================================
// Last modified on 11/17/16; 11/18/16; 11/25/16; 11/26/16
// ==========================================================================

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <iostream>
#include <string>
#include <vector>
#include "math/genvector.h"
#include "games/maze.h"
#include "games/tictac3d.h"
#include "image/TwoDarray.h"

class environment
{
   
  public:

   typedef enum{
      MAZE = 0,
      TTT = 1
   } environment_t;

// Initialization, constructor and destructor functions:

   environment(int world_type);
   ~environment();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const environment& E);

   void set_maze(maze* m_ptr);
   maze* get_maze();
   const maze* get_maze() const;
   void set_tictac3d(tictac3d* t_ptr);
   tictac3d* get_tictac3d();
   const tictac3d* get_tictac3d() const;

   void start_new_episode(bool random_start = false);
   genvector* get_curr_state();
   std::string get_curr_state_string();
   bool is_legal_action(int a);
   genvector* compute_next_state(int a, int player_value = 1);
   bool is_terminal_state();
   bool get_game_over();
   void set_game_over(bool flag);

   void set_reward(double r);
   double get_reward() const;

   std::string get_state_action_string(std::string state_str, int a);
   std::string get_state_action_string(genvector* state_ptr, int a);
   std::vector<genvector*> get_all_curr_states();
   std::vector<std::string> get_all_curr_state_strings();

   void append_wtwoDarray(twoDarray* wtwoDarray_ptr);

  private:

   int world_type;
   double reward;
   maze *maze_ptr;
   tictac3d *tictac3d_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void environment::set_maze(maze* m_ptr)
{
   maze_ptr = m_ptr;
}

inline maze* environment::get_maze()
{
   return maze_ptr;
}

inline const maze* environment::get_maze() const
{
   return maze_ptr;
}

inline void environment::set_tictac3d(tictac3d* t_ptr)
{
   tictac3d_ptr = t_ptr;
}

inline tictac3d* environment::get_tictac3d()
{
   return tictac3d_ptr;
}

inline const tictac3d* environment::get_tictac3d() const
{
   return tictac3d_ptr;
}

#endif  // environment.h


