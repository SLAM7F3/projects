// ========================================================================
// Diagnostic program EARTHSCENE generates and displays a tree
// illustrating the scene graph for the input data object.  Various
// OSG nodes (LODS, Geodes, MatrixTransforms, etc) are color-coded
// within the output graph.  Links between children and their common
// parent are also colored to emphasize siblining relationships.

//			earthscene lowell.xyzp

//			earthscene boston.osga

// ========================================================================
// Last updated on 9/18/07; 9/21/07; 10/15/07; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgGA/NodeTrackerManipulator>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/BoxesGroup.h"
#include "osg/osgGeometry/BoxesKeyHandler.h"
#include "osg/osgGeometry/BoxPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/DepthPartitionNode.h"
#include "osg/osgSceneGraph/DistanceAccumulator.h"
#include "osg/osgGrid/EarthGrid.h"
#include "osg/osgEarth/EarthsGroup.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/Ellipsoid_model.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgSceneGraph/TreeVisitor.h"
#include "osg/osgWindow/ViewerManager.h"


// ==========================================================================
int main( int argc, char** argv )
{
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Instantiate clock to keep track of real time:

   Clock clock;
   clock.current_local_time_and_UTC();

/*
   int year=2006;
   int month=8;
   int day=24;
//   int local_hour=12;

//   int local_hour=23;
//   cout << "Enter local hour:" << endl;
//   cin >> local_hour;

//   int UTC_hour=11;
   int UTC_hour=20;
//   cout << "Enter UTC hour:" << endl;
//   cin >> UTC_hour;
   int minutes=0;
   double secs=0;

//   clock.set_UTM_zone_time_offset(19);	// Boston
//   clock.set_daylight_savings_flag(true);
//   clock.set_local_time(year,month,day,local_hour,minutes,secs);
   clock.set_UTC_time(year,month,day,UTC_hour,minutes,secs);
*/

// Read input ladar point cloud file:
   
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   PassInfo* PassInfo_ptr=passes_group.get_passinfo_ptr(cloudpass_ID);

   string OSG_data_dir(getenv("OSG_FILE_PATH"));
   OSG_data_dir += "/";
   string solarsystem_dir=OSG_data_dir+"SolarSystem/";
   string earth_filename=solarsystem_dir+"hires_earth.osga";
   int earthpass_ID=passes_group.generate_new_pass(earth_filename);
   passes_group.get_pass_ptr(earthpass_ID)->set_passtype(Pass::earth);
   cout << " earthpass_ID = " << earthpass_ID 
        << " cloudpass_ID = " << cloudpass_ID << endl;

// Construct the viewers and instantiate ViewerManagers:

   WindowManager* window_mgr_scenegraph_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();

   string window_title="Scene graph currently in memory";
   string window_3D_title="3D imagery";
   window_mgr_scenegraph_ptr->initialize_dual_windows(
      window_title,window_3D_title,window_mgr_3D_ptr);

// Create conventional OSG root node for scenegraph:

   osg::Group* root = new osg::Group;

// Create a DepthPartitionNode to manage partitioning of the scene:

   DepthPartitionNode* root_3D = new DepthPartitionNode;
   root_3D->setActive(true); 	// Control whether the node analyzes the
			     	// scene
//   cout << "max_depth = " << root_3D->getMaxTraversalDepth() << endl;
   root_3D->setName("Root 3D");

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_scenegraph_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::MANIPULATE_GRAPHNODE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

   ModeController* ModeController_3D_ptr=new ModeController();
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_3D_ptr) );
   root_3D->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_3D_ptr));

// Instantiate invisible "holodeck" grid for scenegraph window:

   AlirtGridsGroup alirtgrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   AlirtGrid* scenegraph_grid_ptr=alirtgrids_group.generate_new_Grid();
   threevector* scenegraph_grid_origin_ptr=
      scenegraph_grid_ptr->get_world_origin_ptr();

   window_mgr_scenegraph_ptr->get_EventHandlers_ptr()->push_back(
      new GridKeyHandler(ModeController_ptr,scenegraph_grid_ptr));
//   root->addChild(scenegraph_grid_ptr->get_geode_ptr());

// Instantiate "holodeck" grid surrounding Earth:

   EarthGrid* earthgrid_ptr = new EarthGrid(colorfunc::grey);
   threevector* earthgrid_origin_ptr=earthgrid_ptr->get_world_origin_ptr();
//   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
//      new GridKeyHandler(ModeController_3D_ptr,earthgrid_ptr));

// Instantiate PlanetsGroup to hold sun and planets:

   PlanetsGroup planets_group(
      passes_group.get_pass_ptr(earthpass_ID),&clock,earthgrid_origin_ptr);
   root_3D->addChild(planets_group.get_EarthSpinTransform_ptr());

// Generate the earth as well the solar system:

   DataGraph* EarthGraph_ptr=planets_group.generate_EarthGraph();
   osg::Group* solarsystem_ptr=planets_group.generate_solarsystem(
      earthgrid_ptr);
   root_3D->addChild(solarsystem_ptr);

