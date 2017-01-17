// ==========================================================================
// neural_net class member function definitions
// ==========================================================================
// Last modified on 1/4/17; 1/15/17; 1/16/17; 1/17/17
// ==========================================================================

#include <iostream>
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "plot/metafile.h"
#include "machine_learning/neural_net.h"
#include "numrec/nrfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void neural_net::initialize_member_objects(
   const vector<int>& n_nodes_per_layer)
{
   expt_number = -1;
   include_bias_terms = true;
   n_weights = 0;
   n_layers = n_nodes_per_layer.size();
   for(int l = 0; l < n_layers; l++)
   {
      layer_dims.push_back(n_nodes_per_layer[l]);
      genvector *curr_z = new genvector(layer_dims.back());
      genvector *curr_gamma = new genvector(layer_dims.back());
      genvector *curr_beta = new genvector(layer_dims.back());
      genvector *curr_a = new genvector(layer_dims.back());
      genvector *curr_delta = new genvector(layer_dims.back());
      z.push_back(curr_z);
      gammas.push_back(curr_gamma);
      betas.push_back(curr_beta);
      a.push_back(curr_a);
      delta.push_back(curr_delta);
   } // loop over index l labeling neural network layers
   n_classes = layer_dims.back();

   rmsprop_denom_const = 1E-5;
   solver_type = RMSPROP;
   output_subdir = "./nn_outputs/";
   filefunc::dircreate(output_subdir);
   update_counter = 0;
}		       

void neural_net::allocate_member_objects()
{
}		       

// ---------------------------------------------------------------------
// Input STL vector sizes contains the number of neurons in the
// respective layers of the network.  For example, if sizes == [2, 3,
// 1] then the network will have 3 layers with the first layer
// containing 2 neurons, the second layer 3 neurons, and the third
// layer 1 neuron.

neural_net::neural_net(
   int mini_batch_size, double lambda, double rmsprop_decay_rate, 
   const vector<int>& n_nodes_per_layer)
{
   this->mini_batch_size = mini_batch_size;
   this->lambda = lambda;
   this->rmsprop_decay_rate = rmsprop_decay_rate;

   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();

   vector<double> dummy_dist;

   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         genvector *curr_biases = new genvector(layer_dims[l]);
         biases.push_back(curr_biases);
         genvector *curr_nabla_biases = new genvector(layer_dims[l]);
         nabla_biases.push_back(curr_nabla_biases);
         genvector *curr_delta_nabla_biases = new genvector(layer_dims[l]);
         delta_nabla_biases.push_back(curr_delta_nabla_biases);

// Initialize bias for each network node in layers 1, 2, ... to be
// gaussian random var distributed according to N(0,1).  Recall input
// layer has no biases:

         for(int i = 0; i < layer_dims[l]; i++)
         {
            if(l == 0)
            {
               curr_biases->put(i, 0);
            }
            else
            {
               curr_biases->put(i, nrfunc::gasdev());
            }
         } // loop over index i labeling node in current layer

         bias_01.push_back(dummy_dist);
         bias_05.push_back(dummy_dist);
         bias_10.push_back(dummy_dist);
         bias_25.push_back(dummy_dist);
         bias_35.push_back(dummy_dist);
         bias_50.push_back(dummy_dist);
         bias_65.push_back(dummy_dist);
         bias_75.push_back(dummy_dist);
         bias_90.push_back(dummy_dist);
         bias_95.push_back(dummy_dist);
         bias_99.push_back(dummy_dist);
      } // loop over index l labeling layers
   } // include_bias_terms conditional

// Weights link layer l with layer l+1:
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix *curr_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      weights.push_back(curr_weights);
      genmatrix *curr_weights_transpose = new genmatrix(
         layer_dims[l], layer_dims[l+1]);
      weights_transpose.push_back(curr_weights_transpose);

      genmatrix *curr_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      nabla_weights.push_back(curr_nabla_weights);
      genmatrix *curr_delta_nabla_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      delta_nabla_weights.push_back(curr_delta_nabla_weights);
      genmatrix *curr_rmsprop_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      curr_rmsprop_weights->clear_values();
      rmsprop_weights_cache.push_back(curr_rmsprop_weights);

// Xavier initialize weights connecting network layers l and l+1 to be
// gaussian random vars distributed according to N(0,1/sqrt(n_in)):

      for(int i = 0; i < layer_dims[l+1]; i++)
      {
         for(int j = 0; j < layer_dims[l]; j++)
         {
            curr_weights->put(
               i, j, sqrt(2.0) * nrfunc::gasdev() / sqrt(layer_dims[l]) );
         } // loop over index j labeling node in next layer
      } // loop over index i labeling node in current layer

      log10_lr_mean_abs_nabla_weight_ratios.push_back(dummy_dist);

      weight_01.push_back(dummy_dist);
      weight_05.push_back(dummy_dist);
      weight_10.push_back(dummy_dist);
      weight_25.push_back(dummy_dist);
      weight_35.push_back(dummy_dist);
      weight_50.push_back(dummy_dist);
      weight_65.push_back(dummy_dist);
      weight_75.push_back(dummy_dist);
      weight_90.push_back(dummy_dist);
      weight_95.push_back(dummy_dist);
      weight_99.push_back(dummy_dist);

      weight_1.push_back(dummy_dist);
      weight_2.push_back(dummy_dist);
      weight_3.push_back(dummy_dist);
      weight_4.push_back(dummy_dist);
      weight_5.push_back(dummy_dist);
      weight_6.push_back(dummy_dist);
      weight_7.push_back(dummy_dist);
      weight_8.push_back(dummy_dist);
      weight_9.push_back(dummy_dist);
   } // loop over index l labeling neural net layers
}

// Copy constructor:

neural_net::neural_net(const neural_net& NN)
{
//   docopy(dtree);
}

