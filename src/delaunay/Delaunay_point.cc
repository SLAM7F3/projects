// ==========================================================================
// Delaunay_point class member function definitions
// ==========================================================================
// Last modified on 12/14/05; 7/12/06
// ==========================================================================

#include <iostream>
#include "delaunay/Delaunay_point.h"
#include "math/threevector.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
void Delaunay_point::allocate_member_objects()
{
}

void Delaunay_point::initialize_member_objects()
{
   ID=-1;
}

Delaunay_point::Delaunay_point(double xx,double yy) 
{ 
   allocate_member_objects();
   initialize_member_objects();
   
   x=xx; 
   y=yy; 
}

Delaunay_point::Delaunay_point(int id,double xx,double yy) 
{ 
   allocate_member_objects();
   initialize_member_objects();

   ID=id;
   x=xx; 
   y=yy; 
}

void Delaunay_point::lineto(Delaunay_point* p_ptr,
                            Network<threevector*>* vertex_network_ptr)
{ 
//   cout << "V1:  ID = " << ID 
//        << " posn = (" << this->x << "," << this->y << ")      ";
//   cout << "V2:  ID = " << p_ptr->get_ID()
//        << " posn = (" << p_ptr->x << "," << p_ptr->y << ") " << endl;
   vertex_network_ptr->add_symmetric_link(ID,p_ptr->get_ID());
}
