// =====================================================================
// EarthManipulator class member functions handle non-feature
// manipulation events.
// =====================================================================
// Last updated on 6/19/08; 10/27/08; 3/9/09; 12/4/10
// =====================================================================

#include <iostream> 
#include <string>
#include <vector>
#include <osg/Quat>
#include "astro_geo/Clock.h"
#include "math/constants.h"
#include "osg/osgWindow/CustomAnimationPathManipulator.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "astro_geo/geofuncs.h"
#include "math/mathfuncs.h"
#include "osg/ModeController.h"
#include "math/rotation.h"
#include "math/threevector.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"
#include "osg/osgfuncs.h"

using namespace osg;
using namespace osgGA;
using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void EarthManipulator::allocate_member_objects()
{
}		       

void EarthManipulator::initialize_member_objects()
{
   Ellipsoid_model_ptr=NULL;
   Clock_ptr=NULL;
   setName("EarthManipulator");
}

EarthManipulator::EarthManipulator(
   ModeController* MC_ptr,Ellipsoid_model* emodel_ptr,Clock* clock_ptr,
   WindowManager* WM_ptr):
   Custom3DManipulator(MC_ptr,WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Ellipsoid_model_ptr=emodel_ptr;
   Clock_ptr=clock_ptr;
   WindowManager_ptr=WM_ptr;
}

EarthManipulator::~EarthManipulator()
{
}

// ---------------------------------------------------------------------
bool EarthManipulator::parse_mouse_events(const GUIEventAdapter& ea)
{
//   cout << "inside EarthManipulator::parse_mouse_events()" << endl;
   bool mousemove=false;

   if (! (_ga_t0.get()==NULL || _ga_t1.get()==NULL))
   {    
      float fx0 = _ga_t0->getXnormalized();
      float fy0 = _ga_t0->getYnormalized();
      float fx1 = _ga_t1->getXnormalized();
      float fy1 = _ga_t1->getYnormalized();
        
      float d_fx = fx0-fx1;
      float d_fy = fy0-fy1;

//      float dist2 = (sqr(d_fx) + sqr(d_fy));
//      float dt = _ga_t0->time() - _ga_t1->time(); // use later? 
  
// Return if there is no movement:

      if (d_fx==0 && d_fy==0) 
      {
         return mousemove;
      }
        
      unsigned int buttonMask = _ga_t1->getButtonMask();
      switch( buttonMask )
      {

// On 9/13/07, we changed the mapping of the left, middle and right
// mouse buttons to conform with Google Earth's conventions:

// Do not manipulate camera position or orientation if left or right
// shift keys are depressed.  Regard shift as a toggle for entering
// into 3D object selection mode:
            
// Translate in latitude and longitude if right mouse button is dragged:

         case (GUIEventAdapter::LEFT_MOUSE_BUTTON): 
         {
            if (!(ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT))
            {
               spin_about_earth_center(fx1,fy1,fx0,fy0);
               mousemove=true;
               break;
            } // shift keys not depressed conditional
         }

         case (GUIEventAdapter::MIDDLE_MOUSE_BUTTON): 
         {

// Spin about nadir point on earth ellipsoid if middle mouse button is
// dragged and left & right shift keys are NOT depressed.  Regard
// shift as a toggle for entering into 3D object selection mode:

            if (!(ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT))
            {
               spin_about_nadir(fx1,fy1,fx0,fy0);
               mousemove=true;
               break;
            }
         }
  
// Zoom in radial direction if right mouse button is dragged:
          
         case (GUIEventAdapter::RIGHT_MOUSE_BUTTON):
         {
            if (!(ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT))
            {
//                Zoom(-d_fy);
               Zoom(d_fy);
               mousemove=true;
               break;
            }
         }
       
      } // switch buttommask
   }

   return mousemove;
}

// ---------------------------------------------------------------------
// This overloaded version of member function home sets the camera's
// position so that it is located at the specified
// (longitude,latitude,altitude) point independent of the earth's
// diurnal rotation about its axis.  Moreover, the camera's Xhat basis
// vector points east, Yhat basis vector points north and Zhat basis
// vector points radially out from the earth's center.

void EarthManipulator::home(const GUIEventAdapter& ea, GUIActionAdapter& us)
{
//   cout << "inside EarthManipulator::home()" << endl;

   const threevector earthcenter_ECI_posn(0,0,0);
   set_worldspace_center(earthcenter_ECI_posn);
   rotation_center=earthcenter_ECI_posn;

//   double SanClemente_longitude=-118.48;
//   double SanClemente_latitude=32.915;
//   double Boston_longitude=-71.06;	// degs
//   double Boston_latitude=42.36;	// degs
//   double Baghdad_longitude=44.44;
//   double Baghdad_latitude=33.31;

//   cout << "Enter longitude in degs:" << endl;
//   cin >> longitude;
//   double altitude=2000;	// meters
//   double altitude=20000;	// meters
//   double altitude=2000000;	// meters
   double altitude=25000000;	// meters
//   double altitude=50000000;	// meters

   double home_longitude=0;
   double home_latitude=0;
//   double home_longitude=Baghdad_longitude;
//   double home_latitude=Baghdad_latitude;
//   double home_longitude=Boston_longitude;
//   double home_latitude=Boston_latitude;
//   double home_longitude=SanClemente_longitude;
//   double home_latitude=SanClemente_latitude;
   double home_altitude=altitude;

   threevector camera_ECI_posn=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
      home_longitude,home_latitude,home_altitude,*Clock_ptr);
   Ellipsoid_model_ptr->compute_east_north_radial_dirs(
      camera_ECI_posn,*Clock_ptr);
   genmatrix* Rtrans_ptr=Ellipsoid_model_ptr->
      east_north_radial_to_ECI_rotation(
         home_latitude,home_longitude,*Clock_ptr);

