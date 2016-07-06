// ========================================================================
// Program DRAPE takes as command line arguments an XYZP file along
// with a Group 99 .vid file.  It also reads in interpolated camera
// intrinsic & extrinsic parameters for every video image from
// "camera_params.txt" which we assume has previously been generated
// by program CAMERA_PARAMS.  DRAPE asks the user to enter the number
// of a video image to be draped onto the point cloud.  It then colors
// those XYZ points which project onto the video frame according to
// the RGB information in the image.  Other XYZ points that do not
// project onto the video frame are colored according to height.
// ========================================================================
// Last updated on 9/21/07; 10/11/07; 10/15/07
// ========================================================================

#include <iostream>
#include <string>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>
#include <osgDB/WriteFile>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFusion/FusionGroup.h"
#include "osg/osgFusion/FusionKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewer and instantiate a ViewerManager:

   osgProducer::Viewer viewer(arguments);
   WindowManager* window_mgr_ptr=new ViewerManager(&viewer);
   window_mgr_ptr->initialize_window("3D imagery");
//   viewer.getUsage(*arguments.getApplicationUsage());

// Create OSG root node and black backdrop:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

//   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
//      ModeController_ptr);
   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   viewer.getEventHandlerList().push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr));
   root->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root->addChild( movies_group.get_OSGgroup_ptr() );

// Read in camera intrinsic & extrinsic parameters as functions of
// time generated by main program CAMERA_PARAMS.  Then instantiate a
// camera for each image in the video sequence:

//   movie_ptr->plot_roll_pitch_yaw_vs_time();
   movie_ptr->read_camera_params_for_sequence();

// Instantiate fusion group to hold math relationships between 3D and
// 2D imagery for draping purposes:

   bool view_draped_video_flag=true;
   FusionGroup* FusionGroup_ptr=new FusionGroup(
      passes_group.get_pass_ptr(videopass_ID),cloud_ptr,movie_ptr,
      AnimationController_ptr,view_draped_video_flag);
   viewer.getEventHandlerList().push_back(new FusionKeyHandler(
      ModeController_ptr,FusionGroup_ptr));
   root->addChild(FusionGroup_ptr->get_OSGgroup_ptr());

// Instantiate center pick handler to handle mid point selection:

   CentersGroup centers_group(
      3,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      3,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   viewer.getEventHandlerList().push_back(CenterPickHandler_ptr);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(root);

   root->addChild(decorations.get_OSGgroup_ptr());
   viewer.setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CUstomManipulator's home() method:

   viewer.realize();

   while( !viewer.done() )
   {
      window_mgr_ptr->process();
   }

}

