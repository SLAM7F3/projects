// ==========================================================================
// ROADSEGMENT base class member function definitions
// ==========================================================================
// Last modified on 3/13/05; 6/14/06; 8/6/06
// ==========================================================================

#include <iostream>
#include "urban/roadsegment.h"

using std::ostream;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void roadsegment::allocate_member_objects()
{
}		       

void roadsegment::initialize_member_objects()
{
   start_roadpoint_ptr=stop_roadpoint_ptr=NULL;
}		       

roadsegment::roadsegment():
   contour_element()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

roadsegment::roadsegment(
   int identification,int begin_label,int end_label,
   Network<roadpoint*> const *roadpoints_network_ptr):
   contour_element(identification)
{
   start_roadpoint_ptr=
      roadpoints_network_ptr->get_site_data_ptr(begin_label);
   stop_roadpoint_ptr=
      roadpoints_network_ptr->get_site_data_ptr(end_label);
   l=linesegment(start_roadpoint_ptr->get_posn(),
                 stop_roadpoint_ptr->get_posn());
   set_posn(l.get_midpoint());
}

// Copy constructor:

roadsegment::roadsegment(const roadsegment& r):
   contour_element(r)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(r);
}

roadsegment::~roadsegment()
{
}

// ---------------------------------------------------------------------
void roadsegment::docopy(const roadsegment& r)
{
   start_roadpoint_ptr=r.start_roadpoint_ptr;
   stop_roadpoint_ptr=r.stop_roadpoint_ptr;
   l=r.l;
}

// ---------------------------------------------------------------------
// Overload = operator:

roadsegment& roadsegment::operator= (const roadsegment& r)
{
   if (this==&r) return *this;
   contour_element::operator=(r);
   docopy(r);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (std::ostream& outstream,const roadsegment& r)
{
   outstream << endl;
   outstream << (contour_element&)r << endl;
   outstream << "start_roadpoint = " << *(r.start_roadpoint_ptr) << endl;
   outstream << "stop_roadpoint = " << *(r.stop_roadpoint_ptr) << endl;
   outstream << "road segment = " << r.l << endl;
   return outstream;
}