// Instantiate a PointFinder;

   PointFinder pointfinder(&planets_group);

// Instantiate an EarthsGroup to hold "blue marble" coordinate system
// information:

   EarthsGroup earths_group(
      passes_group.get_pass_ptr(cloudpass_ID),&clock,earthgrid_origin_ptr);
   planets_group.get_EarthSpinTransform_ptr()->
      addChild(earths_group.get_OSGgroup_ptr());
   Earth* Earth_ptr=earths_group.generate_new_Earth();

// Add custom manipulators for both the 3D world and 2D scenegraph
// windows:

   bool disable_rotations_flag=true;
   osgGA::Custom3DManipulator* CM_scenegraph_ptr=
      new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_scenegraph_ptr,disable_rotations_flag);
   window_mgr_scenegraph_ptr->set_CameraManipulator(CM_scenegraph_ptr);


// Add a custom manipulator to the event handler list:

   osgGA::EarthManipulator* CM_3D_ptr=new osgGA::EarthManipulator(
      ModeController_3D_ptr,Earth_ptr->get_Ellipsoid_model_ptr(),&clock,
      window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

   CM_3D_ptr->set_PointFinder(&pointfinder);
   Earth_ptr->set_EarthManipulator_ptr(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_scenegraph_ptr,ModeController_ptr,
                           CM_3D_ptr);

// Generate random background star field:

//   root_3D->addChild(planets_group.generate_starfield(earthgrid_ptr));

// Instantiate groups to hold multiple point clouds,
// latitude-longitude grids and associated earth regions:

   PointCloudsGroup clouds_group(passes_group.get_pass_ptr(cloudpass_ID));
   LatLongGridsGroup latlonggrids_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),CM_3D_ptr);
   EarthRegionsGroup earth_regions_group(
      passes_group.get_pass_ptr(earthpass_ID),
      &clouds_group,&latlonggrids_group,Earth_ptr);

   earth_regions_group.generate_regions(passes_group);
   earths_group.get_OSGgroup_ptr()->addChild(
      earth_regions_group.get_OSGgroup_ptr());

   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));

// Instantiate a MyDatabasePager to handle paging of files from disk:

   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
      clouds_group.get_SetupGeomVisitor_ptr(),
      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate group to hold GraphNodes to visualize scenegraph:

   clouds_group.get_TreeVisitor_ptr()->set_DataNode_ptr(root_3D);

   GraphNodesGroup graphnodes_group(
      passes_group.get_pass_ptr(cloudpass_ID),
      scenegraph_grid_origin_ptr,clouds_group.get_TreeVisitor_ptr(),
      AnimationController_ptr);

   root->addChild(graphnodes_group.createBoxLight(threevector(20,10,10)));
   root->addChild(graphnodes_group.get_OSGgroup_ptr());

// Instantiate a GraphNodesKeyHandler for debugging purposes:

   window_mgr_scenegraph_ptr->get_EventHandlers_ptr()->push_back(
      new GraphNodesKeyHandler(&graphnodes_group,ModeController_ptr));
   graphnodes_group.generate_Graph_from_tree();

// Insert "holodeck" grid in XY plane underneath point cloud and other
// geometrical objects:

   double min_X=0;
   double max_X=10;
   double min_Y=0;
   double max_Y=10;
   double min_Z=0;
//   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
//   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;

   scenegraph_grid_ptr->set_XYZ_extents(min_X,max_X,min_Y,max_Y,min_Z);
   scenegraph_grid_ptr->set_axes_labels("X","Y");
   scenegraph_grid_ptr->set_delta_xy(2,2);
   scenegraph_grid_ptr->set_axis_char_label_size(1);
   scenegraph_grid_ptr->set_tick_char_label_size(1);
   scenegraph_grid_ptr->update_grid();

   PointCloud* cloud_ptr=clouds_group.get_Cloud_ptr(0);
   min_X=cloud_ptr->get_min_value(0);
   max_X=cloud_ptr->get_max_value(0);
   min_Y=cloud_ptr->get_min_value(1);
   max_Y=cloud_ptr->get_max_value(1);
   min_Z=cloud_ptr->get_min_value(2);
//   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
//   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   scenegraph_grid_ptr->initialize_ALIRT_grid(min_X,max_X,min_Y,max_Y,min_Z);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   root_3D->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_scenegraph_ptr->setSceneData(root);
   window_mgr_3D_ptr->setSceneData(root_3D);

// Create the windows and run the threads:

   window_mgr_scenegraph_ptr->realize();
   window_mgr_3D_ptr->realize();

// Scale initial scenegraph height according to scenegraph grid's
// maximal linear dimension:

   CM_scenegraph_ptr->set_eye_to_center_distance(
      basic_math::max(scenegraph_grid_ptr->get_xsize(),
          scenegraph_grid_ptr->get_ysize()));      

   while( !window_mgr_scenegraph_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_scenegraph_ptr->process();
      window_mgr_3D_ptr->process();
   }

   delete window_mgr_scenegraph_ptr;
   delete window_mgr_3D_ptr;
}

