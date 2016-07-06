// =====================================================================
// Terrain_Manipulator class member function definitions
// =====================================================================
// Last updated on 9/11/11; 12/1/11; 2/9/13
// =====================================================================

#include <iostream> 
#include <string>
#include <vector>
#include <osg/Quat>
#include "math/constant_vectors.h"
#include "osg/osgWindow/CustomAnimationPathManipulator.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/mathfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/Transformer.h"

using namespace osg;
using namespace osgGA;
using std::cin;
using std::cout;
using std::deque;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;


void Terrain_Manipulator::allocate_member_objects()
{
   initial_camera_rotation_ptr=new rotation();
   final_camera_rotation_ptr=new rotation();
}		       

void Terrain_Manipulator::initialize_member_objects()
{
   grid_ptr=NULL;
   rotate_about_current_eyepoint_flag=false;
   allow_only_az_rotation_flag=false;
   disallow_zoom_flag=false;
   flying_maneuver_finished_flag=false;
   enable_underneath_looking_flag=false;

   flyout_zoom_counter=0;
   max_camera_height_above_grid_factor=10;
   depth_range=1000;	// meters
   setName("Terrain_Manipulator");

// On Family Day 15 Sep 2007, we discovered that inexperienced users
// should not be allowed to zoom too far in towards or out from the
// grid plane:

// FAKE FAKE:  Weds Sep 21, 2011 at 10:37 am
// switched from 5 to 0.5

   min_camera_height_above_grid=0.5;		// meters
//   min_camera_height_above_grid=5;		// meters

   image_refptr = new osg::Image;
   ColorbarHUD_ptr=NULL;
}

Terrain_Manipulator::Terrain_Manipulator(
   ModeController* MC_ptr,WindowManager* WM_ptr):
   Custom3DManipulator(MC_ptr,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
}

Terrain_Manipulator::Terrain_Manipulator(
   ModeController* MC_ptr,threevector* GO_ptr,WindowManager* WM_ptr):
   Custom3DManipulator(MC_ptr,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   grid_origin_ptr=GO_ptr;
}

Terrain_Manipulator::~Terrain_Manipulator()
{
   delete initial_camera_rotation_ptr;
   delete final_camera_rotation_ptr;
}

// ---------------------------------------------------------------------
// This overloaded version of member function home resets the camera
// so that its X, Y and Z axes are aligned with the world grid's X, Y
// and Z axes.  The camera's position is also set to (0,0,0) in world
// space.

bool Terrain_Manipulator::reset_view_to_home()
{
//   cout << "inside Terrain_Manipulator::reset_view_to_home()" << endl;
   set_rotate_about_current_eyepoint_flag(false);
   home(*ea_ptr,*us_ptr);
   return true;
}

