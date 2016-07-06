// ==========================================================================
// Header file for tictac2d class 
// ==========================================================================
// Last modified on 8/23/15; 8/24/15; 8/26/15
// ==========================================================================

#ifndef TICTAC2D_H
#define TICTAC2D_H

#include <map>
#include <vector>
#include "math/ltmatrix.h"
#include "math/threematrix.h"

class tictac2d
{
   
  public:

   typedef std::map<threematrix, std::pair<int, double>, ltmatrix> 
      BOARD_SCORES_MAP;

// independent threematrix key:  board state
// dependent pair<int, double> value: 
//   int = number of times board state has been simulated
//   double = board score averaged over all simulations for player 1
//            Multiply board score by -1 to obtain score for player -1 

// Initialization, constructor and destructor functions:

   tictac2d();
   tictac2d(const tictac2d& C);
   ~tictac2d();
//   tictac2d operator= (const tictac2d& C);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const tictac2d& C);

   void reset_game_params();
   void initialize_weights();
   void renormalize_weights();
   void print_weights();

   void print_board_scores_map();
   void export_board_scores_map();
   void import_board_scores_map();
   void score_board_history();

   void print_board_history();
   void print_board();
   void make_next_move();
   void reset_curr_player();
   bool is_game_over();
   void train_weights();
   void print_final_weights();

  private: 

   int game_counter;
   int nsize;		// 2D board contains nsize x nsize cells
   int curr_player;	// 1 for "x", -1 for "o"
   int winning_player;
   int curr_turn;
   threematrix board;
   std::vector<threematrix> board_history;
   BOARD_SCORES_MAP board_scores_map;
   std::vector<double> weight, x;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const tictac2d& T);

   bool legal_move(int row, int column);
   int n_empty_cells();
   int check_for_game_winner();
   std::vector<int> count_ways_to_win(int player_id);
   double estimate_board_score(int player_id);
   double retrieve_board_score(int player_id);
   void next_best_move(int& best_r, int& best_c);
//   double get_training_example_score();
//   void update_weights();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:



#endif  // tictac2d.h


