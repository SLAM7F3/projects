// ==========================================================================
// Header file for environment class 
// ==========================================================================
// Last modified on 12/7/16; 12/8/16; 12/10/16; 12/30/16
// ==========================================================================

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <iostream>
#include <string>
#include <vector>
#include "games/breakout.h"
#include "math/genvector.h"
#include "games/maze.h"
#include "games/pong.h"
#include "games/spaceinv.h"
#include "games/tictac3d.h"
#include "image/TwoDarray.h"

class environment
{
   
  public:

   typedef enum{
      MAZE = 0,
      TTT = 1,
      SPACEINV = 2,
      BREAKOUT = 3,
      PONG = 4
   } environment_t;

// Initialization, constructor and destructor functions:

   environment(int world_type);
   ~environment();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const environment& E);

   void set_use_big_states_flag(bool flag);
   void set_frame_skip(int skip);
   int get_frame_skip() const;
   void set_maze(maze* m_ptr);
   maze* get_maze();
   const maze* get_maze() const;
   void set_spaceinv(spaceinv* s_ptr);
   spaceinv* get_spaceinv();
   const spaceinv* get_spaceinv() const;
   void set_breakout(breakout* b_ptr);
   breakout* get_breakout();
   const breakout* get_breakout() const;
   void set_pong(pong* p_ptr);
   pong* get_pong();
   const pong* get_pong() const;

   void set_tictac3d(tictac3d* t_ptr);
   tictac3d* get_tictac3d();
   const tictac3d* get_tictac3d() const;

   void start_new_episode(bool random_start = false);
   genvector* get_curr_state();
   std::string get_curr_state_string();
   genvector* get_next_state();
   bool is_legal_action(int a);
   genvector* compute_next_state(int a, int player_value = 1);
   bool is_terminal_state();
   bool get_game_over();
   void set_game_over(bool flag);

   int get_episode_framenumber() const;
   int get_min_episode_framenumber() const;
   double get_max_score_per_episode() const;

   std::string get_state_action_string(std::string state_str, int a);
   std::string get_state_action_string(genvector* state_ptr, int a);
   std::vector<genvector*> get_all_curr_states();
   std::vector<std::string> get_all_curr_state_strings();

   void append_wtwoDarray(twoDarray* wtwoDarray_ptr);

   std::vector<genvector*>* get_all_afterstates(int player_value);

  private:

   bool use_big_states_flag;
   int world_type;
   int frame_skip;

   maze *maze_ptr;
   breakout *breakout_ptr;
   pong *pong_ptr;
   spaceinv *spaceinv_ptr;
   tictac3d *tictac3d_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void environment::set_use_big_states_flag(bool flag)
{
   use_big_states_flag = flag;
}

inline void environment::set_frame_skip(int skip)
{
   frame_skip = skip;
}

inline int environment::get_frame_skip() const
{
   return frame_skip;
}


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

inline void environment::set_spaceinv(spaceinv* s_ptr)
{
   spaceinv_ptr = s_ptr;
}

inline spaceinv* environment::get_spaceinv()
{
   return spaceinv_ptr;
}

inline const spaceinv* environment::get_spaceinv() const
{
   return spaceinv_ptr;
}

inline void environment::set_breakout(breakout* b_ptr)
{
   breakout_ptr = b_ptr;
}

inline breakout* environment::get_breakout()
{
   return breakout_ptr;
}

inline const breakout* environment::get_breakout() const
{
   return breakout_ptr;
}

inline void environment::set_pong(pong* p_ptr)
{
   pong_ptr = p_ptr;
}

inline pong* environment::get_pong()
{
   return pong_ptr;
}

inline const pong* environment::get_pong() const
{
   return pong_ptr;
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