void Terrain_Manipulator::home(
   const GUIEventAdapter& ea ,GUIActionAdapter& us)
{
//   cout << "inside TerrainManipulator::home()" << endl;

   set_rotate_about_current_eyepoint_flag(false);
   if (grid_ptr != NULL)
   {

// Set homed camera's lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

      set_worldspace_center(grid_ptr->get_world_middle());
      set_eye_to_center_distance(
         3*basic_math::max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   }

   if (CompassHUD_ptr != NULL)
   {

// Compass points north whenever Terrain_Manipulator is homed:

      double az=90*osg::PI/180;
      CompassHUD_ptr->rotate_compass(az);
   }
   
   Custom3DManipulator::home(ea,us);
}

// ---------------------------------------------------------------------
// Member function adjust_horiz_vert_fovs() queries the user to enter
// new horizontal and vertical FOV angles in degrees.  It then resets
// the Viewer's fields-of-view based upon these input values.

void Terrain_Manipulator::adjust_horiz_vert_fovs()
{
//   cout << "inside Terrain_Manipulator::adjust_horiz_vert_fovs()" << endl;

   double hfov=WindowManager_ptr->get_lens_horizontal_FOV();
   double vfov=WindowManager_ptr->get_lens_vertical_FOV();
   cout << "Current horizontal FOV = " << hfov << endl;
   cout << "Current vertical FOV = " << vfov << endl;

   cout << "Enter new horizontal FOV in degs:" << endl;
   cin >> hfov;
   cout << "Enter new vertical FOV in degs:" << endl;
   cin >> vfov;

   WindowManager_ptr->set_viewer_horiz_vert_fovs(hfov,vfov);
}

// ---------------------------------------------------------------------
// Member function compute_screen_intercept returns the intercept
// world location of screen ray corresponding to input screen coords
// (fx,fy) with the z-grid world plane.  It also computes the camera's
// distance to this intercept point.

threevector Terrain_Manipulator::compute_screen_intercept(
   double fx,double fy,double& camera_to_center_distance)
{
//   cout << "inside Terrain_Manipulator::compute_screen_intercept()"  << endl;
//   cout << "rotate_about_current_eyepoint_flag = "
//        << rotate_about_current_eyepoint_flag << endl;

   threevector screen_center_intercept;
   if (!Transformer_ptr->compute_screen_ray_intercept_with_zplane(
      fx,fy,grid_origin_ptr->get(2),screen_center_intercept) ||
       rotate_about_current_eyepoint_flag)
   {
      screen_center_intercept=Transformer_ptr->
         compute_screen_ray_forward_position(fx,fy,depth_range);
   }

   camera_to_center_distance=(get_eye_world_posn()-screen_center_intercept).
      magnitude();
//   cout << "camera_to_center_distance = " << camera_to_center_distance
//        << endl;
//   cout << "screen_center_intercept = " << screen_center_intercept
//        << endl;
   return screen_center_intercept;
}

// ---------------------------------------------------------------------
// Member function Translate takes in the normalized fraction
// coordinates of two mouse events -1 <= (fx_curr,fy_curr) &
// (fx_prev,fy_prev) <= +1. It first computes the camera rays
// corresponding to these two imageplane points.  It next computes the
// world coordinates of the intersections of these two rays with the
// grid plane.  The camera translation is set equal to the difference
// between the intercept points.  By construction, the camera's
// altitude remains unchanged.

// This method implements a reasonably fast approximation to Ross
// Anderson's principle that translations should move individual
// trees, houses, cars from one point on the screen plane to another.
// A true implementation would need to compute the world-Z coordinate
// at the initial (U,V) selection point.  But as of Aug 2007, we're
// willing to live with some perspective corruption of camera
// translation movements in order to have a fast, simple translation
// method...

void Terrain_Manipulator::Translate(
   float fx_curr,float fy_curr,float fx_prev,float fy_prev)
{
//   cout << "inside Terrain_Manipulator::Translate(fx_curr,fy_curr,fx_prev,fy_prev)" << endl;
//   cout << "fx_curr = " << fx_curr << " fy_curr = " << fy_curr
//        << " fx_prev = " << fx_prev << " fy_prev = " << fy_prev << endl;
//   cout << "rotate_about_current_eyepoint_flag = "
//        << rotate_about_current_eyepoint_flag << endl;

// To prevent accidental and unwanted misalignments of the virtual
// camera with OBSFRUSTA image planes during touchtable operations, we
// forbid any translation from being performed if the virtual camera
// is rotating about current eyepoint and if no zoom away from this
// eyepoint location has been intentionally performed...
   
   if (rotate_about_current_eyepoint_flag) return;

//   cout << "fabs(fx_prev-fx_curr) = " << fabs(fx_prev-fx_curr) << endl;
//   cout << "fabs(fy_prev-fy_curr) = " << fabs(fy_prev-fy_curr) << endl;

//   cout << "fx_curr = " << fx_curr << " fx_prev = " << fx_prev << endl;
//   cout << "fy_curr = " << fy_curr << " fy_prev = " << fy_prev << endl;
//   cout << "fabs(fx_curr-fx_prev) = " << fabs(fx_curr-fx_prev) << endl;
//   cout << "fabs(fy_curr-fy_prev) = " << fabs(fy_curr-fy_prev) << endl;

/*
// FAKE FAKE:  Monday Sept 5, 2011 at 4:30 pm
// Experiment with hardwiring input fx,fy values for Wiimote
// simulation purposes...

   const double f_curr=0.004;
   if (fx_curr-fx_prev > 0)
   {
      fx_curr=f_curr;
      fx_prev=0;
   }
   else if (fx_curr-fx_prev < 0)
   {
      fx_curr=-f_curr;
      fx_prev=0;
   }
   if (fy_curr-fy_prev > 0)
   {
      fy_curr=f_curr;
      fy_prev=0;
   }
   else if (fy_curr-fy_prev < 0)
   {
      fy_curr=-f_curr;
      fy_prev=0;
   }
*/

// To minimize number of jarring, unintended translations which can
// occur on the touchtable when the user intends to perform a rotation
// with two fingers but both do not touch the table at exactly the
// same instant, we forbid any translation from taking place if the
// spatial difference between the current and previous touch locations
// are too widely separated...

   if (fabs(fx_curr-fx_prev) > 0.1 || fabs(fy_curr-fy_prev) > 0.1)
   {
     // cout << endl;
     // cout << "****************************" << endl;
     // cout << "Discontinuous jump detected" << endl;
     // cout << "****************************" << endl;
     // cout << endl;
      return;
   }

   double camera_to_center_dist;
   threevector intercept_pt0=compute_screen_intercept(
      fx_curr,fy_curr,camera_to_center_dist);
   threevector intercept_pt1=compute_screen_intercept(
      fx_prev,fy_prev,camera_to_center_dist);
   threevector trans=intercept_pt1-intercept_pt0;

   threevector orig_eye_posn=get_eye_world_posn();
   threevector new_eye_posn=orig_eye_posn+trans;
   M(3,0)=new_eye_posn.get(0);
   M(3,1)=new_eye_posn.get(1);
   M(3,2)=new_eye_posn.get(2);

   Minv=M.inverse(M);
   set_ViewMatrix(Minv);
   update_viewfrustum();

// If translated ViewFrustum does not intercept Grid, undo the
// translation so that Grid and its contents always remain visible:

   if (!rotate_about_current_eyepoint_flag && !ViewFrustum_relative_to_Grid())
   {
      M(3,0)=orig_eye_posn.get(0);
      M(3,1)=orig_eye_posn.get(1);
      M(3,2)=orig_eye_posn.get(2);
      Minv=M.inverse(M);
      set_ViewMatrix(Minv);
      update_viewfrustum();
   }

   update_compass_heading();
}

// ---------------------------------------------------------------------
// Member function Rotate spins the camera about the screen
// center intercept point on the z-plane world grid.  As of Aug 2007,
// this method implements Ross Anderson's recommendation that the
// virtual camera exhibit zero "roll" relative to the ground z-plane.
// We also no longer call OSG's trackball manipulator.  Instead,
// vertical mouse movements tilt the ground-plane about screen Xhat.
// Horizontal mouse movements effectively azimuthally spin the
// ground-plane about world Zhat.

void Terrain_Manipulator::Rotate(
   float fx_prev,float fy_prev,float fx_curr,float fy_curr)
{
//   cout << "inside Terrain_Manipulator::Rotate()" << endl;
//   cout << "fx_curr = " << fx_curr << " fx_prev = " << fx_prev << endl;
//   cout << "fy_curr = " << fy_curr << " fy_prev = " << fy_prev << endl;

//   cout <<  "eye_world_posn = " 
//        << get_eye_world_posn() << endl;
//   cout << "rotate_about_current_eyepoint_flag = "
//        << rotate_about_current_eyepoint_flag << endl;

/*
// FAKE FAKE:  Sunday, Sept 4, 2011 at 8:25 pm
// Hardwire fy_curr=fy_prev to simulate Wii interaction for Family Day demo

   fy_curr=fy_prev;
*/

// Do not allow any rotations to be performed if user is in the middle
// of flying out from the OBSFRUSTUM along an auto-generated animation
// path:

   if (rotate_about_current_eyepoint_flag && flyout_zoom_counter > 0)
   {
      return;
   }

// Note added on Oct 17, 2007: We empirically determined that touch
// table response to 2-fingered rotation gestures is much better if
// prefactor==50 rather than 25 provided
// rotate_about_current_eyepoint_flag==false:

   double prefactor=25;
   if (!rotate_about_current_eyepoint_flag)
   {
      prefactor=50;
   }
   
   double az=prefactor*(fx_curr-fx_prev)*3.14159/180.0;
   if (!rotate_about_current_eyepoint_flag && get_mouse_input_device_flag())
   {
      az *= sgn(fy_curr);
   }
//   cout << "az = " << az*180/3.14159 << endl;

   if (rotate_about_current_eyepoint_flag)
   {
      threevector camera_Yhat(get_camera_Yhat());
      osg::Vec3 axisY(camera_Yhat.get(0),camera_Yhat.get(1),
                      camera_Yhat.get(2));
      osg::Quat delta_rotationY;
      delta_rotationY.makeRotate(az,axisY);
      _rotation = _rotation*delta_rotationY;
   }
   else
   {
      
// Perform azimuthal spin about world-Z axis passing through screen
// center point.

      osg::Vec3 axisZ(0,0,1);
      osg::Quat delta_rotationZ;
      delta_rotationZ.makeRotate(az,axisZ);
      _rotation = _rotation*delta_rotationZ;
   }
   
// Next tilt about instantaneous camera Xhat direction vector passing
// through screen center point.  On 9/11/07, we changed the sign on
// the elevation parameter below so that our Terrain_Manipulator's
// movements conform with Google Earth's:

   double elev=-prefactor*(fy_curr-fy_prev)*3.145159/180.0;
 
// For indoor panorama viewing, we may want to limit rotations to just
// occur about the virtual camera's current Z-axis:

//   cout << "allow_only_az_rotation_flag = " << allow_only_az_rotation_flag
//        << endl;
   if (allow_only_az_rotation_flag)
   {
      elev=0;
   }

   threevector camera_Xhat(get_camera_Xhat());
   osg::Vec3 axisX(camera_Xhat.get(0),camera_Xhat.get(1),
                   camera_Xhat.get(2));
   osg::Quat delta_rotationX;
   delta_rotationX.makeRotate(elev,axisX);

// If manipulator is being used to view ground digital photos, any
// user rotation should be allowed.  But if manipulator is being used
// to view terrain, we do not want to allow the user the tilt a
// terrain map so far forward so that world Z_hat projects along
// -Yscreen_hat.  If this is allowed to happen, the sign of azimuthal
// rotations flips and Terrain Manipulator control becomes very
// counter intuitive.  So we simply forbid any rotation about
// Xscreen_hat which would cause world Z_hat to point along
// -Yscreen_hat.

// On Family Day 15 Sep 2007, we discovered that inexperienced users
// should not be allowed to tilt terrain maps so that they can see
// their undersides (no matter what Ross has to say about this
// issue!). Actually, we want to prevent the virtual camera's
// elevation from dipping below some minimal value in order to
// maintain control over its manipulation....

   osg::Matrix candidate_rot_mat=
      get_rotation_matrix(_rotation*delta_rotationX);
//   osgfunc::print_matrix(candidate_rot_mat);

// Note added on 11/25/11: Reset sin_5_degs to NEGATIVEINFINITY in
// order to allow Terrain Manipulator to peer "underneath" a point
// cloud which has a reasonably well defined ground Z-plane...

   double sin_5_degs=0.087155669; // sin(5*3.14159/180);
   if (enable_underneath_looking_flag)
   {
      sin_5_degs=NEGATIVEINFINITY;
   }
  
   if (rotate_about_current_eyepoint_flag)
   {
      _rotation = _rotation*delta_rotationX;
   }
   else if (!rotate_about_current_eyepoint_flag &&
            candidate_rot_mat(1,2) >= 0 &&
            candidate_rot_mat(2,2) >= sin_5_degs)
   {
      _rotation = _rotation*delta_rotationX;
   }
   else
   {
//      cout << "candidate_rot_mat(1,2) = " << candidate_rot_mat(1,2) << endl;
//      cout << "candidate_rot_mat(2,2) = " << candidate_rot_mat(2,2) << endl;
   }

// Rotate the camera about the screen center intercept point:

   osg::Matrix M_orig=M;
   osg::Matrix Minv_orig=Minv;
   osg::Quat _rotation_orig=_rotation;

   double eye_to_rotation_center_dist;
   threevector screen_center_intercept;

   if (rotate_about_current_eyepoint_flag) 
	// (e.g. after flying to apex of some camera OBSFRUSTUM)
   {
      Minv=osg::Matrixd::translate(
         -osg::Vec3(get_eye_world_posn().get(0),
                    get_eye_world_posn().get(1),
                    get_eye_world_posn().get(2)))*
         osg::Matrixd::rotate(_rotation.inverse());
   }
   else
	// rotate about surface point at screen center
   {
      screen_center_intercept=compute_screen_intercept(
         0,0,eye_to_rotation_center_dist);
      Minv=osg::Matrixd::translate(
         -osg::Vec3(screen_center_intercept.get(0),
                    screen_center_intercept.get(1),
                    screen_center_intercept.get(2)))*
         osg::Matrixd::rotate(_rotation.inverse())*
         osg::Matrixd::translate(0,0,-eye_to_rotation_center_dist);
   }
   M=Minv.inverse(Minv);

// Subtract off component of camera Uhat direction vector which points
// along world Zhat.  As Ross Anderson recommends, we want the virtual
// camera's Uhat direction vector to be as level as possible with the
// ground Z-plane when viewing 3D terrain data.  But Uhat must also
// remain orthogonal to the camera's line-of-sight which always points
// towards the center of the screen.  So we must reorthogonalize Uhat
// wrt What after performing the Zhat projection.  Finally, take Vhat
// = What x Uhat to form an orthonormal basis.

   threevector Uhat(get_camera_Xhat());
   Uhat.put(2,0);

   threevector What(get_camera_Zhat());
   Uhat -= (Uhat.dot(What))*What;
   Uhat=Uhat.unitvector();
   threevector Vhat(What.cross(Uhat));

   M.set(Uhat.get(0) , Uhat.get(1) , Uhat.get(2) , 0 ,
         Vhat.get(0) , Vhat.get(1) , Vhat.get(2) , 0 ,
         M(2,0) , M(2,1) , M(2,2) , M(2,3),
         M(3,0),  M(3,1) , M(3,2) , M(3,3) );

// In February 2009, we discovered the hard and painful way that small
// errors can creep into the camera position information stored within
// the last row of matrix M.  When
// rotate_about_current_eyepoint_flag==true, the camera position
// should remain precisely constant.  We therefore explicitly recopy
// the last row of M_orig onto M if
// rotate_about_current_eyepoint_flag==true:

   if (rotate_about_current_eyepoint_flag)
   {
      M.set(M(0,0) , M(0,1) , M(0,2) , M(0,3),
            M(1,0) , M(1,1) , M(1,2) , M(1,3),
            M(2,0) , M(2,1) , M(2,2) , M(2,3),
            M_orig(3,0) ,  M_orig(3,1) , M_orig(3,2) , M_orig(3,3) );
   }
//   cout << "At end of Terrain_Manipulator::Rotate(), M = " << endl;
//   osgfunc::print_matrix(M);

   Minv=M.inverse(M);

   reset_member_quaternion(
      eye_to_rotation_center_dist,screen_center_intercept);
   set_ViewMatrix(Minv);
   update_viewfrustum();

// If manipulator is being used to view terrain and rotated
// ViewFrustum does not intercept Grid, undo the rotation so that Grid
// and its contents always remain visible:

   if (!rotate_about_current_eyepoint_flag && !ViewFrustum_relative_to_Grid())
   {
      M=M_orig;
      Minv=Minv_orig;
      _rotation=_rotation_orig;
      set_ViewMatrix(Minv);
      update_viewfrustum();
   }
   update_compass_heading();

//   double virtual_az,virtual_el,virtual_roll;
//   get_curr_az_el_roll(virtual_az,virtual_el,virtual_roll);
//   cout << "virtual_az = " << virtual_az*180/osg::PI
//        << " virtual_el = " << virtual_el*180/osg::PI
//        << " virtual_roll = " << virtual_roll*180/osg::PI << endl;
}

// --------------------------------------------------------------------------
// Member function Zoom radially translates the camera's position
// in[out]ward along the ray connecting the camera to the screen
// center intercept with the world z-grid.

void Terrain_Manipulator::Zoom(float ds)
{
//   cout << "Inside Terrain_Manip::Zoom(), ds = " << ds << endl;
//   cout << "rotate_about_current_eyepoint_flag = "
//        << rotate_about_current_eyepoint_flag << endl;

// For GOOGLE STREETS viewer, we prohibit "street view"
// Terrain_Manipulator from zooming:

   if (disallow_zoom_flag) return;

// For touchtable demo purposes, we disallow any zooming in if
// rotate_about_curr_eyepoint_flag==true

   if (rotate_about_current_eyepoint_flag)
   {
      if (ds < 0) // zooming in
      {
         return;
      }
      else if (ds > 0) // zooming out
      {
         if (flyout_zoom_counter >= int(camera_posns.size()))
         {
            flyout_zoom_counter=0;
            set_rotate_about_current_eyepoint_flag(false);

            double eye_to_rotation_center_dist;
            threevector screen_center_intercept=compute_screen_intercept(
               0,0,eye_to_rotation_center_dist);
            reset_member_quaternion(
               eye_to_rotation_center_dist,screen_center_intercept);
         }
         else
         {

// Calculate zoom-out path which users are forced to follow when they
// move the virtual camera away from an OBSFRUSTUM's apex:

            if (flyout_zoom_counter==0)
            {
//              bool write_to_file_flag=true;
               bool write_to_file_flag=false;
               bool no_final_slowdown_flag=true;
               int n_anim_steps=75;
//              int n_anim_steps=150;	// Touch table

               rotation Rcamera;
               for (int i=0; i<3; i++)
               {
                  for (int j=0; j<3; j++)
                  {
                     Rcamera.put(i,j,M(i,j));
                  }
               }

               threevector final_eye_posn=
                  final_camera_posn+(get_eye_world_posn()-
                                     initial_camera_posn);

               rotation R0;
               R0.clear_values();
               R0.put(0,2,-1);
               R0.put(1,0,-1);
               R0.put(2,1,1);

//              rotation R_intermediate=R0*get_camera_rotation()*
//                 initial_camera_rotation_ptr->transpose()*R0.transpose();
               rotation R_intermediate=get_camera_rotation()*
                  initial_camera_rotation_ptr->transpose();
//              cout << "R_intermediate = " << R_intermediate << endl;
              
               double az,el,roll;
               R_intermediate.az_el_roll_from_rotation(az,el,roll);
//              cout << "intermediate az = " << az*180/osg::PI
//                   << " el = " << el*180/osg::PI 
//                   << " roll = " << roll*180/osg::PI
//                   << endl;

               rotation final_R=
//                 R0*
//                 R_intermediate*
                  (*final_camera_rotation_ptr)
//                 *R_intermediate.transpose()
//                 *R0.transpose()
                  ;
//              cout << "final_R = " << final_R << endl;

               generate_animation_path(
                  get_eye_world_posn(),Rcamera.transpose(),
                  final_eye_posn,final_R.transpose(),
                  write_to_file_flag,no_final_slowdown_flag,n_anim_steps);

            } // flyout_zoom_counter==0 conditional

            rotation R;
            R.rotation_corresponding_to_quaternion(
               camera_quats[flyout_zoom_counter]);
            R=R.transpose();

            threevector p(camera_posns[flyout_zoom_counter]);

//           cout << "flyout_zoom_counter = "
//                << flyout_zoom_counter << endl;
//           cout << "camera_posn = " << p << endl;
//           cout << "camera_rot = " << R << endl;
           
            M.set(R.get(0,0),R.get(0,1),R.get(0,2),0,
                  R.get(1,0),R.get(1,1),R.get(1,2),0,
                  R.get(2,0),R.get(2,1),R.get(2,2),0,
                  p.get(0),p.get(1),p.get(2),1);
            Minv=M.inverse(M);
            set_ViewMatrix(Minv);
            update_viewfrustum();
           
            flyout_zoom_counter++;
            return;

         } // flyout_zoom_counter >= int(camera_posns.size())
      } // ds > 0 (zooming out) conditional
   } // rotate_about_current_eyepoint_flag conditional

   double camera_to_center_distance;
   threevector screen_center_intercept=compute_screen_intercept(
      0,0,camera_to_center_distance);
//   cout << "screen_center_intercept = " << screen_center_intercept
//        << endl;

// Recall ds > 0 for zooming out, while ds < 0 for zooming in:

   double scale = 1.0+ds;
   threevector p_orig=get_eye_world_posn();
   threevector p=screen_center_intercept+scale*camera_to_center_distance*
      get_camera_Zhat();

   if (rotate_about_current_eyepoint_flag)
   {
      threevector q=p-p_orig;
      const double max_q_mag=10;	// meter
      if (q.magnitude() > max_q_mag)
      {
         p=p_orig+max_q_mag*q.unitvector();
      }
   }

   double delta_x_grid=grid_ptr->get_max_grid_x()-
      grid_ptr->get_min_grid_x();
   double delta_y_grid=grid_ptr->get_max_grid_y()-
      grid_ptr->get_min_grid_y();
   double max_camera_height_above_grid=max_camera_height_above_grid_factor
      *basic_math::max(delta_x_grid,delta_y_grid);
   
   double z_wrt_grid=p.get(2)-grid_origin_ptr->get(2);
//   cout << "z_wrt_grid = " << z_wrt_grid << endl;

// ds > 0 --> zooming out
// ds < 0 --> zooming in

   if (z_wrt_grid < min_camera_height_above_grid) 
   {
//      cout << "z_wrt_grid < min_camera_height_above_grid!" << endl;
//      cout << "-camera_Zhat . zhat = " << -z_hat.dot(get_camera_Zhat())
//           << endl;

      double dotproduct=-z_hat.dot(get_camera_Zhat());
//      cout << "camera_Zhat = " << get_camera_Zhat() << endl;
//      cout << "dotproduct = " << dotproduct << endl;
      
//      if (dotproduct < 0) cout << "Camera points downwards" << endl;
//      if (dotproduct > 0) cout << "Camera points upwards" << endl;

// dotproduct < 0 --> camera points downwards
// dotproduct > 0 --> camera points upwards

      if (ds < 0 && dotproduct <= 0) // zooming in & looking down
      {
         cout << "Zooming in while looking downwards forbidden" << endl;
         return;
      }
      else if (ds > 0 && dotproduct >= 0) // zooming out & looking up
      {
         cout << "Zooming out while looking upwards forbidden" << endl;
         return;
      }
      else if (ds < 0 && dotproduct > 0)	// zooming in & lookup up
      {
//         cout << "Zooming out OK" << endl;
      }
   }

   if (z_wrt_grid > max_camera_height_above_grid)
   {
      cout << "Zoom operation forbidden" << endl;
      return;
   }

   M.set(M(0,0),M(0,1),M(0,2),0,
         M(1,0),M(1,1),M(1,2),0,
         M(2,0),M(2,1),M(2,2),0,
         p.get(0),p.get(1),p.get(2),1);
   Minv=M.inverse(M);
   set_ViewMatrix(Minv);
   update_viewfrustum();

//   cout << "M = " << endl;
//   osgfunc::print_matrix(M);

// If zoomed ViewFrustum does not intercept Grid, undo the zoom
// operation so that Grid and its contents always remain visible:

   if (!rotate_about_current_eyepoint_flag && !ViewFrustum_relative_to_Grid())
   {
      M(3,0)=p_orig.get(0);
      M(3,1)=p_orig.get(1);
      M(3,2)=p_orig.get(2);
      Minv=M.inverse(M);
      set_ViewMatrix(Minv);
      update_viewfrustum();

//      cout << "Zoom operation being reset" << endl;
   }

   update_compass_heading();

//   cout << "At end of Terrain_Manip::zoom()" << endl;
//   cout << "eye posn = " << get_eye_world_posn() << endl;
//   cout << "grid_world_origin = " 
//        << grid_ptr->get_world_origin() << endl;
}

// ---------------------------------------------------------------------
// Auxilliary member function reset_member_quaternion recompuotes
// member quaternion _rotation after the 4x4 matrix M has been adjusted.

void Terrain_Manipulator::reset_member_quaternion(
   double eye_to_rotation_center_dist,
   const threevector& screen_center_intercept)
{
   if (rotate_about_current_eyepoint_flag) 
      // (e.g. after flying to apex of some camera OBSFRUSTUM)
   {
      _rotation.set( M * osg::Matrixd::translate(
         -osg::Vec3(get_eye_world_posn().get(0),
                    get_eye_world_posn().get(1),
                    get_eye_world_posn().get(2))));
   }
   else
   {
      _rotation.set(
         osg::Matrixd::translate(0,0,-eye_to_rotation_center_dist) * 
         M * osg::Matrixd::translate(
            -osg::Vec3(screen_center_intercept.get(0),
                       screen_center_intercept.get(1),
                       screen_center_intercept.get(2)) ) );
   }
}

// ---------------------------------------------------------------------
bool Terrain_Manipulator::parse_keyboard_events(const GUIEventAdapter& ea)
{
   bool keymove = false;

   if (get_ModeController_ptr()->getState()==ModeController::VIEW_DATA)
   {
   }

   return keymove;
}

// =====================================================================
// Animation path member functions
// =====================================================================

// Member function jumpto() is a stripped-down version of flyto().  It
// executes an animation path generated from a single final_posn and
// final_R input.

void Terrain_Manipulator::jumpto(
   const threevector& final_posn,const rotation& final_R,
   double final_FOV_u,double final_FOV_v)
{
//   cout << "inside Terrain_Manipulator::jumpto()" << endl;
//   cout << "final_posn = " << final_posn << endl;
//   cout << "final_R = " << final_R << endl;

  osg::Matrixd Rfinal;
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         Rfinal(i,j)=final_R.get(j,i);
      }
   }
   osg::Quat qfinal;
   qfinal.set(Rfinal);

