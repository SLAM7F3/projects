// ==========================================================================
// PowerPointsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 8/24/07
// ==========================================================================

#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osgAnnotators/PowerPointsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PowerPointsKeyHandler::allocate_member_objects()
{
}

void PowerPointsKeyHandler::initialize_member_objects()
{
}

PowerPointsKeyHandler::PowerPointsKeyHandler(
   PowerPointsGroup* PPG_ptr,ModeController* MC_ptr):
   BoxesKeyHandler(PPG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PowerPointsGroup_ptr=PPG_ptr;
}

PowerPointsKeyHandler::~PowerPointsKeyHandler()
{
}

// ---------------------------------------------------------------------
PowerPointsGroup* const PowerPointsKeyHandler::get_PowerPointsGroup_ptr()
{
   return PowerPointsGroup_ptr;
}

// ------------------------------------------------------
bool PowerPointsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_BOX)
      {

// Press "Delete" key to completely destroy an PowerPoint:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
//            get_PowerPointsGroup_ptr()->destroy_Graphical();
            get_PowerPointsGroup_ptr()->destroy_PowerPoint();
            return true;
         }

// Press "s" to save Box information to ascii text file:

//         else if (ea.getKey()=='s')
//         {
//            get_PowerPointsGroup_ptr()->save_info_to_file();
//            return true;
//         }

// Press "r" to restore Box information from ascii text file:
      
//         else if (ea.getKey()=='r')
//         {
//            get_PowerPointsController_ptr()->read_PowerPoints();
//            return true;
//         }

// Press "Up arrow" or "Down arrow" to move a selected Box up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            get_PowerPointsGroup_ptr()->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            get_PowerPointsGroup_ptr()->move_z(-1);
            return true;
         }
         
      } // mode = MANIPULATE_BOX conditional
   } // key down conditional
   
   return false;
}


