// ========================================================================
// Program WISPPROP is a variant of mains/photosynth/PROPAGATOR.  It
// pops open a 3D window displaying a point cloud (ladar and/or
// bundler reconstructed) and georegistered WISP panel frames'
// OBSFRUSTA. It also opens a 2D window in which the video frame for
// the most recently selected OBSFRUSTUM is displayed.

// This program projects 3D SignPosts into the selected 2D image
// plane.  It also backprojects 2D features into the 3D map taking
// occlusion into account.  The feature is subsequently reprojected
// into different image planes, and its instantaneous range and
// altitude are displayed.

/*

./wispprop \
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
--ActiveMQ_hostname tcp://127.0.0.1:61616 \
--initial_mode Manipulate_Fused_Data_Mode

*/

// ========================================================================
// Last updated on 2/27/11; 6/4/11; 7/9/11
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
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
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

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

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

   string wiki_message_queue_channel_name="wiki";
   Messenger wiki_messenger( broker_URL, wiki_message_queue_channel_name );

// Create two OSG root nodes:

   osg::Group* root_2D = new osg::Group;
   osg::Group* root_3D = new osg::Group;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);
   
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
   AnimationController_ptr->set_nframes(n_photos);
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

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   movies_group.set_photogroup_ptr(photogroup_ptr);
   movies_group.pushback_Messenger_ptr(&Movies_photo_network_messenger);
//   movies_group.set_DTED_ztwoDarray_ptr(DTED_ztwoDarray_ptr);
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

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

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

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   pointfinder.set_minimal_allowed_range(100);	// meters
//   pointfinder.set_maximal_rho(10);
   pointfinder.set_maximal_rho(20);	// meters
//   pointfinder.set_maximal_rho(30);	// meters
//   pointfinder.set_maximal_rho(70);	// meters
   pointfinder.set_max_cone_halfangle(4);	// degs
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate Arrows decoration group:

   ArrowsGroup* ArrowsGroup_ptr=decorations_3D.add_Arrows(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   ArrowsGroup_ptr->set_altitude_dependent_size_flag(false);
//   ArrowsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

// Instantiate 3D & 2D features decorations group:

   FeaturesGroup* FeaturesGroup_3D_ptr=
      decorations_3D.add_Features(3,passes_group.get_pass_ptr(cloudpass_ID));

   FeaturesGroup* FeaturesGroup_2D_ptr=decorations_2D.add_Features(
      2,passes_group.get_pass_ptr(videopass_ID),
      NULL,movie_ptr,NULL,NULL,AnimationController_ptr);
   FeaturesGroup_2D_ptr->set_raytracer_ptr(ray_tracer_ptr);

   FeaturesGroup_2D_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   FeaturesGroup_2D_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
//    FeaturesGroup_2D_ptr->set_MoviesGroup_ptr(&movies_group);
   FeaturesGroup_2D_ptr->set_photogroup_ptr(photogroup_ptr);
   FeaturesGroup_2D_ptr->set_PointFinder_ptr(&pointfinder);
   FeaturesGroup_2D_ptr->set_FeaturesGroup_3D_ptr(FeaturesGroup_3D_ptr);
   FeaturesGroup_2D_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);

   FeaturesGroup_2D_ptr->set_display_geocoords_flag(false);
//   FeaturesGroup_2D_ptr->set_display_geocoords_flag(true);
//   FeaturesGroup_2D_ptr->set_DTED_ztwoDarray_ptr(DTED_ztwoDarray_ptr);

// Instantiate rectangles decorations group:

   decorations_2D.add_Rectangles(
      2,passes_group.get_pass_ptr(videopass_ID));

// Instantiate 3D SignPosts decorations group:

   SignPostsGroup* SignPostsGroup_ptr=decorations_3D.add_SignPosts(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);

   for (int n=0; n<decorations_3D.get_n_SignPostsGroups(); n++)
   {
      decorations_3D.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
   }

   SignPostsGroup_ptr->set_photogroup_ptr(photogroup_ptr);
   SignPostsGroup_ptr->set_aerial_video_frame_flag(true);
   SignPostsGroup_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   SignPostsGroup_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());
