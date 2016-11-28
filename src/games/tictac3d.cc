// ==========================================================================
// tictac3d class member function definitions
// ==========================================================================
// Last modified on 11/25/16; 11/26/16; 11/27/16; 11/28/16
// ==========================================================================

#include <iostream>
#include <string>
#include "color/colortext.h"
#include "general/filefuncs.h"
#include "plot/metafile.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "games/tictac3d.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void tictac3d::allocate_member_objects()
{
   n_size_sqr = n_size * n_size;
   n_cells = n_size_sqr * n_zlevels;
   curr_board_state = new int[n_cells];
   board_state_ptr = new genvector(n_cells);
   inverse_board_state_ptr = new genvector(n_cells);
   for(int s = 0; s < n_cells; s++)
   {
      afterstate_ptrs.push_back(new genvector(n_cells));
   }
}		       

void tictac3d::initialize_member_objects()
{
   n_human_turns = n_AI_turns = n_agent_turns = 0;
   game_over = false;
   generate_all_winnable_paths();
   correlate_cells_with_winnable_paths();
//    print_cell_ID_vs_winnable_path_IDs();

   board_state_ptr->clear_values();
   genuine_board_state_ptrs = NULL;

// Load cell_decomposition vector with (px,py,pz) triples
// corresponding to p = 0 --> n_cells - 1:

   for(int q = 0; q < n_cells; q++)
   {
      int p = q;
      int pz = p / (n_size_sqr);
      p -= n_size_sqr * pz;
      int py = p / n_size;
      p -= n_size * py;
      int px = p;
      cell_decomposition.push_back(triple(px,py,pz));
   }
}		       

// ---------------------------------------------------------------------
tictac3d::tictac3d(int n_size, int n_zlevels)
{
   this->n_size = n_size;
   this->n_zlevels = n_zlevels;

   allocate_member_objects();
   initialize_member_objects();

   reset_board_state();
}

// Copy constructor:

