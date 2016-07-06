// ========================================================================
// Program QTLADARVIEWER is an OSG based viewer for 3D point clouds.

// ========================================================================
// Last updated on 4/10/11; 12/3/11; 6/28/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <QtCore/QtCore>
#include <QtGui/QApplication>

#include "Qt/web/LadarServer.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Initialize Qt application:

   QApplication app(argc,argv);

// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="QTLADARVIEWER";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

   string tomcat_subdir="/usr/local/apache-tomcat/webapps/ladar/";

// Instantiate LadarServer which receives get calls from web page
// buttons:

   string LadarServer_hostname="127.0.0.1";
   int LadarServer_portnumber=4041;
   string LadarServer_URL;
   if (LadarServer_URL.size() > 0)
   {
      LadarServer_hostname=stringfunc::get_hostname_from_URL(
         LadarServer_URL);
      LadarServer_portnumber=stringfunc::get_portnumber_from_URL(
         LadarServer_URL);
   }
   cout << "LadarServer_hostname = " << LadarServer_hostname
        << " LadarServer_portnumber = " << LadarServer_portnumber
        << endl;
   LadarServer Ladar_server(LadarServer_hostname,LadarServer_portnumber);

   vector<string> input_filenames=Ladar_server.open_input_file_dialog();

// Generates point cloud pass from ladar data file entered via Qt:
   
   const int ndims=3;

   PassesGroup passes_group(input_filenames);
   cout << "passes_group.get_n_passes() = "
        << passes_group.get_n_passes() << endl;
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->set_thick_client_window_position();
   window_mgr_ptr->initialize_window("3D imagery");
   window_mgr_ptr->set_auto_generate_movies_flag(true);

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      window_mgr_ptr);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_Viewer_Messenger_ptr(&viewer_messenger);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      set_tomcat_subdir(tomcat_subdir);
   ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
      purge_flash_movies();

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
//   CM_3D_ptr->set_min_camera_height_above_grid(100);	// meters
   CM_3D_ptr->set_enable_underneath_looking_flag(true);	
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   clouds_group.pushback_Messenger_ptr(&viewer_messenger);

   bool index_tree_flag=false;
   clouds_group.generate_Clouds(passes_group,index_tree_flag);
   clouds_group.broadcast_cloud_params();
   clouds_group.set_height_color_map(4);	// large hue value sans white
//   clouds_group.set_prob_color_map(6);		// grey
   clouds_group.set_prob_color_map(0);		// jet

   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());
   clouds_group.get_HiresDataVisitor_ptr()->setDatabasePager(
      MyDatabasePager_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder(&clouds_group);
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate Signpost and Feature decoration groups:

   SignPostsGroup* SignPostsGroup_ptr=
      decorations.add_SignPosts(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   SignPostsGroup_ptr->pushback_Messenger_ptr(&viewer_messenger);

   FeaturesGroup* FeaturesGroup_ptr=
      decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   FeaturesGroup_ptr->pushback_Messenger_ptr(&viewer_messenger);

   decorations.get_FeaturePickHandler_ptr()->
      set_surface_picking_flag(false);
   decorations.get_FeaturePickHandler_ptr()->
      set_Zplane_picking_flag(false);

// Instantiate Polyhedra, Polygons & PolyLines decoration groups:

/*
   PolyhedraGroup* PolyhedraGroup_ptr=decorations.add_Polyhedra(
      passes_group.get_pass_ptr(cloudpass_ID));
*/

   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),AnimationController_ptr);
   PolygonsGroup_ptr->set_permanent_colorfunc_color(colorfunc::purple);

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   PolyLinesGroup_ptr->set_width(8);
   PolyLinesGroup_ptr->set_n_text_messages(1);
   PolyLinesGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);
   PolyLinesGroup_ptr->set_variable_Point_size_flag(true);
   PolyLinesGroup_ptr->set_altitude_dependent_labels_flag(false);
   PolyLinesGroup_ptr->set_ID_labels_flag(false);

   PolyLinePickHandler* PolyLinePickHandler_ptr=
      decorations.get_PolyLinePickHandler_ptr();
   PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.25);
