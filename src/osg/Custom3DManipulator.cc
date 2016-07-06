// =====================================================================
// Custom3DManipulator class member functions handle non-feature
// manipulation events.
// =====================================================================
// Last updated on 9/4/11; 9/5/11; 9/11/11
// =====================================================================

#include <osg/Quat>
#include "osg/Custom3DManipulator.h"
#include "astro_geo/geofuncs.h"
#include "osg/ModeController.h"
#include "osg/osgfuncs.h"
#include "osg/Transformer.h"

using namespace osg;
using namespace osgGA;
using std::cout;
using std::endl;

void Custom3DManipulator::allocate_member_objects()
{
   Transformer_ptr=new Transformer(WindowManager_ptr);
   ViewFrustum_ptr=new ViewFrustum(WindowManager_ptr);
}		       

void Custom3DManipulator::initialize_member_objects()
{
//   cout << "inside C3DM::initialize_member_objects()" << endl;

   set_ndims(3);
   active_control_flag=true;
   DiamondTouchTable_flag=false;
   mouse_input_device_flag=true;
   hmi_select_flag=false;
   hmi_select_value=-1;
   grid_origin_ptr=NULL;
   CompassHUD_ptr=NULL;
   PointFinder_ptr=NULL;
   log_eye_center_dist=NEGATIVEINFINITY;
}

void Custom3DManipulator::store_initial_zoom_params()
{
//   cout << "inside C3DM::store_initial_zoom_params()" << endl;
   init_worldspace_center=get_worldspace_center();
   init_eye_to_center_distance=get_eye_to_center_distance();
}

Custom3DManipulator::Custom3DManipulator(
   ModeController* MC_ptr,WindowManager* WM_ptr,bool disable_rotations,
   bool emulate_GoogleEarth_rotations):
   CustomManipulator(MC_ptr,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   disable_rotations_flag=disable_rotations;
   emulate_GoogleEarth_rotations_flag=emulate_GoogleEarth_rotations;
}

Custom3DManipulator::~Custom3DManipulator()
{
   delete Transformer_ptr;
}

// =====================================================================
// Set & get member functions
// =====================================================================

void Custom3DManipulator::set_hmi_select_value(int value)
{
   hmi_select_value=value;
}

int Custom3DManipulator::get_hmi_select_value() const 
{
   return hmi_select_value;
}

// ---------------------------------------------------------------------
void Custom3DManipulator::set_fx_fy_params(
   float& fx_curr,float& fy_curr,float& fx_prev,float& fy_prev)
{
//   cout << "inside C3DM::set_fx_fy_params()" << endl;

   fx_curr=0.01;
   fy_curr=0.01;
   fx_prev=0;
   fy_prev=0;
}