// ---------------------------------------------------------------------
neural_net::~neural_net()
{
   for(int l = 0; l < n_layers; l++)
   {
      delete z[l];
      delete gammas[l];
      delete betas[l];
      delete a[l];
      delete delta[l];
   }
   
   if(include_bias_terms)
   {
      for(unsigned int l = 0; l < biases.size(); l++)
      {
         delete biases[l];
         delete nabla_biases[l];
         delete delta_nabla_biases[l];
      }
   } // include_bias_terms conditional

   for(unsigned int l = 0; l < weights.size(); l++)
   {
      delete weights[l];
      delete nabla_weights[l];
      delete delta_nabla_weights[l];
      delete rmsprop_weights_cache[l];
   }
}

// ==========================================================================
// Network training methods
// ==========================================================================

// Member function feedforward returns the output of the network given
// an input set of values.  

void neural_net::feedforward(genvector* a_input)
{
   a[0] = a_input;
//    cout << "inside feedforward, a_input = " << *a_input << endl;

   for(int l = 0; l < n_layers-1; l++)
   {
      if(include_bias_terms)
      {
         z[l+1]->matrix_vector_mult_sum(*weights[l], *a[l], *biases[l+1]);
      }
      else
      {
         z[l+1]->matrix_vector_mult(*weights[l], *a[l]);
      }

//      machinelearning_func::batch_normalization(
//         *z[l+1], *gammas[l+1], *betas[l+1]);

//      cout << "l = " << l << endl;
//      cout << "z[l+1] = " << *z[l+1] << endl;

// Perform soft-max classification on final-layer's weighted inputs:

      if(l == n_layers - 2)
      {
         machinelearning_func::softmax(*z[l+1], *a[l+1]);
      }
      else // perform ReLU on hidden layer's weight inputs
      {
         machinelearning_func::leaky_ReLU(*z[l+1], *a[l+1]);
      }
//       cout << "a[l+1] = " << *a[l+1] << endl;
   }

//    outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
genvector* neural_net::get_softmax_class_probs() const
{
   return a[n_layers-1];
}

// ---------------------------------------------------------------------
// Member function get_sample_loss() computes the input data sample's
// contribution to the cross-entropy loss function.  

double neural_net::get_sample_loss(const DATA_PAIR& curr_data)
{
   genvector* class_probs = get_softmax_class_probs();
   int class_label = curr_data.second;

   double curr_prob = class_probs->get(class_label);
   double curr_loss = 15;
   double exp_neg_15 = 3.05902321E-7;
   if(curr_prob > exp_neg_15)
   {
      curr_loss = -log(curr_prob);
      curr_loss += L2_loss_contribution();
   }
   return curr_loss;
}

// ---------------------------------------------------------------------
// Member function L2_loss_contribution() adds the L2 regularization
// term's contribution to the loss function.

double neural_net::L2_loss_contribution()
{
   double L2_loss = 0;

   const double TINY = 1E-8;
   if(lambda > TINY)
   {
      double sqrd_weight_sum = 0;
      for(int l = 0; l < n_layers - 1; l++)
      {
         for(unsigned int r = 0; r < weights[l]->get_mdim(); r++)
         {
            for(unsigned int c = 0; c < weights[l]->get_ndim(); c++)
            {
               sqrd_weight_sum += sqr(weights[l]->get(r,c));
            }
         }
      } // loop over index l labeling network layers
      L2_loss += lambda * sqrd_weight_sum;
   }
   return L2_loss;
}

// ---------------------------------------------------------------------
double neural_net::evaluate_model_on_data_set(
   const vector<DATA_PAIR>& sample_data)
{
   int n_data_samples = sample_data.size();
   int n_correct_predictions = 0;
   incorrect_classifications.clear();
   for(int t = 0; t < n_data_samples; t++)
   {
      feedforward( sample_data[t].first );
      genvector* class_probs = get_softmax_class_probs();

      double max_prob = NEGATIVEINFINITY;
      int predicted_class = -1;
      for(int i = 0; i < n_classes; i++)
      {
         if(class_probs->get(i) > max_prob)
         {
            predicted_class = i;
            max_prob = class_probs->get(i);
         }
      }

      if(predicted_class == sample_data[t].second)
      {
         n_correct_predictions++;
      }
      else
      {
         incorrect_classifications.push_back(t);
      }
      
   } // loop over index t labeling test samples

   double frac_correct = double(n_correct_predictions) / n_data_samples;
   return frac_correct;
}

// ---------------------------------------------------------------------
double neural_net::evaluate_model_on_training_set() 
{
   return evaluate_model_on_data_set(training_data);
}

double neural_net::evaluate_model_on_test_set() 
{
   return evaluate_model_on_data_set(test_data);
}

// ---------------------------------------------------------------------
void neural_net::import_training_data(const vector<DATA_PAIR>& data)
{
   n_training_samples = data.size();
   for(int t = 0; t < n_training_samples; t++)
   {
      DATA_PAIR curr_data;
      curr_data.first = data[t].first;
      curr_data.second = data[t].second;
      training_data.push_back(curr_data);

//      print_data_pair(t, training_data.back());
   }
}

// ---------------------------------------------------------------------
void neural_net::import_test_data(const vector<DATA_PAIR>& data)
{
   n_test_samples = data.size();
   for(int t = 0; t < n_test_samples; t++)
   {
      DATA_PAIR curr_data;
      curr_data.first = data[t].first;
      curr_data.second = data[t].second;
      test_data.push_back(curr_data);
   }
}

// ---------------------------------------------------------------------
void neural_net::print_data_pair(int t, const DATA_PAIR& curr_data)
{
   cout << "Data index = " << t << endl;
   cout << "X = " << *(curr_data.first) << endl;
   cout << "Y = " << curr_data.second << endl << endl;
}

// ---------------------------------------------------------------------
vector<neural_net::DATA_PAIR> neural_net::randomly_shuffle_training_data()
{
   vector<DATA_PAIR> shuffled_training_data;
   vector<int> ran_seq = mathfunc::random_sequence(n_training_samples);
   for(int i = 0; i < n_training_samples; i++)
   {
      shuffled_training_data.push_back(training_data[ran_seq[i]]);
//      print_data_pair(i, shuffled_training_data.back());
   }
   return shuffled_training_data;
}

// ---------------------------------------------------------------------
vector< vector<neural_net::DATA_PAIR> > 
neural_net::generate_training_mini_batches(
   const vector<DATA_PAIR>& shuffled_training_data)
{
   int counter = 0;
   int n_mini_batches = n_training_samples / mini_batch_size;
   vector< vector<DATA_PAIR> > mini_batches;
   
   for(int b = 0; b < n_mini_batches; b++)
   {
      vector<DATA_PAIR> curr_mini_batch;
      for(int i = 0; i < mini_batch_size; i++)
      {
         curr_mini_batch.push_back(shuffled_training_data[counter++]);
      }
      mini_batches.push_back(curr_mini_batch);
   } // loop of index b labeling mini batches

   if(n_training_samples % mini_batch_size > 0)
   {
      vector<DATA_PAIR> last_mini_batch;
      while(counter < n_training_samples)
      {
         last_mini_batch.push_back(shuffled_training_data[counter++]);
      }
      mini_batches.push_back(last_mini_batch);
   }

   return mini_batches;
}

// ---------------------------------------------------------------------
// Member function decrease_learning_rate decreases the learning rate
// down to some minimal floor value.

void neural_net::decrease_learning_rate()
{
   double curr_learning_rate = get_learning_rate();
   double min_learning_rate = 0.1 * get_base_learning_rate();

   if(curr_learning_rate > min_learning_rate)
   {
      push_back_learning_rate(0.8 * curr_learning_rate);
   }
}

// ---------------------------------------------------------------------
// Member function train_network() trains the neural network using
// mini-batch stochastic gradient descent.  The training data is a
// list of "(x, y)" tuples representing the training inputs and the
// desired outputs.  The other non-optional parameters are
// self-explanatory.  If "test_data" is provided then the network will
// be evaluated against the test data after each epoch, and partial
// progress printed out.  This is useful for tracking progress, but
// slows things down substantially.  Input parameter lambda governs L2
// regularization.

void neural_net::train_network(int n_epochs)
{
   int n_update = 1 * 1000;
   int n_export_metafiles = 10 * 1000;

   for(int e = 0; e < n_epochs; e++)
   {
      cout << "Starting epoch e = " << e << " of " << n_epochs << endl;      
      
      vector<DATA_PAIR> shuffled_training_data = 
         randomly_shuffle_training_data();
      vector<vector<DATA_PAIR> > mini_batches = generate_training_mini_batches(
         shuffled_training_data);

      for(unsigned int b = 0; b < mini_batches.size(); b++)
      {
         epoch_history.push_back(e + double(b) / mini_batches.size());
//         cout << "b = " << b << " of " << mini_batches.size() 
//              << " effective epoch = " << epoch_history.back()
//              << endl;
         double curr_minibatch_loss = update_nn_params(mini_batches[b]);
         avg_minibatch_loss.push_back(curr_minibatch_loss);
         update_counter++;

         if(update_counter % n_update == 0)
         {
            training_accuracy_history.push_back(
               evaluate_model_on_training_set());
            test_accuracy_history.push_back(
               evaluate_model_on_test_set());
            cout << "Epoch e = " << epoch_history.back() 
                 << " of " << n_epochs << endl;
            cout << "   Correct prediction fracs: " << endl;
            cout << "   test samples = " << test_accuracy_history.back() 
                 << " training samples = " << training_accuracy_history.back()
                 << endl;
            cout << "   n_training_samples = " << training_data.size() 
                 << "  n_test_samples = " << test_data.size()
                 << endl;
         }
         if(update_counter % n_export_metafiles == 0)
         {
            string extrainfo = "";
            if(include_bias_terms)
            {
               compute_bias_distributions();
            }
            compute_weight_distributions();
            store_quasirandom_weight_values();
            generate_summary_plots(extrainfo);
            update_counter = 0;
         }
      } // loop over index b labeling mini batches
   } // loop over index e labeling training epochs
}

// ---------------------------------------------------------------------
// Member function update_nn_params() updates the network's weights
// and biases by applying gradient descent using backpropagation to a
// single mini batch.  lambda = L2 regularization parameter.  This
// method returns the loss averaged over all samples within the
// current mini_batch.

double neural_net::update_nn_params(vector<DATA_PAIR>& mini_batch)
{

// Initialize cumulative (over mini batch) weight and bias gradients
// to zero:

   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         nabla_biases[l]->clear_values();
      }
   }
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      nabla_weights[l]->clear_values();
   }

   double avg_minibatch_loss = 0;
   int mini_batch_size = mini_batch.size();
   for(int i = 0; i < mini_batch_size; i++)
   {
      feedforward(mini_batch[i].first);
      avg_minibatch_loss += get_sample_loss(mini_batch[i]);
      backpropagate(mini_batch[i]);

// Accumulate weights and bias gradients for each network layer:

      if(include_bias_terms)
      {
         for(int l = 0; l < n_layers; l++)
         {
            *nabla_biases[l] += *delta_nabla_biases[l] / mini_batch_size;
         }
      }
      
      for(int l = 0; l < n_layers - 1; l++)
      {
         *nabla_weights[l] += *delta_nabla_weights[l] / mini_batch_size;
         *rmsprop_weights_cache[l] = 
            rmsprop_decay_rate * (*rmsprop_weights_cache[l])
            + (1 - rmsprop_decay_rate) * nabla_weights[l]->hadamard_power(2);
      }
   } // loop over index i labeling training samples within mini batch
   avg_minibatch_loss /= mini_batch_size;
   
