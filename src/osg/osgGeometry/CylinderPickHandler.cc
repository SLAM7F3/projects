// ==========================================================================
// CylinderPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 6/15/08; 9/2/08; 2/9/11
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>

#include "math/constant_vectors.h"
#include "general/stringfuncs.h"
#include "osg/CustomManipulator.h"
#include "osg/osgGeometry/Cylinder.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgGeometry/CylinderPickHandler.h"
#include "osg/ModeController.h"
#include "osg/PickHandlerCallbacks.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CylinderPickHandler::allocate_member_objects()
{
}		       

void CylinderPickHandler::initialize_member_objects()
{
//    text_screen_axis_alignment_flag=true;
   text_size=1.0;
   text_displacement=1.0; // meter;

// Recall instantaneous color = red.  So choose some other color
// besides red as the permanent color:

   permanent_color=colorfunc::blue;
//   permanent_color=colorfunc::yellow;

   PickHandlerCallbacks_ptr=NULL;
}		       

CylinderPickHandler::CylinderPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,CylindersGroup* CG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,CG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   CylindersGroup_ptr=CG_ptr;
}

CylinderPickHandler::~CylinderPickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool CylinderPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CylinderPickHandler::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_CYLINDER ||
       curr_state==ModeController::MANIPULATE_CYLINDER)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_CYLINDER, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Cylinder whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_CYLINDER)
         {
            return instantiate_Cylinder(ea.getX(),ea.getY());
         }
         else
         {
            return select_Cylinder();
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
   } // INSERT_CYLINDER or MANIPULATE_CYLINDER mode conditional
}

// --------------------------------------------------------------------------
bool CylinderPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_CYLINDER ||
       curr_state==ModeController::MANIPULATE_CYLINDER)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
//         Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->
//            get_ID_labeled_Cylinder_ptr(get_selected_Graphical_ID());
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool CylinderPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CylinderPickHandler::doubleclick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();

// As of 4/27/08, we experiment with allowing Bluegrass application
// users to double click on cylinders representing vehicle tracks in
// RUN_MOVIE mode:

   if (curr_state==ModeController::MANIPULATE_CYLINDER ||
       curr_state==ModeController::RUN_MOVIE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {
         return select_Cylinder();
      }
   }

   return false;
}

