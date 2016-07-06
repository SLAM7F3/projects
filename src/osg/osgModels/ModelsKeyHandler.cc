// ==========================================================================
// ModelsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 10/31/06; 11/1/06; 2/21/07
// ==========================================================================

#include "osg/ModeController.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgModels/ModelsKeyHandler.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ModelsKeyHandler::allocate_member_objects()
{
}

void ModelsKeyHandler::initialize_member_objects()
{
   ModelsGroup_ptr=NULL;
}

ModelsKeyHandler::ModelsKeyHandler(
   ModelsGroup* MG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(MG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModelsGroup_ptr=MG_ptr;
}

ModelsKeyHandler::~ModelsKeyHandler()
{
}

// ------------------------------------------------------
bool ModelsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_MODEL)
      {

// Press "Delete" key to completely destroy a Model:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            ModelsGroup_ptr->destroy_Graphical();
            return true;
         }

// Press ">" ["<"] key to increase [decrease] a Model's size:

         else if (ea.getKey()=='>')
         {
            ModelsGroup_ptr->change_scale(1.5);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            ModelsGroup_ptr->change_scale(0.666);
            return true;
         }

// Press "Up arrow" or "Down arrow" to move a selected Model up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            ModelsGroup_ptr->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            ModelsGroup_ptr->move_z(-1);
            return true;
         }

         else if (ea.getKey()=='p')
         {
            ModelsGroup_ptr->record_waypoint();
         }
         else if (ea.getKey()=='f')
         {
            ModelsGroup_ptr->finish_waypoint_entry();
         }
         else if (ea.getKey()=='m')
         {
            ModelsGroup_ptr->unmask_next_model();
         }
         
      } // mode = MANIPULATE_MODEL conditional
   } // key down conditional
   
   return false;
}


