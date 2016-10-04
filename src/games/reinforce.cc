// ==========================================================================
// reinforce class member function definitions
// ==========================================================================
// Last modified on 10/4/16
// ==========================================================================

#include <string>
#include "math/genmatrix.h"
#include "machine_learning/machinelearningfuncs.h"
#include "numrec/nrfuncs.h"
#include "games/reinforce.h"

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

void reinforce::initialize_member_objects()
{
   Din = 4 * 4;		// Input dimensionality
   Dout = 4 * 4;	// Output dimensionality
   H = 200;		// Number of hidden layer neurons

   Din = 3;
   Dout = 4;
   H = 5;
   
   batch_size = 10;	// Perform parameter update after this many episodes
   learning_rate = 0.001;	
   gamma = 0.99;	// Discount factor for reward
}		       

void reinforce::allocate_member_objects()
{
   W1_ptr = new genmatrix(H, Din);
   W2_ptr = new genmatrix(Dout, H);
   grad1_ptr = new genmatrix(H, Din);
   grad2_ptr = new genmatrix(Dout, H);
   rmsprop1_ptr = new genmatrix(H, Din);
   rmsprop2_ptr = new genmatrix(Dout, H);
}		       

void reinforce::clear_matrices()
{
   W1_ptr->clear_matrix_values();
   W2_ptr->clear_matrix_values();
   grad1_ptr->clear_matrix_values();
   grad2_ptr->clear_matrix_values();
   rmsprop1_ptr->clear_matrix_values();
   rmsprop2_ptr->clear_matrix_values();
}		       

void reinforce::print_matrices()
{
   cout << "*W1_ptr = " << *W1_ptr << endl;
   cout << "*W2_ptr = " << *W2_ptr << endl;

   cout << "*grad1_ptr = " << *grad1_ptr << endl;
   cout << "*grad2_ptr = " << *grad2_ptr << endl;

   cout << "*rmsprop1_ptr = " << *rmsprop1_ptr << endl;
   cout << "*rmsprop2_ptr = " << *rmsprop2_ptr << endl;
}		       

// ---------------------------------------------------------------------
reinforce::reinforce()
{
   initialize_member_objects();
   allocate_member_objects();
   clear_matrices();
}

// Copy constructor:

reinforce::reinforce(const reinforce& R)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
reinforce::~reinforce()
{
   delete W1_ptr;
   delete W2_ptr;
   delete grad1_ptr;
   delete grad2_ptr;
   delete rmsprop1_ptr;
   delete rmsprop2_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const reinforce& R)
{
   outstream << endl;
   outstream << "Din = " << R.Din << " Dout = " << R.Dout << endl;
   outstream << "H = " << R.H << endl;
   outstream << "batch_size = " << R.batch_size << " learning_rate = "
             << R.learning_rate << endl;
   outstream << "gamma = " << R.gamma << endl;
   return outstream;
}

// ==========================================================================

void reinforce::xavier_init_weight_matrices()
{
   for(int py = 0; py < W1_ptr->get_ndim(); py++)
   {
      for(int px = 0; px < W1_ptr->get_mdim(); px++)
      {
         W1_ptr->put(px,py,nrfunc::gasdev() / sqrt(Din) );
      } // loop over px
   } // loop over py 

   for(int py = 0; py < W2_ptr->get_ndim(); py++)
   {
      for(int px = 0; px < W2_ptr->get_mdim(); px++)
      {
         W2_ptr->put(px,py,nrfunc::gasdev() / sqrt(H) );
      } // loop over px
   } // loop over py 

   cout << "W1 = " << *W1_ptr << endl;
   cout << "W2 = " << *W2_ptr << endl;
}

