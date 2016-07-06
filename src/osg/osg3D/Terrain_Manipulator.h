// ========================================================================
// Header file for Terrain_Manipulator class
// ========================================================================
// Last updated on 9/11/11; 12/1/11; 2/9/13
// ========================================================================

#ifndef OSGGA_TERRAIN_MANIPULATOR
#define OSGGA_TERRAIN_MANIPULATOR 

#include <deque>
#include <iostream>
#include <string>
#include <vector>
#include <osg/AnimationPath>
#include <osg/Quat>
#include "osg/osg2D/ColorbarHUD.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgGrid/Grid.h"
#include "math/rotation.h"

namespace osgGA
{
   class CustomAnimationPathManipulator;
}

class ModeController;
class WindowManager;

namespace osgGA {

   class OSGGA_EXPORT Terrain_Manipulator : public osgGA::Custom3DManipulator
      {
        public:

         Terrain_Manipulator(ModeController* MC_ptr,WindowManager* WM_ptr);
         Terrain_Manipulator(ModeController* MC_ptr,threevector* GO_ptr,
                             WindowManager* WM_ptr);
         virtual const char* className() const { 
            return "Terrain_Manipulator"; }

// Set & get member functions:

         osg::Node* get_root_ptr();
         virtual void set_flying_maneuver_finished_flag(bool flag);
         virtual bool get_flying_maneuver_finished_flag() const;
         void set_flyout_zoom_counter(int counter);
         virtual int get_flyout_zoom_counter() const;
         virtual void set_rotate_about_current_eyepoint_flag(bool flag);
         bool get_rotate_about_current_eyepoint_flag() const;
         void set_allow_only_az_rotation_flag(bool flag);
         void set_disallow_zoom_flag(bool flag);
         void set_max_camera_height_above_grid_factor(double factor);
         void set_enable_underneath_looking_flag(bool flag);

         void set_min_camera_height_above_grid(double h);
         void set_Grid_ptr(Grid* grid_ptr);

         virtual void set_initial_camera_posn(const threevector& posn);
         virtual void set_initial_camera_rotation(const rotation& R);
         virtual void set_final_camera_posn(const threevector& posn);
         virtual void set_final_camera_rotation(const rotation& R);
         
         std::vector<twovector>& get_Grid_corner_screen_coords();
         const std::vector<twovector>& get_Grid_corner_screen_coords() const;
         void set_ColorbarHUD_ptr(ColorbarHUD* ColorbarHUD_ptr);

         twovector get_projected_north_hat();
         const twovector get_projected_north_hat() const;
         void update_compass_heading();

         void adjust_horiz_vert_fovs();
         virtual bool reset_view_to_home();

// Animation path member functions:
         
         virtual void jumpto(
            const threevector& final_posn,const rotation& final_R,
            double final_FOV_u=-1,double final_FOV_v=-1);

         virtual void flyto(
            const threevector& final_posn,const rotation& final_R,
            bool write_to_file_flag=false,
            bool no_final_slowdown_flag=false,
            double final_FOV_u=-1,double final_FOV_v=-1,
            int n_anim_steps=-1,double t_flight=-1);
         virtual void flyto(
            const threevector& final_posn,const rotation& final_R,
            const threevector& init_posn,const rotation& init_R,
            bool write_to_file_flag=false,
            bool no_final_slowdown_flag=false,
            double final_FOV_u=-1,double final_FOV_v=-1,
            int n_anim_steps=-1,double t_flight=-1);

         virtual void reset_Manipulator_control();
         osg::AnimationPath* generate_animation_path(
            const threevector& init_posn,const rotation& init_R,
            const threevector& final_posn,const rotation& final_R,
            bool write_to_file_flag=false,bool no_final_slowdown_flag=false,
            int n_anim_steps=-1,double t_flight=-1);
         void animation_twirl_params(
            const rotation& init_R,const rotation& final_R,
            double& chi,threevector& n_hat);
         double flight_slew_rate(double slew_distance);
         
         void reset_member_quaternion(
            double eye_to_rotation_center_dist,
            const threevector& screen_center_intercept);

        protected:

         virtual ~Terrain_Manipulator();
         virtual void home(const GUIEventAdapter& ea,GUIActionAdapter& us);

         virtual void Translate(
            float fx_curr,float fy_curr,float fx_prev,float fy_prev);
         virtual void Rotate(
            float fx_prev,float fy_prev,float fx_curr,float fy_curr);
         virtual void Zoom(float zoomDist);

         virtual bool parse_keyboard_events(const GUIEventAdapter& ea);

        private:

         bool rotate_about_current_eyepoint_flag;
         bool allow_only_az_rotation_flag;
         bool disallow_zoom_flag;
         bool flying_maneuver_finished_flag;
         bool enable_underneath_looking_flag;

         int flyout_zoom_counter;
         double depth_range,min_camera_height_above_grid;
         double max_camera_height_above_grid_factor;
         std::deque<double> roll_data,pitch_data,yaw_data;
         Grid* grid_ptr;

