// ==========================================================================
// tictac2d class member function definitions
// ==========================================================================
// Last modified on 8/23/15; 8/24/15; 8/26/15
// ==========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "games/tictac2d.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void tictac2d::allocate_member_objects()
{
}		       

void tictac2d::initialize_member_objects()
{
   game_counter = 0;
   nsize = 3;
   reset_game_params();
}		       

// ---------------------------------------------------------------------
void tictac2d::reset_game_params()
{
   game_counter++;
   curr_player = 1;
   winning_player = 0;
   curr_turn = 0;
   board_history.clear();
   board.clear_values();
   board_history.push_back(board);
}		       

// ---------------------------------------------------------------------
tictac2d::tictac2d()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

tictac2d::tictac2d(const tictac2d& T)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
tictac2d::~tictac2d()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const tictac2d& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

void tictac2d::initialize_weights()
{
   weight.push_back(0);
   x.push_back(1);
   
   for(int n = 0; n <= nsize; n++)
   {
      weight.push_back(pow(2,n));
      x.push_back(1);
   }

   for(int n = 0; n <= nsize; n++)
   {
      weight.push_back(-pow(2,n));
      x.push_back(1);
   }


/*
   weight.clear();

// 1K games
   weight.push_back(0.062946205421);
   weight.push_back(0.0175621744183);
   weight.push_back(-0.179695018082);
   weight.push_back(-0.091658231447);
   weight.push_back(0.778845210291);
   weight.push_back(0.0175621744183);
   weight.push_back(0.251083233722);
   weight.push_back(-0.0936479016781);
   weight.push_back(-1);

// 5K games
weight.push_back(0.0522506937388);
weight.push_back(0.134020893044);
weight.push_back(-0.224439461668);
weight.push_back(0.0671497923165);
weight.push_back(0.882490268006);
weight.push_back(-0.0211028619511);
weight.push_back(0.128281902946);
weight.push_back(-0.0169945674983);
weight.push_back(-1);

// 10K games

weight.push_back(0.173192093936);
weight.push_back(0.0329882794074);
weight.push_back(-0.136324104452);
weight.push_back(-0.0285253798507);
weight.push_back(0.872584160094);
weight.push_back(-0.017338588782);
weight.push_back(0.0388712205593);
weight.push_back(0.0152701830758);
weight.push_back(-1);

weight.push_back(0.150480760663);
weight.push_back(0.125006659407);
weight.push_back(-0.145662447422);
weight.push_back(-0.0620610180481);
weight.push_back(0.851080874554);
weight.push_back(0.0152293236031);
weight.push_back(0.115290881292);
weight.push_back(-0.058024405865);
weight.push_back(-1);

*/

// 100K games

   weight.push_back(0.154888190631);

   weight.push_back(0.076840128427);
   weight.push_back(-0.11468110145);
   weight.push_back(-0.000342117320314);
   weight.push_back(0.799191122694);

   weight.push_back(0.00383650796058);
   weight.push_back(0.131987913966);
   weight.push_back(-0.0400399634984);
   weight.push_back(-1);

   renormalize_weights();
   print_weights();
}		       

// ---------------------------------------------------------------------
void tictac2d::renormalize_weights()
{
   double max_weight = NEGATIVEINFINITY;
   for(unsigned int w = 0; w < weight.size(); w++)
   {
      max_weight = basic_math::max(max_weight, fabs(weight[w]));
   }

   cout << "max_weight = " << max_weight << endl;
   if(max_weight <= 0) return;

// Renormalize all weights relative to max_weight:

   for(unsigned int w = 0; w < weight.size(); w++)
   {
      weight[w] = weight[w] / max_weight;
   }
}

// ---------------------------------------------------------------------
void tictac2d::print_weights()
{
   for(unsigned int c = 0; c < weight.size(); c++)
   {
      cout << "c = " << c << " weight[c] = " << weight[c] << endl;
   }
}		       


// ---------------------------------------------------------------------
void tictac2d::print_final_weights()
{
   for(unsigned int c = 0; c < weight.size(); c++)
   {
      cout << "weight.push_back(" << weight[c] << ");" << endl;
   }
}		       

