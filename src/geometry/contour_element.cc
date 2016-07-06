// ==========================================================================
// CONTOUR_ELEMENT base class member function definitions
// ==========================================================================
// Last modified on 1/27/05; 4/13/06; 6/14/06; 7/29/06
// ==========================================================================

#include <iostream>
#include "geometry/contour.h"
#include "geometry/contour_element.h"
#include "datastructures/Linkedlist.h"
#include "geometry/polygon.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void contour_element::allocate_member_objects()
{
}		       

void contour_element::initialize_member_objects()
{
   contour_ptr=NULL;
   subcontour_list_ptr=NULL;
   voronoi_region_ptr=NULL;
}		       

contour_element::contour_element():
   network_element()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

contour_element::contour_element(int identification):
   network_element(identification)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

contour_element::contour_element(const threevector& p):
   network_element(p)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

contour_element::contour_element(int identification,const threevector& p):
   network_element(identification,p)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

contour_element::contour_element(const contour_element& n):
   network_element(n)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(n);
}

contour_element::~contour_element()
{
   delete contour_ptr;
   delete voronoi_region_ptr;

   contour_ptr=NULL;
   voronoi_region_ptr=NULL;

   if (subcontour_list_ptr != NULL)
   {
      for (Mynode<contour*>* currnode_ptr=subcontour_list_ptr->
              get_start_ptr(); currnode_ptr != NULL; 
           currnode_ptr=currnode_ptr->get_nextptr())
      {
         delete currnode_ptr->get_data();
      }
      delete subcontour_list_ptr;
   }
   subcontour_list_ptr=NULL;
}

// ---------------------------------------------------------------------
void contour_element::docopy(const contour_element& n)
{
   *contour_ptr=*(n.contour_ptr);
   *subcontour_list_ptr=*(n.subcontour_list_ptr);
   *voronoi_region_ptr=*(n.voronoi_region_ptr);
}

// ---------------------------------------------------------------------
// Overload = operator:

contour_element& contour_element::operator= (const contour_element& n)
{
   if (this==&n) return *this;
   network_element::operator=(n);
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const contour_element& n)
{
   outstream << endl;
   outstream << (network_element&)n << endl;
   return outstream;
}
