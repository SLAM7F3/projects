// ==========================================================================
// CentersKeyHandler class member function definitions
// ==========================================================================
// Last modified on 7/10/06; 12/8/06
// ==========================================================================

#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CentersKeyHandler::allocate_member_objects()
{
}

void CentersKeyHandler::initialize_member_objects()
{
}

CentersKeyHandler::CentersKeyHandler(
   const int p_ndims,CenterPickHandler* CPH_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(CPH_ptr->get_CentersGroup_ptr(),MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ndims=p_ndims;
   CenterPickHandler_ptr=CPH_ptr;
}

CentersKeyHandler::~CentersKeyHandler()
{
}

// ------------------------------------------------------
bool CentersKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_CENTER)
      {

// Press "e" to edit center location:

         if (ea.getKey()=='s')
         {
            CenterPickHandler_ptr->get_CentersGroup_ptr()->toggle_spin_flag();
         }
         else if (ea.getKey()=='>')
         {
            CenterPickHandler_ptr->get_CentersGroup_ptr()->increase_spin();
         }
         else if (ea.getKey()=='<')
         {
            CenterPickHandler_ptr->get_CentersGroup_ptr()->decrease_spin();
         }
         else if (ea.getKey()=='e')
         {
            CenterPickHandler_ptr->edit_center_location();
            return true;
         }

      } // mode = MANIPULATE_CENTER conditional         
      
   } // key down conditional
   
   return false;
}