// Xhat_camera is locked to the screen's positive horizontal direction. 

// Yhat_camera is locked to the screen's positive vertical direction.

// Zhat_camera is locked to the normal direction coming out of the screen.

// [ M(0,0) , M(0,1) , M(0,2) ] represents Xhat_camera in ECI coordinates

// [ M(1,0) , M(1,1) , M(1,2) ] represents Yhat_camera in ECI coordinates

// [ M(2,0) , M(2,1) , M(2,2) ] represents Zhat_camera in ECI coordinates

   M.set(Rtrans_ptr->get(0,0),Rtrans_ptr->get(1,0),Rtrans_ptr->get(2,0),0,
         Rtrans_ptr->get(0,1),Rtrans_ptr->get(1,1),Rtrans_ptr->get(2,1),0,
         Rtrans_ptr->get(0,2),Rtrans_ptr->get(1,2),Rtrans_ptr->get(2,2),0,
         camera_ECI_posn.get(0),camera_ECI_posn.get(1),
         camera_ECI_posn.get(2),1);
   Minv=M.inverse(M);
   _rotation.set(M);
   set_ViewMatrix(Minv);

//   cout << "M = " << endl;
//   osgfunc::print_matrix(M);
//   cout << "Minv = " << endl;
//   osgfunc::print_matrix(Minv);

   us.requestRedraw();
}

// ---------------------------------------------------------------------
// Member function spin_about_earth_center

void EarthManipulator::spin_about_earth_center(
   float fx1,float fy1,float fx0,float fy0)
{
//   cout << "****************************************************" << endl;
//   cout << "inside EM::spin_about_earth_center()" << endl;

   float angle;
   osg::Vec3 axis;
   getTrackballRotation(axis,angle,fx1,fy1,fx0,fy0);
   
   osg::Quat delta_rotation;
   delta_rotation.makeRotate(compute_speed_factor(false)*angle,axis);
   _rotation = _rotation*delta_rotation;
   Matrix _rotation_matrix(_rotation);

// Convert delta_rotation quaternion into 3x3 rotation matrix delta_R.
// Then multiply eye_ECI_posn by delta_R_inv to obtain camera's new
// rotated position in ECI coordinates:

   Matrix dR(delta_rotation);
   genmatrix R(3,3),delta_R(3,3),delta_R_inv(3,3);
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         R.put(i,j,_rotation_matrix(i,j));
         delta_R.put(i,j,dR(i,j));
      }
   }
   delta_R.inverse(delta_R_inv);
   threevector p_new=delta_R_inv*get_eye_world_posn();
 
   M.set(R.get(0,0),R.get(0,1),R.get(0,2),0,
         R.get(1,0),R.get(1,1),R.get(1,2),0,
         R.get(2,0),R.get(2,1),R.get(2,2),0,
         p_new.get(0),p_new.get(1),p_new.get(2),1);
   Minv=M.inverse(M);
   set_ViewMatrix(Minv);

//   cout << "M = " << endl;
//   osgfunc::print_matrix(M);

//   cout << "camera direction = " << camera_direction_in_ECI()
//        << endl;

//   cout << "eye_ECI_posn.mag = " << get_eye_world_posn().magnitude() << endl;
//   cout << "Zcamera in ECI = " << Zcamera << endl;
//   cout << "eye_ECI_posn.mag * Zcamera in ECI = "
//        << get_eye_world_posn().magnitude() * Zcamera << endl;

//   cout << "p_new = " << p_new << endl;
//   cout << "p_new.mag = " << p_new.magnitude() << endl;

//   cout << "Minv = " << endl;
//   osgfunc::print_matrix(Minv);
}

// ---------------------------------------------------------------------
// Member function spin_about_nadir() first computes the point on the
// blue marble located at the screen's center.  It then interprets
// vertical mouse movements as elevation tilts about the instantaneous
// horizontal axis passing through the center point.  It further
// interprets horizontal mouse movements as azimuthual spins about the
// radial axis emanating outwards from the screen center.  Reasonable
// restrictions are placed upon how far forward and backward a user
// can tilt the tangent plane passing through the center point.

// The functionality implemented by this method intentionally mimics
// that of Google Earth.

