// ==========================================================================
// maze class member function definitions
// ==========================================================================
// Last modified on 11/5/16
// ==========================================================================

#include <iostream>
#include <string>
#include "math/mathfuncs.h"
#include "games/maze.h"
#include "numrec/nrfuncs.h"
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
   visited_ptr = new genmatrix(n_cells);
}		       

void maze::initialize_member_objects()
{
   game_over = false;

   direction.push_back(1); // up
   direction.push_back(2); // right
   direction.push_back(4); // down
   direction.push_back(8); // left

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
   delete visited_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const maze& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

DUPLE maze::getDirection(int curr_dir)
{
   if(curr_dir == 0)
   {
      return DUPLE(0,-1);
   }
   else if(curr_dir == 1)
   {
      return DUPLE(1,0);
   }
   else if(curr_dir == 2)
   {
      return DUPLE(0,1);
   }
   else if(curr_dir == 3)
   {
      return DUPLE(-1,0);
   }
   else
   {
      cout << "Error in maze::getDirection()" << endl;
      cout << "curr_dir = " << curr_dir << endl;
      exit(-1);
   }
}

bool maze::IsDirValid(int px, int py, int curr_dir)
{
   DUPLE currDirection = getDirection(curr_dir);
   int qx = px + currDirection.first;
   int qy = py + currDirection.second;
   cout << "qx = " << qx << " qy = " << qy << endl;
   if(qx < 0 || qx >= n_size || qy < 0 || qy >= n_size)
   {
      return false;
   }
   return true;
}

int maze::get_neighbor(int p, int curr_dir)
{
   int px = cell_decomposition[p].first;
   int py = cell_decomposition[p].second;
   if(!IsDirValid(px,py,curr_dir)) return -1;
   DUPLE currDirection = getDirection(curr_dir);
   int qx = px + currDirection.first;
   int qy = py + currDirection.second;
   cout << "qx = " << qx << " qy = " << qy << endl;

   if(qx < 0 || qx >= n_size || qy < 0 || qy >= n_size)
   {
      return -1;
   }
   else
   {
      return get_cell(qx,qy);
   }
}


void maze::generate_maze()
{
   cout << "inside generate_maze()" << endl;

   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         grid_ptr->put(px,py,15); // All cells start with 4 walls
         visited_ptr->put(px,py,0);
      }
   }

   int p = mathfunc::getRandomInteger(n_cells);
   int curr_dir;
   int q = -1;
   while(q < 0)
   {
      curr_dir = mathfunc::getRandomInteger(4);
      q = get_neighbor(p, curr_dir);   
   }

   cout << "p = " << p << " curr_dir = " << curr_dir
        << " q = " << q << endl;
   

//   char curr_char = stringfunc::ascii_integer_to_char(n);
//   string curr_str = stringfunc::byte_bits_rep(curr_char,nbits);
   

}
