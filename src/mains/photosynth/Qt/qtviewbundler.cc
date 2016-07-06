// ========================================================================
// Program QTVIEWBUNDLER 
// ========================================================================
// Last updated on 4/20/10; 7/14/10; 12/24/10
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <QtCore/QtCore>

#include "Qt/web/PhotoServer.h"

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "bundler/bundlerfuncs.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgModels/PhotoToursGroup.h"
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

// Initialize Qt application:

   QCoreApplication app(argc,argv);

// Reset timefunc clock (for 3D photo fading purposes):

   timefunc::initialize_timeofday_clock();

// Instantiate PhotoServer which receives get calls from web page
// buttons:

   string PhotoServer_hostname="127.0.0.1";
   int PhotoServer_portnumber=4041;
   string PhotoServer_URL;
   if (PhotoServer_URL.size() > 0)
   {
      PhotoServer_hostname=stringfunc::get_hostname_from_URL(PhotoServer_URL);
      PhotoServer_portnumber=stringfunc::get_portnumber_from_URL(
         PhotoServer_URL);
   }
   cout << "PhotoServer_hostname = " << PhotoServer_hostname
        << " PhotoServer_portnumber = " << PhotoServer_portnumber
        << endl;
   PhotoServer Photo_server(PhotoServer_hostname,PhotoServer_portnumber);

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="QTVIEWBUNDLER";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

//   string photo_network_message_queue_channel_name="photo_network";
//   Messenger OBSFRUSTA_photo_network_messenger( 
//      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

//   string photo_set_subdir="iPhone/";
   string photo_set_subdir="kermit/";
//   string photo_set_subdir="MIT2317/";

// Comment out next communication section with thin client for debugging
// purposes

// Wait for thin client to transmit selected photo set name before
// instantiating reconstructed point cloud & cameras:

   while (!Photo_server.get_photoset_selected_flag())
   {
      string command="SEND_THICKCLIENT_READY_FOR_PHOTOSET_INFORMATION";
      viewer_messenger.broadcast_subpacket(command);
      app.processEvents();
      cout << "Waiting for photoset info from thin client" << endl;
   }
   cout << "Photo set selected flag = true" << endl;
   photo_set_subdir=Photo_server.get_photoset_name()+"/";
   cout << "photo_set_subdir = " << photo_set_subdir << endl;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read in reconstructed point cloud & photos.  Generate new pass for
// each input data set:

   int cloudpass_ID=0;
   int photo_number_step=1;
//   int photo_number_step=500;
   string bundler_IO_subdir,image_sizes_filename,xyz_points_filename;
   bundlerfunc::read_in_pointcloud_and_photos(
      photo_set_subdir,passes_group,photo_number_step,cloudpass_ID,
      bundler_IO_subdir,image_sizes_filename,xyz_points_filename);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

// Import common planes for pairs of overlapping cameras:

//   photogroup_ptr->import_common_photo_planes(common_planes_filename);

// Read in reconstructed XYZ points plus their IDs along with visible
// camera IDs into videofunc map *xyz_map_ptr:

   videofunc::import_reconstructed_XYZ_points(xyz_points_filename);

//  Generate STL map containing STL vectors of reconstructed XYZ
//  points as a function of visible camera ID:

   videofunc::CAMERAID_XYZ_MAP* cameraID_xyz_map_ptr=
      videofunc::sort_XYZ_points_by_camera_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
//   cout << "window_mgr_ptr = " << window_mgr_ptr << endl;
   window_mgr_ptr->set_default_Window_X_origin(835);
   window_mgr_ptr->set_default_Window_Y_origin(800); 
     // Origin values appropriate for 30" monitor & laptop screen
   window_mgr_ptr->initialize_window("3D Viewer");
   window_mgr_ptr->set_auto_generate_movies_flag(true);
   window_mgr_ptr->set_horiz_scale_factor(0.66);

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      window_mgr_ptr);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_Viewer_Messenger_ptr(&viewer_messenger);
   string tomcat_subdir="/usr/local/apache-tomcat/webapps/photo/";
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_tomcat_subdir(tomcat_subdir);
   