void EarthManipulator::spin_about_nadir(
   float fx1,float fy1,float fx0,float fy0)
{
//   cout << "inside EarthManipulator::spin_about_nadir()" << endl;

   threevector new_rotation_center;
   if (compute_screen_center_intercept(new_rotation_center))
   {
      rotation_center=new_rotation_center;
   }

//   double prefactor=5;
   double prefactor=25;
   double az=-prefactor*sgn(fy0)*(fx1-fx0)*3.14159/180.0;

// On 9/11/07, we changed the sign on the elevation parameter below so
// that our Terrain_Manipulator's movements would conform with Google
// Earth's:

   double elev=prefactor*(fy1-fy0)*3.145159/180.0;

   Ellipsoid_model_ptr->compute_east_north_radial_dirs(
      rotation_center,*Clock_ptr);
   threevector r_hat=Ellipsoid_model_ptr->get_radial_ECI_hat();

// Perform azimuthal spin about instantaneous center radial direction
// vector:

   osg::Vec3 axisR(r_hat.get(0),r_hat.get(1),r_hat.get(2));
   osg::Quat delta_rotationR;
   delta_rotationR.makeRotate(az,axisR);
   _rotation = _rotation*delta_rotationR;

// Perform tilt about camera's instantaneous Xhat direction vector:

   threevector camera_Xhat(get_camera_Xhat());
   osg::Vec3 axisX(camera_Xhat.get(0),camera_Xhat.get(1),
                   camera_Xhat.get(2));
   osg::Quat delta_rotationX;
   delta_rotationX.makeRotate(elev,axisX);

//   cout << "rhat . Xhat = " << r_hat.dot(get_camera_Xhat()) << endl;
//   cout << "rhat . Yhat = " << r_hat.dot(get_camera_Yhat()) << endl;
//   cout << "rhat . Zhat = " << r_hat.dot(get_camera_Zhat()) << endl;

// Do not let user tilt earth surface too far forward or backward:

   const double sin_2_degs=0.034899497; 	// sin(2*3.14159/180);
   const double sin_89_degs=0.999847695; 	// sin (89*3.14159/180);
   double rZdotproduct=r_hat.dot(get_camera_Zhat());
   if ( (rZdotproduct > sin_2_degs && elev >= 0) ||
        (rZdotproduct < sin_89_degs&& elev < 0))
   {
      _rotation = _rotation * delta_rotationX;
   }

   double eye_to_rotation_center_dist=
      (get_eye_world_posn()-rotation_center).magnitude();

   Minv=osg::Matrixd::translate(
      -osg::Vec3(rotation_center.get(0),rotation_center.get(1),
                 rotation_center.get(2)))*
      osg::Matrixd::rotate(_rotation.inverse())*
      osg::Matrixd::translate(0,0,-eye_to_rotation_center_dist);
   M=Minv.inverse(Minv);

// Subtract off component of camera Uhat direction vector which points
// along rhat_ECI.  As Ross Anderson recommends, we want the virtual
// camera's Uhat direction vector to be as level as possible with the
// ground plane when viewing 3D terrain data.  But Uhat must also
// remain orthogonal to the camera's line-of-sight which always points
// towards the center of the screen.  So we must reorthogonalize Uhat
// wrt What after performing the rhat_ECI projection.  Finally, take
// Vhat = What x Uhat to form an orthonormal basis.

   threevector Uhat=get_camera_Xhat();
   threevector Vhat=get_camera_Yhat();
   threevector What=get_camera_Zhat();

   Uhat -= (Uhat.dot(r_hat))*r_hat;
   Uhat -= (Uhat.dot(What)) * What;
   Uhat=Uhat.unitvector();
   Vhat=What.cross(Uhat);

   M.set(Uhat.get(0) , Uhat.get(1) , Uhat.get(2) , 0 ,
         Vhat.get(0) , Vhat.get(1) , Vhat.get(2) , 0 ,
//         What.get(0) , What.get(1) , What.get(2) , 0 ,
         M(2,0) , M(2,1) , M(2,2) , M(2,3),
         M(3,0),  M(3,1) , M(3,2) , M(3,3) );
   Minv=M.inverse(M);

/*
   cout << "M = " << endl;
   osgfunc::print_matrix(M);

   cout << "Uhat = " << Uhat << endl;
   cout << "Vhat = " << Vhat << endl;
   cout << "Uhat.Uhat = " << Uhat.dot(Uhat) << endl;
   cout << "Vhat.Vhat = " << Vhat.dot(Vhat) << endl;
   cout << "Uhat.Vhat = " << Uhat.dot(Vhat) << endl;
   cout << "Vhat.What = " << Vhat.dot(What) << endl;
   cout << "What.Uhat = " << What.dot(Uhat) << endl;
   cout << "Uhat.rhat = " << Uhat.dot(r_hat) << endl;
   cout << endl;
*/

   set_ViewMatrix(Minv);
}

// --------------------------------------------------------------------------
// Member function compute_speed_factor returns an exponentially
// decreasing function of the camera's altitude above the surface of
// the earth ellipsoid.  When the camera is zoomed far out from the
// earth, the speed factor equals unity.  But when it is zoomed close
// in towards the ellipsoid's surface, the speed factor is
// exponentially small.  Both radial and angular translations are
// multiplied by this speed factor.

