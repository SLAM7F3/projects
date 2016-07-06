// ========================================================================
// Program PLANNER is a stripped-down variant of QTCITIES intended to
// eventually run as a webservice within a netcentric architecture.
// ========================================================================
// Last updated on 3/5/09; 3/6/09; 5/29/09; 6/27/09; 12/31/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>

#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "track/tracks_group.h"
#include "osg/osgWindow/ViewerManager.h"

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

// Read input data files:
   
   const int ndims=3;
   PassesGroup passes_group(&arguments);
//   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

   string cloud_filename="empty.dat";
   int cloudpass_ID=0;
   passes_group.generate_new_pass(cloud_filename,cloudpass_ID);
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   cout << "n_passes = " << passes_group.get_n_passes() << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

//   string broker_URL=passes_group.get_pass_ptr(cloudpass_ID)->
//      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="PLANNER";

// Instantiate robot messengers for communication with Luca
// Bertucelli's MATLAB UAV task assignment codes:

   string robots_message_queue_channel_name="robots";
   Messenger EarthRegions_robots_messenger( 
      broker_URL,robots_message_queue_channel_name,message_sender_ID);

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

   bool display_movie_state=false; 
   bool display_movie_number=false;
   bool display_movie_world_time=false;
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
//   cout << "CM_3D_ptr = " << CM_3D_ptr << endl;

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),&latlonggrids_group);

   earth_regions_group.set_PassesGroup_ptr(&passes_group);
   earth_regions_group.set_AnimationController_ptr(AnimationController_ptr);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_robots_messenger);
   earth_regions_group.pushback_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);
   earth_regions_group.assign_EarthRegionsGroup_Messenger_ptrs();
   earth_regions_group.generate_regions(passes_group);

   root->addChild( earth_regions_group.get_OSGgroup_ptr() );

   LatLongGrid* latlonggrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   threevector* grid_origin_ptr=latlonggrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

   CM_3D_ptr->set_Grid_ptr(latlonggrid_ptr);

   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate separate PolyLines group to hold ROI skeletons:

   RegionPolyLinesGroup* ROILinesGroup_ptr=
      earth_regions_group.generate_ROIlines_group(0);
   ROILinesGroup_ptr->set_ROI_PolyLinesGroup_flag(true);
   ROILinesGroup_ptr->set_width(passes_group.get_line_width());
   ROILinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

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

// Instantiate PolyLinesGroup to hold Predator flight paths.  Interact
// with these Polylines in INSERT[MANIPULATE]_POLYLINE mode rather
// than INSERT[MANIPULATE]_LINE mode:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(4);

// Instantiate Predator MODELSGROUP:

   MODELSGROUP* Predator_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,
      NULL,AnimationController_ptr);

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
   Predator_MODELSGROUP_ptr->set_long_initial_UAV_track_flag(true);

//   Predator_MODELSGROUP_ptr->set_fade_UAV_track_color_flag(true);
   Predator_MODELSGROUP_ptr->set_fade_UAV_track_color_flag(false);

   movers_group* UAV_movers_group_ptr=Predator_MODELSGROUP_ptr->
      get_movers_group_ptr();
   Flight_PolyLinesGroup_ptr->set_movers_group_ptr(UAV_movers_group_ptr);
   
   UAV_movers_group_ptr->set_Messenger_ptr(
      &EarthRegions_GoogleEarth_messenger);

//   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// ========================================================================

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      app.processEvents();
   }
   delete window_mgr_ptr;

}