// Create OSG root node:

   osg::Group* root = new osg::Group;
   
// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
//   bool hide_Mode_HUD_flag=true;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(3);

   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate Grid:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   clouds_group.generate_Clouds(passes_group);

   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);
   decorations.set_PointCloudsGroup_ptr(&clouds_group);

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

   osg::BoundingBox bbox=clouds_group.get_xyz_bbox();
   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(grid_ptr,bbox);
   grid_ptr->set_delta_xy(1,1);

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
   OBSFRUSTAGROUP_ptr->pushback_Messenger_ptr(&viewer_messenger);
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
   OBSFRUSTAGROUP_ptr->set_cameraid_xyz_map_ptr(cameraID_xyz_map_ptr);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

// Instantiate second OBSFRUSTAGROUP to hold sub frusta:

   OBSFRUSTAGROUP* SUBFRUSTAGROUP_ptr=new OBSFRUSTAGROUP(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr,
      grid_origin_ptr);
   SUBFRUSTAGROUP_ptr->set_PARENT_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);
   SUBFRUSTAGROUP_ptr->set_photogroup_ptr(photogroup_ptr);
   decorations.get_OSGgroup_ptr()->
      addChild(SUBFRUSTAGROUP_ptr->get_OSGgroup_ptr());

// Instantiate bounding box for subfrusta:

   bounding_box subfrustum_bbox;
   SUBFRUSTAGROUP_ptr->set_subfrustum_bbox_ptr(&subfrustum_bbox);

// Instantiate Polygons & PolyLines decoration groups:

//   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
//      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
//   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

   PolyLinesGroup* PhotoTour_PolyLinesGroup_ptr=
      decorations.add_PolyLines(
         ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PhotoTour_PolyLinesGroup_ptr->set_width(8);

//   PolygonsGroup_ptr->set_PhotoTour_PolyLinesGroup_ptr(PhotoTour_PolyLinesGroup_ptr);

/*
   PolyLinePickHandler* PhotoTour_PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PhotoTour_PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PhotoTour_PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   PhotoTour_PolyLinePickHandler_ptr->set_z_offset(5);
*/

   osgGeometry::PointsGroup* PointsGroup_ptr=decorations.add_Points(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PointsGroup_ptr(PointsGroup_ptr);

// Instantiate PhotoToursGroup:

   PhotoToursGroup* PhotoToursGroup_ptr=new PhotoToursGroup(
      OBSFRUSTAGROUP_ptr,PhotoTour_PolyLinesGroup_ptr);

// Instantiate MODEL decorations group to hold 3D human figure:

   MODELSGROUP* human_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   SUBFRUSTAGROUP_ptr->set_target_MODELSGROUP_ptr(human_MODELSGROUP_ptr);

// Attach all data and decorations to scenegraph:

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(PhotoToursGroup_ptr->get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   Photo_server.set_northern_hemisphere_flag(true);
   Photo_server.set_specified_UTM_zonenumber(19);	// Boston
   Photo_server.set_bounding_box_ptr(&subfrustum_bbox);
   Photo_server.set_Grid_ptr(grid_ptr);
   Photo_server.set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);
   Photo_server.set_Operations_ptr(&operations);
   Photo_server.set_PointCloudsGroup_ptr(&clouds_group);
   Photo_server.set_PhotoToursGroup_ptr(PhotoToursGroup_ptr);
   Photo_server.set_PhotoTour_PolyLinesGroup_ptr(
      PhotoTour_PolyLinesGroup_ptr);
   Photo_server.set_SUBFRUSTAGROUP_ptr(SUBFRUSTAGROUP_ptr);
   Photo_server.set_SignPostsGroup_ptr(SignPostsGroup_ptr);
   Photo_server.set_tomcat_subdir(tomcat_subdir);
   Photo_server.set_CM_3D_ptr(CM_3D_ptr);

   string command="SEND_THICKCLIENT_READY_FOR_USER_INPUT";
   viewer_messenger.broadcast_subpacket(command);

// ========================================================================
   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
      app.processEvents();
   }
   delete window_mgr_ptr;

}
