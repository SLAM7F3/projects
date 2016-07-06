// ========================================================================
// Program EOCITES is a variant of program TESTCITIES specialized for
// the Real-time persistent EO surveillance project.
// ========================================================================
// Last updated on 12/21/09; 5/11/10; 5/12/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
// #include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"
#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "osg/osgEarth/TextureSectorsGroup.h"
#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
//   int texturepass_ID=passes_group.get_curr_texturepass_ID();
//   cout << "texturepass_ID = " << texturepass_ID << endl;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

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

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   int pass_ID=passes_group.get_n_passes()-1;
//   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
//      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="EOCITIES";

   bool include_sender_and_timestamp_info_flag=false;

   string aircraft_message_queue_channel_name="aircraft";
   Messenger FlightLines_aircraft_messenger( 
      broker_URL,aircraft_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   Messenger EarthRegions_aircraft_messenger( 
      broker_URL,aircraft_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string ROIs_message_queue_channel_name="ROIs";
   Messenger ROIs_messenger( 
      broker_URL,ROIs_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);
   
   string cars_message_queue_channel_name="cars";
   Messenger cars_messenger( 
      broker_URL,cars_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string NFOV_lookpoint_message_queue_channel_name="current_NFOV_lookpoint";
   Messenger NFOV_lookpoint_messenger( 
      broker_URL,NFOV_lookpoint_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string aimpoint_message_queue_channel_name="NFOV_aimpoint";
   Messenger aimpoint_messenger( 
      broker_URL,aimpoint_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=false;
   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool hide_Mode_HUD_flag=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Instantiate AnimationController which acts as master game clock:

   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setState(AnimationController::PLAY);

// Instantiate clock pointer to keep track of real time:

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->set_initial_truetime();

// Set master clock's stopping time:

   clock_ptr->current_local_time_and_UTC();
   const int n_simulation_mins=0;
//   const int n_simulation_mins=15;
//   const int n_simulation_mins=46;
//   const int n_simulation_hours=0;	// hours
//   const int n_simulation_hours=1;	// hours
   const int n_simulation_hours=2;	// hours
//   const int n_simulation_hours=3;	// hours
//   const int n_simulation_hours=5;	// hours
   clock_ptr->set_UTC_time(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day(),
      clock_ptr->get_UTC_hour()+n_simulation_hours,
      clock_ptr->get_minute()+n_simulation_mins,clock_ptr->get_seconds());

   bool start_time_flag=false;
   operations.set_master_world_UTC(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day(),
      clock_ptr->get_UTC_hour(),clock_ptr->get_minute(),
      clock_ptr->get_seconds(),start_time_flag);

// Set master clock's starting time:

   clock_ptr->current_local_time_and_UTC();
   start_time_flag=true;
   operations.set_master_world_UTC(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day(),
      clock_ptr->get_UTC_hour(),clock_ptr->get_minute(),
      clock_ptr->get_seconds(),start_time_flag);

   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());

   AnimationController_ptr->set_world_time_params(
      operations.get_master_world_start_time(),
      operations.get_master_world_stop_time(),
      operations.get_delta_master_world_time_step_per_master_frame());
   AnimationController_ptr->setDelay(
      operations.get_delta_master_world_time_step_per_master_frame());
   AnimationController_ptr->set_increment_time_rather_than_frame_number_flag(
      true);

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr);
   movies_group.toggle_OSGgroup_nodemask();

   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(true);
   clouds_group.set_point_transition_altitude_factor(1000.0/400.0);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group,&movies_group);

   earth_regions_group.set_AnimationController_ptr(AnimationController_ptr);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_aircraft_messenger);
   earth_regions_group.assign_EarthRegionsGroup_Messenger_ptrs();
   earth_regions_group.generate_regions(passes_group);
   root->addChild( earth_regions_group.get_OSGgroup_ptr() );

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   LatLongGrid_ptr->set_curr_color(
      colorfunc::get_OSG_color(colorfunc::brightpurple));
   LatLongGrid_ptr->update_grid_text_color();

   LatLongGrid_ptr->set_depth_buffering_off_flag(true);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
//   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   if (postgis_db_ptr != NULL)
      postgis_db_ptr->pushback_gis_bbox(xmin,xmax,ymin,ymax);

/*
// Instantiate SurfaceTexture to hold jpeg images:

   string movie_filename="lubbock.jpg";
   vector<threevector> video_corner_vertices;
   video_corner_vertices.push_back(threevector(410195,4750296,300));
   video_corner_vertices.push_back(threevector(418359,4750195,300));
   video_corner_vertices.push_back(threevector(418491,4761299,300));
   video_corner_vertices.push_back(threevector(410340,4761401,300));
   earth_regions_group.generate_SurfaceTexture_EarthRegion(
      movie_filename,video_corner_vertices);
   Movie* Movie_ptr=earth_regions_group.generate_EarthRegion_video_chip(1);
*/

// Instantiate separate PolyLines group to hold Special Interest Area
// (SIA) skeletons:

   RegionPolyLinesGroup* SIALinesGroup_ptr=
      earth_regions_group.generate_ROIlines_group(0);
   SIALinesGroup_ptr->set_ROI_PolyLinesGroup_flag(true);
//   SIALinesGroup_ptr->set_width(4);	// for ISDS laptop
//   SIALinesGroup_ptr->set_width(6);	// for ppt presentations
   SIALinesGroup_ptr->set_width(passes_group.get_line_width());
   SIALinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
//   SIALinesGroup_ptr->pushback_Messenger_ptr(
//      &SIALines_urban_network_messenger);

   EarthRegion* EarthRegion_ptr=earth_regions_group.
      get_ID_labeled_EarthRegion_ptr(0);
   SIALinesGroup_ptr->set_movers_group_ptr(
      EarthRegion_ptr->get_movers_group_ptr());

   double SIA_skeleton_height=
      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
      get_ROI_skeleton_height();
   if (SIA_skeleton_height > 0)
   {
      SIALinesGroup_ptr->set_skeleton_height(SIA_skeleton_height);
   }

   RegionPolyLinePickHandler* SIALinePickHandler_ptr=
      new RegionPolyLinePickHandler(
         passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr,
         SIALinesGroup_ptr,ModeController_ptr,window_mgr_ptr,grid_origin_ptr);
   SIALinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   SIALinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   SIALinePickHandler_ptr->set_z_offset(5);
   SIALinePickHandler_ptr->set_min_points_picked(3);
   SIALinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
   SIALinePickHandler_ptr->set_surface_picking_flag(false);
   SIALinePickHandler_ptr->set_Zplane_picking_flag(false);
   SIALinePickHandler_ptr->set_Allow_Manipulation_flag(false);
   SIALinePickHandler_ptr->set_label_prefix("SIA");

// Specify color for SIA polyhedron skeleton:

   colorfunc::Color SIA_color=colorfunc::white;
   string SIA_skeleton_color=
      passes_group.get_pass_ptr(cloudpass_ID)->get_PassInfo_ptr()->
      get_ROI_skeleton_color();

   if (SIA_skeleton_color.size() > 0)
   {
      SIA_color=colorfunc::string_to_color(SIA_skeleton_color);
   }
   earth_regions_group.set_ROI_color(SIA_color);
   SIALinePickHandler_ptr->set_permanent_color(SIA_color);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      SIALinePickHandler_ptr);

