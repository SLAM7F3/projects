// ==========================================================================
// ViewerManager class member function definitions
// ==========================================================================
// Last modified on 2/28/11; 11/6/11; 1/14/12
// ==========================================================================

#include <iostream>
#include <osgUtil/SceneView>
#include <osg/Vec3>
#include "osg/AnimationPathCreator.h"
#include "osg/CustomManipulator.h"
#include "osg/ModeController.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgWindow/ViewerManager.h"
#include "osg/ViewFrustum.h"

#include <Producer/VisualChooser>

#include "osg/osgfuncs.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ViewerManager::allocate_member_objects()
{
}		       

void ViewerManager::initialize_member_objects()
{
   default_Window_X_origin=0;
   default_Window_Y_origin=500;
   default_Window_height=800;
   default_Window_width=basic_math::round(4.0/3.0*default_Window_height);

   Viewer_ptr=NULL;
   Viewer2_ptr=NULL;
}		       

ViewerManager::ViewerManager()
{
   allocate_member_objects();
   initialize_member_objects();

   Viewer_ptr=new osgProducer::Viewer;
   set_EventHandlers_ptr(&(Viewer_ptr->getEventHandlerList()));
   get_EventHandlers_ptr()->push_back(AnimationPathCreator_ptr);

   instantiate_MyViewerEventHandler();
}

ViewerManager::ViewerManager(osgProducer::Viewer* V_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Viewer_ptr=V_ptr;

   set_EventHandlers_ptr(&(Viewer_ptr->getEventHandlerList()));
   get_EventHandlers_ptr()->push_back(AnimationPathCreator_ptr);
   instantiate_MyViewerEventHandler();
}

ViewerManager::ViewerManager(
   osgProducer::Viewer* V_ptr,osgProducer::Viewer* V2_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Viewer_ptr=V_ptr;
   Viewer2_ptr=V2_ptr;

   set_EventHandlers_ptr(&(Viewer_ptr->getEventHandlerList()));
   get_EventHandlers_ptr()->push_back(AnimationPathCreator_ptr);
   instantiate_MyViewerEventHandler();
}

// ==========================================================================
// Camera manipulator member functions
// ==========================================================================

unsigned int ViewerManager::set_CameraManipulator(
   osgGA::CustomManipulator* CM_ptr)
{
   WindowManager::set_CameraManipulator(CM_ptr);
   MyViewerEventHandler_ptr->set_CustomManipulator_ptr(CM_ptr);
   unsigned int camera_num=Viewer_ptr->addCameraManipulator(CM_ptr);
   return camera_num;
}

void ViewerManager::selectCameraManipulator(unsigned int camera_num)
{
   Viewer_ptr->selectCameraManipulator(camera_num);
}

void ViewerManager::selectCameraManipulatorByName(string name)
{
   Viewer_ptr->selectCameraManipulatorByName(name);
}

// ==========================================================================
// Window initialization member functions
// ==========================================================================

void ViewerManager::initialize_window(string window_name)
{
//    cout << "inside ViewerManager::init_window()" << endl;

   Viewer_ptr->setClearColor(osg::Vec4(0.1 , 0.1 , 0.1 , 0.0));

//   Viewer_ptr->setClearColor(osg::Vec4(0,0,0,0));
   
// FAKE FAKE: Mon Nov 30, 2009 
// Reset background color to dark blue for Kermit-with-grid texturing tests

//   Viewer_ptr->setClearColor(osg::Vec4(0,0,0.33,0));

   Viewer_ptr->setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE); // linux
//   Viewer_ptr->setUpViewer(osgProducer::Viewer::SKY_LIGHT_SOURCE |
//                      osgProducer::Viewer::ESCAPE_SETS_DONE);
//   Viewer_ptr->setUpViewer(osgProducer::Viewer::HEAD_LIGHT_SOURCE |
//                      osgProducer::Viewer::ESCAPE_SETS_DONE);

