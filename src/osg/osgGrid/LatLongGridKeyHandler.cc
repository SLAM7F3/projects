// ==========================================================================
// LatLongGridKeyHandler class member function definitions
// ==========================================================================
// Last modified on 3/12/09
// ==========================================================================

#include <iostream>
#include "osg/osgGrid/LatLongGrid.h"
#include "osg/osgGrid/LatLongGridKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LatLongGridKeyHandler::allocate_member_objects()
{
}

void LatLongGridKeyHandler::initialize_member_objects()
{
}

LatLongGridKeyHandler::LatLongGridKeyHandler(
   ModeController* MC_ptr,LatLongGrid* G_ptr):
   GridKeyHandler(MC_ptr,G_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   LatLongGrid_ptr=G_ptr;
}

LatLongGridKeyHandler::~LatLongGridKeyHandler()
{
}

// ------------------------------------------------------
bool LatLongGridKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (ModeController_ptr->getState()==ModeController::VIEW_DATA)
      {
         switch ( ea.getKey() )
         {
            case 'm' :
               cout << "Toggling dynamic long lat lines" << endl;
               LatLongGrid_ptr->turn_off_dynamic_LongLatLines();
               return true;

            default :
               return false;
         } // switch statement
      } // Mode conditional
   } // key down conditional
   
   return false;
}


