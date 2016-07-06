// ==========================================================================
// dtree_training class member function definitions
// ==========================================================================
// Last modified on 8/1/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "classification/dtree_training.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void dtree_training::allocate_member_objects()
{
}		       

void dtree_training::initialize_member_objects()
{
}		       

dtree_training::dtree_training(int id)
{
   allocate_member_objects();
   initialize_member_objects();

   ID = id;
}

// Copy constructor:

/*
dtree_training::dtree_training(const dtree_training& dt)
{
   docopy(dt);
}
*/

// ---------------------------------------------------------------------
dtree_training::~dtree_training()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const dtree_training& training)
{
   outstream << endl;
   outstream << "Example " << training.get_ID() << endl;

   for(unsigned int f = 0; f < training.feature_values.size(); f++)
   {
      outstream << "  Feature " << f << " : " 
                << training.feature_labels[f] << " = "
                << training.feature_values[f] << endl;
   }
   
   outstream << "  Classification value = " 
             << training.classification_value << endl;
   return outstream;
}

// ==========================================================================
// Set and get member functions
// ==========================================================================