// ---------------------------------------------------------------------
void tictac2d::print_board_scores_map()
{
   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
   cout << "board_scores_map.size() = " << board_scores_map.size()
        << endl;
   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

   int n_boards = 1;
   BOARD_SCORES_MAP::iterator iter;
   for(iter = board_scores_map.begin(); iter != board_scores_map.end(); iter++)
   {
      threematrix curr_board = iter->first;
      int n_simuls = iter->second.first;
      double avgd_board_score = iter->second.second;
      cout << "Board #" << n_boards << endl;
      cout << curr_board << endl;
      cout << "n_simuls = " << n_simuls 
           << " avgd_board_score = " << avgd_board_score << endl;
      n_boards++;
   }
}

// ---------------------------------------------------------------------
void tictac2d::export_board_scores_map()
{
   string filename="board_scores_map.dat";
   ofstream outstream;
   filefunc::openfile(filename, outstream);
   outstream << "# Board_ID  n_simuls  Avg_board_score    Board_state" << endl;
   outstream << endl;

   int board_ID = 0;
   BOARD_SCORES_MAP::iterator iter;
   for(iter = board_scores_map.begin(); iter != board_scores_map.end(); iter++)
   {
      threematrix curr_board = iter->first;
      int n_simuls = iter->second.first;
      double avgd_board_score = iter->second.second;
      outstream << board_ID << "   "
                << n_simuls << "   "
                << avgd_board_score << "     ";
      for(int r = 0; r < nsize; r++)
      {
         for(int c = 0; c < nsize; c++)
         {
            outstream << curr_board.get(r,c) << " ";
         }
      }
      outstream << endl;
      board_ID++;
   }

   filefunc::closefile(filename, outstream);
   string banner="Exported board scores map to "+filename;
   outputfunc::write_banner(banner);
}

// ---------------------------------------------------------------------
void tictac2d::import_board_scores_map()
{
   string filename="board_scores_map.dat";
   filefunc::ReadInfile(filename);

   pair<int, double> P;
   int board_ID = 0;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> values = stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      board_ID = values[0];
      int n_simuls = values[1];
      double board_score = values[2];

      for(int r = 0; r < nsize; r++)
      {
         for(int c = 0; c < nsize; c++)
         {
            int counter = c + nsize * r;
            board.put(r,c,values[3+counter]);
         }
      }

//      cout << "board_ID = " << board_ID
//           << " n_simuls = " << n_simuls
//           << " board_score = " << board_score
//           << endl;
//      cout << board << endl;

      P.first = n_simuls;
      P.second = board_score;
      board_scores_map[board] = P;
   }
   board.clear_values();

   string banner="Imported "+stringfunc::number_to_string(board_ID + 1)
      +" boards and scores into map from "+filename;
   outputfunc::write_banner(banner);
}

// ---------------------------------------------------------------------
void tictac2d::score_board_history()
{
//   cout << "inside tictac2d::score_board_history()" << endl;
   int n_moves = board_history.size();
//   cout << "n_moves = " << n_moves << endl;

   pair<int, double> P;
   for(int i = 0; i < n_moves; i++)
   {
      threematrix curr_board = board_history[i];
      double player1_score = winning_player * double(i)/double(n_moves - 1);

      BOARD_SCORES_MAP::iterator iter;
      iter = board_scores_map.find(curr_board);
      if(iter == board_scores_map.end())
      {
         P.first = 1;
         P.second = player1_score;
         board_scores_map[curr_board] = P;
//         cout << "curr_score = " << P.second << endl;
      }
      else
      {
         P = iter->second;
         int prev_count = P.first;
         double prev_score = P.second;
         double curr_score = (prev_count * prev_score + player1_score) / 
            double(prev_count + 1);
         iter->second.first = prev_count + 1;
         iter->second.second = curr_score;
//         cout << "prev_score = " << prev_score 
//              << " curr_score = " << curr_score
//              << " prev_count = " << prev_count
//              << " curr_count = " << prev_count + 1 
//              << endl;
      }
   } // loop over index i labeling game moves

//   outputfunc::enter_continue_char();
}

void tictac2d::print_board_history()
{
   int n_moves = board_history.size();
   for(int i = 0; i < n_moves; i++)
   {
      cout << "Turn " << i ;
      if(i%2 == 0) 
      {
         cout << " : Player 1's move" << endl;
      }
      else
      {
         cout << " : Player -1's move" << endl;
      }
      cout << board_history[i] << endl;
//      double player1_score = winning_player * double(i)/(n_moves - 1);
//      cout << "Player 1 score = " << player1_score << endl;
   }

   if(winning_player == -1) outputfunc::enter_continue_char();
}


