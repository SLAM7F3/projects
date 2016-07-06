// ========================================================================
// Program AIRGROUND_MATCH is a variant of WISPPROP.  It
// pops open a 3D window displaying a point cloud (ladar and/or
// bundler reconstructed) and georegistered WISP panel frames'
// OBSFRUSTA.  Aerial OBSFRUSTA corresponding to reconstructed Twin
// Otter imagery are also displayed in the 3D map.

// AIRGROUND_MATCH opens another 2D window in which the video frame
// for the most recently selected WISP OBSFRUSTUM is displayed.  After
// pressing "Q" to enter into Input Rectangle Mode, the user can
// select a rectangular subregion within the WISP video frame.
// Right/left [up/down] arrow keys can be used to increase/decrease
// the selected rectangles width [height].  When the 2D imagery window
// is in Manipulate Rectangle Mode and the user presses "p" in the 2D
// window, a red subfrustum is displayed within the 3D imagery window
// which represents the best estimate for the world space subvolume
// corresponding to the input 2D rectangle.

// Within the 2D imagery window, the user may subsequently press "a"
// to instruct the machine to find the aerial image which has maximal
// overlap with the selected 3D worldspace subvolume.  A cyan
// OBSFRUSTUM is drawn from the automatically selected aerial camera's
// location down to nearly the location of the ground subvolume's end
// point.  The user may fly into the aerial OBSFRUSTUM and fade away
// its image to compare the corresponding aerial view with the
// selected ground region within the WISP image.

/*

./airground_match \
--region_filename ./packages/miniHAFB.pkg \
--region_filename ./packages/frame_p0_0000.pkg \
--region_filename ./packages/frame_p1_0000.pkg \
--region_filename ./packages/frame_p2_0000.pkg \
--region_filename ./packages/frame_p3_0000.pkg \
--region_filename ./packages/frame_p4_0000.pkg \
--region_filename ./packages/frame_p5_0000.pkg \
--region_filename ./packages/frame_p6_0000.pkg \
--region_filename ./packages/frame_p7_0000.pkg \
--region_filename ./packages/frame_p8_0000.pkg \
--region_filename ./packages/frame_p9_0000.pkg \
--region_filename ./bundler/HAFB/5-25-flight1/packages/photo_0000.pkg \
--region_filename ./bundler/HAFB/5-25-flight1/packages/photo_0001.pkg \
.
.
.
--region_filename ./bundler/HAFB/5-25-flight1-CreditUnion/packages/photo_0184.pkg \
--region_filename ./bundler/HAFB/5-25-flight1-CreditUnion/packages/photo_0185.pkg \
--ActiveMQ_hostname tcp://127.0.0.1:61616 \
--initial_mode Manipulate_Fused_Data_Mode

*/

// ========================================================================
// Last updated on 7/10/11; 7/17/11; 7/18/11
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/ArrowsGroup.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgFusion/FusionGroup.h"
#include "osg/osgFusion/FusionKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgTiles/ray_tracer.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

#include "geometry/polyline.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   cout << "videopass_ID = " << videopass_ID << endl;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Read combined ground WISP and aerial images from input video passes:

   photogroup* total_photogroup_ptr=new photogroup;
   total_photogroup_ptr->read_photographs(passes_group);
   int n_photos=total_photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   int n_ground_photos=10;	// WISP
   photogroup* ground_photogroup_ptr=total_photogroup_ptr->generate_subgroup(
      0,n_ground_photos-1);
   n_ground_photos=ground_photogroup_ptr->get_n_photos();
   cout << "n_ground_photos = " << n_ground_photos << endl;
   
   photogroup* aerial_photogroup_ptr=total_photogroup_ptr->
      generate_subgroup(n_ground_photos,n_photos-1);
   int n_aerial_photos=aerial_photogroup_ptr->get_n_photos();
   cout << "n_aerial_photos = " << n_aerial_photos << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_2D_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   string window_2D_title="2D imagery";
   string window_3D_title="3D imagery";

   window_mgr_2D_ptr->initialize_dual_windows(
      window_2D_title,window_3D_title,window_mgr_3D_ptr);

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

    string message_sender_ID="PROPAGATOR";

// Instantiate photo network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   Messenger OBSFRUSTA_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);
   Messenger Movies_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

