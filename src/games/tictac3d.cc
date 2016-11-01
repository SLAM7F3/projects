// ==========================================================================
// tictac3d class member function definitions
// ==========================================================================
// Last modified on 10/29/16; 10/30/16; 10/31/16; 11/1/16
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
   board_state_ptr = new genvector(n_cells);
   inverse_board_state_ptr = new genvector(n_cells);
}		       

void tictac3d::initialize_member_objects()
{
   game_over = false;
   generate_all_winnable_paths();
   board_state_ptr->clear_values();

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
   delete board_state_ptr;
   delete inverse_board_state_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const tictac3d& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

genvector* tictac3d::get_board_state_ptr()
{
   for(unsigned int p = 0; p < curr_board_state.size(); p++)
   {
      board_state_ptr->put(p, curr_board_state.at(p));
   }
   return board_state_ptr;
}

// ---------------------------------------------------------------------
// Member function get_inverse_board_state_ptr() returns the board
// state vector after swapping all "X" and "O" pieces.

genvector* tictac3d::get_inverse_board_state_ptr()
{
   for(unsigned int p = 0; p < curr_board_state.size(); p++)
   {
      inverse_board_state_ptr->put(p, -curr_board_state.at(p));
   }
   return inverse_board_state_ptr;
}

// ---------------------------------------------------------------------
// Member function push_genuine_board_state() creates a copy of
// curr_board_state before we might start to intentionally alter the
// contents of curr_board_state for path planning.

void tictac3d::push_genuine_board_state()
{
   vector<int> current_board_state;
   for(unsigned int p = 0; p < curr_board_state.size(); p++)
   {
      current_board_state.push_back(curr_board_state[p]);
   }
   genuine_board_state.push_back(current_board_state);

}

void tictac3d::pop_genuine_board_state()
{
   curr_board_state.clear();
   for(unsigned int p = 0; p < genuine_board_state.back().size(); p++)
   {
      curr_board_state.push_back(genuine_board_state.back()[p]);
   }
   genuine_board_state.pop_back();
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
int tictac3d::get_cell_value(triple& t) const
{
   return get_cell_value(t.first, t.second, t.third);
}

int tictac3d::get_cell_value(int px, int py, int pz) const
{
   int p = n_size * n_size * pz + n_size * py + px;   
   return get_cell_value(p);
}

int tictac3d::get_cell_value(int p) const
{
   return curr_board_state.at(p);
}

// ---------------------------------------------------------------------
// Member function decompose_cell_index() returns triple (px,py,pz)
// corresponding to cell index p.

triple tictac3d::decompose_cell_index(int p)
{
   return cell_decomposition[p];
}

// ---------------------------------------------------------------------
// Boolean member function set_cell_value() returns false if specified
// cell is already occupied

bool tictac3d::set_cell_value(triple& t, int value)
{
   return set_cell_value(t.first, t.second, t.third, value);
}

bool tictac3d::set_cell_value(int px, int py, int pz, int value)
{
   int p = n_size * n_size * pz + n_size * py + px;   
   return set_cell_value(p, value);
}

bool tictac3d::set_cell_value(int p, int value)
{
   bool cell_unoccupied = true;
   if(get_cell_value(p) != 0)
   {
      cell_unoccupied = false;
   }

   curr_board_state[p] = value;
   return cell_unoccupied;
}

// ---------------------------------------------------------------------
// Member function record_latest_move()

void tictac3d::record_latest_move(int player_value, triple t)
{
   record_latest_move(player_value, t.first, t.second, t.third);
}

void tictac3d::record_latest_move(int player_value, int px, int py, int pz)
{
   int p = n_size * n_size * pz + n_size * py + px;   
   record_latest_move(player_value, p);
}

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
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
         {
            if(get_cell_value(px,py,pz) != 0)
            {
               n_filled_cells++;
            }
         } // loop over px 
      } // loop over py
   } // loop over pz

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

      legal_move = legal_player_move(px, py, pz);
      if(!legal_move)
      {
         cout << "Cell is already occupied.  Please try again" << endl;
      }
   } // while !legal_move loop
   set_cell_value(px, py, pz, player_value);
   record_latest_move(player_value, px, py, pz);
}

