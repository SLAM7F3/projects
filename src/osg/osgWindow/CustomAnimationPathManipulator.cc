// =======================================================================
// CustomAnimationPathManipulator class
// =======================================================================
// Last updated on 8/24/09; 12/4/10; 2/28/11
// =======================================================================

#include <iostream>
#include <fstream>
#include <string>
#include "osg/Custom3DManipulator.h"
#include "osg/osgWindow/CustomAnimationPathManipulator.h"
#include "osg/osgfuncs.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::list;
using std::string;
using namespace osgGA;


// -----------------------------------------------------------------------
void CustomAnimationPathManipulator::allocate_member_objects()
{
}

void CustomAnimationPathManipulator::initialize_member_objects()
{
   _printOutTiminInfo = true;
   _timeOffset = 0.0;
   _timeScale = 1.0;
   _isPaused = false;

   CM_2D_ptr=NULL;
   CM_3D_ptr=NULL;

   set_initial_FOV_u(WindowManager_ptr->get_lens_horizontal_FOV());
   set_initial_FOV_v(WindowManager_ptr->get_lens_vertical_FOV());
   set_final_FOV_u(WindowManager_ptr->get_lens_horizontal_FOV());
   set_final_FOV_v(WindowManager_ptr->get_lens_vertical_FOV());
}
   
// -----------------------------------------------------------------------
CustomAnimationPathManipulator::CustomAnimationPathManipulator(
   osg::AnimationPath* animationPath,WindowManager* WM_ptr,
   Custom3DManipulator* CM_3D_ptr,const threevector& final_posn):
   Custom3DManipulator(NULL,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   _animationPath = animationPath;
   this->CM_3D_ptr=CM_3D_ptr;
   this->final_posn=final_posn;
    
   _realStartOfTimedPeriod = 0.0;
   _animStartOfTimedPeriod = 0.0;
   _numOfFramesSinceStartOfTimedPeriod = -1; // need to init.
}

// -----------------------------------------------------------------------
CustomAnimationPathManipulator::CustomAnimationPathManipulator( 
   const std::string& filename,WindowManager* WM_ptr,
   Custom3DManipulator* CM_3D_ptr):
   Custom3DManipulator(NULL,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   this->CM_3D_ptr=CM_3D_ptr;

   _animationPath = new osg::AnimationPath;
   _animationPath->setLoopMode(osg::AnimationPath::LOOP);

   std::ifstream in(filename.c_str());

   if (!in)
   {
      osg::notify(osg::WARN) << 
         "CustomAnimationPathManipulator: Cannot open animation path file \"" 
                             << filename << "\".\n";
      _valid = false;
      return;
   }

   _animationPath->read(in);
   in.close();
}

// -----------------------------------------------------------------------
CustomAnimationPathManipulator::CustomAnimationPathManipulator(
   osg::AnimationPath* animationPath,WindowManager* WM_ptr,
   Custom2DManipulator* CM_2D_ptr,const threevector& final_posn):
   Custom3DManipulator(NULL,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   _animationPath = animationPath;
   this->CM_2D_ptr=CM_2D_ptr;
   this->final_posn=final_posn;
    
   _realStartOfTimedPeriod = 0.0;
   _animStartOfTimedPeriod = 0.0;
   _numOfFramesSinceStartOfTimedPeriod = -1; // need to init.
}

// -----------------------------------------------------------------------
CustomAnimationPathManipulator::CustomAnimationPathManipulator( 
   const std::string& filename,WindowManager* WM_ptr,
   Custom2DManipulator* CM_2D_ptr):
   Custom3DManipulator(NULL,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   this->CM_2D_ptr=CM_2D_ptr;

   _animationPath = new osg::AnimationPath;
   _animationPath->setLoopMode(osg::AnimationPath::LOOP);

   std::ifstream in(filename.c_str());

   if (!in)
   {
      osg::notify(osg::WARN) << 
         "CustomAnimationPathManipulator: Cannot open animation path file \"" 
                             << filename << "\".\n";
      _valid = false;
      return;
   }

   _animationPath->read(in);
   in.close();
}

// -----------------------------------------------------------------------
void CustomAnimationPathManipulator::home(double currentTime)
{
   if (_animationPath.valid())
   {
      _timeOffset = _animationPath->getFirstTime()-currentTime; 

   }
   // reset the timing of the animation.
   _numOfFramesSinceStartOfTimedPeriod=-1;
}

void CustomAnimationPathManipulator::home(
   const GUIEventAdapter& ea,GUIActionAdapter&)
{
   home(ea.getTime());
}

void CustomAnimationPathManipulator::init(
   const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
   home(ea,aa);
}

