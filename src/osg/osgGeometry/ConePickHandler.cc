// ==========================================================================
// ConePickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 5/7/07; 6/16/07; 9/21/07; 6/15/08; 6/19/08; 12/4/10
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/osgGeometry/Cone.h"
#include "osg/osgGeometry/ConesGroup.h"
#include "osg/osgGeometry/ConePickHandler.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

#include "math/constant_vectors.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ConePickHandler::allocate_member_objects()
{
}		       

void ConePickHandler::initialize_member_objects()
{
}		       

ConePickHandler::ConePickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ConesGroup* CG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,CG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ConesGroup_ptr=CG_ptr;
}

ConePickHandler::~ConePickHandler() 
{
}

// ---------------------------------------------------------------------
ConesGroup* ConePickHandler::get_ConesGroup_ptr() 
{
   return ConesGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool ConePickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_CONE ||
       curr_state==ModeController::MANIPULATE_CONE)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_CONE, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Cone whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_CONE)
         {
            return instantiate_Cone(ea.getX(),ea.getY());
         }
         else
         {
            return select_Cone(ea.getX(),ea.getY());
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
   } // INSERT_CONE or MANIPULATE_CONE mode conditional
}

// --------------------------------------------------------------------------
bool ConePickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_CONE ||
       curr_state==ModeController::MANIPULATE_CONE)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
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
bool ConePickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_CONE)
   {
      rotation_mode = !rotation_mode;
      scaling_mode=false;
      if (rotation_mode)
      {
         cout << "Cone rotation mode toggled on" << endl;
      }
      else
      {
         cout << "Cone rotation mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool ConePickHandler::toggle_scaling_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_CONE)
   {
      scaling_mode=!scaling_mode;
      rotation_mode=false;
      if (scaling_mode)
      {
         cout << "Cone scaling mode toggled on" << endl;
      }
      else
      {
         cout << "Cone scaling mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool ConePickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_CONE)
   {
      return GraphicalPickHandler::rotate(oldX,oldY,ea);
   }
   else
   {
      return false;
   } // MANIPULATE_CONE mode conditional
}

// --------------------------------------------------------------------------
bool ConePickHandler::release()
{
//   cout << "inside CPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_CONE ||
       curr_state==ModeController::MANIPULATE_CONE)
   {
      get_ConesGroup_ptr()->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Cone generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_Cone creates a new Cone, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_Cone_number equal to that ID and adds it to
// the OSG Cone group.

bool ConePickHandler::instantiate_Cone(double X,double Y)
{   
//   cout << "inside CPH::instantiate_Cone()" << endl;
   bool cone_instantiated_flag=false;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      Cone* curr_Cone_ptr=get_ConesGroup_ptr()->generate_new_Cone();
      instantiate_Graphical(curr_Cone_ptr);

      double curr_t=get_ConesGroup_ptr()->get_curr_t();
      int passnumber=get_ConesGroup_ptr()->get_passnumber();

      curr_Cone_ptr->set_phi(0*PI/180);
      curr_Cone_ptr->set_theta(90*PI/180);
      curr_Cone_ptr->set_scale(threevector(1,1,10));
      threevector trans(0,0,0);
      curr_Cone_ptr->scale_rotate_and_then_translate(curr_t,passnumber,trans);

/*
      threevector tip(0,0,0);
      threevector base(10,10,10);
      double radius=3;
      get_ConesGroup_ptr()->transform_cone(curr_Cone_ptr,tip,base,radius);
*/

      cone_instantiated_flag=true;
   }
   return cone_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_Cone assigns selected_Cone_number equal to the ID of an
// existing Cone which lies sufficiently close to a point picked by the
// user with his mouse.  If no Cone is nearby the selected point,
// selected_Cone_number is set equal to -1, and all Cones are
// effectively de-selected.

bool ConePickHandler::select_Cone(double X,double Y)
{   
//   cout << "inside ConePickHandler::select_Cone()" << endl;

   select_Graphical();
   get_ConesGroup_ptr()->reset_colors();

   int Cone_ID=get_selected_Graphical_ID();
   if (Cone_ID >= 0)
   {
      Cone* curr_Cone_ptr=get_ConesGroup_ptr()->get_ID_labeled_Cone_ptr(
         Cone_ID);

      double t=get_ConesGroup_ptr()->get_curr_t();
      int passnumber=get_ConesGroup_ptr()->get_passnumber();
      threevector Cone_posn;
      curr_Cone_ptr->get_UVW_coords(t,passnumber,Cone_posn);
      Cone_posn  -= get_ConesGroup_ptr()->get_grid_world_origin();
   }

   return true;
}

// --------------------------------------------------------------------------
// This overridden version of method rotate_Graphical rotates a Cone
// about 2 independent directions.  The first corresponds to global
// +z_hat and leads to change in the cone's azimuth angle phi.  The
// second corresponds to the cone's instantaneous local +y_hat and
// leads to change in its elevation angle theta.  

bool ConePickHandler::rotate_Graphical()
{   
   cout << "inside CPH::rotate_Graphical()" << endl;
   
// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

   double curr_t=get_GraphicalsGroup_ptr()->get_curr_t();
   Cone* curr_Cone_ptr=get_ConesGroup_ptr()->get_ID_labeled_Cone_ptr(
      get_selected_Graphical_ID());

   if (curr_Cone_ptr != NULL)
   {
      threevector graphical_posn;
      curr_Cone_ptr->get_UVW_coords(
         curr_t,get_passnumber(),graphical_posn);

      threevector graphical_screenspace_posn(graphical_posn);

      graphical_screenspace_posn=get_CM_3D_ptr()->get_Transformer_ptr()->
         world_to_screen_transformation(graphical_posn);
      graphical_screenspace_posn.put(2,0);
//      cout << "graphical_screenspace_posn = "
//           << graphical_screenspace_posn << endl;

      threevector delta_screenspace_posn=curr_voxel_screenspace_posn-
         prev_voxel_screenspace_posn;
      double mag_factor=sqr(10*
         (curr_voxel_screenspace_posn-graphical_screenspace_posn).
         magnitude());
//      cout << "mag_factor = " << mag_factor << endl;

      double delta_az=mag_factor*delta_screenspace_posn.get(0)*PI/180;
      double delta_el=-mag_factor*delta_screenspace_posn.get(1)*PI/180;

      double new_phi=curr_Cone_ptr->get_phi()+delta_az;
      curr_Cone_ptr->set_phi(basic_math::phase_to_canonical_interval(
         new_phi,-PI,PI));
      double new_theta=curr_Cone_ptr->get_theta()+delta_el;
      new_theta=basic_math::max(0.0,new_theta);
      new_theta=basic_math::min(PI,new_theta);
      curr_Cone_ptr->set_theta(new_theta);
      
      curr_Cone_ptr->scale_rotate_and_then_translate(
         curr_t,get_passnumber(),Zero_vector);
   } // curr_Cone_ptr != NULL conditional
   return false;
}
