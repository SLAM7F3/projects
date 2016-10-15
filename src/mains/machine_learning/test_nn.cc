// ==========================================================================
// Program TEST_NN
// ==========================================================================
// Last updated on 10/15/16
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
   int H = 3;		// Number of single hidden layer nodes
   int Dout = 1;   	// Number of output layer nodes

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
   
   int n_training_samples = 100;
   int n_testing_samples = 0.1 * n_training_samples;

   vector<neural_net::DATA_PAIR> training_samples;
   vector<neural_net::DATA_PAIR> testing_samples;

   machinelearning_func::generate_data_samples(
      n_training_samples, training_samples);
   machinelearning_func::generate_data_samples(
      n_testing_samples, testing_samples);


}

