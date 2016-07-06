// =====================================================================
// CustomManipulator.cc is a variant of
// osgGA/TrackballManipulator.cpp.  It handles non-feature
// manipulation events.

// Note added on 7/19/06: Should this class be made pure virtual by
// setting handle_dim_specific = 0 ???

// =====================================================================
// Last updated on 12/26/10; 2/17/11; 4/5/14; 6/7/14
// =====================================================================

#include <osg/BoundsChecking>
#include <iostream> 
#include <osg/Notify>
#include <osg/Quat>
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgfuncs.h"
#include "osg/osgWindow/WindowManager.h"

using namespace osg;
using namespace osgGA;
using std::cout;
using std::endl;
using std::flush;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CustomManipulator::allocate_member_objects()
{
}		       

void CustomManipulator::initialize_member_objects()
{
   enable_pick_flag=enable_drag_flag=false;
   ModeController_ptr=NULL;
   WindowManager_ptr=NULL;
   ea_ptr=NULL;
   us_ptr=NULL;
   
   _modelScale = 0.01f;
   _minimumZoomScale = 0.05f;
   _thrown = false;
    
   _distance = 1.0f;
   _customSize = 0.8f;

   sticky_action = false;
}		       

// --------------------------------------------------------------------------
CustomManipulator::CustomManipulator(ModeController* MC_ptr,
                                     WindowManager* WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   WindowManager_ptr=WM_ptr;
}

// --------------------------------------------------------------------------
void CustomManipulator::setNode(osg::Node* node)
{
   _node = node;
   if (_node.get())
   {
      const osg::BoundingSphere& boundingSphere=_node->getBound();
      _modelScale = boundingSphere._radius;
   }
   if (getAutoComputeHomePosition()) computeHomePosition();
}

const osg::Node* CustomManipulator::getNode() const
{
   return _node.get();
}

osg::Node* CustomManipulator::getNode()
{
   return _node.get();
}

void CustomManipulator::home(double /*currentTime*/)
{
//   cout << "inside CM::home()" << endl;
   if (getAutoComputeHomePosition()) computeHomePosition();
   computePosition(_homeEye, _homeCenter, _homeUp);
}

void CustomManipulator::home(const GUIEventAdapter& ea ,GUIActionAdapter& us)
{
//   cout << "inside CM::home() #2" << endl;
   home(ea.time());
   us.requestRedraw();
}

void CustomManipulator::init(const GUIEventAdapter& ,GUIActionAdapter& )
{
   flushMouseEventStack();
}

// On 2/17/11, Ross Anderson suggested that we create some function
// like the following to handle head hardware events.  We would need
// to pass a Terrain_Manipulator pointer from our main program to
// whatever software parses the head hardware.  It would then call
// handlehead whenever some head event is detected by the head hardware...

/*
bool CustomManipulator::handlehead(HeadEventAdapter& ha)
{
   
}
*/


