// =====================================================================
// Custom2DManipulator class member functions handle non-feature
// manipulation events.
// =====================================================================
// Last updated on 1/10/11; 1/12/10; 2/28/11
// =====================================================================

#include <iostream> 
#include <osg/Quat>
#include "osg/ModeController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgfuncs.h"

using namespace osg;
using namespace osgGA;
using std::cout;
using std::endl;

void Custom2DManipulator::allocate_member_objects()
{
}		       

void Custom2DManipulator::initialize_member_objects()
{
   set_ndims(2);
}

Custom2DManipulator::Custom2DManipulator(ModeController* MC_ptr,
                                         WindowManager* WM_ptr):
   CustomManipulator(MC_ptr,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
}

Custom2DManipulator::~Custom2DManipulator()
{
}

void Custom2DManipulator::reset_Manipulator_control()
{
//   cout << "inside Custom2DManipulator::reset_Manipulator_control()"
//        << endl;
}

// --------------------------------------------------------------------------
void Custom2DManipulator::Rotate(float fx1,float fy1,float px0,float fy0)
{
   // rotate the camera
   osg::Vec3 axis;
   float angle;
   getTrackballRotation(axis,angle,fx1,fy1,px0,fy0);

// For 2D images, force rotation axis to point directly into the
// screen (i.e. axis must lie purely in the +/- y direction):

   axis._v[0]=axis._v[2]=0;
   axis._v[1] /= fabs(axis._v[1]);

   osg::Quat new_rotate;
   new_rotate.makeRotate(angle,axis);
   _rotation = _rotation*new_rotate;
   return;
}

// ---------------------------------------------------------------------
bool Custom2DManipulator::parse_mouse_events(const GUIEventAdapter& ea)
{
   bool mousemove=false;

   if (! (_ga_t0.get()==NULL || _ga_t1.get()==NULL))
   {    
      float fx0 = _ga_t0->getXnormalized();
      float fy0 = _ga_t0->getYnormalized();
        
      float fx1 = _ga_t1->getXnormalized();
      float fy1 = _ga_t1->getYnormalized();
        
      float d_fx = fx0-fx1;
      float d_fy = fy0-fy1;
        
//      float dist2 = (dx*dx + dy*dy);
//      float dt = _ga_t0->time() - _ga_t1->time(); // use later? 
  
// Return if there is no movement...don't 

      if (d_fx==0 && d_fy==0) return mousemove;
        
      unsigned int buttonMask = _ga_t1->getButtonMask();
      switch( buttonMask )
      {

// On 9/13/07, we remapped the mouse button functions to conform with
// Google Earth's conventions:

         case (GUIEventAdapter::LEFT_MOUSE_BUTTON): 
         {

// Do NOT translate scene objects with left mouse button if either
// left or right shift key is depressed.  Instead, we'll regard shift
// as a toggle for entering into 3D object selection mode:

            bool shift_key_depressed=
               (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT);

            if ( (!shift_key_depressed && 
               !get_enable_pick_flag() && !get_enable_drag_flag() &&
               !(get_ModeController_ptr()->get_picking_mode_flag()))  ||
               (get_ModeController_ptr()->
               get_allow_manipulator_translation_flag())
               )
            {
               Translate(d_fx,d_fy);
               mousemove=true;
               break;
            }
         }
         
         case (GUIEventAdapter::MIDDLE_MOUSE_BUTTON): 
         {
            if (get_ModeController_ptr()->getState()== 
            ModeController::VIEW_DATA ||
            get_ModeController_ptr()->getState()==
            ModeController::RUN_MOVIE)
            {

// Do NOT rotate scene objects with left mouse button if either left
// or right shift key is depressed.  Instead, we'll regard shift as a
// toggle for entering into 3D object selection mode:

               if (!(ea.getModKeyMask()&
                     osgGA::GUIEventAdapter::MODKEY_SHIFT))
               {
                  Rotate(fx1,fy1,fx0,fy0);
                  mousemove=true;
               }
            }
            break;
         }
         case (GUIEventAdapter::RIGHT_MOUSE_BUTTON):
         {
            // zoom model
//            double scalefactor=(1.0/(1.0+d_fy));
            double scalefactor=(1.0/(1.0-d_fy));
            set_eye_to_center_distance(get_eye_to_center_distance()*
                                       scalefactor);
//            _distance /= 1.0+dy; 
            mousemove=true;
            break;
         }
      } // switch buttommask
   }
   return mousemove;
}
