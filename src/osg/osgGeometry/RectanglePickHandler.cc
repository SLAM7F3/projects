// ==========================================================================
// RectanglePickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 2/17/09; 2/9/11; 7/10/11
// ==========================================================================

#include <iostream>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgGeometry/Rectangle.h"
#include "osg/osgGeometry/RectanglesGroup.h"
#include "osg/osgGeometry/RectanglePickHandler.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RectanglePickHandler::allocate_member_objects()
{
}		       

void RectanglePickHandler::initialize_member_objects()
{
}		       

RectanglePickHandler::RectanglePickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,RectanglesGroup* RG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr):
   GeometricalPickHandler(2,PI_ptr,CM_ptr,RG_ptr,MC_ptr,WCC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   RectanglesGroup_ptr=RG_ptr;
}

RectanglePickHandler::~RectanglePickHandler() 
{
}

// ---------------------------------------------------------------------
RectanglesGroup* RectanglePickHandler::get_RectanglesGroup_ptr() 
{
   return RectanglesGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool RectanglePickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside RectanglePickHandler::pick()" << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_RECTANGLE ||
       curr_state==ModeController::MANIPULATE_RECTANGLE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==2)
      {

// If ModeController==INSERT_RECTANGLE, pick point whose screen-space
// coordinates lie closest to (X,Y).  Otherwise, select the Rectangle
// whose center lies closest to (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_RECTANGLE)
         {
            bool flag=instantiate_Rectangle();
            get_ModeController_ptr()->setState(
               ModeController::MANIPULATE_RECTANGLE);
            return flag;
            
         }
         else
         {
            return select_Rectangle();
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
   } // INSERT_RECTANGLE or MANIPULATE_RECTANGLE mode conditional
}

// --------------------------------------------------------------------------
bool RectanglePickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside RPH::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_RECTANGLE ||
       curr_state==ModeController::MANIPULATE_RECTANGLE)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==2)
      {
         Rectangle* Rectangle_ptr=get_rectangle_ptr();
         if (Rectangle_ptr != NULL)
         {
            threevector UVW;
            Rectangle_ptr->get_UVW_coords(
               get_GraphicalsGroup_ptr()->get_curr_t(),get_passnumber(),UVW);
            Rectangle_ptr->reset_bbox(
               get_RectanglesGroup_ptr()->get_curr_t(),
               get_RectanglesGroup_ptr()->get_passnumber());
            return true;
         }
      }
   } // INSERT_RECTANGLE or MANIPULATE_RECTANGLE mode conditional
   return false;
}

// --------------------------------------------------------------------------
bool RectanglePickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_RECTANGLE)
   {
      if (GraphicalPickHandler::rotate(oldX,oldY,ea) && get_ndims()==2)
      {
         if (get_rectangle_ptr() != NULL)
         {
            return true;
         }
      }
   } // MANIPULATE_RECTANGLE mode conditional
   return false;
}

// --------------------------------------------------------------------------
bool RectanglePickHandler::release()
{
//   cout << "inside RPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_RECTANGLE ||
       curr_state==ModeController::MANIPULATE_RECTANGLE)
   {
      get_RectanglesGroup_ptr()->reset_colors();
      return true;
   }
   else
   {
      return false;
   } // INSERT_RECTANGLE or MANIPULATE_RECTANGLE mode conditional
}

// --------------------------------------------------------------------------
bool RectanglePickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_RECTANGLE)
   {
      rotation_mode = !rotation_mode;
      scaling_mode=false;
      if (rotation_mode)
      {
         cout << "Rectangle rotation mode toggled on" << endl;
      }
      else
      {
         cout << "Rectangle rotation mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Rectangle generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_Rectangle creates a new Rectangle, assigns it a
// unique ID, sets the selected_Rectangle_number equal to that ID and
// adds it to the OSG Rectangle group.

bool RectanglePickHandler::instantiate_Rectangle()
{   
//   cout << "inside RectanglePickHandler::instantiate_Rectangle()" << endl;
   Rectangle* curr_Rectangle_ptr=get_RectanglesGroup_ptr()->
      generate_new_Rectangle();
   curr_Rectangle_ptr->set_canonical_local_vertices();
   instantiate_Graphical(curr_Rectangle_ptr);

   curr_Rectangle_ptr->reset_bbox(
      get_RectanglesGroup_ptr()->get_curr_t(),
      get_RectanglesGroup_ptr()->get_passnumber());

   return true;
}

// --------------------------------------------------------------------------
// Method select_Rectangle assigns selected_Rectangle_number equal to the ID
// of an existing cross hair which lies sufficiently close to a point
// picked by the user with his mouse.  If no Rectangle is nearby the
// selected point, selected_Rectangle_number is set equal to -1, and all
// Rectangles are effectively de-selected.

bool RectanglePickHandler::select_Rectangle()
{   
   int Rectangle_ID=select_Graphical();
   get_RectanglesGroup_ptr()->reset_colors();

   if (Rectangle_ID > -1)
   {
      Rectangle* curr_Rectangle_ptr=get_RectanglesGroup_ptr()->
         get_selected_Rectangle_ptr();
      curr_Rectangle_ptr->reset_bbox(
         get_RectanglesGroup_ptr()->get_curr_t(),
         get_RectanglesGroup_ptr()->get_passnumber());
      return true;
   }
   else
   {
      return false;
   }
}