         twovector projected_north_hat;
         std::vector<twovector> Grid_corner_screen_coords;
         std::vector<threevector> camera_posns;
         std::vector<fourvector> camera_quats;

         threevector initial_camera_posn,final_camera_posn;
         rotation *initial_camera_rotation_ptr,*final_camera_rotation_ptr;
         osg::ref_ptr<osg::Image> image_refptr;
         ColorbarHUD* ColorbarHUD_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
         threevector compute_screen_intercept(
            double fx,double fy,double& camera_to_center_distance);
         void interpolate_camera_orientation(
            const rotation& init_R,double curr_chi,
            const threevector& n_hat,osg::Quat& q,rotation& curr_R);
         osg::AnimationPath* generate_animation_path(
            const threevector& init_posn,const rotation& init_R,
            const threevector& final_posn,const rotation& final_R,
            double t_flight,bool write_to_file_flag=false,
            bool no_final_slowdown_flag=false,int n_anim_steps=-1);

// Grid member functions:

         bool ViewFrustum_relative_to_Grid();

         void compute_Grid_corner_screen_coords();
         void check_Grid_overlap_with_colorbar();

         void reset_fields_of_view(
            osgGA::CustomAnimationPathManipulator* apm_ptr,
            double final_FOV_u,double final_FOV_v);
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline osg::Node* Terrain_Manipulator::get_root_ptr()
      {
         return WindowManager_ptr->getSceneData_ptr();
      }
   
   inline void Terrain_Manipulator::set_flying_maneuver_finished_flag(
      bool flag)
      {
         flying_maneuver_finished_flag=flag;
      }
   
   inline bool Terrain_Manipulator::get_flying_maneuver_finished_flag() const
      {
         return flying_maneuver_finished_flag;
      }
   
   inline void Terrain_Manipulator::set_enable_underneath_looking_flag(
      bool flag)
   {
      enable_underneath_looking_flag=flag;
   }

   inline void Terrain_Manipulator::set_flyout_zoom_counter(int counter)
      {
         flyout_zoom_counter=counter;
      }

   inline int Terrain_Manipulator::get_flyout_zoom_counter() const
      {
         return flyout_zoom_counter;
      }

   inline void Terrain_Manipulator::set_min_camera_height_above_grid(
      double h)
      {
         min_camera_height_above_grid=h;
      }

   inline void Terrain_Manipulator::set_Grid_ptr(Grid* grid_ptr)
      {
         this->grid_ptr=grid_ptr;
         set_grid_origin_ptr(grid_ptr->get_world_origin_ptr());
      }

   inline void Terrain_Manipulator::set_rotate_about_current_eyepoint_flag(
      bool flag)
      {
         rotate_about_current_eyepoint_flag=flag;
         if (flag)
         {
            std::cout << "Rotating about current eyepoint" << std::endl;
         }
         else
         {
            std::cout << "Rotating about current Z-grid center" << std::endl;
//            if (grid_ptr != NULL)
//            {
//               grid_ptr->set_mask(1);
//            }
         }
      }
   
   inline bool Terrain_Manipulator::get_rotate_about_current_eyepoint_flag() 
      const
      {
         return rotate_about_current_eyepoint_flag;
      }
   
   inline void Terrain_Manipulator::set_allow_only_az_rotation_flag(bool flag)
      {
         allow_only_az_rotation_flag=flag;
      }

   inline void Terrain_Manipulator::set_disallow_zoom_flag(bool flag)
      {
         disallow_zoom_flag=flag;
      }

   inline void Terrain_Manipulator::set_max_camera_height_above_grid_factor(
      double factor)
   {
      max_camera_height_above_grid_factor=factor;
   }

   inline void Terrain_Manipulator::set_initial_camera_posn(
      const threevector& posn)
      {
         initial_camera_posn=posn;
      }

   inline void Terrain_Manipulator::set_final_camera_posn(
      const threevector& posn)
      {
         final_camera_posn=posn;
      }
   
   inline void Terrain_Manipulator::set_initial_camera_rotation(
      const rotation& R)
      {
         *initial_camera_rotation_ptr=R;
      }

   inline void Terrain_Manipulator::set_final_camera_rotation(
      const rotation& R)
      {
         *final_camera_rotation_ptr=R;
      }

   inline std::vector<twovector>& 
      Terrain_Manipulator::get_Grid_corner_screen_coords()
      {
         return Grid_corner_screen_coords;
      }

   inline const std::vector<twovector>& 
      Terrain_Manipulator::get_Grid_corner_screen_coords() const
      {
         return Grid_corner_screen_coords;
      }

   inline void Terrain_Manipulator::set_ColorbarHUD_ptr(
      ColorbarHUD* ColorbarHUD_ptr)
      {
         this->ColorbarHUD_ptr=ColorbarHUD_ptr;
      }

   inline twovector Terrain_Manipulator::get_projected_north_hat()
      {
         return projected_north_hat;
      }

   inline const twovector Terrain_Manipulator::get_projected_north_hat() const
      {
         return projected_north_hat;
      }
   

} // osgGA namespace

#endif

