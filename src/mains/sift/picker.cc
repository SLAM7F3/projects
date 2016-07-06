// ========================================================================
// Program PICKER pops open a 2D window in which a video can play and
// a 3D window in which a corresponding ladar point cloud is
// displayed.  User-selected features within the 2D window are
// backprojected into the ladar point cloud.  3D feature tiepoint
// counterparts are blinked within the ladar cloud and reprojected
// into all video frames.  3D range and altitude information
// is displayed in the video window next to each 2D feature.
// ========================================================================
// Last updated on 2/16/09; 2/20/09; 2/22/09; 3/1/09; 5/26/09
// ========================================================================

#include <iostream>
#include <string>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
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

   osg::ArgumentParser arguments(&argc,argv);
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

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root_2D->addChild( movies_group.get_OSGgroup_ptr() );

// As of 2/16/09, we hard-code geolocation of camera for Jan 6, 2009
// Boston skyline video sequence.  Should eventually read this
// parameter in via package file...

   threevector camera_XYZ(327957.5 , 4691898.5 , 87.0199966431);
   movies_group.set_static_camera_posn_offset(camera_XYZ);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   decorations_3D.set_PointCloudsGroup_ptr(&clouds_group);

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,&movies_group);
   earth_regions_group.generate_regions(passes_group);

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   LatLongGrid_ptr->set_text_size_prefactor(2.5E-4);
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations_3D.set_grid_origin_ptr(grid_origin_ptr);

   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_3D_ptr,LatLongGrid_ptr));

   root_3D->addChild(clouds_group.get_OSGgroup_ptr());
   root_3D->addChild(latlonggrids_group.get_OSGgroup_ptr());

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   pointfinder.set_minimal_allowed_range(100);	// meters
//   pointfinder.set_maximal_rho(10);
   pointfinder.set_maximal_rho(20);	// meters
//   pointfinder.set_maximal_rho(30);	// meters
//   pointfinder.set_maximal_rho(70);	// meters
   pointfinder.set_max_cone_halfangle(4);	// degs
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate 3D & 2D features decorations group:

   FeaturesGroup* FeaturesGroup_3D_ptr=
      decorations_3D.add_Features(3,passes_group.get_pass_ptr(cloudpass_ID));

   FeaturesGroup* FeaturesGroup_2D_ptr=decorations_2D.add_Features(
      2,passes_group.get_pass_ptr(videopass_ID),
      NULL,movie_ptr,NULL,NULL,AnimationController_ptr);
   decorations_2D.set_DataNode_ptr(movie_ptr->getGeode());

   FeaturesGroup_2D_ptr->set_AnimationController_ptr(AnimationController_ptr);
   FeaturesGroup_2D_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   FeaturesGroup_2D_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
   FeaturesGroup_2D_ptr->set_MoviesGroup_ptr(&movies_group);
   FeaturesGroup_2D_ptr->set_PointFinder_ptr(&pointfinder);
   FeaturesGroup_2D_ptr->set_FeaturesGroup_3D_ptr(FeaturesGroup_3D_ptr);
   FeaturesGroup_2D_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   FeaturesGroup_2D_ptr->set_display_geocoords_flag(false);
//   FeaturesGroup_2D_ptr->set_display_geocoords_flag(true);

// Instantiate 3D SignPosts decorations group:

   decorations_3D.add_SignPosts(
      3,passes_group.get_pass_ptr(cloudpass_ID));
//   for (int n=0; n<decorations_3D.get_n_SignPostsGroups(); n++)
//   {
//      decorations_3D.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
//         &earth_regions_group);
//      decorations_3D.get_SignPostsGroup_ptr(n)->set_Clock_ptr(&clock);
//   }

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate dynamic OBSFRUSTA for video(s):

   double Zplane_altitude=-10000;	// meters 
//   double Zplane_altitude=0;	// meters 
   bool multicolor_frusta_flag=false;
   bool initially_mask_all_frusta_flag=false;

   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,Zplane_altitude,
      multicolor_frusta_flag,initially_mask_all_frusta_flag);

// Create the windows and run the threads:

   root_2D->addChild(decorations_2D.get_OSGgroup_ptr());
   root_3D->addChild(decorations_3D.get_OSGgroup_ptr());

   window_mgr_2D_ptr->setSceneData(root_2D);
   window_mgr_3D_ptr->setSceneData(root_3D);

   window_mgr_2D_ptr->realize();
   window_mgr_3D_ptr->realize();

// Adjust 3D viewer's field-of-view if some positive virtual_horiz_FOV
// parameter is passed an input argument:

   double virtual_horiz_FOV=passes_group.get_virtual_horiz_FOV();
   if (virtual_horiz_FOV > 0)
   {
      double FOV_h=window_mgr_3D_ptr->get_lens_horizontal_FOV();
      double angular_scale_factor=virtual_horiz_FOV/FOV_h;  
      cout << "virtual_horiz_FOV = " << virtual_horiz_FOV << endl;
//   cout << "angular_scale_factor = " << angular_scale_factor << endl;
      window_mgr_3D_ptr->rescale_viewer_FOV(angular_scale_factor);
   }
   
   while( !window_mgr_2D_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_2D_ptr->process();
      window_mgr_3D_ptr->process();
   }

   delete window_mgr_2D_ptr;
   delete window_mgr_3D_ptr;
}
