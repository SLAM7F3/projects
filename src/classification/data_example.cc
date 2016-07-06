// ==========================================================================
// data_example class member function definitions
// ==========================================================================
// Last modified on 8/1/15; 8/30/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "classification/data_example.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void data_example::allocate_member_objects()
{
}		       

void data_example::initialize_member_objects()
{
}		       

data_example::data_example(int id)
{
   allocate_member_objects();
   initialize_member_objects();

   ID = id;
}

// Copy constructor:

/*
data_example::data_example(const data_example& dt)
{
   docopy(dt);
}
*/

// ---------------------------------------------------------------------
data_example::~data_example()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const data_example& training)
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