//   cout << "package_subdir = " 
//        <<  passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
//      get_package_subdir() << endl;
//   cout << "package_filename_prefix = "
//        <<  passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
//      get_package_filename_prefix() << endl;
   SignPostsGroup_ptr->set_MoviesGroup_ptr(&movies_group);

   SignPostsKeyHandler* SignPostsKeyHandler_ptr=decorations_3D.
      get_SignPostsKeyHandler_ptr();
   SignPostsKeyHandler_ptr->set_AnimationController_ptr(
      AnimationController_ptr);

// Instantiate 2D imageplane SignPosts group and pick handler:

   SignPostsGroup* imageplane_SignPostsGroup_ptr=
      decorations_2D.add_SignPosts(2,passes_group.get_pass_ptr(videopass_ID));

   imageplane_SignPostsGroup_ptr->set_package_subdir(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_subdir());
   imageplane_SignPostsGroup_ptr->set_package_filename_prefix(
      passes_group.get_videopass_ptr()->get_PassInfo_ptr()->
      get_package_filename_prefix());

   imageplane_SignPostsGroup_ptr->set_aerial_video_frame_flag(true);
   imageplane_SignPostsGroup_ptr->set_AnimationController_ptr(
      AnimationController_ptr);
   imageplane_SignPostsGroup_ptr->set_EarthRegionsGroup_ptr(
      &earth_regions_group);
   imageplane_SignPostsGroup_ptr->set_photogroup_ptr(photogroup_ptr);
   imageplane_SignPostsGroup_ptr->set_PointFinder_ptr(&pointfinder);
   imageplane_SignPostsGroup_ptr->set_SignPostsGroup_3D_ptr(
      SignPostsGroup_ptr);
   imageplane_SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);

   SignPostPickHandler* imageplane_SignPostPickHandler_ptr=
      decorations_2D.get_SignPostPickHandler_ptr();

   imageplane_SignPostPickHandler_ptr->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);
   imageplane_SignPostPickHandler_ptr->set_DataNode_ptr(
      movie_ptr->getGeode());

   SignPostsGroup_ptr->set_imageplane_SignPostsGroup_ptr(
      imageplane_SignPostsGroup_ptr);
   SignPostsGroup_ptr->set_imageplane_SignPostPickHandler_ptr(
      imageplane_SignPostPickHandler_ptr);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations_3D.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
//   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
//   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(false);
   OBSFRUSTAGROUP_ptr->set_flashlight_mode_flag(false);
//   OBSFRUSTAGROUP_ptr->set_DTED_ztwoDarray_ptr(DTED_ztwoDarray_ptr);
//   OBSFRUSTAGROUP_ptr->set_ArrowsGroup_ptr(ArrowsGroup_ptr);
   FeaturesGroup_2D_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

// Instantiate an individual OBSFRUSTUM for every still image:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   double frustum_sidelength=20;
   
   double movie_downrange_distance=18;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,
      frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);

//   OBSFRUSTAGROUP_ptr->display_OBSFRUSTA_as_time_sequence();

// FAKE FAKE:  Fri, February 11, 2011 at 2:06 pm
// Hide movie for viewgraph purposes only!

//   OBSFRUSTAGROUP_ptr->set_Movie_visibility_flag(false);
   OBSFRUSTAGROUP_ptr->set_Movie_visibility_flag(true);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations_3D.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_photogroup_ptr(
      photogroup_ptr);