// ---------------------------------------------------------------------
bool Custom3DManipulator::parse_mouse_events(const GUIEventAdapter& ea)
{
//   cout << "inside C3DM::parse_mouse_events()" << endl;

   bool mousemove=false;

   if (! (_ga_t0.get()==NULL || _ga_t1.get()==NULL))
   {    
      float fx_curr = _ga_t0->getXnormalized();	// current mouse x
      float fy_curr = _ga_t0->getYnormalized();	// current mouse y
      float fx_prev = _ga_t1->getXnormalized();	// previous mouse x
      float fy_prev = _ga_t1->getYnormalized();	// previous mouse y
//      cout << "fx_curr = " << fx_curr << " fx_prev = " << fx_prev << endl;
//      cout << "fy_curr = " << fy_curr << " fy_prev = " << fy_prev << endl;

      float d_fx = fx_curr-fx_prev; // current - previous mouse x
      float d_fy = fy_curr-fy_prev; // current - previous mouse y
//      cout << "d_fx = " << d_fx << " d_fy = " << d_fy << endl;

// Return if there is no movement: 

      if (d_fx==0 && d_fy==0) 
      {
         return mousemove;
      }
      
// On 9/11/07, we changed the mapping of the left, middle and right
// mouse buttons so that their functions match those of Google Earth:

      unsigned int buttonMask = _ga_t1->getButtonMask();
      if ( buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
      {

// Do NOT translate scene objects with left mouse button if either
// left or right shift key is depressed.  Instead, we'll regard shift
// as a toggle for entering into 3D object selection mode.  DO
// translate if ModeController's allow_manipulator_translation_flag
// has been explicitly set equal to true (e.g. allowing a user to
// select mover PolyLines and translate within MANIPULATE_POLYLINE
// mode):
         
         bool shift_key_depressed=
            (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT);

         if ( (!shift_key_depressed && 
	       !get_enable_pick_flag() && !get_enable_drag_flag() &&
              !(get_ModeController_ptr()->get_picking_mode_flag()))  ||
              (get_ModeController_ptr()->
               get_allow_manipulator_translation_flag())
            )
         {
            Translate(fx_curr,fy_curr,fx_prev,fy_prev);
            mousemove=true;
         }
      }
      else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
      {
         Rotate(fx_prev,fy_prev,fx_curr,fy_curr);
         mousemove=true;
      }
      else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
      {
         Zoom(d_fy);
         mousemove=true;
      }

      fx_prev=fx_curr;
      fy_prev=fy_curr;
   
   } // _ga_t0.get() != NULL && _ga_t1.get() != NULL conditional

   return mousemove;
}

// ---------------------------------------------------------------------
// Member function parse_scroll_events tests whether the mouse's
// scroll wheel has been spun.  We interpret scroll movements in the
// DOWN [UP] as inward [outward] zoom motions.  We wrote this method
// in Aug 2007 to enable zooming via two-handed DiamondTouch gestures.

// As of July 2009, we DO intend to enable zooming via ordinary mouse
// wheel input.

bool Custom3DManipulator::parse_scroll_events(const GUIEventAdapter& ea)
{
//   cout << "inside C3DM:::parse_scroll_events()" << endl;

   bool scrollmove=false;

   double dy=0;
   if (ea.getScrollingMotion()==GUIEventAdapter::SCROLL_UP)
   {
      if (DiamondTouchTable_flag)
      {
         dy=0.0045;   // reasonable value for DiamondTouch w/ ISDS3D laptop
      }
      else
      {
         dy=-0.15;	// reasonable value for mouse wheel
      }
      scrollmove=true;
   }
   else if (ea.getScrollingMotion()==GUIEventAdapter::SCROLL_DOWN)
   {
      if (DiamondTouchTable_flag)
      {
         dy=-0.0045;   // reasonable value for DiamondTouch w/ ISDS3D laptop
      }
      else
      {
         dy=0.15;	// reasonable value for mouse wheel
      }
      scrollmove=true;
   }
   if (scrollmove) Zoom(dy);

   return scrollmove;
}

// ---------------------------------------------------------------------
// This overloaded version of member function home resets the camera
// so that its X, Y and Z axes are aligned with the world grid's X, Y
// and Z axes.  The camera's position is also set to (0,0,0) in world
// space.

bool Custom3DManipulator::reset_view_to_home()
{
   cout << "inside CM3D::reset_view_to_home()" << endl;
   cout << "this = " << this << endl;
   home(*ea_ptr,*us_ptr);
   set_eye_to_center_distance(init_eye_to_center_distance);
   set_worldspace_center(init_worldspace_center);
   update_M_and_Minv();

   return true;
}

void Custom3DManipulator::home(
   const GUIEventAdapter& ea ,GUIActionAdapter& us)
{
//    cout << "inside C3DM::home()" << endl;

   M.set(1,0,0,0,
         0,1,0,0,
	 0,0,1,0,
         0,0,0,1);
   Minv=M.inverse(M);
   _rotation.set(M);

//   cout << "M = " << endl;
//  osgfunc::print_matrix(M);
//  cout.precision(10);
//  cout << "_center = " << get_worldspace_center() << endl;
//  cout << "get_eye_to_center_distance = "
//       << get_eye_to_center_distance() << endl;

   update_M_and_Minv();
   us.requestRedraw();
}

// ---------------------------------------------------------------------
void Custom3DManipulator::Rotate(
   float fx_prev,float fy_prev,float fx_curr,float fy_curr)
{
   if (!disable_rotations_flag)
   {
      osg::Vec3 axis;
      float angle;

// On 11/22/07, we swapped the fy_curr and fy_prev arguments within the call
// to getTrackballRotation so that tilts about the camera's Xhat
// direction vector passing through the screen center point conform
// with Google Earth's:

      if (emulate_GoogleEarth_rotations_flag)
      {
         getTrackballRotation(axis,angle,fx_prev,fy_curr,fx_curr,fy_prev);
      }
      else
      {

// On December 26, 2007, we found that the original argument set for
// getTrackallRotation() makes satellite model viewing much easier
// than the new, Google Earth argument set:

         getTrackballRotation(axis,angle,fx_prev,fy_prev,fx_curr,fy_curr);
      } // emulate_GoogleEarth_rotations_flag
    
      osg::Quat delta_rotation;
      delta_rotation.makeRotate(angle,axis);
      _rotation = _rotation*delta_rotation;

//      Matrix _rotation_matrix(_rotation);

// Copy 3x3 rotation entries into matrix M. Then update 4th row of M
// containing camera's world space position via call to
// update_M_and_Minv():
      
//      M=_rotation_matrix;

      M.set(_rotation);
      update_M_and_Minv();

//      cout << "inside C3DM::Rotate()" << endl;
//      cout << "Camera Zhat = " 
//           << ViewFrustum_ptr->get_camera_Zhat() << endl;

   } // !disable_rotation_flag conditional
}

// --------------------------------------------------------------------------
void Custom3DManipulator::Zoom(float dy)
{
//   cout << "inside C3DM::Zoom()" << endl;
   
   float fd = get_eye_to_center_distance();
   float scale = 1.0+dy;
   if (fd*scale>_modelScale*_minimumZoomScale)
   {
      set_eye_to_center_distance(fd*scale);
   }
   else
   {
      float scale = -fd;
      osg::Matrix rotation_matrix(_rotation);
      osg::Vec3 dv = (osg::Vec3(0.0f,0.0f,-1.0f)*rotation_matrix)*
         (dy*scale);
      set_worldspace_center(get_worldspace_center()+dv);
   }

//   cout << "eye_to_center_dist = " << get_eye_to_center_distance() << endl;
//   cout << "center = " 
//        << get_worldspace_center().get(0) << " , " 
//        << get_worldspace_center().get(1) << " , " 
//        << get_worldspace_center().get(2) << endl;

   update_M_and_Minv();
}

// ---------------------------------------------------------------------
void Custom3DManipulator::Translate(
   float fx_prev,float fy_prev,float fx_curr,float fy_curr)
{
   return Translate(fx_prev-fx_curr,fy_prev-fy_curr);
}

void Custom3DManipulator::Translate(float d_fx,float d_fy)
{
//   cout << "inside Custom3DManipulator::Translate()" << endl;
//   cout << "d_fx = " << d_fx << " d_fy = " << d_fy << endl;

   float scale = -0.3 * get_eye_to_center_distance();
   osg::Vec3 dv(d_fx*scale,d_fy*scale,0);
    
//   osg::Matrix rotation_matrix;
//   rotation_matrix.set(_rotation);
//   set_center(dv*rotation_matrix+get_center());
//   M=rotation_matrix;
    
   M.set(_rotation);
   set_center(dv*M+get_center());
   update_M_and_Minv();
}

// --------------------------------------------------------------------------
void Custom3DManipulator::update_M_and_Minv()
{
//    cout << "inside Custom3DManipulator::update_M_and_Minv()" << endl;
   threevector Zhat_camera(M(2,0) , M(2,1) , M(2,2));
   threevector eye_posn=get_worldspace_center()+
      get_eye_to_center_distance()*Zhat_camera;

   M.set(M(0,0) , M(0,1) , M(0,2) , M(0,3) ,
         M(1,0) , M(1,1) , M(1,2) , M(1,3) ,
         M(2,0) , M(2,1) , M(2,2) , M(2,3) ,
         eye_posn.get(0) , eye_posn.get(1) , eye_posn.get(2), 1 );
   Minv=M.inverse(M);

//   cout << "M = " << endl;
//   osgfunc::print_matrix(M);
//   cout << "Minv = " << endl;
//   osgfunc::print_matrix(Minv);

   set_ViewMatrix(Minv);

   update_viewfrustum();
}

// ---------------------------------------------------------------------
// Member function update_viewfrustum

void Custom3DManipulator::update_viewfrustum()
{
//   cout << "inside Custom3DManipulator::update_viewfrustum()" << endl;
   ViewFrustum_ptr->compute_params_planes_and_vertices();
}

// =====================================================================
// Camera position & orientation member functions
// =====================================================================

// Member function compute_camera_to_screen_center_distance() computes
// the distance between the screen center point and the camera and
// stores the result within member variable log_eye_center_dist.

double Custom3DManipulator::compute_camera_to_screen_center_distance(
   const threevector& screen_center_intercept)
{
   double eye_center_dist=(ViewFrustum_ptr->get_camera_posn()-
                           screen_center_intercept).magnitude();
   log_eye_center_dist=log10(eye_center_dist);

//   cout << " log_eye_center_dist = " << log_eye_center_dist << endl;
   return log_eye_center_dist;
}

// ---------------------------------------------------------------------
// Member function compute_approx_projected_north_hat() assumes that
// world-space x_hat and y_hat represent decent approximations to the
// true east and north direction vector.  (This should be true for
// relatively small longitude-latitude patches on the earth.)  It
// projects x_hat and y_hat onto the virtual camera's U_hat, V_hat and
// W_hat basis. This method computes the average of the azimuthal
// directions corresponding to n_hat and e_hat + 90 degs.  When the 3D
// map is viewed from a nadir orientation, these two angles are equal.
// But when perspective effects become large as the viewing angle
// deviates signficantly from nadir, these two angles can
// substantially differ.  This method returns the average of the two
// angles as a reasonable estimate for north_hat's azimuthal angle.  

// We wrote this method in June 2009 to implement a compass object for
// the LOS project.

double Custom3DManipulator::compute_approx_projected_north_hat_azimuth()
{
   return compute_approx_projected_north_hat_azimuth(Minv);
}

double Custom3DManipulator::compute_approx_projected_north_hat_azimuth(
   const osg::Matrixd& M_inverse)
{
//   cout << "inside Custom3DManipulator::compute_approx_projected_north_hat()" 
//        << endl;
//   cout << "Minv = " << endl;
//   osgfunc::print_matrix(Minv);

   double e_x=M_inverse(0,0);
   double e_y=M_inverse(0,1);
   double n_x=M_inverse(1,0);
   double n_y=M_inverse(1,1);
   double north_azimuth=atan2(n_y,n_x);

   double east_azimuth=atan2(e_y,e_x);
   double north2_azimuth=east_azimuth+osg::PI/2;
   north2_azimuth=basic_math::phase_to_canonical_interval(
      north2_azimuth,north_azimuth-osg::PI,north_azimuth+osg::PI);
//   cout << "projected n_hat = " << projected_north_hat << endl;

//   cout << "north_azimuth = " << north_azimuth*180/osg::PI << endl;
//   cout << "north2_azimuth = " << north2_azimuth*180/osg::PI << endl;

   double azimuth=0.5*(north_azimuth+north2_azimuth);
//   cout << "azimuth = " << azimuth*180/osg::PI << endl;

   return azimuth;
}

// ---------------------------------------------------------------------
// Member function compute_camera_az() computes the virtual camera's
// instantaneous azimuth wrt global +X_hat.  Recall az = 90 if the
// camera points along global +Y_hat.  This method returns the azimuth
// in radians.

double Custom3DManipulator::compute_camera_az()
{
//   cout << "-camera_Zhat = " << -get_camera_Zhat() << endl;
   double camera_az=atan2(
      -get_camera_Zhat().get(1),-get_camera_Zhat().get(0));
//   cout << "camera_az = " << camera_az*180/osg::PI << endl;
   return camera_az;
}

// ---------------------------------------------------------------------
// Member function get_curr_az_el_roll() returns the virtual camera's
// azimuth, elevation and roll angles in radians.  Recall az=0 points
// in the +xhat direction, el=0 points towards the horizon, and el=-90
// points in the -z_hat direction.

void Custom3DManipulator::get_curr_az_el_roll(
   double& curr_az,double& curr_el,double& curr_roll)
{
   rotation Rcurr=get_camera_rotation();
//   rotation R_OSG=Rcurr.OSG_rotation_corresponding_to_rotation();

/*
   Uhat=get_camera_Xhat();
   Vhat=get_camera_Yhat();
   What=get_camera_Zhat();
   cout << "Uhat: " << Uhat.get(0) << " , "
        << Uhat.get(1) << " , "
        << Uhat.get(2) << endl;
   cout << "Vhat: " << Vhat.get(0) << " , "
        << Vhat.get(1) << " , "
        << Vhat.get(2) << endl;
   cout << "What: " << What.get(0) << " , "
        << What.get(1) << " , "
        << What.get(2) << endl;
   cout << "Rcurr = " << Rcurr << endl;
*/

//   rotation R0;
//   R0.clear_values();
//   R0.put(0,2,-1);
//   R0.put(1,0,-1);
//   R0.put(2,1,1);

//   rotation R_camera=R0.transpose() * Rcurr.transpose();
//   rotation R_camera=R0.transpose() * Rcurr;
//   cout << "R_camera = " << R_camera << endl;

   rotation Rtrans=Rcurr.transpose();

   fourvector q_OSG=Rcurr.OSG_quat_corresponding_to_rotation();
   fourvector qtrans_OSG=Rtrans.OSG_quat_corresponding_to_rotation();

   cout << "q_OSG = " << q_OSG << endl;
   cout << "qtrans_OSG = " << qtrans_OSG << endl;

   double az,el,roll;
   Rtrans.az_el_roll_from_rotation(az,el,roll);
//   Rcurr.az_el_roll_from_rotation(curr_az,curr_el,curr_roll);
//   Rcurr.transpose().az_el_roll_from_rotation(curr_az,curr_el,curr_roll);
//   R_camera.az_el_roll_from_rotation(curr_az,curr_el,curr_roll);
//   cout << "curr_az = " << curr_az*180/osg::PI
//        << " curr_el = " << curr_el*180/osg::PI
//        << " curr_roll = " << curr_roll*180/osg::PI << endl;

   curr_az=az+90*osg::PI/180;
   curr_el=roll-90*osg::PI/180;
   curr_roll=el;
//   cout << "Virtual camera's curr az = " << curr_az*180/osg::PI
//        << " curr_el = " << curr_el*180/osg::PI
//        << " curr_roll = " << curr_roll*180/osg::PI << endl;

}

// ---------------------------------------------------------------------
void Custom3DManipulator::reset_Manipulator_control()
{
//   cout << "inside Custom3DManipulator::reset_Manipulator_control()"
//        << endl;
}

// ---------------------------------------------------------------------
osg::Matrixd Custom3DManipulator::getMatrix() const
{
   return M;
}

osg::Matrixd Custom3DManipulator::getInverseMatrix() const
{
   return Minv;
}

// ---------------------------------------------------------------------
threevector Custom3DManipulator::get_eye_world_posn() const
{
   return threevector(M(3,0),M(3,1),M(3,2));
}

// Xhat_camera = Uhat is locked to the screen and points in the
// "right" direction;

threevector Custom3DManipulator::get_camera_Xhat() const
{
   return threevector(M(0,0),M(0,1),M(0,2));
}

// Yhat_camera = Vhat is locked to the screen and points in the "up"
// direction;

threevector Custom3DManipulator::get_camera_Yhat() const
{
   return threevector(M(1,0),M(1,1),M(1,2));
}

// Zhat_camera = What is locked to the screen and points in the "out"
// direction;

threevector Custom3DManipulator::get_camera_Zhat() const
{
   return threevector(M(2,0),M(2,1),M(2,2));
}

rotation Custom3DManipulator::get_camera_rotation() const
{
   rotation R;
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         R.put(i,j,M(i,j));
      }
   }
   return R;
}

