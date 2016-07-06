// ========================================================================
// Program QTEOCITES is a variant of program EOCITIES specialized for
// the Real-time persistent EO surveillance project.
// ========================================================================
// Last updated on 12/21/09; 5/11/10; 5/12/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "osg/osgGraphicals/AnimationController.h"
#include "Qt/web/RTPSButtonServer.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "Qt/web/ImageServer.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"
#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "Qt/rtps/MessageWrapper.h"
#include "Qt/rtps/RTPSMessenger.h"

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

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

   string ImageServer_URL=passes_group.get_LogicServer_URL();
//   cout << "ImageServer_URL = " << ImageServer_URL << endl;
   string ImageServer_hostname=stringfunc::get_hostname_from_URL(
      ImageServer_URL);
   int ImageServer_portnumber=stringfunc::get_portnumber_from_URL(
      ImageServer_URL);
//   cout << "ImageServer_hostname = " << ImageServer_hostname
//        << " ImageServer_portnumber = " << ImageServer_portnumber
//        << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->set_default_Window_Y_origin(800); 
     // Y origin value appropriate for 30" monitor & laptop screen
   window_mgr_ptr->initialize_window("3D Viewer");

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   int pass_ID=passes_group.get_n_passes()-1;
//   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
//      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

// As of 11/30/09, Alex's code on the communications machine is
// expecting the message queue channel name to be
// ANALYSIS_TO_COMMUNICATION for messages originating from Peter's
// analysis machine:

   string message_sender_ID="PETER_ANALYSIS_MACHINE";
   string message_queue_channel_name="ANALYSIS_TO_COMMUNICATION";
   bool include_sender_and_timestamp_info_flag=false;

//   RTPSMessenger ROI_COMMAND_messenger(
//      broker_URL, message_queue_channel_name, message_sender_ID, 
//      include_sender_and_timestamp_info_flag);