/*
   M(3,0)=final_posn.get(0);
   M(3,1)=final_posn.get(1);
   M(3,2)=final_posn.get(2);

   M.set(final_R.get(0,0), final_R.get(1,0), final_R.get(2,0), 0,
         final_R.get(0,1), final_R.get(1,1), final_R.get(2,1), 0,
         final_R.get(0,2), final_R.get(1,2), final_R.get(2,2), 0,
         M(3,0),  M(3,1) , M(3,2) , M(3,3) );

   cout << "M = " << endl;
   osgfunc::print_matrix(M);

   Minv=M.inverse(M);
   set_ViewMatrix(Minv);
   setMatrices(M);
   update_viewfrustum();

   set_eye_to_center_distance(1);
   set_worldspace_center(Zero_vector);
*/

// Initialize animation path:

   osg::AnimationPath* animationPath_ptr = new osg::AnimationPath;
   animationPath_ptr->setLoopMode(osg::AnimationPath::NO_LOOPING);
   
   double t0=0;
   animationPath_ptr->insert(
      t0,osg::AnimationPath::ControlPoint(
         osg::Vec3(final_posn.get(0),final_posn.get(1),
                   final_posn.get(2)),qfinal));

// Note added on 9/9/11 at 1:22 pm: Experiment with speeding up two
// insertions into animationPath in order to make jump seem more
// instantaneous:

