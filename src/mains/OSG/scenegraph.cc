// ========================================================================
// Diagnostic program SCENEGRAPH generates and displays a tree
// illustrating the scene graph for the input data object.  Various
// OSG nodes (LODS, Geodes, MatrixTransforms, etc) are color-coded
// within the output graph.  Links between children and their common
// parent are also colored to emphasize siblining relationships.

//	scenegraph lowell.xyzp

//	scenegraph cambridge.osga

//	scenegraph /home/cho/osg_stuff/dted/puget_Height.osga

//	scenegraph /home/cho/programs/c++/svn/projects/data/OpenSceneGraph-Data/SolarSystem/hires_earth.osga

// ========================================================================
// Last updated on 9/21/07; 10/11/07; 10/15/07; 3/10/09; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGrid/AlirtGridsGroup.h"

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgSceneGraph/TreeVisitor.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input ladar point cloud file:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewers and instantiate ViewerManagers:

   WindowManager* window_mgr_scenegraph_ptr=new ViewerManager();
   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   string window_title="Scene graph currently in memory";
   string window_3D_title="3D imagery";
   window_mgr_scenegraph_ptr->initialize_dual_windows(
      window_title,window_3D_title,window_mgr_3D_ptr);

// Create two OSG root nodes:

   osg::Group* root = new osg::Group;
   osg::Group* root_3D = new osg::Group;
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

// Add custom manipulators:

   bool disable_rotations_flag=true;
   osgGA::Custom3DManipulator* CM_scenegraph_ptr=
      new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_scenegraph_ptr,disable_rotations_flag);
   window_mgr_scenegraph_ptr->set_CameraManipulator(CM_scenegraph_ptr);

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_scenegraph_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);

   bool world_origin_precisely_in_lower_left_corner=true;
   AlirtGrid* scenegraph_grid_ptr=decorations.get_AlirtGridsGroup_ptr()->
      generate_new_Grid(world_origin_precisely_in_lower_left_corner);
   threevector* scenegraph_grid_origin_ptr=
      scenegraph_grid_ptr->get_world_origin_ptr();

// Instantiate a PointFinder;

   PointFinder pointfinder;
   CM_3D_ptr->set_PointFinder(&pointfinder);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   clouds_group.generate_Clouds(passes_group);

   clouds_group.get_TreeVisitor_ptr()->set_DataNode_ptr(root_3D);

   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_3D_ptr));
   root_3D->addChild(clouds_group.get_OSGgroup_ptr());

// Initialize ALIRT grid based upon cloud's bounding box:

   decorations.get_AlirtGridsGroup_ptr()->initialize_grid(
      grid_ptr,clouds_group.get_xyz_bbox());

// Insert scenegraph display's grid:

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

// Instantiate a MyDatabasePager to handle paging of files from disk:

//   viewer::MyDatabasePager* MyDatabasePager_ptr=new viewer::MyDatabasePager(
//      clouds_group.get_SetupGeomVisitor_ptr(),
//      clouds_group.get_ColorGeodeVisitor_ptr());

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_3D_ptr,
      window_mgr_3D_ptr,grid_origin_ptr);
   window_mgr_scenegraph_ptr->get_EventHandlers_ptr()->push_back(
      CenterPickHandler_ptr);

// Instantiate GraphNode decorations group:

//   decorations.add_GraphNodes(
//      passes_group.get_pass_ptr(cloudpass_ID),
//      clouds_group.get_TreeVisitor_ptr(),
//      AnimationController_ptr);
//   root->addChild(decorations.get_GraphNodesGroup_ptr()->
//                  createBoxLight(threevector(20,10,10)));
//   root->addChild(decorations.get_GraphNodesGroup_ptr()->get_OSGgroup_ptr());

//   clouds_group.get_TreeVisitor_ptr()->set_DataNode_ptr(root_3D);
   
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

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   root_3D->addChild(decorations.get_OSGgroup_ptr());
   window_mgr_scenegraph_ptr->setSceneData(root);
   window_mgr_3D_ptr->setSceneData(root_3D);

// Create the windows and run the threads:

   window_mgr_scenegraph_ptr->realize();
   window_mgr_3D_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_scenegraph_ptr->set_eye_to_center_distance(
      basic_math::max(
         scenegraph_grid_ptr->get_xsize(),grid_ptr->get_ysize()));      
   CM_scenegraph_ptr->update_M_and_Minv();

   while( !window_mgr_scenegraph_ptr->done() && !window_mgr_3D_ptr->done() )
   {
      window_mgr_scenegraph_ptr->process();
      window_mgr_3D_ptr->process();
   }

   delete window_mgr_scenegraph_ptr;
   delete window_mgr_3D_ptr;
}

