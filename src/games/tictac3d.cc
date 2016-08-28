// ==========================================================================
// tictac3d class member function definitions
// ==========================================================================
// Last modified on 8/28/16
// ==========================================================================

#include <iostream>
#include <string>
#include "color/colortext.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "games/tictac3d.h"

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
}		       

void tictac3d::initialize_member_objects()
{
   n_size = 4;
}		       


// ---------------------------------------------------------------------
tictac3d::tictac3d()
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

tictac3d::tictac3d(const tictac3d& T)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
tictac3d::~tictac3d()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const tictac3d& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

int tictac3d::get_cell_value(int px, int py, int pz)
{
   int p = n_size * n_size * pz + n_size * py + px;   
   return curr_board_state.at(p);
}

// ---------------------------------------------------------------------
void tictac3d::randomize_board_state()
{
   n_size = 4;

   curr_board_state.clear();
   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
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
void tictac3d::display_board_state()
{
   for(int pz = 0; pz < n_size; pz++)
   {
      cout << endl;
      cout << "Z = " << pz << endl << endl;
      display_Zgrid_state(pz);
   } // loop over pz index
}

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
            else
            {
               cout << yellow;
            }
            cout << cell_str << def << flush;
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
   }

   string winner_symbol = " X ";
   if(winner_ID == 1)
   {
      winner_symbol = " O ";
   }

   cout << endl;
   cout << "*****************************************" << endl;
   cout << "Player" << winner_symbol << "wins!" << endl;
   cout << "*****************************************" << endl;

   display_board_state();
}

// ---------------------------------------------------------------------
// Boolean member function check_player_win() returns true if input
// player_ID has a winning board state.

int tictac3d::check_player_win(int player_ID)
{
   cout << "inside check_player_win, player_ID = " << player_ID << endl;

   int win_in_zplane = 1;
   int win_in_zcolumn = 2;
   int win_in_zslant = 3;
   int win_in_corner_diag = 4;
   winning_posns_map.clear();

// Check if player wins via 4 corner-to-corner diagonals:
   if(corner_2_corner_win(player_ID))
   {
      print_winning_pattern();
      return win_in_corner_diag;
   }

// Check if player wins via 16 Z-slants:

   for(int px = 0; px < n_size; px++)
   {
      if(Zslant_xconst_win(player_ID,px))
      {
         print_winning_pattern();
         return win_in_zslant;
      }
   }

   for(int py = 0; py < n_size; py++)
   {
      if(Zslant_yconst_win(player_ID,py))
      {
         print_winning_pattern();
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
            print_winning_pattern();
            return win_in_zcolumn;
         }
      }
   }

// Check if player wins within any of 40 Z-planes possibilities:
   for(int pz = 0; pz < n_size; pz++)
   {
      if(Zplane_win(player_ID, pz))
      {
         print_winning_pattern();
         return win_in_zplane;
      }
   } // loop over pz


   return 0;
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
   
   return false;
}

// ---------------------------------------------------------------------
// Boolean member function Zcolumn_win() returns true if input
// player_ID occupies 4 continguous cells with Z = 0, 1, ... n_size - 1:

bool tictac3d::Zcolumn_win(int player_ID, int px, int py)
{
   bool column_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
   {
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
      {
         column_win = false;
      }
   }
   return column_win;
}

// ---------------------------------------------------------------------
// Boolean member function Zslant_x[y]const_win() returns true if input
// player_ID occupies 4 continguous cells with Z = 0, 1, ... n_size - 1:

bool tictac3d::Zslant_xconst_win(int player_ID, int px)
{
   bool slant_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
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
   for(int pz = 0; pz < n_size; pz++)
   {
      int py = n_size - 1 - pz;
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
      {
         slant_win = false;
      }
   }
   if(slant_win) return slant_win;

   return false;
}

bool tictac3d::Zslant_yconst_win(int player_ID, int py)
{
   bool slant_win = true;
   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
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
   for(int pz = 0; pz < n_size; pz++)
   {
      int px = n_size - 1 - pz;
      winning_posns_map[triple(px,py,pz)] = player_ID;
      if(get_cell_value(px,py,pz) != player_ID)
      {
         slant_win = false;
      }
   }
   if(slant_win) return slant_win;

   return false;
}

// ---------------------------------------------------------------------
// Boolean member function corner_2_corner_win() returns true if input
// player_ID occupies 4 continguous cells from one 3D corner to another:

bool tictac3d::corner_2_corner_win(int player_ID)
{
   bool corner_diag_win = true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
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
   for(int pz = 0; pz < n_size; pz++)
   {
      int px = n_size - 1 - pz;
      int py = pz;
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
   {
      int px = pz;
      int py = n_size - 1 - pz;
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   winning_posns_map.clear();
   for(int pz = 0; pz < n_size; pz++)
   {
      int px = n_size - 1 - pz;
      int py = n_size - 1 - pz;
      winning_posns_map[triple(px, py, pz)] = player_ID;
      if(get_cell_value(px, py, pz) != player_ID)
      {
         corner_diag_win = false;
      }
   }
   if(corner_diag_win) return true;

   return false;
}
