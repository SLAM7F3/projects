// ==========================================================================
// Satellite class member function definitions 
// ==========================================================================
// Last modified on 4/27/06; 6/7/06
// ==========================================================================

#include "space/satellite.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:
// ---------------------------------------------------------------------

void satellite::allocate_member_objects() 
{
   attitude_ptr=new rpy;
   rate_ptr=new rpy;
   error_ptr=new rpy;
   error_dot_ptr=new rpy;
   max_attitude_ptr=new rpy;
   min_attitude_ptr=new rpy;
   max_rate_ptr=new rpy;
   min_rate_ptr=new rpy;
   max_accel_ptr=new rpy;
}

void satellite::initialize_member_objects() 
{
   attitude_ptr->set_roll(0);
   attitude_ptr->set_pitch(0);
   attitude_ptr->set_yaw(0);
   rate_ptr->set_roll(0);
   rate_ptr->set_pitch(0);
   rate_ptr->set_yaw(0);
   error_ptr->set_roll(0);
   error_ptr->set_pitch(0);
   error_ptr->set_yaw(0);
   error_dot_ptr->set_roll(0);
   error_dot_ptr->set_pitch(0);
   error_dot_ptr->set_yaw(0);
}

satellite::satellite(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

satellite::satellite(const satellite& s)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(s);
}

satellite::~satellite()
{
   delete attitude_ptr;
   delete rate_ptr;
   delete error_ptr;
   delete error_dot_ptr;
   delete max_attitude_ptr;
   delete min_attitude_ptr;
   delete max_rate_ptr;
   delete min_rate_ptr;
   delete max_accel_ptr;
}

// ---------------------------------------------------------------------
void satellite::docopy(const satellite& s)
{
   *(attitude_ptr)=*(s.attitude_ptr);
   *(rate_ptr)=*(s.rate_ptr);
   *(max_attitude_ptr)=*(s.max_attitude_ptr);
   *(min_attitude_ptr)=*(s.min_attitude_ptr);
   *(max_rate_ptr)=*(s.max_rate_ptr);
   *(min_rate_ptr)=*(s.min_rate_ptr);
   *(max_accel_ptr)=*(s.max_accel_ptr);
   *(error_ptr)=*(s.error_ptr);
   *(error_dot_ptr)=*(s.error_dot_ptr);
}	

// Overload = operator:

satellite& satellite::operator= (const satellite& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const satellite& s)
{
   outstream << endl;
   return(outstream);
}
