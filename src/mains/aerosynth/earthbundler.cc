// ========================================================================
// Program EARTHBUNDLER backprojects reconstructed aerial view frusta
// images onto a ground Z-plane.  The geometrically stabilized
// backprojections are played back as the view frusta follow
// the aircraft's flight path over time.  A Cessna model follows the
// air vehicle's track.  A background static satellite image serves as
// an absolute map against which the dynamic foreground
// backprojections can be intuitively compared.
// ========================================================================
// Last updated on 1/10/11; 1/11/12; 5/19/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string puma_metadata_filename="puma.metadata";

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string message_sender_ID="VIEWBUNDLER";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   Messenger OBSFRUSTA_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

// Create OSG root node:

   osg::Group* root = new osg::Group;
   
// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=false;
   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
//   bool display_movie_world_time=false;
   bool display_movie_world_time=true;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(0.2);
//   AnimationController_ptr->setDelay(2);

   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.generate_Clouds(passes_group);
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Instantiate groups to hold multiple surface textures,
// latitude-longitude grids and associated earth regions:

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);

   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);

   bool place_onto_bluemarble_flag=false;
   bool generate_pointcloud_LatLongGrid_flag=false;
   bool display_SurfaceTexture_LatLongGrid_flag=true;
//   bool display_SurfaceTexture_LatLongGrid_flag=false;
   earth_regions_group.generate_regions(
      passes_group,place_onto_bluemarble_flag,
      generate_pointcloud_LatLongGrid_flag,
      display_SurfaceTexture_LatLongGrid_flag);
   root->addChild(earth_regions_group.get_OSGgroup_ptr());

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   if (LatLongGrid_ptr==NULL)
   {

// Ft Devens

      double min_lon=-71.67;
      double max_lon=-71.6555;
      double min_lat=42.4925;
      double max_lat=42.5100;
      double min_Z=100;	// meters

// HAFB

//      double min_lon=-71.3;
//      double max_lon=-71.28;
//      double min_lat=42.4525;
//      double max_lat=42.475;
//      double min_Z=10;	// meters

      LatLongGrid_ptr=latlonggrids_group.generate_new_Grid(
         min_lon,max_lon,min_lat,max_lat,min_Z);
   }

   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);

   decorations.set_grid_origin_ptr(grid_origin_ptr);
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
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate signposts, features, army symbol and sphere segments
// decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=decorations.add_SignPosts(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate OBSFRUSTA decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(
      &OBSFRUSTA_photo_network_messenger);
//   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(false);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
   OBSFRUSTAGROUP_ptr->set_flashlight_mode_flag(true);
   OBSFRUSTAGROUP_ptr->set_project_frames_onto_zplane_flag(true);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

// Instantiate Polygons decoration groups:

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

// Instantiate Points decoration group:

   osgGeometry::PointsGroup* PointsGroup_ptr=decorations.add_Points(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PointsGroup_ptr(PointsGroup_ptr);

// Instantiate PolyLines decoration group:

   PolyLinesGroup* Flight_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   Flight_PolyLinesGroup_ptr->set_width(5);

// Instantiate LOSMODEL decoration group:

   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr=decorations.add_LOSMODELS(
      passes_group.get_pass_ptr(cloudpass_ID),
      Flight_PolyLinesGroup_ptr,NULL,&operations);
   Aircraft_MODELSGROUP_ptr->set_AircraftModelType(MODELSGROUP::LiMIT);
   Aircraft_MODELSGROUP_ptr->set_altitude_dependent_MODEL_scale_flag(false);
   Aircraft_MODELSGROUP_ptr->set_EarthRegionsGroup_ptr(&earth_regions_group);
   Aircraft_MODELSGROUP_ptr->set_CM_3D_ptr(CM_3D_ptr);
   Aircraft_MODELSGROUP_ptr->set_PointCloudsGroup_ptr(&clouds_group);
   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTUMPickHandler_flag(false);
   Aircraft_MODELSGROUP_ptr->set_instantiate_OBSFRUSTAGROUP_flag(false);

   int OSGsubPAT_number;
   double model_scalefactor=1;
   MODEL* Aircraft_MODEL_ptr=Aircraft_MODELSGROUP_ptr->
      generate_Cessna_MODEL(OSGsubPAT_number,model_scalefactor);

   Aircraft_MODELSGROUP_ptr->set_update_dynamic_aircraft_MODEL_flag(true);
   Aircraft_MODEL_ptr->set_dynamically_compute_OBSFRUSTUM_flag(false);
   Aircraft_MODEL_ptr->set_OBSFRUSTAGROUP_ptr(NULL);

// Set master game clock's start time, stop time and time step:

   operations.set_master_world_start_UTC(
      passes_group.get_world_start_UTC_string());
   operations.set_master_world_stop_UTC(
      passes_group.get_world_stop_UTC_string());
   operations.set_delta_master_world_time_step_per_master_frame(
      passes_group.get_world_time_step());
   operations.reset_AnimationController_world_time_params();

// Load aircraft state information into a track object:

   filefunc::ReadInfile(puma_metadata_filename);

   vector<threevector> curr_posn;
   vector<rpy> curr_RPY;

   for (int i=0; i<n_photos; i++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(i);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      curr_posn.push_back(camera_ptr->get_world_posn());

      threevector current_posn,previous_posn;
      if (i==0)
      {
         previous_posn=curr_posn.back();
         current_posn=photogroup_ptr->get_photograph_ptr(i+1)->
            get_camera_ptr()->get_world_posn();
      }
      else
      {
         current_posn=curr_posn.back();
         previous_posn=curr_posn[curr_posn.size()-2];
      }
      threevector delta_posn=current_posn-previous_posn;
      double az=atan2(delta_posn.get(1),delta_posn.get(0));

// Recall yaw = azimuth - 90 (degs):

      double UAV_heading=180/PI*az-90;
      
/*
      vector<string> column_values=
         stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i]);
      double UAV_heading=stringfunc::string_to_number(column_values[5]);
      double UAV_pitch=stringfunc::string_to_number(column_values[6]);
      double UAV_bank=stringfunc::string_to_number(column_values[7]);
      cout << "i = " << i 
           << " heading = " << UAV_heading
           << " pitch = " << UAV_pitch
           << " bank = " << UAV_bank
           << endl;
*/

      curr_RPY.push_back(
         rpy(0,0,UAV_heading));
//         rpy(aircraft_roll[i],aircraft_pitch[i],aircraft_yaw[i]));
   }

   Aircraft_MODELSGROUP_ptr->generate_Aircraft_MODEL_track_and_mover(
      Aircraft_MODEL_ptr);
   track* track_ptr=Aircraft_MODEL_ptr->get_track_ptr();

// Load aircraft positions and orientations as well as sensor
// orientations and fields-of-view into *track_ptr:

   for (int i=0; i<n_photos; i++)
   {
      double curr_time=AnimationController_ptr->
         get_time_corresponding_to_frame(i);
      track_ptr->set_posn_rpy(curr_time,curr_posn[i],curr_RPY[i]);
   }

// Need to set scale for Aircraft MODEL after beginning and ending
// times have been specified:

   Aircraft_MODELSGROUP_ptr->set_constant_scale(
      Aircraft_MODEL_ptr,model_scalefactor);

// Attach all data and decorations to scenegraph:

// Experiment with NOT adding point cloud to root node for
// visualization purposes only

//   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   OBSFRUSTAGROUP_ptr->display_OBSFRUSTA_as_time_sequence();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

}