//   double dt=0.01;	// secs
//   double dt=0.05;	// secs
//   double dt=0.075;	// secs
   double dt=0.1;	// secs
//   double dt=0.25;	// secs
//   double dt=0.33;	// secs

   const int n_iters=4;
   for (int iter=0; iter<n_iters; iter++)
   {
      animationPath_ptr->insert(
         t0+iter*dt,osg::AnimationPath::ControlPoint(
            osg::Vec3(final_posn.get(0),final_posn.get(1),
            final_posn.get(2)),qfinal));
   }
   
   osg::ref_ptr<osgGA::CustomAnimationPathManipulator> apm=
      new osgGA::CustomAnimationPathManipulator(
         animationPath_ptr,WindowManager_ptr,this,final_posn);

   if ( apm.valid() )
   {
      if (WindowManager_ptr != NULL)
      {
         unsigned int num=WindowManager_ptr->set_CameraManipulator(apm.get());
         WindowManager_ptr->selectCameraManipulator(num);

         reset_fields_of_view(apm.get(),final_FOV_u,final_FOV_v);
      } 
   } // apm.valid() conditional

   reset_Manipulator_control();
}

// ---------------------------------------------------------------------
void Terrain_Manipulator::flyto(
   const threevector& final_posn,const rotation& final_R,
   bool write_to_file_flag,bool no_final_slowdown_flag,
   double final_FOV_u,double final_FOV_v,int n_anim_steps,double t_flight)
{
//   cout << "inside TerrainManipulator::flyto(), final_posn = " 
//        << final_posn << endl;

   rotation init_R;
   init_R.put_column(0,get_camera_Xhat());
   init_R.put_column(1,get_camera_Yhat());
   init_R.put_column(2,get_camera_Zhat());

   flyto(
      get_eye_world_posn(),init_R,final_posn,final_R,
      write_to_file_flag,no_final_slowdown_flag,
      final_FOV_u,final_FOV_v,n_anim_steps,t_flight);
}

