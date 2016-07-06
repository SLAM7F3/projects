// ========================================================================
// Header file for CustomManipulator class which is a variant of
// osgGA/TrackballManipulator.
// ========================================================================
// Last updated on 12/26/10; 2/17/11; 2/28/11; 9/1/11; 6/7/14
// ========================================================================

#ifndef OSGGA_CUSTOMMANIPULATOR
#define OSGGA_CUSTOMMANIPULATOR 

#include <osgGA/MatrixManipulator>
#include <osg/Quat>
#include <osgUtil/SceneView>

#include "osg/osgfuncs.h"
#include "math/threevector.h"
#include "osg/osgWindow/WindowManager.h"

class ModeController;

namespace osgGA {

   class OSGGA_EXPORT CustomManipulator : public MatrixManipulator
      {
        public:

         CustomManipulator(ModeController* MC_ptr,WindowManager* WM_ptr);

         virtual const char* className() const { return "Custom"; }

         /** set the position of the matrix manipulator using a 4x4 Matrix.*/
         virtual void setByMatrix(const osg::Matrixd& matrix);

         /** set the position of the matrix manipulator using a 4x4 Matrix.*/

         virtual void setByInverseMatrix(const osg::Matrixd& matrix) 
            { setByMatrix(osg::Matrixd::inverse(matrix)); }

         /** get the position of the manipulator as 4x4 Matrix.*/
         virtual osg::Matrixd getMatrix() const;

         /** get the position of the manipulator as a inverse matrix
             of the manipulator, typically used as a model view
             matrix.*/
         virtual osg::Matrixd getInverseMatrix() const;

         /** Get the FusionDistanceMode. Used by SceneView for setting
             up stereo convergence.*/
         virtual osgUtil::SceneView::FusionDistanceMode 
            getFusionDistanceMode() const { 
            return osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE; }

         /** Get the FusionDistanceValue. Used by SceneView for
              setting up stereo convergence.*/
         virtual float getFusionDistanceValue() const { return _distance; }

         void set_eye_to_center_distance(double d)
            {
               _distance=d;
            }
         
         double get_eye_to_center_distance() const { return _distance; }

         /** Attach a node to the manipulator.  Automatically detaches
             previously attached node.  setNode(NULL) detaches
             previously nodes.  Is ignored by manipulators which do
             not require a reference model.*/
         virtual void setNode(osg::Node*);

         /** Return node if attached.*/
         virtual const osg::Node* getNode() const;

         /** Return node if attached.*/
         virtual osg::Node* getNode();

         /** Move the camera to the default position.  May be ignored
             by manipulators if home functionality is not
             appropriate.*/
         virtual void home(const GUIEventAdapter& ea,GUIActionAdapter& us);
         virtual void home(double);
        
         /** Start/restart the manipulator.*/
         virtual void init(const GUIEventAdapter& ea,GUIActionAdapter& us);

         /** handle events, return true if handled, false otherwise.*/
         virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& us);

         /** set the minimum distance (as ratio) the eye point can be
             zoomed in towards the center before the center is pushed
             forward.*/
         void setMinimumZoomScale(float minimumZoomScale) 
            { _minimumZoomScale=minimumZoomScale; }
         
         /** get the minimum distance (as ratio) the eye point can be
             zoomed in */
         float getMinimumZoomScale() const { return _minimumZoomScale; }

         /** Set the size of the custom. */
         void setCustomSize(float size);

         /** Get the size of the custom. */
         float getCustomSize() const { return _customSize; }

         void set_enable_pick_flag(bool flag);
         bool get_enable_pick_flag() const;
         void set_enable_drag_flag(bool flag);
         bool get_enable_drag_flag() const;

         void set_worldspace_center(const threevector& new_center);
         threevector get_worldspace_center() const;

         void reset_nadir_view();

         osg::Matrix get_rotation_matrix() const;
         osg::Matrix get_rotation_matrix(const osg::Quat& q) const;

         void set_rotation_quat(osg::Quat& q);

         void set_ViewMatrix(const osg::Matrixd& VM);

         ModeController* get_ModeController_ptr();
         const ModeController* get_ModeController_ptr() const;

         const GUIEventAdapter* get_GUIEventAdapter_ptr() const;
         const GUIActionAdapter* get_GUIActionAdapter_ptr() const;

         WindowManager* get_WindowManager_ptr();
         const WindowManager* get_WindowManager_ptr() const;

         virtual bool reset_view_to_home();
         void set_ndims(int d);
         int get_ndims() const;

        protected:

         void set_center(const osg::Vec3d& center);
         const osg::Vec3d& get_center() const;

