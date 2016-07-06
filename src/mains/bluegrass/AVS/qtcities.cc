// ========================================================================
// Program QTCITIES is a variant of VIDEOCITIES intended to run as a
// thick client within a netcentric architecture.  It has also been
// tailored for touch table use.
// ========================================================================
// Last updated on 12/31/11; 5/18/13; 1/5/14
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QtCore/QtCore>

#include "Qt/web/VideoServer.h"

#include "osg/osgGraphicals/AnimationController.h"
#include "Qt/web/BluegrassClient.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osg2D/ClassificationHUD.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgEarth/EarthRegionsKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "Qt/web/OSGButtonServer.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinePickHandler.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"
#include "Qt/web/SKSClient.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"
#include "track/tracks_group.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   cout << "texturepass_ID = " << texturepass_ID << endl;
   int sensormetadatapass_ID=passes_group.get_curr_sensormetadatapass_ID();
   cout << "sensor_metadata_pass_ID = " << sensormetadatapass_ID << endl;

//   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

   cout << "GISlayer_IDS = " << endl;
   templatefunc::printVector(GISlayer_IDs);
   
   vector<int> dataserver_IDs=passes_group.get_dataserver_IDs();
   int dataserverpass_ID=-1;

   string BluegrassServer_URL=passes_group.get_LogicServer_URL();
   cout << "BluegrassServer_URL = " << BluegrassServer_URL << endl;
   string OSGButtonServer_URL=passes_group.get_OSGButtonServer_URL();
   cout << "OSGButtonServer_URL = " << OSGButtonServer_URL << endl;
   string SKSDataServer_URL=passes_group.get_SKSDataServer_URL();
   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;
   string VideoServer_URL=passes_group.get_VideoServer_URL();
   cout << "VideoServer_URL = " << VideoServer_URL << endl;

// Instantiate BluegrassClient which uses Qt http functionality:

   BluegrassClient* BluegrassClient_ptr=new BluegrassClient(
      BluegrassServer_URL);
   SKSClient* SKSClient_ptr=new SKSClient(SKSDataServer_URL);

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string message_sender_ID="QTCITIES";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string urban_network_message_queue_channel_name="urban_network";
   Messenger EarthRegions_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);
   Messenger TrackLines_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);
   Messenger ROILines_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);
   Messenger KOZLines_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);

// Instantiate robot messengers for communication with Luca
// Bertucelli's MATLAB UAV task assignment codes:

   string robots_message_queue_channel_name="robots";
   Messenger EarthRegions_robots_messenger( 
      broker_URL,robots_message_queue_channel_name,message_sender_ID);

// FAKE FAKE: Sun Aug 3 at 7 am 

// For alg development purposes, use set message_sender_ID to
// ALLOW_SELF_MESSAGES.  But when Luca's MATLAB code is integrated in
// with the rest of our demo system, keep message_sender_ID at its
// initial, default value...

//   string MODELS_message_sender_ID="ALLOW_SELF_MESSAGES";
   Messenger Predator_MODELS_robots_messenger( 
      broker_URL,robots_message_queue_channel_name,message_sender_ID);
//      broker_URL,robots_message_queue_channel_name,MODELS_message_sender_ID);

// Instantiate GoogleEarth messengers for communication with Tim
// Schreiner's ROI selection tool:

   string GE_message_queue_channel_name="GoogleEarth";
   Messenger EarthRegions_GoogleEarth_messenger( 
      broker_URL,GE_message_queue_channel_name,message_sender_ID);
   Messenger FlightLines_GoogleEarth_messenger( 
      broker_URL,GE_message_queue_channel_name,message_sender_ID);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true; 
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
//   bool display_movie_world_time=false;

   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root->addChild(operations.get_OSGgroup_ptr());

   operations.get_ImageNumberHUD_ptr()->set_text_color(colorfunc::red);
   operations.get_ImageNumberHUD_ptr()->reset_text_size(1.1 , 0);
   operations.get_ImageNumberHUD_ptr()->reset_text_size(1.1 , 1);

// Display FOUO message for all Bluegrass data!

   ClassificationHUD* ClassificationHUD_ptr=new ClassificationHUD(
      passes_group.get_classification());
   ClassificationHUD_ptr->reset_text_size(1.1 , 0);
   ClassificationHUD_ptr->reset_text_size(1.1 , 1);
   root->addChild(ClassificationHUD_ptr->getProjection());

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master game clock:
   
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0);

