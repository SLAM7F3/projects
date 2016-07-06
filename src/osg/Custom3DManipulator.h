// ========================================================================
// Header file for Custom3DManipulator class
// ========================================================================
// Last updated on 9/1/11; 9/4/11; 9/5/11; 9/11/11
// ========================================================================

#ifndef OSGGA_CUSTOM3DMANIPULATOR
#define OSGGA_CUSTOM3DMANIPULATOR 

#include <iostream>
#include <string>
#include "osg/CompassHUD.h"
#include "osg/CustomManipulator.h"
#include "math/rotation.h"
#include "math/threevector.h"
#include "osg/ViewFrustum.h"

#include "osg/osgfuncs.h"

class ModeController;
class PointFinder;
class Transformer;
class WindowManager;

namespace osgGA {

   class OSGGA_EXPORT Custom3DManipulator : public osgGA::CustomManipulator
      {
        public:

         Custom3DManipulator(
            ModeController* MC_ptr,WindowManager* WM_ptr,
            bool disable_rotations=false,
            bool emulate_GoogleEarth_rotations=true);

// Set & get member functions:

         void set_active_control_flag(bool flag);
         bool get_active_control_flag() const;
         void set_DiamondTouchTable_flag(bool flag);
         void set_mouse_input_device_flag(bool flag);
         bool get_mouse_input_device_flag() const;
         void set_hmi_select_flag(bool flag);
         bool get_hmi_select_flag() const;
         void set_hmi_select_value(int value);
         int get_hmi_select_value() const;

         void set_grid_origin_ptr(threevector* GO_ptr);
         threevector* get_grid_origin_ptr();
         const threevector* get_grid_origin_ptr() const;
         void set_CompassHUD_ptr(CompassHUD* CompassHUD_ptr);
         CompassHUD* get_CompassHUD_ptr();
         const CompassHUD* get_CompassHUD_ptr() const;

         Transformer* get_Transformer_ptr();
         const Transformer* get_Transformer_ptr() const;

         void set_PointFinder(PointFinder* PF_ptr);
         PointFinder* get_PointFinder_ptr();
         const PointFinder* get_PointFinder_ptr() const;

         void setMatrices(const osg::Matrixd& m);
         virtual osg::Matrixd getMatrix() const;
         virtual osg::Matrixd getInverseMatrix() const;
         void update_M_and_Minv();
         void update_viewfrustum();

         void set_eye_world_posn(const threevector& posn);
         threevector get_eye_world_posn() const;
         threevector get_camera_Xhat() const;
         threevector get_camera_Yhat() const;
         threevector get_camera_Zhat() const;
         rotation get_camera_rotation() const;

         ViewFrustum* get_ViewFrustum_ptr();
         const ViewFrustum* get_ViewFrustum_ptr() const;

         double get_camera_height_above_grid() const;
         double get_log_eye_center_dist() const;

         virtual void set_flying_maneuver_finished_flag(bool flag);
         virtual bool get_flying_maneuver_finished_flag() const;
         virtual void set_rotate_about_current_eyepoint_flag(bool flag);
         virtual bool get_rotate_about_current_eyepoint_flag() const;
         virtual void set_initial_camera_posn(const threevector& posn);
         virtual void set_initial_camera_rotation(const rotation& R);
         virtual void set_final_camera_posn(const threevector& posn);
         virtual void set_final_camera_rotation(const rotation& R);
//         virtual void set_flyout_zoom_counter(int counter);
         virtual int get_flyout_zoom_counter() const;

         void set_fx_fy_params(
            float& fx_curr,float& fy_curr,float& fx_prev,float& fy_prev);
         
         void store_initial_zoom_params();
         
// Camera position & orientation member functions

         double compute_camera_to_screen_center_distance(
            const threevector& screen_center_intercept);
         double compute_approx_projected_north_hat_azimuth();
         double compute_approx_projected_north_hat_azimuth(
            const osg::Matrixd& Minv);
         double compute_camera_az();
         void get_curr_az_el_roll(
            double& curr_az,double& curr_el,double& curr_roll);

         virtual void reset_Manipulator_control();

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

// HMI input device handling member functions:

         void reset_translation(
            double fx_curr,double fy_curr,double fx_prev,double fy_prev);
         void reset_az_el(double d_az,double d_el);
         void reset_zoom(double ds);

         virtual bool reset_view_to_home();

        protected:

         bool disable_rotations_flag;
         bool emulate_GoogleEarth_rotations_flag;
         float fx_prev,fy_prev;
         double log_eye_center_dist;
         double init_eye_to_center_distance;
         threevector init_worldspace_center;
         threevector* grid_origin_ptr;
         osg::Matrix M,Minv;

         Transformer* Transformer_ptr;
         PointFinder* PointFinder_ptr;
         ViewFrustum* ViewFrustum_ptr;
         CompassHUD* CompassHUD_ptr;

