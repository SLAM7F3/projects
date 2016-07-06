// ========================================================================
// Program TOUR
// ========================================================================
// Last updated on 2/21/10; 2/28/10; 7/26/10
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
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgModels/PhotoToursGroup.h"
#include "osg/osgModels/PhotoToursKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
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

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string xyz_points_filename=passes_group.get_xyz_points_filename();
   cout << "xyz_points_filename = " << xyz_points_filename << endl;
   string camera_views_filename=passes_group.get_camera_views_filename();
   cout << "camera_views_filename = " << camera_views_filename << endl;
   string common_planes_filename=passes_group.get_common_planes_filename();
   cout << "common_planes_filename = " << common_planes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

// Import common planes for pairs of overlapping cameras:

   photogroup_ptr->import_common_photo_planes(common_planes_filename);

// Read in reconstructed XYZ points plus their IDs along with visible
// camera IDs into videofunc map *xyz_map_ptr:

   videofunc::import_reconstructed_XYZ_points(xyz_points_filename);

//  Generate STL map containing STL vectors of reconstructed XYZ
//  points as a function of visible camera ID:

   videofunc::CAMERAID_XYZ_MAP* cameraid_xyz_map_ptr=
      videofunc::sort_XYZ_points_by_camera_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;

   string message_sender_ID="VIEWBUNDLE";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   Messenger OBSFRUSTA_photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID);

// Create OSG root node:

   osg::Group* root = new osg::Group;
   
// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
//   bool display_movie_state=true;
   bool display_movie_number=false;
//   bool display_movie_number=true;
//   bool hide_Mode_HUD_flag=true;
//   bool hide_Mode_HUD_flag=false;
   bool display_movie_world_time=false;
//   bool generate_AVI_movie_flag=false;
   bool generate_AVI_movie_flag=true;

   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      generate_AVI_movie_flag);

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

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

/*
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
*/

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.set_Terrain_Manipulator_ptr(CM_3D_ptr);
   clouds_group.set_auto_resize_points_flag(false);
//   clouds_group.set_auto_resize_points_flag(true);

   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(cloudpass_ID),      
      &clouds_group,&latlonggrids_group);

   earth_regions_group.generate_regions(passes_group);

   latlonggrids_group.erase_Graphical(0);
//   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(0);
   LatLongGrid* LatLongGrid_ptr=latlonggrids_group.get_Grid_ptr(1);
   LatLongGrid_ptr->set_dynamic_grid_flag(false);
   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(LatLongGrid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

   decorations.set_PointCloudsGroup_ptr(&clouds_group);

   double xmin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMin();
   double xmax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().xMax();
   double ymin=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMin();
   double ymax=clouds_group.get_Cloud_ptr(0)->get_xyz_bbox().yMax();
   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
   cout << "ymin = " << ymin << " ymax = " << ymax << endl;
   
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,LatLongGrid_ptr));

/*
// Initialize ALIRT grid based upon cloud's bounding box:

   osg::BoundingBox bbox=clouds_group.get_xyz_bbox();

   if (filefunc::getbasename(bundler_IO_subdir)=="Noah_MIT2328/" ||
       filefunc::getbasename(bundler_IO_subdir)=="MIT2328/")
   {

// FAKE FAKE:  hack for MIT2328 case...

      double z_grid=-450;	// meters
      decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
         grid_ptr,bbox.xMin(),bbox.xMax(),bbox.yMin(),bbox.yMax(),z_grid);
   }
   else
   {
      decorations.get_AlirtGridsGroup_ptr()->initialize_grid(grid_ptr,bbox);
   }
   grid_ptr->set_delta_xy(1,1);
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
   OBSFRUSTAGROUP_ptr->set_enable_OBSFRUSTA_blinking_flag(true);
   OBSFRUSTAGROUP_ptr->set_erase_other_OBSFRUSTA_flag(true);
   OBSFRUSTAGROUP_ptr->set_cameraid_xyz_map_ptr(cameraid_xyz_map_ptr);

// Instantiate an individual OBSFRUSTUM for every photograph:

   bool multicolor_frusta_flag=false;
   bool thumbnails_flag=true;
   OBSFRUSTAGROUP_ptr->generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,multicolor_frusta_flag,thumbnails_flag);

// Set mask_nonselected_OSGsubPATs_flag=false in order to
// simultaneously display OSGsubPATs 0 and 1 respectively containing
// static panorama and dynamic video images:

   decorations.get_OBSFRUSTUMPickHandler_ptr()->
      set_mask_nonselected_OSGsubPATs_flag(false);
   decorations.get_OBSFRUSTAKeyHandler_ptr()->set_SignPostsGroup_ptr(
      SignPostsGroup_ptr);

// Instantiate Polygons & PolyLines decoration groups:

//   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
//      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
//   OBSFRUSTAGROUP_ptr->set_PolygonsGroup_ptr(PolygonsGroup_ptr);

   PolyLinesGroup* PhotoTour_PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PhotoTour_PolyLinesGroup_ptr->set_width(8);

//   PolygonsGroup_ptr->set_PhotoTour_PolyLinesGroup_ptr(PhotoTour_PolyLinesGroup_ptr);

   PolyLinePickHandler* PhotoTour_PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PhotoTour_PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PhotoTour_PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
   PhotoTour_PolyLinePickHandler_ptr->set_z_offset(5);

   osgGeometry::PointsGroup* PointsGroup_ptr=decorations.add_Points(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_PointsGroup_ptr(PointsGroup_ptr);

// Instantiate PhotoToursGroup:

   PhotoToursGroup* PhotoToursGroup_ptr=new PhotoToursGroup(
      OBSFRUSTAGROUP_ptr,PhotoTour_PolyLinesGroup_ptr);
   PhotoToursKeyHandler* PhotoToursKeyHandler_ptr=
      new PhotoToursKeyHandler(
         PhotoToursGroup_ptr,OBSFRUSTAGROUP_ptr,&clouds_group,
         ModeController_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      PhotoToursKeyHandler_ptr);

   root->addChild(clouds_group.get_OSGgroup_ptr());
   root->addChild(latlonggrids_group.get_OSGgroup_ptr());
   root->addChild(decorations.get_OSGgroup_ptr());
   root->addChild(PhotoToursGroup_ptr->get_OSGgroup_ptr());

// Attach scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
   delete window_mgr_ptr;

}