// Update weights and biases for eacy network layer by their nabla
// values averaged over the current mini-batch:

   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         *biases[l] -= get_learning_rate() * (*nabla_biases[l]);
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
      }
   }
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      genmatrix denom = rmsprop_weights_cache[l]->hadamard_power(0.5);
      denom.hadamard_sum(rmsprop_denom_const);
      
      if(solver_type == RMSPROP)
      {
         nabla_weights[l]->hadamard_division(denom);
         *weights[l] -= get_learning_rate() * (*nabla_weights[l]);
      }
      else if (solver_type == SGD)
      {
         *weights[l] -= get_learning_rate() * (*nabla_weights[l]);
      }
//      cout << "l = " << l << " weights[l] = " << *weights[l] << endl;

// Record average |nabla_weight / weight| to monitor network learning:

      int mdim = nabla_weights[l]->get_mdim();
      int ndim = nabla_weights[l]->get_ndim();
      vector<double> curr_nabla_weights;
      vector<double> curr_nabla_weight_ratios;

      for(int r = 0; r < mdim; r++)
      {
         for(int c = 0; c < ndim; c++)
         {
            curr_nabla_weights.push_back(fabs(nabla_weights[l]->get(r,c)));
            double denom = weights[l]->get(r,c);
            if(fabs(denom) > 1E-10)
            {
               curr_nabla_weight_ratios.push_back(
                  fabs(nabla_weights[l]->get(r,c) / denom ));
            }
         }
      }
      double mean_abs_nabla_weight_ratio = mathfunc::mean(
         curr_nabla_weight_ratios);
      if(mean_abs_nabla_weight_ratio > 0)
      {
         log10_lr_mean_abs_nabla_weight_ratios[l].push_back(
            log10(learning_rate.back() * mean_abs_nabla_weight_ratio));
      }

   } // loop over index l labeling network layers

   return avg_minibatch_loss;
}