         virtual ~Custom3DManipulator();
         virtual void home(const GUIEventAdapter& ea,GUIActionAdapter& us);

         virtual bool parse_mouse_events(const GUIEventAdapter& ea);
         virtual bool parse_scroll_events(const GUIEventAdapter& ea);
        
         virtual void Translate(
            float fx_curr,float fy_curr,float fx_prev,float fy_prev);
         virtual void Translate(float d_fx,float d_fy);
         virtual void Rotate(
            float fx_prev,float fy_prev,float fx_curr,float fy_curr);
         virtual void Zoom(float zoomDist);

        private:

         bool DiamondTouchTable_flag;
         bool mouse_input_device_flag;
         bool active_control_flag;
         bool hmi_select_flag;
         int hmi_select_value;

         void allocate_member_objects();
         void initialize_member_objects();
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

   inline void Custom3DManipulator::set_active_control_flag(bool flag)
      {
         active_control_flag=flag;
      }
   
   inline bool Custom3DManipulator::get_active_control_flag() const 
      {
         return active_control_flag;
      }

   inline void Custom3DManipulator::set_DiamondTouchTable_flag(bool flag)
      {
         DiamondTouchTable_flag=flag;
      }

   inline void Custom3DManipulator::set_mouse_input_device_flag(
      bool flag)
      {
         mouse_input_device_flag=flag;
      }

   inline bool Custom3DManipulator::get_mouse_input_device_flag() const 
      {
         return mouse_input_device_flag;
      }

   inline void Custom3DManipulator::set_hmi_select_flag(bool flag)
      {
         hmi_select_flag=flag;
      }
   
   inline bool Custom3DManipulator::get_hmi_select_flag() const 
      {
         return hmi_select_flag;
      }

   inline void Custom3DManipulator::set_grid_origin_ptr(threevector* GO_ptr)
      {
         grid_origin_ptr=GO_ptr;
      }

   inline threevector* Custom3DManipulator::get_grid_origin_ptr()
   {   
      return grid_origin_ptr;
   }
   
   inline const threevector* Custom3DManipulator::get_grid_origin_ptr() const
   {
      return grid_origin_ptr;
   }

   inline void Custom3DManipulator::set_CompassHUD_ptr(
      CompassHUD* CompassHUD_ptr)
      {
         this->CompassHUD_ptr=CompassHUD_ptr;
      }

   inline CompassHUD* Custom3DManipulator::get_CompassHUD_ptr()
      {
         return CompassHUD_ptr;
      }

   inline const CompassHUD* Custom3DManipulator::get_CompassHUD_ptr() const
      {
         return CompassHUD_ptr;
      }

   inline Transformer* Custom3DManipulator::get_Transformer_ptr()
      {
         return Transformer_ptr;
      }

   inline const Transformer* Custom3DManipulator::get_Transformer_ptr() const 
      {
         return Transformer_ptr;
      }

   inline void Custom3DManipulator::set_PointFinder(PointFinder* PF_ptr)
      {
         PointFinder_ptr=PF_ptr;
      }

   inline PointFinder* Custom3DManipulator::get_PointFinder_ptr()
      {
         return PointFinder_ptr;
      }

   inline const PointFinder* Custom3DManipulator::get_PointFinder_ptr() const 
      {
         return PointFinder_ptr;
      }

// InverseMatrix Minv maps from ECI coordinates to screen space
// coordinates.  Multiply input ECI vectors as rows on the left by
// Minv to obtain their output screen coordinates.  In particular, the
// earth model's origin (0,0,0) in ECI space is mapped to the 4th row
// of Minv.

// Matrix M maps from screen space to ECI coordinates.  The camera's
// (or equivalently eye's) position in ECI space is contained within
// the 4th row of M.

   inline void Custom3DManipulator::setMatrices(const osg::Matrixd& m) 
      {
         M=m;
         Minv=M.inverse(M);
      }

   inline void Custom3DManipulator::set_eye_world_posn(
      const threevector& posn) 
      {
         M.set(M(0,0) , M(0,1) , M(0,2) , M(0,3) , 
               M(1,0) , M(1,1) , M(1,2) , M(1,3) , 
               M(2,0) , M(2,1) , M(2,2) , M(2,3) , 
               posn.get(0) , posn.get(1) , posn.get(2) , M(3,3) );
         Minv=M.inverse(M);
      }

   inline ViewFrustum* Custom3DManipulator::get_ViewFrustum_ptr()
      {
         return ViewFrustum_ptr;
      }

   inline const ViewFrustum* Custom3DManipulator::get_ViewFrustum_ptr() const
      {
         return ViewFrustum_ptr;
      }

   inline double Custom3DManipulator::get_log_eye_center_dist() const
      {
         return log_eye_center_dist;
      }

} // osgGA namespace

#endif


