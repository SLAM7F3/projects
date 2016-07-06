// ==========================================================================
// Pure virtual WindowManager header file 
// ==========================================================================
// Last modified on 2/28/11; 11/6/11; 1/14/12
// ==========================================================================

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <list>
#include <string>
#include <osg/ref_ptr>
#include <osg/Node>
#include "math/twovector.h"

class AnimationPathCreator;
class ModeController;
class ViewFrustum;

namespace osg
{
   class Viewport;
}

namespace osgGA
{
   class CustomManipulator;
}

// On 3/27/07, Anye Li taught us that the following syntax works for
// forward declaring a class which sits in some existing namespace:

namespace osgProducer
{
   class Viewer;
}

namespace osgGA
{
   class GUIEventHandler;
   class MatrixManipulator;
}

// Anye also reminded us that a virtual function must either be
// defined within a base class or else declared pure virtual.  It
// cannot simply be declared virtual and not defined.

class WindowManager 
{
  public:

   WindowManager();
   virtual ~WindowManager();

// Set & get member functions:

   virtual void set_default_Window_X_origin(int x_origin)=0;
   virtual void set_default_Window_Y_origin(int x_origin)=0;
   virtual double get_initial_horizontal_FOV() const = 0;
   virtual double get_initial_vertical_FOV() const = 0;
   virtual osgProducer::Viewer* get_Viewer_ptr()=0;
   virtual const osgProducer::Viewer* get_Viewer_ptr() const = 0;

// EventHandler member functions:

// Following typedef comes from osgProducer::Viewer header file!

   typedef std::list< osg::ref_ptr<osgGA::GUIEventHandler> > EventHandlers;

   void set_EventHandlers_ptr(EventHandlers* EH_ptr);
   EventHandlers* get_EventHandlers_ptr();
   const EventHandlers* get_EventHandlers_ptr() const;
   bool remove_EventHandler_refptr(
      osg::ref_ptr<osgGA::GUIEventHandler> EventHandler_refptr);

// Camera Manipulator member functions:

   virtual unsigned int set_CameraManipulator(
      osgGA::CustomManipulator* CM_ptr);
   virtual void selectCameraManipulator(unsigned int camera_num)=0;
   virtual void selectCameraManipulatorByName(std::string name)=0;

// Window initialization member functions:

   virtual void initialize_window(std::string window_name)=0;
   virtual void initialize_dual_windows(
      std::string window1_name,std::string window2_name,
      WindowManager* WM2_ptr,bool twoD_window_on_right_flag=true)=0;
   virtual void set_thick_client_window_position()=0;

// Window size member functions:

   virtual twovector convert_renormalized_to_pixel_window_coords(
	float X,float Y)=0;

// FOV member functions:

   virtual double get_lens_horizontal_FOV() const=0;
   virtual double get_lens_vertical_FOV() const=0;
   virtual void match_viewer_fovs_to_viewfrustum(ViewFrustum* VF_ptr)=0;
   virtual void set_viewer_horiz_vert_fovs(double hfov,double vfov)=0;
   virtual void rescale_viewer_FOV(double angular_scale_factor)=0;

// Event loop processing member functions:

   virtual void setSceneData(osg::Node* node_ptr)=0;
   virtual void realize(int viewer_ID=0)=0;
   virtual bool done()=0;
   virtual void process()=0;
   virtual void partial_process()=0;

// View and Projection matrix retrieval member functions:

   virtual void setViewMatrix(const osg::Matrix& M)=0;
   virtual osg::Matrix& getViewMatrix()=0;
   virtual osg::Matrix& getProjectionMatrix()=0;
   virtual osg::Viewport* getViewport_ptr()=0;
   virtual osg::Node* getSceneData_ptr()=0;

   virtual void retrieve_camera_posn_and_direction_vectors(
      osg::Vec3& eye,osg::Vec3& center,osg::Vec3& up)=0;

// MyViewerEventHandler member functions:

   virtual void snap_screen(std::string snapshot_filename)=0;
   virtual void set_auto_generate_movies_flag(bool flag)=0;
   virtual void set_horiz_scale_factor(double f)=0;

  protected:

   double initial_horizontal_FOV,initial_vertical_FOV;
   AnimationPathCreator* AnimationPathCreator_ptr;
   osgGA::MatrixManipulator* CameraManipulator_ptr;

   virtual void set_initial_horizontal_FOV()=0;
   virtual void set_initial_vertical_FOV()=0;

  private:

   EventHandlers* EventHandlers_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif 
