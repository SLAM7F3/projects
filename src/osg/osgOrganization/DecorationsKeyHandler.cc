// ==========================================================================
// DecorationsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 12/21/07
// ==========================================================================

#include "osg/osgOrganization/Decorations.h"
#include "osg/osgOrganization/DecorationsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void DecorationsKeyHandler::allocate_member_objects()
{
}

void DecorationsKeyHandler::initialize_member_objects()
{
}

DecorationsKeyHandler::DecorationsKeyHandler(
   Decorations* D_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(NULL,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Decorations_ptr=D_ptr;
}

DecorationsKeyHandler::~DecorationsKeyHandler()
{
}

// ------------------------------------------------------
bool DecorationsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==ModeController::VIEW_DATA)
      {

// Press "o" to write out IVE file:

         if (ea.getKey()=='o')
         {
            Decorations_ptr->write_IVE_file();
            return true;
         }

      } // getState conditional
   } // key down conditional
   
   return false;
}


