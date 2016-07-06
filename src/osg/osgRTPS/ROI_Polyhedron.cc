// ==========================================================================
// ROI_Polyhedron class member function definitions
// ==========================================================================
// Last updated on 12/21/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/polyhedron.h"
#include "osg/osgRTPS/ROI_Polyhedron.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ROI_Polyhedron::allocate_member_objects()
{
}		       

void ROI_Polyhedron::initialize_member_objects()
{
   Graphical_name="ROI_Polyhedron";
}		       

ROI_Polyhedron::ROI_Polyhedron(
   Pass* PI_ptr,const threevector& grid_world_origin,osgText::Font* f_ptr,
   int id,AnimationController* AC_ptr):
   Polyhedron(PI_ptr,grid_world_origin,f_ptr,id,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ROI_Polyhedron::ROI_Polyhedron(
   Pass* PI_ptr,const threevector& grid_world_origin,
   polyhedron* p_ptr,osgText::Font* f_ptr,int id,AnimationController* AC_ptr):
   Polyhedron(PI_ptr,grid_world_origin,p_ptr,f_ptr,id,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ROI_Polyhedron::~ROI_Polyhedron()
{
//   cout << "inside ROI_Polyhedron destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ROI_Polyhedron& P)
{
   outstream << "inside ROI_Polyhedron::operator<<" << endl;
   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

// ==========================================================================
// ROI_Polyhedron generation methods
// ==========================================================================