double EarthManipulator::compute_speed_factor(bool radial_zoom_flag)
{
//   cout << "inside EM::compute_speed_factor()" << endl;
   compute_eye_longitude_latitude_altitude();
   double logalt=log10(eye_altitude);

   double prefactor=1.0;
   double speed_factor=1.0;

   if (radial_zoom_flag)
   {
      speed_factor=pow(200.0, logalt-5.5);

      const double min=0.5;
      const double max=5.0;
      const double mu=log(5000);
      const double sigma=log(3.6);
      double arg=0.5*sqr(log(eye_altitude)-mu)/sqr(sigma);
      prefactor=min+(max-min)*exp(-arg);
   }
   else
   {
      speed_factor=pow(20.0, logalt-6.5);
   }
   speed_factor=basic_math::min(speed_factor,1.0);
   speed_factor=basic_math::max(speed_factor,1.0E-4);
   speed_factor *= prefactor;

//   cout.precision(4);
//   cout << "Alt=" << eye_altitude 
//        << " log alt=" << logalt
//        << " speed=" << speed_factor << endl;
   return speed_factor;
}

// --------------------------------------------------------------------------
// Member function Zoom radially moves the camera outward or inward
// towards the center of the earth ellipsoid model.

void EarthManipulator::Zoom(float dr)
{
//   cout << "inside EarthManipulator::Zoom()" << endl;
   dr *= compute_speed_factor(true);

   threevector p(get_eye_world_posn());
   double magnitude=dr*p.magnitude();
   p += magnitude*get_camera_Zhat();
   M.set(M(0,0),M(0,1),M(0,2),0,
         M(1,0),M(1,1),M(1,2),0,
         M(2,0),M(2,1),M(2,2),0,
         p.get(0),p.get(1),p.get(2),1);
   Minv=M.inverse(M);
   set_ViewMatrix(Minv);

//   cout << "M = " << endl;
//   osgfunc::print_matrix(M);

// Experiment with pushing rotation_center down toward's earth's
// origin as the camera moves radially far out from the earth:

   if (p.magnitude() > 100000 && dr > 0)
   {
      rotation_center *= 0.95;
   }
}

// =====================================================================
// Camera position & orientation member functions
// =====================================================================

// Member function compute_screen_center_intercept() returns within
// screen_center_intercept the coordinates for the ellipsoid point
// located at the center of the screen.  If the ellipsoid does not
// currently cover the screen center, this boolean method returns
// false.

bool EarthManipulator::compute_screen_center_intercept(
   threevector& screen_center_intercept)
{
//   cout << "inside EarthManipulator::compute_screen_center_intercept()"
//        << endl;

   bool intercept_point_on_earth_flag=
      geofunc::groundpoints_ellipsoidal_earth(
         get_eye_world_posn(),-get_camera_Zhat(),
         screen_center_intercept);

//   cout << "screen_center_intercept = " << screen_center_intercept << endl;
   return intercept_point_on_earth_flag;
}

// ---------------------------------------------------------------------
// Member function log_eye_altitude() returns log10 of the camera's
// current altitude.

void EarthManipulator::compute_eye_longitude_latitude_altitude()
{
   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      get_eye_world_posn(),*Clock_ptr,eye_longitude,eye_latitude,
      eye_altitude);
}

double EarthManipulator::log_eye_altitude()
{
   compute_eye_longitude_latitude_altitude();
   return log10(eye_altitude);
}

// ---------------------------------------------------------------------
// Member function altitude_dependent_radial_frac

double EarthManipulator::altitude_dependent_radial_frac()
{
//   cout << "inside EM::alt_depend_radial_frac()" << endl;

   double logalt=log_eye_altitude();
   const double min_log_alt=6.3;
   const double max_log_alt=7.3;

   double logfactor;
   if (logalt > max_log_alt)
   {
      logfactor=-7.0;
   }
   else if (logalt < min_log_alt)
   {
      logfactor=0.0;
   }
   else
   {
      logfactor=0.0+(logalt-min_log_alt)/(max_log_alt-min_log_alt)*(-7.0-0.0);
   }
   cout << "logalt = " << logalt << " logfactor = " << logfactor << endl;
   double radial_factor=pow(10,logfactor);
   cout << " radial_factor = " << radial_factor << endl;

   return radial_factor;
}

// ==========================================================================
// Animation path member functions
// ==========================================================================

// Member function flyto takes in a destination longitude and latitude
// for the camera.  It constructs an animation path linking the
// camera's current position to its final destination.  This method
// then temporarily takes control away from the EarthManipulator and
// gives it to a specialized CustomAnimationPathManipulator.  Once the
// camera has finished slewing from its original position to its final
// destination, control is returned back to the EarthManipulator.

void EarthManipulator::flyto(
   double destination_longitude,double destination_latitude,
   double destination_altitude,bool write_to_file_flag)
{
   double start_longitude,start_latitude,start_altitude;
   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      get_eye_world_posn(),*Clock_ptr,
      start_longitude,start_latitude,start_altitude);
   flyto(start_longitude,start_latitude,start_altitude,
         destination_longitude,destination_latitude,destination_altitude,
         write_to_file_flag);
}