// -----------------------------------------------------------------------
bool CustomAnimationPathManipulator::handle(
   const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
{
   if( !valid() ) return false;

   switch( ea.getEventType() )
   {
      case GUIEventAdapter::FRAME:
         if( _isPaused )
         {
            handleFrame( _pauseTime );
         }
         else
         {
            handleFrame( ea.getTime() );
         }
         return false;
      case GUIEventAdapter::KEYDOWN:
         if (ea.getKey()==' ')
         {
            _isPaused = false;
            home(ea,us);
            us.requestRedraw();
            us.requestContinuousUpdate(false);
                
            return true;
         } 
              
         break;
      default:
         break;
   }
   return false;
}

// -----------------------------------------------------------------------
void CustomAnimationPathManipulator::getUsage(
   osg::ApplicationUsage& usage) const
{
   usage.addKeyboardMouseBinding("AnimationPath: Space","Reset the viewing position to start of animation");
   usage.addKeyboardMouseBinding("AnimationPath: p","Pause/resume animation.");
}

// -----------------------------------------------------------------------
void CustomAnimationPathManipulator::handleFrame( double time )
{
//    cout << "inside CAPM::handleFrame()" << endl;

   double animTime = (time+_timeOffset)*_timeScale;
   osg::AnimationPath::ControlPoint cp;
   _animationPath->getInterpolatedControlPoint( animTime, cp );

   if (_numOfFramesSinceStartOfTimedPeriod==-1)
   {    
      _realStartOfTimedPeriod = time;

// On 1/26/07, we empirically found that ea.getTime() typically skips
// from some negative value (e.g. -4 secs) to a near zero value within
// one timestep.  It then continues to increment in small time steps.
// But the next original line would set _animStartOfTimedPeriod to the
// negative value.  This would result in the total animation path
// being cut short by the difference between 0 and the negative
// _animStartOfTimedPeriod.  So we force _animStartOfTimedPeriod to
// equal 0 to ensure that the animation path runs to completion...

//      _animStartOfTimedPeriod = animTime;
      _animStartOfTimedPeriod = 0;
   }
   ++_numOfFramesSinceStartOfTimedPeriod;

   double delta = (animTime-_animStartOfTimedPeriod);
   frac_path_completed=delta/(_animationPath->getPeriod());
   frac_path_completed=basic_math::max(0.0,frac_path_completed);
   frac_path_completed=basic_math::min(1.0,frac_path_completed);

//   cout << "t=" << time
//        << " animTime=" << animTime 
//        << " _animStart=" << _animStartOfTimedPeriod
//        << " delta=" << delta 
//        <<  " time_offset = " << _timeOffset
//        << " anim Period = " << _animationPath->getPeriod()
//        << " frac path completed = " << frac_path_completed 
//        << endl;

   if (delta >= _animationPath->getPeriod())
   {
      if (getAnimationPath()->getLoopMode()==osg::AnimationPath::LOOP)
      {
         double frameRate = (double)_numOfFramesSinceStartOfTimedPeriod/
            delta;
         osg::notify(osg::NOTICE) << "AnimationPath completed in "
                                  << delta <<" seconds, completing "
                                  << _numOfFramesSinceStartOfTimedPeriod
                                  << " frames," << endl;
         osg::notify(osg::NOTICE) << "             average frame rate = "
                                  << frameRate << endl;
            
//            _realStartOfTimedPeriod = time;
//            _animStartOfTimedPeriod = animTime;
//            _numOfFramesSinceStartOfTimedPeriod = 0;  
      }
      else
      {

// Once AnimationPath has played to its end, reset manipulator from
// current CustomAnimationPathManipulator object to original
// Custom_3D_Manipulator if animation path looping is disabled:

         if (get_ndims()==2)
         {
            WindowManager_ptr->
               selectCameraManipulatorByName(CM_2D_ptr->getName());
            CM_2D_ptr->reset_Manipulator_control();
         }
         else if (get_ndims()==3)
         {
            WindowManager_ptr->
               selectCameraManipulatorByName(CM_3D_ptr->getName());
            CM_3D_ptr->reset_Manipulator_control();
         }

      } // animation path LOOP conditional

// Reset counters for next animation:

      _realStartOfTimedPeriod = time;
      _animStartOfTimedPeriod = animTime;
      _numOfFramesSinceStartOfTimedPeriod = 0;  
      reset_CM_posn_and_orientation(true);

   } // delta >= animation path period conditional
   else
   {
      reset_CM_posn_and_orientation(false);

// Next line is NOT responsible for bad zoom bug in virtual touring...

      update_virtual_camera_FOV();
   }

   if (get_ndims()==3)
   {
      CompassHUD* CompassHUD_ptr=CM_3D_ptr->get_CompassHUD_ptr();
//   cout << "CompassHUD_ptr = " << CompassHUD_ptr << endl;
      if (CompassHUD_ptr != NULL)
      {
//      osg::Matrixd Minv=WindowManager_ptr->getViewMatrix();
         double azimuth=compute_approx_projected_north_hat_azimuth(
            WindowManager_ptr->getViewMatrix());
         CompassHUD_ptr->rotate_compass(azimuth);
      }
   }
   
   cp.getMatrix( _matrix );

//   cout << "_matrix = " << endl;
//   osgfunc::print_matrix(_matrix);
} 

// -----------------------------------------------------------------------
// Member function reset_CM_posn_and_orientation copies the current
// contents of the CustomAnimationPathManipulator's Minverse matrix
// onto the *CM_3D_ptr's Minverse matrix.  This forces the
// CustomManipulator's camera to assume the same position and
// orientation as that of the current Custom Animation Path camera.
// We thereby achieve a smooth transition between the non-interactive
// animation path view and that of the interactive real-time view.

void CustomAnimationPathManipulator::reset_CM_posn_and_orientation(
   bool final_frame_flag)
{
//   cout << "inside CAPM::reset_CM_posn_and_orientation()" << endl;
//   cout << "final_frame_flag = " << final_frame_flag << endl;
   
  if (WindowManager_ptr != NULL)
   {
      osg::Matrixd Minv=WindowManager_ptr->getViewMatrix();
      osg::Matrix M=Minv.inverse(Minv);

// In February 2009, we discovered the hard way that Custom Animation
// paths may not terminate at precisely the specified final
// destination point.  Small discrepancies between the requested and
// actual end points are painfully obvious when zooming into an
// OBSFRUSTUM's vertex.  We therefore explicitly copy the contents of
// member threevector final_posn into M (and subsequently recompute
// Minv) to ensure the Custom Manipulator's position is reset to the
// requested final position:

      if (final_frame_flag)
      {
         M.set(M(0,0) , M(0,1) , M(0,2) , M(0,3),
               M(1,0) , M(1,1) , M(1,2) , M(1,3),
               M(2,0) , M(2,1) , M(2,2) , M(2,3),
               final_posn.get(0), final_posn.get(1), final_posn.get(2), 1);
         Minv=M.inverse(M);
      }
      
      if (get_ndims()==3)
      {
         CM_3D_ptr->set_ViewMatrix(Minv);
         CM_3D_ptr->setMatrices(M);
         CM_3D_ptr->set_eye_to_center_distance(get_eye_to_center_distance());
         CM_3D_ptr->set_worldspace_center(get_worldspace_center());
      }
      else if (get_ndims()==2)
      {
         CM_2D_ptr->set_ViewMatrix(Minv);
         CM_2D_ptr->set_eye_to_center_distance(get_eye_to_center_distance());
         CM_2D_ptr->set_worldspace_center(get_worldspace_center());
      }

//      cout << "M = " << endl;
//      osgfunc::print_matrix(M);

//      cout << "get_eye_to_center_distance() = "
//           << get_eye_to_center_distance() << endl;
//      cout << "get_worldspace_center() = "
//           << get_worldspace_center() << endl;
      
   } // WindowManager_ptr != NULL conditional
}

// -----------------------------------------------------------------------
// Member function update_virtual_camera_FOV() linearly interpolates
// the angular scale factor connecting initial and final
// fields-of-view.  This method resets the virtual camera's current
// FOV to the current interpolated value.

void CustomAnimationPathManipulator::update_virtual_camera_FOV()
{
//   cout << "inside CAPM::update_virtual_camera_FOV()()" << endl;
//   cout << "frac_path_completed = " << frac_path_completed << endl;

   double angular_scale_factor;
   if (final_FOV_u > final_FOV_v)
   {
      angular_scale_factor=1.0+frac_path_completed*(
         final_FOV_u/initial_FOV_u-1.0);
   }
   else
   {
      angular_scale_factor=1.0+frac_path_completed*(
         final_FOV_v/initial_FOV_v-1.0);
   }
//   cout << "init FOV_u = " << initial_FOV_u
//        << " final_FOV_u = " << final_FOV_u << endl;
//   cout << "init FOV_v = " << initial_FOV_v
//        << " final_FOV_v = " << final_FOV_v << endl;
//   cout << "angular scale factor = " << angular_scale_factor << endl;

   double curr_FOV_u=initial_FOV_u*angular_scale_factor;
   double curr_FOV_v=initial_FOV_v*angular_scale_factor;

//   cout << "curr_FOV_u = " << curr_FOV_u 
//        << " curr_FOV_v = " << curr_FOV_v << endl;
   WindowManager_ptr->set_viewer_horiz_vert_fovs(curr_FOV_u,curr_FOV_v);
}

// ---------------------------------------------------------------------
threevector CustomAnimationPathManipulator::get_eye_world_posn() const
{
   return threevector(_matrix(3,0),_matrix(3,1),_matrix(3,2));
}


// ---------------------------------------------------------------------
int CustomAnimationPathManipulator::get_ndims() const
{
   if (get_CM3D_ptr()==NULL)
   {
      return 2;
   }
   else
   {
      return 3;
   }
}