// ---------------------------------------------------------------------
double tictac3d::get_random_player_move(int player_value)
{
   int p = mathfunc::getRandomInteger(n_cells);
   return set_player_move(p, player_value);
}

// ---------------------------------------------------------------------
// Boolean member function legal_player_move() returns false if
// cell corresponding to input (px,py,pz) is already occupied.

bool tictac3d::legal_player_move(int px, int py, int pz, bool print_flag)
{

// Cell values:

// -1   --> AI
// 0    --> empty
// 1    --> agent
// 2    --> 2 agent (illegal)
// 3    --> agent + AI (illegal)

   int curr_cell_value = get_cell_value(px, py, pz);
   if(curr_cell_value == -1)
   {
      if(print_flag)
      {
         cout << "Agent attempted illegal move into row = " << py 
              << " column = " << px << endl;
         cout << "Cell is already occupied by X" << endl;
      }
      return false;
   }
   else if (curr_cell_value == 1)
   {
      if(print_flag)
      {
         cout << "Agent attempted illegal move into row = " << py 
              << " column = " << px << endl;
         cout << "Cell is already occupied by O" << endl;
      }
      return false;
   }
   else
   {
      return true;
   }
}

bool tictac3d::legal_player_move(int p)
{
   return (curr_board_state.at(p) == 0);
}

// ---------------------------------------------------------------------
bool tictac3d::set_player_move(triple t, int player_value)
{
   return set_player_move(t.first, t.second, t.third, player_value);
}

bool tictac3d::set_player_move(int px, int py, int pz, int player_value)
{

// Cell values:

// -1   --> AI
// 0    --> empty
// 1    --> agent
// 2    --> 2 agent (illegal)
// 3    --> agent + AI (illegal)

   bool legal_move = true;
   if(!legal_player_move(px, py, pz))
   {
      legal_move = false;
      if(get_cell_value(px,py,pz) == -1)
      {
         player_value = 3;
      }
      else if(get_cell_value(px,py,pz) == 1)
      {
         player_value = 2;
      }
   }
   set_cell_value(px, py, pz, player_value);
   return legal_move;
}

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
   bool legal_move_flag = true;

   do
   {
      int px = mathfunc::getRandomInteger(n_size);
      int py = mathfunc::getRandomInteger(n_size);
      int pz = mathfunc::getRandomInteger(n_zlevels);
      if(get_cell_value(px,py,pz) == 0)
      {
         set_cell_value(px,py,pz,player_value);
         legal_move_flag = true;
      }
      else
      {
         legal_move_flag = false;
      }
   }
   while(!legal_move_flag);
}

// ---------------------------------------------------------------------
void tictac3d::reset_board_state()
{
   n_AI_turns = n_agent_turns = 0;
   curr_board_state.clear();
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
         {
            int curr_val = 0;
            curr_board_state.push_back(curr_val);
         } // loop over px 
      } // loop over py
   } // loop over pz
   game_over = false;
}		       

// ---------------------------------------------------------------------
void tictac3d::randomize_board_state()
{
   curr_board_state.clear();
   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
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
            
            curr_board_state.push_back(curr_val);
         } // loop over px 
      } // loop over py
   } // loop over pz
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
            int p = n_size * n_size * pz + n_size * py + px;
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
   sysfunc::clearscreen();
   cout << "......................................................." << endl;

