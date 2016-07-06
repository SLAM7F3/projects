// ==========================================================================(5
// ClippingKeyHandler class member function definitions
// ==========================================================================
// Last modified on 2/28/12
// ==========================================================================

#include "osg/osgClipping/Clipping.h"
#include "osg/osgClipping/ClippingKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ClippingKeyHandler::allocate_member_objects()
{
}

void ClippingKeyHandler::initialize_member_objects()
{
}

ClippingKeyHandler::ClippingKeyHandler(
   Clipping* C_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(NULL,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Clipping_ptr=C_ptr;
}

ClippingKeyHandler::~ClippingKeyHandler()
{
}

// ------------------------------------------------------
bool ClippingKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
      ModeController::MANIPULATE_FUSED_DATA)
      {
         if (ea.getKey()=='b')
         {
            Clipping_ptr->backproject_OBSFRUSTA_colorings(2);
//            Clipping_ptr->backproject_OBSFRUSTA_colorings(5);
//            Clipping_ptr->backproject_OBSFRUSTA_colorings(10);
//            Clipping_ptr->backproject_OBSFRUSTA_colorings(20);
//            Clipping_ptr->set_clipping_OBSFRUSTUM_ptr(1);
//            Clipping_ptr->test_clipping();

//            Clipping_ptr->clip_PolyLines();
            return true;
         }

         else if (ea.getKey()=='i')
         {
            Clipping_ptr->import_Building_colors();
//            Clipping_ptr->destroy_all_PolyLines();
         }
         

      } // getState conditional
   } // key down conditional
   
   return false;
}


