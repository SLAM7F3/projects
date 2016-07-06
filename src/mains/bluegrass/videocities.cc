// ========================================================================
// Program VIDEOCITIES is a variant of program TESTCITIES which
// implements video surface textures & dataserver track queries.

//	 videocities --region_filename ./packages/lubbock_fused.pkg --surface_texture ./packages/ar1_texture.pkg

// ========================================================================
// Last updated on 5/13/09; 5/11/10; 5/12/10; 12/31/10
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
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osg2D/ClassificationHUD.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "filter/piecewise_linear_vector.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "math/rpy.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "passes/TextDialogBox.h"
#include "time/timefuncs.h"
#include "track/tracks_group.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

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
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   cout << "texturepass_ID = " << texturepass_ID << endl;
   int sensormetadatapass_ID=passes_group.get_curr_sensormetadatapass_ID();
   cout << "sensor_metadata_pass_ID = " << sensormetadatapass_ID << endl;

//   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
   vector<int> dataserver_IDs=passes_group.get_dataserver_IDs();
//   int dataserverpass_ID=-1;
//   if (dataserver_IDs.size() > 0)
//   {
//      dataserverpass_ID=dataserver_IDs[0];
//   }

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
   
// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
//   window_mgr_ptr->getUsage(*arguments.getApplicationUsage());

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;

   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
//   string broker_URL = "tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string message_sender_ID="VIDEOCITIES";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string urban_network_message_queue_channel_name="urban_network";
   Messenger EarthRegions_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);
   Messenger TrackLines_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);
   Messenger ROILines_urban_network_messenger( 
      broker_URL,urban_network_message_queue_channel_name,message_sender_ID);

// Instantiate robot messengers for communication with Luca
// Bertucelli's MATLAB UAV task assignment codes:

   string robots_message_queue_channel_name="robots";
   Messenger EarthRegions_robots_messenger( 
      broker_URL,robots_message_queue_channel_name,message_sender_ID);
   Messenger Predator_MODELS_robots_messenger( 
      broker_URL,robots_message_queue_channel_name,message_sender_ID);

// Instantiate GoogleEarth messengers for communication with Tim
// Schreiner's ROI selection tool:

   string GE_message_queue_channel_name="GoogleEarth";
   Messenger EarthRegions_GoogleEarth_messenger( 
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

   bool generate_AVI_movie_flag=false;
//   bool generate_AVI_movie_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      generate_AVI_movie_flag);
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

// Specify start, stop and step times for master game clock:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());

   AnimationController_ptr->set_world_time_params(
      operations.get_master_world_start_time(),
      operations.get_master_world_stop_time(),
      operations.get_delta_master_world_time_step_per_master_frame());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
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
   earth_regions_group.set_PassesGroup_ptr(&passes_group);
   earth_regions_group.set_AnimationController_ptr(AnimationController_ptr);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_urban_network_messenger);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_robots_messenger);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);
   earth_regions_group.assign_EarthRegionsGroup_Messenger_ptrs();

   earth_regions_group.generate_regions(passes_group);
   root->addChild( earth_regions_group.get_OSGgroup_ptr() );

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   decorations.set_PointCloudsGroup_ptr(&clouds_group);

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,latlonggrid_ptr));

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
//   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
//      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

// Instantiate PolyLinesGroup to hold ground truth vehicle tracks:   

   PolyLinesGroup* TrackLinesGroup_ptr=
      decorations.add_PolyLines(
         ndims,passes_group.get_pass_ptr(cloudpass_ID));
//   TrackLinesGroup_ptr->set_width(2);	// for touch table projector
//   TrackLinesGroup_ptr->set_width(4);	// for ISDS3D laptop
//   TrackLinesGroup_ptr->set_width(6);	// for ppt presentations
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

   PolyLinesGroup* ROILinesGroup_ptr=
      earth_regions_group.generate_ROIlines_group(0);