// ---------------------------------------------------------------------
// Member function clear_delta_nablas() initializes "instantaneous"
// (i.e. just for curr_data_pair) weight and bias gradients to zero:

void neural_net::clear_delta_nablas()
{
   if(include_bias_terms)
   {
      for(int l = 0; l < n_layers; l++)
      {
         delta_nabla_biases[l]->clear_values();
      }
   }
   
   for(int l = 0; l < n_layers - 1; l++)
   {
      delta_nabla_weights[l]->clear_values();
   }
}

// ---------------------------------------------------------------------
// Member function backpropagate() takes in one training sample within
// curr_data_pair.  It returns delta_nabla_biases and
// delta_nabla_weights representing the gradient for cost function
// C_x.  delta_nabla_biases and delta_nabla_weights are layer-by-layer
// sets of genvectors similar to biases and weights.  See "Cross
// entropy loss function" notes dated 10/15/2016.

void neural_net::backpropagate(const DATA_PAIR& curr_data_pair)
{
//    cout << "inside neural_net::backpropagate()" << endl;

   int y = basic_math::round(curr_data_pair.second);
   clear_delta_nablas();

   // Recall for layer l, delta_j = dC_x / dz_j

// Eqn BP1:

   int curr_layer = n_layers - 1;
//   cout << "curr_layer = num-layers - 1 = " << n_layers - 1 << endl;

//   double curr_cost = -log(a[curr_layer]->get(y));
//    cout << "  Current training sample cost = " << curr_cost << endl;

   for(int j = 0; j < layer_dims[curr_layer]; j++)
   {
      double curr_activation = a[curr_layer]->get(j);
      if(j == y) curr_activation -= 1.0;
      delta[curr_layer]->put(j, curr_activation);
   }
   
   for(int curr_layer = n_layers - 1; curr_layer >= 1; curr_layer--)
   {
      int prev_layer = curr_layer - 1;
//      cout << "curr_layer = " << curr_layer
//           << " prev_layer = " << prev_layer << endl;

// Eqn BP2 (Leaky ReLU):

// Recall weights[prev_layer] = Weight matrix mapping prev layer nodes
// to curr layer nodes:

//      *delta[prev_layer] = weights[prev_layer]->transpose() * 
//         (*delta[curr_layer]);

      weights_transpose[prev_layer]->matrix_transpose(*weights[prev_layer]);
      delta[prev_layer]->matrix_vector_mult(
         *weights_transpose[prev_layer], *delta[curr_layer]);

      for(int j = 0; j < layer_dims[prev_layer]; j++)
      {
         if(z[prev_layer]->get(j) < 0)
         {
            delta[prev_layer]->put(
               j, machinelearning_func::get_leaky_ReLU_small_slope() * 
               delta[prev_layer]->get(j));
         }
      }
//      cout << "delta[curr_layer] = " << *delta[curr_layer] << endl;
//      cout << "delta[prev_layer] = " << *delta[prev_layer] << endl;


// Eqn BP3:   

      if(include_bias_terms)
      {
         *(delta_nabla_biases[curr_layer]) = *delta[curr_layer];
      }
      
// Eqn BP4:

      delta_nabla_weights[prev_layer]->accumulate_outerprod(
         *delta[curr_layer], *a[prev_layer]);

// Add L2 regularization contribution to delta_nabla_weights.  No such
// regularization contribution is conventionally added to
// delta_nabla_biases:

      *delta_nabla_weights[prev_layer] += 2 * lambda * (*weights[prev_layer]);

   } // loop over curr_layer

/*
// Numerically spot-check loss derivatives wrt a few random
// weights:

   if(nrfunc::ran1() < 1E-2)
   {
      numerically_check_derivs(curr_data_pair);
   }
*/

}