void EarthManipulator::flyto(
   double start_long,double start_lat,double start_alt,
   double destination_longitude,double destination_latitude,
   double destination_altitude,bool write_to_file_flag)
{
   threevector destination_ECI_posn=Ellipsoid_model_ptr->
      ConvertLongLatAltToECI(
         destination_longitude,destination_latitude,destination_altitude,
         *Clock_ptr);

   cout << "Start: Longitude = " << start_long
        << " latitude = " << start_lat 
        << " altitude = " << start_alt << endl;

   cout << "Destination: Longitude = " << destination_longitude
        << " latitude = " << destination_latitude
        << " altitude = " << destination_altitude << endl;

   threevector camera_ECI_posn=Ellipsoid_model_ptr->
      ConvertLongLatAltToECI(start_long,start_lat,start_alt,*Clock_ptr);

// FAKE FAKE: Increase animation speed for debugging purposes only...

   double animation_speed_factor=1;
//   double animation_speed_factor=10;
   osg::AnimationPath* animationPath_ptr=
      generate_animation_path(camera_ECI_posn,destination_ECI_posn,
                              write_to_file_flag,animation_speed_factor);

   osg::ref_ptr<osgGA::CustomAnimationPathManipulator> apm=
      new osgGA::CustomAnimationPathManipulator(
         animationPath_ptr,WindowManager_ptr,this,destination_ECI_posn);

   if ( apm.valid() )
   {
      apm.get()->getAnimationPath()->setLoopMode(
         osg::AnimationPath::NO_LOOPING);
      if (WindowManager_ptr != NULL)
      {
         unsigned int num=WindowManager_ptr->set_CameraManipulator(apm.get());
         WindowManager_ptr->selectCameraManipulator(num);
      } 
   } // apm.valid() conditional
}

// ---------------------------------------------------------------------
// Member function init_camera_orientation_wrt_ENR_basis

void EarthManipulator::init_camera_orientation_wrt_ENR_basis(
   double start_latitude,double start_longitude,double& chi_init)
{

// First extract the camera's initial orientation into matrix R_ECI.
// The columns of R_ECI hold xcamera_hat, ycamera_hat and zcamera_hat
// measured wrt the ECI basis:

   genmatrix R_ECI(3,3);
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         R_ECI.put(i,j,Minv(i,j));
      }
   }
//   cout << "R_ECI = " << R_ECI << endl;

// For interpolation purposes, we need to work with the camera's
// orientation wrt the local and position-dependent east-north-radial
// basis rather than the absolute ECI basis.  Convert from ECI to ENR
// bases using matrix *U_ptr which holds east_hat, north_hat and
// radial_hat measured wrt the ECI basis:

   genmatrix* U_ptr=Ellipsoid_model_ptr->east_north_radial_to_ECI_rotation(
      start_latitude,start_longitude,*Clock_ptr);
//   cout << "*U_ptr = " << *U_ptr << endl;

// The columns of matrix R_ENR hold xhat_camera, yhat_camera and
// zhat_camera expressed wrt the local east-north-radial basis:

   genmatrix R_ENR(3,3);
   R_ENR=U_ptr->transpose() * R_ECI;
//   cout << "R_ENR = " << R_ENR << endl;

// Decompose R_ENR into a rotation axis p_hat_ENR (measured in the
// local ENR basis) and a single rotation angle chi:

   mathfunc::decompose_orthogonal_matrix(R_ENR,chi_init,p_hat_ENR);
//   cout << "p_hat_ENR = " << p_hat_ENR << endl;
//   cout << "chi_init = " << chi_init*180/osg::PI << endl;
}

// ---------------------------------------------------------------------
// Member function generate_animation_path takes in starting and
// stopping positions in ECI coordinates.  It first converts these
// positions into corresponding longitudes, latitudes and altitudes.
// It then forms smooth time slew profiles for the camera's longitude,
// latitude and altitude using error functions.  At each time step,
// the camera's XYZ coordinates are aligned with the earth ellipsoid's
// local east, north and radial direction vectors.  Time, camera posn
// and camera orientation information are inserted into a dynamically
// generated AnimationPath object which is returned by this method.

// The animation path continuously interpolates between the initial
// camera orientation wrt the local east, north and radial directions
// and the final camera orientation which is aligned with the
// (different!) local east, north and radial directions.

osg::AnimationPath* EarthManipulator::generate_animation_path(
   const threevector& start_ECI_posn,const threevector& stop_ECI_posn,
   bool write_to_file_flag,double animation_speed_factor)
{
//   cout << "inside EarthManipulator::generate_anim_path()" << endl;

//   double separation_distance=(stop_ECI_posn-start_ECI_posn).magnitude();

   double start_latitude,start_longitude,start_altitude;
   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      start_ECI_posn,*Clock_ptr,start_longitude,start_latitude,
      start_altitude);

   double stop_latitude,stop_longitude,stop_altitude;
   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      stop_ECI_posn,*Clock_ptr,stop_longitude,stop_latitude,stop_altitude);
   stop_longitude=basic_math::phase_to_canonical_interval(
      stop_longitude,start_longitude-180,start_longitude+180);

