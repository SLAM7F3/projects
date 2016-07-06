// ==========================================================================
// BoxPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 6/19/08; 9/2/08; 2/9/11
// ==========================================================================

#include <iostream>
#include <set>
#include <osg/Group>
#include <osgGA/GUIEventAdapter>
#include <osg/Object>
#include <osg/StateSet>
#include "osg/osgGeometry/Box.h"
#include "osg/osgGeometry/BoxesGroup.h"
#include "osg/osgGeometry/BoxPickHandler.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "geometry/mybox.h"
#include "osg/Transformer.h"
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

void BoxPickHandler::allocate_member_objects()
{
}		       

void BoxPickHandler::initialize_member_objects()
{
}		       

BoxPickHandler::BoxPickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,BoxesGroup* BG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,
   threevector* GO_ptr):
   GeometricalPickHandler(3,PI_ptr,CM_ptr,BG_ptr,MC_ptr,WCC_ptr,GO_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   BoxesGroup_ptr=BG_ptr;
}

BoxPickHandler::~BoxPickHandler() 
{
}

// ---------------------------------------------------------------------
BoxesGroup* BoxPickHandler::get_BoxesGroup_ptr() 
{
   return BoxesGroup_ptr;
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool BoxPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside BPH::pick()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      if (GraphicalPickHandler::pick(ea) && get_ndims()==3)
      {

// If ModeController==INSERT_BOX, pick point within the 3D
// cloud whose screen-space coordinates lie closest to (X,Y).
// Otherwise, select the 3D Box whose center lies closest to
// (X,Y) in screen space:

         if (curr_state==ModeController::INSERT_BOX)
         {
            return instantiate_Box(ea.getX(),ea.getY());
         }
         else
         {
            return select_Box(ea.getX(),ea.getY());
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
   } // INSERT_BOX or MANIPULATE_BOX mode conditional
}

// --------------------------------------------------------------------------
bool BoxPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside BPH::drag()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      if (GraphicalPickHandler::drag(ea) && get_ndims()==3)
      {
         Box* curr_Box_ptr=get_BoxesGroup_ptr()->get_ID_labeled_Box_ptr(
            get_selected_Graphical_ID());
         get_BoxesGroup_ptr()->update_mybox(curr_Box_ptr);
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
bool BoxPickHandler::toggle_rotate_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_BOX)
   {
      rotation_mode = !rotation_mode;
      scaling_mode=false;
      if (rotation_mode)
      {
         cout << "Box rotation mode toggled on" << endl;
      }
      else
      {
         cout << "Box rotation mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool BoxPickHandler::toggle_scaling_mode()
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_BOX)
   {
      scaling_mode=!scaling_mode;
      rotation_mode=false;
      if (scaling_mode)
      {
         cout << "Box scaling mode toggled on" << endl;
      }
      else
      {
         cout << "Box scaling mode toggled off" << endl;
      }

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool BoxPickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_BOX)
   {
      return GraphicalPickHandler::rotate(oldX,oldY,ea);
   }
   else
   {
      return false;
   } // MANIPULATE_BOX mode conditional
}

// --------------------------------------------------------------------------
bool BoxPickHandler::scale(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::MANIPULATE_BOX)
   {
      return scale_Box(ea.getX(),ea.getY(),oldX,oldY);
   }
   else
   {
      return false;
   } // MANIPULATE_BOX mode conditional
}

