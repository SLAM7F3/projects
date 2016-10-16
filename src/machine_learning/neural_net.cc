// ==========================================================================
// neural_net class member function definitions
// ==========================================================================
// Last modified on 2/8/16; 2/9/16
// ==========================================================================

#include <iostream>
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "machine_learning/neural_net.h"
#include "numrec/nrfuncs.h"

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

void neural_net::allocate_member_objects()
{
}		       

void neural_net::initialize_member_objects()
{
}		       

// Input STL vector sizes contains the number of neurons in the
// respective layers of the network.  For example, if sizes == [2, 3,
// 1] then the network will have 3 layers with the first layer
// containing 2 neurons, the second layer 3 neurons, and the third
// layer 1 neuron.

neural_net::neural_net(const vector<int>& sizes)
{
   allocate_member_objects();
   initialize_member_objects();

   num_layers = sizes.size();
   for(unsigned int l = 0; l < num_layers; l++)
   {
      layer_sizes.push_back(sizes[l]);
   }
   n_classes = layer_sizes.back();

   for(unsigned int l = 0; l < num_layers; l++)
   {
      genvector *curr_biases = new genvector(layer_sizes[l]);
      biases.push_back(curr_biases);
      genvector *curr_nabla_biases = new genvector(layer_sizes[l]);
      nabla_biases.push_back(curr_nabla_biases);
      genvector *curr_delta_nabla_biases = new genvector(layer_sizes[l]);
      delta_nabla_biases.push_back(curr_delta_nabla_biases);

// Initialize bias for each network node in layers 1, 2, ... to be
// gaussian random var distributed according to N(0,1).  Recall input
// layer has no biases:

      for(unsigned int i = 0; i < layer_sizes[l]; i++)
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
    
      if(l == num_layers-1) continue;
      
      genmatrix *curr_weights = new genmatrix(
         layer_sizes[l+1], layer_sizes[l]);
      weights.push_back(curr_weights);
      genmatrix *curr_nabla_weights = new genmatrix(
         layer_sizes[l+1], layer_sizes[l]);
      nabla_weights.push_back(curr_nabla_weights);
      genmatrix *curr_delta_nabla_weights = new genmatrix(
         layer_sizes[l+1], layer_sizes[l]);
      delta_nabla_weights.push_back(curr_delta_nabla_weights);

// Initialize weights connecting network layers l and l+1 to be
// gaussian random vars distributed according to N(0,1/sqrt(n_in)):

      for(unsigned int i = 0; i < layer_sizes[l+1]; i++)
      {
         for(unsigned int j = 0; j < layer_sizes[l]; j++)
         {
            curr_weights->put(i, j, nrfunc::gasdev() / sqrt(layer_sizes[l]) );
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
   }
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const neural_net& NN)
{
   outstream << endl;
   outstream << "n_layers = " << NN.num_layers << endl << endl;
   for(unsigned int l = 0; l < NN.num_layers; l++)
   {
      cout << "---------------------------" << endl;
      if(l == 0)
      {
         outstream << "INPUT Layer: l = 0" << endl;
      
      }
      else if (l == NN.num_layers - 1)
      {
         outstream << "OUTPUT Layer l = " << l << endl;

      }
      else
      {
         outstream << "Layer: l = " << l << endl;
      }
      cout << "  N_nodes = " << NN.layer_sizes[l] << endl;
      genvector* curr_biases = NN.get_biases(l);
      cout << "biases = " << *curr_biases << endl;
      cout << "---------------------------" << endl;      

      if(l == NN.num_layers-1) continue;
      genmatrix* curr_weights = NN.get_weights(l);
      cout << curr_weights->get_mdim() << " x " << curr_weights->get_ndim()
           << " weights matrix connecting layers " 
           << l << " to " << l+1 << " :" << endl;
      cout << *curr_weights << endl;

   } // loop over index l labeling neural net layer

   return outstream;
}

// ==========================================================================

// Member function feedforward returns the output of the network given
// an input set of values.

genvector neural_net::feedforward(const genvector& a_input)
{
   genvector* a_curr = new genvector(a_input);

   for(unsigned int l = 0; l < num_layers-1; l++)
   {
      genmatrix* curr_weights = weights[l];
      genvector* curr_biases = biases[l+1];
      genvector z_curr( (*curr_weights) * (*a_curr) + *curr_biases );

      delete a_curr;
      a_curr = new genvector(machinelearning_func::sigmoid(z_curr));
   }
   
   genvector a_output(*a_curr);
   delete a_curr;
   return a_output;
}

// ---------------------------------------------------------------------
double neural_net::evaluate()
{
   int n_correct_predictions = 0;
   int n_predicted_0 = 0;
   int n_predicted_1 = 0;
   for(unsigned int t = 0; t < n_test_samples; t++)
   {
      genvector Y_predicted = feedforward( *(test_data[t].first) );
      double max_score = NEGATIVEINFINITY;
      int i_max = -1;
      for(unsigned int i = 0; i < n_classes; i++)
      {
         if(Y_predicted.get(i) > max_score)
         {
            max_score = Y_predicted.get(i);
            i_max = i;
         }
      }

      if(i_max == 0)
      {
         n_predicted_0++;
      }
      else if (i_max == 1)
      {
         n_predicted_1++;
      }
      
      
      if(i_max == test_data[t].second)
      {
         n_correct_predictions++;
      }
   } // loop over index t labeling test samples

   cout << "n_predicted_0 = " << n_predicted_0
        << " n_predicted_1 = " << n_predicted_1 << endl;
   
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
   int mini_batch_size, const vector<DATA_PAIR>& shuffled_training_data)
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
                     double lambda)
{
   cout << "   Test frac correct = " << evaluate() << endl;

   for(int e = 0; e < n_epochs; e++)
   {
      if(e%10 == 0)
      {
         cout << "Epoch e = " << e << " of " << n_epochs << endl;
         cout << "   learning_rate = " << learning_rate 
              << "  regularization lambda = " << lambda << endl;
         cout << "   Test frac correct = " << evaluate() << endl;
//      cout << "   *this = " << *this << endl;
//      outputfunc::enter_continue_char();
      }
      

      vector<DATA_PAIR> shuffled_training_data = 
         randomly_shuffle_training_data();
      vector<vector<DATA_PAIR> > mini_batches = generate_training_mini_batches(
         mini_batch_size, shuffled_training_data);

      for(unsigned int b = 0; b < mini_batches.size(); b++)
      {
         update_mini_batch(mini_batches[b], learning_rate, lambda);
      } // loop over index b labeling mini batches

   } // loop over index e labeling training epochs
}

// ---------------------------------------------------------------------
// Member function update_mini_batch() updates the network's weights
// and biases by applying gradient descent using backpropagation to a
// single mini batch.  lambda = L2 regularization parameter.

void neural_net::update_mini_batch(
   vector<DATA_PAIR>& mini_batch, double learning_rate, double lambda)
{

// Initialize cumulative (over mini batch) weight and bias gradients
// to zero:

   for(unsigned int b = 0; b < nabla_biases.size(); b++)
   {
      nabla_biases[b]->clear_values();
   }
   for(unsigned int w = 0; w < nabla_weights.size(); w++)
   {
      nabla_weights[w]->clear_values();
   }

   for(unsigned int i = 0; i < mini_batch.size(); i++)
   {
      backpropagate(mini_batch[i]);

// Accumulate weights and bias gradients:

      for(unsigned int b = 0; b < nabla_biases.size(); b++)
      {
         *nabla_biases[b] = *nabla_biases[b] + *delta_nabla_biases[b];
      }
      for(unsigned int w = 0; w < nabla_weights.size(); w++)
      {
         *nabla_weights[w] = *nabla_weights[w] + *delta_nabla_weights[w];
      }
   } // loop over index i labeling training samples within mini batch

// Update weights and biases from this mini-batch

   for(unsigned int b = 0; b < biases.size(); b++)
   {
      *biases[b] = *biases[b] - learning_rate/mini_batch.size() * 
         (*nabla_biases[b]);
   }

   for(unsigned int w = 0; w < weights.size(); w++)
   {
      *weights[w] = (1 - learning_rate*lambda/n_training_samples) *
         (*weights[w]) - learning_rate/mini_batch.size() * (*nabla_weights[w]);
   }
}

// ---------------------------------------------------------------------
// Member function backpropagate() takes in one training sample within
// curr_data_pair.  It returns delta_nabla_biases and
// delta_nabla_weights representing the gradient for cost function
// C_x.  delta_nabla_biases and delta_nabla_weights are layer-by-layer
// sets of genvectors similar to biases and weights.

void neural_net::backpropagate(const DATA_PAIR& curr_data_pair)
{
//   cout << "inside neural_net::backpropagate()" << endl;

// Initialize "instantaneous" (i.e. just for curr_data_pair) weight
// and bias gradients to zero:

   for(unsigned int b = 0; b < delta_nabla_biases.size(); b++)
   {
      delta_nabla_biases[b]->clear_values();
   }
   for(unsigned int w = 0; w < delta_nabla_weights.size(); w++)
   {
      delta_nabla_weights[w]->clear_values();
   }

// Forward pass:

//   cout << "Performing forward pass" << endl;
   
   vector<genvector> Zs;  
   // Holds Z for layer 1, layer 2, ... layer (num_layers-1)

   vector<genvector> activations;
   // Holds activation for layer 0, layer 1, ..., layer (num_layers-1)

   activations.push_back(*curr_data_pair.first);

   for(unsigned int l = 0; l < num_layers - 1; l++)
   {
      genmatrix* curr_weights = get_weights(l);
      genvector* curr_biases = get_biases(l+1);
      genvector curr_Z((*curr_weights) * activations.back() + (*curr_biases));
      Zs.push_back(curr_Z);
      activations.push_back( machinelearning_func::sigmoid( curr_Z ));
   }
   
// Backward pass:

//   cout << "Performing backward pass" << endl;

   int y = basic_math::round(curr_data_pair.second);
   genvector delta(
      cost_derivative(activations.back(), y).hadamard_product(
         machinelearning_func::deriv_sigmoid(Zs.back() )) );

   *(delta_nabla_biases[num_layers-1]) = delta;
   *(delta_nabla_weights[num_layers-2]) = delta.outerproduct(
      activations[num_layers-2]);

   genvector *curr_delta=new genvector(delta);
   for(int l = num_layers - 2; l >= 1; l--)
   {
      genvector curr_Z ( Zs[l] );
      genvector sp(machinelearning_func::deriv_sigmoid(curr_Z));

      genvector next_delta( 
         get_weights(l)->transpose() * (*curr_delta) );
      next_delta = next_delta.hadamard_product(sp);

      delete curr_delta;
      curr_delta = new genvector(next_delta);

      *(delta_nabla_biases[l]) = *curr_delta;
      *(delta_nabla_weights[l-1]) = curr_delta->outerproduct(
         activations[l-1]);
      
   } // loop over index l labeling network layers in backwards order
   
   delete curr_delta;
}

// ---------------------------------------------------------------------
// Member function cost_derivative() returns vector of partial derivatives
// partial C_x / partial a for the output activations.

genvector neural_net::cost_derivative(genvector& output_activation, int y)
{
   genvector cost_deriv(output_activation);
   cost_deriv.put(y, cost_deriv.get(y) - 1);
   return cost_deriv;
}

