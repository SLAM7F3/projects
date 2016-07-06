// ==========================================================================
// Program DISPLAY_TABLE is a testing ground for displaying 2D tables
// of string labels.

//			       display_table

// ==========================================================================
// Last updated on 9/9/15
// ==========================================================================

#include  <fstream>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"

#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(4);

   nrfunc::init_time_based_seed();

   int n_rows = 5;
   int n_columns = 7;
   TABLE_LABELS_MAP table_labels_map;

   pair<int,int> P;
   vector<string> labels;
   string curr_label;
   
   for(int r = 0; r < n_rows; r++)
   {
      for(int c = 0; c < n_columns; c++)
      {
         if(nrfunc::ran1() > 0.75) continue;
         P.first = r;
         P.second = c;
         labels.clear();

         curr_label = "row = "+stringfunc::number_to_string(r);
         labels.push_back(curr_label);
         curr_label = "col = "+stringfunc::number_to_string(c);
         labels.push_back(curr_label);
         table_labels_map[P] = labels;
      }
   }
   
// ---------------------------------------------------------
// Display table using our OSG GraphNodesGroup class:

   const int ndims=3;

// Construct the viewer and instantiate ViewerManager:

   WindowManager* window_mgr_scenegraph_ptr=new ViewerManager();
   window_mgr_scenegraph_ptr->initialize_window("Table");
   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_scenegraph_ptr);
   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::MANIPULATE_GRAPHNODE);

// Add custom manipulators:

   bool disable_rotations_flag=true;
   osgGA::Custom3DManipulator* CM_scenegraph_ptr=
      new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_scenegraph_ptr,disable_rotations_flag);
   window_mgr_scenegraph_ptr->set_CameraManipulator(CM_scenegraph_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_scenegraph_ptr,ModeController_ptr,CM_scenegraph_ptr);

// Insert scenegraph display's grid:

   double min_X=0;
   double max_X=10;
   double min_Y=0;
   double max_Y=10;
   double min_Z=0;
   bool world_origin_precisely_in_lower_left_corner=true;

   AlirtGrid* scenegraph_grid_ptr=decorations.add_AlirtGrid(
      ndims, NULL, min_X, max_X, min_Y, max_Y, min_Z,
      world_origin_precisely_in_lower_left_corner);
   threevector* scenegraph_grid_origin_ptr=
      scenegraph_grid_ptr->get_world_origin_ptr();

   scenegraph_grid_ptr->set_axes_labels("X","Y");
   scenegraph_grid_ptr->set_delta_xy(2,2);
   scenegraph_grid_ptr->set_axis_char_label_size(1);
   scenegraph_grid_ptr->set_tick_char_label_size(1);
   scenegraph_grid_ptr->update_grid();

// Instantiate a GraphNodes group:

   GraphNodesGroup graphnodes_group(NULL,scenegraph_grid_origin_ptr);
   graphnodes_group.set_display_scenegraph_flag(false);

   root->addChild(graphnodes_group.createBoxLight(threevector(20,10,10)));
   root->addChild(graphnodes_group.get_OSGgroup_ptr());

// Instantiate a GraphNodesKeyHandler for debugging purposes:

   window_mgr_scenegraph_ptr->get_EventHandlers_ptr()->push_back(
      new GraphNodesKeyHandler(&graphnodes_group,ModeController_ptr));

   graphnodes_group.set_table_labels_map_ptr(&table_labels_map);
   graphnodes_group.generate_Graph_from_table(n_rows, n_columns);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   window_mgr_scenegraph_ptr->setSceneData(root);

// Create the windows and run the threads:

   window_mgr_scenegraph_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_scenegraph_ptr->set_eye_to_center_distance(
      basic_math::max(
         scenegraph_grid_ptr->get_xsize(),scenegraph_grid_ptr->get_ysize()));
   CM_scenegraph_ptr->update_M_and_Minv();

   while( !window_mgr_scenegraph_ptr->done() )
   {
      window_mgr_scenegraph_ptr->process();
   }

   delete window_mgr_scenegraph_ptr;
}