void Terrain_Manipulator::flyto(
   const threevector& init_posn,const rotation& init_R,
   const threevector& final_posn,const rotation& final_R,
   bool write_to_file_flag,bool no_final_slowdown_flag,
   double final_FOV_u,double final_FOV_v,int n_anim_steps,double t_flight)
{
//   cout << "inside Terrain_Manipulator::flyto()" << endl;

//   cout << "initial_posn = " << init_posn << endl;
//   cout << "init_R = " << init_R << endl;
//   cout << "final_posn = " << final_posn << endl;
//   cout << "final_R = " << final_R << endl;
//   cout << "n_anim_steps = " << n_anim_steps << endl;
//   cout << "final_FOV_u = " << final_FOV_u
//        << " final_FOV_v = " << final_FOV_v << endl;
//   cout << "n_anim_steps = " << n_anim_steps << endl;

   osg::AnimationPath* animationPath_ptr=
      generate_animation_path(
         init_posn,init_R,final_posn,final_R,
         write_to_file_flag,no_final_slowdown_flag,
         n_anim_steps,t_flight);

   osg::ref_ptr<osgGA::CustomAnimationPathManipulator> apm=
      new osgGA::CustomAnimationPathManipulator(
         animationPath_ptr,WindowManager_ptr,this,final_posn);

   if ( apm.valid() )
   {
      apm->getAnimationPath()->setLoopMode(osg::AnimationPath::NO_LOOPING);

      if (WindowManager_ptr != NULL)
      {
         unsigned int num=WindowManager_ptr->set_CameraManipulator(apm.get());
         WindowManager_ptr->selectCameraManipulator(num);

         reset_fields_of_view(apm.get(),final_FOV_u,final_FOV_v);

         set_active_control_flag(false);
      } 
   } // apm.valid() conditional
}

// ---------------------------------------------------------------------
// Member function reset_fields_of_view() sets the virtual camera's
// initial horiz & vert FOV's to their current FOV's.  Set virtual
// camera's final horiz & vert FOV's to member vars final_FOV_u,v if
// they do not equal dummy -1 values.

void Terrain_Manipulator::reset_fields_of_view(
   osgGA::CustomAnimationPathManipulator* apm_ptr,
   double final_FOV_u,double final_FOV_v)
{
//   cout << "inside Terrain_Manipulator::reset_fields_of_view()" << endl;
//   cout << "final_FOV_u = " << final_FOV_u
//        << " final_FOV_v = " << final_FOV_v << endl;

   apm_ptr->set_initial_FOV_u(WindowManager_ptr->get_lens_horizontal_FOV());
   apm_ptr->set_initial_FOV_v(WindowManager_ptr->get_lens_vertical_FOV());

   if (final_FOV_u < 0)
   {
      final_FOV_u=WindowManager_ptr->get_lens_horizontal_FOV();
   }
   if (final_FOV_v < 0)
   {
      final_FOV_v=WindowManager_ptr->get_lens_vertical_FOV();
   }

   apm_ptr->set_final_FOV_u(final_FOV_u);
   apm_ptr->set_final_FOV_v(final_FOV_v);
}

