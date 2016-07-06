// ==========================================================================
// CENTER class member function definitions
// ==========================================================================
// Last modified on 9/30/05; 10/13/07
// ==========================================================================

#include <iostream>
#include "osg/osgGraphicals/Center.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Center::allocate_member_objects()
{
}		       

void Center::initialize_member_objects()
{
   Graphical_name="Center";
}		       

Center::Center(const int p_ndims,int id):
   Graphical(p_ndims,id)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

Center::~Center()
{
//   cout << "inside Center destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Center& c)
{
   return(outstream);
}
