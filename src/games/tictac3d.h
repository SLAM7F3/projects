// ==========================================================================
// Header file for tictac3d class 
// ==========================================================================
// Last modified on 11/2/16; 11/3/16; 11/4/16; 11/25/16
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

   typedef std::map<int, int> WINNING_POSNS_MAP;
// independent int:  winning board cell indices
// dependent int:  winner ID

   typedef std::map<int, std::vector<int> > CELL_WINNABLE_PATHS_MAP;
// independent int: cell ID
// winnable path IDs in which cell participates

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
   void set_recursive_depth(int d);

   int get_cell(int px, int py, int pz);
   void set_game_over(bool flag);
   bool get_game_over() const;
   genvector* get_board_state_ptr();
   genvector* get_inverse_board_state_ptr();
   std::string board_state_to_string();

   void push_genuine_board_state();
   void pop_genuine_board_state();
   bool check_filled_board();

   void increment_n_human_turns();
   int get_n_human_turns() const;
   void increment_n_AI_turns();
   int get_n_AI_turns() const;
   void increment_n_agent_turns();
   int get_n_agent_turns() const;
   int get_n_completed_turns() const;

   void reset_board_state();
   void randomize_board_state();
   void display_minimax_scores(int player_value);
   void display_p_action(genvector* p_action);
   void display_board_state();
   void enter_player_move(int player_value);

   int get_n_total_cells() const;
   int get_n_filled_cells() const;
   int get_n_empty_cells() const;
   int check_player_win(int player_ID, bool print_flag = false);
   int check_opponent_win_on_next_turn(int player_value);
   void adjust_intrinsic_cell_prizes();
   void print_winning_pattern();

   double get_random_player_move(int agent_value);

   bool legal_player_move(int p);
   bool set_player_move(int p, int player_value);
   void get_random_legal_player_move(int player_value);
   void record_latest_move(int player_value, int p);

   void append_game_loss_frac(double frac);
   void append_game_illegal_frac(double frac);
   void append_game_stalemate_frac(double frac);
   void append_game_win_frac(double frac);

   int imminent_win_or_loss(int player_value);
   int get_recursive_minimax_move(int player_value);

   double get_minimax_move_score(
      int curr_node, int depth, int player_value, bool maximizing_flag);

   double get_alphabeta_minimax_move_score(
      int curr_node, int depth, double alpha, double beta, int player_value,
      bool maximizing_flag);

   void extremal_winnable_path_scores(
      int player_value, double& integrated_player_path_score, 
      double& integrated_opponent_path_score);
   void compute_winnable_path_occupancies(int player_value);

   void plot_game_frac_histories(int n_episodes, std::string extrainfo);

  private: 

   bool game_over;
   int n_size, n_size_sqr, n_zlevels, n_cells;
   int recursive_depth;
   int n_human_turns, n_AI_turns, n_agent_turns;
   int* curr_board_state;
   std::vector<int*> Genuine_Board_states;
   int** genuine_board_state_ptrs;

   std::vector<triple> cell_decomposition;
// independent int: p
// dependent triple: px, py, pz	

   genvector *board_state_ptr;
   genvector *inverse_board_state_ptr;

   std::vector<double> game_loss_frac, game_illegal_frac;
   std::vector<double> game_stalemate_frac, game_win_frac;

   typedef struct
   {
      std::vector<int> path;  // Holds cell indices for winnable path
//      double X_score;
//      double O_score;
   } winnable_path_t;
   
   int n_winnable_paths;
   std::vector<winnable_path_t> winnable_paths;
   int n_score_evaluations;

   WINNING_POSNS_MAP winning_posns_map;
   WINNING_POSNS_MAP::iterator winning_posns_iter;

   CELL_WINNABLE_PATHS_MAP cell_winnable_paths_map;
   CELL_WINNABLE_PATHS_MAP::iterator cell_winnable_paths_iter;
   std::vector<int> intrinsic_cell_prize;
   int min_intrinsic_cell_prize, max_intrinsic_cell_prize;

   PATH_OCCUPANCY_MAP path_occupancy_map;
   PATH_OCCUPANCY_MAP::iterator path_occupancy_iter;

   LATEST_MOVE_MAP latest_move_map;
   LATEST_MOVE_MAP::iterator latest_move_iter;

   int latest_O_move, latest_X_move;

   void allocate_member_objects();
   void initialize_member_objects();

   int get_cell_value(int p) const;
   void set_cell_value(int p, int value);

   void display_Zgrid_state(int pz);
   bool winning_cell_posn(int player_ID, int px, int py, int pz);

   void generate_all_winnable_paths();
   void correlate_cells_with_winnable_paths();

   void print_winnable_path(int path_ID);
   std::vector<int>* get_winnable_path_IDs(int cell_ID);
   void print_cell_ID_vs_winnable_path_IDs();

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

inline int tictac3d::get_cell(int px, int py, int pz)
{
   return n_size * n_size * pz + n_size * py + px;
}

inline void tictac3d::set_game_over(bool flag)
{
   game_over = flag;
}

inline bool tictac3d::get_game_over() const
{
   return game_over;
}

inline void tictac3d::increment_n_human_turns()
{
   n_human_turns++;
}

inline int tictac3d::get_n_human_turns() const
{
   return n_human_turns;
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

inline int tictac3d::get_n_completed_turns() const
{
   return (n_human_turns + n_AI_turns + n_agent_turns)/2;
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

// ---------------------------------------------------------------------
// Boolean member function legal_player_move() returns false if
// cell corresponding to input p --> (px,py,pz) is already occupied.

inline bool tictac3d::legal_player_move(int p)
{
   return (curr_board_state[p] == 0);
}

inline void tictac3d::set_cell_value(int p, int value)
{
   curr_board_state[p] = value;
}

inline int tictac3d::get_cell_value(int p) const
{
   return curr_board_state[p];
}


#endif  // tictac3d.h