tictac3d::tictac3d(const tictac3d& T)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
tictac3d::~tictac3d()
{
   delete [] curr_board_state;
   delete board_state_ptr;
   delete inverse_board_state_ptr;

   if(genuine_board_state_ptrs != NULL)
   {
      for(int d = 0; d < recursive_depth + 1; d++)
      {
         delete genuine_board_state_ptrs[d];
      }
      delete genuine_board_state_ptrs;
   }

   for(int s = 0; s < n_cells; s++)
   {
      delete afterstate_ptrs[s];
   }
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const tictac3d& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

void tictac3d::set_recursive_depth(int d)
{
   recursive_depth = d;

   genuine_board_state_ptrs = new int*[recursive_depth + 1];
   for(int d = 0; d < recursive_depth + 1; d++)
   {
      genuine_board_state_ptrs[d] = new int[n_cells];
   }
}

genvector* tictac3d::get_board_state_ptr()
{
   for(int p = 0; p < n_cells; p++)
   {
      board_state_ptr->put(p, curr_board_state[p]);
   }
   return board_state_ptr;
}

// ---------------------------------------------------------------------
// Member function get_inverse_board_state_ptr() returns the board
// state vector after swapping all "X" and "O" pieces.

genvector* tictac3d::get_inverse_board_state_ptr()
{
   for(int p = 0; p < n_cells; p++)
   {
      inverse_board_state_ptr->put(p, -curr_board_state[p]);
   }
   return inverse_board_state_ptr;
}

// ---------------------------------------------------------------------
// Member function board_state_to_string() converts genvector
// *board_state_ptr into a corresponding string containing "X" for
// AI pieces, "O" for agent pieces and "E" for empty cell
// locations.  The string can be used as a key inside STL maps.

string tictac3d::board_state_to_string()
{
   string board_state_str="";
   for(int p = 0; p < n_cells; p++)
   {
      double cell_value = board_state_ptr->get(p);
      if(cell_value > 0)
      {
         board_state_str += "O";
      }
      else if(cell_value < 0)
      {
         board_state_str += "X";
      }
      else
      {
         board_state_str += "E";
      }
   }
   return board_state_str;
}

// ---------------------------------------------------------------------
// Member function push_genuine_board_state() creates a copy of
// curr_board_state before we might start to intentionally alter the
// contents of curr_board_state for path planning.

void tictac3d::push_genuine_board_state()
{
   int g = Genuine_Board_states.size();
   memcpy(genuine_board_state_ptrs[g], curr_board_state,
          n_cells * sizeof(int));
   Genuine_Board_states.push_back(genuine_board_state_ptrs[g]);
}

void tictac3d::pop_genuine_board_state()
{
   int g = Genuine_Board_states.size();
   memcpy(curr_board_state, genuine_board_state_ptrs[g-1],
          n_cells * sizeof(int));
   Genuine_Board_states.pop_back();
}

// ---------------------------------------------------------------------
bool tictac3d::check_filled_board()
{
   if(get_n_empty_cells() == 0)
   {
      game_over = true;
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function record_latest_move()

void tictac3d::record_latest_move(int player_value, int p)
{
   latest_move_iter = latest_move_map.find(player_value);
   if(latest_move_iter == latest_move_map.end())
   {
      latest_move_map[player_value] = p;
   }
   else
   {
      latest_move_iter->second = p;
   }
}

// ---------------------------------------------------------------------
int tictac3d::get_n_total_cells() const
{
   return n_zlevels * n_size * n_size;
}

// ---------------------------------------------------------------------
int tictac3d::get_n_filled_cells() const
{
   int n_filled_cells = 0;
   for(int p = 0; p < n_cells; p++)
   {
      if(get_cell_value(p) != 0)
      {
         n_filled_cells++;
      }
   } // loop over p

   return n_filled_cells;
}

// ---------------------------------------------------------------------
int tictac3d::get_n_empty_cells() const
{
   return get_n_total_cells() - get_n_filled_cells();
}

// ---------------------------------------------------------------------
void tictac3d::enter_player_move(int player_value)
{
   int px, py, pz = 0;
   int p = -1;
   bool legal_move = false;
   while(!legal_move)
   {
      if(n_zlevels > 1)
      {
         cout << "Enter Z level:" << endl;
         cin >> pz;
         if(pz < 0 || pz >= n_zlevels)
         {
            cout << "Invalid Z value.  Please try again:" << endl;
            continue;
         }
      }

      cout << "Enter column: " << endl;
      cin >> px;
      if(px < 0 || px >= n_size)
      {
         cout << "Illegal column value.  Please try again" << endl;
         continue;
      }
      
      cout << "Enter row: " << endl;
      cin >> py;
      if(py < 0 || py >= n_size)
      {
         cout << "Illegal row value.  Please try again" << endl;
         continue;
      }

      p = get_cell(px,py,pz);
      legal_move = legal_player_move(p);
      if(!legal_move)
      {
         cout << "Cell is already occupied.  Please try again" << endl;
      }
   } // while !legal_move loop
   set_cell_value(p, player_value);
   record_latest_move(player_value, p);
}

// ---------------------------------------------------------------------
double tictac3d::get_random_player_move(int player_value)
{
   int p = mathfunc::getRandomInteger(n_cells);
   return set_player_move(p, player_value);
}

// ---------------------------------------------------------------------
bool tictac3d::set_player_move(int p, int player_value)
{
// Cell values:

// -1   --> AI
// 0    --> empty
// 1    --> agent
// 2    --> 2 agent (illegal)
// 3    --> agent + AI (illegal)

   bool legal_move = true;
   if(!legal_player_move(p))
   {
      legal_move = false;
      if(get_cell_value(p) == -1)
      {
         player_value = 3;
      }
      else if(get_cell_value(p) == 1)
      {
         player_value = 2;
      }
   }
   set_cell_value(p, player_value);
   return legal_move;
}

// ---------------------------------------------------------------------
void tictac3d::get_random_legal_player_move(int player_value)
{
   bool legal_move_flag = false;
   while(!legal_move_flag)
   {
      int p = mathfunc::getRandomInteger(n_cells);
      if(legal_player_move(p))
      {
         set_cell_value(p, player_value);
         legal_move_flag = true;
      }
   }
}

// ---------------------------------------------------------------------
void tictac3d::reset_board_state()
{
   n_AI_turns = n_agent_turns = 0;
   winning_posns_map.clear();
   for(int p = 0; p < n_cells; p++)
   {
      curr_board_state[p] = 0;
   } 
   game_over = false;
}		       

// ---------------------------------------------------------------------
void tictac3d::randomize_board_state()
{
   winning_posns_map.clear();
   for(int p = 0; p < n_cells; p++)
   {
      double curr_ran = nrfunc::ran1();
      int curr_val = 0;
      if(curr_ran < 0.33)
      {
         curr_val = -1;
      }
      else if (curr_ran > 0.66)
      {
         curr_val = 1;
      }
      
      curr_board_state[p] = curr_val;
   }
}		       

// ---------------------------------------------------------------------
void tictac3d::display_minimax_scores(int player_value)
{
//   cout << "inside display_minimax_scores(), player_value = " 
//        << player_value << endl;
   
   Color::Modifier green(Color::FG_GREEN);
   Color::Modifier purple(Color::FG_PURPLE);
   Color::Modifier yellow(Color::FG_YELLOW);
   Color::Modifier grey(Color::FG_GREY);
   Color::Modifier red(Color::FG_RED);
   Color::Modifier cyan(Color::FG_CYAN);
   Color::Modifier def(Color::FG_DEFAULT);
   Color::Modifier winning_color(Color::FG_CYAN);

   bool maximizing_player;
   if(recursive_depth%2 == 0)
   {
      maximizing_player = true;
   }
   else
   {
      maximizing_player = false;
   }

   vector<double> minimax_scores;
   for(int curr_node = 0; curr_node < n_cells; curr_node++)
   {
      if(!legal_player_move(curr_node))
      {
         minimax_scores.push_back(NEGATIVEINFINITY);
      }
      else
      {
         minimax_scores.push_back(
            get_minimax_move_score(
               curr_node, recursive_depth, player_value, maximizing_player));
      }
   }
   double max_score = mathfunc::maximal_value(minimax_scores);

// Renormalize minimax scores so that maximal value --> 1

   for(int curr_node = 0; curr_node < n_cells; curr_node++)
   {
      minimax_scores[curr_node] /= max_score;
   }

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      cout << endl;
      if(n_zlevels > 1)
      {
         cout << "Z = " << pz << endl << endl;
      }
      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
         {
            int p = get_cell(px, py, pz);
            int cell_value = get_cell_value(p);
            if(cell_value == 1)
            {
               cout << yellow << "   O   " << def << flush;
            }
            else if (cell_value == -1)
            {
               cout << red << "   X   " << def << flush;
            }
            else
            {
               cout << stringfunc::number_to_string(minimax_scores[p],3) 
                    << "  ";
            }
         } // loop over px index
         cout << endl;

         if(py < n_size - 1)
         {
            for(int px = 0; px < n_size; px++)
            {
               cout << "------" << flush;
            }
            cout << endl;
         }
         
      } // loop over py index
   } // loop over pz index
}

// ---------------------------------------------------------------------
void tictac3d::display_p_action(genvector* p_action)
{
   double p_sum = 0;
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      cout << endl;
      if(n_zlevels > 1)
      {
         cout << "Z = " << pz << endl << endl;
      }
      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
         {
            int p = get_cell(px, py, pz);
            double curr_prob = p_action->get(p);
            p_sum += curr_prob;
            cout << stringfunc::number_to_string(curr_prob,3) << "  ";
         } // loop over px index
         cout << endl;

         if(py < n_size - 1)
         {
            for(int px = 0; px < n_size; px++)
            {
               cout << "------" << flush;
            }
            cout << endl;
         }
         
      } // loop over py index
   } // loop over pz index