double Custom3DManipulator::get_camera_height_above_grid() const
{
//   cout << "inside C3DM::get_camera_height_above_grid()" << endl;
   return get_ViewFrustum_ptr()->get_camera_posn().get(2)-
      grid_origin_ptr->get(2);
}


void Custom3DManipulator::set_flying_maneuver_finished_flag(bool flag)
{
}

bool Custom3DManipulator::get_flying_maneuver_finished_flag() const
{
   return true;
}

void Custom3DManipulator::set_rotate_about_current_eyepoint_flag(bool flag)
{
}

bool Custom3DManipulator::get_rotate_about_current_eyepoint_flag() const
{
   return false;
}

void Custom3DManipulator::set_initial_camera_posn(const threevector& posn)
{
}

void Custom3DManipulator::set_initial_camera_rotation(const rotation& R)
{
}

void Custom3DManipulator::set_final_camera_posn(const threevector& posn)
{
}

void Custom3DManipulator::set_final_camera_rotation(const rotation& R)
{
}

int Custom3DManipulator::get_flyout_zoom_counter() const
{
   return 0;
}
         
void Custom3DManipulator::jumpto(
   const threevector& final_posn,const rotation& final_R,
   double final_FOV_u,double final_FOV_v)
{
}

void Custom3DManipulator::flyto(
   const threevector& final_posn,const rotation& final_R,
   bool write_to_file_flag,
   bool no_final_slowdown_flag,
   double final_FOV_u,double final_FOV_v,
   int n_anim_steps,double t_flight)
{
}

