// ==========================================================================
// ModelPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 1/21/07; 6/16/07; 9/2/08
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/CustomManipulator.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgModels/Model.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgModels/ModelPickHandler.h"
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

void ModelPickHandler::allocate_member_objects()
{
}		       

void ModelPickHandler::initialize_member_objects()
{
}		       

ModelPickHandler::ModelPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ModelsGroup* MG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,MG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModelsGroup_ptr=MG_ptr;
}

ModelPickHandler::~ModelPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool ModelPickHandler::pick(const osgGA::GUIEventAdapter& ea)
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
            return select_Model();
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
bool ModelPickHandler::pick_3D_point(float rx,float ry)
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
bool ModelPickHandler::drag(const osgGA::GUIEventAdapter& ea)
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
bool ModelPickHandler::release()
{
//   cout << "inside SPPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_MODEL ||
       curr_state==ModeController::MANIPULATE_MODEL)
   {
//      ModelsGroup_ptr->reset_Model_colors();
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

double ModelPickHandler::select_Zplane_value()
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

bool ModelPickHandler::instantiate_Model()
{   
//    Model* curr_Model_ptr=
      ModelsGroup_ptr->generate_new_Model();
   return true;
}

// --------------------------------------------------------------------------
// Method select_Model assigns selected_Model_number equal to the ID
// of an existing cross hair which lies sufficiently close to a point
// picked by the user with his mouse.  If no Model is nearby the
// selected point, selected_Model_number is set equal to -1, and all
// Models are effectively de-selected.

bool ModelPickHandler::select_Model()
{   
   int selected_Model_ID=select_Graphical();
//   ModelsGroup_ptr->reset_Model_colors();
   return (selected_Model_ID > -1);
}

