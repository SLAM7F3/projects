// ==========================================================================
// Program TEST_NN
// ==========================================================================
// Last updated on 10/17/16; 12/27/16; 1/15/17; 1/16/17
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
   int H1 = 10;		// Number of first hidden layer nodes
   int H2 = 10;
   int H3 = 4;
   int Dout = 2;   	// Number of output layer nodes

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   layer_dims.push_back(H2);
   layer_dims.push_back(H3);
   layer_dims.push_back(Dout);

   int mini_batch_size = 20;
   double lambda = 1E-3;  // L2 regularization coefficient
   double rmsprop_decay_rate = 0.95;

   neural_net NN(mini_batch_size, lambda, rmsprop_decay_rate, layer_dims);
   NN.set_base_learning_rate(1E-2);

   int n_training_samples = 2000;
//   int n_training_samples = 200;
   int n_testing_samples = 0.1 * n_training_samples;


   vector<neural_net::DATA_PAIR> training_samples;
   vector<neural_net::DATA_PAIR> testing_samples;

/*
   machinelearning_func::generate_2d_circular_data_samples(
      n_training_samples, training_samples);
   machinelearning_func::generate_2d_circular_data_samples(
      n_testing_samples, testing_samples);
*/

   machinelearning_func::generate_2d_spiral_data_samples(
      n_training_samples, training_samples);
   machinelearning_func::generate_2d_spiral_data_samples(
      n_testing_samples, testing_samples);

   NN.import_training_data(training_samples);
   NN.import_test_data(testing_samples);

   int n_epochs = 100;
   NN.train_network(n_epochs);
   NN.plot_loss_history();
   NN.plot_accuracies_history();
   vector<int> incorrect_classifications = NN.get_incorrect_classifications();

// Generate metafile plot of training samples, testing samples and
// classification predictions:

   vector<int> labels;
   vector<double> X, Y;
   for(unsigned int i = 0; i < training_samples.size(); i++)
   {
      X.push_back(training_samples[i].first->get(0));
      Y.push_back(training_samples[i].first->get(1));
      labels.push_back(training_samples[i].second);
   }

   for(unsigned int i = 0; i < testing_samples.size(); i++)
   {
      X.push_back(testing_samples[i].first->get(0));
      Y.push_back(testing_samples[i].first->get(1));

      int color_offset = 2;
      for(unsigned int j = 0; j < incorrect_classifications.size(); j++)
      {
         if(int(i) == incorrect_classifications[j])
         {
            color_offset = 4;
            break;
         }
      }
      labels.push_back(testing_samples[i].second + color_offset);
   } // loop over index i labeling data samples
   
   
// Generate metafile output whose markers are colored according to
// class labels:

   metafile curr_metafile;

/*
   string meta_filename="circle";
   string title="Toy circle data classification";
   string x_label="X";
   string y_label="Y";
   double min_val = -2;
   double max_val = 2;
*/

   string meta_filename = NN.get_output_subdir() + "spiral";
   string title="Toy spiral data classification";
   string x_label="X";
   string y_label="Y";
   double min_val = -1;
   double max_val = 1;

   curr_metafile.set_legend_flag(true);
   curr_metafile.set_parameters(
      meta_filename,title,x_label,y_label,
      min_val, max_val, min_val, max_val);
   curr_metafile.openmetafile();
   curr_metafile.write_header();
   curr_metafile.write_markers(labels,X,Y);
   curr_metafile.closemetafile();
   string banner="Exported metafile "+meta_filename+".meta";
   outputfunc::write_banner(banner);
   string unix_cmd="meta_to_jpeg "+meta_filename;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="/bin/rm *.ps";
   sysfunc::unix_command(unix_cmd);

   cout << "training_samples.size = " << training_samples.size() << endl;
   cout << "testing_samples.size = " << testing_samples.size() << endl;
   cout << "Number of testing samples incorrectly classified = "
        << incorrect_classifications.size() << endl;
}

