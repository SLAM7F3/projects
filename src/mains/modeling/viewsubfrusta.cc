// ========================================================================
// Program VIEWSUBFRUSTA is a variant of program VIEWBUNDLER.  It
// instantiates and displays sub OBSFRUSTA corresponding to bounding
// boxes selected within individual photo image planes.  It can also 
// display 3D human figures if the 2D bounding boxes correspond to
// human-sized objects.  

/* 

./viewsubfrusta \
./bundler/MIT2317/thresholded_xyz_points.osga \
--region_filename ./bundler/MIT2317/packages/photo_0031.pkg \
--image_list_filename ./bundler/MIT2317/image_list.dat \
--image_sizes_filename ./bundler/MIT2317/image_sizes.dat \
--xyz_pnts_filename ./bundler/MIT2317/thresholded_xyz_points.dat \
--camera_views_filename ./bundler/MIT2317/sorted_camera_views.dat \
--initial_mode Manipulate_Fused_Data_Mode

*/

// ========================================================================
// Last updated on 1/31/12; 2/1/12; 8/13/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "models/ParkingLotsGroup.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "models/RoadsGroup.h"
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
   int texturepass_ID=passes_group.get_curr_texturepass_ID();

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string xyz_points_filename=passes_group.get_xyz_points_filename();
   cout << "xyz_points_filename = " << xyz_points_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      cout << "n = " << n
           << " camera = " << *camera_ptr 
           << endl;
   }

/*
// Read in reconstructed XYZ points plus their IDs along with visible
// camera IDs into videofunc map *xyz_map_ptr:

   videofunc::CAMERAID_XYZ_MAP* cameraid_xyz_map_ptr=NULL;
   if (xyz_points_filename.size() > 0)
   {
      videofunc::import_reconstructed_XYZ_points(xyz_points_filename);

//  Generate STL map containing STL vectors of reconstructed XYZ
//  points as a function of visible camera ID:

      cameraid_xyz_map_ptr=videofunc::sort_XYZ_points_by_camera_ID();
   }
*/

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

   string GPS_message_queue_channel_name="viewer_update";
   Messenger GPS_messenger( 
      broker_URL, GPS_message_queue_channel_name,message_sender_ID);

// Create OSG root node:

   osg::Group* root = new osg::Group;
   
// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
//   bool hide_Mode_HUD_flag=true;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   AnimationController_ptr->setDelay(2);

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

// Force z_grid for MIT2317 to lie at ground level:

   double z_grid=2;	// meters

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,bbox.xMin(),bbox.xMax(),bbox.yMin(),bbox.yMax(),z_grid);
   grid_ptr->set_delta_xy(1,1);
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

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

// Instantiate signpost decoration group:

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
//   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(false);
//    OBSFRUSTAGROUP_ptr->set_cameraid_xyz_map_ptr(cameraid_xyz_map_ptr);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(true);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

   decorations.get_OBSFRUSTAKeyHandler_ptr()->
      set_z_ground(2.55);  // meters (good for MIT)

// Instantiate second OBSFRUSTAGROUP to hold sub frusta:

   OBSFRUSTAGROUP* SUBFRUSTAGROUP_ptr=OBSFRUSTAGROUP_ptr->
      generate_SUBFRUSTAGROUP();

// Instantiate MODEL decorations group to hold 3D human figure:

   MODELSGROUP* human_MODELSGROUP_ptr=decorations.add_MODELS(
      passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   human_MODELSGROUP_ptr->pushback_Messenger_ptr(&GPS_messenger);

   SUBFRUSTAGROUP_ptr->set_target_MODELSGROUP_ptr(human_MODELSGROUP_ptr);

// Instantiate BuildingsGroup object:

   BuildingsGroup* BuildingsGroup_ptr=new BuildingsGroup();
   string OFF_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/";
   BuildingsGroup_ptr->import_from_OFF_files(OFF_subdir);


// Instantiate ParkingLotsGroup object:

   ParkingLotsGroup* ParkingLotsGroup_ptr=new ParkingLotsGroup();
   ParkingLotsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate RoadsGroup object:

   RoadsGroup* RoadsGroup_ptr=new RoadsGroup();
   RoadsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate PolyLines, Polygons and Polyhedra decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=
      decorations.add_Polyhedra(passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->generate_Building_Polyhedra(BuildingsGroup_ptr);



/*



// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));
   PolyhedraGroup_ptr->set_OFF_subdir(
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/");

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolygonsGroup_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);

   PolyLinesGroup_ptr->set_width(8);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   PolyLinePickHandler_ptr->set_z_offset(5);

//   PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);
//   PolyLinePickHandler_ptr->set_form_polygon_flag(true);
*/

// Instantiate Points decorations group:

   osgGeometry::PointsGroup* PointsGroup_ptr=decorations.add_Points(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PointsGroup_ptr(PointsGroup_ptr);

// Instantiate 2D features decorations group:

   FeaturesGroup* UV_FeaturesGroup_ptr=
      decorations.add_Features(2,passes_group.get_pass_ptr(cloudpass_ID));
   UV_FeaturesGroup_ptr->set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);

// Attach all data and decorations to scenegraph:

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

// ========================================================================
// For viewchart purposes only, we try to suppress point cloud display

   clouds_group.set_OSGgroup_nodemask(0);

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

}