// ---------------------------------------------------------------------
// On 12/27/16, we numerically spot-checked soft-max loss function
// derivatives wrt random neural network weights for synthetically
// generated 2D spiral training data. We empirically found that the 
// ratio of numerically derived to backpropagated derivatives lay
// within the interval [0.99, 1.01] provided that the derivatives
// absolute magnitude exceeded 1E-9 and eps = 1E-5.  (As eps --> 0, we
// believe the numerical derivative becomes noisier.  So the ratio
// lies within [0.99, 1.01] provided the derivatives' absolute
// magnitude exceeds 1E-8 if eps = 1E-6.)  Derivatives with
// smaller magnitudes effectively equal 0.  So their ratio becomes
// noisier.

void neural_net::numerically_check_derivs(const DATA_PAIR& curr_data_pair)
{
   const double eps = 1E-5;
//   const double eps = 1E-6;
//   const double eps = 1E-8;
   for(unsigned int l = 0; l < weights.size(); l++)
   {
      int row = nrfunc::ran1() * weights[l]->get_mdim();
      int col = nrfunc::ran1() * weights[l]->get_ndim();
      double orig_weight = weights[l]->get(row, col);

      weights[l]->put(row, col, orig_weight + eps);
      feedforward(curr_data_pair.first);
      double pos_loss = get_sample_loss(curr_data_pair);

      weights[l]->put(row, col, orig_weight - eps);
      feedforward(curr_data_pair.first);
      double neg_loss = get_sample_loss(curr_data_pair);
      double curr_deriv = (pos_loss - neg_loss) / (2 * eps);
      
      weights[l]->put(row, col, orig_weight);
      cout.precision(12);
      cout << "l = " << l << " row = " << row << " col = " << col 
           << " orig_weight = " << orig_weight << endl;
      cout << "  pos_loss = " << pos_loss << " neg_loss = " << neg_loss
           << endl;
      cout << "  pos - neg loss = " << pos_loss - neg_loss << endl;
      cout << "  curr_deriv = " << curr_deriv 
           << " delta_nabla_weight = " 
           << delta_nabla_weights[l]->get(row, col)
           << endl;

      if(fabs(delta_nabla_weights[l]->get(row, col)) > 1E-10)
      {
         double ratio = 
            curr_deriv / delta_nabla_weights[l]->get(row, col);
         cout << "  curr_deriv / delta_nabla_weight = " << ratio << endl;

         if(fabs(curr_deriv) > 1E-9 && (ratio < 0.99 || ratio > 1.01)) 
         {
            cout << endl;
            outputfunc::enter_continue_char();
         }
      }
         
   } // loop over index l labeling network layers
}


// ==========================================================================
// Monitoring network training methods
// ==========================================================================

// Member function count_weights() sums up the total number of weights
// among all network layers assuming the network is fully connected.

int neural_net::count_weights()
{
   if(n_weights == 0)
   {
      for(int l = 0; l < n_layers - 1; l++)
      {
         n_weights += layer_dims[l] * layer_dims[l+1];
      }
   }
   return n_weights;
}

// ---------------------------------------------------------------------
// Member function summarize_parameters() exports most parameters and
// hyperparameters to a specified text file for book-keeping purposes.

void neural_net::summarize_parameters(string params_filename)
{
   ofstream params_stream;
   filefunc::openfile(params_filename, params_stream);

   params_stream << "Experiment " << expt_number << endl;
   params_stream << timefunc::getcurrdate() << endl;
   params_stream << "Neural net params:" << endl;
   params_stream << "   n_layers = " << n_layers << endl;
   for(int l = 0; l < n_layers; l++)
   {
      params_stream << "   layer = " << l << " n_nodes = " 
                    << layer_dims[l] << endl;
   }
   params_stream << "   n_weights = " << count_weights() << " (FC)" 
                 << endl;
   params_stream << "   include_bias_terms = " << include_bias_terms 
                 << endl;
 
   params_stream << "base_learning_rate = " << base_learning_rate 
                 << "; batch_size = " << mini_batch_size
                 << endl;

   params_stream << "solver type = RMSPROP" << endl;
   params_stream << "   rmsprop_decay_rate = " << rmsprop_decay_rate
                 << endl;
   params_stream << "   rmsprop_denom_const = " << rmsprop_denom_const
                 << endl;
   params_stream << "L2 regularization lambda coeff = " << lambda << endl;
   params_stream << "n_training_samples = " << n_training_samples << endl;
   params_stream << "n_test_samples = " << n_test_samples << endl;
   
   filefunc::closefile(params_filename, params_stream);
}

// ---------------------------------------------------------------------
void neural_net::compute_bias_distributions()
{
   for(int l = 0; l < n_layers; l++)
   {
      vector<double> bias_values;
      for(unsigned int r = 0; r < biases[l]->get_mdim(); r++)
      {
         bias_values.push_back(biases[l]->get(r));
      }

      double blo = mathfunc::minimal_value(bias_values);
      double bhi = mathfunc::maximal_value(bias_values);
      if(nearly_equal(blo,bhi))
      {
         bias_01[l].push_back(blo);
         bias_05[l].push_back(blo);
         bias_10[l].push_back(blo);
         bias_25[l].push_back(blo);
         bias_35[l].push_back(blo);
         bias_50[l].push_back(blo);
         bias_65[l].push_back(blo);
         bias_75[l].push_back(blo);
         bias_90[l].push_back(blo);
         bias_95[l].push_back(blo);
         bias_99[l].push_back(blo);
      }
      else
      {
         int nbins = 500;
         prob_distribution prob_biases(nbins, blo, bhi, bias_values);
         bias_01[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.01));
         bias_05[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.05));
         bias_10[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.10));
         bias_25[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.25));
         bias_35[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.35));
         bias_50[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.50));
         bias_65[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.65));
         bias_75[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.75));
         bias_90[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.90));
         bias_95[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.95));
         bias_99[l].push_back(prob_biases.find_x_corresponding_to_pcum(0.99));
      }
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
void neural_net::compute_weight_distributions()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      vector<double> weight_values;
      for(unsigned int r = 0; r < weights[l]->get_mdim(); r++)
      {
         for(unsigned int c = 0; c < weights[l]->get_ndim(); c++)
         {
            weight_values.push_back(weights[l]->get(r,c));
         }
      }
      int nbins = 500;
      double wlo = mathfunc::minimal_value(weight_values);
      double whi = mathfunc::maximal_value(weight_values);
      prob_distribution prob_weights(nbins, wlo, whi, weight_values);

      weight_01[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.01));
      weight_05[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.05));
      weight_10[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.10));
      weight_25[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.25));
      weight_35[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.35));
      weight_50[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.50));
      weight_65[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.65));
      weight_75[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.75));
      weight_90[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.90));
      weight_95[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.95));
      weight_99[l].push_back(prob_weights.find_x_corresponding_to_pcum(0.99));
      
