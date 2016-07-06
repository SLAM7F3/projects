// ==========================================================================
// decision_tree class member function definitions
// ==========================================================================
// Last modified on 9/2/15; 9/3/15; 9/4/15; 9/7/15
// ==========================================================================

#include <iostream>
#include <stdlib.h>
#include "classification/decision_tree.h"
#include "templates/mytemplates.h"
#include "graphs/node.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void decision_tree::allocate_member_objects()
{
   feature_values_map_ptr = new FEATURE_VALUES_MAP;
   unique_feature_values_map_ptr = new FEATURE_VALUES_MAP;
   feature_examples_map_ptr = new FEATURE_EXAMPLES_MAP;
   node_data_map_ptr = new NODE_DATA_MAP;
   tree_branch_map_ptr = new TREE_BRANCH_MAP;
   decision_rules_map_ptr = new DECISION_RULES_MAP;
}		       

void decision_tree::initialize_member_objects()
{
   split_stopping_threshold = 1.0;
   tree_ptr = tree_visitor.get_tree_ptr();
}		       

decision_tree::decision_tree(int n)
{
   allocate_member_objects();
   initialize_member_objects();

   n_features = n;
}

// Copy constructor:

decision_tree::decision_tree(const decision_tree& dtree)
{
//   docopy(dtree);
}

// ---------------------------------------------------------------------
decision_tree::~decision_tree()
{
   delete feature_values_map_ptr;
   delete unique_feature_values_map_ptr;
   delete feature_examples_map_ptr;
   delete node_data_map_ptr;
   delete tree_branch_map_ptr;

// We should traverse through decision_rules_map_ptr and delete each
// of its FEATURE_LABELS_VALUES_MAPs before deleting
// decision_rules_map_ptr itself!

   delete decision_rules_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const decision_tree& dtree)
{
   outstream << endl;
   outstream << "n_training_examples = " << dtree.training_example_IDs.size()
             << endl;
   outstream << "n_validation_examples = " 
             << dtree.validation_example_IDs.size() << endl;
   outstream << "n_testing_examples = " << dtree.test_example_IDs.size()
             << endl;
   outstream << "n_classes = " << dtree.n_classes << endl;
   for(unsigned int i = 0; i < dtree.data_examples.size(); i++)
   {
      outstream << dtree.data_examples[i] << endl;
   }

   for(int c = 0; c < dtree.n_classes; c++)
   {
      cout << "c = " << c << " classification_value = " 
           << dtree.unique_classification_values[c] << endl;
   }

   return outstream;
}

// ==========================================================================

void decision_tree::append_data_example(data_example& curr_data_example)
{
//   cout << "inside decision_tree::append_data_example() " << endl;
//   cout << "n_features = " << n_features << endl;
   
   for(int f = 0; f < n_features; f++)
   {
      curr_data_example.append_feature_label(feature_labels[f]);
   }
   data_examples.push_back(curr_data_example);

// Looping over each feature index, add current training example's
// feature value to member *unique_feature_values_map_ptr if it
// doesn't already exist within this STL map:

   for(int f = 0; f < n_features; f++)
   {
      string curr_feature_value = curr_data_example.get_feature_value(f);
      FEATURE_VALUES_MAP::iterator feature_values_iter = 
         unique_feature_values_map_ptr->find(f);
      if(feature_values_iter == unique_feature_values_map_ptr->end())
      {
         vector<string> unique_feature_values;
         unique_feature_values.push_back(curr_feature_value);
         (*unique_feature_values_map_ptr)[f] = unique_feature_values;
      }
      else
      {
         vector<string> *unique_feature_values_ptr = 
            &feature_values_iter->second;
         bool already_exists = false;
         for(unsigned int v = 0; v < unique_feature_values_ptr->size(); v++)
         {
            if(curr_feature_value == unique_feature_values_ptr->at(v))
            {
               already_exists = true;
               break;
            }
         }
         if(!already_exists)
         {
            unique_feature_values_ptr->push_back(curr_feature_value);
         }
      }
   } // loop over index f labeling feature IDs

// Add current training example's classification to member
// unique_classification_values if it doesn't already exist within
// this STL vector:

   bool curr_classification_already_exists = false;
   string curr_classification_value = curr_data_example.
      get_classification_value();

   for(unsigned int c = 0 ; c < unique_classification_values.size(); c++)
   {
      if(curr_classification_value == unique_classification_values[c])
      {
         curr_classification_already_exists = true;
         break;
      }
   }

   if(!curr_classification_already_exists)
   {
      unique_classification_values.push_back(curr_classification_value);
   }
}

// ---------------------------------------------------------------------
void decision_tree::separate_training_validation_test_example_IDs(
   double training_example_frac, double validation_example_frac)
{
   cout << "inside decision_tree::separate_training_validation_test_example_IDs()" << endl;

   for(unsigned int i = 0 ; i < data_examples.size(); i++)
   {
      double curr_frac = nrfunc::ran1();
      if(curr_frac < training_example_frac)
      {
         training_example_IDs.push_back(i);
      }
      else if (curr_frac < training_example_frac + validation_example_frac)
      {
         validation_example_IDs.push_back(i);
      }
      else
      {
         test_example_IDs.push_back(i);
      }
   }
   cout << "training_example_IDs.size() = " << training_example_IDs.size()
        << endl;
   cout << "validation_example_IDs.size() = " << validation_example_IDs.size()
        << endl;
   cout << "test_example_IDs.size() = " << test_example_IDs.size()
        << endl;
}


// ---------------------------------------------------------------------
void decision_tree::print_unique_feature_values_map()
{
   cout << "inside decision_tree::print_unique_feature_values_map()"
        << endl;
   cout << "unique_feature_values_map_ptr->size() = "
        << unique_feature_values_map_ptr->size() << endl;

   for(FEATURE_VALUES_MAP::iterator feature_values_iter = 
          unique_feature_values_map_ptr->begin();
       feature_values_iter != unique_feature_values_map_ptr->end();
       feature_values_iter++)
   {
      int feature_ID = feature_values_iter->first;
      vector<string> *unique_feature_values_ptr = &feature_values_iter->second;
      cout << "feature_ID = " << feature_ID 
           << " feature_label = " << feature_labels[feature_ID]
           << endl;
      for(unsigned int v = 0; v < unique_feature_values_ptr->size(); v++)
      {
         cout << " v = " << v << " unique_feature_value = "
              << unique_feature_values_ptr->at(v) << endl;
      }
   }
}

// ---------------------------------------------------------------------
// Member function fill_feature_examples_map() loops over all training
// examples.  It fills *feature_examples_map_ptr with (feature_ID,
// feature_value) keys and vector<int> values holding corresponding
// training example IDs.