// Note added on 7/6/07: We empirically determined that we need to
// include VIEWER_MANIPULATOR to the list of osgProducer::Viewer
// options in order for windowing to work OK under Mac OS X.  But then
// 'f' for "full screen" trumps "flyin" under MANIPULATE FUSED DATA
// mode....

//   Viewer_ptr->setUpViewer(
//      osgProducer::Viewer::VIEWER_MANIPULATOR |
//      osgProducer::Viewer::ESCAPE_SETS_DONE);			// Mac OS X

//   Viewer_ptr->setUpViewer(osgProducer::Viewer::SKY_LIGHT_SOURCE |
//                      osgProducer::Viewer::ESCAPE_SETS_DONE);
//   Viewer_ptr->setUpViewer(osgProducer::Viewer::HEAD_LIGHT_SOURCE |
//                      osgProducer::Viewer::ESCAPE_SETS_DONE);

   Producer::RenderSurface* rs_ptr =
      Viewer_ptr->getCameraConfig()->getCamera(0)->getRenderSurface();

// FAKE FAKE: Thurs Feb 28 at 5:03 pm...  

// Note added on 3/3/08: Ross tried to find some Producer way to set
// Depth Buffer's number of bits from 16 to 24 in order to reduce
// flickering of CH video on top of bluegrass ladar cloud in program
// VIDEOCITIES.  As of 2/28/08, we were unsuccessful in finding the
// necessary osgProducer commands to do this depth buffer bit
// resetting.  But the commented out lines immediately below represent
// our initial attempt to do so...


//   Producer::VisualChooser* VisualChooser_ptr=new Producer::VisualChooser();
//   VisualChooser_ptr->setDepthSize(24);
//   rs_ptr->setVisualChooser(VisualChooser_ptr);
//   cout << "Visualchooser ptr = " << rs_ptr->getVisualChooser() << endl;


   rs_ptr->setScreenNum(0);

#ifndef WIN32                    
   rs_ptr->useBorder(true);
#else
   rs_ptr->fullScreen(false);
#endif

   rs_ptr->setWindowName(window_name);
   
   rs_ptr->setWindowRectangle(
      default_Window_X_origin,
      default_Window_Y_origin,
      default_Window_width,default_Window_height);
   
// On 6/27/05, we discovered that the osgIntersect codes expect window
// coordinates to run from -1 to 1 rather than from 0 to 1 in both the
// horizontal and vertical directions:

// As of 7/21/05, we discovered that our feature picking current
// requires l=-1, r=+1, b=-1, t=+1 !!!

// left,right,bottom,top

   rs_ptr->setInputRectangle( 
      Producer::RenderSurface::InputRectangle(-1.0,1.0,-1.0,1.0));
}

// ---------------------------------------------------------------------
// Method initialize_dual_windows sets up two identically-sized
// windows which can display 3D and 2D data side-by-side.

void ViewerManager::initialize_dual_windows(
   string window1_name,string window2_name,WindowManager* WM2_ptr,
   bool twoD_window_on_right_flag)
{
//   cout << "inside VM::init_dual_windows()" << endl;

   Viewer2_ptr=WM2_ptr->get_Viewer_ptr();

   Viewer_ptr->setClearColor(osg::Vec4(0,0,0,0));
   Viewer2_ptr->setClearColor(osg::Vec4(0,0,0,0));

// FAKE FAKE:  Thurs Jun 7, 2012 at 9:24 am
// change background color from black to white for viewgraphs only

//   Viewer_ptr->setClearColor(osg::Vec4(1,1,1,0));
//   Viewer2_ptr->setClearColor(osg::Vec4(1,1,1,0));

   Viewer_ptr->setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE); // linux
   Viewer2_ptr->setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE); // linux
//   Viewer2_ptr->setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE |
//                            osgProducer::Viewer::HEAD_LIGHT_SOURCE);

   Producer::RenderSurface* rs_2D_ptr =
      Viewer_ptr->getCameraConfig()->getCamera(0)->getRenderSurface();
   Producer::RenderSurface* rs_3D_ptr =
      Viewer2_ptr->getCameraConfig()->getCamera(0)->getRenderSurface();

   rs_2D_ptr->setScreenNum(0);
   rs_3D_ptr->setScreenNum(0);

