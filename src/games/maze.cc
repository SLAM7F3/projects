// ==========================================================================
// maze class member function definitions
// ==========================================================================
// Last modified on 11/5/16
// ==========================================================================

#include <iostream>
#include <string>
#include "games/maze.h"
#include "general/stringfuncs.h"

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

void maze::allocate_member_objects()
{
   n_cells = n_size * n_size;
   grid_ptr = new genmatrix(n_cells);
}		       

void maze::initialize_member_objects()
{
   game_over = false;


// Load cell_decomposition vector with (px,py,pz) triples
// corresponding to p = 0 --> n_cells - 1:

   for(int q = 0; q < n_cells; q++)
   {
      int p = q;
      int py = p / n_size;
      p -= n_size * py;
      int px = p;
      cell_decomposition.push_back(DUPLE(px,py));
   }
}		       

// ---------------------------------------------------------------------
maze::maze(int n_size)
{
   this->n_size = n_size;

   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

maze::maze(const maze& T)
{
//   docopy(T);
   generate_maze();
}

// ---------------------------------------------------------------------
maze::~maze()
{
   delete grid_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const maze& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

void maze::generate_maze()
{
   cout << "inside generate_maze()" << endl;

   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         grid_ptr->put(px,py,1);
      }
   }
   
   int nbits = 5;
   for(int n = 0; n < 32; n++)
   {
      char curr_char = stringfunc::ascii_integer_to_char(n);
      string curr_str = stringfunc::byte_bits_rep(curr_char,nbits);
      cout << "n = " << n << " curr_str = " << curr_str 
           << " base10_value = " << stringfunc::bits_rep_to_integer(curr_str)
           << endl;

   } // loop over index n 
   

}