void Custom3DManipulator::flyto(
   const threevector& final_posn,const rotation& final_R,
   const threevector& init_posn,const rotation& init_R,
   bool write_to_file_flag,
   bool no_final_slowdown_flag,
   double final_FOV_u,double final_FOV_v,
   int n_anim_steps,double t_flight)
{
}

// =====================================================================
// HMI input device handling member functions
// =====================================================================

// Member function reset_translation()

void Custom3DManipulator::reset_translation(
   double fx_curr,double fy_curr,double fx_prev,double fy_prev)
{
//   cout << "inside Custom3DManipulator::reset_translation()" << endl;

   Translate(fx_curr,fy_curr,fx_prev,fy_prev);
}

// ---------------------------------------------------------------------
// Member function reset_az_el()

void Custom3DManipulator::reset_az_el(double d_az,double d_el)
{
//   cout << "inside Terrain_Manipulator::reset_az_el()" << endl;

   const double az_prefactor=1*2.3;	// WAGONWHEELS

//   const double az_prefactor=1*1.15;	// INSPOINTS
   const double el_prefactor=1*1.15;	// INSPOINTS

//   const double az_prefactor=0.1/1.1;
//   const double el_prefactor=0.1;

   const double min_d_az=5E-4;		// WAGONWHEELS
//   const double min_d_az=0.001;
//   const double min_d_az=0.003;			// INSPOINTS
//   const double min_d_az=0.03;

//   cout << "fabs(d_az) = " << fabs(d_az) << endl;

   double fx_prev=0;
   double fy_prev=0;
   double fx_curr=0;
   double fy_curr=0;

   if (fabs(d_az) > min_d_az)
   {
      fx_curr=az_prefactor*(d_az);
   }
   fy_curr=el_prefactor*(d_el);

//   cout << "fx_curr = " << fx_curr 
//        << " fy_curr = " << fy_curr << endl;

   Rotate(fx_prev,fy_prev,fx_curr,fy_curr);
}

// ---------------------------------------------------------------------
// Member function reset_zoom()

void Custom3DManipulator::reset_zoom(double ds)
{
//   cout << "inside Custom3DManipulator::reset_zoom()" << endl;

   const double zoom_prefactor=1.0;     
   double zoomDist=zoom_prefactor*ds;

   Zoom(zoomDist);
}