// Create two OSG root nodes:

   osg::Group* root_2D = new osg::Group;
   osg::Group* root_3D = new osg::Group;
 
// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=false;
   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
//   bool hide_Mode_HUD_flag=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(2,window_mgr_2D_ptr,display_movie_state,
                         display_movie_number,hide_Mode_HUD_flag);

   ModeController* ModeController_2D_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
//   AnimationController_ptr->set_nframes(n_photos);
   root_2D->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_3D_ptr=new ModeController();
   ModeController_3D_ptr->setState(ModeController::MANIPULATE_FUSED_DATA);
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

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   string map_countries_name="HAFB";
   string geotif_subdir="/data/DTED/"+map_countries_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_ladar_height_data_flag(true);
   TilesGroup_ptr->purge_tile_files();

   string ladar_tile_filename=geotif_subdir+"Ztiles/"+
      "larger_flightfacility_ladarmap.tif";
   TilesGroup_ptr->load_ladar_height_data_into_ztwoDarray(ladar_tile_filename);

   ray_tracer* ray_tracer_ptr=new ray_tracer();
   ray_tracer_ptr->set_TilesGroup_ptr(TilesGroup_ptr);

// Instantiate group to hold 2D movie:

   MoviesGroup movies_group(
      2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   movies_group.set_photogroup_ptr(ground_photogroup_ptr);
   movies_group.pushback_Messenger_ptr(&Movies_photo_network_messenger);
   movies_group.set_play_photos_as_video_flag(false);
   movies_group.set_aerial_video_frame_flag(false);

   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   root_2D->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_2D_ptr,&movies_group);
   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back(
      MoviesKeyHandler_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   decorations_3D.set_PointCloudsGroup_ptr(&clouds_group);
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
//   clouds_group.set_auto_resize_points_flag(false);
   clouds_group.set_auto_resize_points_flag(true);

   LatLongGridsGroup latlonggrids_group(
      3,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);
   earth_regions_group.generate_regions(passes_group);
   root_3D->addChild( earth_regions_group.get_OSGgroup_ptr() );

   double Zmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMax();
   ray_tracer_ptr->set_max_zground(Zmax);

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
//   LatLongGrid_ptr->set_dynamic_grid_flag(false);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations_3D.set_grid_origin_ptr(grid_origin_ptr);

//   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
//   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
//   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
//   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();

   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_3D_ptr,LatLongGrid_ptr));

   root_3D->addChild(clouds_group.get_OSGgroup_ptr());
   root_3D->addChild(latlonggrids_group.get_OSGgroup_ptr());

// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
//      clouds_group.get_SetupGeomVisitor_ptr(),
//      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate OBSFRUSTA for all aerial video frames.  Need to
// instantiate aerial OBSFRUSTAGROUP BEFORE ground OBSFRUSTAGROUP in
// order for picking of Twin Otter OBSFRUSTA to take precedence over
// picking of WISP OBSFRUSTA:

   OBSFRUSTAGROUP* Aerial_OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   Aerial_OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   Aerial_OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);



   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   Aerial_OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      aerial_photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

// Instantiate OBSFRUSTA for picked aerial video frame:

   OBSFRUSTAGROUP* sub_Aerial_OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   sub_Aerial_OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
   sub_Aerial_OBSFRUSTAGROUP_ptr->set_photogroup_ptr(aerial_photogroup_ptr);
   sub_Aerial_OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   sub_Aerial_OBSFRUSTAGROUP_ptr->set_PARENT_OBSFRUSTAGROUP_ptr(
      Aerial_OBSFRUSTAGROUP_ptr);

// Instantiate OBSFRUSTA for ground-level WISP video frames:

   OBSFRUSTAGROUP* Ground_OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   Ground_OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
   Ground_OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   Ground_OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);

// Instantiate an individual OBSFRUSTUM for every still ground image:

   Ground_OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      ground_photogroup_ptr,
      multicolor_frusta_flag,thumbnails_flag);
   Ground_OBSFRUSTAGROUP_ptr->set_Movie_visibility_flag(true);