//   ROILinesGroup_ptr->set_width(4);	// for ISDS laptop
//   ROILinesGroup_ptr->set_width(6);	// for ppt presentations
   ROILinesGroup_ptr->set_width(passes_group.get_line_width());
   ROILinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   ROILinesGroup_ptr->pushback_Messenger_ptr(
      &ROILines_urban_network_messenger);

   PolyLinePickHandler* ROILinePickHandler_ptr=new PolyLinePickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr,
      ROILinesGroup_ptr,ModeController_ptr,window_mgr_ptr,grid_origin_ptr);
   ROILinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   ROILinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   ROILinePickHandler_ptr->set_z_offset(5);
   ROILinePickHandler_ptr->set_min_points_picked(3);
   ROILinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
   ROILinePickHandler_ptr->set_surface_picking_flag(false);
   ROILinePickHandler_ptr->set_Zplane_picking_flag(false);
   ROILinePickHandler_ptr->set_Allow_Manipulation_flag(false);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      ROILinePickHandler_ptr);
 
// Instantiate PolyLinesGroup to hold Predator flight paths.  Interact
// with these Polylines in INSERT[MANIPULATE]_POLYLINE mode rather
// than INSERT[MANIPULATE]_LINE mode:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinePickHandler* Flight_PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr(1);
   Flight_PolyLinePickHandler_ptr->set_regular_vertex_spacing(100); // meters

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
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(&clock);
   }

// Display Locations of Interest as orange SignPosts:

   colorfunc::Color signposts_color=colorfunc::orange;
//   double SignPost_size=50;
//   double SignPost_size=5;
//   double SignPost_size=1;
   double SignPost_size=0.25;
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
//   CylindersGroup_ptr->pushback_Messenger_ptr(&urban_network_messenger);
   earth_regions_group.set_CylindersGroup_ptr(CylindersGroup_ptr);

   CylinderPickHandler* CylinderPickHandler_ptr=
      decorations.get_CylinderPickHandler_ptr();
   CylinderPickHandler_ptr->set_text_size(8);
   CylinderPickHandler_ptr->set_text_screen_axis_alignment_flag(false);

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_z_ColorMap_ptr(
      clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr());
   OBSFRUSTAGROUP_ptr->set_ImageNumberHUD_ptr(
      operations.get_ImageNumberHUD_ptr());      
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_Grid_ptr(
      latlonggrid_ptr);
   decorations.get_OBSFRUSTUMPickHandler_ptr()->set_Grid_ptr(
      latlonggrid_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      ARs_PolyhedraGroup_ptr);

   bool multicolor_frusta_flag=false;
//   bool initially_mask_all_frusta_flag=true;
   bool initially_mask_all_frusta_flag=false;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,multicolor_frusta_flag,initially_mask_all_frusta_flag);

// Instantiate PowerPoints decoration group:

//   PowerPointsGroup* PowerPointsGroup_ptr=
//      decorations.add_PowerPoints(passes_group.get_pass_ptr(cloudpass_ID));
//   PowerPointsGroup_ptr->pushback_Messenger_ptr(&ppt_messenger);
 
// Instantiate model decorations group. Then generate Cessna model,
// racetrack orbit and OBSFRUSTUM:

   if (sensormetadatapass_ID >= 0)
   {
      MODELSGROUP* MODELSGROUP_ptr=decorations.add_MODELS(
         passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);

      int cessna_OSGsubPAT_number;
      MODEL* cessna_ptr=MODELSGROUP_ptr->generate_Cessna_MODEL(
         cessna_OSGsubPAT_number);
      cessna_ptr->set_dynamically_compute_OBSFRUSTUM_flag(true);
      cessna_ptr->push_back_OBSFRUSTUM_pitch(-70*PI/180);
      cessna_ptr->push_back_OBSFRUSTUM_yaw(90*PI/180);
//      cessna_ptr->set_OBSFRUSTUM_z_base_face(grid_origin_ptr->get(2)+35); 
       // AR #1
      cessna_ptr->push_back_OBSFRUSTUM_z_base_face(
         grid_origin_ptr->get(2)+71);
      // Full Lubbock 3D map

// Initially mask Cessna OBSFRUSTUM:

      MODELSGROUP_ptr->set_OSGsubPAT_nodemask(cessna_OSGsubPAT_number,0);
//      MODELSGROUP_ptr->set_OSGsubPAT_nodemask(cessna_OSGsubPAT_number,1);

      vector<Triple<threevector,rpy,int> > sensor_posn_orientation_frames=
         passes_group.get_pass_ptr(sensormetadatapass_ID)->
         get_PassInfo_ptr()->get_posn_orientation_frames();

      vector<threevector> posn_reg,orientation_reg;
      MODELSGROUP_ptr->regularize_aircraft_posns_and_orientations(
         sensor_posn_orientation_frames,movie_anim_controller_ptr,
         posn_reg,orientation_reg);
      MODELSGROUP_ptr->generate_racetrack_orbit(posn_reg,cessna_ptr);

      double FOV_az_extent=85*PI/180;  	// rads
      double FOV_el_extent=75*PI/180;	// degs
      OBSFRUSTUM* cessna_FOV_OBSFRUSTUM_ptr=
         MODELSGROUP_ptr->instantiate_MODEL_FOV_OBSFRUSTUM(
            cessna_OSGsubPAT_number,cessna_ptr,FOV_az_extent,FOV_el_extent);
   } // sensormetadatapass_ID >= 0 conditional

