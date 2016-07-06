// ==========================================================================
// ROADPOINT base class member function definitions
// ==========================================================================
// Last modified on 3/12/05; 6/14/06; 7/29/06
// ==========================================================================

#include <iostream>
#include "datastructures/Linkedlist.h"
#include "urban/roadpoint.h"
#include "math/threevector.h"

using std::ostream;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void roadpoint::allocate_member_objects()
{
}		       

void roadpoint::initialize_member_objects()
{
   intersection=near_bldg_island=false;
   at_infinity=in_front_of_bldg=false;
   on_data_bbox=data_bbox_corner=false;
   nearby_bldg_list_ptr=NULL;
}		       

roadpoint::roadpoint()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

roadpoint::roadpoint(const threevector& p):
   contour_element(p)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

roadpoint::roadpoint(int identification,const threevector& p):
   contour_element(identification,p)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

roadpoint::roadpoint(const roadpoint& r):
   contour_element(r)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(r);
}

roadpoint::~roadpoint()
{
   delete nearby_bldg_list_ptr;
   nearby_bldg_list_ptr=NULL;
}

// ---------------------------------------------------------------------
void roadpoint::docopy(const roadpoint& r)
{
   intersection=r.intersection;
   near_bldg_island=r.near_bldg_island;
   at_infinity=r.at_infinity;
   in_front_of_bldg=r.in_front_of_bldg;
   on_data_bbox=r.on_data_bbox;
   data_bbox_corner=r.data_bbox_corner;
   if (r.nearby_bldg_list_ptr != NULL)
   {
      if (nearby_bldg_list_ptr==NULL) 
         nearby_bldg_list_ptr=new Linkedlist<int>(
            *(r.nearby_bldg_list_ptr));
   }
   adjacent_cityblock=r.adjacent_cityblock;
}

// ---------------------------------------------------------------------
// Overload = operator:

roadpoint& roadpoint::operator= (const roadpoint& r)
{
   if (this==&r) return *this;
   contour_element::operator=(r);
   docopy(r);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (std::ostream& outstream,const roadpoint& r)
{
   outstream << endl;
   outstream << (contour_element&)r << endl;
   outstream << "in front of building = " << r.in_front_of_bldg << endl;
   outstream << "on data bounding box = " << r.on_data_bbox << endl;
   outstream << "data bbox corner = " << r.data_bbox_corner << endl;
   return outstream;
}