//   cout << "p_sum = " << p_sum << endl;
}

// ---------------------------------------------------------------------
void tictac3d::display_board_state()
{
//   sysfunc::clearscreen();
   cout << "......................................................." << endl;

// First retrieve cells containing latest X and O moves:

   latest_O_move = -1;
   latest_move_iter = latest_move_map.find(1);
   if(latest_move_iter != latest_move_map.end())
   {
      latest_O_move = latest_move_iter->second;
   }

   latest_X_move = -1;
   latest_move_iter = latest_move_map.find(-1);
   if(latest_move_iter != latest_move_map.end())
   {
      latest_X_move = latest_move_iter->second;
   }

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      cout << endl;
      if(n_zlevels > 1)
      {
         cout << "Z = " << pz << endl << endl;
      }
      display_Zgrid_state(pz);
   } // loop over pz index
}

// ---------------------------------------------------------------------
void tictac3d::display_Zgrid_state(int pz)
{
   Color::Modifier green(Color::FG_GREEN);
   Color::Modifier purple(Color::FG_PURPLE);
   Color::Modifier yellow(Color::FG_YELLOW);
   Color::Modifier grey(Color::FG_GREY);
   Color::Modifier red(Color::FG_RED);
   Color::Modifier cyan(Color::FG_CYAN);
   Color::Modifier def(Color::FG_DEFAULT);
   Color::Modifier winning_color(Color::FG_CYAN);

   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         int curr_cell_value = get_cell_value(get_cell(px, py, pz));
         string cell_str;

         if(curr_cell_value == 0)
         {
            cell_str = "   ";
            cout << cell_str << flush;
         }
         else if(curr_cell_value == -1)
         {
            cell_str = " X ";
            if(winning_cell_posn(-1, px, py, pz))
            {
               cout << winning_color;
            }
            else if (latest_X_move == get_cell(px,py,pz))
            {
               cout << purple;
            }
            else
            {
               cout << red;
            }
            cout << cell_str << def << flush;
         }
         else if(curr_cell_value == 1)
         {
            cell_str = " O ";
            if(winning_cell_posn(1, px, py, pz))
            {
               cout << winning_color;
            }
            else if (latest_O_move == get_cell(px,py,pz))
            {
               cout << purple;
            }
            else
            {
               cout << yellow;
            }
            cout << cell_str << def << flush;
         }
         else if (curr_cell_value == 2)
         {
            cell_str = " OO";
            cout << purple << cell_str << def << flush;
         }
         else if (curr_cell_value == 3)
         {
            cell_str = " XO";
            cout << purple << cell_str << def << flush;
         }
         
         if(px < n_size - 1)
         {
            cout << grey << "|" << def << flush;
         }
      } // loop over px index
      cout << endl;

      if(py < n_size - 1)
      {
         for(int px = 0; px < n_size; px++)
         {
            cout << grey << "----" << def << flush;
         }
      }
      cout << endl;
   } // loop over py index
}

// ---------------------------------------------------------------------
bool tictac3d::winning_cell_posn(int player_ID, int px, int py, int pz)
{
   winning_posns_iter = winning_posns_map.find(get_cell(px,py,pz));
   if(winning_posns_iter == winning_posns_map.end()) return false;
   if(winning_posns_iter->second != player_ID) return false;
   return true;
}

// ---------------------------------------------------------------------
void tictac3d::print_winning_pattern()
{
//   cout << "inside print_winning_pattern()" << endl;
   
   int winner_ID = 0;
   for(winning_posns_iter = winning_posns_map.begin();
       winning_posns_iter != winning_posns_map.end();
       winning_posns_iter++)
   {
//      cout << " X = " << winning_posns_iter->first.first
//           << " Y = " << winning_posns_iter->first.second
//           << " Z = " << winning_posns_iter->first.third
//           << endl;
      winner_ID = winning_posns_iter->second;
//      cout << "winner_ID = " << winner_ID << endl;
   }

   string winner_symbol = " X ";
   if(winner_ID == 1)
   {
      winner_symbol = " O ";
   }

   display_board_state();

   cout << endl;
   cout << "*****************************************" << endl;
   cout << "Player" << winner_symbol << "wins!" << endl;
   cout << "*****************************************" << endl;
   cout << endl;
   cout << "GAME OVER" << endl;
}

