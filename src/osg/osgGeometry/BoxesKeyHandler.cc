// ==========================================================================
// BoxesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 12/7/05; 4/22/06
// ==========================================================================

#include "osg/osgGeometry/BoxesGroup.h"
#include "osg/osgGeometry/BoxesKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void BoxesKeyHandler::allocate_member_objects()
{
}

void BoxesKeyHandler::initialize_member_objects()
{
}

BoxesKeyHandler::BoxesKeyHandler(BoxesGroup* BG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(BG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   BoxesGroup_ptr=BG_ptr;
}

BoxesKeyHandler::~BoxesKeyHandler()
{
}

// ---------------------------------------------------------------------
BoxesGroup* const BoxesKeyHandler::get_BoxesGroup_ptr()
{
   return BoxesGroup_ptr;
}

// ------------------------------------------------------
bool BoxesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

/*
      if (get_ModeController_ptr()->getState()==
          ModeController::INSERT_BOX ||
          get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_BOX)
      {
      }
*/
    
// ......................................................................

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_BOX)
      {

// Press "s" to save Box information to ascii text file:

         if (ea.getKey()=='s')
         {
            get_BoxesGroup_ptr()->save_info_to_file();
            return true;
         }

// Press "Delete" key to completely destroy a Box:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_GraphicalsGroup_ptr()->destroy_Graphical();
            return true;
         }

// Press "r" to restore Box information from ascii text file:
      
//         else if (ea.getKey()=='r')
//         {
//            get_BoxesController_ptr()->read_Boxes();
//            return true;
//         }

// Press "c" to change box's color:

         else if (ea.getKey()=='c')
         {
            get_BoxesGroup_ptr()->change_color();
            return true;
         }

/*
// Press ">" ["<"] key to increase [decrease] a Box's size:

         else if (ea.getKey()=='>')
         {
            get_BoxesGroup_ptr()->change_size(2.0);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            get_BoxesGroup_ptr()->change_size(0.5);
            return true;
         }
*/

// Press "Up arrow" or "Down arrow" to move a selected Box up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            get_BoxesGroup_ptr()->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            get_BoxesGroup_ptr()->move_z(-1);
            return true;
         }
         
      } // mode = MANIPULATE_BOX conditional
   } // key down conditional
   
   return false;
}