// Instantiate clock pointer to keep track of real time:

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->current_local_time_and_UTC();

// Specify start, stop and step times for master game clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());
   operations.reset_AnimationController_world_time_params();

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_DiamondTouchTable_flag(true);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate group to hold movie:

   AnimationController* movie_anim_controller_ptr=new AnimationController;
   movie_anim_controller_ptr->set_master_AnimationController_ptr(
      AnimationController_ptr);
   PassInfo* texture_passinfo_ptr=
      passes_group.get_passinfo_ptr(texturepass_ID);
   if (texture_passinfo_ptr != NULL)
   {
      movie_anim_controller_ptr->correlate_frames_to_world_times(
         texture_passinfo_ptr->get_frame_times());
   }

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(texturepass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),movie_anim_controller_ptr);
   
   movies_group.set_OSGgroup_nodemask(1);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,&movies_group);
   EarthRegionsKeyHandler* EarthRegionsKeyHandler_ptr=
      new EarthRegionsKeyHandler(&earth_regions_group,ModeController_ptr,
                                 &movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      EarthRegionsKeyHandler_ptr);

   earth_regions_group.set_PassesGroup_ptr(&passes_group);
   earth_regions_group.set_AnimationController_ptr(AnimationController_ptr);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_urban_network_messenger);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_robots_messenger);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);
   earth_regions_group.assign_EarthRegionsGroup_Messenger_ptrs();
   BluegrassClient_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);

   earth_regions_group.generate_regions(passes_group);
   root->addChild( earth_regions_group.get_OSGgroup_ptr() );

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   LatLongGrid_ptr->set_dynamic_grid_flag(false);
//   LatLongGrid_ptr->set_dynamic_grid_flag(true);
//   LatLongGrid_ptr->set_z_plane(1200);
//   LatLongGrid_ptr->set_world_origin_and_middle();
//   LatLongGrid_ptr->update_grid();

   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   decorations.set_PointCloudsGroup_ptr(&clouds_group);

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   double zmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().zMin();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   if (!passes_group.get_pick_points_on_Zplane_flag())
   {
      CM_3D_ptr->set_PointFinder(&pointfinder);
   }
   
// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   
// Instantiate Polyhedra and Polygons decoration groups:

   PolyhedraGroup* ARs_PolyhedraGroup_ptr=
      decorations.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
//   ARs_PolyhedraGroup_ptr->set_geometric_mean_text_size_flag(true);
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate PolyLinesGroup to hold ground truth vehicle tracks:   

   PolyLinesGroup* TrackLinesGroup_ptr=
      decorations.add_PolyLines(
         ndims,passes_group.get_pass_ptr(cloudpass_ID));
   TrackLinesGroup_ptr->set_GraphicalsGroup_bug_ptr(TrackLinesGroup_ptr);

//   TrackLinesGroup_ptr->set_width(2);	// for touch table projector
//   TrackLinesGroup_ptr->set_width(4);	// for ISDS3D laptop
   TrackLinesGroup_ptr->set_width(6);	// for ppt presentations
   TrackLinesGroup_ptr->set_width(passes_group.get_line_width());
   TrackLinesGroup_ptr->pushback_Messenger_ptr(
      &TrackLines_urban_network_messenger);

   PolyLinePickHandler* TrackLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr(0);
   TrackLinePickHandler_ptr->set_approx_range_to_polyline(3);	// meters

// Note added on 6/30/08: Will probably need to relax next boolean
// restriction once we want to start repairing ground truth tracks...

   TrackLinePickHandler_ptr->set_Allow_Insertion_flag(false);

// Instantiate separate PolyLines group to hold ROI skeletons:

   RegionPolyLinesGroup* ROILinesGroup_ptr=
      earth_regions_group.generate_ROIlines_group(0);
   ROILinesGroup_ptr->set_ROI_PolyLinesGroup_flag(true);