#ifndef WIN32                    
   rs_2D_ptr->useBorder(true);
   rs_3D_ptr->useBorder(true);
#else
   rs_2D_ptr->fullScreen(false);
   rs_3D_ptr->fullScreen(false);
#endif

   rs_2D_ptr->setWindowName(window1_name);
   rs_3D_ptr->setWindowName(window2_name);

// Following values empirically set for Dell laptop (M6300,M6400) on
// 8/26/09:

   int delta_x=10;
   int y_offset=800;
   int height=712;
//   int y_offset=height;
//         int delta_x=25;
//         int y_offset=500;
//         int height=580;
   int width=basic_math::round(4.0/3.0*height);

// On 1/19/2010, we found that the DVI2USB frame grabber could not
// handle our Dell laptop's 1980x1200 screen resolution.  So we were
// forced to use the Nvidia X Server tool to reset the laptop's screen
// resolution to 1600x1200.  In this case, this method's dual window
// display must be resized in order to fit: width = 800 , height=600.

//   width=800;
//   height=600;

   if (twoD_window_on_right_flag)
   {
      rs_3D_ptr->setWindowRectangle(0,y_offset,width,height);
      rs_2D_ptr->setWindowRectangle(delta_x+width,y_offset,width,height);
   }
   else
   {
      rs_2D_ptr->setWindowRectangle(0,y_offset,width,height);
      rs_3D_ptr->setWindowRectangle(delta_x+width,y_offset,width,height);
   }

   rs_3D_ptr->setInputRectangle( 
      Producer::RenderSurface::InputRectangle(-1.0,1.0,-1.0,1.0));
   rs_2D_ptr->setInputRectangle( 
      Producer::RenderSurface::InputRectangle(-1.0,1.0,-1.0,1.0));
}

// ---------------------------------------------------------------------
// Member function set_thick_client_window_position() sets the
// Window's X and Y origin coordinates to values appropriate for 30"
// monitor and LOST laptop screens.

void ViewerManager::set_thick_client_window_position()
{
   set_default_Window_X_origin(840);
   set_default_Window_Y_origin(800); 
}

// ---------------------------------------------------------------------
twovector ViewerManager::convert_renormalized_to_pixel_window_coords(
   float X,float Y)
{
//   cout << "inside ViewerManager::convert_renormalized_to_pixel_window_coords()"
//        << endl;

   float pixel_x,pixel_y;
   Viewer_ptr->computePixelCoords(X,Y,0,pixel_x,pixel_y);

   twovector pixel_mouse_coords(pixel_x,pixel_y);
//   cout << "pixel_mouse_coords = " << pixel_mouse_coords << endl;
   
   return pixel_mouse_coords;
}

// ---------------------------------------------------------------------
void ViewerManager::compute_current_window_dimensions()
{
   Producer::RenderSurface* rs_ptr =
      Viewer_ptr->getCameraConfig()->getCamera(0)->getRenderSurface();

   Window_X_origin=rs_ptr->getWindowOriginX();
   Window_Y_origin=rs_ptr->getWindowOriginY();
   Window_width=rs_ptr->getWindowWidth();
   Window_height=rs_ptr->getWindowHeight();

//   cout << "WindowOriginX = " << Window_X_origin
//        << " WindowOriginY = " << Window_Y_origin << endl;
//   cout << "WindowWidth = " << Window_width
//        << " WindowHeight = " << Window_height << endl;
}

// ==========================================================================
// FOV member functions
// ==========================================================================

double ViewerManager::get_lens_horizontal_FOV() const
{
   if (Viewer_ptr != NULL)
   {
//      cout << "lens_horiz_fov = " << Viewer_ptr->getLensHorizontalFov()
//           << endl;
      return Viewer_ptr->getLensHorizontalFov();
   }
   else
   {
      return 45*PI/180;
   }
}

