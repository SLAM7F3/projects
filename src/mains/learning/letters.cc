// ==========================================================================
// Program LETTERS implements our solution to Decision Trees problem
// for 1st assignment of CSE 446 (Spring 2015).  Unfortunately, the
// chisq split stopping criterion definitely does not work as
// expected.  We expect to see classification performance on the test
// set increase as the split stopping threshold is lowered from 1 to
// 0.1 and 0.05.  But we believe that the rest of our decision tree
// approach is basically correct.  

// LETTERS visualizes the final decision tree using our OSG
// GraphNodesGroup class.

//			       letters

// ==========================================================================
// Last updated on 9/1/15; 9/2/15; 9/4/15; 9/7/15; 11/28/15
// ==========================================================================

#include  <fstream>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "classification/decision_tree.h"
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

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

   timefunc::initialize_timeofday_clock();
   nrfunc::init_time_based_seed();
   int n_features = 16;
   decision_tree dtree(n_features);

//   double split_stopping_threshold = 0.05;
   double split_stopping_threshold = 1.1;
//   cout << "Enter split stopping threshold" << endl;
//   cin >> split_stopping_threshold;
   dtree.set_split_stopping_threshold(split_stopping_threshold);

//   dtree.set_split_stopping_threshold(0.05);	// 54.475% correct classification
//   dtree.set_split_stopping_threshold(0.1);	// 54.6625% correct classification
//   dtree.set_split_stopping_threshold(1.0);	// 69.6% correct classification
   for(int n=0; n < n_features; n++)
   {
      dtree.append_feature_label("Feature_"+stringfunc::number_to_string(n));
   }

   string input_filename="./data/letter-recognition.data";
   filefunc::ReadInfile(input_filename);

   int n_data_examples = 0;
   int n_lines = filefunc::text_line.size();
   for(int i = 0; i < n_lines; i++)
   {
      string curr_line = filefunc::text_line[i];
//      cout << curr_line << endl;
     
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line,",");

      data_example curr_data_example(n_data_examples);
      for(unsigned int s = 1; s < substrings.size(); s++)
      {
//         cout << "s = " << s << " substring = " << substrings[s] << endl;
         curr_data_example.append_feature_value(substrings[s]);
      }
      curr_data_example.set_classification_value(substrings.front());

      dtree.append_data_example(curr_data_example);
      n_data_examples++;

   } // loop over index i labeling lines within input data text file

   double training_example_frac = 13.0/20.0;
   double validation_example_frac = 3.0/20.0;
//   double training_example_frac = 12.0/20.0;
//   double validation_example_frac = 0.0;
   dtree.separate_training_validation_test_example_IDs(
      training_example_frac, validation_example_frac);

   dtree.set_n_classes();
   dtree.record_data_example_classification_values();
   dtree.fill_feature_values_map();
   dtree.fill_feature_examples_map();

//   cout << "Decision tree = " << endl;
//   cout << dtree << endl;
//   dtree.print_feature_values();

   dtree.build_tree_graph();
//   dtree.print_tree_graph();
   outputfunc::print_elapsed_time();

// Generate equivalent set of decision rules from decision tree:

   dtree.generate_decision_rules();
//   dtree.print_decision_rules();
   outputfunc::print_elapsed_time();

// Evaluate classification performance on training examples:

   dtree.evaluate_classification_performance(0);
   double avg_training_rule_accuracy = 
      dtree.compute_decision_rule_accuracies(0);
   cout << "Average training rule accuracy = " << avg_training_rule_accuracy
        << endl;
   outputfunc::print_elapsed_time();

// Prune decision rules using validation examples:

   cout << endl;
   cout << "========================================================" << endl;
   cout << "Pruning decision rules using validation examples:"
        << endl;

   dtree.evaluate_classification_performance(1);
   double avg_validation_rule_accuracy = 
      dtree.compute_decision_rule_accuracies(1);
   cout << "Before pruning, average validation rule accuracy = "
        << avg_validation_rule_accuracy << endl;
   dtree.prune_decision_rules();
   bool sort_rules_by_accuracy_flag = true;
   avg_validation_rule_accuracy = 
      dtree.compute_decision_rule_accuracies(1,sort_rules_by_accuracy_flag);
   cout << "After pruning, average validation rule accuracy = "
        << avg_validation_rule_accuracy << endl;
   outputfunc::print_elapsed_time();

// Evaluate classification performance on test examples:

   dtree.evaluate_classification_performance(2);
   double avg_test_rule_accuracy = dtree.compute_decision_rule_accuracies(2);
   cout << "Average test rule accuracy = " << avg_test_rule_accuracy
        << endl;
   outputfunc::print_elapsed_time();

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

