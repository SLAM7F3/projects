// ==========================================================================
// FishnetsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 11/17/11; 12/5/11; 12/6/11
// ==========================================================================

#include "osg/ModeController.h"
#include "osg/osg3D/FishnetsGroup.h"
#include "osg/osg3D/FishnetsKeyHandler.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FishnetsKeyHandler::allocate_member_objects()
{
}

void FishnetsKeyHandler::initialize_member_objects()
{
}

FishnetsKeyHandler::FishnetsKeyHandler(
   FishnetsGroup* FG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(FG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   FishnetsGroup_ptr=FG_ptr;
}

FishnetsKeyHandler::~FishnetsKeyHandler()
{
}

// ------------------------------------------------------
bool FishnetsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_FISHNET)
      {
         if (ea.getKey()=='e')
         {
            FishnetsGroup_ptr->get_Fishnet_ptr(0)->
               export_inverted_ground_points();
            FishnetsGroup_ptr->get_Fishnet_ptr(0)->
               export_inverted_ground_surface();
//            FishnetsGroup_ptr->get_Fishnet_ptr(0)->
//               export_inverted_ground_twoDarray();
            return true;
         }
         else if (ea.getKey()=='g')
         {
            FishnetsGroup_ptr->compute_initial_surface_energy();
            return true;
         }
         else if (ea.getKey()=='l')
         {
            FishnetsGroup_ptr->get_Fishnet_ptr(0)->
               identify_masses_close_to_pointcloud();
            return true;
         }
         else if (ea.getKey()=='r')
         {
            FishnetsGroup_ptr->relax_ground_surface();
//            FishnetsGroup_ptr->refine_Fishnet();
            return true;
         }
         else if (ea.getKey()=='t')
         {
            Fishnet* Fishnet_ptr=FishnetsGroup_ptr->get_Fishnet_ptr(0);
            Fishnet_ptr->toggle_PolyLines();
            return true;
         }

         
      } // mode = MANIPULATE_FISHNET_DATA conditional
   } // key down conditional
   
   return false;
}

