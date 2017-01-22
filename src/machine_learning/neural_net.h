// ==========================================================================
// Header file for neural_net class 
// ==========================================================================
// Last modified on 1/19/17; 1/20/17; 1/21/17; 1/22/17
// ==========================================================================

#ifndef NEURAL_NET_H
#define NEURAL_NET_H

#include <set>
#include <string>
#include <vector>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"

class environment;
class genmatrix;
class genvector;

class neural_net
{
   
  public:

   typedef enum{
      SGD = 0,
      RMSPROP = 1,
      MOMENTUM = 2,
      NESTEROV = 3,
      ADAM = 4
   } solver_t;

   typedef std::pair<genvector*, int> DATA_PAIR;

// First genvector holds input training/testing data vector
// Second int holds class ID corresponding to input data vector

// Recall isomorphism between integer class IDs and unit vectors
// e.g. class 0  <--->  [1, 0, 0, ..., 0]
//      class 1  <--->  [0, 1, 0, ..., 0]

// Initialization, constructor and destructor functions:

   neural_net(
      int mini_batch_size, double lambda, double rmsprop_decay_rate, 
      const std::vector<int>& n_nodes_per_layer);
   neural_net(
      int mini_batch_size, double lambda, double rmsprop_decay_rate, 
      const std::vector<int>& n_nodes_per_layer, 
      environment* env_ptr, bool sym_weights_biases_flag = false);

   neural_net(const neural_net& NN);
   ~neural_net();
//   neural_net operator= (const neural_net& NN);
   friend std::ostream& operator<< 
      (std::ostream& outstream, neural_net& NN);

// Set and get member functions

   void set_expt_number(int n);
   int get_expt_number() const;
   void set_output_subdir(std::string subdir);
   void set_extrainfo(std::string info);
   std::string get_params_filename() const;
   void set_include_bias_terms(bool flag);
   bool get_include_bias_terms() const;
   void set_environment(environment* e_ptr);   
   void set_perm_symmetrize_weights_and_biases(bool flag);
   int get_layer_dim(int l) const;
   genvector* get_biases(int l) const;
   genmatrix* get_weights(int l) const;
   std::string get_output_subdir() const;
   void set_base_learning_rate(double rate);
   double get_base_learning_rate() const;
   void push_back_learning_rate(double rate);
   double get_learning_rate() const;

   void import_training_data(const std::vector<DATA_PAIR>& data);
   void import_validation_data(const std::vector<DATA_PAIR>& data);
   void import_test_data(const std::vector<DATA_PAIR>& data);
   std::vector<DATA_PAIR> randomly_shuffle_training_data();
   std::vector< std::vector<neural_net::DATA_PAIR> > 
      generate_training_mini_batches(
         const std::vector<DATA_PAIR>& shuffled_training_data);
   void print_data_pair(int t, const DATA_PAIR& curr_data);

// Network training member functions:

   void feedforward(genvector* a_input);
   genvector* get_softmax_class_probs() const;
   double get_sample_loss(const DATA_PAIR& curr_data);
   double L2_loss_contribution();
   void decrease_learning_rate();
   void train_network(int n_epochs);

// Network evaluation member functions:

   void get_class_probabilities(
      genvector* input_state, bool sort_probs_flag, 
      std::vector<double>& class_probabilities, 
      std::vector<int>& class_IDs);
   int get_class_prediction(genvector* input_state);
   int get_class_prediction_given_probs(
      const std::vector<double>& class_probs);
   double evaluate_model_on_data_set(
      const std::vector<DATA_PAIR>& sample_data);
   double evaluate_model_on_training_set();
   double evaluate_model_on_validation_set();
   double evaluate_model_on_test_set();
   std::vector<int>& get_incorrect_classifications();

// Monitoring network training member functions:

   void summarize_parameters();
   void compute_bias_distributions();
   void compute_weight_distributions();
   void store_quasirandom_weight_values();

   std::string init_subtitle();
   bool generate_metafile_plot(
      const std::vector<double>& values,
      std::string metafile_basename, 
      std::string title, std::string y_label, 
      bool plot_smoothed_values_flag, bool zero_min_value_flag);

   void plot_loss_history();
   void plot_log10_lr_mean_abs_nabla_weight_ratios();
   void plot_accuracies_history();
   bool plot_bias_distributions();
   bool plot_weight_distributions();
   bool plot_quasirandom_weight_values();
   void generate_summary_plots();
   void generate_view_metrics_script();

   void create_snapshots_subdir();
   std::string export_snapshot();
   void import_snapshot(std::string snapshot_filename);

  private: 

   bool include_bias_terms;
   bool perm_symmetrize_weights_and_biases;
   int n_layers, n_training_samples, n_validation_samples, n_test_samples;
   int n_classes;
   int expt_number;
   int solver_type;
   int n_weights;
   std::vector<int> layer_dims;
   environment* environment_ptr;

   std::vector<genvector*> biases, nabla_biases, delta_nabla_biases;
   std::vector<genvector*> permuted_biases, sym_biases;
//	Bias STL vectors are nonzero for layers 1 thru n_layers-1