// ---------------------------------------------------------------------
// Boolean member function check_player_win() returns a positive
// integer if input player_ID has a winning board state.

int tictac3d::check_player_win(int player_ID, bool print_flag)
{
   int win_in_zplane = 1;
   int win_in_zcolumn = 2;
   int win_in_zslant = 3;
   int win_in_corner_diag = 4;
   winning_posns_map.clear();

// Check if player wins via 4 corner-to-corner diagonals:
   if(corner_2_corner_win(player_ID))
   {
      if(print_flag) print_winning_pattern();
      return win_in_corner_diag;
   }

// Check if player wins via 16 Z-slants:

   for(int px = 0; px < n_size; px++)
   {
      if(Zslant_xconst_win(player_ID,px))
      {
         if(print_flag) print_winning_pattern();
         return win_in_zslant;
      }
   }

   for(int py = 0; py < n_size; py++)
   {
      if(Zslant_yconst_win(player_ID,py))
      {
         if(print_flag) print_winning_pattern();
         return win_in_zslant;
      }
   }

// Check if player wins via 16 Z-column:
   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         if(Zcolumn_win(player_ID, px, py))
         {
            if(print_flag) print_winning_pattern();
            return win_in_zcolumn;
         }
      }
   }

// Check if player wins within any of 40 Z-planes possibilities:

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      if(Zplane_win(player_ID, pz))
      {
         if(print_flag) print_winning_pattern();
         return win_in_zplane;
      }
   } // loop over pz

   return 0;
}

// ---------------------------------------------------------------------
// Member function check_opponent_win_on_next_move() examines every
// possible move that the opponent player could make on its next turn.
// If moving into some cell will result in the opponent winning, the
// cell's ID is returned by this method.  Otherwise, this method
// returns -1.

int tictac3d::check_opponent_win_on_next_turn(int player_value)
{
   for(int p = 0; p < n_cells; p++)
   {
      if(!legal_player_move(p)) continue;
      
      push_genuine_board_state();
      set_cell_value(p, -player_value);
      if(check_player_win(-player_value) > 0)
      {
//         cout << "*** OPPONENT CAN WIN ON NEXT TURN! ***" << endl;
//         triple curr_t = cell_decomposition[p];
//         cout << " Z = " << curr_t.third
//              << " col = " << curr_t.first
//              << " row = " << curr_t.second
//              << endl;
         pop_genuine_board_state();
         return p;
      }
      pop_genuine_board_state();
   }
   return -1;
}

// ---------------------------------------------------------------------
// Member function correlate_cells_with_winnable_paths() fills STL map
// cell_winnable_paths_map with key = cell ID and value =
// vector<winnable path IDs> .  

void tictac3d::correlate_cells_with_winnable_paths()
{
   for(unsigned int w = 0; w < winnable_paths.size(); w++)
   {
      winnable_path_t curr_winnable_path = winnable_paths[w];
      for(unsigned int i = 0; i < curr_winnable_path.path.size(); i++)
      {
         int cell_ID = curr_winnable_path.path[i];
         cell_winnable_paths_iter = cell_winnable_paths_map.find(cell_ID);
         if(cell_winnable_paths_iter == cell_winnable_paths_map.end())
         {
            vector<int> V;
            V.push_back(w);
            cell_winnable_paths_map[cell_ID] = V;
         }
         else
         {
            cell_winnable_paths_iter->second.push_back(w);
         }
      } // loop over index i labeling cells in curr_winnable_path
   } // loop over index w labeling winnable paths

   min_intrinsic_cell_prize = 10000;
   max_intrinsic_cell_prize = -min_intrinsic_cell_prize;
   for(int cell_ID = 0; cell_ID < n_cells; cell_ID++)
   {
      cell_winnable_paths_iter = cell_winnable_paths_map.find(cell_ID);

      intrinsic_cell_prize.push_back(
         cell_winnable_paths_iter->second.size());

      min_intrinsic_cell_prize = 
         basic_math::min(min_intrinsic_cell_prize,
                         intrinsic_cell_prize.back());
      max_intrinsic_cell_prize = 
         basic_math::max(max_intrinsic_cell_prize,
                         intrinsic_cell_prize.back());
   }
   cout << "min_intrinsic_cell_prize = " << min_intrinsic_cell_prize
        << " max_intrinsic_cell_prize = " << max_intrinsic_cell_prize
        << endl;
}

// ---------------------------------------------------------------------
void tictac3d::adjust_intrinsic_cell_prizes()
{
   for(int cell_ID = 0; cell_ID < n_cells; cell_ID++)
   {
      int curr_cell_prize = intrinsic_cell_prize[cell_ID];
      int new_cell_prize = basic_math::min(max_intrinsic_cell_prize,
                                           curr_cell_prize + 1);
      intrinsic_cell_prize[cell_ID] = new_cell_prize;
//      cout << "cell_ID = " << cell_ID
//           << " new_cell_prize = " << intrinsic_cell_prize[cell_ID]
//           << endl;
   }
}

