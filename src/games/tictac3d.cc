// ==========================================================================
// tictac3d class member function definitions
// ==========================================================================
// Last modified on 8/28/16
// ==========================================================================

#include <iostream>
#include <string>
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
            if(curr_ran < 0.1)
            {
               curr_val = -1;
            }
            else if (curr_ran > 0.9)
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
   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         string cell_str;
         if(get_cell_value(px, py, pz) == 0)
         {
            cell_str = "   ";
         }
         else if(get_cell_value(px, py, pz) == -1)
         {
            cell_str = " X ";
         }
         else if(get_cell_value(px, py, pz) == 1)
         {
            cell_str = " O ";
         }
         cout << cell_str << flush;
         if(px < n_size - 1)
         {
            cout << "|" << flush;
         }
      } // loop over px index
      cout << endl;

      if(py < n_size - 1)
      {
         for(int px = 0; px < n_size; px++)
         {
            cout << "----" << flush;
         }
      }
      cout << endl;
   } // loop over py index
}


void tictac3d::display_board_state()
{
   for(int pz = 0; pz < n_size; pz++)
   {
      cout << endl;
      cout << "Z = " << pz << endl << endl;
      display_Zgrid_state(pz);
   } // loop over pz index
   
}
