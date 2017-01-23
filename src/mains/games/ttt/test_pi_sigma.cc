// ==========================================================================
// Program TEST_PI_SIGMA performs inference using policy networks trained via
// program TRAIN_PI_SIGMA.  User specifies minimum and maximum number
// of moves till end-of-game.  TEST_PI_SIGMA returns number of test
// samples lying within requested move range and trained model's
// accuracy fraction for predicting next supervised move.
// ==========================================================================
// Last updated on 1/22/17; 
// ==========================================================================

#include <iostream>
#include <string>
#include <unistd.h>     // needed for getpid()
#include <vector>
#include "machine_learning/environment.h"
#include "general/filefuncs.h"
#include "machine_learning/machinelearningfuncs.h"
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
   using std::ofstream;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();
   long seed = nrfunc::init_time_based_seed();
//   long seed = -11;
//   cout << "Enter negative seed:" << endl;
//   cin >> seed;
//   nrfunc::init_default_seed(seed);

   int nsize = 4;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_cells = ttt_ptr->get_n_total_cells();

   int Din = n_cells;
   int H1 = 256;			// Number of first hidden layer nodes
   int H2 = 256;			// Number of 2nd hidden layer nodes
   int H3 = 0;				// Number of 3rd hidden layer nodes
//   cout << "Enter H1:" << endl;
//   cin >> H1;
//   cout << "Enter H2:" << endl;
//   cin >> H2;
//   cout << "Enter H3:" << endl;
//   cin >> H3;
   int Dout = n_cells;   	// Number of output layer nodes

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

// Set up neural network:

   int mini_batch_size = 100;
//   double lambda = 0;  // L2 regularization coefficient
   double lambda = 1E-4;  // L2 regularization coefficient
//   cout << "Enter L2 regularization coefficient lambda:" << endl;
//   cin >> lambda;
   double rmsprop_decay_rate = 0.95;

   neural_net NN(mini_batch_size, lambda, rmsprop_decay_rate, layer_dims);

   int min_move_rel_to_game_end = 1;
   cout << "Enter minimum move relative to game end:" << endl;
   cin >> min_move_rel_to_game_end;

   int max_move_rel_to_game_end = 1;
   cout << "Enter maximum move relative to game end:" << endl;
   cin >> max_move_rel_to_game_end;

// Initialize output subdirectory within an experiments folder:

   string experiments_subdir="./experiments/";
   filefunc::dircreate(experiments_subdir);
   string pi_sigma_subdir = experiments_subdir + "pi_sigma/";
   filefunc::dircreate(pi_sigma_subdir);

   int expt_number;
   cout << "Enter experiment number:" << endl;
   cin >> expt_number;
   NN.set_expt_number(expt_number);
   string output_subdir=pi_sigma_subdir+
      "expt"+stringfunc::integer_to_string(expt_number,3)+"/";
   NN.set_output_subdir(output_subdir);
   
// Import test set afterstate-action pairs generated by program
// minimax2:

   vector<neural_net::DATA_PAIR> testing_samples;
   int n_testing_samples = 0;

   string input_subdir = "./afterstate_action_pairs/";
   string input_filename = input_subdir + "test_afterstate_action_pairs.txt";
   filefunc::ReadInfile(input_filename);

   vector<vector<string> > line_substrings = 
      filefunc::ReadInSubstrings(input_filename);

   neural_net::DATA_PAIR curr_data_pair;

   for(unsigned int i = 0; i < line_substrings.size(); i++)
   {
      string board_state_str = line_substrings[i].at(0);
      int move_rel_to_game_end = stringfunc::string_to_integer(
         line_substrings[i].at(1));

      if(move_rel_to_game_end < min_move_rel_to_game_end) continue;
      if(move_rel_to_game_end >= max_move_rel_to_game_end) continue;

      int aplus1 = stringfunc::string_to_integer(line_substrings[i].at(2));
      double player_value = 1;
      if(aplus1 < 0) player_value = -1;
      int a = abs(aplus1) - 1;
      curr_data_pair.second = a;

// Store player_value = +1 or -1 within last entry in
// *player_board_state_ptr:

// FAKE FAKE:  Weds Jan 18 at 8:06 am

// Experiment with forming 64-dim player_board states in which action ALWAYS
// corresponds to agent_value = +1:

      genvector *player_board_state_ptr = new genvector(n_cells);
//      genvector *player_board_state_ptr = new genvector(n_cells + 1);
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

      *player_board_state_ptr *= player_value;
      player_board_state_ptr->put(n_cells, player_value);
      testing_samples.push_back(curr_data_pair);
   } // loop over index i labeling input text lines
   n_testing_samples = testing_samples.size();
   cout << "n_testing_samples = " << n_testing_samples << endl;
   NN.import_test_data(testing_samples);

   NN.set_extrainfo(stringfunc::number_to_string(max_move_rel_to_game_end)
                    + " end moves");

// Import neural network snapshot:

   string snapshots_subdir = output_subdir + "snapshots/";
   string snapshot_filename = snapshots_subdir + "snapshot.txt";
   NN.import_snapshot(snapshot_filename);

   double test_accuracy = NN.evaluate_model_on_test_set();

   vector<int> incorrect_classifications = NN.get_incorrect_classifications();
   cout << "Number of test samples incorrectly classified = "
        << incorrect_classifications.size() << endl;
   cout << "Test accuracy = " << test_accuracy << endl;

// Delete dynamically allocated memory:

   for(int i = 0; i < n_testing_samples; i++)
   {
      delete testing_samples[i].first;
   }
}