// ---------------------------------------------------------------------
void tictac3d::print_winnable_path(int path_ID)
{
   cout << "Winnable path ID = " << path_ID << endl;
   winnable_path_t curr_winnable_path = winnable_paths[path_ID];
   for(unsigned int i = 0; i < curr_winnable_path.path.size(); i++)
   {
      int cell_ID = curr_winnable_path.path[i];
      triple curr_triple = cell_decomposition[cell_ID];
      cout << "i = " << i << " Z = " << curr_triple.third
           << " col = " << curr_triple.first 
           << " row = " << curr_triple.second 
           << endl;
   }
}

// ---------------------------------------------------------------------
vector<int>* tictac3d::get_winnable_path_IDs(int cell_ID)
{
   cell_winnable_paths_iter = cell_winnable_paths_map.find(cell_ID);
   return &cell_winnable_paths_iter->second;
}

// ---------------------------------------------------------------------
void tictac3d::print_cell_ID_vs_winnable_path_IDs()
{
   for(int cell_ID = 0; cell_ID < n_cells; cell_ID++)
   {
      cout << "cell_ID = " << cell_ID << endl;
      vector<int>* winnable_path_IDs = get_winnable_path_IDs(cell_ID);
      for (unsigned int j = 0; j < winnable_path_IDs->size(); j++)
      {
         cout << "   winnable path ID = " << winnable_path_IDs->at(j) << endl;
      }
   }

   for(int cell_ID = 0; cell_ID < n_cells; cell_ID++)
   {
      cout << "cell_ID = " << cell_ID 
           << " prize = " << intrinsic_cell_prize[cell_ID]
           << endl;
   }
}

// ---------------------------------------------------------------------
// Member function generate_all_winnable_paths()

void tictac3d::generate_all_winnable_paths()
{
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      generate_winnable_Zplane_paths(pz);
   }

   if(n_zlevels >= n_size)
   {

      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
         {
            generate_winnable_Zcolumn_path(px,py);
         }
      }
   
      for(int px = 0; px < n_size; px++)
      {
         generate_Zslant_xconst_winnable_paths(px);
      }
      
      for(int py = 0; py < n_size; py++)
      {
         generate_Zslant_yconst_winnable_paths(py);
      }
      
      generate_corner_2_corner_winnable_paths();
   }
   
   n_winnable_paths = winnable_paths.size();
   cout << "Total number of winnable paths = " << n_winnable_paths << endl;
}

// ---------------------------------------------------------------------
// Member function generate_winnable_Zplane_paths()

void tictac3d::generate_winnable_Zplane_paths(int pz)
{
   winnable_path_t curr_path;

// Firstly add n_size horizontal rows within Z-plane:

   for(int py = 0; py < n_size; py++)
   {
      curr_path.path.clear();
      for(int px = 0; px < n_size; px++)
      {
         curr_path.path.push_back(get_cell(px,py,pz));
      }
      winnable_paths.push_back(curr_path);
   }

// Secondly add n_size vertical columns within Z-plane:

   for(int px = 0; px < n_size; px++)
   {
      curr_path.path.clear();
      for(int py = 0; py < n_size; py++)
      {
         curr_path.path.push_back(get_cell(px,py,pz));
      }
      winnable_paths.push_back(curr_path);
   }

// Thirdly add 2 diagonals within Z-plane:

   curr_path.path.clear();
   for(int px = 0; px < n_size; px++)
   {
      int py = px;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.path.clear();
   for(int px = 0; px < n_size; px++)
   {
      int py = n_size - 1 - px;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
}

// ---------------------------------------------------------------------
// Boolean member function Zplane_win() returns true if input
// player_ID occupies 4 continguous cells within some row, column or
// diagonal of a Z-plane.

bool tictac3d::Zplane_win(int player_ID, int pz)
{
   bool horiz_row_win = false, vert_column_win = false, diagonal_win = false;

// Firstly check 4 horizontal rows within Z-plane:

   for(int py = 0; py < n_size; py++)
   {
      horiz_row_win = true;
      winning_posns_map.clear();
      for(int px = 0; px < n_size && horiz_row_win; px++)
      {
         int p = get_cell(px,py,pz);
         winning_posns_map[p] = player_ID;
         if(get_cell_value(p) != player_ID)
         {
            horiz_row_win = false;
         }
      }
      if(horiz_row_win) return true;
   }

// Secondly check 4 vertical columns within Z-plane:

   for(int px = 0; px < n_size; px++)
   {
      winning_posns_map.clear();
      vert_column_win = true;
      for(int py = 0; py < n_size && vert_column_win; py++)
      {
         int p = get_cell(px,py,pz);
         winning_posns_map[p] = player_ID;
         if(get_cell_value(p) != player_ID)
         {
            vert_column_win = false;
         }
      }
      if(vert_column_win) return true;   
   }

// Thirdly check 2 diagonals within Z-plane:

   diagonal_win = true;
   winning_posns_map.clear();
   for(int px = 0; px < n_size && diagonal_win; px++)
   {
      int py = px;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         diagonal_win = false;
      }
   }
   if(diagonal_win) return true;   

   diagonal_win = true;
   winning_posns_map.clear();
   for(int px = 0; px < n_size && diagonal_win; px++)
   {
      int py = n_size - 1 - px;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         diagonal_win = false;
      }
   }
   if(diagonal_win) return true;   
   
   winning_posns_map.clear();
   return false;
}

