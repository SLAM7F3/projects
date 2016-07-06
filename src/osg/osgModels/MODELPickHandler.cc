// ==========================================================================
// MODELPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 1/21/07; 6/16/07; 7/20/07; 9/1/08; 9/2/08
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgModels/MODELPickHandler.h"
#include "osg/PickHandlerCallbacks.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MODELPickHandler::allocate_member_objects()
{
}		       

void MODELPickHandler::initialize_member_objects()
{
}		       

MODELPickHandler::MODELPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,MODELSGROUP* MG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,MG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   MODELSGROUP_ptr=MG_ptr;
}

MODELPickHandler::~MODELPickHandler() 
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void MODELPickHandler::set_PickHandlerCallbacks_ptr(
PickHandlerCallbacks* PHCB_ptr)
{
//   cout << "inside MODELPickHandler::set_PickHandlerCallbacks_ptr()" << endl;
   PickHandlerCallbacks_ptr=PHCB_ptr;
//   cout << "PickHandlerCallbacks_ptr = " << PickHandlerCallbacks_ptr << endl;
}

// --------------------------------------------------------------------------
float MODELPickHandler::get_max_distance_to_Graphical()
{
//   cout << "inside MODELPickHandler::get_max_distance_to_Graphical()" << endl;
   double scale_factor=10.0;
   double max_dist=0.2/scale_factor;
   return max_dist;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool MODELPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//    cout << "inside SPPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_MODEL ||
       curr_state==ModeController::MANIPULATE_MODEL)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_MODEL, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Model whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_MODEL)
         {
            select_Zplane_value();
            return pick_3D_point(ea.getX(),ea.getY());
         }
         else
         {
            return select_MODEL();
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
   } // INSERT_MODEL or MANIPULATE_MODEL mode conditional
}

// --------------------------------------------------------------------------
bool MODELPickHandler::pick_3D_point(float rx,float ry)
{

   if (GraphicalPickHandler::pick_point_on_Zplane(rx,ry,Zplane_value))
   {
      
// Instantiate a Model at the picked 3D point's location:

      return instantiate_Model();
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool MODELPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside SPPH::drag()" << endl;
   ModeController::eState curr_state=get_ModeController_ptr()->getState();

   bool drag_handled=false;
   if (curr_state==ModeController::INSERT_MODEL ||
       curr_state==ModeController::MANIPULATE_MODEL)
   {
      if (get_ndims()==3)
      {
         drag_handled=GraphicalPickHandler::drag(ea);
      } // ndims==3 conditional
   } // INSERT_MODEL or MANIPULATE_MODEL mode conditional
   return drag_handled;
}

// --------------------------------------------------------------------------
bool MODELPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside MODELPickHandler::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

// As of 9/1/08, we experiment with allowing Bluegrass application
// users to double click on UAV models in RUN_MOVIE mode:

   if (curr_state==ModeController::MANIPULATE_MODEL ||
       curr_state==ModeController::RUN_MOVIE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {
         return select_MODEL();
      }
   }

   return false;
}

// --------------------------------------------------------------------------
bool MODELPickHandler::release()
{
//   cout << "inside SPPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_MODEL ||
       curr_state==ModeController::MANIPULATE_MODEL)
   {
//      MODELSGROUP_ptr->reset_Model_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Model generation, manipulation and annihilation methods
// ==========================================================================

double MODELPickHandler::select_Zplane_value()
{
//   string banner="Enter Zplane value for model to be instantiated:";
//   outputfunc::write_banner(banner);
//   cin >> Zplane_value;

   Zplane_value=500;
   return Zplane_value;
}

// --------------------------------------------------------------------------
// Method instantiate_Model creates a new Model, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_Model_number equal to that ID and adds it to
// the OSG Model group.

bool MODELPickHandler::instantiate_Model()
{   
//    MODEL* curr_Model_ptr=
      MODELSGROUP_ptr->generate_new_Model();
   return true;
}

// --------------------------------------------------------------------------
// Method select_MODEL assigns selected_Model_number equal to the ID
// of an existing cross hair which lies sufficiently close to a point
// picked by the user with his mouse.  If no Model is nearby the
// selected point, selected_Model_number is set equal to -1, and all
// Models are effectively de-selected.

bool MODELPickHandler::select_MODEL()
{   
//   cout << "inside MODELPickHandler::select_MODEL()" << endl;
   int selected_MODEL_ID=select_Graphical();
//   cout << "selected_MODEL_ID = " << selected_MODEL_ID << endl;
   return select_MODEL(selected_MODEL_ID);
}

bool MODELPickHandler::select_MODEL(int selected_MODEL_ID)
{   
//   cout << "inside MODELPickHandler::select_MODEL(), selected ID = "
//        << selected_MODEL_ID << endl;
//   MODELSGROUP_ptr->issue_selection_message();

   MODEL* selected_MODEL_ptr=MODELSGROUP_ptr->
      get_ID_labeled_MODEL_ptr(selected_MODEL_ID);
      
   if (selected_MODEL_ptr != NULL)
   {
      track* track_ptr=selected_MODEL_ptr->get_track_ptr();
      if (track_ptr != NULL)
      {
//         if (PickHandlerCallbacks_ptr != NULL)
//         {
//            string track_label=track_ptr->get_label();
//            PickHandlerCallbacks_ptr->display_selected_vehicle_webpage(
//               track_label);
//         } // PickHandlerCallbacks_ptr != NULL

         osgGA::Terrain_Manipulator* TM_ptr=
            dynamic_cast<osgGA::Terrain_Manipulator*>(
               get_CustomManipulator_ptr());

// Translate and rotate virtual camera so that it's located directly
// above the selected MODEL while keeping its altitude fixed:

         threevector final_posn;
         selected_MODEL_ptr->get_UVW_coords(
            MODELSGROUP_ptr->get_curr_t(),
            MODELSGROUP_ptr->get_passnumber(),final_posn);
         final_posn.put(2,TM_ptr->get_eye_world_posn().get(2));
      
         genmatrix final_R(3,3);
         final_R.identity();
         TM_ptr->jumpto(final_posn,final_R);
            
      } // track_ptr != NULL conditional
   } // selectedMODEL_ptr != NULL conditional

   return (selected_MODEL_ID > -1);
}