void tictac2d::print_board()
{
   cout << "................................." << endl;
   cout << "Turn " << curr_turn << endl;
   if(curr_turn > 0)
   {
      cout << "Current player: " << curr_player << endl;
   }

   cout << board << endl;
   if(curr_turn > 0)
   {
      vector<int> n_favorable_corridors = count_ways_to_win(curr_player);
      cout << "Current player's favorable corridors:" << endl;

      for(unsigned int c = 0; c < n_favorable_corridors.size(); c++)
      {
         cout << "n_corridors = " << n_favorable_corridors[c] 
              << " with " << c << " of current player's pieces"
              << endl;
      }

      cout << "Board score for player 1: " << estimate_board_score(1) << endl;
      cout << "Board score for player -1: " << estimate_board_score(-1) 
           << endl;
   }
}

// ---------------------------------------------------------------------
int tictac2d::n_empty_cells()
{
   int n_zeros = 0;
   for(int r = 0; r < nsize; r++)
   {
      for(int c = 0; c < nsize; c++)
      {
         if(nearly_equal(board.get(r,c), 0))
         {
            n_zeros++;
         }
      }
   }
   return n_zeros;
}

// ---------------------------------------------------------------------
bool tictac2d::legal_move(int row, int column)
{
   if(board.get(row,column) == 0)
   {
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
void tictac2d::make_next_move()
{   
   int curr_row, curr_column;
//   double random_move_threshold = -1;
   double random_move_threshold = 0.10;
//   double random_move_threshold = 0.25;

   if(nrfunc::ran1() < random_move_threshold)
   {
      bool curr_move_legal = false;
      while(!curr_move_legal)
      {
         curr_row = nrfunc::ran1() * nsize;
         curr_column = nrfunc::ran1() * nsize;
         curr_move_legal = legal_move(curr_row, curr_column);
      }
   }
   else
   {
      next_best_move(curr_row, curr_column);
   }

   board.put(curr_row, curr_column, curr_player);
   board_history.push_back(board);

   if(check_for_game_winner() != 0)
   {
      winning_player = curr_player;
   }
   else
   {
      curr_turn++;
   }
}


// ---------------------------------------------------------------------
void tictac2d::reset_curr_player()
{
   curr_player *= -1;
}


// ---------------------------------------------------------------------
// Member function check_for_game_winner() returns +/- 1 if either
// player has currently won.  Otherwise, it returns 0.

int tictac2d::check_for_game_winner()
{
   int sum;
   for(int r = 0; r < nsize; r++)
   {
      sum = 0;
      for(int c = 0; c < nsize; c++)
      {
         sum += board.get(r,c);
      }
      if(sum == nsize * curr_player) return curr_player;
   }

   for(int c = 0; c < nsize; c++)
   {
      sum = 0;
      for(int r = 0; r < nsize; r++)
      {
         sum += board.get(r,c);
      }
      if(sum == nsize * curr_player) return curr_player;
   }

   sum = 0;
   for(int d = 0; d < nsize; d++)
   {
      sum += board.get(d,d);
   }
   if(sum == nsize * curr_player) return curr_player;
   
   sum = 0;
   for(int d = 0; d < nsize; d++)
   {
      sum += board.get(d,nsize - 1 - d);
   }
   if(sum == nsize * curr_player) return curr_player;

   return 0;
}

// ---------------------------------------------------------------------
bool tictac2d::is_game_over()
{
   if(winning_player != 0)
   {
      cout << "*****************************************************" << endl;
      cout << "Player " << winning_player << " wins!" << endl;
      return true;
   }
   else if(n_empty_cells() == 0)
   {
      cout << "*****************************************************" << endl;
      cout << "Game over with no winner!" << endl;
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function count_ways_to_win() checks every possible way to
// win in 2D tic-tac-toe.  If any cell along a possible way is already
// blocked by the current player's opponent, then that way is not a
// winning option.  This method returns the total number of possible
// winning options for the current player.

vector<int> tictac2d::count_ways_to_win(int player_id)
{
   vector<int> n_favorable_corridors;
   for(int c = 0; c <= nsize; c++)
   {
      n_favorable_corridors.push_back(0);
   }

   bool winning_option;
   int sum;
   for(int r = 0; r < nsize; r++)
   {
      winning_option = true;
      sum = 0;
      for(int c = 0; c < nsize; c++)
      {
         if(board.get(r,c) == -player_id)
         {
            winning_option = false;
            break;
         }
         else
         {
            sum += board.get(r,c);
         }
      }
      if(winning_option)
      {
         int index = fabs(sum);
         n_favorable_corridors[index] = n_favorable_corridors[index] + 1;
      }
   }
   
   for(int c = 0; c < nsize; c++)
   {
      winning_option = true;
      sum = 0;
      for(int r = 0; r < nsize; r++)
      {
         if(board.get(r,c) == -player_id)
         {
            winning_option = false;
            break;
         }
         else
         {
            sum += board.get(r,c);
         }
      }
      if(winning_option)
      {
         int index = fabs(sum);
         n_favorable_corridors[index] = n_favorable_corridors[index] + 1;
      }
   }

   winning_option = true;
   sum = 0;
   for(int d = 0; d < nsize; d++)
   {
      if(board.get(d,d) == -player_id)
      {
         winning_option = false;
         break;
      }
      else
      {
         sum += board.get(d,d);
      }
   }
   if(winning_option)
   {
      int index = fabs(sum);
      n_favorable_corridors[index] = n_favorable_corridors[index] + 1;
   }
   
   winning_option = true;
   sum = 0;
   for(int d = 0; d < nsize; d++)
   {
      if(board.get(d,nsize - 1 - d) == -player_id)
      {
         winning_option = false;
         break;
      }
      else
      {
         sum += board.get(d, nsize - 1 - d);
      }
   }

   if(winning_option)
   {
      int index = fabs(sum);
      n_favorable_corridors[index] = n_favorable_corridors[index] + 1;
   }

   return n_favorable_corridors;
}
// ---------------------------------------------------------------------
// Member function estimate_board_score() assigns a score to the current
// board state for the specified input player based upon a weighted
// linear combination of hand-crafted features.

double tictac2d::estimate_board_score(int player_id)
{
   vector<int> n_favorable_corridors = count_ways_to_win(player_id);
   vector<int> n_unfavorable_corridors = count_ways_to_win(-player_id);
   
   int counter = 0;
   double score = 0;

   x[counter] = 1;
   score += weight[counter] * x[counter];
   counter++;
   
   for(unsigned int c = 0; c < n_favorable_corridors.size(); c++)
   {
      x[counter] = n_favorable_corridors[c];
      score += weight[counter] * x[counter];
      counter++;
   }

   for(unsigned int c = 0; c < n_unfavorable_corridors.size(); c++)
   {
      x[counter] = n_unfavorable_corridors[c];
      score += weight[counter] * x[counter];
      counter++;
   }
   return score;
}

// ---------------------------------------------------------------------
// Member function retrieve_board_score() 

double tictac2d::retrieve_board_score(int player_id)
{
   
   BOARD_SCORES_MAP::iterator iter;
   iter = board_scores_map.find(board);
   if(iter == board_scores_map.end())
   {
      cout << "Didn't find board within board_scores_map" << endl;
      return 0;
   }
   else
   {
      pair<int, double> P;
      P = iter->second;
      double score = player_id * P.second;
      return score;
   }
}

// ---------------------------------------------------------------------
// Member function best_next_move()

void tictac2d::next_best_move(int& best_r, int& best_c)
{
   best_r = best_c = -1;
   double max_score = NEGATIVEINFINITY;
   for(int r = 0; r < nsize; r++)
   {
      for(int c = 0; c < nsize; c++)
      {
         if(!legal_move(r, c)) continue;
         board.put(r, c, curr_player);         
//         double curr_score = estimate_board_score(curr_player);
         double curr_score = retrieve_board_score(curr_player);
         if(curr_score > max_score)
         {
            max_score = curr_score;
            best_r = r;
            best_c = c;
         }
         board.put(r, c, 0);         
      }
   }

//   cout << "curr_player = " << curr_player 
//        << " best_r = " << best_r << " best_c = " << best_c << endl;

/*
   if(best_r < 0 || best_c < 0)
   {
      cout << "Error in tictac2d::next_best_move()" << endl;
      exit(-1);
   }
*/

}

/*
// ---------------------------------------------------------------------
// Member function training_example_score()

double tictac2d::get_training_example_score()
{
   int best_r1, best_c1;
   int best_r2, best_c2;

// Compute current player's next best move and temporarily perform
// that move:

   next_best_move(best_r1, best_c1);
   if(best_r1 < 0 || best_c1 < 0) return NEGATIVEINFINITY;

   board.put(best_r1, best_c1, curr_player);            
   
   if(check_for_game_winner() != 0 || n_empty_cells() == 0)
   {
      board.put(best_r1, best_c1, 0);
      return NEGATIVEINFINITY;
   }
   
// Compute opponent player's next-to-next best move and temporarily
// perform that move:

   reset_curr_player();
   next_best_move(best_r2, best_c2);
   board.put(best_r2, best_c2, curr_player);
   
   if(check_for_game_winner() != 0 || n_empty_cells() == 0)
   {
      board.put(best_r1, best_c1, 0);
      board.put(best_r2, best_c2, 0);
      reset_curr_player();
      return NEGATIVEINFINITY;
   }

   reset_curr_player();
   double training_example_score = estimate_board_score(curr_player);
   board.put(best_r1, best_c1, 0);
   board.put(best_r2, best_c2, 0);

   return training_example_score;
}
*/

/*
// ---------------------------------------------------------------------
// Member function update_weights()

void tictac2d::update_weights()
{
//   cout << "inside update_weights()" << endl;

   double eta = 0.1 * pow(0.999, game_counter);
   double curr_board_score = estimate_board_score(curr_player);
   double training_example_score = get_training_example_score();
//   cout << "game_counter = " << game_counter << " eta = " << eta << endl;

   if(training_example_score < -1000) 
   {
      training_example_score = curr_board_score;
   }
   
//   cout << "curr_board_score = " << curr_board_score << endl;
//   cout << "training_example_score = " << training_example_score << endl;

   for(unsigned int c = 0; c < weight.size(); c++)
   {
      weight[c] = weight[c] + eta * (
         training_example_score - curr_board_score) * x[c];
   }

// Renormalize weights:

   double weight_sum = 0;
   for(unsigned int c = 0; c < weight.size()/2; c++)
   {
      weight_sum += weight[c];
   }
   for(unsigned int c = 0; c < weight.size()/2; c++)
   {
      weight[c] = weight[c] / fabs(weight_sum);
   }

   weight_sum = 0;
   for(unsigned int c = 0; c < weight.size()/2; c++)
   {
      weight_sum += weight[c+weight.size()/2];
   }
   for(unsigned int c = 0; c < weight.size()/2; c++)
   {
      weight[c+weight.size()/2] = 
         weight[c+weight.size()/2] / fabs(weight_sum);
   }

   for(unsigned int c = 0; c < weight.size(); c++)
   {
      cout << "c = " << c 
           << " weight[c] = " << weight[c]
           << " x[c] = " << x[c]
           << endl;
   }
}
*/


// ---------------------------------------------------------------------
// Member function train_weights()

void tictac2d::train_weights()
{
   cout << "inside train_weights()" << endl;

   int player_id = 1;
   int n_training_examples = 1;
   BOARD_SCORES_MAP::iterator iter;
   for(iter = board_scores_map.begin(); iter != board_scores_map.end(); iter++)
   {
      board = iter->first;
      double measured_board_score = iter->second.second;
      double estimated_board_score = estimate_board_score(player_id);
      double eta = 0.1 * pow(0.999, n_training_examples);

      cout << "n_training_examples = " << n_training_examples << endl;
      for(unsigned int c = 0; c < weight.size(); c++)
      {
         weight[c] = weight[c] + eta * (
            measured_board_score - estimated_board_score) * x[c];
//         cout << "c = " << c 
//              << " weight[c] = " << weight[c] 
//              << " x[c] = " << x[c] << endl;
      }
      renormalize_weights();
      print_weights();
      n_training_examples++;
   } // loop over iter index labeling training examples

// After adjusting weights, compare measured vs estimated board
// scores:

   vector<double> measured_board_scores, estimated_board_scores;
   vector<double> error_board_scores;
   for(iter = board_scores_map.begin(); iter != board_scores_map.end(); iter++)
   {
      board = iter->first;
      double measured_board_score = iter->second.second;
      double estimated_board_score = estimate_board_score(player_id);
      double error_board_score = measured_board_score - estimated_board_score;
      measured_board_scores.push_back(measured_board_score);
      estimated_board_scores.push_back(estimated_board_score);
      error_board_scores.push_back(error_board_score);
   }

   prob_distribution prob_measured(measured_board_scores, 50);
   prob_distribution prob_estimated(estimated_board_scores, 50);
   prob_distribution prob_error(error_board_scores, 50);

//   prob_measured.writeprobdists(false,true);
//   prob_estimated.writeprobdists(false,true);
   prob_error.writeprobdists(false,true);
}