// ---------------------------------------------------------------------
// Boolean member function generate_winnable_Zcolumn_path()

void tictac3d::generate_winnable_Zcolumn_path(int px, int py)
{
   winnable_path_t curr_path;
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
}

// ---------------------------------------------------------------------
// Boolean member function Zcolumn_win() returns true if input
// player_ID occupies 4 continguous cells with Z = 0, 1, ... n_size - 1:

bool tictac3d::Zcolumn_win(int player_ID, int px, int py)
{
   if(n_zlevels < n_size) return false;
   bool column_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         column_win = false;
      }
   }

   if(column_win) return true;

   winning_posns_map.clear();
   return false;
}

// ---------------------------------------------------------------------
// Boolean member function generate_Zslant_x[y]_winnable_paths()

void tictac3d::generate_Zslant_xconst_winnable_paths(int px)
{
   winnable_path_t curr_path;

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int py = pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int py = n_zlevels - 1 - pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
}

// ---------------------------------------------------------------------
void tictac3d::generate_Zslant_yconst_winnable_paths(int py)
{
   winnable_path_t curr_path;

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
}

// ---------------------------------------------------------------------
// Boolean member function Zslant_x[y]const_win() returns true if input
// player_ID occupies 4 continguous cells with Z = 0, 1, ... n_size - 1:

bool tictac3d::Zslant_xconst_win(int player_ID, int px)
{
   if(n_zlevels < n_size) return false;
   bool slant_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int py = pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         slant_win = false;
      }
   }
   if(slant_win) return slant_win;

   slant_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int py = n_zlevels - 1 - pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         slant_win = false;
      }
   }
   if(slant_win) return slant_win;

   winning_posns_map.clear();
   return false;
}

// ---------------------------------------------------------------------
bool tictac3d::Zslant_yconst_win(int player_ID, int py)
{
   if(n_zlevels < n_size) return false;
   bool slant_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         slant_win = false;
      }
   }
   if(slant_win) return slant_win;

   slant_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         slant_win = false;
      }
   }
   if(slant_win) return slant_win;

   winning_posns_map.clear();
   return false;
}

// ---------------------------------------------------------------------
// Member function generate_corner_2_corner_winnable_paths() 