//   cout << "stop_long = " << stop_longitude
//        << " stop_lat = " << stop_latitude
//        << " stop_alt = " << stop_altitude << endl;
   
   double delta_longitude=stop_longitude-start_longitude;
   double delta_latitude=stop_latitude-start_latitude;
   double delta_theta=sqrt(sqr(delta_longitude)+sqr(delta_latitude));
//   cout << "delta_theta = " << delta_theta << endl;

   double omega,max_altitude;
   compute_flight_params(delta_theta,omega,max_altitude);
   omega *= animation_speed_factor;
   double middle_altitude=basic_math::max(start_altitude,stop_altitude);
   middle_altitude=basic_math::max(middle_altitude,max_altitude);
   
   double t_quadratic=delta_theta/omega;
   t_quadratic=basic_math::max(t_quadratic,3.0);	// secs

// Times t0, t2 and t4 correspond to starting, middle and stopping
// points in camera slew profile.  Times t1 and t3 represent midpoints
// within error functions which are used to incorporate acceleration
// and deceleration into the longitude, latitude and altitude
// profiles:

   double t0=0;

// If the camera's altitude is initially large, camera should not
// spend much time radially translating away from the earth's surface.
// So take the [t0,t1] interval to depend upon the following fraction:

   double frac=fabs(middle_altitude-start_altitude)/
      basic_math::max(middle_altitude,start_altitude);
   frac=basic_math::max(frac,0.1);
   double t1=0.5*frac*t_quadratic;
   const double shortening_factor=0.8;
   double t3=t1+shortening_factor*t_quadratic;
   double t2=0.5*(t1+t3);
   double t4=t3+0.5*t_quadratic;

//   cout << "t0 = " << t0 
//        << " t1 = " << t1
//        << " t2 = " << t2
//        << " t3 = " << t3
//        << " t4 = " << t4 << endl;
 
   double dt=0.1;	// secs
   int nbins=static_cast<int>((t4-t0)/dt+1);
//   cout << "nbins = " << nbins << endl;

// Compute camera's initial orientation wrt local east-north-radial
// basis:

   double chi_init;
   init_camera_orientation_wrt_ENR_basis(
      start_latitude,start_longitude,chi_init);

// Initialize animation path:

   osg::AnimationPath* animationPath_ptr = new osg::AnimationPath;
   animationPath_ptr->setLoopMode(osg::AnimationPath::NO_LOOPING);

   ofstream outputstream;
   outputstream.precision(12);
   if (write_to_file_flag)
   {
      string output_filename="earth_manip_flyby.path";
      filefunc::openfile(output_filename,outputstream);
   }

   double t=0;
   threevector ECI_posn;
   osg::Quat q;
   for (int n=0; n<nbins; n++)
   {
      t=t0+n*dt;

// Use error function to model dependence of longitude and latitude
// slews from starting to stopping locations:

      double sigma=0.25*(t4-t2);

      double arg=(t-t2)/(sqrt(2.0)*sigma);
      double longitude=start_longitude+(stop_longitude-start_longitude)*
         0.5*(1+mathfunc::error_function(arg));
      double latitude=start_latitude+(stop_latitude-start_latitude)*
         0.5*(1+mathfunc::error_function(arg));

      double sigma1=0.25*(t1-t0);
      double sigma3=0.25*(t4-t3);
      double arg1=(t-t1)/(sqrt(2.0)*sigma1);
      double arg3=(t-t3)/(sqrt(2.0)*sigma3);

// Use two error functions to incorporate acceleration and
// deceleration into altitude profile:

      double altitude;
      if (t < t2)
      {
         altitude=start_altitude+(middle_altitude-start_altitude)
            *0.5*(1+mathfunc::error_function(arg1));
      }
      else
      {
         altitude=middle_altitude+(stop_altitude-middle_altitude)
            *0.5*(1+mathfunc::error_function(arg3));
      }
      
//      cout << "n = " << n 
//           << " t = " << t 
//           << " arg = " << arg
//           << " erf(arg) = " << mathfunc::error_function(arg)
//           << " long = " << longitude
//           << " lat = " << latitude
//           << " alt = " << altitude 
//           << endl;

// Interpolate between camera's initial orientation and current east,
// north and radial directions:

      double frac=2*double(n)/double(nbins-1);
      frac=basic_math::min(frac,1.0);
      double chi=(1-frac)*chi_init;
      interpolate_camera_posn_and_orientation(
         longitude,latitude,altitude,chi,ECI_posn,q);
      
//      cout << "ECI_posn = " << ECI_posn.get(0) << "," 
//           << ECI_posn.get(1) << ","
//           << ECI_posn.get(2) << endl;
//      cout << "q = " 
//           << q._v[0] << ","
//           << q._v[1] << ","
//           << q._v[2] << ","
//           << q._v[3] << endl;

      animationPath_ptr->insert(t,osg::AnimationPath::ControlPoint(
         osg::Vec3(ECI_posn.get(0),ECI_posn.get(1),ECI_posn.get(2)),q));

      if (write_to_file_flag)
      {
         outputstream << t << " "
                      << ECI_posn.get(0) << " "
                      << ECI_posn.get(1) << " "
                      << ECI_posn.get(2) << " "
                      << q._v[0] << " "
                      << q._v[1] << " "
                      << q._v[2] << " "
                      << q._v[3] << " " 
                      << endl;
      }

   } // loop over index n labeling interpolation time steps

   animationPath_ptr->insert(t+dt,osg::AnimationPath::ControlPoint(
      osg::Vec3(ECI_posn.get(0),ECI_posn.get(1),ECI_posn.get(2)),q));
   animationPath_ptr->insert(t+2*dt,osg::AnimationPath::ControlPoint(
      osg::Vec3(ECI_posn.get(0),ECI_posn.get(1),ECI_posn.get(2)),q));


   if (write_to_file_flag)
   {
      outputstream << t+dt << " "
                   << ECI_posn.get(0) << " "
                   << ECI_posn.get(1) << " "
                   << ECI_posn.get(2) << " "
                   << q._v[0] << " "
                   << q._v[1] << " "
                   << q._v[2] << " "
                   << q._v[3] << " " 
                   << endl;
      outputstream << t+2*dt << " "
                   << ECI_posn.get(0) << " "
                   << ECI_posn.get(1) << " "
                   << ECI_posn.get(2) << " "
                   << q._v[0] << " "
                   << q._v[1] << " "
                   << q._v[2] << " "
                   << q._v[3] << " " 
                   << endl;
      outputstream.close();
   }
   

