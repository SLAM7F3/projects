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

   for(int pz = 0; pz < n_size; pz++)
   {
      for(int py = 0; py < n_size; py++)
      {
         for(int px = 0; px < n_size; px++)
         {

            double curr_ran = nrfunc::ran1();

            int curr_val = 0;
            if(curr_ran < 0.45)
            {
               curr_val = -1;
            }
            else if (curr_ran > 0.55)
            {
               curr_val = 1;
            }
            curr_board_state.push_back(curr_val);
         } // loop over px 
      } // loop over py
   } // loop over pz
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
void tictac3d::display_board_state()
{
   for(int pz = 0; pz < n_size; pz++)
   {
      cout << endl;
      cout << "Z = " << pz << endl << endl;
      display_Zgrid_state(pz);
   } // loop over pz index
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
      cout << " X = " << winning_posns_iter->first.first
           << " Y = " << winning_posns_iter->first.second
           << " Z = " << winning_posns_iter->first.third
           << endl;
      winner_ID = winning_posns_iter->second;
   }
   cout << "Player " << winner_ID << " wins!" << endl;

   display_board_state();
}

// ---------------------------------------------------------------------
// Boolean member function check_player_win() returns true if input
// player_ID has a winning board state.

bool tictac3d::check_player_win(int player_ID)
{
   cout << "inside check_player_win, player_ID = " << player_ID << endl;

   bool game_over = false;

// Check if player wins within any of the Z-planes:
   for(int pz = 0; pz < n_size; pz++)
   {
      if(Zplane_win(player_ID, pz))
      {
         print_winning_pattern();
         game_over = true;
      }
   } // loop over pz

   return game_over;
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


