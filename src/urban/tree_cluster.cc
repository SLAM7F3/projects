// ==========================================================================
// TREE_CLUSTER class member function definitions
// ==========================================================================
// Last modified on 4/17/05; 6/14/06; 7/29/06
// ==========================================================================

#include <iostream>
#include "urban/tree_cluster.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void tree_cluster::allocate_member_objects()
{
}		       

void tree_cluster::initialize_member_objects()
{
}		       

tree_cluster::tree_cluster()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

tree_cluster::tree_cluster(int identification):
   contour_element(identification)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

tree_cluster::tree_cluster(int identification,const threevector& p):
   contour_element(identification,p)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

tree_cluster::tree_cluster(const tree_cluster& t):
   contour_element(t)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(t);
}

tree_cluster::~tree_cluster()
{
}

// ---------------------------------------------------------------------
void tree_cluster::docopy(const tree_cluster& t)
{
}

// ---------------------------------------------------------------------
// Overload = operator:

tree_cluster& tree_cluster::operator= (const tree_cluster& t)
{
   if (this==&t) return *this;
   contour_element::operator=(t);
   docopy(t);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const tree_cluster& t)
{
   outstream << endl;
   outstream << (contour_element&)t << endl;
   return outstream;
}