// --------------------------------------------------------------------------
bool CylinderPickHandler::release()
{
//   cout << "inside CPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_CYLINDER ||
       curr_state==ModeController::MANIPULATE_CYLINDER)
   {
      CylindersGroup_ptr->reset_colors();

      double proximity_distance=15;	// meters
      CylindersGroup_ptr->generate_proximity_messages(proximity_distance);
      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool CylinderPickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_CYLINDER)
   {
      rotation_mode = !rotation_mode;
      scaling_mode=false;
      if (rotation_mode)
      {
         cout << "Cylinder rotation mode toggled on" << endl;
      }
      else
      {
         cout << "Cylinder rotation mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool CylinderPickHandler::toggle_scaling_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_CYLINDER)
   {
      scaling_mode=!scaling_mode;
      rotation_mode=false;
      if (scaling_mode)
      {
         cout << "Cylinder scaling mode toggled on" << endl;
      }
      else
      {
         cout << "Cylinder scaling mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool CylinderPickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_CYLINDER)
   {
      return GraphicalPickHandler::rotate(oldX,oldY,ea);
   }
   else
   {
      return false;
   } // MANIPULATE_CYLINDER mode conditional
}

// ==========================================================================
// Cylinder generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_Cylinder creates a new Cylinder, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_Cylinder_number equal to that ID and adds it to
// the OSG Cylinder group.

bool CylinderPickHandler::instantiate_Cylinder(double X,double Y)
{   
//   cout << "inside CPH::instantiate_Cylinder()" << endl;

   bool cylinder_instantiated_flag=false;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      const unsigned int n_text_messages=1;

      if (ParentVisitor_refptr->get_earth_flag())
      {
         text_displacement=50000;	// meters
         text_size=20000;
      }

      osg::Quat trivial_q(0,0,0,1);

      Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->
         generate_new_Cylinder(
            Zero_vector,trivial_q,permanent_color,
            n_text_messages,text_displacement,text_size);
      instantiate_Graphical(curr_Cylinder_ptr);

      if (ParentVisitor_refptr->get_earth_flag())
      {
         CylindersGroup_ptr->orient_cylinder_with_ellipsoid_radial_dir(
            curr_Cylinder_ptr);
      }

      string text_label=stringfunc::number_to_string(
         curr_Cylinder_ptr->get_ID());
      curr_Cylinder_ptr->set_text_label(0,text_label);

/*
  double theta=135*PI/180;
  double phi=45*PI/180;
  threevector scale(1,1,15);
  threevector trans(0,0,0);
  CylindersGroup_ptr->scale_rotate_and_then_translate_cylinder(
  curr_Cylinder_ptr,theta,phi,scale,trans);
*/

//   CylindersGroup_ptr->transform_cylinder(
// 	curr_Cylinder_ptr,tip,base,radius);
      cylinder_instantiated_flag=true;
   }

   return cylinder_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_Cylinder assigns selected_Cylinder_number equal to
// the ID of an existing Cylinder which lies sufficiently close to a
// point picked by the user with his mouse.  If no Cylinder is nearby
// the selected point, selected_Cylinder_number is set equal to -1,
// and all Cylinders are effectively de-selected.

bool CylinderPickHandler::select_Cylinder()
{   
   int selected_Cylinder_ID=select_Graphical();
   return select_Cylinder(selected_Cylinder_ID);
}

bool CylinderPickHandler::select_Cylinder(int selected_Cylinder_ID)
{   
//   cout << "inside CylinderPickHandler::select_Cylinder(), selected ID = "
//        << selected_Cylinder_ID << endl;

   CylindersGroup_ptr->reset_colors();
   CylindersGroup_ptr->issue_selection_message();

   Cylinder* selected_Cylinder_ptr=CylindersGroup_ptr->
      get_ID_labeled_Cylinder_ptr(selected_Cylinder_ID);
   
   if (selected_Cylinder_ptr != NULL)
   {
      track* vehicle_track_ptr=selected_Cylinder_ptr->get_track_ptr();

      if (vehicle_track_ptr != NULL)
      {
         if (PickHandlerCallbacks_ptr != NULL)
         {
            string vehicle_label=vehicle_track_ptr->get_label();
            PickHandlerCallbacks_ptr->display_selected_vehicle_webpage(
               vehicle_label);
         } // PickHandlerCallbacks_ptr != NULL

         osgGA::Terrain_Manipulator* TM_ptr=
            dynamic_cast<osgGA::Terrain_Manipulator*>(
               get_CustomManipulator_ptr());

// Translate and rotate virtual camera so that it's located directly
// above the selected Cylinder while keeping its altitude fixed:

         threevector final_posn;
         selected_Cylinder_ptr->get_UVW_coords(
            CylindersGroup_ptr->get_curr_t(),
            CylindersGroup_ptr->get_passnumber(),final_posn);
         final_posn.put(2,TM_ptr->get_eye_world_posn().get(2));
      
         genmatrix final_R(3,3);
         final_R.identity();
         TM_ptr->jumpto(final_posn,final_R);
            
      } // vehicle_track_ptr != NULL conditional
   } // selectedCylinder_ptr != NULL conditional
 
   return (selected_Cylinder_ID > -1);
}

// ==========================================================================
// Bluegrass application member functions
// ==========================================================================

void CylinderPickHandler::set_PickHandlerCallbacks_ptr(
PickHandlerCallbacks* PHCB_ptr)
{
//   cout << "inside CylinderPHandler::set_PickHandlerCallbacks_ptr()" 
//        << endl;
   PickHandlerCallbacks_ptr=PHCB_ptr;
//   cout << "PickHandlerCallbacks_ptr = " << PickHandlerCallbacks_ptr << endl;
}