// ---------------------------------------------------------------------
// Member function reset_Manipulator_control()

void Terrain_Manipulator::reset_Manipulator_control()
{
//   cout << "inside Terrain_Manipulator::reset_Manipulator_control()" << endl;
   set_active_control_flag(true);
   set_flying_maneuver_finished_flag(true);
}

// ---------------------------------------------------------------------
// Member function generate_animtion_path fits a spline in time to
// the camera's flight path from its initial to final position and
// attitude.

osg::AnimationPath* Terrain_Manipulator::generate_animation_path(
   const threevector& init_posn,const rotation& init_R,
   const threevector& final_posn,const rotation& final_R,
   bool write_to_file_flag,bool no_final_slowdown_flag,int n_anim_steps,
   double t_flight)
{
//   cout << "inside Terrain_Manipulator::generate_animation_path()" << endl;
//   cout << "final_posn = " << final_posn << endl;

   if (t_flight < 0)
   {

// Next compute camera slew parameters:

      double slew_distance=(final_posn-init_posn).magnitude();
      double slew_speed=flight_slew_rate(slew_distance);

// FAKE FAKE:  thurs, February 10, 2011 at 12:53 pm

//   slew_speed *= 0.125;		// movie_makeing
//   slew_speed *= 0.25;		// movie_makeing
      t_flight=slew_distance/slew_speed;

      t_flight=basic_math::max(t_flight,2.0);	// secs  OK for LOST laptops
      t_flight=basic_math::min(t_flight,6.0);	// secs  OK for LOST laptops


//   t_flight=basic_math::max(t_flight,3.0);	// secs  OK for netbooks
//   t_flight=basic_math::min(t_flight,9.0);	// secs  OK for netbooks

//   t_flight=min(t_flight,18.0);	// secs  movie making
//   t_flight=max(t_flight,2.0);	// secs
//   t_flight=min(t_flight,10.0);	// secs
   }

   return generate_animation_path(
      init_posn,init_R,final_posn,final_R,
      t_flight,write_to_file_flag,no_final_slowdown_flag,n_anim_steps);
}

// ---------------------------------------------------------------------
// In this private, overloaded version of generate_animation_path(),
// input parameter t_flight must be specified and cannot be passed with a
// default argument value.

osg::AnimationPath* Terrain_Manipulator::generate_animation_path(
   const threevector& init_posn,const rotation& init_R,
   const threevector& final_posn,const rotation& final_R,
   double t_flight,bool write_to_file_flag,bool no_final_slowdown_flag,
   int n_anim_steps)
{
//   cout << "inside Terrain_Manipulator::generate_animation_path()" << endl;
//   cout << "final_posn = " << final_posn << endl;

// First compute camera twirl parameters:

   double chi;
   threevector n_hat;
   animation_twirl_params(init_R,final_R,chi,n_hat);

// Times t0, t2 and t4 correspond to starting, middle and stopping
// points in camera slew profile.  Times t1 and t3 represent midpoints
// within spline function used to incorporate acceleration and
// deceleration into the slew profile:

   double t0=0;
   double t4=t_flight;
   double Delta_start=0.18*(t4-t0);
   double Delta_stop=0.18*(t4-t0);
   if (no_final_slowdown_flag)
   {
      Delta_stop *= 0.01;
   }
   double t1=t0+Delta_start;
   double t2=0.5*(t0+t4);
   double t3=t4-Delta_stop;

//   cout << "t0 = " << t0 
//        << " t1 = " << t1
//        << " t2 = " << t2
//        << " t3 = " << t3
//        << " t4 = " << t4 << endl;

   vector<double> t;
   t.push_back(t0);
   t.push_back(t1);
   t.push_back(t2);
   t.push_back(t3);
   t.push_back(t4);
      
   double frac_start=0.05;
   double frac_stop=0.05;
   if (no_final_slowdown_flag)
   {
      frac_stop *= 0.01;
   }
   
   fourvector r0(init_posn,0);
   fourvector r4(final_posn,chi);
   fourvector r2=0.5*(r0+r4);
   fourvector r1=r0+frac_start*(r4-r0);
   fourvector r3=r4-frac_stop*(r4-r0);

   vector<fourvector> XYZc;
   XYZc.push_back(r0);
   XYZc.push_back(r1);
   XYZc.push_back(r2);
   XYZc.push_back(r3);
   XYZc.push_back(r4);

   double dt=0.1;	// secs
//   double dt=0.3;	// secs		movie making
   if (n_anim_steps==-1)
   {
      n_anim_steps=static_cast<int>((t4-t0)/dt+1);
   }
   else
   {
      dt=(t4-t0)/(n_anim_steps-1);
   }
//   cout << "n_anim_steps = " << n_anim_steps << endl;

   vector<double> t_reg;
   for (int n=0; n<n_anim_steps; n++)
   {
      t_reg.push_back(t0+n*dt);
   }

   vector<fourvector> interp_XYZc;
   mathfunc::spline_interp(t,XYZc,t_reg,interp_XYZc);

// Initialize animation path:

   osg::AnimationPath* animationPath_ptr = new osg::AnimationPath;
   animationPath_ptr->setLoopMode(osg::AnimationPath::NO_LOOPING);

   ofstream outputstream;
   outputstream.precision(12);
   if (write_to_file_flag)
   {
      string output_filename="terrain_manip_flyby.path";
      filefunc::openfile(output_filename,outputstream);
   }

   camera_posns.clear();
   camera_quats.clear();

   rotation curr_R;
   osg::Quat q;
   for (int n=0; n<n_anim_steps; n++)
   {

// Interpolate between camera's initial orientation and current east,
// north and radial directions:

      double curr_chi=interp_XYZc[n].get(3);
      interpolate_camera_orientation(init_R,curr_chi,n_hat,q,curr_R);

      camera_posns.push_back(threevector(interp_XYZc[n].get(0),
                                         interp_XYZc[n].get(1),
                                         interp_XYZc[n].get(2)));

//      cout << "n = " << n << " init_R = " << init_R << endl;
//      cout << "curr_chi = " << curr_chi*180/osg::PI << endl;
//      cout << "n_hat = " << n_hat << endl;
//      cout << "curr_R = " << curr_R << endl;
      
      camera_quats.push_back(
         curr_R.quaternion_corresponding_to_rotation());
      
//      cout << "OSG quat = " 
//           << q._v[0] << " "
//           << q._v[1] << " "
//           << q._v[2] << " "
//           << q._v[3] << " " 
//           << endl;

//      cout << "Fourvector quat = " << camera_quats.back() << endl;
//      outputfunc::enter_continue_char();

      rotation R;
      R.rotation_corresponding_to_quaternion(camera_quats.back());

      animationPath_ptr->insert(t_reg[n],osg::AnimationPath::ControlPoint(
         osg::Vec3(interp_XYZc[n].get(0),interp_XYZc[n].get(1),
                   interp_XYZc[n].get(2)),q));

      if (write_to_file_flag)
      {
         outputstream << t_reg[n] << " "
                      << interp_XYZc[n].get(0) << " "
                      << interp_XYZc[n].get(1) << " "
                      << interp_XYZc[n].get(2) << " "
                      << q._v[0] << " "
                      << q._v[1] << " "
                      << q._v[2] << " "
                      << q._v[3] << " " 
                      << endl;
      }
      
   } // loop over index n labeling interpolation time steps

// In order to be absolutely certain that the OSG camera's position
// and attitude at the time when it arrives at its destination match
// those passed into this method as arguments, we force the former to
// equal the latter:

   osg::Matrixd Rfinal;
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         Rfinal(i,j)=final_R.get(j,i);
      }
   }
   osg::Quat qfinal;
   qfinal.set(Rfinal);

