// ========================================================================
// Program FUSESAT is a specialized version of program FUSION created
// for satellite imagery draping purposes.
// ========================================================================;
// Last updated on 9/21/07; 10/15/07; 12/30/07
// ========================================================================

#include <iostream>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <string>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom2DManipulator.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFusion/FusionGroup.h"
#include "osg/osgFusion/FusionKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "passes/TextDialogBox.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments_2D(&argc,argv);
   osg::ArgumentParser arguments_3D(&argc,argv);
   osg::ArgumentParser arguments(&argc,argv);

// Read input ladar point cloud file and video imagery:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewers and instantiate ViewerManagers:

   WindowManager* window_mgr_2D_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   string window_2D_title="2D imagery";
   string window_3D_title="3D imagery";
   window_mgr_2D_ptr->initialize_dual_windows(
      window_2D_title,window_3D_title,window_mgr_3D_ptr);
//   window_mgr_2D_ptr->getUsage(*arguments_2D.getApplicationUsage());
//   window_mgr_3D_ptr->getUsage(*arguments_3D.getApplicationUsage());

// Create two OSG root nodes and black backdrop:

   osg::Group* root_2D = new osg::Group;
   osg::Group* root_3D = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(2,window_mgr_2D_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_2D_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root_2D->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_3D_ptr=new ModeController();
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_3D_ptr) );
   root_3D->addChild(osgfunc::create_Mode_HUD(3,ModeController_3D_ptr));

// Add custom manipulators to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = new 
      osgGA::Custom2DManipulator(ModeController_2D_ptr,window_mgr_2D_ptr);
   window_mgr_2D_ptr->set_CameraManipulator(CM_2D_ptr);

   osgGA::Custom3DManipulator* CM_3D_ptr = new
      osgGA::Custom3DManipulator(ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_2D(
      window_mgr_2D_ptr,ModeController_2D_ptr,CM_2D_ptr);
   Decorations decorations_3D(
      window_mgr_3D_ptr,ModeController_3D_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations_3D.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root_2D->addChild( movies_group.get_OSGgroup_ptr() );

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));
   root_3D->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations_3D.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate center pick handlers to handle mid point selection:

   CentersGroup centers2_group(
      2,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* Center_2D_PickHandler_ptr=new CenterPickHandler(
      2,passes_group.get_pass_ptr(videopass_ID),
      CM_2D_ptr,&centers2_group,ModeController_2D_ptr,
      window_mgr_2D_ptr);
   CentersGroup centers3_group(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   CenterPickHandler* Center_3D_PickHandler_ptr=new CenterPickHandler(
      3,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers3_group,ModeController_3D_ptr,
      window_mgr_3D_ptr,grid_origin_ptr);

   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back(
      Center_2D_PickHandler_ptr);
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      Center_3D_PickHandler_ptr);

// Instantiate features decorations group:

   decorations_2D.add_Features(
      2,passes_group.get_pass_ptr(videopass_ID),
      &centers2_group,movie_ptr,NULL,NULL,AnimationController_ptr);
   decorations_3D.add_Features(3,passes_group.get_pass_ptr(cloudpass_ID));

// SPASE parameters:

   decorations_2D.get_FeaturesGroup_ptr()->set_crosshairs_size(0.04);
   decorations_3D.get_FeaturesGroup_ptr()->set_crosshairs_text_size(0.05);

// Reasonable parameters for SJ7:

//   decorations_3D.get_FeaturesGroup_ptr()->set_crosshairs_size(0.1);
//   decorations_3D.get_FeaturesGroup_ptr()->set_crosshairs_text_size(0.1);


// Instantiate fusion group to hold math relationships between 3D and
// 2D imagery for draping purposes:

   FusionGroup* FusionGroup_ptr=new FusionGroup(
      &passes_group,passes_group.get_pass_ptr(videopass_ID),
      &clouds_group,movie_ptr,
      decorations_2D.get_FeaturesGroup_ptr(),
      decorations_3D.get_FeaturesGroup_ptr(),
      grid_origin_ptr,AnimationController_ptr);

   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back(
      new FusionKeyHandler(ModeController_2D_ptr,FusionGroup_ptr));
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new FusionKeyHandler(ModeController_3D_ptr,FusionGroup_ptr));

// Create the windows and run the threads:

   root_2D->addChild(decorations_2D.get_OSGgroup_ptr());
   root_3D->addChild(decorations_3D.get_OSGgroup_ptr());

   window_mgr_2D_ptr->setSceneData(root_2D);
   window_mgr_3D_ptr->setSceneData(root_3D);

   window_mgr_2D_ptr->realize();
   window_mgr_3D_ptr->realize();

// Open a single text dialog box which can be used to display both 2D
// and 3D feature information:

//  decorations_2D.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->open(
//     "Feature Information");
//  decorations_3D.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->connect(
//      features_2D_group.get_TextDialogBox_ptr()->get_tty_devname());
//  decorations_2D.get_FeaturesGroup_tpr()->update_feature_text();
//  decorations_3D.get_FeaturesGroup_tpr()->update_feature_text();

// Set initial camera view to grid's midpoint:

   CM_3D_ptr->set_worldspace_center(grid_ptr->get_world_middle());
   CM_3D_ptr->set_eye_to_center_distance(
      2*basic_math::max(grid_ptr->get_xsize(),grid_ptr->get_ysize()));
   CM_3D_ptr->update_M_and_Minv();

   while( !window_mgr_2D_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_2D_ptr->process();
      window_mgr_3D_ptr->process();
   }

//   decorations_2D->get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();
//   decorations_3D->get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->close();
}
