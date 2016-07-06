// ==========================================================================
// SphereSegmentPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 6/16/07; 9/2/08; 12/2/11
// ==========================================================================

#include <iostream>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgAnnotators/SphereSegmentsGroup.h"
#include "osg/osgAnnotators/SphereSegmentPickHandler.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SphereSegmentPickHandler::allocate_member_objects()
{
}		       

void SphereSegmentPickHandler::initialize_member_objects()
{
}		       

SphereSegmentPickHandler::SphereSegmentPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,SphereSegmentsGroup* SSG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr,double radius,
   double az_min,double az_max,double el_min,double el_max,
   bool display_spokes_flag,bool include_blast_flag):
   GeometricalPickHandler(
      3,PI_ptr,CM_ptr,SSG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   SphereSegmentsGroup_ptr=SSG_ptr;
   SphereSegmentRadius=radius;
   this->display_spokes_flag=display_spokes_flag;
   this->include_blast_flag=include_blast_flag;
   this->az_min=az_min;
   this->az_max=az_max;
   this->el_min=el_min;
   this->el_max=el_max;
}

SphereSegmentPickHandler::~SphereSegmentPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool SphereSegmentPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//    cout << "inside SphereSegmentPickHandler::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_HEMISPHERE ||
       curr_state==ModeController::MANIPULATE_HEMISPHERE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_HEMISPHERE, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D SphereSegment whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_HEMISPHERE)
         {
            return instantiate_SphereSegment(ea.getX(),ea.getY());
         }
         else
         {
            return select_SphereSegment();
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
   } // INSERT_HEMISPHERE or MANIPULATE_HEMISPHERE mode conditional
}

// --------------------------------------------------------------------------
bool SphereSegmentPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside SphereSegmentPickHandler::drag()" << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_HEMISPHERE ||
       curr_state==ModeController::MANIPULATE_HEMISPHERE)
   {
      return (GraphicalPickHandler::drag(ea) && get_ndims()==3);
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool SphereSegmentPickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_HEMISPHERE)
   {
      rotation_mode = !rotation_mode;
      if (rotation_mode)
      {
         cout << "SphereSegment rotation mode toggled on" << endl;
      }
      else
      {
         cout << "SphereSegment rotation mode toggled off" << endl;
      }
      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool SphereSegmentPickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_HEMISPHERE)
   {
      return GraphicalPickHandler::rotate(oldX,oldY,ea);
   }
   else
   {
      return false;
   } // MANIPULATE_HEMISPHERE mode conditional
}

// --------------------------------------------------------------------------
bool SphereSegmentPickHandler::release()
{
//   cout << "inside SPPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_HEMISPHERE ||
       curr_state==ModeController::MANIPULATE_HEMISPHERE)
   {
      SphereSegmentsGroup_ptr->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// SphereSegment generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_SphereSegment creates a new SphereSegment, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_SphereSegment_number equal to that ID and adds it to
// the OSG SphereSegment group.

bool SphereSegmentPickHandler::instantiate_SphereSegment(double X,double Y)
{   
   cout << "inside SphereSegmentPickHandler::instantiate_SphereSegment()"
        << endl;
   
   bool spheresegment_instantiated_flag=false;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      SphereSegment* curr_SphereSegment_ptr=SphereSegmentsGroup_ptr->
         generate_new_SphereSegment(
            SphereSegmentRadius,az_min,az_max,el_min,el_max);
      instantiate_Graphical(curr_SphereSegment_ptr);

      spheresegment_instantiated_flag=true;
   }
   return spheresegment_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_SphereSegment assigns selected_SphereSegment_number
// equal to the ID of an existing cross hair which lies sufficiently
// close to a point picked by the user with his mouse.  If no
// SphereSegment is nearby the selected point,
// selected_SphereSegment_number is set equal to -1, and all
// SphereSegments are effectively de-selected.

bool SphereSegmentPickHandler::select_SphereSegment()
{   
   int selected_SphereSegment_ID=select_Graphical();
   SphereSegmentsGroup_ptr->reset_colors();
   return (selected_SphereSegment_ID > -1);
}
