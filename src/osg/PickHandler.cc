// ==========================================================================
// Purely virtual PickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 9/2/08; 9/12/08; 12/26/10
// ==========================================================================

#include <iostream>
#include <vector>
#include "osg/ModeController.h"
#include "osg/PickHandler.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::vector;

void PickHandler::allocate_member_objects()
{
}		       

void PickHandler::initialize_member_objects()
{
   enable_pick_flag=false;
   enable_drag_flag=false;
   rotation_mode=scaling_mode=false;
   t_curr_click=t_prev_click=-1;
   curr_X=curr_Y=1;
   prev_X=prev_Y=-1;

   min_doubleclick_time_spread=0.05;	// sec
   //   max_doubleclick_time_spread=0.25;	// sec
   max_doubleclick_time_spread=0.35;	// sec	increased value for DT use
   max_delta=0.03;	// percentage fraction
}		       

PickHandler::PickHandler(const int p_ndims,ModeController* MC_ptr):
   ndims(p_ndims)
{
   allocate_member_objects();
   initialize_member_objects();
   this->MC_ptr=MC_ptr;
}

PickHandler::~PickHandler() 
{
}

// ---------------------------------------------------------------------
bool PickHandler::handle(
   const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
//   cout << "inside PickHandler::handle()" << endl;
   static float oldX, oldY;

// Press left shift key to override customary mouse button activity
// and to graphical objects:

   bool shift_key_depressed=
      (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT);

//   cout << "ea.getEventType() = " << ea.getEventType() << endl;
   switch(ea.getEventType())
   {
      case(osgGA::GUIEventAdapter::PUSH):
      {

         // If time interval between previous and current mouse button
         // push is sufficiently small and if screenspace separation
         // between 2 pushes is also sufficiently small , interpret
         // mouse event as a doubleclick rather than single mouse
         // click:

         t_curr_click=ea.getTime();
         curr_X=ea.getX();
         curr_Y=ea.getY();
         double dt_click=t_curr_click-t_prev_click;
         dX=curr_X-prev_X;
         dY=curr_Y-prev_Y;

//          cout.precision(10);
//          cout << "----------------------------------------------" << endl;
//          cout << "this = " << this << endl;
//          cout << "t_curr_click = " << t_curr_click 
//               << " t_prev_click = " << t_prev_click << endl;
//          cout << "dt_click = " << dt_click << endl;
//          cout << "dX = " << dX << " dY = " << dY << endl;

         if (dt_click > min_doubleclick_time_spread &&
             dt_click < max_doubleclick_time_spread &&
             fabs(dX) < max_delta && fabs(dY) < max_delta)
         {
//            cout << "Mouse double clicked" << endl;
//            cout << "t_curr_click = " << t_curr_click 
//                 << " t_prev_click = " << t_prev_click << endl;

//            cout << "dt_click = " << dt_click << endl;
//            cout << "dX = " << dX << " dY = " << dY << endl;
            return doubleclick(ea);
         }
         else
         {
            ModeController::eState curr_state=MC_ptr->getState();
            if (curr_state==ModeController::INSERT_ANNOTATION || 
                shift_key_depressed || enable_pick_flag)
            {
               return pick(ea);
            }
            
            unsigned int buttonMask = ea.getButtonMask();

//            cout << "MC_ptr->get_picking_mode_flag() = "
//                 << MC_ptr->get_picking_mode_flag() << endl;
            
            if (MC_ptr->get_picking_mode_flag() &&
                buttonMask==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
               oldX = ea.getX();
               oldY = ea.getY();

               t_prev_click=t_curr_click;
               prev_X=curr_X;
               prev_Y=curr_Y;
               bool pick_flag=pick(ea);
//               cout << "pick return flag = " << pick_flag << endl;

               if (curr_state==ModeController::MANIPULATE_POLYLINE)
               {
                  MC_ptr->set_allow_manipulator_translation_flag(true);
               }

/*
// As of 9/2/08, we want users in MANIPULATE_POLYLINE mode to be able
// to both translate the background map as well as select only UAV
// PolyLines via single picks.  So if pick_flag==false, we set
// ModeController's allow_manipulator_translation_flag to true so that
// the left button mouse event can fall through to Custom3DManipulator
// to be handled as a translation.

               if ((curr_state==ModeController::MANIPULATE_POLYLINE 
                   pick_flag==false)
               {
                  MC_ptr->set_allow_manipulator_translation_flag(true);
               }
               else
               {
                  MC_ptr->set_allow_manipulator_translation_flag(false);
               }
*/

               return pick_flag;
            }
         } // dt_click conditional
         oldX = ea.getX();
         oldY = ea.getY();

         t_prev_click=t_curr_click;
         prev_X=curr_X;
         prev_Y=curr_Y;

         break;
      }
      
      case(osgGA::GUIEventAdapter::DRAG):
      {

         ModeController::eState curr_state=MC_ptr->getState();

// As of 5/2/09, we want users to be able to move ROI bboxes in
// Real-time persistent surveillance demo without having to hold down
// shift key.  So we explicitly enable Polyhedron dragging if current
// mode = MANIPULATE_POLYHEDRON:

         if (curr_state==ModeController::MANIPULATE_POLYHEDRON ||
             shift_key_depressed || enable_drag_flag)
         {
            if (rotation_mode)
            {
//               bool rotate_flag=rotate(ea);
               bool rotate_flag=rotate(oldX,oldY,ea);
               oldX = ea.getX();
               oldY = ea.getY();
               return rotate_flag;
            }
            else if (scaling_mode)
            {
               bool scale_flag=scale(oldX,oldY,ea);
               oldX = ea.getX();
               oldY = ea.getY();
               return scale_flag;
            }
            else
            {
               bool drag_flag=drag(ea);
               return drag_flag;
            }
         }
         break;
      }

/*
  case(osgGA::GUIEventAdapter::DOUBLECLICK):
  {
  if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT)
  {
  return doubleclick(ea);
  }
  break;
  }
*/

      case(osgGA::GUIEventAdapter::RELEASE):
      {
         if (ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_META)
         {
            return pick_box(oldX, oldY, ea);
         }
         else if (shift_key_depressed || MC_ptr->get_picking_mode_flag())
//         else if (shift_key_depressed)
         {
            return release();
         }
//         cout << "at end of release case" << endl;
         //t_release=ea.getTime();
         //double dt_release=t_release-t_curr_click;
         //cout << "dt_release = " << dt_release << endl;
         break;
      }

      case(osgGA::GUIEventAdapter::KEYDOWN):
      {
         if (ea.getKey()=='r') 
         {
            return toggle_rotate_mode();
         }
         else if (ea.getKey()=='s')
         {
            return toggle_scaling_mode();
         }
         else
         {
            return false;
         }
      }

      default:
         return false;

   }

   return false; 
}