//   cout << "start_longitude = " << start_longitude
//        << " start_latitude = " << start_latitude
//        << " start_altitude = " << start_altitude << endl;

//   cout << "stop_longitude = " << stop_longitude
//        << " stop_latitude = " << stop_latitude
//        << " stop_altitude = " << stop_altitude << endl;

   return animationPath_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_flight_params returns the camera's angular
// slewing rate omega and max_altitude which depend upon the angular
// separation delta_theta between the starting and stopping locations:

void EarthManipulator::compute_flight_params(
   double delta_theta,double& omega,double& max_altitude)
{
   if (delta_theta > 100)
   {
//      omega=10;
      omega=12.5;  // degs/sec
//      omega=15;
   }
   else if (delta_theta > 50)
   {
//      omega=7.5;
      omega=8.75;
//      omega=10;
   }
   else if (delta_theta > 5)
   {
//      omega=5;
      omega=6.25;
//      omega=7.5;
   }
   else if (delta_theta > 1)
   {
      omega=1;
   }
   else 
   {
      omega=0.2;
   }

// Maximum possible altitude for camera's orbit also depends upon
// angular separation between starting and stopping locations:

   if (delta_theta <1)
   {
      max_altitude=1*1000; 	// meters
   }
   else if (delta_theta < 5)
   {
      max_altitude=20*1000;
   }
   else if (delta_theta < 10)
   {
      max_altitude=200 * 1000;		// meters
   }
   else if (delta_theta < 20)
   {
      max_altitude=666 * 1000;		// meters
   }
   else if (delta_theta < 40)
   {
      max_altitude=2000 * 1000;		// meters
   }
   else if (delta_theta < 80)
   {
      max_altitude=6666 * 1000;		// meters
   }
   else 
   {
      max_altitude=20000 * 1000;      	// meters
   }
//   cout << "max_altitude = " << max_altitude << endl;
}

// ---------------------------------------------------------------------
// Interpolate between camera's initial orientation and current east,
// north and radial directions:

void EarthManipulator::interpolate_camera_posn_and_orientation(
   double longitude,double latitude,double altitude,
   double chi,threevector& ECI_posn,osg::Quat& q)
{
//   cout << "inside EarthManipulator::interpolate_camera_posn_and_orientation()"
//        << endl;
   genmatrix* R_enr_curr_ptr=Ellipsoid_model_ptr->
      east_north_radial_to_ECI_rotation(latitude,longitude,*Clock_ptr);

   threevector p_hat_ECI=
      p_hat_ENR.get(0)*Ellipsoid_model_ptr->get_east_ECI_hat()+
      p_hat_ENR.get(1)*Ellipsoid_model_ptr->get_north_ECI_hat()+
      p_hat_ENR.get(2)*Ellipsoid_model_ptr->get_radial_ECI_hat();

// When frac = 0, twirl angle chi = chi_init.  Camera is then aligned
// in its current local ENR basis in the same way as it was in its
// initial local ENR basis.

// When frac = 1, twirl angle chi = 0. Camera is then aligned with the
// local ENR's basis direction vectors.  

   rotation delta_R;   
   delta_R.rotation_about_nhat_by_theta(chi,p_hat_ECI);
   rotation R_avg=delta_R * (*R_enr_curr_ptr);

   ECI_posn=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
      longitude,latitude,altitude,*Clock_ptr);

   osg::Matrix M_tmp;
   M_tmp.set(R_avg.get(0,0),R_avg.get(1,0),R_avg.get(2,0),0,
             R_avg.get(0,1),R_avg.get(1,1),R_avg.get(2,1),0,
             R_avg.get(0,2),R_avg.get(1,2),R_avg.get(2,2),0,
             ECI_posn.get(0),ECI_posn.get(1),ECI_posn.get(2),1);
   q.set(M_tmp);
}