//      cout << "layer = " << l
//           << " wlo = " << wlo
//           << " w_05 = " << weight_05[l].back()
//           << " w_25 = " << weight_25[l].back();
//      cout << "   w_50 = " << weight_50[l].back()
//           << " w_75 = " << weight_75[l].back()
//           << " w_95 = " << weight_95[l].back()
//           << " whi = " << whi
//           << endl;
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
void neural_net::store_quasirandom_weight_values()
{
   for(int l = 0; l < n_layers - 1; l++)
   {
      vector<double> weight_values;
      int n_weights_for_layer = weights[l]->get_mdim() * 
         weights[l]->get_ndim();
      int weight_skip = n_weights_for_layer / 9;
      for(unsigned int i = 1; i <= 9; i++)
      {
         int weight_index = i * weight_skip;
         int r = weight_index / weights[l]->get_ndim();
         int c = weight_index % weights[l]->get_ndim();
         double curr_w = weights[l]->get(r,c);

         if(i == 1)
         {
            weight_1[l].push_back(curr_w);
         }
         else if (i == 2)
         {
            weight_2[l].push_back(curr_w);
         }
         else if (i == 3)
         {
            weight_3[l].push_back(curr_w);
         }
         else if (i == 4)
         {
            weight_4[l].push_back(curr_w);
         }
         else if (i == 5)
         {
            weight_5[l].push_back(curr_w);
         }
         else if (i == 6)
         {
            weight_6[l].push_back(curr_w);
         }
         else if (i == 7)
         {
            weight_7[l].push_back(curr_w);
         }
         else if (i == 8)
         {
            weight_8[l].push_back(curr_w);
         }
         else if (i == 9)
         {
            weight_9[l].push_back(curr_w);
         }
      }
   } // loop over index l labeling network layers
}

// ---------------------------------------------------------------------
string neural_net::init_subtitle()
{
   string subtitle=
      "blr="+stringfunc::scinumber_to_string(base_learning_rate,2)+
      "; lambda="+stringfunc::scinumber_to_string(lambda,2)+
      "; batch size="+stringfunc::number_to_string(mini_batch_size)+
      "; ";
   if(solver_type == SGD)
   {
      subtitle += "SGD";
   }
   else if(solver_type == RMSPROP)
   {
      subtitle += "RMSPROP;";
      subtitle += " decay="+stringfunc::scinumber_to_string(
         rmsprop_decay_rate,2);
   }
   else if(solver_type == MOMENTUM)
   {
      subtitle += "MOMENTUM";
   }
   else if(solver_type == NESTEROV)
   {
      subtitle += "NESTEROV";
   }
   else if(solver_type == ADAM)
   {
      subtitle += "ADAM";
//      subtitle += " b1="+stringfunc::scinumber_to_string(beta1,2);
//      subtitle += " b2="+stringfunc::scinumber_to_string(beta2,2);
   }
   return subtitle;
}

// ---------------------------------------------------------------------
// Generate metafile plot of input STL vector of values plotted
// against either epoch or episode independent variables.  The total
// number of independent and dependent variables do NOT need to be
// equal.  

bool neural_net::generate_metafile_plot(
   const vector<double>& values, string metafile_basename, string title,
   string y_label, string extrainfo, 
   bool plot_smoothed_values_flag, bool zero_min_value_flag)
{
   if(epoch_history.size() < 3 || values.size() < 3) return false;

   metafile curr_metafile;
   string meta_filename=output_subdir+metafile_basename;
   title += "; nweights="+stringfunc::number_to_string(count_weights());
   string subtitle=init_subtitle() + " " + extrainfo;
   string x_label="Epoch";
   double xmax = epoch_history.back();

   double min_value = mathfunc::minimal_value(values);
   if(zero_min_value_flag)
   {
      min_value = 0;
   }
   double max_value = mathfunc::maximal_value(values);

   if(nearly_equal(min_value, max_value))
   {
      double avg_value = 0.5 * (min_value + max_value);
      min_value = 0.5 * avg_value;
      max_value = 1.5 * avg_value;
   }
   
   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, 
      min_value, max_value);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.openmetafile();
   curr_metafile.write_header();

   curr_metafile.write_curve(epoch_history, values);
   curr_metafile.set_thickness(3);

// Temporally smooth noisy input values:

   if(plot_smoothed_values_flag)
   {
      int n_values = values.size();
      double sigma = 10;
      if(n_values > 100)
      {
         sigma += log10(values.size())/log10(2.0);
      }
      double dx = 1;
      int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx, 3.0);

      if(gaussian_size < n_values)
      {
         vector<double> h;
         h.reserve(gaussian_size);
         filterfunc::gaussian_filter(dx, sigma, h);

         bool wrap_around_input_values = false;
         vector<double> smoothed_values;
         filterfunc::brute_force_filter(
            values, h, smoothed_values, wrap_around_input_values);
         curr_metafile.write_curve(
            epoch_history, smoothed_values, colorfunc::blue);
      }
   } // plot_smoothed_values_flag conditional
   
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
   return true;
}

