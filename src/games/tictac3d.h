// ==========================================================================
// Header file for tictac3d class 
// ==========================================================================
// Last modified on 8/28/16
// ==========================================================================

#ifndef TICTAC3D_H
#define TICTAC3D_H

#include <map>
#include <vector>
#include "math/ltmatrix.h"
#include "math/threematrix.h"

class tictac3d
{
   
  public:

// Initialization, constructor and destructor functions:

   tictac3d();
   tictac3d(const tictac3d& C);
   ~tictac3d();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const tictac3d& C);

   void display_board_state();

  private: 

   int n_size;
   std::vector<int> curr_board_state;


   void allocate_member_objects();
   void initialize_member_objects();

   int get_cell_value(int px, int py, int pz);
   void display_Zgrid_state(int pz);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:



#endif  // tictac3d.h


