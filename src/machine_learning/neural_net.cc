// ==========================================================================
// neural_net class member function definitions
// ==========================================================================
// Last modified on 10/18/16; 10/19/16; 10/20/16; 12/27/16
// ==========================================================================

#include <iostream>
#include "filter/filterfuncs.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "plot/metafile.h"
#include "machine_learning/neural_net.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void neural_net::initialize_member_objects(
   const vector<int>& n_nodes_per_layer)
{
   n_layers = n_nodes_per_layer.size();
   for(unsigned int l = 0; l < n_layers; l++)
   {
      layer_dims.push_back(n_nodes_per_layer[l]);
      genvector *curr_z = new genvector(layer_dims.back());
      genvector *curr_a = new genvector(layer_dims.back());
      genvector *curr_delta = new genvector(layer_dims.back());
      z.push_back(curr_z);
      a.push_back(curr_a);
      delta.push_back(curr_delta);
   }
   n_classes = layer_dims.back();
}		       

void neural_net::allocate_member_objects()
{
   for(unsigned int l = 0; l < n_layers; l++)
   {
      
   }
}		       

// Input STL vector sizes contains the number of neurons in the
// respective layers of the network.  For example, if sizes == [2, 3,
// 1] then the network will have 3 layers with the first layer
// containing 2 neurons, the second layer 3 neurons, and the third
// layer 1 neuron.

neural_net::neural_net(const vector<int>& n_nodes_per_layer)
{
   initialize_member_objects(n_nodes_per_layer);
   allocate_member_objects();

   for(unsigned int l = 0; l < n_layers; l++)
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

// Weights link layer l with layer l+1:
    
      if(l == n_layers-1) continue;
      
      genmatrix *curr_weights = new genmatrix(
         layer_dims[l+1], layer_dims[l]);
      weights.push_back(curr_weights);
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
            curr_weights->put(i, j, nrfunc::gasdev() / sqrt(layer_dims[l]) );
         } // loop over index j labeling node in next layer
      } // loop over index i labeling node in current layer

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
   for(unsigned int l = 0; l < z.size(); l++)
   {
      delete z[l];
      delete a[l];
      delete delta[l];
   }
   
   for(unsigned int l = 0; l < biases.size(); l++)
   {
      delete biases[l];
      delete nabla_biases[l];
      delete delta_nabla_biases[l];
   }
   
   for(unsigned int l = 0; l < weights.size(); l++)
   {
      delete weights[l];
      delete nabla_weights[l];
      delete delta_nabla_weights[l];
      delete rmsprop_weights_cache[l];
   }
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream, neural_net& NN)
{
   outstream << endl;
   outstream << "n_layers = " << NN.n_layers << endl << endl;
   for(unsigned int l = 0; l < NN.n_layers; l++)
   {
      cout << "---------------------------" << endl;
      if(l == 0)
      {
         outstream << "INPUT Layer: l = 0" << endl;
      
      }
      else if (l == NN.n_layers - 1)
      {
         outstream << "OUTPUT Layer l = " << l << endl;

      }
      else
      {
         outstream << "Layer: l = " << l << endl;
      }
      cout << "  N_nodes = " << NN.layer_dims[l] << endl;
      genvector* curr_biases = NN.get_biases(l);
      cout << "biases = " << *curr_biases << endl;
      cout << "---------------------------" << endl;      

      if(l == NN.n_layers-1) continue;
      genmatrix* curr_weights = NN.get_weights(l);
      cout << curr_weights->get_mdim() << " x " << curr_weights->get_ndim()
           << " weights matrix connecting layers " 
           << l << " to " << l+1 << " :" << endl;
      cout << *curr_weights << endl;

   } // loop over index l labeling neural net layer

   cout << "Correct test prediction frac = " 
        << NN.evaluate_model_on_test_set()
        << endl;

   return outstream;
}

// ==========================================================================
// Member function feedforward returns the output of the network given
// an input set of values.  