void decision_tree::fill_feature_examples_map()
{
//   cout << "inside fill_feature_examples_map() " << endl;

   vector<int> all_data_example_IDs;
   for(unsigned int i = 0; i < data_examples.size(); i++)
   {
      all_data_example_IDs.push_back(data_examples[i].get_ID());
   }

   for(FEATURE_VALUES_MAP::iterator feature_values_iter=
          unique_feature_values_map_ptr->begin();
       feature_values_iter != unique_feature_values_map_ptr->end();
       feature_values_iter++)
   {
      int feature_ID = feature_values_iter->first;
//      cout << "feature_ID = " << feature_ID << endl;

      vector<string> unique_feature_values = feature_values_iter->second;
      for(unsigned int v = 0; v < unique_feature_values.size(); v++)
      {
         vector<int> subset_data_example_IDs;

//         cout << "v = " << v 
//              << " unique_feature_values[v] = "
//              << unique_feature_values[v] << endl;

         retrieve_data_example_IDs_for_feature_ID_and_value(
            feature_ID, unique_feature_values[v], 
            all_data_example_IDs, subset_data_example_IDs);
//         cout << "subset_data_example_IDs.size() = "
//              << subset_data_example_IDs.size() << endl;

         pair<int, string> P;
         P.first = feature_ID;
         P.second = unique_feature_values[v];
//         cout << "P = " << P.first << " , " << P.second << endl;
         
         (*feature_examples_map_ptr)[P] = subset_data_example_IDs;
      } // loop over index v labeling unique feature values for feature_ID
   } // loop over feature_values_iter

//   cout << "feature_examples_map.size() = "
//        << feature_examples_map_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function compute_classification_value_multiplicities()
// counts the number of input training examples which correspond to
// each possible classification value.  

void decision_tree::compute_classification_value_multiplicities(
   const vector<int>& curr_data_example_IDs,
   vector<int>& classification_value_multiplicities)
{
   classification_value_multiplicities.clear();
   for(int c = 0; c < n_classes; c++)
   {
      classification_value_multiplicities.push_back(0);
   }
   
   for(unsigned int t = 0; t < curr_data_example_IDs.size(); t++)
   {
      int curr_data_value_ID = curr_data_example_IDs[t];
      string cvalue = data_examples[curr_data_value_ID].
         get_classification_value();
      for(int c = 0; c < n_classes; c++)
      {
         if(cvalue == unique_classification_values[c])
         {
            classification_value_multiplicities[c] = 
               classification_value_multiplicities[c] + 1;
            break;
         }
      }
   } // loop over index t labeling training examples
}

// ---------------------------------------------------------------------
// Member function compute_classification_value_probs() convert
// classification value multiplicities into n_classes probabilities.

void decision_tree::compute_classification_value_probs(
   const vector<int>& curr_data_example_IDs,
   vector<double>& classification_value_probs)
{
   classification_value_probs.clear();

   vector<int> classification_value_multiplicities;
   compute_classification_value_multiplicities(
      curr_data_example_IDs, classification_value_multiplicities);

// Convert classification value multiplicities into probabilities.

   for(int c = 0; c < n_classes; c++)
   {
      double curr_prob = double(classification_value_multiplicities[c]) / 
         curr_data_example_IDs.size();
      classification_value_probs.push_back(curr_prob);
   }
}

// ---------------------------------------------------------------------
double decision_tree::compute_entropy(const vector<int>& curr_data_example_IDs)
{
   vector<double> classification_value_probs;
   compute_classification_value_probs(
      curr_data_example_IDs, classification_value_probs);

   double S = 0;
   const double TINY=1E-10;
   for(int c = 0; c < n_classes; c++)
   {
      double curr_prob = classification_value_probs[c];
      if(curr_prob > TINY)
      {
         S += -curr_prob * log(curr_prob) / log(2.0);
      }
   }

//   cout << "Entropy = " << S << endl;
   return S;
}

// ---------------------------------------------------------------------
// Given a feature label, get_feature_label() returns its
// corresponding feature ID.  If input feature label is invalid, this
// method returns -1.

int decision_tree::get_feature_label_ID(string feature_label)
{
   for(unsigned int i = 0; i < feature_labels.size(); i++)
   {
      if(feature_label == feature_labels[i])
      {
         return i;
      }
   }

   return -1;
}

// ---------------------------------------------------------------------
// Given a feature ID and a corresponding unique feature value,
// get_feature_value_ID() returns its corresponding unique feature
// value ID.  If input label is invalid, this method returns -1.

int decision_tree::get_feature_value_ID(int feature_ID, string feature_value)
{
   FEATURE_VALUES_MAP::iterator feature_values_iter = 
      unique_feature_values_map_ptr->find(feature_ID);
   vector<string> unique_feature_values = feature_values_iter->second;

   for(unsigned int i = 0; i < unique_feature_values.size(); i++)
   {
      if(feature_value == unique_feature_values[i])
      {
         return i;
      }
   }

   return -1;
}

// ---------------------------------------------------------------------
// Given a unique classification value, get_classifcation_value_ID()
// returns its corresponding unique classification value ID.  If input
// label is invalid, this method returns -1.

int decision_tree::get_classification_value_ID(string classification_value)
{
   for(int c = 0; c < n_classes; c++)
   {
      if(classification_value == unique_classification_values[c])
      {
         return c;
      }
   }

   return -1;
}

// ---------------------------------------------------------------------
// Member function retrieve_data_example_IDs_for_feature_value() takes
// in a feature ID, particular feature value and the set of all
// data example IDs.  It performs a brute-force search and returns
// a vector<int> containing the subset of data example IDs that
// correspond to the specified feature value.

void decision_tree::retrieve_data_example_IDs_for_feature_ID_and_value(
   int feature_ID, string feature_value, 
   const vector<int>& all_data_example_IDs,
   vector<int>& subset_data_example_IDs)
{
   subset_data_example_IDs.clear();

   FEATURE_VALUES_MAP::iterator feature_values_iter = 
      feature_values_map_ptr->find(feature_ID);

   vector<string> all_feature_values = feature_values_iter->second;
   for(unsigned int i = 0; i < all_data_example_IDs.size(); i++)
   {
      int t = all_data_example_IDs[i];
      if(feature_value == all_feature_values[t])
      {
         subset_data_example_IDs.push_back(t);
      }
   }
}

// ---------------------------------------------------------------------
// Member function get_data_example_IDs_for_feature_value() takes
// in a feature ID, particular feature value and some set of training
// example IDs.  It utilizes STL set_intersection in order to quickly
// compute the vector<int> containing the subset of training example
// IDs that correspond to the specified feature value.

void decision_tree::get_data_example_IDs_for_feature_value(
   int feature_ID, string feature_value, 
   const vector<int>& curr_data_example_IDs,
   vector<int>& subset_data_example_IDs)
{
   subset_data_example_IDs.clear();
   
   pair<int, string> P;
   P.first = feature_ID;
   P.second = feature_value;

   FEATURE_EXAMPLES_MAP::iterator feature_examples_iter = 
      feature_examples_map_ptr->find(P);
   vector<int> all_data_example_IDs = feature_examples_iter->second;

// Set output subset_data_example_IDs = intersection between
// all_data_example_IDs and input curr_data_example_IDs:

   set_intersection(
      all_data_example_IDs.begin(), all_data_example_IDs.end(), 
      curr_data_example_IDs.begin(), curr_data_example_IDs.end(),
      back_inserter(subset_data_example_IDs));
}

// ---------------------------------------------------------------------
// Member function get_data_example_IDs_for_classification_value()
// takes in a classification value and input set of training example
// IDs.  It returns a vector<int> containing the subset of training
// example IDs that correspond to the specified classification value.

void decision_tree::get_data_example_IDs_for_classification_value(
   string classification_value, const vector<int>& curr_data_example_IDs,
   vector<int>& subset_data_example_IDs)
{
   subset_data_example_IDs.clear();

   for(unsigned int i = 0; i < curr_data_example_IDs.size(); i++)
   {
      int t = curr_data_example_IDs[i];
      data_example curr_data_example = data_examples[t];
      if(classification_value == 
         curr_data_example.get_classification_value())
      {
         subset_data_example_IDs.push_back(t);
      }
   }
}

// ---------------------------------------------------------------------
// Given a feature ID and particular feature value, return vector<int>
// containing classification value multiplicities.

void decision_tree::count_unique_classification_value_multiplicities(
   int feature_ID, string feature_value, 
   vector<int>& unique_classification_value_multiplicities)
{
//   cout << "inside count_unique_classification_value_mults() " << endl;

   FEATURE_VALUES_MAP::iterator feature_values_iter = 
      feature_values_map_ptr->find(feature_ID);
   vector<string> all_feature_values = feature_values_iter->second;

   unique_classification_value_multiplicities.clear();
   for(int c = 0; c < n_classes; c++)
   {
      unique_classification_value_multiplicities.push_back(0);
   }

   for(unsigned int v = 0; v < all_feature_values.size(); v++){
      if(feature_value != all_feature_values[v]) continue;
      int u = get_classification_value_ID(classification_values[v]);
      unique_classification_value_multiplicities[u] = 
         unique_classification_value_multiplicities[u] + 1;
   }
}

// ---------------------------------------------------------------------
// Given a feature ID and a set of training value IDs, compute
// information gain for the specified feature.

double decision_tree::compute_information_gain(
   int feature_ID, const vector<int>& curr_data_example_IDs)
{

// First compute entropy of entire set of current data examples:

   double S = compute_entropy(curr_data_example_IDs);

   FEATURE_VALUES_MAP::iterator feature_values_iter = 
      unique_feature_values_map_ptr->find(feature_ID);
   vector<string> unique_feature_values = feature_values_iter->second;

   double sum = 0;
   for(unsigned int v = 0; v < unique_feature_values.size(); v++)
   {
      vector<int> subset_data_example_IDs;
      get_data_example_IDs_for_feature_value(
         feature_ID, unique_feature_values[v], 
         curr_data_example_IDs, subset_data_example_IDs);
      double S_v = compute_entropy(subset_data_example_IDs);

//      cout << "v = " << v 
//           << " |curr_data_example_IDs| = " << curr_data_example_IDs.size()
//           << " |subset_data_example_IDs| = " 
//           << subset_data_example_IDs.size()
//           << " S_v = " << S_v 
//           << endl;
      sum += subset_data_example_IDs.size() * S_v / 
         curr_data_example_IDs.size();
   }
   double gain = S - sum;
   return gain;
}

// ---------------------------------------------------------------------
// Member function compute_chisq_Pvalue() computes P-value = Pr(true
// chisq >= observed chisq) . If the returned P-value is smaller than
// split_stopping_threshold, it is likely that the tested feature is
// not irrelevant.

double decision_tree::compute_chisq_Pvalue(
   int feature_ID, const vector<int>& curr_data_example_IDs)
{
//   cout << "inside compute_chisq_Pvalue() " << endl;
//   cout << "feature_ID = " << feature_ID << endl;
   
   FEATURE_VALUES_MAP::iterator feature_values_iter = 
      unique_feature_values_map_ptr->find(feature_ID);
   vector<string> unique_feature_values = feature_values_iter->second;

   vector<double> classification_value_probs;
   compute_classification_value_probs(
      curr_data_example_IDs, classification_value_probs);

   double chisq_sum = 0;
   for(unsigned int v = 0; v < unique_feature_values.size(); v++)
   {
      vector<int> subset_data_example_IDs;
      get_data_example_IDs_for_feature_value(
         feature_ID, unique_feature_values[v], 
         curr_data_example_IDs, subset_data_example_IDs);

      for(int c = 0; c < n_classes; c++)
      {
         vector<int> subsubset_data_example_IDs;
         get_data_example_IDs_for_classification_value(
            unique_classification_values[c], subset_data_example_IDs,
            subsubset_data_example_IDs);

// Recall	chisq = Sum (observed - expected)**2 / expected

         double observed_count = subsubset_data_example_IDs.size();
         double expected_count = subset_data_example_IDs.size() *
            classification_value_probs[c];
         if(expected_count > 0)
            chisq_sum += sqr(observed_count - expected_count) / expected_count;
//         cout << "v=" << v << "  c=" << c
//              << "  observed=" << observed_count
//              << "  expected=" << expected_count
//              << "  chisq_sum=" << chisq_sum 
//              << endl;

      } // loop over index c labeling classes
   } // loop over index v labeling unique feature values

   int n_dof = (unique_feature_values.size() - 1 ) * (n_classes - 1);
//   cout << "chisq_sum = " << chisq_sum << " n_dof = " << n_dof 
//        << " ratio = " << chisq_sum / n_dof << endl;

   double p_value = 1 - mathfunc::cumulative_chisq_prob(chisq_sum, n_dof);
//   cout << "p_value = " << p_value << endl;
   return p_value;
}

// ---------------------------------------------------------------------
// Member function set_n_classes()

void decision_tree::set_n_classes()
{
   n_classes = unique_classification_values.size();
}
   
// ---------------------------------------------------------------------
// Member function record_data_example_classification_values

void decision_tree::record_data_example_classification_values()
{
   for(unsigned int i = 0; i < data_examples.size(); i++)
   {
      int curr_data_value_ID = data_examples[i].get_ID();
      string cvalue = data_examples[curr_data_value_ID].
         get_classification_value();
      classification_values.push_back(cvalue);
   }
}

// ---------------------------------------------------------------------
// Member function determine_most_common_classification_value()
// recovers input data example multiplicities for each possible
// classification value.  It returns the classification value with the
// greatest multiplicity.  If the number of input training examples ==
// 0, this method returns a random classification value.

string decision_tree::determine_most_common_classification_value(
   const vector<int>& curr_data_example_IDs)
{
   string most_common_classification_value="";
   vector<int> classification_value_IDs;

   if(curr_data_example_IDs.size() == 0)
   {
      int random_c = nrfunc::ran1() * n_classes;
      most_common_classification_value = 
         unique_classification_values[random_c];
   }
   else
   {
      for(int c = 0; c < n_classes; c++)
      {
         classification_value_IDs.push_back(c);
      }
   
      vector<int> classification_value_multiplicities;
      compute_classification_value_multiplicities(
         curr_data_example_IDs, classification_value_multiplicities);
      
      templatefunc::Quicksort_descending(classification_value_multiplicities,
                                         classification_value_IDs);
      most_common_classification_value = 
         unique_classification_values[classification_value_IDs.front()];
   }

//   cout << "most_common_classification_value = "
//        << most_common_classification_value << endl;
   return most_common_classification_value;
}

// ---------------------------------------------------------------------
// Member function fill_feature_values_map() assigns an STL vector containing 
// feature value strings to each feature labeled by integer feature ID keys.

void decision_tree::fill_feature_values_map()
{
   for(int f = 0; f < n_features; f++)
   {
      vector<string> particular_feature_values;
      for(unsigned int t = 0; t < data_examples.size(); t++)
      {
         string curr_feature_value = data_examples[t].get_feature_value(f);
         particular_feature_values.push_back(curr_feature_value);
      } // loop over index t labeling training examples

      (*feature_values_map_ptr)[f] = particular_feature_values;

   } // loop over index f labeling features
}

// ---------------------------------------------------------------------
void decision_tree::print_feature_values()
{
   vector<int> curr_data_example_IDs;
   for(unsigned int i = 0; i < data_examples.size(); i++)
   {
      curr_data_example_IDs.push_back(data_examples[i].get_ID());
   }

   for(int f = 0; f < n_features; f++)
   {
      cout << "Feature " << f << " : " << feature_labels[f] << endl;

      int feature_ID = get_feature_label_ID(feature_labels[f]);
      FEATURE_VALUES_MAP::iterator feature_values_iter = 
         unique_feature_values_map_ptr->find(f);
      vector<string> unique_feature_values = feature_values_iter->second;

      int multiplicity_sum = 0;
      for(unsigned int v = 0; v < unique_feature_values.size(); v++)
      {
         cout << "v = " << v << endl;
//         int feature_value_ID = get_feature_value_ID(
//            feature_ID, unique_feature_values[v]);
         cout << "unique_feature_values[v] = "
              << unique_feature_values[v] << endl;


         vector<int> subset_data_example_IDs;
         get_data_example_IDs_for_feature_value(
            f, unique_feature_values[v], 
            curr_data_example_IDs, subset_data_example_IDs);

         vector<int> unique_classification_value_multiplicities;
         count_unique_classification_value_multiplicities(
            feature_ID, unique_feature_values[v], 
            unique_classification_value_multiplicities);
         multiplicity_sum += subset_data_example_IDs.size();

         cout << " Unique value " << v << " : " << unique_feature_values[v] 
//              << " feature_ID = " << feature_ID
//              << " feature_value_ID = " << feature_value_ID
//              << " multiplicity = " << unique_feature_value_multiplicities[v]
              << " multiplicity = " << subset_data_example_IDs.size()
              << endl;

/*
  cout << "subset training example IDs : ";
  for(unsigned int t = 0; t < subset_data_example_IDs.size(); t++)
  {
  cout << subset_data_example_IDs[t] << " ";
  }
  cout << endl;

  for(unsigned int m = 0; m < n_classes; m++)
  {
  cout << "   m = " << m 
  << " classification value = " 
  << unique_classification_values[m] 
//                 << " multiplicity = " << curr_data_example_IDs.size()
<< " multiplicity = " 
<< unique_classification_value_multiplicities[m]
<< endl;
}
*/


      }
      cout << "...................." << endl;
      cout << "multiplicity_sum = " << multiplicity_sum << endl;

      feature_values_iter = feature_values_map_ptr->find(f);
      vector<string> curr_feature_values = feature_values_iter->second;
      for(unsigned int v = 0; v < curr_feature_values.size(); v++)
      {
         cout << " Feature value " << v << " : " 
              << curr_feature_values[v] << endl;
      }

      double gain = compute_information_gain(feature_ID,curr_data_example_IDs);
      cout << "Information gain = " << gain << endl;
      cout << "------------------------------------------------" << endl;
   } // loop over index f labeling features
}

// ---------------------------------------------------------------------
void decision_tree::print_tree_graph()
{
   cout << "********************************************************" << endl;
   cout << "Tree graph" << endl;
   for(unsigned int n = 0; n < tree_graph.get_n_nodes(); n++)
   {
      node *curr_node_ptr = tree_graph.get_node_ptr(n);

      cout  << "curr node ID = " << curr_node_ptr->get_ID() 
            << " curr_node level = " << curr_node_ptr->get_level() 
            << endl;
      cout << "curr node's parent ID = " << curr_node_ptr->get_parent_ID() 
           << endl;

      node_data_iter = node_data_map_ptr->find(curr_node_ptr->get_ID());
      dtree_node_data *node_data = node_data_iter->second;
      string prev_decision_feature_value = 
         node_data->get_prev_decision_feature_value();
      string decision_feature_label = node_data->get_decision_feature_label();


      if(prev_decision_feature_value.size() > 0)
      {
         cout << "Previous decision feature value = " 
              << prev_decision_feature_value << endl;
      }
      
      if(decision_feature_label.size() > 0)
      {
         cout << "Decision feature label = " << decision_feature_label
              << endl;
      }

      vector<int> children_node_IDs=curr_node_ptr->get_children_node_IDs();
      if(children_node_IDs.size() > 0)
      {
         cout << "Children node IDs: " << endl;
         for (unsigned int c=0; c<children_node_IDs.size(); c++)
         {
            cout << children_node_IDs[c] << " ";
         }
         cout << endl;
      }
      else
      {
         cout << "leaf node classification = " 
              << node_data->get_classification_value() << endl;
      }
      cout << endl;
   } // loop over index n labeling decision tree nodes
}

// ---------------------------------------------------------------------
// Member function generate_tree_node() instantiates a new child node
// within the decision tree.  The child node knows about its parent
// node and vice-versa.  Input training example IDs and active feature
// IDs are assigned to the new child node.

node* decision_tree::generate_tree_node(
   int node_ID, int parent_ID, const vector<int> training_example_IDs,
   const vector<int> active_feature_IDs)
{
//   cout << "inside generate_tree_node() " << endl;

   int level = 0;
   node *parent_node = tree_graph.get_node_ptr(parent_ID);
   if(parent_node != NULL)
   {
      level = parent_node->get_level() + 1;
   }

   node *child_node = new node(node_ID, level);
   child_node->set_parent_ID(parent_ID);
   if(parent_node != NULL)
   {
      parent_node->add_child_node_ID(node_ID);
   }
   tree_graph.add_node(child_node);

   dtree_node_data* child_node_data = new dtree_node_data(node_ID);
   if(training_example_IDs.size() > 0)
      child_node_data->set_training_example_IDs(training_example_IDs);
   if(active_feature_IDs.size() > 0)
      child_node_data->set_active_feature_IDs(active_feature_IDs);

   (*node_data_map_ptr)[node_ID] = child_node_data;
   return child_node;
}

// ---------------------------------------------------------------------
node* decision_tree::initialize_root_node()
{
//   cout << "inside initialize_root_node()" << endl;

   int node_ID = 0;
   int parent_node_ID = -1;

   vector<int> active_feature_IDs;
   for(int f = 0; f < n_features; f++)
   {
      active_feature_IDs.push_back(f);
   } // loop over index f labeling feature IDs

   node *curr_node = generate_tree_node(
      node_ID, parent_node_ID, training_example_IDs, active_feature_IDs);
   node_data_iter = node_data_map_ptr->find(curr_node->get_ID());
   dtree_node_data *node_data = node_data_iter->second;
   node_data->set_prev_decision_feature_value("");

   return curr_node;
}

// ---------------------------------------------------------------------
// Member function build_tree_graph() 

void decision_tree::build_tree_graph()
{
//   cout << "inside decision_tree::build_tree_graph() " << endl;

   initialize_root_node();

   node *curr_node = tree_graph.get_node_ptr(0);
   curr_node->set_parent_ID(-1);
//   cout << "curr_node->ID = " << curr_node->get_ID() << endl;

   Treenode<DATA_TYPE>* treenode_ptr = tree_ptr->get_root_ptr();
//   cout << "treenode: ID = " << treenode_ptr->get_ID()
//        << " level = " << treenode_ptr->get_level()
//        << " column = " << treenode_ptr->get_column()
//        << endl;

   DATA_TYPE treenode_lines;
   string curr_line="Level=0  ID="+
      stringfunc::number_to_string(curr_node->get_ID());
   treenode_lines.push_back(curr_line);

   treenode_ptr->set_data(treenode_lines);

   node_data_iter = node_data_map_ptr->find(curr_node->get_ID());
   dtree_node_data *node_data = node_data_iter->second;

   split_node_or_identify_as_leaf(
      curr_node, treenode_ptr, node_data->get_training_example_IDs(),
      node_data->get_active_feature_IDs());

   tabulate_node_children();
}

// ---------------------------------------------------------------------
// Member function split_node_or_identify_as_leaf() determines whether
// input *curr_node is a leaf or internal node.  Leaf nodes are
// assigned some classification value.  Internal nodes are split
// according to whatever feature yields maximum information gain.
// This method is then recursively called by each of the children of
// the split parent node.

void decision_tree::split_node_or_identify_as_leaf(
   node *curr_node, Treenode<DATA_TYPE>* curr_treenode_ptr,
   vector<int>& input_data_example_IDs, 
   vector<int>& input_active_feature_IDs)
{
//   cout << "inside decision_tree::split_node_or_identify_as_leaf()" << endl;
//   cout << "curr_node->ID = " << curr_node->get_ID() 
//        << " curr_node->level = " << curr_node->get_level() << endl;
//   cout << "input_data_example_IDs.size() = " 
//        << input_data_example_IDs.size() << endl;
//   cout << "input_active_feature_IDs.size() = " 
//        << input_active_feature_IDs.size() << endl;
   
// First create local copies of input_data_example_IDs and
// input_active_feature_IDs:

   vector<int> curr_data_example_IDs, active_feature_IDs;
   for(unsigned int t = 0; t < input_data_example_IDs.size(); t++)
   {
      curr_data_example_IDs.push_back(input_data_example_IDs[t]);
   }

   if(curr_data_example_IDs.size() == 0)
   {
      cout << "inside split_node_or_identify_leaf(), curr_data_example_IDs.size = 0! " << endl;
   }
   
   
   for(unsigned int a = 0; a < input_active_feature_IDs.size(); a++)
   {
      active_feature_IDs.push_back(input_active_feature_IDs[a]);
   }

   int n_active_feature_IDs = active_feature_IDs.size();
/*
   if(n_active_feature_IDs > 0 && split_stopping_threshold < 1)
   {
      vector<int> feature_IDs_passing_chisq_test;
      for(unsigned int f = 0; f < active_feature_IDs.size(); f++)
      {
         int feature_ID = active_feature_IDs[f];
         double p_value = compute_chisq_Pvalue(
            feature_ID, curr_data_example_IDs);
         cout << "f = " << f << " feature_ID = " << feature_ID
              << " p_value = " << p_value << endl;

         if(p_value < split_stopping_threshold)
         {
            feature_IDs_passing_chisq_test.push_back(feature_ID);
         }
      }

      active_feature_IDs.clear();
      for(unsigned int f = 0; f < feature_IDs_passing_chisq_test.size(); f++)
      {
         active_feature_IDs.push_back(feature_IDs_passing_chisq_test[f]);
      }
      n_active_feature_IDs = active_feature_IDs.size();
      cout << "After chisq test, n_active_feature_IDs = " 
           << n_active_feature_IDs << endl;
   } // n_active_feature_IDs > 0 && split_stopping_threshold < 1 conditional
*/

// If one of the stopping criteria is fulfilled, mark the current node
// as a leaf and assign its classification value as its label:

   if(potentially_set_leaf_node_classification(
         curr_node, curr_treenode_ptr, curr_data_example_IDs, 
         n_active_feature_IDs))
   {
      return;
   }

// Compute information gain for all active feature IDs.  Select
// decision_feature as one with maximum information gain:

   int decision_feature_ID = -1;
   double max_info_gain = -1;
   for(unsigned int f = 0; f < active_feature_IDs.size(); f++)
   {
      int feature_ID = active_feature_IDs[f];
      double info_gain = compute_information_gain(
         feature_ID, curr_data_example_IDs);
      if(info_gain > max_info_gain)
      {
         max_info_gain = info_gain;
         decision_feature_ID = feature_ID;
      }
//      cout << "feature_ID = " << feature_ID
//           << " feature_label = " << feature_labels[feature_ID]
//           << " info_gain = " << info_gain 
//           << endl;
   }
   string decision_feature_label=feature_labels[decision_feature_ID];
//   cout << "decision_feature_ID = " << decision_feature_ID
//        << " decision_feature_label = " << decision_feature_label
//        << endl;

//    outputfunc::enter_continue_char();

   int data_ID = curr_node->get_data_ID();
   node_data_iter = node_data_map_ptr->find(data_ID);
   dtree_node_data *curr_node_data = node_data_iter->second;
   curr_node_data->set_decision_feature_label(
      feature_labels[decision_feature_ID]);

   string label_line="Decision feature: "+decision_feature_label;
   curr_treenode_ptr->get_data().push_back(label_line);

   vector<int> subset_active_feature_IDs;
   for(unsigned int a = 0; a < active_feature_IDs.size(); a++)
   {
      if(active_feature_IDs[a] == decision_feature_ID) continue;
      subset_active_feature_IDs.push_back(active_feature_IDs[a]);
   }

   FEATURE_VALUES_MAP::iterator feature_values_iter = 
      unique_feature_values_map_ptr->find(decision_feature_ID);
   vector<string> decision_feature_values = feature_values_iter->second;
   for(unsigned int v = 0; v < decision_feature_values.size(); v++)
   {
//      cout << "v = " << v << " feature_value = " << decision_feature_values[v]
//           << endl;

      vector<int> subset_data_example_IDs;
      get_data_example_IDs_for_feature_value(
         decision_feature_ID, decision_feature_values[v], 
         curr_data_example_IDs, subset_data_example_IDs);
      

//      cout << "subset_data_example_IDs.size() = "
//           << subset_data_example_IDs.size() << endl;
//      for(unsigned int s = 0; s < subset_data_example_IDs.size(); s++)
//      {
//         cout << "s = " << s 
//              << " subset training example ID = " 
//              << subset_data_example_IDs[s] 
//              << endl;
//      }

// WRONG WRONG !!! Fri Sep 4 at 7:10 am

//      if(subset_data_example_IDs.size() == 0) continue;


      int child_node_ID = tree_graph.get_n_nodes();
      int parent_ID = curr_node->get_ID();
      node* child_node = generate_tree_node(
         child_node_ID, parent_ID, subset_data_example_IDs,
         subset_active_feature_IDs);

      node_data_iter = node_data_map_ptr->find(child_node_ID);
      dtree_node_data *child_node_data = node_data_iter->second;
      child_node_data->set_prev_decision_feature_value(
         decision_feature_values[v]);


      Treenode<DATA_TYPE>* child_treenode_ptr = tree_ptr->addChild(parent_ID);
      DATA_TYPE child_treenode_lines;
      string label_line="Parent: ID="+stringfunc::number_to_string(parent_ID)
         +" Decision feature="+decision_feature_label;
      child_treenode_lines.push_back(label_line);
      label_line="Level="+stringfunc::number_to_string(
         child_treenode_ptr->get_level())+ 
         "  ID="+stringfunc::number_to_string(child_node_ID)
         +" Decision value="+decision_feature_values[v];
      child_treenode_lines.push_back(label_line);
      child_treenode_ptr->set_data(child_treenode_lines);

// If child node contains 0 training examples, treat it as a leaf
// node.  Assign most common classification among curr node's training
// examples as empty leaf's decision value:

      if(subset_data_example_IDs.size() == 0)
      {
         string leaf_classification = 
            determine_most_common_classification_value(
               curr_data_example_IDs);
         set_leaf_node_classification(child_node, child_treenode_ptr,
                                      leaf_classification);
      }
      else
      {
         split_node_or_identify_as_leaf(
            child_node,child_treenode_ptr,subset_data_example_IDs,
            subset_active_feature_IDs);
      }

   } // loop over index v labeling decision feature values
}

// ---------------------------------------------------------------------
// Member function potentially_set_leaf_node_classification()
// evaluates if one of the stopping criteria is fulfilled.  If so, the
// input curr_node is marked as a leaf.  The most common
// classification value among the leaf's training examples is assigned
// as the classification for the leaf.

bool decision_tree::potentially_set_leaf_node_classification(
   node *curr_node, Treenode<DATA_TYPE>* curr_treenode_ptr,
   vector<int>& curr_data_example_IDs, int n_active_feature_IDs)
{
//   cout << "inside decision_tree::potentially_set_leaf_node_classification()
//        << endl;

   if(n_active_feature_IDs == 0 
//    || curr_data_example_IDs.size() < some_minimal_threshold
      )
   {
      string most_common_classification=
         determine_most_common_classification_value(curr_data_example_IDs);
      set_leaf_node_classification(curr_node, curr_treenode_ptr,
                                   most_common_classification);
      return true;
   }

// Next check whether input data examples all have the same
// classification value:

   bool uniform_classification_values = true;
   string zeroth_data_example_classification = 
      data_examples[curr_data_example_IDs[0]].get_classification_value();
   for(unsigned int i = 1; i < curr_data_example_IDs.size(); i++)
   {
      data_example curr_data_example = data_examples[curr_data_example_IDs[i]];
      if(curr_data_example.get_classification_value() != 
         zeroth_data_example_classification)
      {
         uniform_classification_values = false;
         break;
      }
   } // loop over index i labeling training examples for input node

   if(uniform_classification_values)
   {
      set_leaf_node_classification(curr_node, curr_treenode_ptr,
                                   zeroth_data_example_classification);
      return true;
   }

   return false;
}

// ---------------------------------------------------------------------
// Member function set_leaf_node_classification() adds a
// classification value to input *curr_node.  It also adds a text line
// to the associated *curr_treenode_ptr for OSG graph display
// purposes.

void decision_tree::set_leaf_node_classification(
   node *curr_node, Treenode<DATA_TYPE>* curr_treenode_ptr,
   string leaf_classification)
{
//   cout << "inside decision_tree::set_leaf_node_classification() " << endl;
   
   int data_ID = curr_node->get_data_ID();
   node_data_iter = node_data_map_ptr->find(data_ID);
   dtree_node_data *curr_node_data = node_data_iter->second;
   curr_node_data->set_classification_value(leaf_classification);

   string label_line="Classification: "+leaf_classification;
   curr_treenode_ptr->get_data().push_back(label_line);
   generate_tree_branch(curr_node);
}

// ---------------------------------------------------------------------
// Member function generate_tree_branch() starts with a leaf node
// within the decision tree and climbs its branch back to the root
// node.  IDs for all nodes between the leaf and root are saved within
// an STL vector.  The STL vector of IDs is added as a value with an
// integer tree branch ID to STL map member *tree_branch_map_ptr.

void decision_tree::generate_tree_branch(node *curr_node)
{
//   cout << "inside decision_tree::generate_tree_branch()" << endl;

   vector<int> tree_branch_node_IDs;
   while(curr_node != NULL)
   {
      tree_branch_node_IDs.push_back(curr_node->get_ID());
      int parent_ID = curr_node->get_parent_ID();

//      cout  << "curr node: ID = " << tree_branch_node_IDs.back()
//            << " level = " << curr_node->get_level() 
//            << " parent_ID = " << parent_ID
//            << endl;
      if(parent_ID >= 0)
      {
         node *parent_node = tree_graph.get_node_ptr(parent_ID);
         curr_node = parent_node;
      }
      else
      {
         curr_node = NULL;
      }
   }

   int tree_branch_ID = tree_branch_map_ptr->size();
   (*tree_branch_map_ptr)[tree_branch_ID] = tree_branch_node_IDs;
}

// ---------------------------------------------------------------------
// Member function tabulate_node_children() iterates over all nodes
// within member tree_graph.  It retrieves the number of children for
// each node and adds a string label to the node's corresponding
// Treenode.

void decision_tree::tabulate_node_children()
{
//   cout << "inside decision_tree::tabulate_node_children()" << endl;

   for(unsigned int n = 0; n < tree_graph.get_n_nodes(); n++)
   {
      node *curr_node_ptr = tree_graph.get_node_ptr(n);
      int curr_node_ID = curr_node_ptr->get_ID();
      int n_children = curr_node_ptr->get_n_children();
      string label_line="n_children="+stringfunc::number_to_string(n_children);

      Treenode<DATA_TYPE>* curr_treenode_ptr = tree_ptr->get_Treenode_ptr(
         curr_node_ID);
//      DATA_TYPE curr_treenode_lines = curr_treenode_ptr->get_data();
      curr_treenode_ptr->get_data().push_back(label_line);
   }
}

// ---------------------------------------------------------------------
// Member function classify_example_via_decision_tree() takes in some
// test example containing feature values.  It returns the decision
// tree classification for the input test example.

string decision_tree::classify_example_via_decision_tree(
   const data_example& curr_data_example)
{
//   cout << "inside decision_tree::classify_example()" << endl;

   node *curr_node = tree_graph.get_node_ptr(0);
//   cout << "curr_node->ID = " << curr_node->get_ID() << endl;
   return recursively_find_leaf_classification(curr_node, curr_data_example);
}
 
// ---------------------------------------------------------------------
// Member function recursively_find_leaf_classification() takes in a
// decision tree node.  If the node is a leaf, this method returns its
// classification value.  Otherwise, it retrieves all children node of
// the current node and picks the one whose previous decision feature
// value matches that of the current node.  This method then
// recursively calls itself with the selected child node.

string decision_tree::recursively_find_leaf_classification(
   node *curr_node, const data_example& curr_data_example)
{
//   cout << "inside decision_tree::recursively_find_leaf_classification()" 
//        << endl;
//   cout << "curr_node->ID = " << curr_node->get_ID() << endl;

   node_data_iter = node_data_map_ptr->find(curr_node->get_ID());
   dtree_node_data *node_data = node_data_iter->second;

   if(node_data->get_classification_value() != "")
   {
      return node_data->get_classification_value();
   }
   else
   {
      int decision_feature_ID = get_feature_label_ID(
         node_data->get_decision_feature_label());
      string data_example_decision_feature_value = 
         curr_data_example.get_feature_value(decision_feature_ID);
      
// Scan through all children nodes.  Pick the child whose
// prev_decision_feature_value matches
// test_example_decision_feature_value

      vector<int> children_node_IDs = curr_node->get_children_node_IDs();
      for(unsigned int c = 0; c < children_node_IDs.size(); c++)
      {
         int child_node_ID=children_node_IDs[c];
         node *child_node = tree_graph.get_node_ptr(child_node_ID);
         node_data_iter = node_data_map_ptr->find(child_node_ID);
         dtree_node_data *child_node_data = node_data_iter->second;
         if(child_node_data->get_prev_decision_feature_value() ==
            data_example_decision_feature_value)
         {
            return recursively_find_leaf_classification(
               child_node, curr_data_example);
            break;
         }
      } // loop over index c labeling children node IDs
   }
   return "";
}

// ==========================================================================
// Decision rules member functions
// ==========================================================================

// Member function generate_decision_rules() iterates over all tree
// branches.  It converts each set of branch nodes plus leaf
// classification into a decision rule.  Decision rules are stored
// within STL map member *decision_rules_map_ptr.

void decision_tree::generate_decision_rules()
{
//   cout << "inside decision_tree::generate_decision_rules()" << endl;

   TREE_BRANCH_MAP::iterator tree_branch_iter;   
   for(tree_branch_iter = tree_branch_map_ptr->begin();
       tree_branch_iter != tree_branch_map_ptr->end(); tree_branch_iter++)
   {
      int decision_rule_ID = tree_branch_iter->first;
      string decision;
      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr = 
         new FEATURE_LABELS_VALUES_MAP;
      pair<FEATURE_LABELS_VALUES_MAP*, string> P;

      vector<int> tree_branch_node_IDs = tree_branch_iter->second;
//      cout << endl;
//      cout << "Rule " << decision_rule_ID << " : " << endl;
      for(unsigned int i = 0; i < tree_branch_node_IDs.size(); i++)
      {
         node *curr_node = tree_graph.get_node_ptr(tree_branch_node_IDs[i]);
         node_data_iter = node_data_map_ptr->find(curr_node->get_ID());
         dtree_node_data *node_data = node_data_iter->second;

         string decision_feature_label;
         int parent_ID = curr_node->get_parent_ID();
         if(parent_ID >= 0)
         {
            node_data_iter = node_data_map_ptr->find(parent_ID);
            dtree_node_data *parent_node_data = node_data_iter->second;
            decision_feature_label = parent_node_data->
               get_decision_feature_label();
         }

         string decision_feature_value = 
            node_data->get_prev_decision_feature_value();
         if(decision_feature_value.size() > 0)
         {
//            cout << "  Tree node ID = " << tree_branch_node_IDs[i]
//                 << "    feature label = " << decision_feature_label
//                 << "    feature value = " << decision_feature_value
//                 << endl;
            (*feature_labels_values_map_ptr)[decision_feature_label] = 
               decision_feature_value;
         }

         vector<int> children_node_IDs=curr_node->get_children_node_IDs();
         if(children_node_IDs.size() == 0)
         {
            decision = node_data->get_classification_value();
            P.second = decision;
         }
      } // loop over index i labeling decision rule node IDs
//      cout << "  Decision : " << decision << endl;

      P.first = feature_labels_values_map_ptr;
      (*decision_rules_map_ptr)[decision_rule_ID] = P;
      rule_IDs.push_back(decision_rule_ID);
   } // loop over tree_branch_iter labeling decision rules
}

// ---------------------------------------------------------------------
// Member function print_decision_rules() loops over all rules within
// STL map *decision_rules_map_ptr.  It prints each rule's ID plus its
// set of feature labels and values.  This method all prints each
// rule's decision value.

void decision_tree::print_decision_rules()
{
//   cout << "inside decision_tree::print_decision_rules()" << endl;

   DECISION_RULES_MAP::iterator decision_rules_iter;
   for(decision_rules_iter = decision_rules_map_ptr->begin();
       decision_rules_iter != decision_rules_map_ptr->end();
       decision_rules_iter++)
   {
      int rule_ID = decision_rules_iter->first;
      cout << "Rule " << rule_ID << endl;

      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr = 
         decision_rules_iter->second.first;
      FEATURE_LABELS_VALUES_MAP::iterator flv_iter;
      for(flv_iter = feature_labels_values_map_ptr->begin();
          flv_iter != feature_labels_values_map_ptr->end(); flv_iter++)
      {
         string feature_label = flv_iter->first;
         string feature_value = flv_iter->second;
         cout << "   Feature : label = " << feature_label 
              << "   value = " << feature_value << endl;
      }
      string decision = decision_rules_iter->second.second;
      cout << "   Decision = " << decision << endl;
      cout << endl;
   }
}

// ---------------------------------------------------------------------
// Member function classify_example_via_decision_rules() iterates
// over all decision rules which are assumed to be sorted according to
// accuracy.  It returns the classification output for the first (and
// presumably most accurate) decision rule which is consistent with
// the input curr_data_example.

string decision_tree::classify_example_via_decision_rules(
   const data_example& curr_data_example, int& classifying_rule_ID)
{
//   cout << "inside decision_tree::classify_example_via_decision_rules()" 
//        << endl;

   string rule_decision = "NULL";
   vector<int> consistent_rule_IDs;
   DECISION_RULES_MAP::iterator decision_rules_iter;
   for(unsigned int i = 0; i < rule_IDs.size(); i++)
   {
      int rule_ID = rule_IDs[i];
      decision_rules_iter = decision_rules_map_ptr->find(rule_ID);
      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr = 
         decision_rules_iter->second.first;

      if(rule_consistent_with_example(
            curr_data_example, feature_labels_values_map_ptr))
      {
         classifying_rule_ID = rule_ID;
         decision_rules_iter = decision_rules_map_ptr->find(rule_ID);
         rule_decision = decision_rules_iter->second.second;
         return rule_decision;
      }
   } // loop over index i labeling decision rules
   return rule_decision;
}

// ---------------------------------------------------------------------
// Member function rule_consistent_with_example() loops over the input test
// example's features.  Each feature's label is used as a key to query
// input STL map *feature_labels_values_map_ptr.  If the feature label
// query returns a feature value which does not match that of the
// input test example, the rule is regarded as inconsistent with the
// input test example.  This boolean then returns false.

bool decision_tree::rule_consistent_with_example(
   const data_example& curr_data_example,
   FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr)
{
//   cout << "inside decision_tree::rule_consistent_with_example()" << endl;

   for(int f = 0; f < n_features; f++)
   {
      string example_feature_label = curr_data_example.get_feature_label(f);
      FEATURE_LABELS_VALUES_MAP::iterator flv_iter = 
         feature_labels_values_map_ptr->find(example_feature_label);
      if(flv_iter != feature_labels_values_map_ptr->end())
      {
         string example_feature_value = curr_data_example.get_feature_value(f);
         if(example_feature_value != flv_iter->second)
         {
            return false;
         }
      }
   } // loop over index f labeling data example features

   return true;
}

// ---------------------------------------------------------------------
// Member function decision_rule_vs_example_decision() takes in a
// feature_labels_values_map and rule decision for one decision rule
// and one training example.  If the rule and training example do not
// have consistent feature labels and values, they cannot be directly
// compared.  This method then returns 0.  Otherwise, it returns 1
// [-1] if the rule's decision is the same as [different from] the
// training example's classification value.

int decision_tree::decision_rule_vs_example_decision(
   const data_example& curr_data_example,
   FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr,
   string rule_decision)
{
//    cout << "inside decision_tree::decision_rule_vs_example_decision()" << endl;

   if(!rule_consistent_with_example(
         curr_data_example, feature_labels_values_map_ptr))
   {
      return 0;
   }

   if(rule_decision ==  curr_data_example.get_classification_value())
   {
      return 1;
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
// Member function compute_decision_rule_accuracies()

double decision_tree::compute_decision_rule_accuracies(
   int data_examples_type, bool sort_rules_by_accuracy_flag)
{
//   cout << "inside decision_tree::compute_decision_rule_accuracies()"
//        << endl;

   if(data_examples_type == 0)
   {
      cout << "Computing rule accuracies on training data examples:" << endl;
   }
   else if(data_examples_type == 1)
   {
      cout << "Computing rule accuracies on validation data examples:" << endl;
   }
   else if(data_examples_type == 2)
   {
      cout << "Computing rule accuracies on test data examples:" << endl;
   }
   cout << "Total number of decision rules = " 
        << decision_rules_map_ptr->size() << endl;

   rule_IDs.clear();
   rule_accuracies.clear();

   int n_applicable_rules = 0;
   int n_flawed_rules = 0;
   double avg_rule_accuracy = 0;
   DECISION_RULES_MAP::iterator decision_rules_iter;
   for(decision_rules_iter = decision_rules_map_ptr->begin();
       decision_rules_iter != decision_rules_map_ptr->end();
       decision_rules_iter++)
   {
      FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr = 
         decision_rules_iter->second.first;
      string rule_decision = decision_rules_iter->second.second;
      double rule_accuracy = decision_rule_accuracy(
         data_examples_type, feature_labels_values_map_ptr, rule_decision);

      rule_accuracies.push_back(rule_accuracy);
      rule_IDs.push_back(decision_rules_iter->first);

      if(rule_accuracy < 0)
      {
         continue;
      }
      else if(rule_accuracy < 0.9999) 
      {
         n_flawed_rules++;
      }

      n_applicable_rules++;
      avg_rule_accuracy += rule_accuracy;
/*
      double flawed_rule_frac = double(n_flawed_rules) / n_applicable_rules;
      cout << "rule: ID=" << rule_ID << " of " << n_decision_rules;
      if(data_examples_type == 0)
      {
         cout << " training";
      }
      else if(data_examples_type == 1)
      {
         cout << " validation";
      }
      if(data_examples_type == 2)
      {
         cout << " test";
      }
      cout << " accuracy=" << rule_accuracy 
           << " n_flawed_rules=" << n_flawed_rules
           << " flawed rule frac=" << flawed_rule_frac
           << endl;
*/
   }

// Sort decision rules according to their accuracies in descending
// order:

   if(sort_rules_by_accuracy_flag)
      templatefunc::Quicksort_descending(rule_accuracies, rule_IDs);

/*
   for(unsigned int r = 0; r < rule_accuracies.size(); r++)
   {
      cout << "rule: ID = " << rule_IDs[r] << " accuracy = "
           << rule_accuracies[r] << endl;
   }
*/

   avg_rule_accuracy /= n_applicable_rules;
//   cout << "Average rule accuracy = " << avg_rule_accuracy << endl;
   return avg_rule_accuracy;
}

// ---------------------------------------------------------------------
// Member function decision_rule_accuracy() takes in flag
// data_examples_type along with one decision rule's
// feature_labels_values_map and decision.  The flag selects example
// IDs from the training, validation or test data sets.  This method
// then compares the input decision rule's result against all data
// examples which are comparable.  It returns the ratio of correct
// rule predictions to total number of rule predictions as a measure
// of the decision rule's accuracy.

double decision_tree::decision_rule_accuracy(
   int data_examples_type, 
   FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr, 
   string rule_decision)
{
   int n_rule_comparisons = 0;
   int n_rule_matches = 0;
   vector<int> curr_data_example_IDs;
   if(data_examples_type == 0)
   {
      for(unsigned int i = 0; i < training_example_IDs.size(); i++)
      {
         curr_data_example_IDs.push_back(training_example_IDs[i]);
      }
   }
   else if(data_examples_type == 1)
   {
      for(unsigned int i = 0; i < validation_example_IDs.size(); i++)
      {
         curr_data_example_IDs.push_back(validation_example_IDs[i]);
      }
   }
   else if(data_examples_type == 2)
   {
      for(unsigned int i = 0; i < test_example_IDs.size(); i++)
      {
         curr_data_example_IDs.push_back(test_example_IDs[i]);
      }
   }

   for(unsigned int i = 0; i < curr_data_example_IDs.size(); i++)
   {
      int match_flag = decision_rule_vs_example_decision(
         get_data_example(curr_data_example_IDs[i]), 
         feature_labels_values_map_ptr, rule_decision);
      if(match_flag == 0) continue;
      n_rule_comparisons++;
      if(match_flag == 1) n_rule_matches++;
   } // loop over index t labeling stored data examples
   double rule_accuracy = -1;
   if(n_rule_comparisons > 0)
   {
      rule_accuracy = double(n_rule_matches) / double(n_rule_comparisons);
   }
   return rule_accuracy;
}

// ---------------------------------------------------------------------
// Member function prune_decision_rules()

void decision_tree::prune_decision_rules()
{
   cout << "inside decision_tree::prune_decision_rules()" << endl;

   int n_rules = decision_rules_map_ptr->size();
   cout << "Total number of decision rules = " << n_rules << endl;

   int data_examples_type = 1;	// validation data
   DECISION_RULES_MAP::iterator decision_rules_iter;
   for(decision_rules_iter = decision_rules_map_ptr->begin();
       decision_rules_iter != decision_rules_map_ptr->end();
       decision_rules_iter++)
   {
      int n_curr_rule_prunings = 0;
      while(potentially_prune_decision_rule(
               data_examples_type, decision_rules_iter, n_curr_rule_prunings))
      {
         cout << "rule ID = " << decision_rules_iter->first
              << " n_prunings = " << n_curr_rule_prunings
              << endl;
      }
      
   } // loop over decision_rules_iter labeling all decision rules
}

// ---------------------------------------------------------------------
// Member function potentially_prune_decision_rule()

bool decision_tree::potentially_prune_decision_rule(
   int data_examples_type, DECISION_RULES_MAP::iterator& decision_rules_iter,
   int& n_curr_rule_prunings)
{
//   cout << "inside decision_tree::potentially_prune_decision_rule()" << endl;

   FEATURE_LABELS_VALUES_MAP* feature_labels_values_map_ptr = 
      decision_rules_iter->second.first;
   string rule_decision = decision_rules_iter->second.second;
   double rule_accuracy = decision_rule_accuracy(
      data_examples_type, feature_labels_values_map_ptr, rule_decision);

   bool rule_reduced = false;
   if(rule_accuracy < 0.0001 || rule_accuracy > 0.9999)
   {
      return rule_reduced;
   }

   FEATURE_LABELS_VALUES_MAP::iterator flv_iter;
   int n_features = feature_labels_values_map_ptr->size();

   vector<double> reduced_rule_accuracy;
   vector<FEATURE_LABELS_VALUES_MAP*> reduced_feature_labels_values_map_ptrs;
   for(int f = 0; f < n_features; f++)
   {
      FEATURE_LABELS_VALUES_MAP* reduced_feature_labels_values_map_ptr = 
         new FEATURE_LABELS_VALUES_MAP;
      reduced_feature_labels_values_map_ptrs.push_back( 
         reduced_feature_labels_values_map_ptr);
      int feature_counter = 0;
      for(flv_iter = feature_labels_values_map_ptr->begin();
          flv_iter != feature_labels_values_map_ptr->end(); flv_iter++)
      {
         if(feature_counter == f) 
         {
            feature_counter++;
            continue;
         }
         string feature_label = flv_iter->first;
         string feature_value = flv_iter->second;
         (*reduced_feature_labels_values_map_ptr)[feature_label] 
            = feature_value;
         feature_counter++;
      }

      reduced_rule_accuracy.push_back(
         decision_rule_accuracy(
            data_examples_type, reduced_feature_labels_values_map_ptr, 
            rule_decision));
   } // loop over index f labeling ignored feature within reduced rules
      
   templatefunc::Quicksort_descending(
      reduced_rule_accuracy, reduced_feature_labels_values_map_ptrs);

// FAKE FAKE: Following logic is definitely not fully correct...but
// good enough for initial testing...  Tues Sep 1, 2015 at 7:23 am

   int fstart = 0;
   if(reduced_rule_accuracy[0] > rule_accuracy)
   {
//            cout << "rule_ID = " << rule_ID
//                 << " init rule accuracy = " << rule_accuracy
//                 << " reduced rule accuracy = " << reduced_rule_accuracy
//                 << endl;
      decision_rules_iter->second.first = 
         reduced_feature_labels_values_map_ptrs[0];
      fstart = 1;
      rule_reduced = true;
      n_curr_rule_prunings++;
   }
   else
   {
      rule_reduced = false;
   }
      
   for(int f = fstart; f < n_features; f++)
   {
      delete reduced_feature_labels_values_map_ptrs[f];
   }

   return rule_reduced;
} 


// ---------------------------------------------------------------------
// Member function data_example_type_label() returns a descriptive
// string label corresponding to input data_example_type integers.

string decision_tree::data_example_type_label(int data_example_type)
{
   if(data_example_type == 0)
   {
      return "training";
   }
   else if(data_example_type == 1)
   {
      return "validation";
   }
   else if(data_example_type == 2)
   {
      return "test";
   }
   else
   {
      return "";
   }
}

// ---------------------------------------------------------------------
// Member function evaluate_classification_performance() compares
// predicted classification results from decision rules (or trees)
// with actual measured values.  It returns the fraction of data
// examples which are correctly classified.

double decision_tree::evaluate_classification_performance(int data_example_type)
{
//   cout << "inside decision_tree::evaluate_classification_performance()" << endl;
   string example_label = data_example_type_label(data_example_type);

   cout << endl;
   cout << "========================================================" << endl;
   cout << "Evaluating classification performance on "
        << example_label << " examples:" << endl;

   int n_correct_classifications = 0;
   vector<int> example_IDs;
   if(data_example_type == 0)
   {
      example_IDs = get_training_example_IDs();
   }
   else if (data_example_type == 1)
   {
      example_IDs = get_validation_example_IDs();
   }
   else
   {
      example_IDs = get_test_example_IDs();
   }

   for(unsigned int i = 0; i < example_IDs.size(); i++){
      data_example curr_data_example = get_data_example(example_IDs[i]);
//      string tree_classification_value = 
//         classify_example_via_decision_tree(curr_data_example);
      int classifying_rule_ID;
      string rule_classification_value = classify_example_via_decision_rules(
         curr_data_example, classifying_rule_ID);
      
//      cout << curr_data_example << endl;
//      cout << "Classification value: Actual = "
//           << curr_data_example.get_classification_value()
//           << " Decision tree = " << tree_classification_value
//           << " Decision rule = " << rule_classification_value
//           << endl;

      if(rule_classification_value == curr_data_example.get_classification_value())
      {
         n_correct_classifications++;
//         cout << "Decision tree classification is CORRECT" << endl;
      }
      else
      {
//         cout << "Decision tree classification is WRONG" << endl;
      }

   } // loop over index i labeling test examples

   double correct_classification_frac = double(n_correct_classifications)/
      example_IDs.size();

   cout << "n_examples = " << example_IDs.size()
        << " n_correct = " << n_correct_classifications
        << " correct_frac = " << correct_classification_frac
        << endl;
   return correct_classification_frac;
}