   std::vector<genmatrix*> weights, weights_transpose;
   std::vector<genmatrix*> permuted_weights, sym_weights;
//	Weight STL vectors connect layer pairs {0,1}, {1,2}, ... , 
//      {n_layers-2, n_layers-1}
   std::vector<genmatrix*> nabla_weights, delta_nabla_weights;

   std::vector<genmatrix*> rmsprop_weights_cache;

   std::vector<DATA_PAIR> training_data;
   std::vector<DATA_PAIR> validation_data;
   std::vector<DATA_PAIR> test_data;

// Node weighted inputs:
   std::vector<genvector*> z;
   std::vector<genvector*> gammas, betas;  // Batch normalization parameters

// Node activation outputs:
   std::vector<genvector*> a;

// Node errors:
   std::vector<genvector*> delta;

   int mini_batch_size;
   double base_learning_rate;
   std::vector<double> learning_rate;
   double lambda; // L2-regularization coefficient
   double rmsprop_decay_rate;
   double rmsprop_denom_const;

   int update_counter;
   std::vector<double> epoch_history;
   std::vector<double> avg_minibatch_loss;
   std::vector<double> training_accuracy_history;
   std::vector<double> validation_accuracy_history;
   std::vector<double> test_accuracy_history;
   std::vector<std::vector<double> > log10_lr_mean_abs_nabla_weight_ratios;
   std::string output_subdir;
   std::string extrainfo;
   std::string layer_label;
   std::string params_filename;
   std::string snapshots_subdir;

// Store indices for test samples whose class if incorrectly labeled
// by the trained model:

   std::vector<int> incorrect_classifications;

   std::vector<std::vector<double> > weight_01, weight_05, weight_10;
   std::vector<std::vector<double> > weight_25, weight_35, weight_50;
   std::vector<std::vector<double> > weight_65, weight_75, weight_90;
   std::vector<std::vector<double> > weight_95, weight_99;
   std::vector<std::vector<double> > bias_01, bias_05, bias_10, bias_25;
   std::vector<std::vector<double>  >bias_35, bias_50, bias_65, bias_75;
   std::vector<std::vector<double> > bias_90, bias_95, bias_99;

// Store histories for 9 "quasi-random" weights for each layer within
// following vectors of vectors:

   std::vector<std::vector<double> > weight_1, weight_2, weight_3;
   std::vector<std::vector<double> > weight_4, weight_5, weight_6;
   std::vector<std::vector<double> > weight_7, weight_8, weight_9;

   int count_weights();
   double update_nn_params(std::vector<DATA_PAIR>& mini_batch);
   void clear_delta_nablas();
   void backpropagate(const DATA_PAIR& curr_data_pair);
   void numerically_check_derivs(const DATA_PAIR& curr_data_pair);

   void allocate_member_objects();
   void initialize_member_objects(const std::vector<int>& n_nodes_per_layer);
   void docopy(const neural_net& N);
   void instantiate_weights_and_biases();
   void instantiate_training_variables();
   void initialize_weights_and_biases();

   void delete_weights_and_biases();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void neural_net::set_expt_number(int n)
{
   expt_number = n;
}

inline int neural_net::get_expt_number() const
{
   return expt_number;
}

inline void neural_net::set_output_subdir(std::string subdir)
{
   output_subdir = subdir;
   filefunc::dircreate(output_subdir);
   create_snapshots_subdir();
}

inline void neural_net::set_extrainfo(std::string info)
{
   extrainfo = info;
}

inline std::string neural_net::get_params_filename() const
{
   return params_filename;
}

inline void neural_net::set_include_bias_terms(bool flag)
{
   include_bias_terms = flag;
}

inline bool neural_net::get_include_bias_terms() const
{
   return include_bias_terms;
}

inline void neural_net::set_environment(environment* e_ptr)
{
   environment_ptr = e_ptr;
}

inline void neural_net::set_perm_symmetrize_weights_and_biases(bool flag)
{
   perm_symmetrize_weights_and_biases = flag;
}

inline int neural_net::get_layer_dim(int l) const
{
   return layer_dims[l];
}

inline genvector* neural_net::get_biases(int l) const
{
   return biases[l];
}

inline genmatrix* neural_net::get_weights(int l) const
{
   return weights[l];
}

inline std::vector<int>& neural_net::get_incorrect_classifications() 
{
   return incorrect_classifications;
}

inline std::string neural_net::get_output_subdir() const
{
   return output_subdir;
}

inline void neural_net::set_base_learning_rate(double rate)
{
   base_learning_rate = rate;
   learning_rate.push_back(base_learning_rate);
}

inline double neural_net::get_base_learning_rate() const
{
   return base_learning_rate;
}

inline void neural_net::push_back_learning_rate(double rate)
{
   learning_rate.push_back(rate);
}

inline double neural_net::get_learning_rate() const
{
   return learning_rate.back();
}


#endif  // neural_net.h


