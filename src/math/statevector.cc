// ==========================================================================
// Statevector class member function definitions 
// ==========================================================================
// Last modified on 4/27/06
// ==========================================================================

#include <iostream>
#include "math/statevector.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization functions:

void statevector::allocate_member_objects()
{
}		 

void statevector::initialize_member_objects()
{
   time=0;
   position=velocity=threevector(0,0,0);
}

statevector::statevector()
{
   allocate_member_objects();
   initialize_member_objects();
}

statevector::statevector(
   double t,const threevector& posn,const threevector& vel)
{
   allocate_member_objects();
   initialize_member_objects();

   time=t;
   position=posn;
   velocity=vel;
}

// Copy constructor:

statevector::statevector(const statevector& s)
{
   docopy(s);
}

statevector::~statevector()
{
}

// ---------------------------------------------------------------------
void statevector::docopy(const statevector& s)
{
   time=s.time;
   position=s.position;
   velocity=s.velocity;
}	

// Overload = operator:

statevector& statevector::operator= (const statevector& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const statevector& s)
{
   outstream << endl;
   outstream << "time = " << s.time << endl;
   outstream << "position = " << s.position << endl;
   outstream << "velocity = " << s.velocity << endl;
   return(outstream);
}
