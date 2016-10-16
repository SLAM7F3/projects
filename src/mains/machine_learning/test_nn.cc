// ==========================================================================
// Program TEST_NN
// ==========================================================================
// Last updated on 10/15/16; 10/16/16
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
#include "machine_learning/neural_net.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::ifstream;
using std::ofstream;

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   nrfunc::init_time_based_seed();

   int Din = 1;   	// Number of input layer nodes
   int H = 5;		// Number of single hidden layer nodes
   int Dout = 2;   	// Number of output layer nodes

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H);
   layer_dims.push_back(Dout);
   
   neural_net NN(layer_dims);
   cout << "NN = " << NN << endl;

// Binary classification experiment #1:  

//    Training sample X_i is an integer
//    Desired labels:  0 <--> X_i is even
//                     1 <--> X_i is odd   
   
//   int n_training_samples = 500;
   int n_training_samples = 500;
   int n_testing_samples = 0.1 * n_training_samples;
   int mini_batch_size = 5;

   vector<neural_net::DATA_PAIR> training_samples;
   vector<neural_net::DATA_PAIR> testing_samples;

   machinelearning_func::generate_data_samples(
      n_training_samples, training_samples);
   machinelearning_func::generate_data_samples(
      n_testing_samples, testing_samples);
   machinelearning_func::remove_data_samples_mean(training_samples);
   machinelearning_func::remove_data_samples_mean(testing_samples);

   NN.import_training_data(training_samples);
   NN.import_test_data(testing_samples);

/*
   cout << "Training samples" << endl;
   machinelearning_func::print_data_samples(training_samples);
   cout << "Testing samples" << endl;
   machinelearning_func::print_data_samples(testing_samples);
*/

/*
   vector<neural_net::DATA_PAIR> shuffled_training_samples = 
      NN.randomly_shuffle_training_data();   
//   machinelearning_func::print_data_samples(shuffled_training_samples);

   vector< vector<neural_net::DATA_PAIR> > mini_batches = 
      NN.generate_training_mini_batches(mini_batch_size,
                                        shuffled_training_samples);


   for(unsigned int b = 0; b < mini_batches.size(); b++)
   {
      cout << "b = " << b << "  ----------------" << endl;
      machinelearning_func::print_data_samples(mini_batches[b]);
   }
*/

   int n_epochs = 100;
   double learning_rate = 0.001;
   double lambda = 0.1;	
   NN.sgd(n_epochs, mini_batch_size, learning_rate, lambda);
}