void tictac3d::generate_corner_2_corner_winnable_paths()
{
   winnable_path_t curr_path;

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
   
   curr_path.path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = n_zlevels - 1 - pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = n_zlevels - 1 - pz;
      curr_path.path.push_back(get_cell(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
}

// ---------------------------------------------------------------------
// Boolean member function corner_2_corner_win() returns true if input
// player_ID occupies 4 continguous cells from one 3D corner to another:

bool tictac3d::corner_2_corner_win(int player_ID)
{
   if(n_zlevels < n_size) return false;
   
   bool corner_diag_win = true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   corner_diag_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   corner_diag_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = n_zlevels - 1 - pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   corner_diag_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = n_zlevels - 1 - pz;
      int p = get_cell(px,py,pz);
      winning_posns_map[p] = player_ID;
      if(get_cell_value(p) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   return false;
}

// ---------------------------------------------------------------------
// Member function imminent_win_or_loss()

int tictac3d::imminent_win_or_loss(int player_value)
{
   int winning_player_move = -1;
   int winning_opponent_move = -1;
   for(int curr_node = 0; curr_node < n_cells; curr_node++)
   {
      if(!legal_player_move(curr_node)) continue;

      int orig_cell_value = get_cell_value(curr_node);

      set_cell_value(curr_node, -player_value);
      if(check_player_win(-player_value) > 0)
      {
         winning_opponent_move = curr_node;
      }
      
      set_cell_value(curr_node, player_value);
      if(check_player_win(player_value) > 0)
      {
         winning_player_move = curr_node;
      }
      set_cell_value(curr_node, orig_cell_value);
   }
   if(winning_player_move >= 0)
   {
//      cout << "Winning player move = " << winning_player_move << endl;
      return winning_player_move;
   }
   else if (winning_opponent_move >= 0)
   {
//      cout << "Winning opponent move = " << winning_opponent_move << endl;
      return winning_opponent_move;
   }
   else
   {
      return -1;
   }
}

// =====================================================================
// Minimax member functions
// =====================================================================

// Member function get_recursive_minimax_move()

int tictac3d::get_recursive_minimax_move(int player_value)
{
//   cout << "inside get_recursive_minimax_move, player_value = " 
//        << player_value << " recursive_depth = " << recursive_depth << endl;

//   timefunc::initialize_timeofday_clock();   
   bool maximizing_player = false;
   double best_value;
   int best_move = -1;
   n_score_evaluations = 0;

   if(recursive_depth%2 == 0)
   {
      maximizing_player = true;
      best_value = NEGATIVEINFINITY;
   }
   else
   {
      maximizing_player = false;
      best_value = POSITIVEINFINITY;
   }

   int curr_prime = 3;
   if(n_size > 3)
   {
      int random = 10 * nrfunc::ran1();
      if(random == 1)
      {
         curr_prime = 11;
      }
      else if (random == 2)
      {
         curr_prime = 13;
      }
      else if (random == 3)
      {
         curr_prime = 17;
      }
      else if (random == 4)
      {
         curr_prime = 19;
      }
      else if (random == 5)
      {
         curr_prime = 19;
      }
      else if (random == 6)
      {
         curr_prime = 23;
      }
      else if (random == 7)
      {
         curr_prime = 29;
      }
      else if (random == 8)
      {
         curr_prime = 31;
      }
      else if (random == 9)
      {
         curr_prime = 37;
      }
      else if (random == 9)
      {
         curr_prime = 41;
      }
   }
   
   for(int p = 0; p < n_cells; p++)
   {
      int curr_node = ((p+1) * curr_prime) % n_cells;
//      int curr_node = p;
      if(!legal_player_move(curr_node)) continue;

//      double curr_value = get_minimax_move_score(
//         curr_node, recursive_depth, player_value, maximizing_player);

      double min_value = NEGATIVEINFINITY;
      double max_value = POSITIVEINFINITY;
      double curr_value = get_alphabeta_minimax_move_score(
         curr_node, recursive_depth, min_value, max_value, player_value, 
         maximizing_player);

      if(maximizing_player)
      {
         if(curr_value > best_value)
         {
            best_value = curr_value;
            best_move = curr_node;
         }
      }
      else
      {
         if(curr_value < best_value)
         {
            best_value = curr_value;
            best_move = curr_node;
         }
      } // maximizing player conditional

   } // loop over index p labeling cells
//   double elapsed_secs = timefunc::elapsed_timeofday_time();   
//   cout << "n_score_evaluations = " << n_score_evaluations << endl;
//   cout << "Recursive computation time = " << elapsed_secs << " secs" << endl;

   return best_move;
}

// ---------------------------------------------------------------------
// Member function get_minimax_move_score()

double tictac3d::get_minimax_move_score(
   int curr_node, int depth, int player_value, bool maximizing_flag)
{
   push_genuine_board_state();
   set_cell_value(curr_node, player_value);

   if(depth == 0 || get_n_empty_cells() == 0)
   {
      double integrated_player_path_score, integrated_opponent_path_score;
      extremal_winnable_path_scores(
         player_value, integrated_player_path_score, 
         integrated_opponent_path_score);
      pop_genuine_board_state();

      if(maximizing_flag)
      {
         return integrated_player_path_score;
      }
      else
      {
         return integrated_opponent_path_score;
      }
   }

   if(maximizing_flag)  // maximizing player
   {
      double best_value = NEGATIVEINFINITY;
      for(int next_node = 0; next_node < n_cells; next_node++)
      {
         if(!legal_player_move(next_node)) continue;
         double curr_value = get_minimax_move_score(
            next_node, depth - 1, -player_value, maximizing_flag);
         best_value = basic_math::max(best_value, curr_value);
      } // loop over next_node
      pop_genuine_board_state();
      return best_value;
   }
   else // !maximizing_player
   {
      double best_value = POSITIVEINFINITY;
      for(int next_node = 0; next_node < n_cells; next_node++)
      {
         if(!legal_player_move(next_node)) continue;

         double curr_value = get_minimax_move_score(
            next_node, depth - 1, -player_value, !maximizing_flag);
         best_value = basic_math::min(best_value, curr_value);
      } // loop over next_node
      pop_genuine_board_state();
      return best_value;
   }
}

// ---------------------------------------------------------------------
// Member function get_alphabeta_minimax_move_score().  See
// https://www.cs.cornell.edu/courses/cs312/2002sp/lectures/rec21.htm
// On 11/2/16, we verified that alpha-beta pruning can reduce the
// number of path scoring evaluations performed by factors of 2 - 3 !

double tictac3d::get_alphabeta_minimax_move_score(
   int curr_node, int depth, double min_value, double max_value, 
   int player_value, bool maximizing_flag)
{
   push_genuine_board_state();
   set_cell_value(curr_node, player_value);

   if(depth == 0 || get_n_empty_cells() == 0)
   {
      double integrated_player_path_score, integrated_opponent_path_score;
      extremal_winnable_path_scores(
         player_value, integrated_player_path_score, 
         integrated_opponent_path_score);
      pop_genuine_board_state();

      if(maximizing_flag)
      {
         return integrated_player_path_score;
      }
      else
      {
         return integrated_opponent_path_score;
      }
   }

   if(maximizing_flag)  // maximizing player
   {
      double best_value = min_value;

// Visit nodes in descending order of move score:

      for(int next_node = 0; next_node < n_cells; next_node++)
      {
         if(!legal_player_move(next_node)) continue;
         double curr_value = get_alphabeta_minimax_move_score(
            next_node, depth - 1, best_value, max_value, -player_value, 
            maximizing_flag);

         best_value = basic_math::max(best_value, curr_value);
         if(best_value > max_value)
         {
            best_value = max_value;
            break;
         }
      } // loop over next_node
      pop_genuine_board_state();

/*
      cout << "curr_node = " << curr_node
           << " best_value = " << best_value << endl;
      triple curr_triple = cell_decomposition[curr_node];
      cout << " Z = " << curr_triple.third
           << " col = " << curr_triple.first 
           << " row = " << curr_triple.second 
           << endl;
*/

      return best_value;
   }
   else // !maximizing_player
   {
      double best_value = max_value;

// Visit nodes in ascending order of move score:

      for(int next_node = 0; next_node < n_cells; next_node++)
      {
         if(!legal_player_move(next_node)) continue;
         double curr_value = get_alphabeta_minimax_move_score(
            next_node, depth - 1, min_value, best_value, -player_value, 
            !maximizing_flag);

         best_value = basic_math::min(best_value, curr_value);
         if(best_value < min_value)
         {
            best_value = min_value;
            break;
         }
      } // loop over next_node
      pop_genuine_board_state();
      return best_value;
   }
}

// ---------------------------------------------------------------------
// Member function extremal_winnable_path_scores() counts the number
// of player_value pieces within each winnable path.  If both players
// have pieces in a path, the path is ignored.  

void tictac3d::extremal_winnable_path_scores(
   int player_value, double& integrated_player_path_score, 
   double& integrated_opponent_path_score)
{
   integrated_player_path_score = integrated_opponent_path_score = 0;
   for(int p = 0; p < n_winnable_paths; p++)
   {
      int n_player_pieces_in_path = 0;
      int n_opponent_pieces_in_path = 0;
      int n_player_path_prize = 0;
      int n_opponent_path_prize = 0;

      for(int i = 0; i < n_size; i++)
      {
         int curr_cell = winnable_paths[p].path[i];
         int curr_cell_value = get_cell_value(curr_cell);
         if(curr_cell_value == 0) continue;
         
         if(curr_cell_value == -player_value)
         {
            n_opponent_pieces_in_path++;
            n_opponent_path_prize += intrinsic_cell_prize[curr_cell];
         }
         else if (curr_cell_value == player_value)
         {
            n_player_pieces_in_path++;
            n_player_path_prize += intrinsic_cell_prize[curr_cell];
         }

         if(n_opponent_pieces_in_path > 0 && n_player_pieces_in_path > 0)
         {
            break;
         }
      } // loop over index i 

      double player_path_score = 0;
      double opponent_path_score = 0;
      if(n_opponent_pieces_in_path == 0)
      {
//         player_path_score += n_player_pieces_in_path * n_player_path_prize;
         player_path_score += sqr(n_player_pieces_in_path) 
            * n_player_path_prize;
      }

      if(n_player_pieces_in_path == 0)
      {
//         opponent_path_score += n_opponent_pieces_in_path * 
//            n_opponent_path_prize;
         opponent_path_score += sqr(n_opponent_pieces_in_path) * 
            n_opponent_path_prize;
      }

      integrated_player_path_score += player_path_score;
      integrated_opponent_path_score += opponent_path_score;

   } // loop over index p labeling all winnable paths

   n_score_evaluations++;

//   cout << "integrated_player_path_score = " << integrated_player_path_score
//        << " integrated_opponent_path_score = " << integrated_opponent_path_score << endl;
}

// =====================================================================
// Results display member functions
// =====================================================================

// Generate metafile plot of game_illegal_frac, game_loss_frac,
// game_stalemate_frac and game_win_frac versus episodes

void tictac3d::plot_game_frac_histories(
   string output_subdir, int n_episodes, string extrainfo)
{
   metafile curr_metafile;
   string meta_filename=output_subdir+"game_performance";
   string title="Game performance";
   string subtitle = extrainfo;
   string x_label="Episode number";
   string y_label="Game win/loss fractions";

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, n_episodes, 0, 1);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.set_legend_flag(true);
   curr_metafile.openmetafile();
   curr_metafile.write_header();

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Illegal");
   curr_metafile.write_curve(
      0, n_episodes, game_illegal_frac, colorfunc::purple);

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Loss");
   curr_metafile.write_curve(0, n_episodes, game_loss_frac, colorfunc::red);

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Stalemate");
   curr_metafile.write_curve(
      0, n_episodes, game_stalemate_frac, colorfunc::cyan);

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Win");
   curr_metafile.write_curve(0, n_episodes, game_win_frac, colorfunc::green);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Member function compute_all_afterstates() loops over all currently
// vacant cells within the current board.  It sets a genvector*
// corresponding to each possible afterstate for the current board
// state into member vector afterstate_ptrs. 

vector<genvector*>& tictac3d::compute_all_afterstates(int player_value)
{
   int n_afterstates = 0;
   for(int p = 0; p < n_cells; p++)
   {
      if(legal_player_move(p))
      {
         set_cell_value(p, player_value);
         for(int c = 0; c < n_cells; c++)
         {
            afterstate_ptrs[n_afterstates]->put(c, curr_board_state[c]);
         }
         n_afterstates++;
         set_cell_value(p, 0);  // Reset board state back to initial condition
      } 
      else
      {
// Fill afterstate genvector with dummy -999 values for illegal
// moves:

         for(int c = 0; c < n_cells; c++)
         {
            afterstate_ptrs[n_afterstates]->put(c, -999);
         }
      } // legal player move conditional
   } // loop over index p labeling cells

   return afterstate_ptrs;
}
