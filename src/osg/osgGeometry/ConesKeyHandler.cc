// ==========================================================================
// ConesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 9/20/06; 11/28/06; 1/21/07
// ==========================================================================

#include "osg/osgGeometry/ConesGroup.h"
#include "osg/osgGeometry/ConesKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ConesKeyHandler::allocate_member_objects()
{
}

void ConesKeyHandler::initialize_member_objects()
{
}

ConesKeyHandler::ConesKeyHandler(ConesGroup* CG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(CG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ConesGroup_ptr=CG_ptr;
}

ConesKeyHandler::~ConesKeyHandler()
{
}

// ---------------------------------------------------------------------
ConesGroup* const ConesKeyHandler::get_ConesGroup_ptr()
{
   return ConesGroup_ptr;
}

// ------------------------------------------------------
bool ConesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

/*
      if (get_ModeController_ptr()->getState()==
          ModeController::INSERT_CONE ||
          get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_CONE)
      {
      }
*/
    
// ......................................................................

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_CONE)
      {

// Press "s" to save Cone information to ascii text file:

/*
         if (ea.getKey()=='s')
         {
            get_ConesGroup_ptr()->save_info_to_file();
            return true;
         }
*/


// Press "Delete" key to completely destroy a Cone:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_GraphicalsGroup_ptr()->destroy_Graphical();
            return true;
         }

// Press "r" to restore Cone information from ascii text file:
      
//         else if (ea.getKey()=='r')
//         {
//            get_ConesController_ptr()->read_Cones();
//            return true;
//         }

// Press "c" to change cone's color:

         else if (ea.getKey()=='c')
         {
            get_ConesGroup_ptr()->change_color();
            return true;
         }

// Press "f" to draw cone representing view frustum:

         else if (ea.getKey()=='f')
         {
            get_ConesGroup_ptr()->draw_FOV_cone();
            return true;
         }

/*
// Press ">" ["<"] key to increase [decrease] a Cone's size:

         else if (ea.getKey()=='>')
         {
            get_ConesGroup_ptr()->change_size(2.0);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            get_ConesGroup_ptr()->change_size(0.5);
            return true;
         }
*/

// Press "Up arrow" or "Down arrow" to move a selected Cone up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            get_ConesGroup_ptr()->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            get_ConesGroup_ptr()->move_z(-1);
            return true;
         }
         
      } // mode = MANIPULATE_CONE conditional
   } // key down conditional
   
   return false;
}