//   ROILinesGroup_ptr->set_width(4);	// for ISDS laptop
//   ROILinesGroup_ptr->set_width(6);	// for ppt presentations
   ROILinesGroup_ptr->set_width(passes_group.get_line_width());
   ROILinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   ROILinesGroup_ptr->pushback_Messenger_ptr(
      &ROILines_urban_network_messenger);

   EarthRegion* EarthRegion_ptr=earth_regions_group.
      get_ID_labeled_EarthRegion_ptr(0);
   ROILinesGroup_ptr->set_movers_group_ptr(
      EarthRegion_ptr->get_movers_group_ptr());

   double ROI_skeleton_height=
      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
      get_ROI_skeleton_height();
   if (ROI_skeleton_height > 0)
   {
      ROILinesGroup_ptr->set_skeleton_height(ROI_skeleton_height);
   }

   RegionPolyLinePickHandler* ROILinePickHandler_ptr=
      new RegionPolyLinePickHandler(
         passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr,
         ROILinesGroup_ptr,ModeController_ptr,window_mgr_ptr,grid_origin_ptr);
   ROILinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   ROILinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   ROILinePickHandler_ptr->set_z_offset(5);
   ROILinePickHandler_ptr->set_min_points_picked(3);
   ROILinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
   ROILinePickHandler_ptr->set_surface_picking_flag(false);
   ROILinePickHandler_ptr->set_Zplane_picking_flag(false);
   ROILinePickHandler_ptr->set_Allow_Manipulation_flag(false);

// Specify color for ROI polyhedron skeleton:

   colorfunc::Color ROI_color=colorfunc::white;
   string ROI_skeleton_color=
      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
      get_ROI_skeleton_color();

   if (ROI_skeleton_color.size() > 0)
   {
      ROI_color=colorfunc::string_to_color(ROI_skeleton_color);
   }
   earth_regions_group.set_ROI_color(ROI_color);
   ROILinePickHandler_ptr->set_permanent_color(ROI_color);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      ROILinePickHandler_ptr);

// Instantiate separate PolyLines group to hold KOZ skeletons:

   RegionPolyLinesGroup* KOZLinesGroup_ptr=
      earth_regions_group.generate_KOZlines_group(0);
   KOZLinesGroup_ptr->set_KOZ_PolyLinesGroup_flag(true);
//   KOZLinesGroup_ptr->set_width(4);	// for ISDS laptop
//   KOZLinesGroup_ptr->set_width(6);	// for ppt presentations
   KOZLinesGroup_ptr->set_width(passes_group.get_line_width());
   KOZLinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   KOZLinesGroup_ptr->pushback_Messenger_ptr(
      &KOZLines_urban_network_messenger);
   KOZLinesGroup_ptr->set_movers_group_ptr(
      EarthRegion_ptr->get_movers_group_ptr());

   double KOZ_skeleton_height=2500;
//      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
//      get_KOZ_skeleton_height();
   if (KOZ_skeleton_height > 0)
   {
      KOZLinesGroup_ptr->set_skeleton_height(KOZ_skeleton_height);
   }

   RegionPolyLinePickHandler* KOZLinePickHandler_ptr=
      new RegionPolyLinePickHandler(
         passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr,
         KOZLinesGroup_ptr,ModeController_ptr,window_mgr_ptr,grid_origin_ptr);
   KOZLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   KOZLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   KOZLinePickHandler_ptr->set_z_offset(5);
   KOZLinePickHandler_ptr->set_min_points_picked(3);
   KOZLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
   KOZLinePickHandler_ptr->set_surface_picking_flag(false);
   KOZLinePickHandler_ptr->set_Zplane_picking_flag(false);
   KOZLinePickHandler_ptr->set_Allow_Manipulation_flag(false);

// Specify color for KOZ polyhedron skeleton:

   colorfunc::Color KOZ_color=colorfunc::red;
//   string KOZ_skeleton_color=
//      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
//      get_KOZ_skeleton_color();
//   if (KOZ_skeleton_color.size() > 0)
//   {
//      KOZ_color=colorfunc::string_to_color(KOZ_skeleton_color);
//   }
   earth_regions_group.set_KOZ_color(KOZ_color);
   KOZLinePickHandler_ptr->set_permanent_color(KOZ_color);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      KOZLinePickHandler_ptr);

// Instantiate PolyLinesGroup to hold Predator flight paths.  Interact
// with these Polylines in INSERT[MANIPULATE]_POLYLINE mode rather
// than INSERT[MANIPULATE]_LINE mode:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(4);
//   Flight_PolyLinesGroup_ptr->set_width(6);
   PolyLinePickHandler* Flight_PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr(1);
   Flight_PolyLinePickHandler_ptr->set_regular_vertex_spacing(100); // meters
   Flight_PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(false);

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   SignPostsGroup_ptr->pushback_Messenger_ptr(&wiki_messenger);
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);

   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(clock_ptr);
   }

