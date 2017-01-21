// ==========================================================================
// Program TTT_SYMS
// ==========================================================================
// Last updated on 1/21/17
// ==========================================================================

#include <iostream>
#include <string>
#include <unistd.h>     // needed for getpid()
#include <vector>
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

   int nsize = 4;
   int n_zlevels = 4;
   tictac3d* ttt_ptr = new tictac3d(nsize, n_zlevels);
   int n_cells = ttt_ptr->get_n_total_cells();

   ttt_ptr->generate_permutation_matrices();
   ttt_ptr->compute_cell_orbits();

   ttt_ptr->reset_board_state();

   int input_cell;
   cout << "Enter input cell ID:" << endl;
   cin >> input_cell;
   ttt_ptr->set_player_move(input_cell,1);

   ttt_ptr->display_board_state();
   genvector* curr_s = ttt_ptr->update_board_state_ptr();
   //cout << "curr_s = " << *curr_s << endl;

/*
   int s;
   cout << "Enter permutation index s:" << endl;
   cin >> s;
   ttt_ptr->permute_board_state(s);
   ttt_ptr->display_board_state();
*/

   genvector* b = new genvector(n_cells);
   genvector* bperm = new genvector(n_cells);
   genvector* bsym = new genvector(n_cells);

   int K = 1;
   cout << "Enter K" << endl;
   cin >> K;

   genmatrix* W = new genmatrix(K * n_cells, n_cells);
   genmatrix* Wperm = new genmatrix(K * n_cells, n_cells);
   genmatrix* Wsym = new genmatrix(K * n_cells, n_cells);

   genmatrix* W2 = new genmatrix(n_cells, K * n_cells);
   genmatrix* Wperm2 = new genmatrix(n_cells, K * n_cells);
   genmatrix* Wsym2 = new genmatrix(n_cells, K * n_cells);
   
   for(unsigned int i = 0; i < b->get_mdim(); i++)
   {
      double curr_val = 2 * (nrfunc::ran1() - 0.5);
      b->put(i, curr_val);
   }
   
   for(unsigned int i = 0; i < W->get_mdim(); i++)
   {
      for(unsigned int j = 0; j < W->get_ndim(); j++)
      {
         double curr_val = 2 * (nrfunc::ran1() - 0.5);
         W->put(i,j,curr_val);
      }
   }

   for(unsigned int i = 0; i < W2->get_mdim(); i++)
   {
      for(unsigned int j = 0; j < W2->get_ndim(); j++)
      {
         double curr_val = 2 * (nrfunc::ran1() - 0.5);
         W2->put(i,j,curr_val);
      }
   }

   ttt_ptr->symmetrize_weight_matrix(W, Wperm, Wsym);
   *W = *Wsym;

   ttt_ptr->symmetrize_weight_matrix(W2, Wperm2, Wsym2);
   *W2 = *Wsym2;

   ttt_ptr->symmetrize_bias_vector(b, bperm, bsym);
   *b = *bsym;

   genvector *curr_a = new genvector(n_cells);
   *curr_a = *W2 * *W * *curr_s + *b;
   ttt_ptr->display_p_action(curr_a);

   delete b;
   delete bperm;
   delete bsym;

   delete W;
   delete Wperm;
   delete Wsym;
}



