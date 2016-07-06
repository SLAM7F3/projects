// ========================================================================
// Program VIEWDRAPE takes as command line arguments an XYZP file
// along with a Group 99 .vid file.  It also reads in time-dependent
// RGBA color information from "draped_color_arrays.rgba".  This
// program plays back a temporal sequence of colored XYZP point clouds
// as a 4D movie.

// To see an example of this 4D movie player, chant

// 		viewdrape HAFB.xyzp HAFB_overlap_RGB.vid

//	- Press "=" while in View Data mode to set 3D point size to one
//	  larger than default.  
 
//	- Enter 'R' to enter Run Movie mode. Press "+" once to increase 
//	  delay between video frames to 0.1.  Then press "=" to set skip
//	  between video frames to 2.   

//	- Press 'f' to set first frame to display.  Enter 15 in the
//	  console.  Then Enter "p" to play draped video.

//	- Press "V" to enter silent "Generate Movie" mode.  Press "Z"
//	  to read in saved_animation.path = cars.path for this movie.
//	  Press "b" to go back to beginning of video.  Finally press "F1" 
//	  to display results on full screen.

// Last verified that program VIEWDRAPE correctly ran on 11/6/06, 11/20/06,
// 11/22/06; 12/4/06; 12/19/06; 12/27/06; 12/28/06; 12/29/06; 1/17/07; 2/8/07

// VIEWDRAPE does not play more than zeroth image as of 3/13/07.
// SATDRAPE works OK.

// ========================================================================
// Last updated on 6/16/07; 8/20/07; 9/21/07; 10/11/07; 10/15/07
// ========================================================================

#include <iostream>
#include <string>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>
#include <osgDB/WriteFile>

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
//    viewer.getUsage(*arguments.getApplicationUsage());

// Create OSG root node:

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

// Instantiate group to hold "holodeck" grid:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=true;
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud(index_tree_flag);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
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
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);

// Instantiate fusion group to hold math relationships between 3D and
// 2D imagery for draping purposes:

   bool view_draped_video_flag=true;
   FusionGroup* FusionGroup_ptr=new FusionGroup(
      passes_group.get_pass_ptr(videopass_ID),cloud_ptr,movie_ptr,
      AnimationController_ptr,view_draped_video_flag);
   root->addChild(FusionGroup_ptr->get_OSGgroup_ptr());

// Instantiate center pick handler to handle mid point selection:

   CentersGroup centers_group(
      3,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      3,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

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