// Display Locations of Interest as orange SignPosts:

   colorfunc::Color signposts_color=colorfunc::orange;
//   double SignPost_size=50;
//   double SignPost_size=5;
//   double SignPost_size=1;
//   double SignPost_size=0.25;
   double SignPost_size=0.125;
//   double SignPost_size=0.05;
   SignPostsGroup_ptr->retrieve_all_signposts_from_PostGIS_databases(
      signposts_color,SignPost_size);
   decorations.get_SignPostPickHandler_ptr()->
      set_allow_doubleclick_in_manipulate_fused_data_mode(true);

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   decorations.get_FeaturesGroup_ptr()->set_EarthRegionsGroup_ptr(
//      &earth_regions_group);
//   decorations.add_ArmySymbols(passes_group.get_pass_ptr(cloudpass_ID));

   decorations.add_SphereSegments(passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(
         passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   CylindersGroup_ptr->set_rh(3,0.35);
//   CylindersGroup_ptr->set_rh(5,0.35);
   earth_regions_group.set_CylindersGroup_ptr(CylindersGroup_ptr);

   CylinderPickHandler* CylinderPickHandler_ptr=
      decorations.get_CylinderPickHandler_ptr();
   CylinderPickHandler_ptr->set_text_size(8);
   CylinderPickHandler_ptr->set_text_screen_axis_alignment_flag(false);
   CylinderPickHandler_ptr->set_PickHandlerCallbacks_ptr(
      BluegrassClient_ptr);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_z_ColorMap_ptr(
      clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr());
   OBSFRUSTAGROUP_ptr->set_ImageNumberHUD_ptr(
      operations.get_ImageNumberHUD_ptr());      
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_Grid_ptr(
      LatLongGrid_ptr);
   decorations.get_OBSFRUSTUMPickHandler_ptr()->set_Grid_ptr(
      LatLongGrid_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      ARs_PolyhedraGroup_ptr);

   double Zplane_altitude=0;	// meters
   bool multicolor_frusta_flag=false;
//   bool initially_mask_all_frusta_flag=true;
   bool initially_mask_all_frusta_flag=false;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,Zplane_altitude,multicolor_frusta_flag,
      initially_mask_all_frusta_flag);

// For Bluegrass demo, instantiate Cessna model decorations
// group. Then generate Cessna model, racetrack orbit and Constant
// Hawk OBSFRUSTUM:

   if (sensormetadatapass_ID >= 0)
   {
      MODELSGROUP* Cessna_MODELSGROUP_ptr=decorations.add_MODELS(
         passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

      int cessna_OSGsubPAT_number;
      MODEL* cessna_ptr=Cessna_MODELSGROUP_ptr->generate_Cessna_MODEL(
         cessna_OSGsubPAT_number);
      
      cessna_ptr->set_dynamically_compute_OBSFRUSTUM_flag(true);
//      cessna_ptr->push_back_OBSFRUSTUM_pitch(-70*PI/180);
//      cessna_ptr->push_back_OBSFRUSTUM_yaw(90*PI/180);
      cessna_ptr->push_back_OBSFRUSTUM_pitch(00*PI/180);
      cessna_ptr->push_back_OBSFRUSTUM_roll(20*PI/180);
//      cessna_ptr->set_OBSFRUSTUM_z_base_face(grid_origin_ptr->get(2)+35); 
       // AR1
      cessna_ptr->push_back_OBSFRUSTUM_z_base_face(
         grid_origin_ptr->get(2)+71);  // Full Lubbock 3D map

      vector<Triple<threevector,rpy,int> > sensor_posn_orientation_frames=
         passes_group.get_pass_ptr(sensormetadatapass_ID)->
         get_PassInfo_ptr()->get_posn_orientation_frames();

      vector<threevector> posn_reg,orientation_reg;
      Cessna_MODELSGROUP_ptr->regularize_aircraft_posns_and_orientations(
         sensor_posn_orientation_frames,movie_anim_controller_ptr,
         posn_reg,orientation_reg);
      Cessna_MODELSGROUP_ptr->generate_racetrack_orbit(posn_reg,cessna_ptr);

      double FOV_az_extent=65*PI/180;  	// radians
      double FOV_el_extent=65*PI/180;	// radians
      OBSFRUSTUM* cessna_FOV_OBSFRUSTUM_ptr=
         Cessna_MODELSGROUP_ptr->instantiate_MODEL_FOV_OBSFRUSTUM(
            cessna_OSGsubPAT_number,cessna_ptr,FOV_az_extent,FOV_el_extent);

// Recall instantiate_MODEL_FOV_OBSFRUSTUM automatically unmasks
// Cessna and its OBSFRUSTUM.  For demo purposes, we want to initially
// mask Cessna & its OBSFRUSTUM:

      Cessna_MODELSGROUP_ptr->set_OSGsubPAT_nodemask(
         cessna_OSGsubPAT_number,0);

   } // sensormetadatapass_ID >= 0 conditional

// Instantiate Predator MODELSGROUP:

   MODELSGROUP* Predator_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,Flight_PolyLinePickHandler_ptr,
      AnimationController_ptr);

// As of 1/24/09, we assume that the zeroth Messenger for
// Predator_MODELSGROUP communicates on the robots channel.  The
// "GoogleEarth" messenger must therefore be pushed back onto the
// Messengers STL vector member of GraphicalsGroup AFTER the "robots"
// messenger:

   Predator_MODELSGROUP_ptr->pushback_Messenger_ptr(
      &Predator_MODELS_robots_messenger);
   Predator_MODELSGROUP_ptr->pushback_Messenger_ptr(
      &FlightLines_GoogleEarth_messenger);

   Predator_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Predator_MODELSGROUP_ptr->set_continuously_plan_UAV_paths_flag(
      passes_group.get_continuously_update_UAV_paths_flag());

   Predator_MODELSGROUP_ptr->set_fade_UAV_track_color_flag(true);
//   Predator_MODELSGROUP_ptr->set_fade_UAV_track_color_flag(false);

   MODELPickHandler* Predator_MODELPickHandler_ptr=
      decorations.get_last_MODELPickHandler_ptr();
   Predator_MODELPickHandler_ptr->set_PickHandlerCallbacks_ptr(
      BluegrassClient_ptr);

   movers_group* UAV_movers_group_ptr=Predator_MODELSGROUP_ptr->
      get_movers_group_ptr();
   Flight_PolyLinesGroup_ptr->set_movers_group_ptr(UAV_movers_group_ptr);
   
   UAV_movers_group_ptr->set_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));
