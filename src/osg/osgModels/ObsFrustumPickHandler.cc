// ==========================================================================
// ObsFrustumPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 9/12/07; 9/13/07; 9/22/07; 10/25/07; 6/15/08
// ==========================================================================

#include <iostream>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgModels/ObsFrustumPickHandler.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ObsFrustumPickHandler::allocate_member_objects()
{
}		       

void ObsFrustumPickHandler::initialize_member_objects()
{
}		       

ObsFrustumPickHandler::ObsFrustumPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ObsFrustaGroup* OFG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(
      3,PI_ptr,CM_ptr,OFG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ObsFrustaGroup_ptr=OFG_ptr;
}

ObsFrustumPickHandler::~ObsFrustumPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool ObsFrustumPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//    cout << "inside OFPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// Select the 3D ObsFrustum whose center lies closest to (X,Y) in
// screen space:

         return select_ObsFrustum();
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   } // MANIPULATE_FUSED_DATA mode conditional
}

// --------------------------------------------------------------------------
bool ObsFrustumPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   return false;
}

// --------------------------------------------------------------------------
bool ObsFrustumPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ObsFrustumPickHandler::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state != ModeController::MANIPULATE_FUSED_DATA) return false;

   if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
   {
      if (select_ObsFrustum())
      {
         ObsFrustaGroup_ptr->flyto_camera_location(
            get_selected_Graphical_ID());
         return true;
      }
   }
   return false;
}

// --------------------------------------------------------------------------
bool ObsFrustumPickHandler::release()
{
//   cout << "inside OFPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      ObsFrustaGroup_ptr->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// ObsFrustum generation, manipulation and annihilation methods
// ==========================================================================

// Method select_ObsFrustum assigns selected_ObsFrustum_ID equal to
// the ID of an existing cross hair which lies sufficiently close to a
// point picked by the user with his mouse.  If no ObsFrustum is
// nearby the selected point, selected_ObsFrustum_ID is set equal to
// -1, and all ObsFrusta are effectively de-selected.

bool ObsFrustumPickHandler::select_ObsFrustum()
{   
//   cout << "inside OFPH::select_ObsFrustum()" << endl;
   int selected_ObsFrustum_ID=select_Graphical();
   cout << "Selected ObsFrustum ID = " << selected_ObsFrustum_ID << endl;
//   ObsFrustaGroup_ptr->reset_colors();
   ObsFrustaGroup_ptr->hide_nonselected_ObsFrusta();
   return (selected_ObsFrustum_ID > -1);
}