// Instantiate OBSFRUSTA for picked rectangular regions within WISP
// video frames:

   OBSFRUSTAGROUP* sub_Ground_OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   sub_Ground_OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
   sub_Ground_OBSFRUSTAGROUP_ptr->set_photogroup_ptr(ground_photogroup_ptr);
   sub_Ground_OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   sub_Ground_OBSFRUSTAGROUP_ptr->set_PARENT_OBSFRUSTAGROUP_ptr(
      Ground_OBSFRUSTAGROUP_ptr);

   cout << "Aerial_OBSFRUSTAGROUP_ptr = " << Aerial_OBSFRUSTAGROUP_ptr << endl;
   cout << "sub_Aerial_OBSFRUSTAGROUP_ptr = " 
        << sub_Aerial_OBSFRUSTAGROUP_ptr << endl;
   cout << "Ground_OBSFRUSTAGROUP_ptr = " << Ground_OBSFRUSTAGROUP_ptr << endl;
   cout << "sub_Ground_OBSFRUSTAGROUP_ptr = " 
        << sub_Ground_OBSFRUSTAGROUP_ptr << endl;

// Instantiate 3D & 2D features decorations group:

   FeaturesGroup* FeaturesGroup_3D_ptr=
      decorations_3D.add_Features(3,passes_group.get_pass_ptr(cloudpass_ID));

   FeaturesGroup* FeaturesGroup_2D_ptr=decorations_2D.add_Features(
      2,passes_group.get_pass_ptr(videopass_ID),
      NULL,movie_ptr,NULL,NULL,AnimationController_ptr);
   FeaturesGroup_2D_ptr->set_raytracer_ptr(ray_tracer_ptr);
   FeaturesGroup_2D_ptr->set_OBSFRUSTAGROUP_ptr(Ground_OBSFRUSTAGROUP_ptr);

   FeaturesGroup_2D_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   FeaturesGroup_2D_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
   FeaturesGroup_2D_ptr->set_photogroup_ptr(ground_photogroup_ptr);
   FeaturesGroup_2D_ptr->set_FeaturesGroup_3D_ptr(FeaturesGroup_3D_ptr);
   FeaturesGroup_2D_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);

   FeaturesGroup_2D_ptr->set_display_geocoords_flag(false);
//   FeaturesGroup_2D_ptr->set_display_geocoords_flag(true);

// Instantiate rectangles decorations group:

   RectanglesGroup* RectanglesGroup_ptr=decorations_2D.add_Rectangles(
      2,passes_group.get_pass_ptr(videopass_ID));

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations_3D.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_photogroup_ptr(
      ground_photogroup_ptr);

// Instantiate Polyhedra decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations_3D.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);

// Instantiate fusion group to hold math relationships between 3D and
// 2D imagery:

   FusionGroup* FusionGroup_ptr=new FusionGroup(
      &passes_group,passes_group.get_pass_ptr(videopass_ID),
      &clouds_group,movie_ptr,
      FeaturesGroup_2D_ptr,FeaturesGroup_3D_ptr,
      grid_origin_ptr,AnimationController_ptr);
   
   window_mgr_2D_ptr->get_EventHandlers_ptr()->push_back(
      new FusionKeyHandler(ModeController_2D_ptr,FusionGroup_ptr));
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new FusionKeyHandler(ModeController_3D_ptr,FusionGroup_ptr));

   FusionGroup_ptr->set_RectanglesGroup_ptr(RectanglesGroup_ptr);
   FusionGroup_ptr->set_raytracer_ptr(ray_tracer_ptr);
   FusionGroup_ptr->set_Aerial_OBSFRUSTAGROUP_ptr(Aerial_OBSFRUSTAGROUP_ptr);
   FusionGroup_ptr->set_Ground_OBSFRUSTAGROUP_ptr(Ground_OBSFRUSTAGROUP_ptr);
   FusionGroup_ptr->set_sub_Ground_OBSFRUSTAGROUP_ptr(
      sub_Ground_OBSFRUSTAGROUP_ptr);
   FusionGroup_ptr->set_sub_Aerial_OBSFRUSTAGROUP_ptr(
      sub_Aerial_OBSFRUSTAGROUP_ptr);
   FusionGroup_ptr->set_PolyhedraGroup_ptr(PolyhedraGroup_ptr);

// Create the windows and run the threads:

   decorations_2D.set_DataNode_ptr(movie_ptr->getGeode());

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

// ========================================================================

   while( !window_mgr_2D_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_2D_ptr->process();
      window_mgr_3D_ptr->process();
   }
   delete window_mgr_2D_ptr;
   delete window_mgr_3D_ptr;
}