//   root->addChild(centers_group.get_OSGgroup_ptr());

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of a user
// selected center location.  On 2/5/07, we learned (the painful and
// hard way!) that the order in which nodes are added to the
// SpinTransform is important for alpha-blending.  In particular, we
// must add decorations' OSGgroup AFTER adding clouds_group OSGgroup
// if alpha blending of 3D video imagery is to work...

   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      latlonggrids_group.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());

   root->addChild(centers_group.get_SpinTransform_ptr());
//   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// Instantiate OSGButtonServer which receives get calls from web page
// buttons that can be mapped onto ModeController state changes:

   string OSGButtonServer_hostname="127.0.0.1";
   int OSGButtonServer_portnumber=4041;
   if (OSGButtonServer_URL.size() > 0)
   {
      OSGButtonServer_hostname=stringfunc::get_hostname_from_URL(
         OSGButtonServer_URL);
      OSGButtonServer_portnumber=stringfunc::get_portnumber_from_URL(
         OSGButtonServer_URL);
   }
//   cout << "OSGButtonServer_hostname = " << OSGButtonServer_hostname
//        << " OSGButtonServer_portnumber = " << OSGButtonServer_portnumber
//        << endl;

   OSGButtonServer button_server(
      OSGButtonServer_hostname,OSGButtonServer_portnumber);
   button_server.set_WindowManager_ptr(window_mgr_ptr);
   button_server.set_EarthRegionsGroup_ptr(&earth_regions_group);
   button_server.set_ROILinePickHandler_ptr(ROILinePickHandler_ptr);
   button_server.set_KOZLinePickHandler_ptr(KOZLinePickHandler_ptr);
   button_server.set_BluegrassClient_ptr(BluegrassClient_ptr);
   button_server.set_SKSClient_ptr(SKSClient_ptr);
   button_server.set_pointfinder_ptr(&pointfinder);
   button_server.set_MoviesGroup_ptr(&movies_group);
   button_server.set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);
   button_server.set_Decorations_ptr(&decorations);
   button_server.set_Operations_ptr(&operations);
   button_server.set_Predator_MODELSGROUP_ptr(Predator_MODELSGROUP_ptr);
   button_server.set_ARs_PolyhedraGroup_ptr(ARs_PolyhedraGroup_ptr);
   button_server.set_n_ROI_states(passes_group.get_n_ROI_states());

