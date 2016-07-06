// ==========================================================================
// Header file for decision_tree class 
// ==========================================================================
// Last modified on 8/30/15; 9/2/15; 9/3/15; 9/7/15
// ==========================================================================

#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "classification/dtree_node_data.h"
#include "classification/data_example.h"
#include "graphs/graph.h"
#include "osg/osgSceneGraph/TreeVisitor.h"

class decision_tree
{
   
  public:

   typedef std::map<int, std::vector<std::string> > FEATURE_VALUES_MAP;
// independent int var: feature ID
// dependent vector<string>: feature values for each training example

   typedef std::map<std::pair<int, std::string>, std::vector<int> > 
      FEATURE_EXAMPLES_MAP;
// independent int var: (feature_ID, feature value)
// dependent vector<int>: IDs for all training examples which have the
// specified feature value for feature_ID

   typedef std::map<int, dtree_node_data* > NODE_DATA_MAP;
// independent int var: node ID
// dependent dtree_node_data: data for decision tree node

   typedef std::map<int, std::vector<int> > TREE_BRANCH_MAP;
// independent int var: decision rule ID
// dependent vector<int>: decision rule node IDs

   typedef std::map<std::string, std::string > FEATURE_LABELS_VALUES_MAP;
// independent string var: feature label
// dependent string var: feature value

   typedef std::map<int, std::pair<FEATURE_LABELS_VALUES_MAP*, std::string> >
      DECISION_RULES_MAP;
// independent int var: decision rule id
// dependent pair var contains map holding feature labels vs values 
//      and string containing rule's decision

// Initialization, constructor and destructor functions:

   decision_tree(int n);
   decision_tree(const decision_tree& C);
   ~decision_tree();
//   decision_tree operator= (const decision_tree& C);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const decision_tree& C);

// Set and get member functions

   std::vector<int>& get_training_example_IDs();
   const std::vector<int>& get_training_example_IDs() const;
   std::vector<int>& get_validation_example_IDs();
   const std::vector<int>& get_validation_example_IDs() const;
   std::vector<int>& get_test_example_IDs();
   const std::vector<int>& get_test_example_IDs() const;
   void set_split_stopping_threshold(double threshold);


   void append_feature_label(std::string label);
   data_example& get_data_example(int data_example_ID);
   const data_example& get_data_example(int data_example_ID) const;
   Tree< DATA_TYPE >* get_tree_ptr();
   const Tree< DATA_TYPE >* get_tree_ptr() const;
   TreeVisitor* get_tree_visitor_ptr();

   void append_data_example(data_example& curr_data_example);
   void separate_training_validation_test_example_IDs(
      double training_example_frac, double validation_example_frac);
   void print_unique_feature_values_map();
   void fill_feature_examples_map();

   void compute_classification_value_multiplicities(
      const std::vector<int>& curr_data_example_IDs,
      std::vector<int>& classification_value_multiplicities);
   void compute_classification_value_probs(
      const std::vector<int>& curr_data_example_IDs,
      std::vector<double>& classification_value_probs);
   double compute_entropy(const std::vector<int>& curr_data_example_IDs);

   void set_n_classes();
   void record_data_example_classification_values();
   std::string determine_most_common_classification_value(
      const std::vector<int>& curr_data_example_IDs);

   void fill_feature_values_map();
   void print_feature_values();

   void build_tree_graph();
   void print_tree_graph();

   void generate_decision_rules();
   void print_decision_rules();
   std::string classify_example_via_decision_rules(
      const data_example& curr_data_example, int& classifying_rule_ID);
   double compute_decision_rule_accuracies(
      int data_examples_type, bool sort_rules_by_accuracy_flag=false);

   std::string classify_example_via_decision_tree(
      const data_example& curr_data_example);
   std::string recursively_find_leaf_classification(
      node *curr_node, const data_example& curr_data_example);

   void prune_decision_rules();
   bool potentially_prune_decision_rule(
      int data_examples_type, 
      DECISION_RULES_MAP::iterator& decision_rules_iter,
      int& n_curr_rule_prunings);

   double evaluate_classification_performance(int data_example_type);

  private: 

   int n_features;
   int n_classes; // Number unique classification values 
		  //   among all training examples
   double split_stopping_threshold;

   std::vector<std::string> feature_labels;
   std::vector<data_example> data_examples;
   std::vector<std::string> classification_values; // classification value
						   // for each training example
   std::vector<int> training_example_IDs;
   std::vector<int> validation_example_IDs;
   std::vector<int> test_example_IDs;

