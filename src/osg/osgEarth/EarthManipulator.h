// ========================================================================
// Header file for EarthManipulator class
// ========================================================================
// Last updated on 8/19/07; 8/20/07; 9/21/07; 10/19/07
// ========================================================================

#ifndef OSGGA_EARTHMANIPULATOR
#define OSGGA_EARTHMANIPULATOR 

#include <string>
#include <osg/AnimationPath>
#include "osg/Custom3DManipulator.h"
#include "astro_geo/Ellipsoid_model.h"

class Clock;
class Earth;
class grid_origin_ptr;
class ModeController;
class WindowManager;

namespace osgGA {

   class OSGGA_EXPORT EarthManipulator : public osgGA::Custom3DManipulator
      {
        public:

         EarthManipulator(ModeController* MC_ptr,
                          Ellipsoid_model* emodel_ptr,Clock* clock_ptr,
                          WindowManager* WM_ptr);
         virtual const char* className() const { return "EarthManipulator"; }

         virtual bool parse_mouse_events(const GUIEventAdapter& ea);
         virtual void home(const GUIEventAdapter& ea,GUIActionAdapter& us);

// Camera position & orientation member functions

         bool compute_screen_center_intercept(
            threevector& screen_center_intercept);
         void compute_eye_longitude_latitude_altitude();
         double log_eye_altitude();
         double get_eye_altitude();

// Animation path member functions:

         void flyto(double destination_longitude,double destination_latitude,
                    double destination_altitude,
                    bool write_to_file_flag=false);
         void flyto(double start_long,double stop_long,double start_alt,
                    double destination_longitude,double destination_latitude,
                    double destination_altitude,
                    bool write_to_file_flag=false);
         void init_camera_orientation_wrt_ENR_basis(
            double start_latitude,double start_longitude,double& chi_init);
         osg::AnimationPath* generate_animation_path(
            const threevector& start_ECI_posn,
            const threevector& stop_ECI_posn,bool write_to_file_flag=false,
            double animation_speed_factor=1);
         void compute_flight_params(
            double delta_theta,double& omega,double& max_altitude);
         void generate_animation_path_file(
            double t_start,double t_stop,const threevector& start_ECI_posn,
            const threevector& stop_ECI_posn);

        protected:

         virtual ~EarthManipulator();
         void spin_about_earth_center(
            float fx1,float fy1,float fx0,float fy0);
         void spin_about_nadir(float fx1,float fy1,float fx0,float fy0);
         virtual void Zoom(float dr);

        private:

         double eye_latitude,eye_longitude,eye_altitude;
         threevector screen_center_intercept_ECI;
         threevector rotation_center;	// probably redundant member var
         threevector p_hat_ENR;
         Clock* Clock_ptr;
         Ellipsoid_model* Ellipsoid_model_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
         double compute_speed_factor(bool radial_zoom_flag);
         double altitude_dependent_radial_frac();

         void interpolate_camera_posn_and_orientation(
            double longitude,double latitude,double altitude,
            double chi,threevector& ECI_posn,osg::Quat& q);
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

   inline double EarthManipulator::get_eye_altitude()
      {
         compute_eye_longitude_latitude_altitude();
         return eye_altitude;
      }

} // osgGA namespace

#endif