// ---------------------------------------------------------------------
// Generate metafile plot of averaged minibatch loss versus training epoch.

void neural_net::plot_loss_history()
{
   string metafile_basename="loss";
   string title="Loss vs model training";
   string extrainfo = "";
   string y_label="Averaged minibatch loss";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = true;

   generate_metafile_plot(
      avg_minibatch_loss, metafile_basename, 
      title, y_label, extrainfo, 
      plot_smoothed_values_flag, zero_min_value_flag);
}

// ---------------------------------------------------------------------
// Generate metafile plot of log10(lr * mean_abs_nabla_weight_ratios)
// versus epoch for each individual network layer.

void neural_net::plot_log10_lr_mean_abs_nabla_weight_ratios()
{
   string title="learning rate * <|nabla_weight_ratio|>";
   string y_label="log10(learning rate * <|nabla_weight_ratio|>)";
   bool plot_smoothed_values_flag = true;
   bool zero_min_value_flag = false;

   for(unsigned int l = 0; l < log10_lr_mean_abs_nabla_weight_ratios.size();
       l++)
   {
      string metafile_basename = "lr_nabla_weight_ratios_"+
         stringfunc::number_to_string(l);
      string extrainfo = "Layer "+stringfunc::number_to_string(l);
      generate_metafile_plot(
         log10_lr_mean_abs_nabla_weight_ratios[l], metafile_basename, 
         title, y_label, extrainfo, 
         plot_smoothed_values_flag, zero_min_value_flag);
   }
}

// ---------------------------------------------------------------------
// Generate metafile plot of training and testing set accuracies vs
// epoch.

void neural_net::plot_accuracies_history()
{
   if(test_accuracy_history.size() < 3) return;

   metafile curr_metafile;
   string meta_filename=output_subdir + "accuracies";
   string title="Training and testing samples accuracy vs training epoch";
   string subtitle=init_subtitle();
   string x_label="Epoch";
   string y_label="Model accuracy";
   double xmax = epoch_history.back();
   double min_accuracy = 0;
   double max_accuracy = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, xmax, 
      min_accuracy, max_accuracy);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();

   curr_metafile.write_curve(
      epoch_history, test_accuracy_history, colorfunc::red);
   curr_metafile.write_curve(
      epoch_history, training_accuracy_history, colorfunc::blue);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of bias distributions versus episode number.

bool neural_net::plot_bias_distributions(string extrainfo)
{
   if(bias_50[0].size() < 5) return false;
   
   string script_filename=output_subdir + "view_bias_dists";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 1; l < bias_50.size(); l++)
   {
      string basename="bias_dists_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;
      string jpg_filename=basename+".jpg";

      string title="Bias dists for layer "+stringfunc::number_to_string(l);
      string subtitle=init_subtitle();
      subtitle += ";"+extrainfo;
      string x_label="Epoch";
      double xmax = epoch_history.back();
      string y_label="Bias distributions";

      double max_bias = NEGATIVEINFINITY;
      double min_bias = POSITIVEINFINITY; 
      max_bias = basic_math::max(
         max_bias, mathfunc::maximal_value(bias_99[l]));
      min_bias = basic_math::min(
         min_bias, mathfunc::minimal_value(bias_01[l]));

      metafile curr_metafile;
      curr_metafile.set_parameters(
         meta_filename, title, x_label, y_label, 0, xmax, min_bias, max_bias);
      curr_metafile.set_subtitle(subtitle);
      curr_metafile.openmetafile();
      curr_metafile.write_header();
      curr_metafile.set_thickness(2);

      curr_metafile.write_curve(
         epoch_history, bias_01[l], colorfunc::get_color(0));
      curr_metafile.write_curve(
         epoch_history, bias_05[l], colorfunc::get_color(1));
      curr_metafile.write_curve(
         epoch_history, bias_10[l], colorfunc::get_color(2));
      curr_metafile.write_curve(
         epoch_history, bias_25[l], colorfunc::get_color(3));
      curr_metafile.write_curve(
         epoch_history, bias_35[l], colorfunc::get_color(4));
      curr_metafile.write_curve(
         epoch_history, bias_50[l], colorfunc::get_color(5));
      curr_metafile.write_curve(
         epoch_history, bias_65[l], colorfunc::get_color(6));
      curr_metafile.write_curve(
         epoch_history, bias_75[l], colorfunc::get_color(7));
      curr_metafile.write_curve(
         epoch_history, bias_90[l], colorfunc::get_color(8));
      curr_metafile.write_curve(
         epoch_history, bias_95[l], colorfunc::get_color(9));
      curr_metafile.write_curve(
         epoch_history, bias_99[l], colorfunc::get_color(10));
      
      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);
      script_stream << "view "+jpg_filename << endl;

   } // loop over index l labeling network layers

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
   return true;
}

// ---------------------------------------------------------------------
// Generate metafile plot of weight distributions versus episode number.