// Instantiate Video Server to enable chip handling:

   VideoServer* video_server_ptr=NULL;
   if (VideoServer_URL.size() > 0)
   {
      EarthRegion* EarthRegion_ptr=earth_regions_group.
         get_ID_labeled_EarthRegion_ptr(1);
      Movie* Movie_ptr=EarthRegion_ptr->get_TextureSector_ptr()->
         get_Movie_ptrs().front();

      string VideoServer_hostname=stringfunc::get_hostname_from_URL(
         VideoServer_URL);
      int VideoServer_portnumber=stringfunc::get_portnumber_from_URL(
         VideoServer_URL);
//   cout << "VideoServer_hostname = " << VideoServer_hostname
//        << " VideoServer_portnumber = " << VideoServer_portnumber
//        << endl;

      video_server_ptr=new VideoServer(
         VideoServer_hostname,VideoServer_portnumber);
      video_server_ptr->set_Movie_ptr(Movie_ptr);
      video_server_ptr->set_CylindersGroup_ptr(CylindersGroup_ptr);
      video_server_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   }

// ========================================================================
// Bluegrass specific commands:

// Add Activity region bboxes into Lubbock point cloud...

   earth_regions_group.set_PolyhedraGroup_ptr(ARs_PolyhedraGroup_ptr);
   earth_regions_group.generate_annotation_bboxes(0);

// When demo starts up, we initially mask the Activity Regions'
// bounding boxes.  We can later toggle them on by pressing the 't'
// key from within Run Movie Mode:

   int AR_bboxes_subgroup_number=ARs_PolyhedraGroup_ptr->get_n_OSGsubPATs()-1;
   ARs_PolyhedraGroup_ptr->set_OSGsubPAT_nodemask(
      AR_bboxes_subgroup_number,0);
   decorations.get_PolyhedraKeyHandler_ptr()->
      set_OSGsubPAT_number_to_toggle(AR_bboxes_subgroup_number);
   button_server.set_ARs_PolyhedraGroup_OSGsubPAT_number_to_toggle(
      AR_bboxes_subgroup_number);

// Instantiate separate PolyLines group to hold roadway GIS network:

   PolyLinesGroup* RoadLinesGroup_ptr=
      earth_regions_group.generate_roadlines_group(0);
   if (postgis_db_ptr != NULL)
   {
      postgis_db_ptr->set_PolyLinesGroup_ptr(RoadLinesGroup_ptr);

// Activity Region #1 in UTM coords:

      double roads_xmin=224349.725385;
      double roads_xmax=229525.809632;
      double roads_ymin=3708458.10529;
      double roads_ymax=3713854.75201;

/*
// Greater Texas Tech region:

      double roads_xmin=230525;
      double roads_ymin=3717739;
      double roads_xmax=234488;
      double roads_ymax=3721830;
*/

      postgis_db_ptr->pushback_gis_bbox(
         roads_xmin,roads_xmax,roads_ymin,roads_ymax);
      postgis_db_ptr->set_altitude(0);
//      postgis_db_ptr->set_altitude(-30);	// Lubbock around Texas Tech
      postgis_db_ptr->set_PolyLine_text_size(9);

// AR1_streets table in bluegrass gis database

      string geom_name="s911name";

// Lubbock_streets table in bluegrass gis database:

//      string geom_name="name";		

      postgis_db_ptr->parse_table_contents(geom_name);
      postgis_db_ptr->popback_gis_bbox();
   }
   
   RoadLinesGroup_ptr->set_uniform_color(
      colorfunc::get_OSG_color(colorfunc::purple));
//      colorfunc::get_OSG_color(colorfunc::cyan));
   int OSGsubPAT_number=RoadLinesGroup_ptr->get_n_OSGsubPATs()-1;
   RoadLinesGroup_ptr->attach_bunched_geometries_to_OSGsubPAT(
      *grid_origin_ptr,OSGsubPAT_number);
   cout << "# Roadway PolyLines = " 
        << RoadLinesGroup_ptr->get_n_Graphicals() << endl;

// Retrieve vehicle ground truth from track server:

   double secs_offset=0;
//   cout << "Enter secs offset for all tracks read in from track server:"
//        << endl;
//   cin >> secs_offset;

// dataserver_IDs.size() = 1