bool CustomManipulator::handle(
   const GUIEventAdapter& ea,GUIActionAdapter& us) 
{
//   cout << "inside CustomManipulator::handle()" << endl;
//   cout << "ea.getEventType() = " << ea.getEventType() << endl;

   ea_ptr=&(ea);
   us_ptr=&(us);

//   cout << "ea.getEventType() = " << ea.getEventType() << endl;
   switch(ea.getEventType()) // parse most recent event (ea)
   {
      //==== USE MOUSE MOVEMENT TO CALCULATE MOVEMENT VELOCITY ====//

      case (GUIEventAdapter::PUSH):
      {
//         cout << "Push event" << endl;
         flushMouseEventStack();

         addMouseEvent(ea);
	 bool moved=calcMovement(ea);
         if (moved) us.requestRedraw();
         us.requestContinuousUpdate(false);
         _thrown = false;
         return moved;
      }

      case (GUIEventAdapter::SCROLL):
      {
         flushMouseEventStack();

         //addMouseEvent(ea);
	 bool moved=calcMovement(ea);
         if (moved) us.requestRedraw();

         us.requestContinuousUpdate(false);
         _thrown = false;
         return moved;
      }

      case (GUIEventAdapter::RELEASE):
      {
//         cout << "Release event" << endl;
         if (!sticky_action) 
            flushMouseEventStack();
         
         if ( !sticky_action || ea.getButtonMask()) 
            addMouseEvent(ea);
         
         bool moved=false;
         if ( sticky_action || ea.getButtonMask())
            moved=calcMovement(ea);
            if (moved) us.requestRedraw();

         if (moved) 	// if there is a release, but no movement, don't
			//  bother with continuous update 
         {
            us.requestContinuousUpdate(sticky_action && !ea.getButtonMask());
            _thrown = sticky_action && !ea.getButtonMask();
         }
         else 
         {
            us.requestContinuousUpdate(false);
            _thrown = false;
         }
         
         return moved;
      }

      case (GUIEventAdapter::DRAG):
      {
//         cout << "Drag event" << endl;

// Do not perform any dragging operations if Control key is depressed.
// We save this combination for the PickHandler.

         if (!(ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL))
         {
            addMouseEvent(ea);
            bool moved=calcMovement(ea);
            if (moved) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
            return moved;
         }
         return false;
      }

      case (GUIEventAdapter::MOVE):
         return false;

      case (GUIEventAdapter::FRAME):
      {
//         cout << "Frame event" << endl;

//         cout << "_thrown = " << _thrown << endl;
//         cout << "sticky_action = " << sticky_action << endl;
         if (_thrown)
            if (calcMovement(ea)) 
               us.requestRedraw();

         return false;
      }
         
      //==== NON-MOVEMENT KEYS ====//
      case (GUIEventAdapter::KEYDOWN):
      {
//         cout << "Keydown event" << endl;
         switch(ea.getKey())
         {
            case (GUIEventAdapter::KEY_Space): // same as " "
            {
               return reset_view_to_home(ea,us);
            }

// Caps lock key toggles between manipulations which continue even
// when mouse is not moved and manipulations which terminate when
// mouse is stopped:

            case (GUIEventAdapter::KEY_Caps_Lock ):
            {
               sticky_action = !sticky_action;
               if (!sticky_action)
               {
                  flushMouseEventStack();
               }
               if (calcMovement(ea)) us.requestRedraw();
               us.requestContinuousUpdate(false);
               _thrown=false;
               return true;
            }

            // any other key
            default: 
            {
               if (calcMovement(ea)) // check movement keys (specified
 // in calcMovement)
               {
                  us.requestRedraw(); 
                  us.requestContinuousUpdate(true);
                  _thrown=true; 
                   
                  return true; 
               }
               else
                  return false;
            }
         }
         return false;
      } // case GUIEventAdaptor::KEYDOWN

      case (GUIEventAdapter::KEYUP):
      { 
//         cout << "Key up event" << endl;
         if (!calcMovement(ea)) // if we're not moving, stop the frame update
         {
            _thrown = false;
            us.requestContinuousUpdate(false);
         }
      }

      // all other events besides push,release,drag,frame,key_down
      default: 

         return false;
   }
   
   return false;
}


bool CustomManipulator::isMouseMoving()
{
   if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

   static const float velocity = 0.1f; 
    
   float dx = _ga_t0->getXnormalized()-_ga_t1->getXnormalized();
   float dy = _ga_t0->getYnormalized()-_ga_t1->getYnormalized(); 
   float len = sqrtf(dx*dx+dy*dy); 
   float dt = _ga_t0->time()-_ga_t1->time(); 
   return (len>dt*velocity);
}

void CustomManipulator::flushMouseEventStack()
{
   _ga_t1 = NULL;
   _ga_t0 = NULL;
}

void CustomManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
   _ga_t1 = _ga_t0;
   _ga_t0 = &ea;
}