/*
//   string ROIs_message_queue_channel_name="ROIs";
   Messenger ROIs_messenger( 
      broker_URL,ROIs_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);
*/

   string aircraft_message_queue_channel_name="aircraft";
   Messenger FlightLines_aircraft_messenger( 
      broker_URL,aircraft_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);
   Messenger EarthRegions_aircraft_messenger( 
      broker_URL,aircraft_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string cars_message_queue_channel_name="cars";
   Messenger cars_messenger( 
      broker_URL,cars_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string blueforce_car_message_queue_channel_name="blueforce_car";
   Messenger blueforce_car_transmitter_messenger( 
      broker_URL,blueforce_car_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);
   Messenger blueforce_car_receiver_messenger( 
      broker_URL,blueforce_car_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string NFOV_lookpoint_message_queue_channel_name="current_NFOV_lookpoint";
   Messenger NFOV_lookpoint_messenger( 
      broker_URL,NFOV_lookpoint_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

   string aimpoint_message_queue_channel_name="NFOV_aimpoint";
   Messenger aimpoint_messenger( 
      broker_URL,aimpoint_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

//   RTPSMessenger NFOV_COMMAND_publisher(
//      broker_URL, message_queue_channel_name, message_sender_ID, 
//      include_sender_and_timestamp_info_flag);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=false;
   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
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
   AnimationController_ptr->set_increment_time_rather_than_frame_number_flag(
      true);

// Set master game clock's starting time to current UTC, simulation's
// duration and time step:

   double n_simulation_hours=2;     
   operations.set_current_master_clock_time_duration_and_step(
      n_simulation_hours,passes_group.get_world_time_step());
   AnimationController_ptr->set_frame_counter_offset(
      -AnimationController_ptr->get_frame_corresponding_to_elapsed_secs());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

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
      &clouds_group,&latlonggrids_group);
   earth_regions_group.set_AnimationController_ptr(AnimationController_ptr);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_aircraft_messenger);
   earth_regions_group.pushback_Messenger_ptr(
      &blueforce_car_receiver_messenger);
   earth_regions_group.assign_EarthRegionsGroup_Messenger_ptrs();

   earth_regions_group.generate_regions(passes_group);
   earth_regions_group.set_propagate_all_tracks_flag(false);
   earth_regions_group.set_check_Cylinder_ROI_intersections_flag(true);
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

// Instantiate separate PolyLines group to hold Special Interest Area
// (SIA) skeletons:

   RegionPolyLinesGroup* SIALinesGroup_ptr=
      earth_regions_group.generate_ROIlines_group(0);
   SIALinesGroup_ptr->set_ROI_PolyLinesGroup_flag(true);
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

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   
// Instantiate ROI Polyhedra decoration group:

   ROI_PolyhedraGroup* ROI_PolyhedraGroup_ptr=
      decorations.add_ROI_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   ROI_PolyhedraGroup_ptr->set_bbox_sidelength(500);	// meters
//   ROI_PolyhedraGroup_ptr->set_bbox_sidelength(10*1000);	// meters
   ROI_PolyhedraGroup_ptr->set_bbox_height(100);	// meters
   ROI_PolyhedraGroup_ptr->set_bbox_color_str("orange");	
   ROI_PolyhedraGroup_ptr->set_bbox_label_color_str("yellow");	
   ROI_PolyhedraGroup_ptr->set_altitude_dependent_volume_alphas_flag(true);
   ROI_PolyhedraGroup_ptr->set_min_alpha_altitude(3*1000);
   ROI_PolyhedraGroup_ptr->set_max_alpha_altitude(10*1000);
   ROI_PolyhedraGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
//   ROI_PolyhedraGroup_ptr->pushback_RTPSMessenger_ptr(&ROI_COMMAND_messenger);
   earth_regions_group.set_PolyhedraGroup_ptr(ROI_PolyhedraGroup_ptr);

// Instantiate NFOV camera aimpoint SignPosts group:

   SignPostsGroup* NFOV_aimpoint_SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(
                                   cloudpass_ID));
   NFOV_aimpoint_SignPostsGroup_ptr->set_size(10);
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

   NFOV_aimpoint_SignPostsGroup_ptr->set_broadcast_NFOV_aimpoint_flag(true);
   NFOV_aimpoint_SignPostsGroup_ptr->pushback_Messenger_ptr(
      &aimpoint_messenger);

// Instantiate Features decoration group:

   decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate Cylinders decoration group:

   CylindersGroup* CylindersGroup_ptr=decorations.add_Cylinders(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   CylindersGroup_ptr->set_rh(4,100);
   CylindersGroup_ptr->pushback_Messenger_ptr(&cars_messenger);
   earth_regions_group.set_CylindersGroup_ptr(CylindersGroup_ptr);

   bool associate_tracks_with_movers_flag=true;
   earth_regions_group.initialize_RTPS_mover_Cylinders(
      5,0,associate_tracks_with_movers_flag);
//   earth_regions_group.initialize_RTPS_mover_Cylinders(
//      50,0,associate_tracks_with_movers_flag);
   earth_regions_group.initialize_RTPS_mover_Cylinders(
      1,10000,associate_tracks_with_movers_flag);

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
      LatLongGrid_ptr);
   decorations.get_OBSFRUSTUMPickHandler_ptr()->set_Grid_ptr(
      LatLongGrid_ptr);
//   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
//      SignPostsGroup_ptr);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_PolyhedraGroup_ptr(
      ROI_PolyhedraGroup_ptr);

   double Zplane_altitude=20;	// meters (appropriate for NYC demo)
   bool multicolor_frusta_flag=false;
   bool initially_mask_all_frusta_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_from_projection_matrices(
      passes_group,Zplane_altitude,
      multicolor_frusta_flag,initially_mask_all_frusta_flag);
    
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

// Instantiate RTPSButtonServer which receives get calls from web page
// buttons that can be mapped onto ModeController state changes:

   string RTPSButtonServer_hostname="127.0.0.1";
   int RTPSButtonServer_portnumber=4041;
   RTPSButtonServer button_server(
      RTPSButtonServer_hostname,RTPSButtonServer_portnumber);
   button_server.set_Decorations_ptr(&decorations);
   button_server.set_Operations_ptr(&operations);
   button_server.set_ROI_PolyhedraGroup_ptr(ROI_PolyhedraGroup_ptr);
   button_server.set_SIALinePickHandler_ptr(SIALinePickHandler_ptr);
   button_server.set_EarthRegionsGroup_ptr(&earth_regions_group);
   button_server.set_NFOV_aimpoint_SignPostsGroup_ptr(
      NFOV_aimpoint_SignPostsGroup_ptr);

// Instantiate Imageserver:

   ImageServer* ImageServer_ptr=new ImageServer(
      ImageServer_hostname,ImageServer_portnumber);
   ImageServer_ptr->pushback_Messenger_ptr(
      &blueforce_car_transmitter_messenger);

   bool iPhone_beacon_flag=true;
   ImageServer_ptr->set_iPhone_beacon_flag(iPhone_beacon_flag);

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      app.processEvents();
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