double ViewerManager::get_lens_vertical_FOV() const
{
   if (Viewer_ptr != NULL)
   {
      return Viewer_ptr->getLensVerticalFov();
   }
   else
   {
      return 35*PI/180;
   }
}

// ---------------------------------------------------------------------
void ViewerManager::match_viewer_fovs_to_viewfrustum(ViewFrustum* VF_ptr)
{
   if (Viewer_ptr != NULL)
   {
      Viewer_ptr->sync();
      Viewer_ptr->update();

      double horiz_FOV=VF_ptr->get_horiz_FOV();
      double vert_FOV=VF_ptr->get_vert_FOV();
      double eps=1;	// deg
      set_viewer_horiz_vert_fovs(
         horiz_FOV*180/PI-eps,vert_FOV*180/PI-eps);
   }
}

// ---------------------------------------------------------------------
void ViewerManager::set_viewer_horiz_vert_fovs(double hfov,double vfov)
{
//   cout << "inside ViewerManager::set_viewer_horiz_vert_fovs()" << endl;
//   cout << "hfov = " << hfov << " vfov = " << vfov << endl;
   
   if (Viewer_ptr != NULL)
   {
      double left,right,bottom,top,nearClip,farClip;
      Viewer_ptr->getLensParams(left,right,bottom,top,nearClip,farClip);
//      cout << "left = " << left << " right = " << right << endl;
//      cout << "bottom = " << bottom << " top = " << top << endl;
//      cout << "nearClip = " << nearClip << " farClip = " << farClip << endl;
   
//      cout << "Original viewer hfov = " << Viewer_ptr->getLensHorizontalFov()
//           << endl;
//      cout << "Original viewer vfov = " << Viewer_ptr->getLensVerticalFov()
//           << endl;
      Viewer_ptr->setLensPerspective(hfov,vfov,nearClip,farClip);
   }
}

// ---------------------------------------------------------------------
// Member function rescale_viewer_FOV() multiplies OSG Producer's
// default horizontal and vertical fields-of-view by the input angular
// scale factor.  

void ViewerManager::rescale_viewer_FOV(double angular_scale_factor)
{
//   cout << "inside ViewerManager::rescale_viewer_FOV()" << endl;
   
   double new_horiz_FOV=angular_scale_factor*get_lens_horizontal_FOV();
   double new_vert_FOV=angular_scale_factor*get_lens_vertical_FOV();
   
//   cout << "new horizontal FOV = " << new_horiz_FOV 
//        << " new vertical FOV = " << new_vert_FOV << endl;

   set_viewer_horiz_vert_fovs(new_horiz_FOV,new_vert_FOV);
}

// ==========================================================================
// Event loop processing member functions:
// ==========================================================================

void ViewerManager::setSceneData(osg::Node* node_ptr)
{
   Viewer_ptr->setSceneData(node_ptr);
}

void ViewerManager::realize(int viewer_ID)
{
//   cout << "inside ViewerManager::realize, viewer_ID = "
//        << viewer_ID << endl;
   if (viewer_ID==0)
   {
      Viewer_ptr->realize();
      set_initial_horizontal_FOV();
      set_initial_vertical_FOV();
   }
   else if (viewer_ID==1)
   {
      Viewer2_ptr->realize();
   }
}

bool ViewerManager::done()
{
//   cout << "inside ViewerMananger::done()" << endl;
   return Viewer_ptr->done();
}

// ---------------------------------------------------------------------
// Member function process calls the sync, update and frame methods
// which OSG1 performs in each event loop iteration

void ViewerManager::process()
{
//   cout << "inside ViewerManager::process()" << endl;

// Wait for all cull and draw threads to complete:

//   cout << "Before call to sync()" << endl;
//   cout << "getViewMatrix() = " << endl;
//   osgfunc::print_matrix(getViewMatrix());

   Viewer_ptr->sync();

// Update the scene by traversing it with the update visitor which
// will call all node update callbacks and animations:

//   cout << "Before call to update()" << endl;
//   cout << "getViewMatrix() = " << endl;
//   osgfunc::print_matrix(getViewMatrix());

   Viewer_ptr->update();

// Fire off the cull and draw traversals of the scene:
      
//   cout << "Before call to frame()" << endl;
//   cout << "getViewMatrix() = " << endl;
//   osgfunc::print_matrix(getViewMatrix());

   Viewer_ptr->frame();

//   cout << "After call to frame()" << endl;
//   cout << "getViewMatrix() = " << endl;
//   osgfunc::print_matrix(getViewMatrix());
}