// --------------------------------------------------------------------------
bool BoxPickHandler::release()
{
//   cout << "inside BPH::release()" << endl;

   ModeController::eState curr_state=get_ModeController_ptr()->getState();
   if (curr_state==ModeController::INSERT_BOX ||
       curr_state==ModeController::MANIPULATE_BOX)
   {
      get_BoxesGroup_ptr()->reset_colors();
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// Box generation, manipulation and annihilation methods
// ==========================================================================

// Method instantiate_Box creates a new Box, assigns it a
// unique ID, associates an attendant text label's value equal to the
// ID, sets the selected_Box_number equal to that ID and adds it to
// the OSG Box group.

bool BoxPickHandler::instantiate_Box(double X,double Y)
{   
//   cout << "inside BPH::instantiate_Box()" << endl;

   if (GraphicalPickHandler::pick_point_on_Zplane(
      X,Y,get_grid_origin().get(2)))
   {
      
// Instantiate a Box at the picked 3D point's location:

      Box* curr_Box_ptr=get_BoxesGroup_ptr()->generate_new_canonical_Box();
      instantiate_Graphical(curr_Box_ptr);

// Move instantiated box in the positive z direction by half its
// height so that its bottom just touches the world-grid:

      double t=get_BoxesGroup_ptr()->get_curr_t();
      int passnumber=get_BoxesGroup_ptr()->get_passnumber();
      threevector Box_posn;
      if (curr_Box_ptr->get_UVW_coords(t,passnumber,Box_posn))
      {
         Box_posn.put(2,Box_posn.get(2)+0.5*get_BoxesGroup_ptr()->
                      get_height());
         curr_Box_ptr->set_UVW_coords(t,passnumber,Box_posn);
      }
      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Method select_Box assigns selected_Box_number equal to the ID of an
// existing Box which lies sufficiently close to a point picked by the
// user with his mouse.  If no Box is nearby the selected point,
// selected_Box_number is set equal to -1, and all Boxes are
// effectively de-selected.

bool BoxPickHandler::select_Box(double X,double Y)
{   
   select_Graphical();
   get_BoxesGroup_ptr()->reset_colors();

// Renormalize ray's world-space base point relative to grid world
// origin before comparing to mybox coordinates:

   threevector ray_basepoint=get_CM_3D_ptr()->get_eye_world_posn();
   ray_basepoint -= get_BoxesGroup_ptr()->get_grid_world_origin();
   threevector r_hat=get_CM_3D_ptr()->get_Transformer_ptr()->
      compute_ray_into_screen(X,Y);

   get_BoxesGroup_ptr()->deselect_all_faces();
   int Box_ID=get_selected_Graphical_ID();
   if (Box_ID >= 0)
   {
      Box* curr_Box_ptr=get_BoxesGroup_ptr()->get_ID_labeled_Box_ptr(Box_ID);
      mybox* curr_mybox_ptr=curr_Box_ptr->get_mybox_ptr();

//      cout << "curr mybox = " << *curr_mybox_ptr << endl;
      int selected_face=curr_mybox_ptr->face_intercepted_by_ray(
         pair<threevector,threevector>(ray_basepoint,r_hat));
//      cout << "selected face number = " << selected_face << endl;

      double t=get_BoxesGroup_ptr()->get_curr_t();
      int passnumber=get_BoxesGroup_ptr()->get_passnumber();
      threevector Box_posn;
      curr_Box_ptr->get_UVW_coords(t,passnumber,Box_posn);
      Box_posn  -= get_BoxesGroup_ptr()->get_grid_world_origin();
      curr_Box_ptr->reset_selected_face_drawable(selected_face,Box_posn);
   }
   else
   {
//      get_BoxesGroup_ptr()->deselect_all_faces();
   }

   return (Box_ID > -1);
}

// --------------------------------------------------------------------------
// Member function scale_Box

bool BoxPickHandler::scale_Box(float X,float Y,float oldX,float oldY)
{
//   cout << "inside BPH::scale_Box()" << endl;
//   cout << "X = " << X << " oldX = " << oldX << endl;
//   cout << "Y = " << Y << " oldY = " << oldY << endl;

   int Box_ID=get_selected_Graphical_ID();
   if (Box_ID >= 0)
   {
      Box* curr_Box_ptr=get_BoxesGroup_ptr()->get_ID_labeled_Box_ptr(Box_ID);
      mybox* curr_mybox_ptr=curr_Box_ptr->get_mybox_ptr();
      threevector world_center(curr_mybox_ptr->get_center().get_center());

// Recall that we need to undo the world_center's renormalization
// relative to the grid world origin BEFORE converting it to screen
// space:

      world_center  += get_BoxesGroup_ptr()->get_grid_world_origin();
      threevector screen_center(
         get_CM_3D_ptr()->get_Transformer_ptr()->
         world_to_screen_transformation(world_center));

// Compare previous and current mouse positions relative to
// *curr_box_ptr's screen space center:

      twovector prev_rel_posn( 
         twovector(oldX,oldY)- 
         twovector(screen_center.get(0),screen_center.get(1)));
      
      twovector curr_rel_posn( 
         twovector(X,Y)- 
         twovector(screen_center.get(0),screen_center.get(1)));
      
      double prev_rel_dist=prev_rel_posn.magnitude();
      double curr_rel_dist=curr_rel_posn.magnitude();
      
      bool mouse_towards_box=true;
      if (curr_rel_dist > prev_rel_dist)
      {
         mouse_towards_box=false;
      }
//      cout << "mouse towards box = " << mouse_towards_box << endl;

      int selected_face=curr_Box_ptr->get_selected_face_number();
//      cout << "selected face number = " << selected_face << endl;

      double frac=1.01;
      if (mouse_towards_box)
      {
         frac=0.99;
      }

      if (selected_face==-1)
      {
         curr_mybox_ptr->reset_dimension_fractions(1,1,frac);
      }
      else if (selected_face==0)
      {
         curr_mybox_ptr->reset_dimension_fractions(1,frac,1);
      }
      else if (selected_face==1)
      {
         curr_mybox_ptr->reset_dimension_fractions(frac,1,1);
      }
      else if (selected_face==2)
      {
         curr_mybox_ptr->reset_dimension_fractions(1,frac,1);
      }
      else if (selected_face==3)
      {
         curr_mybox_ptr->reset_dimension_fractions(frac,1,1);
      }
      else if (selected_face==4)
      {
         curr_mybox_ptr->reset_dimension_fractions(1,1,frac);
      }
      else
      {
         curr_mybox_ptr->reset_dimension_fractions(frac,frac,1);
      }

// UGLY UGLY...fix these next lines later...

      double t=get_BoxesGroup_ptr()->get_curr_t();
      int passnumber=get_BoxesGroup_ptr()->get_passnumber();

      threevector Box_scale(curr_mybox_ptr->get_width(),
                            curr_mybox_ptr->get_length(),
                            curr_mybox_ptr->get_height());
      curr_Box_ptr->set_scale(t,passnumber,Box_scale);

      threevector Box_posn;
      curr_Box_ptr->get_UVW_coords(t,passnumber,Box_posn);
      Box_posn  -= get_BoxesGroup_ptr()->get_grid_world_origin();
      curr_Box_ptr->reset_selected_face_drawable(selected_face,Box_posn);

   } // Box_ID >= 0 conditional
   
   return false;
}