//   PolyLinePickHandler_ptr->set_form_polygon_flag(true);
//   PolyLinePickHandler_ptr->set_form_polyhedron_skeleton_flag(true);

// Instantiate photogroup to hold simulated or genuine images:

   photogroup* photogroup_ptr=new photogroup;

// Instantiate OBSFRUSTAGROUP decoration group:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=decorations.add_OBSFRUSTA(
      passes_group.get_pass_ptr(0),AnimationController_ptr);
   OBSFRUSTAGROUP_ptr->set_FOV_excess_fill_factor(1.0);

// Instantiate separate RegionPolyLinesGroup to hold ROI skeletons:

   RegionPolyLinesGroup* ROIsGroup_ptr=decorations.add_RegionPolyLines(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   ROIsGroup_ptr->set_ROI_PolyLinesGroup_flag(true);
   ROIsGroup_ptr->set_width(4);	
   ROIsGroup_ptr->set_CM_3D_ptr(CM_3D_ptr);

   double ROI_skeleton_height=50;	// meters
   ROIsGroup_ptr->set_skeleton_height(ROI_skeleton_height);

   RegionPolyLinePickHandler* ROILinePickHandler_ptr=
      decorations.get_RegionPolyLinePickHandler_ptr();
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
   ROILinePickHandler_ptr->set_permanent_color(ROI_color);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      AnimationController_ptr,grid_origin_ptr);
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,window_mgr_ptr,
      grid_origin_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(CenterPickHandler_ptr);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new CentersKeyHandler(ndims,CenterPickHandler_ptr,ModeController_ptr));

// Attach all 3D graphicals to CentersGroup SpinTransform which can
// perform global rotation about some axis coming out of user selected
// center location:

   centers_group.get_SpinTransform_ptr()->addChild(
      decorations.get_OSGgroup_ptr());
   centers_group.get_SpinTransform_ptr()->addChild(
      clouds_group.get_OSGgroup_ptr());
   root->addChild(centers_group.get_SpinTransform_ptr());

// Ladar server variable settings:

   Ladar_server.set_CM_ptr(CM_3D_ptr);
   Ladar_server.set_Grid_ptr(grid_ptr);
   Ladar_server.set_Operations_ptr(&operations);
   Ladar_server.set_PointCloudsGroup_ptr(&clouds_group);
   Ladar_server.set_SignPostsGroup_ptr(SignPostsGroup_ptr);
   Ladar_server.set_SignPostPickHandler_ptr(
      decorations.get_SignPostPickHandler_ptr());
   Ladar_server.set_FeaturesGroup_ptr(FeaturesGroup_ptr);
   Ladar_server.set_FeaturePickHandler_ptr(
      decorations.get_FeaturePickHandler_ptr());
   Ladar_server.set_PolygonsGroup_ptr(PolygonsGroup_ptr);
   Ladar_server.set_PolyLinesGroup_ptr(PolyLinesGroup_ptr);
   Ladar_server.set_PolyLinePickHandler_ptr(
      decorations.get_PolyLinePickHandler_ptr());
   Ladar_server.set_photogroup_ptr(photogroup_ptr);
   Ladar_server.set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP_ptr);
   Ladar_server.set_RegionPolyLinesGroup_ptr(ROIsGroup_ptr);
   Ladar_server.set_ROIPickHandler_ptr(
      decorations.get_RegionPolyLinePickHandler_ptr());

   Ladar_server.set_tomcat_subdir(tomcat_subdir);

// Attach scene graph to viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();
   
   while( !window_mgr_ptr->done() )
   {
      app.processEvents();
      window_mgr_ptr->process();
   }

   delete window_mgr_ptr;
   delete photogroup_ptr;
}
