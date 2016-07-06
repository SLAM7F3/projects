// ========================================================================
// Program GRAPH3D is a testing ground for visualizing Scenegraphs.
// ========================================================================
// Last updated on 11/7/06; 11/8/06; 11/9/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>
#include <osgDB/WriteFile>

#include "osg/osgGrid/AlirtGrid.h"
#include "osg/AnimationPathCreator.h"
#include "osg/osgGeometry/Box.h"
#include "osg/osgGeometry/BoxesGroup.h"
#include "osg/osgGeometry/BoxesKeyHandler.h"
#include "osg/osgGeometry/BoxPickHandler.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom3DManipulator.h"
#include "general/filefuncs.h"
#include "osg/osgAnnotators/GraphNode.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGeometry/LineSegment.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgGeometry/LineSegmentsKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGraphicals/Transformer.h"
#include "datastructures/Tree.h"

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

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=3;

// Read input ladar point cloud file:
   
   PassesGroup passes_group;
   string cloud_filename="empty.xyzp";
   int cloudpass_ID=passes_group.add_pass(cloud_filename);

// Construct the viewer and set it up with sensible event handlers:

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
//   viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
   viewer.setUpViewer(osgProducer::Viewer::HEAD_LIGHT_SOURCE |
                      osgProducer::Viewer::ESCAPE_SETS_DONE);

// Initialize viewer window:

   Producer::RenderSurface* rs_ptr =
      viewer.getCameraConfig()->getCamera(0)->getRenderSurface();

   string window_title="3D boxes";
   osgfunc::initialize_window(rs_ptr,window_title);

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate a mode controller and key event handler:

   ModeController* ModeController_ptr=new ModeController();
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Instantiate "holodeck" grid:

   bool world_origin_precisely_in_lower_left_corner=true;
   AlirtGrid* grid_ptr = new AlirtGrid(
      world_origin_precisely_in_lower_left_corner);
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();

   viewer.getEventHandlerList().push_back(
      new GridKeyHandler(ModeController_ptr,grid_ptr));
   root->addChild(grid_ptr->get_geode_ptr());

// Instantiate a transformer in order to convert between screen and
// world space coordinate systems:

   Transformer transformer(&viewer);

// Add a custom manipulator to the event handler list:

//   bool disable_rotations_flag=true;
   bool disable_rotations_flag=false;
   osgGA::Custom3DManipulator* CM_3D_ptr=new osgGA::Custom3DManipulator(
      ModeController_ptr,disable_rotations_flag);
   window_mgr.set_CameraManipulator(CM_3D_ptr);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(cloudpass_ID),
      CM_3D_ptr,&centers_group,ModeController_ptr,
      &transformer,grid_origin_ptr);
   viewer.getEventHandlerList().push_back(CenterPickHandler_ptr);

// Instantiate group to hold LineSegments:

   LineSegmentsGroup linesegments_group(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   root->addChild(linesegments_group.get_OSGgroup_ptr());

// Instantiate LineSegment key handler and pick handler:

//   viewer.getEventHandlerList().push_back(new LineSegmentsKeyHandler(
//      &linesegments_group,ModeController_ptr));

   LineSegment* line0_ptr=linesegments_group.generate_new_segment(
      threevector(5,5),threevector(10,9));
   LineSegment* line1_ptr=linesegments_group.generate_new_segment(
      threevector(4,3),threevector(7,2));
   linesegments_group.reset_colors();

// Generate Scenegraph tree:

   typedef string data_type;
   Tree<data_type>* tree_ptr=new Tree<data_type>;

   Treenode<data_type>* root_ptr=tree_ptr->get_root_ptr();
   root_ptr->set_data("root");

   Treenode<data_type>* node00_ptr=tree_ptr->addChild(root_ptr->get_ID());
   node00_ptr->set_data("LOD");
   Treenode<data_type>* node000_ptr=tree_ptr->addChild(node00_ptr->get_ID());
   node000_ptr->set_data("Geode");
   Treenode<data_type>* node001_ptr=tree_ptr->addChild(node00_ptr->get_ID());
   node001_ptr->set_data("Node");

   Treenode<data_type>* node01_ptr=tree_ptr->addChild(root_ptr->get_ID());
   node01_ptr->set_data("MatrixTransform");
   Treenode<data_type>* node010_ptr=tree_ptr->addChild(node01_ptr->get_ID());
   node010_ptr->set_data("Geode");
   Treenode<data_type>* node011_ptr=tree_ptr->addChild(node01_ptr->get_ID());
   node011_ptr->set_data("Node");
   Treenode<data_type>* node012_ptr=tree_ptr->addChild(node01_ptr->get_ID());
   node012_ptr->set_data("Geode");

   Treenode<data_type>* node02_ptr=tree_ptr->addChild(root_ptr->get_ID());
   node02_ptr->set_data("PageLOD");
   Treenode<data_type>* node020_ptr=tree_ptr->addChild(node02_ptr->get_ID());
   node020_ptr->set_data("geode");

   vector<int> tot_indices;
   tot_indices.push_back(0);
   tot_indices.push_back(2);
   tot_indices.push_back(1);
   Treenode<data_type>* node_021_ptr=tree_ptr->addChild(tot_indices);

   tot_indices.clear();
   tot_indices.push_back(0);
   tot_indices.push_back(1);
   tot_indices.push_back(1);
   tot_indices.push_back(1);
   tot_indices.push_back(0);
   Treenode<data_type>* node_0110_ptr=tree_ptr->addChild(tot_indices);   

// Instantiate group to hold GraphNodes to visualize scenegraph:

   GraphNodesGroup graphnodes_group(
      passes_group.get_pass_ptr(cloudpass_ID),tree_ptr,grid_origin_ptr);
   root->addChild(graphnodes_group.createBoxLight(threevector(20,10,10)));
   root->addChild(graphnodes_group.get_OSGgroup_ptr());

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

   grid_ptr->set_XYZ_extents(min_X,max_X,min_Y,max_Y,min_Z);
   grid_ptr->set_axes_labels("X","Y");
   grid_ptr->set_delta_xy(2,2);
   grid_ptr->set_axis_char_label_size(1);
   grid_ptr->set_tick_char_label_size(1);
   grid_ptr->update_grid();

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(root);
   viewer.setSceneData(root);

// Create the windows and run the threads:

   viewer.realize();

// Add an animation path creator to the event handler list AFTER the
// viewer has been realized:

   AnimationPathCreator* animation_path_handler = 
      new AnimationPathCreator(&viewer);
   viewer.getEventHandlerList().push_back(animation_path_handler);

   viewer.getUsage(*arguments.getApplicationUsage());

   int count = 0;
   while( !viewer.done() )
   {
      // Wait for all cull and draw threads to complete:
      viewer.sync();

      // Update the scene by traversing it with the the update visitor which
      // will call all node update callbacks and animations:
      viewer.update();

      // Fire off the cull and draw traversals of the scene:
      viewer.frame();
      count++; 
   }

// Wait for all cull and draw threads to complete before exiting:

   viewer.sync();

    // Delete dymamically allocated objects:

   exit(0);
}