void CustomManipulator::setByMatrix(const osg::Matrixd& matrix)
{
   _center = osg::Vec3(0.0f,0.0f,-_distance)*matrix;
   matrix.get(_rotation);
}

osg::Matrixd CustomManipulator::getMatrix() const
{
   return osg::Matrixd::translate(0.0,0.0,_distance)*osg::Matrixd::rotate(
      _rotation)*osg::Matrixd::translate(_center);
}

osg::Matrixd CustomManipulator::getInverseMatrix() const
{
//   cout << "inside CM::getInverseMatrix()" << endl;
//   cout.precision(12);
//   cout << "_center: X = " << _center.x() << " Y = " << _center.y()
//        << " Z = " << _center.z() << endl;
//   cout << "camera distance from center = " << _distance << endl;
   return osg::Matrixd::translate(-_center)*
      osg::Matrixd::rotate(_rotation.inverse())*
      osg::Matrixd::translate(0,0,-_distance);
}

void CustomManipulator::computePosition(
   const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{
   _center = center;
   osg::Vec3 lv(center-eye);
   _distance = lv.length();

   osg::Vec3 f(lv);
   f.normalize();
   osg::Vec3 s(f^up);
   s.normalize();
   osg::Vec3 u(s^f);
   u.normalize();
   osg::Matrix rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                               s[1],     u[1],     -f[1],     0.0f,
                               s[2],     u[2],     -f[2],     0.0f,
                               0.0f,     0.0f,     0.0f,      1.0f);

   rotation_matrix.get(_rotation);
   _rotation = _rotation.inverse();
}

void CustomManipulator::Rotate(
   float fx_prev,float fy_prev,float fx_curr,float fy_curr)
{
}

// ---------------------------------------------------------------------
void CustomManipulator::Translate(float d_fx,float d_fy)
{
//   cout << "inside CustomManipulator::Translate()" << endl;

   float scale = -0.3 * _distance;
   osg::Vec3 dv(d_fx*scale,d_fy*scale,0);
    
   osg::Matrix rotation_matrix;
   rotation_matrix.set(_rotation);
   _center += dv*rotation_matrix;
}

// ---------------------------------------------------------------------
bool CustomManipulator::calcMovement(const GUIEventAdapter& ea)
{
//   cout << "inside CM::calcMovement()" << endl;
   bool keymove = parse_keyboard_events(ea);
   bool mousemove = parse_mouse_events(ea);
   bool scrollmove = parse_scroll_events(ea);
//   cout << "keymove = " << keymove << " mousemove = " << mousemove
//        << " scrollmove = " << scrollmove << endl;
   
   return (keymove || mousemove || scrollmove);
}

bool CustomManipulator::parse_keyboard_events(const GUIEventAdapter& ea)
{
//   cout << "inside CM::parse_keyboard_events() before returning false:"
//        << endl;
   return false;
}

bool CustomManipulator::parse_mouse_events(const GUIEventAdapter& ea)
{
//   cout << "inside CM::parse_mouse_events() before returning false:"
//        << endl;
   return false;
}

bool CustomManipulator::parse_scroll_events(const GUIEventAdapter& ea)
{
//   cout << "inside CM::parse_scroll_events() before returning false:"
//        << endl;
   return false;
}

// ---------------------------------------------------------------------
/*
* This size should really be based on the distance from the center of
* rotation to the point on the object underneath the mouse.  That
* point would then track the mouse as closely as possible.  This is a
* simple example, though, so that is left as an Exercise for the
* Programmer.
 */
void CustomManipulator::setCustomSize(float size)
{
   _customSize = size;
   osg::clampBetweenRange(
      _customSize,0.1f,1.0f,"CustomManipulator::setCustomSize(float)");
}

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * custom, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed custom-- is a custom in the center,
 * but is deformed into a hyperbolic sheet of rotation away from the
 * center.  This particular function was chosen after trying out
 * several variations.
 *
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */

void CustomManipulator::getTrackballRotation(
   osg::Vec3& axis,float& angle, float f1x, float f1y, float f2x, float f2y)
{
//   cout << "inside CM::get_rot_from_pnts()" << endl;
//   cout << "f1x = " << f1x << " f2x = " << f2x << endl;
//   cout << "f1y = " << f1y << " f2y = " << f2y << endl;
   
   /*
    * First, figure out z-coordinates for projection of P1 and P2 to
    * deformed sphere
    */

   osg::Matrix rotation_matrix(_rotation);

   osg::Vec3 uv = osg::Vec3(0.0f,1.0f,0.0f)*rotation_matrix;
   osg::Vec3 sv = osg::Vec3(1.0f,0.0f,0.0f)*rotation_matrix;
   osg::Vec3 lv = osg::Vec3(0.0f,0.0f,-1.0f)*rotation_matrix;

   osg::Vec3 p1 = sv * f1x + uv * f1y 
      - lv * tb_project_to_sphere(_customSize, f1x, f1y);
   osg::Vec3 p2 = sv * f2x + uv * f2y 
      - lv * tb_project_to_sphere(_customSize, f2x, f2y);

    /*
     *  Now, we want the cross product of P1 and P2
     */

// Robert,
//
// This was the quick 'n' dirty  fix to get the custom doing the right 
// thing after fixing the Quat rotations to be right-handed.  You may want
// to do something more elegant.
//   axis = p1^p2;
   axis = p2^p1;
   axis.normalize();

    /*
     *  Figure out how much to rotate around that axis.
     */
   float t = (p2 - p1).length() / (2.0 * _customSize);

   /*
     * Avoid problems with out-of-control values...
     */
   if (t > 1.0) t = 1.0;
   if (t < -1.0) t = -1.0;
   angle = inRadians(asin(t));

}


/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
float CustomManipulator::tb_project_to_sphere(float r, float x, float y)
{
   float d, t, z;

   d = sqrt(x*x + y*y);
   /* Inside sphere */
   if (d < r * 0.70710678118654752440)
   {
      z = sqrt(r*r - d*d);
   }                            /* On hyperbola */
   else
   {
      t = r / 1.41421356237309504880;
      z = t*t / d;
   }
   return z;
}

// ---------------------------------------------------------------------
// Method set_worldspace_center resets the worldspace point which is
// mapped to the screen's physical center location. 

void CustomManipulator::set_worldspace_center(const threevector& new_center)
{
//   cout << "inside CustomManipulator::set_worldspace_center()" << endl;
   set_center(
      osg::Vec3(new_center.get(0),new_center.get(1),new_center.get(2)));
}

// ---------------------------------------------------------------------
// Method reset_nadir_view sets the internal _rotation quaternion so
// that the camera looks straight down onto the screen.

void CustomManipulator::reset_nadir_view()
{
   osg::Matrix identity_matrix;
   _rotation.set(identity_matrix);
}

// ---------------------------------------------------------------------
// Method get_rotation_matrix outputs the internal _rotation
// quaternion as a 4x4 matrix.

osg::Matrix CustomManipulator::get_rotation_matrix() const
{
   return get_rotation_matrix(_rotation);
}

osg::Matrix CustomManipulator::get_rotation_matrix(const osg::Quat& q) const
{
   osg::Matrix rotation_matrix(q);
   return rotation_matrix;
}

// ---------------------------------------------------------------------
// Method reset_view_to_home()

bool CustomManipulator::reset_view_to_home()
{
   cout << "inside CustomManipulator::reset_view_to_home()" << endl;
   return reset_view_to_home(*ea_ptr,*us_ptr);
}

bool CustomManipulator::reset_view_to_home(
   const GUIEventAdapter& ea,GUIActionAdapter& us) 
{
   cout << "inside CustomManipulator::reset_view_to_home() #2" << endl;

   flushMouseEventStack();
   _thrown = false;
   home(ea,us);
   us.requestContinuousUpdate(false);

   return true;
}