// Instantiate Predator MODELSGROUP:

   MODELSGROUP* Predator_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,Flight_PolyLinePickHandler_ptr,
      AnimationController_ptr);
   Predator_MODELSGROUP_ptr->pushback_Messenger_ptr(
      &Predator_MODELS_robots_messenger);
   Predator_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);

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

// Open text dialog box to display feature information:

//   decorations.get_FeaturesGroup_ptr()->get_TextDialogBox_ptr()->
//      open("Feature Information");
//   decorations.get_FeaturesGroup_ptr()->update_feature_text();

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

//   timefunc::initialize_timeofday_clock();
//   osg::FrameStamp* FrameStamp_ptr=window_mgr_ptr->getFrameStamp();

//   cout << "Before entering infinite viewer loop" << endl;
//   outputfunc::enter_continue_char();

// ========================================================================
// Bahgdad specific commands:

// Temporary hack as of 5/3/07: Hardwire min/max height thresholds for
// entire Baghdad ladar map:

//   clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr()->
//      set_min_threshold(-71.5953);
//   clouds_group.get_Cloud_ptr(0)->get_z_ColorMap_ptr()->
//      set_max_threshold(63.9469);

// ========================================================================
// NYC specific commands:

//   SignPostsGroup_ptr->set_extra_textmessage_info(" New York City ");
//   OBSFRUSTAGROUP_ptr->generate_skyscraper_bboxes(PolyhedraGroup_ptr);

// ========================================================================
// Bluegrass specific commands:

// Add Activity region bboxes into Lubbock point cloud...
      
   earth_regions_group.set_PolyhedraGroup_ptr(ARs_PolyhedraGroup_ptr);
   earth_regions_group.generate_annotation_bboxes(0);

// When demo starts up, we initially mask the Activity Regions'
// bounding boxes.  We can later toggle them on by pressing the
// '3/page down' key on the RHS keypad:

   int AR_bboxes_subgroup_number=ARs_PolyhedraGroup_ptr->get_n_OSGsubPATs()-1;
   ARs_PolyhedraGroup_ptr->set_OSGsubPAT_nodemask(
      AR_bboxes_subgroup_number,0);
   decorations.get_PolyhedraKeyHandler_ptr()->
      set_OSGsubPAT_number_to_toggle(AR_bboxes_subgroup_number);

/*
// Retrieve vehicle ground truth from track server:

   double secs_offset=0;
//   double secs_offset=15;
//   cout << "Enter secs offset for all tracks read in from track server:"
//        << endl;
//   cin >> secs_offset;

   for (int d=0; d<dataserver_IDs.size(); d++)
   {
      PassInfo* passinfo_ptr=
         passes_group.get_pass_ptr(dataserver_IDs[d])->get_PassInfo_ptr();
      EarthRegion* curr_EarthRegion_ptr=earth_regions_group.
         get_ID_labeled_EarthRegion_ptr(d);
      tracks_group* groundtruth_tracks_ptr=
         curr_EarthRegion_ptr->get_dynamic_tracks_group_ptr();

      ROILinesGroup_ptr->set_movers_group_ptr(
         curr_EarthRegion_ptr->get_movers_group_ptr());

//      earth_regions_group.retrieve_mover_tracks(
//         passinfo_ptr,secs_offset);
//      earth_regions_group.display_tracks_as_PolyLines(
//         0,pointfinder,Flight_PolyLinesGroup_ptr);
//      earth_regions_group.initialize_track_mover_Cylinders(0);

   } // loop over index d labeling dataserver queries
*/

// ========================================================================

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

   delete postgis_databases_group_ptr;
}
