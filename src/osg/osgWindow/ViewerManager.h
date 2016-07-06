// ==========================================================================
// ViewerManager header file 
// ==========================================================================
// Last modified on 2/28/11; 11/6/11; 1/14/12
// ==========================================================================

#ifndef VIEWERMANAGER_H
#define VIEWERMANAGER_H

#include <iostream>
#include <osgProducer/Viewer>
#include "osg/osgWindow/WindowManager.h"

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

namespace osgUtil
{
   class SceneView;
}

namespace osgProducer
{
   class MyViewerEventHandler;
}

class ViewerManager : public WindowManager
{
  public:

   ViewerManager();
   ViewerManager(osgProducer::Viewer* V_ptr);
   ViewerManager(osgProducer::Viewer* V_ptr,osgProducer::Viewer* V2_ptr);

// Set & get member functions:

   virtual void set_default_Window_X_origin(int X);
   int get_default_Window_X_origin() const;
   virtual void set_default_Window_Y_origin(int Y);
   int get_default_Window_Y_origin() const;
   int get_default_Window_width() const;
   int get_default_Window_height() const;

   int get_Window_X_origin() const;
   int get_Window_Y_origin() const;
   int get_Window_width() const;
   int get_Window_height() const;

   virtual double get_initial_horizontal_FOV() const;
   virtual double get_initial_vertical_FOV() const;

   void set_Viewer_ptr(osgProducer::Viewer* V_ptr);
   void set_Viewer2_ptr(osgProducer::Viewer* V_ptr);
   virtual osgProducer::Viewer* get_Viewer_ptr();
   virtual const osgProducer::Viewer* get_Viewer_ptr() const;
   osgProducer::Viewer* get_Viewer2_ptr();
   const osgProducer::Viewer* get_Viewer2_ptr() const;

// Camera Manipulator member functions:

   virtual unsigned int set_CameraManipulator(
      osgGA::CustomManipulator* CM_ptr);
   virtual void selectCameraManipulator(unsigned int camera_num);
   virtual void selectCameraManipulatorByName(std::string name);

// Window initialization member functions:

   virtual void initialize_window(std::string window_name);
   virtual void initialize_dual_windows(
      std::string window1_name,std::string window2_name,
      WindowManager* WM2_ptr,bool twoD_window_on_right_flag=true);
   virtual void set_thick_client_window_position();

// Window size member functions:

   void compute_current_window_dimensions();
   virtual twovector convert_renormalized_to_pixel_window_coords(
      float X,float Y);

// FOV member functions:

   virtual double get_lens_horizontal_FOV() const;
   virtual double get_lens_vertical_FOV() const;
   virtual void match_viewer_fovs_to_viewfrustum(ViewFrustum* VF_ptr);
   virtual void set_viewer_horiz_vert_fovs(double hfov,double vfov);
   virtual void rescale_viewer_FOV(double angular_scale_factor);

// Event loop processing member functions:

   virtual void setSceneData(osg::Node* node_ptr);
   virtual void realize(int viewer_ID=0);
   virtual bool done();
   virtual void process();
   virtual void partial_process();

// View and Projection matrix retrieval member functions:

   virtual void setViewMatrix(const osg::Matrix& M);
   virtual osg::Matrix& getViewMatrix();
   virtual osg::Matrix& getProjectionMatrix();
   virtual osg::Viewport* getViewport_ptr();
   virtual osg::Node* getSceneData_ptr();

   virtual void retrieve_camera_posn_and_direction_vectors(
      osg::Vec3& eye,osg::Vec3& center,osg::Vec3& up);

// MyViewerEventHandler member functions:

   osgProducer::MyViewerEventHandler* get_MyViewerEventHandler_ptr();
   const osgProducer::MyViewerEventHandler* get_MyViewerEventHandler_ptr()
      const;
   virtual void snap_screen(std::string snapshot_filename);
   virtual void set_auto_generate_movies_flag(bool flag);
   virtual void set_horiz_scale_factor(double f);

  protected:

   virtual void set_initial_horizontal_FOV();
   virtual void set_initial_vertical_FOV();

  private:

   int default_Window_X_origin,default_Window_Y_origin;
   int default_Window_width,default_Window_height;
   int Window_X_origin,Window_Y_origin;
   int Window_width,Window_height;
   osgProducer::Viewer *Viewer_ptr,*Viewer2_ptr;
   osg::ref_ptr<osgUtil::SceneView> SceneView_refptr;
   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void instantiate_MyViewerEventHandler();
   osgUtil::SceneView* get_SceneView_ptr();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ViewerManager::set_Viewer_ptr(osgProducer::Viewer* V_ptr)
{ 
   Viewer_ptr=V_ptr;
}

inline void ViewerManager::set_Viewer2_ptr(osgProducer::Viewer* V_ptr)
{ 
   Viewer2_ptr=V_ptr;
}

inline osgProducer::Viewer* ViewerManager::get_Viewer_ptr()
{ 
   return Viewer_ptr;
}

inline const osgProducer::Viewer* ViewerManager::get_Viewer_ptr() const
{ 
   return Viewer_ptr;
}

inline osgProducer::Viewer* ViewerManager::get_Viewer2_ptr()
{ 
   return Viewer2_ptr;
}

inline const osgProducer::Viewer* ViewerManager::get_Viewer2_ptr() 
   const
{ 
   return Viewer2_ptr;
}

inline void ViewerManager::set_default_Window_X_origin(int X)
{
   default_Window_X_origin=X;
}

inline int ViewerManager::get_default_Window_X_origin() const
{
   return default_Window_X_origin;
}

inline void ViewerManager::set_default_Window_Y_origin(int Y)
{
   default_Window_Y_origin=Y;
}

inline int ViewerManager::get_default_Window_Y_origin() const
{
   return default_Window_Y_origin;
}

inline int ViewerManager::get_default_Window_width() const
{
   return default_Window_width;
}

inline int ViewerManager::get_default_Window_height() const
{
   return default_Window_height;
}

inline int ViewerManager::get_Window_X_origin() const
{
   return Window_X_origin;
}

inline int ViewerManager::get_Window_Y_origin() const
{
   return Window_Y_origin;
}

inline int ViewerManager::get_Window_width() const
{
   return Window_width;
}

inline int ViewerManager::get_Window_height() const
{
   return Window_height;
}

inline void ViewerManager::set_initial_horizontal_FOV()
{
   initial_horizontal_FOV=get_lens_horizontal_FOV();
//    std::cout << "init horiz fov = " << initial_horizontal_FOV << std::endl;
}

inline double ViewerManager::get_initial_horizontal_FOV() const
{
   return initial_horizontal_FOV;
}

inline void ViewerManager::set_initial_vertical_FOV()
{
   initial_vertical_FOV=get_lens_vertical_FOV();
}

inline double ViewerManager::get_initial_vertical_FOV() const
{
   return initial_vertical_FOV;
}




#endif 
