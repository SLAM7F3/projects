// ==========================================================================
// Program TRAIN_NN is a playground for training our neural_nets
// using simulated 2D spiral training data.
// ==========================================================================
// Last updated on 1/16/17; 1/18/17; 1/19/17; 1/23/17
// ==========================================================================

#include <stdint.h>
#include <byteswap.h>
#include <iostream>
#include <string>
#include <vector>

#include "math/constants.h"
#include "general/filefuncs.h"
#include "math/genvector.h"
#include "machine_learning/machinelearningfuncs.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "machine_learning/neural_net.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::ifstream;
using std::ofstream;

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();
   long seed = nrfunc::init_time_based_seed();
//   long seed = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> seed;
//   nrfunc::init_default_seed(seed);

   int Din = 2;   	// Number of input layer nodes
//   int H1 = 3;		// Number of first hidden layer nodes
   int H1 = 10;		// Number of first hidden layer nodes
//   int H2 = 0;
   int H2 = 10;
   int H3 = 0;
//   int H3 = 4;
   int Dout = 2;   	// Number of output layer nodes

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   if(H2 > 0)
   {
      layer_dims.push_back(H2);
   }
   if(H3 > 0)
   {
      layer_dims.push_back(H3);
   }
   layer_dims.push_back(Dout);

   int mini_batch_size = 32;
//   double lambda = 0;  // L2 regularization coefficient
   double lambda = 1E-3;  // L2 regularization coefficient
   double rmsprop_decay_rate = 0.95;

   neural_net NN(mini_batch_size, lambda, rmsprop_decay_rate, layer_dims);
   NN.set_base_learning_rate(1E-2);
//   NN.set_include_bias_terms(false);
   NN.set_include_bias_terms(true);
   NN.set_output_subdir("./nn_output/");

//   int n_training_samples = 20; // Loss-->0 when lambda-->0 
				// for small training set
   int n_training_samples = 2000;
   int n_validation_samples = 0.1 * n_training_samples;

   vector<neural_net::DATA_PAIR> training_samples;
   vector<neural_net::DATA_PAIR> validation_samples;

/*
   machinelearning_func::generate_2d_circular_data_samples(
      n_training_samples, training_samples);
   machinelearning_func::generate_2d_circular_data_samples(
      n_validation_samples, validation_samples);
*/

   machinelearning_func::generate_2d_spiral_data_samples(
      n_training_samples, training_samples);
   machinelearning_func::generate_2d_spiral_data_samples(
      n_validation_samples, validation_samples);

   NN.import_training_data(training_samples);
   NN.import_validation_data(validation_samples);
   NN.summarize_parameters();

   int n_epochs = 200;
   
   NN.train_network(n_epochs);
   NN.plot_loss_history();
   NN.plot_accuracies_history();
   string snapshot_filename = NN.export_snapshot();

   double training_accuracy = NN.evaluate_model_on_training_set();
   vector<int> incorrect_training_classifications = 
      NN.get_incorrect_classifications();

   double validation_accuracy = NN.evaluate_model_on_validation_set();
   vector<int> incorrect_validation_classifications = 
      NN.get_incorrect_classifications();

   cout << "training_samples.size = " << training_samples.size() << endl;
   cout << "validation_samples.size = " << validation_samples.size() << endl;
   cout << "Number of training samples incorrectly classified = "
        << incorrect_training_classifications.size() << endl;
   cout << "Number of validation samples incorrectly classified = "
        << incorrect_validation_classifications.size() << endl;
   cout << "Training accuracy = " << training_accuracy << endl;
   cout << "Validation accuracy = " << validation_accuracy << endl;
}

