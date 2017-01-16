// ==========================================================================
// Program TRAIN_PI_SIGMA
// ==========================================================================
// Last updated on 1/15/17; 1/16/17
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "machine_learning/neural_net.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "games/tictac3d.h"
#include "time/timefuncs.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock();

   int n_cells = 64;
   vector<neural_net::DATA_PAIR> training_samples;
   vector<neural_net::DATA_PAIR> testing_samples;
   const double testing_sample_frac = 0.1;
   int n_training_samples = 0;
   int n_testing_samples = 0;

   string input_subdir = "./afterstate_action_pairs/";
   string input_filename = input_subdir + "afterstate_action_pairs.txt";
   filefunc::ReadInfile(input_filename);

   vector<vector<string> > line_substrings = 
      filefunc::ReadInSubstrings(input_filename);

   neural_net::DATA_PAIR curr_data_pair;
   for(unsigned int i = 0; i < line_substrings.size(); i++)
   {
      string board_state_str = line_substrings[i].at(0);
      int aplus1 = stringfunc::string_to_integer(line_substrings[i].at(1));
      int player_value = 1;
      if(aplus1 < 0) player_value = -1;
      int a = abs(aplus1) - 1;
      curr_data_pair.second = a;

//      cout << board_state_str << "  "
//           << " player_value = " << player_value
//           << " a = " << a << endl;

// Store player_value = +1 or -1 within last entry in
// *player_board_state_ptr:

      genvector *player_board_state_ptr = new genvector(n_cells + 1);
      curr_data_pair.first = player_board_state_ptr;
      player_board_state_ptr->clear_values();
      
      for(int c = 0; c < n_cells; c++)
      {
         if(c != a)
         {
            if(board_state_str[c] == 'X')
            {
               player_board_state_ptr->put(c, -1);
            }
            else if(board_state_str[c] == 'O')
            {
               player_board_state_ptr->put(c, 1);
            }
         }
      }
      player_board_state_ptr->put(n_cells, player_value);

      if(nrfunc::ran1() < testing_sample_frac)
      {
         testing_samples.push_back(curr_data_pair);
      }
      else
      {
         training_samples.push_back(curr_data_pair);
      }
      
      n_training_samples = training_samples.size();
      n_testing_samples = testing_samples.size();
   } // loop over index i labeling input text lines

   int n_data_samples = n_training_samples + n_testing_samples;
   cout << "n_training_samples = " << n_training_samples
        << " n_testing_samples = " << n_testing_samples
        << " n_data_samples = " << n_data_samples
        << endl;

// Set up neural network:

   int Din = n_cells + 1;   	// Number of input layer nodes
   int H1 = 64;			// Number of first hidden layer nodes
   int H2 = 32;			// Number of 2nd hidden layer nodes
//   int H3 = 16;			// Number of 3rd hidden layer nodes
   int Dout = n_cells;   	// Number of output layer nodes

   vector<int> layer_dims;
   layer_dims.push_back(Din);
   layer_dims.push_back(H1);
   layer_dims.push_back(H2);
//   layer_dims.push_back(H3);
   layer_dims.push_back(Dout);
   neural_net NN(layer_dims);
   
   int mini_batch_size = 20;

   NN.import_training_data(training_samples);
   NN.import_test_data(testing_samples);

   int n_epochs = 100;
   double learning_rate = 1E-3;
   double lambda = 0.001;  // L2 regularization coefficient
   double rmsprop_decay_rate = 0.95;
   NN.train_network(
      n_epochs, mini_batch_size, learning_rate, lambda, rmsprop_decay_rate);
   NN.plot_loss_history();
   NN.plot_accuracies_history();
   vector<int> incorrect_classifications = NN.get_incorrect_classifications();
   double incorrect_testing_frac = incorrect_classifications.size() / 
      n_testing_samples;
   cout << "Number of testing samples incorrectly classified = "
        << incorrect_classifications.size() << endl;
   cout << "incorrect_testing_frac = " << incorrect_testing_frac << endl;

// Delete dynamically allocated memory:

   for(int i = 0; i < n_training_samples; i++)
   {
      delete training_samples[i].first;
   }
   for(int i = 0; i < n_testing_samples; i++)
   {
      delete testing_samples[i].first;
   }

}



