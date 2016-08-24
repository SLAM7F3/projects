// ========================================================================
// Program ACTIVATIONS

//                              ./activations

// ========================================================================
// Last updated on 8/24/16
// ========================================================================

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv) 
{
   string imagechips_subdir = "./vis_facenet/node_images/";
   string activations_subdir = imagechips_subdir + "activations/";
   string activations_filename = activations_subdir+"activations.dat";

   string ordered_activations_filename=activations_subdir+
      "ordered_activations.dat";
   ofstream outstream;
   filefunc::openfile(ordered_activations_filename, outstream);

      vector< vector<double> > row_numbers = 
      filefunc::ReadInRowNumbers(activations_filename);
   int prev_layer = -1;
   vector<int> node_ID;
   vector<double> stimulation_frac, mu_activation, sigma_activation;

   outstream << "# ====================================================="
             << endl;
   outstream << "# Nodes ordered by their test image stimulation frequency"
             << endl;
   outstream << "# ====================================================="
             << endl;
   outstream << "# Layer  Local     Global   Stimul  Mu    Sigma" << endl;
   outstream << "# ID     node ID   node ID  freq    act   act" << endl;
   outstream << "# ====================================================="
             << endl << endl;

   int global_node_ID = 0;
   for(unsigned int r = 0; r < row_numbers.size(); r++)
   {
//      for(unsigned int c = 0 ; c < row_numbers.at(r).size(); c++)
//      {
//         cout << row_numbers.at(r).at(c) << "  ";
//      }

      node_ID.push_back(row_numbers.at(r).at(1));
      stimulation_frac.push_back(row_numbers.at(r).at(2));
      mu_activation.push_back(row_numbers.at(r).at(5));
      sigma_activation.push_back(row_numbers.at(r).at(6));

      int curr_layer = row_numbers.at(r).at(0);
      if((r > 0 && curr_layer != prev_layer) || r == row_numbers.size() - 1)
      {
         outstream << "# ====================================================="
                   << endl << endl;
         templatefunc::Quicksort_descending(
            stimulation_frac, node_ID, mu_activation, sigma_activation);
//             mu_activation, node_ID, stimulation_frac, sigma_activation);

         for(unsigned int i = 0; i < node_ID.size(); i++)
         {
            outstream << prev_layer+1 << "    "
                      << node_ID[i] << "    "
                      << global_node_ID++ << "    "
                      << stimulation_frac[i] << "    "
                      << mu_activation[i] << "    "
                      << sigma_activation[i] << endl;
         }
         outstream << endl;

         stimulation_frac.clear();
         node_ID.clear();
         mu_activation.clear();
         sigma_activation.clear();
      }
      prev_layer = curr_layer;

   } // loop over index r labeling rows in activations_filename

   filefunc::closefile(ordered_activations_filename, outstream);
   string banner="Exported ordered activations to "+
      ordered_activations_filename;
   outputfunc::write_banner(banner);
}

   
