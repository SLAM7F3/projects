// ==========================================================================
// NETLINK base class member function definitions
// ==========================================================================
// Last modified on 3/31/04
// ==========================================================================

#include <iostream>
#include "netlink.h"

using std::ostream;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void netlink::allocate_member_objects()
{
}		       

void netlink::initialize_member_objects()
{
   neighbor_ID=-1;
   score=-1;
}		       

netlink::netlink()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

netlink::netlink(int ID)
{	
   initialize_member_objects();
   allocate_member_objects();
   neighbor_ID=ID;
}		       

netlink::netlink(int ID,double s)
{	
   initialize_member_objects();
   allocate_member_objects();
   neighbor_ID=ID;
   score=s;
}		       

// Copy constructor:

netlink::netlink(const netlink& r)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(r);
}

netlink::~netlink()
{
}

// ---------------------------------------------------------------------
void netlink::docopy(const netlink& r)
{
   neighbor_ID=r.neighbor_ID;
   score=r.score;
}

// ---------------------------------------------------------------------
// Overload = operator:

netlink& netlink::operator= (const netlink& r)
{
   if (this==&r) return *this;
   docopy(r);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const netlink& r)
{
   outstream << endl;
   outstream << "neighbor_ID = " << r.neighbor_ID << endl;
   outstream << "score = " << r.score << endl;
   return outstream;
}