// Instantiate PolyLinesGroup to hold aircraft flight paths.  Interact
// with these Polylines in INSERT[MANIPULATE]_POLYLINE mode rather
// than INSERT[MANIPULATE]_LINE mode:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(4);

// Instantiate Aircraft MODELSGROUP:

   MODELSGROUP* aircraft_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,NULL,AnimationController_ptr);
   aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::cessna);
   aircraft_MODELSGROUP_ptr->set_compute_passed_ground_targets_flag(false);
   aircraft_MODELSGROUP_ptr->set_n_future_repeats(5);

   aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(
      &FlightLines_aircraft_messenger);
   aircraft_MODELSGROUP_ptr->pushback_Messenger_ptr(
      &NFOV_lookpoint_messenger);
   aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   aircraft_MODELSGROUP_ptr->set_fade_UAV_track_color_flag(false);

/*
   movers_group* UAV_movers_group_ptr=aircraft_MODELSGROUP_ptr->
      get_movers_group_ptr();
   Flight_PolyLinesGroup_ptr->set_movers_group_ptr(UAV_movers_group_ptr);
   UAV_movers_group_ptr->set_Messenger_ptr(
      &EarthRegions_aircraft_messenger);
*/

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   if (!passes_group.get_pick_points_on_Zplane_flag())
   {
      CM_3D_ptr->set_PointFinder(&pointfinder);
   }
   
// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
//      clouds_group.get_SetupGeomVisitor_ptr(),
//      clouds_group.get_ColorGeodeVisitor_ptr());
   
// Instantiate Polyhedra decoration group:

   ROI_PolyhedraGroup* ROI_PolyhedraGroup_ptr=
      decorations.add_ROI_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   ROI_PolyhedraGroup_ptr->set_bbox_sidelength(500);	// meters
   ROI_PolyhedraGroup_ptr->set_bbox_height(100);	// meters
   ROI_PolyhedraGroup_ptr->set_bbox_color_str("orange");	
   ROI_PolyhedraGroup_ptr->set_bbox_label_color_str("yellow");	
   ROI_PolyhedraGroup_ptr->pushback_Messenger_ptr(&ROIs_messenger);

/*
// Instantiate SignPosts decoration group:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->set_postgis_databases_group_ptr(
      postgis_databases_group_ptr);

   for (int n=0; n<decorations.get_n_SignPostsGroups(); n++)
   {
      decorations.get_SignPostsGroup_ptr(n)->set_EarthRegionsGroup_ptr(
         &earth_regions_group);
      decorations.get_SignPostsGroup_ptr(n)->set_Clock_ptr(clock_ptr);
   }
*/

// Instantiate NFOV camera aimpoint SignPosts group:

   SignPostsGroup* NFOV_aimpoint_SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(
                                   cloudpass_ID));
   NFOV_aimpoint_SignPostsGroup_ptr->set_size(50);
   colorfunc::Color permanent_color=colorfunc::cyan;
   colorfunc::Color selected_color=colorfunc::cyan;
   NFOV_aimpoint_SignPostsGroup_ptr->set_colors(
      permanent_color,selected_color);

   int n_aimpoints=10;
   for (int n=0; n<n_aimpoints; n++)
   {
      NFOV_aimpoint_SignPostsGroup_ptr->set_fixed_label_to_SignPost_ID(
         n,"Aimpt "+stringfunc::number_to_string(n));
   }
   NFOV_aimpoint_SignPostsGroup_ptr->pushback_Messenger_ptr(
      &aimpoint_messenger);

// Instantiate Features decoration group:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate Cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=
      decorations.add_Cylinders(
         passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   CylindersGroup_ptr->set_rh(4,300);
   CylindersGroup_ptr->pushback_Messenger_ptr(&cars_messenger);
//   CylindersGroup_ptr->generate_RTPS_Cylinders(50);

   CylinderPickHandler* CylinderPickHandler_ptr=
      decorations.get_CylinderPickHandler_ptr();
   CylinderPickHandler_ptr->set_text_size(8);
   CylinderPickHandler_ptr->set_text_screen_axis_alignment_flag(false);

/*
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
//   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
//      SignPostsGroup_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      ROI_PolyhedraGroup_ptr);
*/
  
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

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      if (AnimationController_ptr->getState()==AnimationController::PLAY)
      {
         if (AnimationController_ptr->time_to_display_new_frame())
         {
            AnimationController_ptr->increment_time();
         }
      }
   }
   delete window_mgr_ptr;

   delete postgis_databases_group_ptr;
}