// ---------------------------------------------------------------------
// Member function partial_process() calls just Producer's sync() and
// update() methods and not frame().  We wrote this variant of
// process() for "black box" LOST server purposes.

void ViewerManager::partial_process()
{
//   cout << "inside ViewerManager::partial_process()" << endl;

   Viewer_ptr->sync();
   Viewer_ptr->update();
//   Viewer_ptr->frame();
}

// ==========================================================================
// View and Projection matrix retrieval member functions:
// ==========================================================================

osgUtil::SceneView* ViewerManager::get_SceneView_ptr()
{
   if (!SceneView_refptr.valid())
   {
      SceneView_refptr=
         Viewer_ptr->getSceneHandlerList().front()->getSceneView();
   }
   return SceneView_refptr.get();
}

void ViewerManager::setViewMatrix(const osg::Matrix& M)
{
//   cout << "inside ViewerManager::setViewMatrix()" << endl;
   get_SceneView_ptr()->setViewMatrix(M);
}

osg::Matrix& ViewerManager::getViewMatrix()
{
//   cout << "inside ViewerManager::getViewMatrix()" << endl;
   return get_SceneView_ptr()->getViewMatrix();
}

osg::Matrix& ViewerManager::getProjectionMatrix()
{
   return get_SceneView_ptr()->getProjectionMatrix();
}

osg::Viewport* ViewerManager::getViewport_ptr()
{
   return get_SceneView_ptr()->getViewport();
}

osg::Node* ViewerManager::getSceneData_ptr()
{
   return get_SceneView_ptr()->getSceneData();
}

void ViewerManager::retrieve_camera_posn_and_direction_vectors(
   osg::Vec3& eye,osg::Vec3& center,osg::Vec3& up)
{
//   cout << "inside VM::retrieve_camera_posn & dir_vecs" << endl;
   double look_distance=1.0E6;
   get_SceneView_ptr()->getViewMatrixAsLookAt(eye,center,up,look_distance);
//   cout << "eye = " << endl;
//   osgfunc::print_Vec3(eye);
//   cout << "center = " << endl;
//   osgfunc::print_Vec3(center);
//   cout << "up = " << endl;
//   osgfunc::print_Vec3(up);
}

// ==========================================================================
// MyViewerEventHandler member functions
// ==========================================================================

void ViewerManager::instantiate_MyViewerEventHandler()
{
   MyViewerEventHandler_ptr=
      new osgProducer::MyViewerEventHandler(this);

   get_EventHandlers_ptr()->push_front(MyViewerEventHandler_ptr);
}

// ---------------------------------------------------------------------
osgProducer::MyViewerEventHandler* 
ViewerManager::get_MyViewerEventHandler_ptr()
{
   return MyViewerEventHandler_ptr;
}

const osgProducer::MyViewerEventHandler* 
ViewerManager::get_MyViewerEventHandler_ptr() const
{
   return MyViewerEventHandler_ptr;
}

// ---------------------------------------------------------------------
void ViewerManager::snap_screen(string snapshot_filename)
{
   MyViewerEventHandler_ptr->setWriteImageFileName(snapshot_filename);
   MyViewerEventHandler_ptr->setWriteImageOnNextFrame(true);                
}

// ---------------------------------------------------------------------
void ViewerManager::set_auto_generate_movies_flag(bool flag)
{
   MyViewerEventHandler_ptr->set_auto_generate_movies_flag(flag);
}

// ---------------------------------------------------------------------
void ViewerManager::set_horiz_scale_factor(double f)
{
   MyViewerEventHandler_ptr->set_horiz_scale_factor(f);
}

