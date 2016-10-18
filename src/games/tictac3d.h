// ==========================================================================
// Header file for tictac3d class 
// ==========================================================================
// Last modified on 8/29/16; 9/12/16; 10/12/16; 10/18/16
// ==========================================================================

#ifndef TICTAC3D_H
#define TICTAC3D_H

#include <map>
#include <vector>
#include "math/lttriple.h"
#include "math/threevector.h"

class tictac3d
{
   
  public:

   typedef std::map<triple, int, lttriple> WINNING_POSNS_MAP;
// independent triple:  winning board posns
// dependent int:  winner ID

// Initialization, constructor and destructor functions:

   tictac3d(int n_size, int n_zlevels);
   tictac3d(const tictac3d& C);
   ~tictac3d();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const tictac3d& C);

   void set_n_zlevels(int n);
   double get_score() const;
   bool get_game_over() const;
   genvector* get_board_state_ptr();

   void reset_board_state();
   void randomize_board_state();
   void display_board_state();
   void enter_human_move();
   int get_n_filled_cells() const;
   int check_player_win(int player_ID);

   double get_random_agent_move(bool print_flag = false);
   double set_agent_move(int px, int py, int pz, bool print_flag = false);
   void get_random_legal_AI_move();

  private: 

   bool game_over;
   int n_size;
   int n_zlevels;
   double curr_score;
   std::vector<int> curr_board_state;
   genvector* board_state_ptr;

   std::vector<std::vector<triple> > winnable_paths;

   WINNING_POSNS_MAP winning_posns_map;
   WINNING_POSNS_MAP::iterator winning_posns_iter;

   void allocate_member_objects();
   void initialize_member_objects();

   int get_cell_value(triple t) const;
   int get_cell_value(int px, int py, int pz) const;
   bool set_cell_value(int px, int py, int pz, int value);

   void display_Zgrid_state(int pz);

   bool winning_cell_posn(int player_ID, int px, int py, int pz);
   void print_winning_pattern();

   void generate_all_winnable_paths();
   void generate_winnable_Zplane_paths(int pz);
   void generate_winnable_Zcolumn_path(int px, int py);
   void generate_Zslant_xconst_winnable_paths(int px);
   void generate_Zslant_yconst_winnable_paths(int py);
   void generate_corner_2_corner_winnable_paths();

   bool Zplane_win(int player_ID, int pz);
   bool Zcolumn_win(int player_ID, int px, int py);
   bool corner_2_corner_win(int player_ID);
   bool Zslant_xconst_win(int player_ID, int px);
   bool Zslant_yconst_win(int player_ID, int py);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void tictac3d::set_n_zlevels(int n)
{
   n_zlevels = n;
}

inline double tictac3d::get_score() const
{
   return curr_score;
}

inline bool tictac3d::get_game_over() const
{
   return game_over;
}


#endif  // tictac3d.h


