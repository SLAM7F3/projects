// ==========================================================================
// GeometricalPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 11/1/06; 12/29/06; 1/21/07; 8/17/07; 6/30/08
// ==========================================================================

#include <iostream>
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GeometricalPickHandler::allocate_member_objects()
{
}		       

void GeometricalPickHandler::initialize_member_objects()
{
   Allow_Insertion_flag=Allow_Manipulation_flag=true;
}		       

GeometricalPickHandler::GeometricalPickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   GraphicalsGroup* GG_ptr,ModeController* MC_ptr,
   WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GraphicalPickHandler(p_ndims,PI_ptr,CM_ptr,GG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
}

GeometricalPickHandler::~GeometricalPickHandler() 
{
}

// --------------------------------------------------------------------------
// Member function get_max_distance_to_Graphical hardwires in
// reasonable screen-space distances within which a mouse click must
// lie in order to select some Geometrical.

float GeometricalPickHandler::get_max_distance_to_Graphical()
{
   double max_dist=-1.0;
   if (get_ndims()==2)
   {
      max_dist=0.1;	 
   }
   else if (get_ndims()==3)
   {
      max_dist=0.2;
   }

   return max_dist;
}

// ==========================================================================
// Mouse event handling methods:
// ==========================================================================

bool GeometricalPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside GeomPickHandler::doubleclick()" << endl;
   return false;
}
