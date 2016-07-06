// ========================================================================
// Program FUSION drapes 2D video imagery onto 3D point clouds.  

// From within src/mains/fusion, chant 

//	 fusion HAFB.xyzp --region_filename ./packages/HAFB_fusion.pkg

// 			fusion HAFB.xyzp HAFB.vid

// 	      or 	fusion HAFB.xyzp HAFB_overlap_RGB.vid

//	fusion /data/alirt2/Boston_2005/boston_30cm_tiles/boston_osga/*.osga copley.vid

//	 fusion /data/alirt2/Boston_2005/boston_30cm_tiles/boston_osga/*.osga panorama.vid

/*

	 cd /data/alirt2/Boston_2005/boston_30cm_tiles/boston_osga
	 fusion --region_filename boston.pkg copley.jpg

*/


//	fusion --region_filename nyc_rtv_EO.pkg east_crop.jpg


// goto video image #10, press I twice to enter MANIPULATE FEATURE
// MODE, import default 2D and 3D features to see one example of this
// program. press "U" to enter into FUSION MODE, press "c" to combine
// 2D and 3D feature information.  Finally, press "d" to drape video
// image onto 3D point cloud.

// Note added on 9/9/06: We empirically determined that using the
// wrap4 colormap plus a 60% saturation weight leads to nice looking
// fused Boston imagery.

// ========================================================================
// Last updated on 9/21/07; 10/11/07; 10/15/07; 1/15/11
// ========================================================================

#include <iostream>
#include <string>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFusion/FusionGroup.h"
#include "osg/osgFusion/FusionKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

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

// Create two OSG root nodes:

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

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_2D(
      window_mgr_2D_ptr,ModeController_2D_ptr,CM_2D_ptr);
   Decorations decorations_3D(
      window_mgr_3D_ptr,ModeController_3D_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations_3D.add_AlirtGrid(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations_3D.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   decorations_3D.set_PointCloudsGroup_ptr(&clouds_group);
   clouds_group.generate_new_Cloud();

   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));
   root_3D->addChild(clouds_group.get_OSGgroup_ptr());

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root_2D->addChild( movies_group.get_OSGgroup_ptr() );

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations_3D.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate feature decorations groups:

   FeaturesGroup* FeaturesGroup_2D_ptr=decorations_2D.add_Features(
      2,passes_group.get_pass_ptr(videopass_ID),
      NULL,movie_ptr,NULL,NULL,AnimationController_ptr);
   FeaturesGroup* FeaturesGroup_3D_ptr=
      decorations_3D.add_Features(3,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate PolyLine decorations group:

   PolyLinesGroup* PolyLinesGroup_2D_ptr=decorations_2D.add_PolyLines(
      2,passes_group.get_pass_ptr(videopass_ID));
   PolyLinesGroup_2D_ptr->set_width(5);
   PolyLinesGroup_2D_ptr->set_n_text_messages(1);
   PolyLinesGroup_2D_ptr->set_CM_2D_ptr(CM_2D_ptr);
   PolyLinesGroup_2D_ptr->set_variable_Point_size_flag(false);
   PolyLinesGroup_2D_ptr->set_altitude_dependent_labels_flag(false);

   PolyLinesGroup* PolyLinesGroup_3D_ptr=decorations_3D.add_PolyLines(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_3D_ptr->set_width(8);
   PolyLinesGroup_3D_ptr->set_n_text_messages(1);
   PolyLinesGroup_3D_ptr->set_CM_3D_ptr(CM_3D_ptr);
   PolyLinesGroup_3D_ptr->set_variable_Point_size_flag(true);
   PolyLinesGroup_3D_ptr->set_altitude_dependent_labels_flag(false);
   PolyLinesGroup_3D_ptr->set_ID_labels_flag(true);

// Instantiate ObsFrusta decorations group:

   decorations_3D.add_ObsFrusta(
      passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);

// Instantiate fusion group to hold math relationships between 3D and
// 2D imagery for draping purposes:

   FusionGroup* FusionGroup_ptr=new FusionGroup(
      &passes_group,passes_group.get_pass_ptr(videopass_ID),
      &clouds_group,movie_ptr,
      FeaturesGroup_2D_ptr,FeaturesGroup_3D_ptr,
      PolyLinesGroup_2D_ptr,PolyLinesGroup_3D_ptr,
      grid_origin_ptr,AnimationController_ptr);
   FusionGroup_ptr->set_ObsFrustaGroup_ptr(
      decorations_3D.get_ObsFrustaGroup_ptr());
   
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

   while( !window_mgr_2D_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_2D_ptr->process();
      window_mgr_3D_ptr->process();
   }
}
