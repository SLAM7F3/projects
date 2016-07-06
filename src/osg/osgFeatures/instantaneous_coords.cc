// ==========================================================================
// INSTANTANEOUS_COORDS class member function definitions
// ==========================================================================
// Last modified on 7/18/05; 4/27/06
// ==========================================================================

#include <iostream>
#include "osg/osgFeatures/instantaneous_coords.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void instantaneous_coords::allocate_member_objects()
{
}		       

void instantaneous_coords::initialize_member_objects()
{
}		       

instantaneous_coords::instantaneous_coords()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

instantaneous_coords::instantaneous_coords(double curr_t):
   instantaneous_obs(curr_t)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

instantaneous_coords::~instantaneous_coords()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const instantaneous_coords& C)
{
   outstream << "inside instantaneous_coords::operator<<" << endl;
   outstream << static_cast<const instantaneous_obs&>(C) << endl;

   outstream << "State-vector position = " << C.S.get_position() << endl;
   outstream << "State-vector velocity = " << C.S.get_velocity() << endl;

   return(outstream);
}
