// ==========================================================================
// PyramidPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 7/21/07; 9/21/07; 6/15/08; 9/2/08; 12/4/10
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include "osg/osgGeometry/Pyramid.h"
#include "osg/osgGeometry/PyramidsGroup.h"
#include "osg/osgGeometry/PyramidPickHandler.h"
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

void PyramidPickHandler::allocate_member_objects()
{
}		       

void PyramidPickHandler::initialize_member_objects()
{
}		       

PyramidPickHandler::PyramidPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PyramidsGroup* PG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,PG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PyramidsGroup_ptr=PG_ptr;
}

PyramidPickHandler::~PyramidPickHandler() 
{
}

// ---------------------------------------------------------------------
PyramidsGroup* PyramidPickHandler::get_PyramidsGroup_ptr() 
{
   return PyramidsGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool PyramidPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_PYRAMID ||
       curr_state==ModeController::MANIPULATE_PYRAMID)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_PYRAMID, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Pyramid whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_PYRAMID)
         {
            return instantiate_Pyramid(ea.getX(),ea.getY());
         }
         else
         {
            return select_Pyramid(ea.getX(),ea.getY());
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
   } // INSERT_PYRAMID or MANIPULATE_PYRAMID mode conditional
}

// --------------------------------------------------------------------------
bool PyramidPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside CPH::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_PYRAMID ||
       curr_state==ModeController::MANIPULATE_PYRAMID)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
         Pyramid* curr_Pyramid_ptr=get_PyramidsGroup_ptr()->
            get_ID_labeled_Pyramid_ptr(get_selected_Graphical_ID());
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
bool PyramidPickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_PYRAMID)
   {
      rotation_mode = !rotation_mode;
      scaling_mode=false;
      if (rotation_mode)
      {
         cout << "Pyramid rotation mode toggled on" << endl;
      }
      else
      {
         cout << "Pyramid rotation mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool PyramidPickHandler::toggle_scaling_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_PYRAMID)
   {
      scaling_mode=!scaling_mode;
      rotation_mode=false;
      if (scaling_mode)
      {
         cout << "Pyramid scaling mode toggled on" << endl;
      }
      else
      {
         cout << "Pyramid scaling mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool PyramidPickHandler::rotate(const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_PYRAMID)
   {
      return GraphicalPickHandler::rotate(ea);
   }
   else
   {
      return false;
   } // MANIPULATE_PYRAMID mode conditional
}

// --------------------------------------------------------------------------
bool PyramidPickHandler::release()
{
//   cout << "inside CPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_PYRAMID ||
       curr_state==ModeController::MANIPULATE_PYRAMID)
   {
      get_PyramidsGroup_ptr()->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Pyramid generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_Pyramid creates a new Pyramid, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_Pyramid_number equal to that ID and adds it to
// the OSG Pyramid group.

bool PyramidPickHandler::instantiate_Pyramid(double X,double Y)
{   
//   cout << "inside CPH::instantiate_Pyramid()" << endl;
   bool pyramid_instantiated_flag=false;
   if (GraphicalPickHandler::pick_3D_point(X,Y))
   {
      Pyramid* curr_Pyramid_ptr=get_PyramidsGroup_ptr()->
         generate_new_Pyramid();
      instantiate_Graphical(curr_Pyramid_ptr);

      double curr_t=get_PyramidsGroup_ptr()->get_curr_t();
      int passnumber=get_PyramidsGroup_ptr()->get_passnumber();

      curr_Pyramid_ptr->set_phi(0*PI/180);
      curr_Pyramid_ptr->set_theta(90*PI/180);
      curr_Pyramid_ptr->set_scale(threevector(1,1,10));
      threevector trans(0,0,0);
      curr_Pyramid_ptr->scale_rotate_and_then_translate(
         curr_t,passnumber,trans);


      pyramid_instantiated_flag=true;
   }
   return pyramid_instantiated_flag;
}

// --------------------------------------------------------------------------
// Method select_Pyramid assigns selected_Pyramid_number equal to the ID of an
// existing Pyramid which lies sufficiently close to a point picked by the
// user with his mouse.  If no Pyramid is nearby the selected point,
// selected_Pyramid_number is set equal to -1, and all Pyramids are
// effectively de-selected.

bool PyramidPickHandler::select_Pyramid(double X,double Y)
{   
   select_Graphical();
   get_PyramidsGroup_ptr()->reset_colors();

   int Pyramid_ID=get_selected_Graphical_ID();
   if (Pyramid_ID >= 0)
   {
      Pyramid* curr_Pyramid_ptr=get_PyramidsGroup_ptr()->
         get_ID_labeled_Pyramid_ptr(Pyramid_ID);

      double t=get_PyramidsGroup_ptr()->get_curr_t();
      int passnumber=get_PyramidsGroup_ptr()->get_passnumber();
      threevector Pyramid_posn;
      curr_Pyramid_ptr->get_UVW_coords(t,passnumber,Pyramid_posn);
      Pyramid_posn  -= get_PyramidsGroup_ptr()->get_grid_world_origin();
   }

   return (Pyramid_ID > -1 );
}

// --------------------------------------------------------------------------
// This overridden version of method rotate_Graphical rotates a Pyramid
// about 2 independent directions.  The first corresponds to global
// +z_hat and leads to change in the pyramid's azimuth angle phi.  The
// second corresponds to the pyramid's instantaneous local +y_hat and
// leads to change in its elevation angle theta.  

bool PyramidPickHandler::rotate_Graphical()
{   
   cout << "inside PPH::rotate_Graphical()" << endl;
   
// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

   double curr_t=get_GraphicalsGroup_ptr()->get_curr_t();
   Pyramid* curr_Pyramid_ptr=get_PyramidsGroup_ptr()->
      get_ID_labeled_Pyramid_ptr(get_selected_Graphical_ID());

   if (curr_Pyramid_ptr != NULL)
   {
      threevector graphical_posn;
      curr_Pyramid_ptr->get_UVW_coords(
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

      double new_phi=curr_Pyramid_ptr->get_phi()+delta_az;
      curr_Pyramid_ptr->set_phi(basic_math::phase_to_canonical_interval(
         new_phi,-PI,PI));
      double new_theta=curr_Pyramid_ptr->get_theta()+delta_el;
      new_theta=basic_math::max(0.0,new_theta);
      new_theta=basic_math::min(PI,new_theta);
      curr_Pyramid_ptr->set_theta(new_theta);
      
      curr_Pyramid_ptr->scale_rotate_and_then_translate(
         curr_t,get_passnumber(),Zero_vector);
   } // curr_Pyramid_ptr != NULL conditional
   return false;
}