// ---------------------------------------------------------------------
void EarthManipulator::generate_animation_path_file(
   double t_start,double t_stop,const threevector& start_ECI_posn,
   const threevector& stop_ECI_posn)
{
   double start_latitude,start_longitude,start_altitude;
   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      start_ECI_posn,*Clock_ptr,start_longitude,start_latitude,
      start_altitude);

   double stop_latitude,stop_longitude,stop_altitude;
   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      stop_ECI_posn,*Clock_ptr,stop_longitude,stop_latitude,stop_altitude);

   int nbins=200;
   double dt=(t_stop-t_start)/(nbins-1);

   double d_latitude=(stop_latitude-start_latitude)/(nbins-1);
   double d_longitude=(stop_longitude-start_longitude)/(nbins-1);
//   double d_altitude=(stop_altitude-start_altitude)/(nbins-1);

//   double start_log_altitude=log10(start_altitude);
//   double stop_log_altitude=log10(stop_altitude);
//   double d_log_alt=(stop_log_altitude-start_log_altitude)/(nbins-1);

// Construct piecewise linear altitude function of bin number 0 <= n
// <= nbins-1:

   vector<double> altitude_frac_at_regular_intervals;

//   altitude_frac_at_regular_intervals.push_back(1.0);
//   altitude_frac_at_regular_intervals.push_back(0.95);
//   altitude_frac_at_regular_intervals.push_back(0.85);
//   altitude_frac_at_regular_intervals.push_back(0.70);
//   altitude_frac_at_regular_intervals.push_back(0.50);
//   altitude_frac_at_regular_intervals.push_back(0.25);
//   altitude_frac_at_regular_intervals.push_back(0.01);

   altitude_frac_at_regular_intervals.push_back(1.0);
   altitude_frac_at_regular_intervals.push_back(0.98);
   altitude_frac_at_regular_intervals.push_back(0.94);
   altitude_frac_at_regular_intervals.push_back(0.86);
   altitude_frac_at_regular_intervals.push_back(0.70);
   altitude_frac_at_regular_intervals.push_back(0.50);
   altitude_frac_at_regular_intervals.push_back(0.25);
   altitude_frac_at_regular_intervals.push_back(0.01);
   altitude_frac_at_regular_intervals.push_back(0.0);

   vector<double> x;
   for (int i=0; i<int(altitude_frac_at_regular_intervals.size()); i++)
   {
      x.push_back(i/double(altitude_frac_at_regular_intervals.size()-1));
   }

   vector<double> alt_frac;
   for (int n=0; n<nbins; n++)
   {
      double f=double(n)/(nbins-1);
      int bin=mathfunc::binary_locate(x,0,x.size()-1,f);
      alt_frac.push_back(
         altitude_frac_at_regular_intervals[bin]+(f-x[bin])/
         (x[bin+1]-x[bin])*(altitude_frac_at_regular_intervals[bin+1]-
                            altitude_frac_at_regular_intervals[bin]));
   }

   string anim_path_filename="earth_flyby.path";
   ofstream outstream;
   if (filefunc::openfile(anim_path_filename,outstream))
   {
      for (int n=0; n<nbins; n++)
      {
         double t=t_start+n*dt;

         double longitude=start_longitude+n*d_longitude;
         double latitude=start_latitude+n*d_latitude;
         double altitude=start_altitude+(1-alt_frac[n])*(
            stop_altitude-start_altitude);
         threevector ECI_posn=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
            longitude,latitude,altitude,*Clock_ptr);

         Ellipsoid_model_ptr->compute_east_north_radial_dirs(
            latitude,longitude);
         Ellipsoid_model_ptr->convert_surface_to_ECI_directions(*Clock_ptr);
   
         threevector east_hat=Ellipsoid_model_ptr->get_east_ECI_hat();
         threevector north_hat=Ellipsoid_model_ptr->get_north_ECI_hat();
         threevector radial_hat=Ellipsoid_model_ptr->get_radial_ECI_hat();

         osg::Matrix M_tmp;
         M_tmp.set(east_hat.get(0),east_hat.get(1),east_hat.get(2),0,
               north_hat.get(0),north_hat.get(1),north_hat.get(2),0,
               radial_hat.get(0),radial_hat.get(1),radial_hat.get(2),0,
               ECI_posn.get(0),ECI_posn.get(1),ECI_posn.get(2),1);
         osg::Quat q;
         q.set(M_tmp);

         outstream << t << " "
                   << ECI_posn.get(0) << " "
                   << ECI_posn.get(1) << " "
                   << ECI_posn.get(2) << " "
                   << q._v[0] << " "
                   << q._v[1] << " "
                   << q._v[2] << " "
                   << q._v[3] << " " 
                   << endl;
      } // loop over index n labeling interpolation time steps
      filefunc::closefile(anim_path_filename,outstream);
   } // animation path file successfully opened conditional
}
