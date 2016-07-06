// ==========================================================================
// PowerPointPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 8/24/07; 9/11/07; 6/15/08; 9/2/08
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include <osg/Object>
#include "osg/osgAnnotators/PowerPoint.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osgAnnotators/PowerPointPickHandler.h"
#include "osg/CustomManipulator.h"
#include "general/inputfuncs.h"
#include "osg/ModeController.h"
#include "osg/ModeController.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PowerPointPickHandler::allocate_member_objects()
{
}		       

void PowerPointPickHandler::initialize_member_objects()
{
}		       

PowerPointPickHandler::PowerPointPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PowerPointsGroup* ASG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   BoxPickHandler(PI_ptr,CM_ptr,ASG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PowerPointsGroup_ptr=ASG_ptr;
}

PowerPointPickHandler::~PowerPointPickHandler() 
{
}

// ---------------------------------------------------------------------
PowerPointsGroup* PowerPointPickHandler::get_PowerPointsGroup_ptr() 
{
   return PowerPointsGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool PowerPointPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside ASPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_BOX, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Box whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_BOX)
         {
            return instantiate_PowerPoint(ea.getX(),ea.getY());
         }
         else
         {
            return select_PowerPoint();
          }
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   } // INSERT_BOX or MANIPULATE_BOX mode conditional
}

// --------------------------------------------------------------------------
bool PowerPointPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PowerPointPickHandler::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
         PowerPoint* curr_PowerPoint_ptr=get_PowerPointsGroup_ptr()->
            get_ID_labeled_PowerPoint_ptr(
               get_selected_Graphical_ID());
         get_PowerPointsGroup_ptr()->update_mybox(curr_PowerPoint_ptr);
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool PowerPointPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside PPPH::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state != ModeController::MANIPULATE_BOX) return false;

   return PowerPointsGroup_ptr->issue_invocation_message();
}

// ==========================================================================
// PowerPoint generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_PowerPoint creates a new PowerPoint, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_PowerPoint_number equal to that ID and adds it to
// the OSG PowerPoint group.

bool PowerPointPickHandler::instantiate_PowerPoint(double X,double Y)
{   
//   cout << "inside PPPH::instantiate_PowerPoint" << endl;

   bool powerpoint_instantiated_flag=true;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      PowerPoint* curr_PowerPoint_ptr=PowerPointsGroup_ptr->
         generate_new_PowerPoint();
      instantiate_Graphical(curr_PowerPoint_ptr);
      PowerPointsGroup_ptr->generate_powerpoint_group(curr_PowerPoint_ptr);

// Move instantiated box in the positive z direction by half its
// height so that its bottom just touches the world-grid:

      double t=get_PowerPointsGroup_ptr()->get_curr_t();
      int passnumber=get_PowerPointsGroup_ptr()->get_passnumber();
      threevector PowerPoint_posn;
      if (curr_PowerPoint_ptr->get_UVW_coords(t,passnumber,PowerPoint_posn))
      {
         PowerPoint_posn.put(2,PowerPoint_posn.get(2)+
                             0.5*get_PowerPointsGroup_ptr()->get_height());
         curr_PowerPoint_ptr->set_UVW_coords(t,passnumber,PowerPoint_posn);
      }

      int face_number=4;	// Initially select top face
      threevector origin(0,0,0);
      curr_PowerPoint_ptr->reset_selected_face_drawable(face_number,origin);

      get_PowerPointsGroup_ptr()->update_mybox(curr_PowerPoint_ptr);
   }
   
   return powerpoint_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_PowerPoint assigns selected_PowerPoint_number equal
// to the ID of an existing PowerPoint which lies sufficiently close
// to a point picked by the user with his mouse.  If no PowerPoint is
// nearby the selected point, selected_PowerPoint_number is set equal
// to -1, and all PowerPointes are effectively de-selected.

bool PowerPointPickHandler::select_PowerPoint()
{   
   int selected_PowerPoint_ID=select_Graphical();
   PowerPointsGroup_ptr->reset_colors();
   return (selected_PowerPoint_ID > -1);
}