// First retrieve cells containing latest X and O moves:

   latest_O = triple(-1,-1,-1);
   latest_move_iter = latest_move_map.find(1);
   if(latest_move_iter != latest_move_map.end())
   {
      int latest_O_move = latest_move_iter->second;
//      cout << "latest_O_move = " << latest_O_move << endl;
      latest_O = decompose_cell_index(latest_O_move);
//      cout << "latest_O: px = " << latest_O.first
//           << " py = " << latest_O.second
//           << " pz = " << latest_O.third << endl;
   }

   latest_X = triple(-1,-1,-1);
   latest_move_iter = latest_move_map.find(-1);
   if(latest_move_iter != latest_move_map.end())
   {
      int latest_X_move = latest_move_iter->second;
//      cout << "latest_X_move = " << latest_X_move << endl;
      latest_X = decompose_cell_index(latest_X_move);
//      cout << "latest_X: px = " << latest_X.first
//           << " py = " << latest_X.second
//           << " pz = " << latest_X.third << endl;
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
         int curr_cell_value = get_cell_value(px, py, pz);
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
            else if (px == latest_X.first && py == latest_X.second &&
                     pz == latest_X.third)
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
            else if (px == latest_O.first && py == latest_O.second &&
                     pz == latest_O.third)
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
   winning_posns_iter = winning_posns_map.find(triple(px,py,pz));
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
// Boolean member function check_player_win() returns true if input
// player_ID has a winning board state.

int tictac3d::check_player_win(int player_ID, bool print_flag)
{
//   cout << "inside check_player_win(), player_ID = " << player_ID << endl;

   int win_in_zplane = 1;
   int win_in_zcolumn = 2;
   int win_in_zslant = 3;
   int win_in_corner_diag = 4;
   winning_posns_map.clear();

// Check if player wins via 4 corner-to-corner diagonals:
   if(corner_2_corner_win(player_ID))
   {
      if(print_flag) print_winning_pattern();
      game_over = true;
      return win_in_corner_diag;
   }

// Check if player wins via 16 Z-slants:

   for(int px = 0; px < n_size; px++)
   {
      if(Zslant_xconst_win(player_ID,px))
      {
         if(print_flag) print_winning_pattern();
         game_over = true;
         return win_in_zslant;
      }
   }

   for(int py = 0; py < n_size; py++)
   {
      if(Zslant_yconst_win(player_ID,py))
      {
         if(print_flag) print_winning_pattern();
         game_over = true;
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
            game_over = true;
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
         game_over = true;
         return win_in_zplane;
      }
   } // loop over pz

   return 0;
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
   
   cout << "Total number of winnable paths = "
        << winnable_paths.size() << endl;
}

// ---------------------------------------------------------------------
// Member function generate_winnable_Zplane_paths()

void tictac3d::generate_winnable_Zplane_paths(int pz)
{
   vector<triple> curr_path;
   
// Firstly add 4 horizontal rows within Z-plane:

   for(int py = 0; py < n_size; py++)
   {
      curr_path.clear();
      for(int px = 0; px < n_size; px++)
      {
         curr_path.push_back(triple(px,py,pz));
      }
      winnable_paths.push_back(curr_path);
   }

// Secondly add 4 vertical columns within Z-plane:

   for(int px = 0; px < n_size; px++)
   {
      curr_path.clear();
      for(int py = 0; py < n_size; py++)
      {
         curr_path.push_back(triple(px,py,pz));
      }
      winnable_paths.push_back(curr_path);
   }

// Thirdly add 2 diagonals within Z-plane:

   curr_path.clear();
   for(int px = 0; px < n_size; px++)
   {
      int py = px;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.clear();
   for(int px = 0; px < n_size; px++)
   {
      int py = n_size - 1 - px;
      curr_path.push_back(triple(px,py,pz));
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
         winning_posns_map[triple(px,py,pz)] = player_ID;
         if(get_cell_value(px,py,pz) != player_ID)
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
         winning_posns_map[triple(px,py,pz)] = player_ID;
         if(get_cell_value(px,py,pz) != player_ID)
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
   vector<triple> curr_path;
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      curr_path.push_back(triple(px,py,pz));
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
   vector<triple> curr_path;

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int py = pz;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int py = n_zlevels - 1 - pz;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
}

// ---------------------------------------------------------------------
void tictac3d::generate_Zslant_yconst_winnable_paths(int py)
{
   vector<triple> curr_path;

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      curr_path.push_back(triple(px,py,pz));
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
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
   vector<triple> curr_path;

   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = pz;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);
   
   curr_path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = pz;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = n_zlevels - 1 - pz;
      curr_path.push_back(triple(px,py,pz));
   }
   winnable_paths.push_back(curr_path);

   curr_path.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = n_zlevels - 1 - pz;
      curr_path.push_back(triple(px,py,pz));
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
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = pz;
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = pz;
      int py = n_zlevels - 1 - pz;
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_zlevels; pz++)
   {
      int px = n_zlevels - 1 - pz;
      int py = n_zlevels - 1 - pz;
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   return false;
}

// ---------------------------------------------------------------------
// Member function get_recursive_minimax_move_score()

int tictac3d::get_recursive_minimax_move(int player_value, int depth)
{
   cout << "inside get_recursive_minimax_move, player_value = " 
        << player_value << " depth = " << depth << endl;

   timefunc::initialize_timeofday_clock();   
   bool maximizing_player = false;
   double best_value;
   int best_move = -1;

   if(depth%2 == 0)
   {
      maximizing_player = true;
      best_value = NEGATIVEINFINITY;
   }
   else
   {
      maximizing_player = false;
      best_value = POSITIVEINFINITY;
   }
   
   for(int curr_node = 0; curr_node < n_cells; curr_node++)
   {
      if(!legal_player_move(curr_node)) continue;

      double curr_value = get_minimax_move_score(
         curr_node, depth, player_value);

      if(maximizing_player)
      {
         if(best_value < curr_value)
         {
            best_value = curr_value;
            best_move = curr_node;
         }
      }
      else
      {
         if(best_value > curr_value)
         {
            best_value = curr_value;
            best_move = curr_node;
         }
      } // maximizing player conditional
   } // loop over curr_node
   double elapsed_secs = timefunc::elapsed_timeofday_time();   
   cout << "Recursive computation time = " << elapsed_secs << " secs" << endl;

   return best_move;
}

// ---------------------------------------------------------------------
// Member function get_minimax_move_score()

double tictac3d::get_minimax_move_score(
   int curr_node, int depth, int player_value)
{
   push_genuine_board_state();
   set_cell_value(curr_node, player_value);

   double best_value;
   if(depth == 0 || get_n_empty_cells() == 0)
   {
      best_value = best_winnable_path(player_value);      
      pop_genuine_board_state();
      return best_value;
   }

   bool maximizing_player = false;
   if(depth%2 == 0) maximizing_player = true;
   
   if(maximizing_player)
   {
      best_value = NEGATIVEINFINITY;
      for(int next_node = 0; next_node < n_cells; next_node++)
      {
         if(!legal_player_move(next_node)) continue;
         double curr_value = get_minimax_move_score(
            next_node, depth - 1, -player_value);
         best_value = basic_math::max(best_value, curr_value);
      } // loop over next_node
      pop_genuine_board_state();
   }
   else if (!maximizing_player)
   {
      best_value = POSITIVEINFINITY;
      for(int next_node = 0; next_node < n_cells; next_node++)
      {
         if(!legal_player_move(next_node)) continue;
         double curr_value = get_minimax_move_score(
            next_node, depth - 1, -player_value);
         best_value = basic_math::min(best_value, curr_value);
      } // loop over next_node
      pop_genuine_board_state();
   }
   return best_value;
}

// ---------------------------------------------------------------------
// Member function best_winnable_path() returns the best score among
// all winnable paths for the specified input player given the
// current board state.

double tictac3d::best_winnable_path(int player_value)
{
//   cout << "inside best_winnable_path()" << endl;
   int n_winnable_paths = winnable_paths.size();
   compute_winnable_path_occupancies(player_value);

//   int best_path_ID = 0;
   double best_path_score = NEGATIVEINFINITY;

   for(path_occupancy_iter = path_occupancy_map.begin();
       path_occupancy_iter != path_occupancy_map.end(); path_occupancy_iter++)
   {
      double curr_path_score = 0;
      int curr_occupancy = path_occupancy_iter->second;
      if(curr_occupancy <= 0) continue;
      DUPLE curr_duple = path_occupancy_iter->first;
      if(curr_duple.first == player_value)
      {
         curr_path_score += (n_winnable_paths + 1) * curr_occupancy;
      }
      else
      {
         curr_path_score -= (n_winnable_paths + 2) * curr_occupancy;
      }

      if(curr_path_score > best_path_score)
      {
         best_path_score = curr_path_score;
//         best_path_ID = curr_duple.second;
      }

   } // loop over path_occupancy_iter

/*
   cout << "Player " << player_value << endl;
   cout << "best_path_ID = " << best_path_ID << endl;
   cout << "best_path_score = " << best_path_score << endl;

   vector<triple> best_path = winnable_paths[best_path_ID];
   for(unsigned int i = 0; i < best_path.size(); i++)
   {
      int px = best_path[i].first;
      int py = best_path[i].second;
      int pz = best_path[i].third;
      cout << "Z = " << pz << " row = " << py << " col = " << px << endl;
   }
*/

   return best_path_score;
}

// ---------------------------------------------------------------------
// Member function compute_winnable_path_occupancies() counts the
// number of player_value pieces within each winnable path.  If both
// players have pieces in a path, the number is set to -1.  

void tictac3d::compute_winnable_path_occupancies(int player_value)
{
   DUPLE D1, D2;
   D1.first = player_value;
   D2.first = -player_value;
   
   int n_winnable_paths = winnable_paths.size();
   for(int p = 0; p < n_winnable_paths; p++)
   {
      D1.second = p;
      D2.second = p;

      int n_player_pieces_in_path = 0;
      int n_opponent_pieces_in_path = 0;
      for(int i = 0; i < n_size; i++)
      {
         int curr_cell_value = get_cell_value(winnable_paths[p].at(i));
         if(curr_cell_value == -player_value)
         {
            n_opponent_pieces_in_path++;
         }
         else if (curr_cell_value == player_value)
         {
            n_player_pieces_in_path++;
         }

         if(n_player_pieces_in_path > 0 && n_opponent_pieces_in_path > 0)
         {
            path_occupancy_map[D1] = -1;
            path_occupancy_map[D2] = -1;
            break;
         }
      }

      if(n_opponent_pieces_in_path == 0)
      {
         path_occupancy_map[D1] = n_player_pieces_in_path;
      }

      if(n_player_pieces_in_path == 0)
      {
         path_occupancy_map[D2] = n_opponent_pieces_in_path;
      }

   } // loop over index p labeling all winnable paths
}

// ---------------------------------------------------------------------
// Generate metafile plot of game_illegal_frac, game_loss_frac,
// game_stalemate_frac and game_win_frac versus episodes

void tictac3d::plot_game_frac_histories(int n_episodes, string extrainfo)
{
   metafile curr_metafile;
   string meta_filename="game_histories";
   string title="Game histories vs episode";
   string x_label="Episode number";
   string y_label="Game history fractions";

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, n_episodes, 0, 1);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.set_legend_flag(true);
   curr_metafile.openmetafile();
   curr_metafile.write_header();

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Illegal");
   curr_metafile.write_curve(0, n_episodes, game_illegal_frac, colorfunc::purple);

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Loss");
   curr_metafile.write_curve(0, n_episodes, game_loss_frac, colorfunc::red);

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Stalemate");
   curr_metafile.write_curve(0, n_episodes, game_stalemate_frac, colorfunc::cyan);

   curr_metafile.set_thickness(3);
   curr_metafile.set_legendlabel("Win");
   curr_metafile.write_curve(0, n_episodes, game_win_frac, colorfunc::green);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}