//   cout << "qfinal = " 
//        << qfinal._v[0] << " "
//        << qfinal._v[1] << " "
//        << qfinal._v[2] << " "
//        << qfinal._v[3] << " " << endl;
   
   animationPath_ptr->insert(
      t_reg.back()+dt,osg::AnimationPath::ControlPoint(
         osg::Vec3(final_posn.get(0),final_posn.get(1),
                   final_posn.get(2)),qfinal));
   animationPath_ptr->insert(
      t_reg.back()+2*dt,osg::AnimationPath::ControlPoint(
         osg::Vec3(final_posn.get(0),final_posn.get(1),
                   final_posn.get(2)),qfinal));

   if (write_to_file_flag)
   {
      outputstream << t_reg.back()+dt << " "
                   << final_posn.get(0) << " "
                   << final_posn.get(1) << " "
                   << final_posn.get(2) << " "
                   << qfinal._v[0] << " "
                   << qfinal._v[1] << " "
                   << qfinal._v[2] << " "
                   << qfinal._v[3] << " " 
                   << endl;
      outputstream << t_reg.back()+2*dt << " "
                   << final_posn.get(0) << " "
                   << final_posn.get(1) << " "
                   << final_posn.get(2) << " "
                   << qfinal._v[0] << " "
                   << qfinal._v[1] << " "
                   << qfinal._v[2] << " "
                   << qfinal._v[3] << " " 
                   << endl;
      outputstream.close();
   }
   
   return animationPath_ptr;
}

// ---------------------------------------------------------------------
// Member function animation_twirl_params takes in the initial and
// final poses for the camera within input matrices init_R and
// final_R.  This method computes the rotation matrix twirl_R which
// maps the former into the latter.  It then returns the axis n_hat
// and angle chi corresponding to twirl_R.

void Terrain_Manipulator::animation_twirl_params(
   const rotation& init_R,const rotation& final_R,
   double& chi,threevector& n_hat)
{
//   cout << "inside Terrain_Manipulator::animation_twirl_params()" << endl;

   threevector U_init,V_init,W_init,U_final,V_final,W_final;
   init_R.get_column(0,U_init);
   init_R.get_column(1,V_init);
   init_R.get_column(2,W_init);

   final_R.get_column(0,U_final);
   final_R.get_column(1,V_final);
   final_R.get_column(2,W_final);

   rotation twirl_R;
   twirl_R.rotation_taking_pqr_to_uvw(
      U_init,V_init,W_init,U_final,V_final,W_final);

// In Aug 2009, we empirically found that setting tiny entries in
// twirl_R to zero seems to yield n_hat w/o NAN entries:

   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         if (nearly_equal(twirl_R.get(i,j),0))
         {
            twirl_R.put(i,j,0);
         }
      }
   }

   mathfunc::decompose_orthogonal_matrix(twirl_R,chi,n_hat);

   if (mathfunc::my_isnan(n_hat.get(0)) ||
       mathfunc::my_isnan(n_hat.get(1)) ||
       mathfunc::my_isnan(n_hat.get(2)) )
   {
      cout << "inside Terrain_Manipulator::animation_twirl_params()" << endl;
      cout << "Real n_hat = " << n_hat << endl;      
      cout << "init_R = " << init_R << endl;
      cout << "final_R = " << final_R << endl;
      cout << "twirl_R = " << twirl_R << endl;
      cout << "chi = " << chi*180/osg::PI << endl;
      cout << "Reset n_hat to z_hat" << endl;
      outputfunc::enter_continue_char();
      n_hat=z_hat;
   }
}
// ---------------------------------------------------------------------
// Member function flight_slew_rate returns the camera's slewing rate
// which depends upon the distance delta_theta between the starting
// and stopping locations.

double Terrain_Manipulator::flight_slew_rate(double slew_distance)
{
//   cout << "inside Terrain_Manipulator::flight_angular_speed" << endl;
//   cout << "slew_distance = " << slew_distance << endl;

   double slew_rate;
   double factor=2500;
   if (slew_distance > 100*1000)
   {
      slew_rate=10*factor;  // meters/sec
   }
   else if (slew_distance > 10*1000)
   {
      slew_rate=1*factor;
   }
   else if (slew_distance > 3.33*1000)
   {
      slew_rate=.2*factor;
   }
   else if (slew_distance > 1*1000)
   {
      slew_rate=.08*factor;
   }
   else if (slew_distance > 1*100)
   {
      slew_rate=.03*factor;
   }
   else 
   {
      slew_rate=0.01*factor;
   }
//   cout << "slew_rate = " << slew_rate << endl;

// As of 7/5/09, we find the virtual camera slew rate to be too slow.
// So we'll increase its rate in order for flying into OBSFRUSTA to
// not take so long...

   slew_rate *= 1.5;
//   slew_rate *= 2;

   return slew_rate;
}

// ---------------------------------------------------------------------
// Interpolate between camera's initial orientation and current east,
// north and radial directions:

void Terrain_Manipulator::interpolate_camera_orientation(
   const rotation& init_R,double curr_chi,
   const threevector& n_hat,osg::Quat& q,rotation& curr_R)
{

// Columns of input matrix init_R correspond to Uhat, Vhat and What
// expressed in terms of world basis vectors Xhat, Yhat and Zhat:

//   cout << "inside Terrain_Manipulator::interpolate_camera_orientation()"
//        << endl;
//   cout << "init_R = " << init_R << endl;
//   cout << "curr_chi = " << curr_chi*180/3.14159 << endl;
   
   rotation delta_R;
   delta_R.rotation_about_nhat_by_theta(curr_chi,n_hat);
   curr_R=delta_R*init_R;

//   cout << "delta_R = " << delta_R << endl;
//   cout << "curr_R = " << curr_R << endl;
//   outputfunc::enter_continue_char();

   osg::Matrix M_tmp;
   M_tmp.set(curr_R.get(0,0),curr_R.get(1,0),curr_R.get(2,0),0,
             curr_R.get(0,1),curr_R.get(1,1),curr_R.get(2,1),0,
             curr_R.get(0,2),curr_R.get(1,2),curr_R.get(2,2),0,
             0,0,0,1);
   q.set(M_tmp);
}

// =====================================================================
// Grid member functions
// =====================================================================

// Member function ViewFrustum_relative_to_Grid checks whether the
// current ViewFrustum's rays projected into the Grid plane lie inside
// the Grid.  It also checks whether the Grid's corner points lie
// within the ViewFrustum.  If neither condition is satisfied, the
// Grid lies outside the current ViewFrustum and is consequently
// invisible to the virtual camera.  This boolean method returns false
// in that case.

