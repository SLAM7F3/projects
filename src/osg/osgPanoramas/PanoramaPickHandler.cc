// ==========================================================================
// PanoramaPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 2/24/11
// ==========================================================================

#include <iostream>
#include <vector>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "osg/osgPanoramas/PanoramaPickHandler.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PanoramaPickHandler::allocate_member_objects()
{
}		       

void PanoramaPickHandler::initialize_member_objects()
{
   mask_nonselected_OSGsubPATs_flag=true;
   disallow_Panorama_doubleclicking_flag=false;
   Grid_ptr=NULL;
}		       

PanoramaPickHandler::PanoramaPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PanoramasGroup* OFG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   GeometricalPickHandler(
      3,PI_ptr,CM_ptr,OFG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PanoramasGroup_ptr=OFG_ptr;
}

PanoramaPickHandler::~PanoramaPickHandler() 
{
//   cout << "inside PanoramaPickHandler destructor" << endl;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool PanoramaPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside OFPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// Select the 3D Panorama whose center lies closest to (X,Y) in
// screen space:

         return select_Panorama();
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   } // MANIPULATE_FUSED_DATA mode conditional
}

// --------------------------------------------------------------------------
bool PanoramaPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
   return false;
}

// --------------------------------------------------------------------------
bool PanoramaPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state != ModeController::MANIPULATE_FUSED_DATA) return false;

//   cout << "inside PanoramaPickHandler::doubleclick()" << endl;
//   cout << "disallow_Panorama_doubleclicking_flag = "
//        << disallow_Panorama_doubleclicking_flag << endl;

   if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
   {
      if (!disallow_Panorama_doubleclicking_flag && select_Panorama())
      {
         Panorama* next_Panorama_ptr=PanoramasGroup_ptr->
            get_ID_labeled_Panorama_ptr(get_selected_Graphical_ID());
         PanoramasGroup_ptr->move_from_curr_to_next_panorama(
            next_Panorama_ptr,x_hat);
//         cout << "Selected panorama ID = " 
//              << get_selected_Graphical_ID() << endl;
         return true;
      }
   }
   return false;
}

// --------------------------------------------------------------------------
bool PanoramaPickHandler::release()
{
//   cout << "inside OFPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_FUSED_DATA)
   {
      PanoramasGroup_ptr->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Panorama generation, manipulation and annihilation methods
// ==========================================================================

// Method select_Panorama 

// If no Panorama is nearby the selected point,
// selected_Panorama_ID is set equal to -1, and all Panoramas
// are effectively de-selected.

bool PanoramaPickHandler::select_Panorama()
{   
//   cout << "inside PanoramaPickHandler::select_Panorama()" << endl;

   int selected_Panorama_ID=-1;
   double min_p_scrn_distance=0.1;

   double min_scrn_distance=POSITIVEINFINITY;
   threevector selected_Panorama_posn;
   for (unsigned int n=0; n<PanoramasGroup_ptr->get_n_Graphicals(); n++)
   {
      Panorama* Panorama_ptr=PanoramasGroup_ptr->get_Panorama_ptr(n);
//      cout << "n = " << n << " Panorama_ptr = " << Panorama_ptr << endl;
//      cout << "Panorama ID = " << Panorama_ptr->get_ID() << endl;

// First check whether *Panorama_ptr is masked.  If so, it should
// not be selected!

      bool mask_flag=Panorama_ptr->get_mask(
         PanoramasGroup_ptr->get_curr_t(),
         PanoramasGroup_ptr->get_passnumber());
      if (mask_flag) continue;

      threevector panorama_posn=Panorama_ptr->get_posn();
//      cout << "panorama_world_posn = " << panorama_posn << endl;

      threevector panorama_screen_posn=
         get_CM_3D_ptr()->get_Transformer_ptr()->
         world_to_screen_transformation(panorama_posn);
//      cout << "panorama_screen_posn = " << panorama_screen_posn << endl;
      
//      cout << "curr_voxel_screenspace_posn = "
//           << curr_voxel_screenspace_posn << endl;

      twovector delta_scrn(curr_voxel_screenspace_posn-
		           panorama_screen_posn);
      double curr_scrn_distance=delta_scrn.magnitude();

      if ( curr_scrn_distance < min_p_scrn_distance &&
           curr_scrn_distance < min_scrn_distance)
      {
         min_scrn_distance=curr_scrn_distance;
         selected_Panorama_ID=Panorama_ptr->get_ID();
         selected_Panorama_posn=panorama_posn;
      }
   } // loop over index n labeling OBSFRUSTA

   set_selected_Graphical_ID(selected_Panorama_ID);
//   cout << "Selected Panorama ID = " << selected_Panorama_ID << endl;
//   cout << "min_scrn_distance = " << min_scrn_distance << endl;
   
//   cout << "Selected OSGsubPAT = " 
//        << PanoramasGroup_ptr->get_selected_OSGsubPAT_ID()
//        << endl;

   return (selected_Panorama_ID > -1);
}