         /** Reset the internal GUIEvent stack.*/
         void flushMouseEventStack();

         /** Add the current mouse GUIEvent to internal stack.*/
         void addMouseEvent(const GUIEventAdapter& ea);

         void computePosition(
            const osg::Vec3& eye,const osg::Vec3& lv,const osg::Vec3& up);

         /** For the given mouse movement calculate the movement of
             the camera.  Return true is camera has moved and a redraw
             is required.*/

         bool calcMovement(const GUIEventAdapter& ea);

         void getTrackballRotation(
            osg::Vec3& axis,float& angle, 
            float f1x, float f1y, float f2x, float f2y);

         virtual void Translate(float d_fx,float d_fy);
         virtual void Rotate(
            float fx_prev,float fy_prev,float fx_curr,float fy_curr);

         float tb_project_to_sphere(float r, float x, float y);

         /** Check the speed at which the mouse is moving.  If speed
             is below a threshold then return false, otherwise return
             true.*/
         bool isMouseMoving();

         virtual bool parse_keyboard_events(const GUIEventAdapter& ea);
         virtual bool parse_mouse_events(const GUIEventAdapter& ea);
         virtual bool parse_scroll_events(const GUIEventAdapter& ea);

         bool reset_view_to_home(
            const GUIEventAdapter& ea,GUIActionAdapter& us);

         // Internal event stack comprising last three mouse events.
         osg::ref_ptr<const GUIEventAdapter> _ga_t1;
         osg::ref_ptr<const GUIEventAdapter> _ga_t0;

         osg::ref_ptr<osg::Node>       _node;


         double _modelScale;
         double _minimumZoomScale;
         bool _thrown;
         osg::Quat    _rotation;
         float        _customSize;

         bool sticky_action;

         WindowManager* WindowManager_ptr;

         const GUIEventAdapter* ea_ptr;
         GUIActionAdapter* us_ptr;

        private:

	 bool enable_pick_flag,enable_drag_flag;
         int ndims;

         
         osg::Vec3d   _center;
         double       _distance;

         ModeController* ModeController_ptr;
         void allocate_member_objects();
         void initialize_member_objects();


      };

// ========================================================================
// Inlined member functions
// ========================================================================

   inline void CustomManipulator::set_enable_pick_flag(bool flag)
   {
      enable_pick_flag=flag;
   }

   inline bool CustomManipulator::get_enable_pick_flag() const
   {
      return enable_pick_flag;
   }

   inline void CustomManipulator::set_enable_drag_flag(bool flag)
   {
      enable_drag_flag=flag;
   }

   inline bool CustomManipulator::get_enable_drag_flag() const
   {
      return enable_drag_flag;
   }

   inline ModeController* CustomManipulator::get_ModeController_ptr()
      {
         return ModeController_ptr;
      }

   inline const ModeController* CustomManipulator::get_ModeController_ptr() 
      const
      {
         return ModeController_ptr;
      }

   inline threevector CustomManipulator::get_worldspace_center() const
      {
         return threevector(_center.x(),_center.y(),_center.z());
      }

   inline void CustomManipulator::set_center(const osg::Vec3d& center)
      {
         _center=center;
      }

   inline const osg::Vec3d& CustomManipulator::get_center() const
      {
         return _center;
      }

   inline void CustomManipulator::set_rotation_quat(osg::Quat& q)
      {
         _rotation=q;
      }
   
   inline void CustomManipulator::set_ViewMatrix(const osg::Matrixd& VM)
      {
         if (WindowManager_ptr != NULL)
         {
//            std::cout << "inside CustomManipulator::set_ViewMatrix()" 
//                      << std::endl;
//            std::cout << "WindowManager_ptr = " << WindowManager_ptr
//                      << std::endl;
            WindowManager_ptr->setViewMatrix(VM);
         }
      }

   inline const GUIEventAdapter* CustomManipulator::get_GUIEventAdapter_ptr() const
      {
         return ea_ptr;
      }

   inline const GUIActionAdapter* CustomManipulator::get_GUIActionAdapter_ptr() const
      {
         return us_ptr;
      }

   inline WindowManager* CustomManipulator::get_WindowManager_ptr()
      {
         return WindowManager_ptr;
      }

   inline const WindowManager* CustomManipulator::get_WindowManager_ptr() const
      {
         return WindowManager_ptr;
      }
   
   inline void CustomManipulator::set_ndims(int d)
   {
      ndims=d;
   }
   
   inline int CustomManipulator::get_ndims() const
   {
      return ndims;
   }
   


} // osgGA namespace

#endif