/*
   tracks_group* groundtruth_tracks_ptr=NULL;

   for (int d=0; d<dataserver_IDs.size(); d++)
   {
      PassInfo* passinfo_ptr=
         passes_group.get_pass_ptr(dataserver_IDs[d])->get_PassInfo_ptr();
      string SKS_query=BluegrassClient_ptr->
         form_mover_tracks_query(passinfo_ptr);
      cout << "SKS_query = " << SKS_query << endl;

      BluegrassClient_ptr->query_BluegrassServer(SKS_query);
      string SKS_response=BluegrassClient_ptr->get_BluegrassServer_response();
//      cout << "SKS_response = " << SKS_response << endl;
      cout << "SKS_response.size() = " << SKS_response.size() << endl;

      double elapsed_secs_min=
         passinfo_ptr->get_elapsed_secs_since_epoch_lo();
      double elapsed_secs_max=
         passinfo_ptr->get_elapsed_secs_since_epoch_hi();
      earth_regions_group.set_t_start(elapsed_secs_min);
      earth_regions_group.set_t_stop(elapsed_secs_max);

      EarthRegion* curr_EarthRegion_ptr=earth_regions_group.
         get_ID_labeled_EarthRegion_ptr(d);
      groundtruth_tracks_ptr=curr_EarthRegion_ptr->
         get_dynamic_tracks_group_ptr();

//      ROILinesGroup_ptr->set_movers_group_ptr(
//         curr_EarthRegion_ptr->get_movers_group_ptr());
      CylindersGroup_ptr->set_movers_group_ptr(
         curr_EarthRegion_ptr->get_movers_group_ptr());

      bool associate_tracks_with_movers_flag;
      double cylinder_radius,cylinder_height,cylinder_alpha;
      if (earth_regions_group.get_specified_UTM_zonenumber()==38)  // Baghdad
      {

// In order to use Bluegrass truth tracks for Baghdad dynamic ground
// mover simulations, use following call to
// BluegrassClient::retrieve_mover_tracks():

         const int Lubbock_UTM_zonenumber=14;
         threevector old_origin_offset(227715,3711685); // AR1 center
         double track_rescaling_factor=5.0;
         threevector new_origin_offset(444500,3686279); // Baghdad center
         double tracks_altitude=grid_origin_ptr->get(2)+20;
         BluegrassClient_ptr->retrieve_mover_tracks(
            SKS_response,Lubbock_UTM_zonenumber,
            old_origin_offset,track_rescaling_factor,
            new_origin_offset,secs_offset,tracks_altitude,
            groundtruth_tracks_ptr);

// In order to maintain ground vehicle speeds, we need to rescale
// track time values by the same value as track ground positions:

//         cout << "Curr track rescaling factor = "
//              << track_rescaling_factor << endl;
//         cout << "Enter track rescaling factor:" << endl;
//         cin >> track_rescaling_factor;
         groundtruth_tracks_ptr->rescale_time_values_for_all_tracks(
            track_rescaling_factor);

         cylinder_radius=200;
         cylinder_height=1000;
         cylinder_alpha=1.0;
         associate_tracks_with_movers_flag=true;
      }
      else
      {

// For Bluegrass demo, use following call to retrieve SKS tracks:

         double tracks_altitude=1000;  // meters (appropriate for Lubbock, TX)
 				       // Grid_origin.z = 944.9 meters
         BluegrassClient_ptr->retrieve_mover_tracks(
            SKS_response,secs_offset,tracks_altitude,groundtruth_tracks_ptr);
         cylinder_radius=10;
         cylinder_height=70;
         cylinder_alpha=0.15;
         associate_tracks_with_movers_flag=false;
      }

//      curr_EarthRegion_ptr->display_tracks_as_PolyLines(
//         d,groundtruth_tracks_ptr,&pointfinder,TrackLinesGroup_ptr);
    
      earth_regions_group.initialize_track_mover_Cylinders(
         d,cylinder_radius,cylinder_height,cylinder_alpha,
         associate_tracks_with_movers_flag);
   } // loop over index d labeling dataserver queries

   if (video_server_ptr != NULL)
      video_server_ptr->set_tracks_group_ptr(groundtruth_tracks_ptr);
*/

// ========================================================================

   timefunc::initialize_timeofday_clock();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      app.processEvents();
   }
   delete window_mgr_ptr;

   delete video_server_ptr;

   delete postgis_databases_group_ptr;
}