bool neural_net::plot_weight_distributions(string extrainfo)
{
   if(weight_50[0].size() < 5) return false;

   string script_filename=output_subdir + "view_weight_dists";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 0; l < weight_50.size(); l++)
   {
      metafile curr_metafile;
      string basename="weight_dists_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;

      string title="Weight dists for layer "+stringfunc::number_to_string(l);
      string subtitle=init_subtitle();
      subtitle += ";"+extrainfo;
      string x_label="Epoch";
      double xmax = epoch_history.back();
      string y_label="Weight distributions";

      double max_weight = NEGATIVEINFINITY;
      double min_weight = POSITIVEINFINITY; 
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_99[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_01[l]));

      curr_metafile.set_parameters(
         meta_filename, title, x_label, y_label, 0, xmax, 
         min_weight, max_weight);
      curr_metafile.set_subtitle(subtitle);
      curr_metafile.openmetafile();
      curr_metafile.write_header();
      curr_metafile.set_thickness(2);

      curr_metafile.write_curve(
         epoch_history, weight_01[l], colorfunc::get_color(0));
      curr_metafile.write_curve(
         epoch_history, weight_05[l], colorfunc::get_color(1));
      curr_metafile.write_curve(
         epoch_history, weight_10[l], colorfunc::get_color(2));
      curr_metafile.write_curve(
         epoch_history, weight_25[l], colorfunc::get_color(3));
      curr_metafile.write_curve(
         epoch_history, weight_35[l], colorfunc::get_color(4));
      curr_metafile.write_curve(
         epoch_history, weight_50[l], colorfunc::get_color(5));
      curr_metafile.write_curve(
         epoch_history, weight_65[l], colorfunc::get_color(6));
      curr_metafile.write_curve(
         epoch_history, weight_75[l], colorfunc::get_color(7));
      curr_metafile.write_curve(
         epoch_history, weight_90[l], colorfunc::get_color(8));
      curr_metafile.write_curve(
         epoch_history, weight_95[l], colorfunc::get_color(9));
      curr_metafile.write_curve(
         epoch_history, weight_99[l], colorfunc::get_color(10));
      
      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      string jpg_filename=basename+".jpg";
      script_stream << "view "+jpg_filename << endl;

   } // loop over index l labeling network layers

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
   return true;
}

// ---------------------------------------------------------------------
// Generate metafile plot of quasi-random weight values versus episode number.

bool neural_net::plot_quasirandom_weight_values(string extrainfo)
{
   if(weight_1[0].size() < 5) return false;

   string script_filename=output_subdir + "view_weight_values";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   for(unsigned int l = 0; l < weight_50.size(); l++)
   {
      metafile curr_metafile;
      string basename="weight_values_"+stringfunc::number_to_string(l);
      string meta_filename=output_subdir + basename;
      string title="Quasi random weight values for layer "
         +stringfunc::number_to_string(l);

      string subtitle=init_subtitle();
      subtitle += ";"+extrainfo;
      string x_label="Epoch";
      double xmax = epoch_history.back();
      string y_label="Weight values";

      double max_weight = NEGATIVEINFINITY;
      double min_weight = POSITIVEINFINITY; 
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_1[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_2[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_3[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_4[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_5[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_6[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_7[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_8[l]));
      max_weight = basic_math::max(
         max_weight, mathfunc::maximal_value(weight_9[l]));

      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_1[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_2[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_3[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_4[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_5[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_6[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_7[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_8[l]));
      min_weight = basic_math::min(
         min_weight, mathfunc::minimal_value(weight_9[l]));

      curr_metafile.set_parameters(
         meta_filename, title, x_label, y_label, 0, xmax,
         min_weight, max_weight);
      curr_metafile.set_subtitle(subtitle);
      curr_metafile.openmetafile();
      curr_metafile.write_header();
      curr_metafile.set_thickness(2);

      curr_metafile.write_curve(
         epoch_history, weight_1[l], colorfunc::get_color(0));
      curr_metafile.write_curve(
         epoch_history, weight_2[l], colorfunc::get_color(1));
      curr_metafile.write_curve(
         epoch_history, weight_3[l], colorfunc::get_color(2));
      curr_metafile.write_curve(
         epoch_history, weight_4[l], colorfunc::get_color(3));
      curr_metafile.write_curve(
         epoch_history, weight_5[l], colorfunc::get_color(4));
      curr_metafile.write_curve(
         epoch_history, weight_6[l], colorfunc::get_color(5));
      curr_metafile.write_curve(
         epoch_history, weight_7[l], colorfunc::get_color(6));
      curr_metafile.write_curve(
         epoch_history, weight_8[l], colorfunc::get_color(7));
      curr_metafile.write_curve(
         epoch_history, weight_9[l], colorfunc::get_color(8));

      curr_metafile.closemetafile();
      string banner="Exported metafile "+meta_filename+".meta";
      outputfunc::write_banner(banner);

      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      string jpg_filename=basename+".jpg";
      script_stream << "view "+jpg_filename << endl;
   } // loop over index l labeling network layers

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
   return true;
}

// ---------------------------------------------------------------------
// Member function generate_summary_plots() outputs metafile plots of
// loss, training/testing accuracies, bias and weight distribution,
// and random weight value histories.

void neural_net::generate_summary_plots(string extrainfo)
{
//   plot_lr_history(output_subdir, extrainfo, epoch_indep_var);

   plot_loss_history();
   plot_log10_lr_mean_abs_nabla_weight_ratios();
   plot_accuracies_history();
   if(include_bias_terms)
   {
      plot_bias_distributions(extrainfo);
   }
   plot_weight_distributions(extrainfo);
   plot_quasirandom_weight_values(extrainfo);
   //   filefunc::purge_files_with_suffix_in_subdir(output_subdir, "ps");
   generate_view_metrics_script();
}

// ---------------------------------------------------------------------
// Member function generate_view_metrics_script creates an executable
// script which displays reward, nframes/episode, max Q and epsilon
// history metafile outputs.

void neural_net::generate_view_metrics_script()
{
   string script_filename=output_subdir + "view_metrics";
   ofstream script_stream;
   filefunc::openfile(script_filename, script_stream);

   script_stream << "view loss.jpg" << endl;
   script_stream << "view accuracies.jpg" << endl;
   for(unsigned int l = 0; l < log10_lr_mean_abs_nabla_weight_ratios.size(); 
       l++)
   {
      script_stream << "view lr_nabla_weight_ratios_"+
         stringfunc::number_to_string(l)+".jpg" << endl;
   }

   filefunc::closefile(script_filename, script_stream);
   filefunc::make_executable(script_filename);
}
