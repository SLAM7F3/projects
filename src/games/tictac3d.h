// ==========================================================================
// Header file for tictac3d class 
// ==========================================================================
// Last modified on 10/28/16; 10/29/16; 10/30/16; 10/31/16
// ==========================================================================

#ifndef TICTAC3D_H
#define TICTAC3D_H

#include <map>
#include <vector>
#include "math/ltduple.h"
#include "math/lttriple.h"
#include "math/threevector.h"

class tictac3d
{
   
  public:

   typedef std::map<triple, int, lttriple> WINNING_POSNS_MAP;
// independent triple:  winning board posns
// dependent int:  winner ID

   typedef std::map<DUPLE, int, ltduple> PATH_OCCUPANCY_MAP;
// independent duple: (player_value, path index)
// dependent int: path occupancy for player_value  
//           (-1 if occupied by both players)

   typedef std::map<int, int> LATEST_MOVE_MAP;
// independent int: 1 or -1 player value
// dependent int: cell index

// Initialization, constructor and destructor functions:

   tictac3d(int n_size, int n_zlevels);
   tictac3d(const tictac3d& C);
   ~tictac3d();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const tictac3d& C);

   int get_n_size() const;
   void set_n_zlevels(int n);
   void set_game_over(bool flag);
   bool get_game_over() const;
   genvector* get_board_state_ptr();
   genvector* get_inverse_board_state_ptr();
   void push_genuine_board_state();
   void pop_genuine_board_state();
   bool check_filled_board();

   void increment_n_AI_turns();
   int get_n_AI_turns() const;
   void increment_n_agent_turns();
   int get_n_agent_turns() const;

   void reset_board_state();
   void randomize_board_state();
   void display_p_action(genvector* p_action);
   void display_board_state();
   void enter_player_move(int player_value);

   int get_n_total_cells() const;
   int get_n_filled_cells() const;
   int get_n_empty_cells() const;
   int check_player_win(int player_ID, bool print_flag = false);
   void print_winning_pattern();

   double get_random_player_move(int agent_value);
   bool legal_player_move(int px, int py, int pz, bool print_flag = false);
   bool set_player_move(triple t, int player_value);
   bool set_player_move(int px, int py, int pz, int player_value);
   void get_random_legal_player_move(int player_value);
   void record_latest_move(int player_value, triple t);
   void record_latest_move(int player_value, int px, int py, int pz);

   void append_game_loss_frac(double frac);
   void append_game_illegal_frac(double frac);
   void append_game_stalemate_frac(double frac);
   void append_game_win_frac(double frac);


   void max_move(int player_value, triple& best_xyz);
   void minimax_move(int player_value, triple& best_xyz);


   double get_max_best_player_score(int player_value, triple& t);
   double get_min_best_opponent_score(int player_value);
   
   double best_winnable_path(int player_value);
   void compute_winnable_path_occupancies(int player_value);
   void plot_game_frac_histories(int n_episodes, std::string extrainfo);

  private: 

   bool game_over;
   int n_size;
   int n_zlevels;
   int n_AI_turns, n_agent_turns;
   std::vector<int> curr_board_state;
   std::vector<std::vector<int> > genuine_board_state;
   genvector *board_state_ptr;
   genvector *inverse_board_state_ptr;

   std::vector<double> game_loss_frac, game_illegal_frac;
   std::vector<double> game_stalemate_frac, game_win_frac;

   std::vector<std::vector<triple> > winnable_paths;

   WINNING_POSNS_MAP winning_posns_map;
   WINNING_POSNS_MAP::iterator winning_posns_iter;

   PATH_OCCUPANCY_MAP path_occupancy_map;
   PATH_OCCUPANCY_MAP::iterator path_occupancy_iter;

   LATEST_MOVE_MAP latest_move_map;
   LATEST_MOVE_MAP::iterator latest_move_iter;

   triple latest_O, latest_X;

   void allocate_member_objects();
   void initialize_member_objects();

   int get_cell_value(triple t) const;
   int get_cell_value(int px, int py, int pz) const;
   triple decompose_cell_index(int p);
   bool set_cell_value(int px, int py, int pz, int value);

   void display_Zgrid_state(int pz);

   bool winning_cell_posn(int player_ID, int px, int py, int pz);


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

inline int tictac3d::get_n_size() const
{
   return n_size;
}

inline void tictac3d::set_n_zlevels(int n)
{
   n_zlevels = n;
}

inline void tictac3d::set_game_over(bool flag)
{
   game_over = flag;
}

inline bool tictac3d::get_game_over() const
{
   return game_over;
}

inline void tictac3d::increment_n_AI_turns()
{
   n_AI_turns++;
}

inline int tictac3d::get_n_AI_turns() const
{
   return n_AI_turns;
}

inline void tictac3d::increment_n_agent_turns()
{
   n_agent_turns++;
}

inline int tictac3d::get_n_agent_turns() const
{
   return n_agent_turns;
}

inline void tictac3d::append_game_loss_frac(double frac)
{
   game_loss_frac.push_back(frac);
}

inline void tictac3d::append_game_illegal_frac(double frac)
{
   game_illegal_frac.push_back(frac);
}

inline void tictac3d::append_game_stalemate_frac(double frac)
{
   game_stalemate_frac.push_back(frac);
}

inline void tictac3d::append_game_win_frac(double frac)
{
   game_win_frac.push_back(frac);
}


#endif  // tictac3d.h