/*
// Instantiate PolyLines group to hold roadway GIS network:

   PolyLinesGroup* RoadLinesGroup_ptr=
      earth_regions_group.generate_roadlines_group(0);
   if (postgis_db_ptr != NULL)
   {
      postgis_db_ptr->set_PolyLinesGroup_ptr(RoadLinesGroup_ptr);

// Greater Texas Tech region:

      double roads_xmin=230525;
      double roads_ymin=3717739;
      double roads_xmax=234488;
      double roads_ymax=3721830;

      postgis_db_ptr->pushback_gis_bbox(
         roads_xmin,roads_xmax,roads_ymin,roads_ymax);
//      postgis_db_ptr->set_altitude(0);
      postgis_db_ptr->set_altitude(985);	// Lubbock, TX
      postgis_db_ptr->set_PolyLine_text_size(9);

// Lubbock_streets table in bluegrass gis database:

      string geom_name="name";		

      postgis_db_ptr->parse_table_contents(geom_name);
      postgis_db_ptr->popback_gis_bbox();
   }
   
   RoadLinesGroup_ptr->set_uniform_color(
      colorfunc::get_OSG_color(colorfunc::purple));
   int OSGsubPAT_number=RoadLinesGroup_ptr->get_n_OSGsubPATs()-1;
   RoadLinesGroup_ptr->attach_bunched_geometries_to_OSGsubPAT(
      *grid_origin_ptr,OSGsubPAT_number);
   cout << "# Roadway PolyLines = " 
        << RoadLinesGroup_ptr->get_n_Graphicals() << endl;
   RoadLinesGroup_ptr->set_OSGgroup_nodemask(1);

//   RoadLinesGroup_ptr->set_aerial_video_frame_flag(true);
   RoadLinesGroup_ptr->set_AnimationController_ptr(AnimationController_ptr);
//   RoadLinesGroup_ptr->set_MoviesGroup_ptr(&movies_group);
   RoadLinesGroup_ptr->set_photogroup_ptr(photogroup_ptr);
   movies_group.set_PolyLinesGroup_ptr(RoadLinesGroup_ptr);

// Instantiate PolyLines

   PolyLinesGroup* imageplane_RoadLinesGroup_ptr=
      decorations_2D.add_PolyLines(2,passes_group.get_pass_ptr(videopass_ID));
   imageplane_RoadLinesGroup_ptr->set_OSGgroup_nodemask(1);
   imageplane_RoadLinesGroup_ptr->set_AnimationController_ptr(
      AnimationController_ptr);

   imageplane_RoadLinesGroup_ptr->set_photogroup_ptr(photogroup_ptr);
   imageplane_RoadLinesGroup_ptr->set_PolyLinesGroup_3D_ptr(
      RoadLinesGroup_ptr);
   imageplane_RoadLinesGroup_ptr->set_width(1);
   imageplane_RoadLinesGroup_ptr->set_n_text_messages(1);
   imageplane_RoadLinesGroup_ptr->set_variable_Point_size_flag(false);
   imageplane_RoadLinesGroup_ptr->set_altitude_dependent_labels_flag(false);

   RoadLinesGroup_ptr->set_imageplane_PolyLinesGroup_ptr(
      imageplane_RoadLinesGroup_ptr);
*/

/*
// Instantiate Polygons and Polyhedra decoration groups:

   osgGeometry::PolygonsGroup* imageplane_PolygonsGroup_ptr=
      decorations_2D.add_Polygons(
         2,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   imageplane_PolygonsGroup_ptr->set_PolyLinesGroup_ptr(
      imageplane_RoadLinesGroup_ptr);

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations_3D.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   decorations_3D.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      PolyhedraGroup_ptr);
   OBSFRUSTAGROUP_ptr->set_PolyhedraGroup_ptr(PolyhedraGroup_ptr);
   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(imageplane_PolygonsGroup_ptr);
*/

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

   enum PROPAGATOR_TYPE 
   {
      features,signposts,polyhedra,polylines
   };

   PROPAGATOR_TYPE propagator_type=features;
//   PROPAGATOR_TYPE propagator_type=signposts;
//   PROPAGATOR_TYPE propagator_type=polyhedra;
//   PROPAGATOR_TYPE propagator_type=polylines;

// For NYC demo purposes, automatically retrieve all signposts from
// PostGIS database:

   colorfunc::Color signposts_color=colorfunc::white;
//   colorfunc::Color signposts_color=colorfunc::red;
   double SignPost_size=0.5;

   if (propagator_type==signposts)
   {
//      SignPostsGroup_ptr->retrieve_all_signposts_from_PostGIS_databases(
//         signposts_color,SignPost_size);
   }
   else if (propagator_type==polyhedra)
   {
      SignPostsGroup_ptr->retrieve_all_signposts_from_PostGIS_databases(
         signposts_color,SignPost_size);
      imageplane_SignPostsGroup_ptr->set_OSGgroup_nodemask(0);

/*
      PolyhedraGroup_ptr->generate_skyscraper_bboxes();      
      OBSFRUSTAGROUP_ptr->compute_photo_views_of_polyhedra_bboxes(
         photogroup_ptr);
      photogroup_ptr->order_photos_by_their_scores();
*/

   } // propagator type conditional
   
   decorations_3D.get_SignPostPickHandler_ptr()->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);


   while( !window_mgr_2D_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_2D_ptr->process();
      window_mgr_3D_ptr->process();
   }
   delete window_mgr_2D_ptr;
   delete window_mgr_3D_ptr;

   delete postgis_databases_group_ptr;

//   delete DTED_ztwoDarray_ptr;
}


