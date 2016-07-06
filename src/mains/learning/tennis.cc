// ==========================================================================
// Program TENNIS generates a decision tree for data provided in table
// 3.2 of DecisionTreeLearning.pdf

//			       tennis

// ==========================================================================
// Last updated on 8/30/15; 9/1/15; 9/4/15; 9/7/15
// ==========================================================================

#include  <fstream>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "classification/decision_tree.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"

#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgSceneGraph/TreeVisitor.h"
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

   int n_features = 4;
   int n_data_examples = 0;
   decision_tree dtree(n_features);

//   double split_stopping_threshold = 0.05;
   double split_stopping_threshold = 1.1;
//   cout << "Enter split stopping threshold" << endl;
//   cin >> split_stopping_threshold;
   dtree.set_split_stopping_threshold(split_stopping_threshold);

   dtree.append_feature_label("Outlook");
   dtree.append_feature_label("Temperature");
   dtree.append_feature_label("Humidity");
   dtree.append_feature_label("Wind");

   vector<data_example> training_examples;

   string input_filename="./data/tennis.dat";
   filefunc::ReadInfile(input_filename);
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      string curr_line = filefunc::text_line[i];
//      cout << curr_line << endl;
     
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line);

      data_example curr_data_example(n_data_examples);
      
      for(unsigned int s = 1; s < substrings.size() - 1; s++)
      {
//         cout << "s = " << s << " substring = " << substrings[s] << endl;
         curr_data_example.append_feature_value(substrings[s]);
      }
      curr_data_example.set_classification_value(substrings.back());

//      cout << "i = " << i << " curr_data_example = " << curr_data_example << endl;
      dtree.append_data_example(curr_data_example);
      n_data_examples++;
   } // loop over index i labeling lines within input data text file

   double training_example_frac = 1.0;
   double validation_example_frac = 0.0;
   dtree.separate_training_validation_test_example_IDs(
      training_example_frac, validation_example_frac);

   dtree.set_n_classes();
   dtree.record_data_example_classification_values();
   dtree.fill_feature_values_map();
   dtree.print_unique_feature_values_map();
   dtree.fill_feature_examples_map();

   cout << "Decision tree = " << endl;
   cout << dtree << endl;
   dtree.print_feature_values();

   dtree.build_tree_graph();
   dtree.print_tree_graph();

// Generate equivalent set of decision rules from decision tree.  Then
// prune rules in order to reduce overfitting:

   dtree.generate_decision_rules();
   dtree.print_decision_rules();

// Evaluate classification performance on training examples:

   dtree.evaluate_classification_performance(0);
   dtree.compute_decision_rule_accuracies(0);
   
   outputfunc::enter_continue_char();

// Print level and column values within *tree_ptr member of decision
// tree which is used to generate OSG display

   Tree< DATA_TYPE >* tree_ptr = dtree.get_tree_ptr();
   cout << "---------------------------------------------------" << endl;
   cout << "tree: size() = " << tree_ptr->size() << endl;
   cout << "tree: n_levels() = " << tree_ptr->get_n_levels() << endl;
   for(int l = 0; l <= tree_ptr->get_n_levels(); l++)
   {
      int n_nodes_on_level = tree_ptr->number_nodes_on_level(l);
      tree_ptr->compute_columns_for_nodes_on_level(l);

      for(int c = 0; c < n_nodes_on_level; c++)
      {
         Treenode<DATA_TYPE>* treenode_ptr = tree_ptr->get_node(l,c);
         DATA_TYPE curr_data = treenode_ptr->get_data();
//         cout << "curr_data.size() = " << curr_data.size() << endl;
         for(unsigned int s = 0; s < curr_data.size(); s++)
         {
            cout << "level = " << l << " column = " << c 
                 << " label_" << s << " = " << curr_data[s] << endl;
         }
      } // loop over index c labeling columns on level l
   } // loop over index l labeling tree levels
 
// ---------------------------------------------------------
// Display decision tree using our OSG GraphNodesGroup
// class:

   const int ndims=3;

// Construct the viewer and instantiate ViewerManager:

   WindowManager* window_mgr_scenegraph_ptr=new ViewerManager();
   window_mgr_scenegraph_ptr->initialize_window("Decision tree");
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

   TreeVisitor *tree_visitor_ptr = dtree.get_tree_visitor_ptr();

   tree_visitor_ptr->set_DataNode_ptr(root);

// Instantiate a GraphNodes group:

   GraphNodesGroup graphnodes_group(
      NULL,scenegraph_grid_origin_ptr, tree_visitor_ptr);
   graphnodes_group.set_display_scenegraph_flag(false);

   root->addChild(graphnodes_group.createBoxLight(threevector(20,10,10)));
   root->addChild(graphnodes_group.get_OSGgroup_ptr());

// Instantiate a GraphNodesKeyHandler for debugging purposes:

   window_mgr_scenegraph_ptr->get_EventHandlers_ptr()->push_back(
      new GraphNodesKeyHandler(&graphnodes_group,ModeController_ptr));

   graphnodes_group.generate_Graph_from_tree();

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