void neural_net::feedforward(genvector* a_input)
{
   a[0] = a_input;
//    cout << "inside feedforward, a_input = " << *a_input << endl;

   for(unsigned int l = 0; l < n_layers-1; l++)
   {
      genmatrix* curr_weights = weights[l];
      genvector* curr_biases = biases[l+1];
      *z[l+1] = (*curr_weights) * (*a[l]) + *curr_biases;

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
      for(unsigned int l = 0; l < n_layers - 1; l++)
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
double neural_net::evaluate_model_on_test_set() 
{
   int n_correct_predictions = 0;
   incorrect_classifications.clear();
   for(unsigned int t = 0; t < n_test_samples; t++)
   {
      feedforward( test_data[t].first );
      genvector* class_probs = get_softmax_class_probs();

      double max_prob = NEGATIVEINFINITY;
      int predicted_class = -1;
      for(unsigned int i = 0; i < n_classes; i++)
      {
         if(class_probs->get(i) > max_prob)
         {
            predicted_class = i;
            max_prob = class_probs->get(i);
         }
      }

      if(predicted_class == test_data[t].second)
      {
         n_correct_predictions++;
      }
      else
      {
         incorrect_classifications.push_back(t);
      }
      
   } // loop over index t labeling test samples

//   cout << "n_correct_predictions = " << n_correct_predictions
//        << " n_test_samples = " << n_test_samples << endl;
   double frac_correct = double(n_correct_predictions) / n_test_samples;
   return frac_correct;
}

// ---------------------------------------------------------------------
void neural_net::import_training_data(const vector<DATA_PAIR>& data)
{
   n_training_samples = data.size();
   for(unsigned int t = 0; t < n_training_samples; t++)
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
   for(unsigned int t = 0; t < n_test_samples; t++)
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
   for(unsigned int i = 0; i < n_training_samples; i++)
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
   unsigned int counter = 0;
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
// Member function sgd() trains the neural network using mini-batch
// stochastic gradient descent.  The "training_data" is a list of
// tuples "(x, y)" representing the training inputs and the desired
// outputs.  The other non-optional parameters are self-explanatory.
// If "test_data" is provided then the network will be evaluated
// against the test data after each epoch, and partial progress
// printed out.  This is useful for tracking progress, but slows
// things down substantially.  Input parameter lambda governs L2
// regularization.

void neural_net::sgd(int n_epochs, int mini_batch_size, double learning_rate,
                     double lambda, double rmsprop_decay_rate)
{
   this->mini_batch_size = mini_batch_size;
   this->learning_rate = learning_rate;
   this->lambda = lambda;
   this->rmsprop_decay_rate = rmsprop_decay_rate;

   for(int e = 0; e < n_epochs; e++)
   {
      if(e%10 == 0)
      {
         cout << "Epoch e = " << e << " of " << n_epochs << endl;
         cout << "   learning_rate = " << learning_rate 
              << "  regularization lambda = " << lambda << endl;
         cout << "   *this = " << *this << endl;
//         outputfunc::enter_continue_char();
      }
      
      vector<DATA_PAIR> shuffled_training_data = 
         randomly_shuffle_training_data();
      vector<vector<DATA_PAIR> > mini_batches = generate_training_mini_batches(
         shuffled_training_data);

      for(unsigned int b = 0; b < mini_batches.size(); b++)
      {
         e_effective.push_back(e + double(b) / mini_batches.size());
         avg_minibatch_loss.push_back(update_mini_batch(mini_batches[b]));
      } // loop over index b labeling mini batches

      test_accuracy_history.push_back(evaluate_model_on_test_set());

   } // loop over index e labeling training epochs
}

// ---------------------------------------------------------------------
// Generate metafile plot of averaged minibatch loss versus epoch.

void neural_net::plot_loss_history()
{
   int n_epochs = e_effective.back();

// Temporally smooth noisy loss values:

   double sigma = 10;
   double dx = 1;
   int gaussian_size = filterfunc::gaussian_filter_size(sigma, dx);
   vector<double> h;
   h.reserve(gaussian_size);
   filterfunc::gaussian_filter(dx, sigma, h);

   bool wrap_around_input_values = false;
   vector<double> smoothed_minibatch_loss;
   filterfunc::brute_force_filter(
      avg_minibatch_loss, h, smoothed_minibatch_loss, 
      wrap_around_input_values);

   metafile curr_metafile;
   string meta_filename="avg_minibatch_loss";
   string title="Loss vs RMSprop model training";
   string subtitle=
      "Base learning rate="+stringfunc::number_to_string(learning_rate,3)+
      "; Weight decay="+stringfunc::number_to_string(lambda,3)+
      "; batch size="+stringfunc::number_to_string(mini_batch_size);
   string x_label="Epoch";
   string y_label="Averaged minibatch loss";
   double min_loss = 0;
   double max_loss = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, n_epochs, min_loss, max_loss);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_curve(e_effective, avg_minibatch_loss, colorfunc::red);
   
   curr_metafile.set_thickness(3);
   curr_metafile.write_curve(e_effective, smoothed_minibatch_loss,
                             colorfunc::blue);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Generate metafile plot of training and testing set accuracies vs
// epoch.

void neural_net::plot_accuracies_history()
{
   int n_epochs = test_accuracy_history.size();
   vector<double> epochs;
   for(int e = 0; e < n_epochs; e++)
   {
      epochs.push_back(e);
   }

   metafile curr_metafile;
   string meta_filename="accuracies";
   string title="Model accuracy vs RMSprop model training";
   string subtitle=
      "Base learning rate="+stringfunc::number_to_string(learning_rate,3)+
      "; Weight decay="+stringfunc::number_to_string(lambda,3)+
      "; batch size="+stringfunc::number_to_string(mini_batch_size);
   string x_label="Epoch";
   string y_label="Model accuracy";
   double min_loss = 0;
   double max_loss = 1;

   curr_metafile.set_parameters(
      meta_filename, title, x_label, y_label, 0, n_epochs, min_loss, max_loss);
   curr_metafile.set_subtitle(subtitle);
   curr_metafile.set_ytic(0.2);
   curr_metafile.set_ysubtic(0.1);
   curr_metafile.openmetafile();
   curr_metafile.write_header();

   curr_metafile.write_curve(epochs, test_accuracy_history, colorfunc::red);

   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);

   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
// Member function update_mini_batch() updates the network's weights
// and biases by applying gradient descent using backpropagation to a
// single mini batch.  lambda = L2 regularization parameter.  This
// method returns the loss averaged over all samples within the
// current mini_batch.

double neural_net::update_mini_batch(vector<DATA_PAIR>& mini_batch)
{

// Initialize cumulative (over mini batch) weight and bias gradients
// to zero:

   for(unsigned int l = 0; l < n_layers; l++)
   {
      nabla_biases[l]->clear_values();
   }
   for(unsigned int l = 0; l < n_layers - 1; l++)
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

      for(unsigned int l = 0; l < n_layers; l++)
      {
         *nabla_biases[l] += *delta_nabla_biases[l] / mini_batch_size;
      }
      for(unsigned int l = 0; l < n_layers - 1; l++)
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

   for(unsigned int l = 0; l < n_layers; l++)
   {
      *biases[l] -= learning_rate * (*nabla_biases[l]);
//      cout << "l = " << l << " biases[l] = " << *biases[l] << endl;
   }

   const double epsilon = 1E-5;
   for(unsigned int l = 0; l < n_layers - 1; l++)
   {
      genmatrix denom = rmsprop_weights_cache[l]->hadamard_power(0.5);
      denom.hadamard_sum(epsilon);
      
// RMSprop
      nabla_weights[l]->hadamard_division(denom);
      *weights[l] -= learning_rate * (*nabla_weights[l]);

// Vanilla SGD
//      *weights[l] -= learning_rate * (*nabla_weights[l]);

//      cout << "l = " << l << " weights[l] = " << *weights[l] << endl;
   }

//   cout << "Correct test prediction frac = " << evaluate_model_on_test_set()
//        << endl;

//   outputfunc::enter_continue_char();
   return avg_minibatch_loss;
}

// ---------------------------------------------------------------------
// Member function clear_delta_nablas() initializes "instantaneous"
// (i.e. just for curr_data_pair) weight and bias gradients to zero:

void neural_net::clear_delta_nablas()
{
   for(unsigned int l = 0; l < n_layers; l++)
   {
      delta_nabla_biases[l]->clear_values();
   }
   for(unsigned int l = 0; l < n_layers - 1; l++)
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

      *delta[prev_layer] = weights[prev_layer]->transpose() * 
         (*delta[curr_layer]);

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
      *(delta_nabla_biases[curr_layer]) = *delta[curr_layer];

// Eqn BP4:

      *(delta_nabla_weights[prev_layer]) = delta[curr_layer]->outerproduct(
         *a[prev_layer]);

// Add L2 regularization contribution to delta_nabla_weights.  No such
// regularization contribution is conventionally added to
// delta_nabla_biases:

      *delta_nabla_weights[prev_layer] += 
         2 * lambda * (*weights[prev_layer]);

   } // loop over curr_layer

// Numerically spot-check loss derivatives wrt a few random
// weights:

   if(nrfunc::ran1() < 1E-2)
   {
      numerically_check_derivs(curr_data_pair);
   }
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