bool Terrain_Manipulator::ViewFrustum_relative_to_Grid()
{
//   cout << "inside Terrain_Manipulator::ViewFrustum_relative_to_Grid()" 
//        << endl;
   
   bool VF_inside_Grid_flag=false;
   bool Grid_inside_VF_flag=false;

//   const double edge_frac=0.00;
   const double edge_frac=0.02;

   threevector relative_grid_world_max=
      grid_ptr->get_world_maximum()-grid_ptr->get_world_origin();

// Compute ViewFrustum's ray intercepts with grid's z-plane.  Check
// whether any ray's intercept falls inside grid:

   threevector camera_posn(get_ViewFrustum_ptr()->get_camera_posn());
   double z_grid=grid_origin_ptr->get(2);

   for (int r=-1; r<=1; r++)
   {
      for (int c=-1; c<=1; c++)
      {
         threevector curr_ray(get_ViewFrustum_ptr()->get_ray(r,c));
         double lambda=(z_grid-camera_posn.get(2))/curr_ray.get(2);
         threevector zplane_intercept(
            camera_posn+lambda*curr_ray-(*grid_origin_ptr));
         double x_intercept=zplane_intercept.get(0);
         double y_intercept=zplane_intercept.get(1);

//         cout << "r = " << r << " c = " << c << endl;
//         cout << "curr_ray = " << curr_ray << endl;

//         cout << "x_intercept = " << x_intercept
//              << " y_intercept = " << y_intercept << endl;

//         cout << "grid world origin = " << *grid_origin_ptr << endl;
//         cout << "grid world middle = " << grid_ptr->get_world_middle()
//              << endl;
//         cout << "grid world max = " << grid_ptr->get_world_maximum() << endl;
//         cout << "relative grid world max = " 
//              << relative_grid_world_max << endl;

         if (x_intercept > edge_frac*relative_grid_world_max.get(0) &&
             x_intercept < (1-edge_frac)*relative_grid_world_max.get(0) &&
             y_intercept > edge_frac*relative_grid_world_max.get(1) &&
             y_intercept < (1-edge_frac)*relative_grid_world_max.get(1))
         {
            VF_inside_Grid_flag=true;
//            cout << "r = " << r << " c = " << c << " ray lies in grid"
//                 << endl;
         }

      } // loop over index c labeling column loop
   } // loop over index r labeling row loop

// Check whether any Grid corner's 2D screen coordinates lies within
// visible region -(1-edge_frac) < X_scrn,Y_scrn < (1-edge_frac) : 

   compute_Grid_corner_screen_coords();
   if (ColorbarHUD_ptr != NULL)
   {
      check_Grid_overlap_with_colorbar();
   }

   for (int c=0; c<=4; c++)
   {
//      threevector Grid_corner=grid_ptr->get_corner(c);
//      threevector grid_corner_scrn=Transformer_ptr->
//         world_to_screen_transformation(Grid_corner);
//      double grid_corner_scrn_x=grid_corner_scrn.get(0);
//      double grid_corner_scrn_y=grid_corner_scrn.get(1);

      double grid_corner_scrn_x=Grid_corner_screen_coords[c].get(0);
      double grid_corner_scrn_y=Grid_corner_screen_coords[c].get(1);
//      cout << "c = " << c 
//           << " grid corner scrn x = " << grid_corner_scrn_x
//           << " grid corner scrn y = " << grid_corner_scrn_y << endl;

      if (grid_corner_scrn_x > -(1-edge_frac) && 
          grid_corner_scrn_x < (1-edge_frac) &&
          grid_corner_scrn_y > -(1-edge_frac) && 
          grid_corner_scrn_y < (1-edge_frac))
      {
         Grid_inside_VF_flag=true;
      }
   }
 
   bool VF_relative_to_Grid_OK_flag=VF_inside_Grid_flag ||
      Grid_inside_VF_flag;

//   cout << "VF_inside_Grid_flag = " << VF_inside_Grid_flag << endl;
//   cout << "Grid_inside_VF_flag = " << Grid_inside_VF_flag << endl;
//   cout << "VF_relative_to_Grid_OK_flag = " << VF_relative_to_Grid_OK_flag
//        << endl;

   return VF_relative_to_Grid_OK_flag;
}

// ---------------------------------------------------------------------
// Member function update_compass_heading()

void Terrain_Manipulator::update_compass_heading()
{
//   cout << "inside Terrain_Manipulator::update_compass_heading()" << endl;

   if (CompassHUD_ptr != NULL)
   {
      if (CompassHUD_ptr->get_nadir_oriented_compass_flag())
      {
         double azimuth=compute_approx_projected_north_hat_azimuth();
         CompassHUD_ptr->rotate_compass(azimuth);
      }
      else
      {
         double theta=(osg::PI-compute_camera_az());
         CompassHUD_ptr->rotate_compass(theta);
      }
   }

/*
// FAKE FAKE:  Fri Nov 27, 2009 at 8 pm
// Print out ViewMatrix and ProjMatrix for alg debugging only...

   
   osg::Matrix VM=WindowManager_ptr->getViewMatrix();
   osg::Matrix PM=WindowManager_ptr->getProjectionMatrix();
   
   cout << "ProjMatrix = " << endl;
   osgfunc::print_matrix(PM);

   cout << "ViewMatrix = " << endl;
   osgfunc::print_matrix(VM);
*/

}

// ---------------------------------------------------------------------
// Member function compute_Grid_corner_screen_coords() updates member
// STL vector Grid_corner_screen_coords.

void Terrain_Manipulator::compute_Grid_corner_screen_coords()
{
//   cout << "inside Terrain_Manipulator::Grid_corner_screen_coords
//        << endl;

   Grid_corner_screen_coords.clear();
   for (int c=0; c<=8; c++)
   {
      threevector grid_corner_scrn(
         Transformer_ptr->world_to_screen_transformation(
            grid_ptr->get_corner(c)));
      Grid_corner_screen_coords.push_back(grid_corner_scrn);
//      cout << "c = " << c 
//           << " grid corner scrn x = " << grid_corner_scrn.get(0)
//           << " grid corner scrn y = " << grid_corner_scrn.get(1) << endl;
   }
}

// ---------------------------------------------------------------------
// Member function check_Grid_overlap_with_colorbar() checks whether
// any of the screen x values for the Grid's 9 "corner" points exceeds
// max_grid_x.  If so, this method masks the *ColorbarHUD_ptr.  If
// not, *ColorbarHUD_ptr is unmasked.

void Terrain_Manipulator::check_Grid_overlap_with_colorbar()
{
//   cout << "inside Terrain_Manipulator::check_Grid_overlap_with_colorbar()"
//        << endl;

   const double max_grid_x=0.8;
   bool hide_colorbar_flag=false;

   for (int c=0; c<=8; c++)
   {
      double grid_corner_screen_x=
         Grid_corner_screen_coords[c].get(0);
//      cout << "c = " << c << " grid_corner_screen_x = "
//           << grid_corner_screen_x << endl;
      if (grid_corner_screen_x > max_grid_x) hide_colorbar_flag=true;
   }

   if (hide_colorbar_flag)
   {
      ColorbarHUD_ptr->set_nodemask(0);
   }
   else
   {
      ColorbarHUD_ptr->set_nodemask(1);
/*
      for (int c=0; c<4; c++)
      {
         double grid_corner_screen_x=
            Grid_corner_screen_coords[c].get(0);
         cout << "c = " << c << " grid_corner_screen_x = "
              << grid_corner_screen_x << endl;
      }
*/

   }
}