   FEATURE_VALUES_MAP* feature_values_map_ptr;
   FEATURE_VALUES_MAP* unique_feature_values_map_ptr;
   FEATURE_EXAMPLES_MAP* feature_examples_map_ptr;

   graph tree_graph;

   std::vector<std::string> unique_classification_values;
   std::vector<double> classification_value_probs;

   NODE_DATA_MAP* node_data_map_ptr;
   NODE_DATA_MAP::iterator node_data_iter;

   TREE_BRANCH_MAP* tree_branch_map_ptr;
   DECISION_RULES_MAP* decision_rules_map_ptr;
   std::vector<int> rule_IDs;
   std::vector<double> rule_accuracies;

   TreeVisitor tree_visitor;
   Tree< DATA_TYPE >* tree_ptr;

   int get_feature_label_ID(std::string feature_label);
   int get_feature_value_ID(int feature_ID, std::string feature_value);
   int get_classification_value_ID(std::string classification_value);

   void retrieve_data_example_IDs_for_feature_ID_and_value(
      int feature_ID, std::string feature_value, 
      const std::vector<int>& all_data_example_IDs,
      std::vector<int>& subset_data_example_IDs);
   void get_data_example_IDs_for_feature_value(
      int feature_ID, std::string feature_value, 
      const std::vector<int>& curr_data_example_IDs,
      std::vector<int>& subset_data_example_IDs);
   void get_data_example_IDs_for_classification_value(
      std::string classification_value, 
      const std::vector<int>& curr_data_example_IDs,
      std::vector<int>& subset_data_example_IDs);

   void count_unique_classification_value_multiplicities(
      int feature_ID, std::string feature_value, 
      std::vector<int>& unique_classification_value_multiplicities);

   double compute_information_gain(int feature_ID, const std::vector<int>&
                                   curr_data_example_IDs);
   double compute_chisq_Pvalue(
      int feature_ID, const std::vector<int>& curr_data_example_IDs);

   node* generate_tree_node(
      int node_ID, int parent_ID, const std::vector<int> training_example_IDs,
      const std::vector<int> active_feature_IDs);
   node* initialize_root_node();
   void split_node_or_identify_as_leaf(
      node *curr_node, Treenode<DATA_TYPE>* curr_treenode_ptr,
      std::vector<int>& input_data_example_IDs, 
      std::vector<int>& input_active_feature_IDs);
   bool potentially_set_leaf_node_classification(
      node *curr_node, Treenode<DATA_TYPE>* curr_treenode_ptr,
      std::vector<int>& curr_data_example_IDs, int n_active_feature_IDs);
   void set_leaf_node_classification(
      node *curr_node, Treenode<DATA_TYPE>* curr_treenode_ptr,
      std::string leaf_classification);

   void generate_tree_branch(node *curr_node);
   void tabulate_node_children();

   bool rule_consistent_with_example(
      const data_example& curr_data_example,
      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr);
   int decision_rule_vs_example_decision(
      const data_example& curr_data_example,
      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr,
      std::string rule_decision);
   double decision_rule_accuracy(
      int data_examples_type, 
      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr, 
      std::string rule_decision);
   std::string data_example_type_label(int data_example_type);

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const decision_tree& C);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline std::vector<int>& decision_tree::get_training_example_IDs() 
{
   return training_example_IDs;
}

inline const std::vector<int>& decision_tree::get_training_example_IDs() const
{
   return training_example_IDs;
}

inline std::vector<int>& decision_tree::get_validation_example_IDs() 
{
   return validation_example_IDs;
}

inline const std::vector<int>& decision_tree::get_validation_example_IDs() const
{
   return validation_example_IDs;
}

inline std::vector<int>& decision_tree::get_test_example_IDs() 
{
   return test_example_IDs;
}

inline const std::vector<int>& decision_tree::get_test_example_IDs() const
{
   return test_example_IDs;
}

inline void decision_tree::set_split_stopping_threshold(double threshold)
{
   split_stopping_threshold = threshold;
}

inline void decision_tree::append_feature_label(std::string label)
{
   feature_labels.push_back(label);
}

inline data_example& decision_tree::get_data_example(
   int data_example_ID)
{
   return data_examples[data_example_ID];
}

inline const data_example& decision_tree::get_data_example(
   int data_example_ID) const
{
   return data_examples[data_example_ID];
}

inline Tree< DATA_TYPE >* decision_tree::get_tree_ptr()
{
   return tree_ptr;
}

inline const Tree< DATA_TYPE >* decision_tree::get_tree_ptr() const
{
   return tree_ptr;
}

inline TreeVisitor* decision_tree::get_tree_visitor_ptr()
{
   return &tree_visitor;
}

#endif  // decision_tree.h


