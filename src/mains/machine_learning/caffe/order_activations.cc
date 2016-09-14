// ========================================================================
// Program ORDER_ACTIVATIONS imports raw activation statistics for
// each FACENET neuron generated by program RAW_ACTIVATIONS.  For each
// network layer, activations are reordered according to their
// stimulation frequencies.  The new ordering for nodes is exported to
// an output text file.

//                         ./order_activations

// ========================================================================
// Last updated on 9/7/16; 9/8/16; 9/11/16; 9/14/16
// ========================================================================

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
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
   string facenet_model_label;
   cout << "Enter facenet model label: (e.g. 2e, 2n, 2r)" << endl;
   cin >> facenet_model_label;

   typedef std::map<DUPLE, int, ltduple> NODE_IDS_MAP;
// independent duple contains (layer_ID, old_local_node_ID)
// dependent int contains new_local_node_ID
   NODE_IDS_MAP node_ids_map;
   NODE_IDS_MAP::iterator node_ids_iter;

   string network_subdir = "./vis_facenet/network/";
   string base_activations_subdir = network_subdir + "activations/";
   string activations_subdir = base_activations_subdir + "model_"+
      facenet_model_label+"/";
   string subnetwork_subdir = network_subdir + "subnetworks/";
   filefunc::dircreate(subnetwork_subdir);
   string male_subnetwork_subdir=subnetwork_subdir+"male/";
   string female_subnetwork_subdir=subnetwork_subdir+"female/";
   filefunc::dircreate(male_subnetwork_subdir);
   filefunc::dircreate(female_subnetwork_subdir);

   string activations_filename = activations_subdir+"activations.dat";
   string ordered_activations_filename=activations_subdir+
      "ordered_activations.dat";
   ofstream outstream;
   filefunc::openfile(ordered_activations_filename, outstream);

   vector< vector<double> > row_numbers = 
      filefunc::ReadInRowNumbers(activations_filename);

   vector<int> old_global_node_ID, node_ID, new_node_ID;
   vector<double> stimulation_frac, median_activation, quartile_activation;

// First count total number of nodes and number of nodes per network
// layer:

   vector<int> n_layer_nodes;
   n_layer_nodes.push_back(3);  // Zeroth input data layer has 3 RGB channels
   int n_total_nodes = n_layer_nodes.back();

   cout << "Layer 0 has " << n_layer_nodes.back() << " nodes " << endl;

   int prev_layer = 0;
   int n_nodes_per_layer = 0;
   for(unsigned int r = 0; r < row_numbers.size(); r++)
   {
      int curr_layer = row_numbers.at(r).at(0);
      if (r > 0)
      {
         if(curr_layer != prev_layer)
         {
            n_layer_nodes.push_back(n_nodes_per_layer);
            cout << "Layer " << prev_layer
                 << " has " << n_layer_nodes.back() << " nodes " << endl;
            n_nodes_per_layer = 0;
         }
      }
      n_nodes_per_layer++;
      n_total_nodes++;
      prev_layer = curr_layer;
   }

   int max_layer_ID = prev_layer;
   n_layer_nodes.push_back(n_nodes_per_layer);
   cout << "Layer " << prev_layer
        << " has " << n_layer_nodes.back() << " nodes " << endl;
   cout << "Total number of network nodes = " << n_total_nodes << endl;
   
   outstream << "# ====================================================="
             << endl;
   outstream << "# Nodes ordered by their test image stimulation frequency"
             << endl;
   outstream << "# ====================================================="
             << endl;
   outstream << "# Layer  Old local   Old global New local    New global  Stimul  Nonzero     Nonzero median" << endl;
   outstream << "# ID     node ID     node ID    node ID      node ID     freq    median act  act frac      " << endl;
   outstream << "# ====================================================="
             << endl << endl;

// First print out 3 input RGB channels:

   int n_RGB_channels = 3;
   prev_layer = -1;

   for(int i = 0; i < n_RGB_channels; i++)
   {
      double stimul_frac = 1.0;
      double median_activation = 1.0;
      double quartile_activation = 0.5;
      double median_act_frac = 1.0;
      int old_local_node_ID = i;
      int new_local_node_ID = i;

// Save relationship between old unordered and new ordered global node
// IDs within STL map:

      DUPLE duple(0,old_local_node_ID);
      node_ids_map[ duple ] = new_local_node_ID;

      outstream << prev_layer+1 << "    "
                << i << "    "
                << i << "    "
                << i << "    "
                << i << "    "
                << stimul_frac << "    "
                << median_activation << "    "
                << median_act_frac << endl;
   }
   outstream << endl;

   prev_layer = 0;
   int local_node_ID = 0;
   int RGB_data_offset = 3;
   int new_global_node_ID = 0 + RGB_data_offset;

   double min_median_activation = 1E10;
   double max_median_activation = -min_median_activation;
   
   for(unsigned int r = 0; r < row_numbers.size(); r++)
   {
      if(r == row_numbers.size() -1)
      {
         old_global_node_ID.push_back(row_numbers.at(r).at(1));
         node_ID.push_back(row_numbers.at(r).at(2));
         new_node_ID.push_back(local_node_ID++);
         stimulation_frac.push_back(row_numbers.at(r).at(3));
         median_activation.push_back(row_numbers.at(r).at(4));
         quartile_activation.push_back(row_numbers.at(r).at(5));

         double curr_median_activation = median_activation.back();
         if(curr_median_activation > 0)
         {
           min_median_activation = basic_math::min(min_median_activation,
                                                   curr_median_activation);
           max_median_activation = basic_math::max(max_median_activation,
                                                   curr_median_activation);
         }
      }

      int curr_layer = row_numbers.at(r).at(0);
      if((r > 0 && curr_layer != prev_layer) || r == row_numbers.size() - 1)
      {
         outstream << "# ====================================================="
                   << endl << endl;
         outstream << "# Minimum non-zero activation = "
                   << min_median_activation << endl;
         outstream << "# Maximum non-zero activation = "
                   << max_median_activation << endl << endl;

         templatefunc::Quicksort_descending(
            stimulation_frac, old_global_node_ID, node_ID, 
            median_activation, quartile_activation);

         for(unsigned int i = 0; i < node_ID.size(); i++)
         {

// Save relationship between old unordered and new ordered global node
// IDs within STL map:

            DUPLE duple(prev_layer,node_ID[i]);
            node_ids_map[ duple ] = new_node_ID[i];

            double median_activation_frac = 
               (median_activation[i] - min_median_activation)/ 
               (max_median_activation - min_median_activation);

            outstream << prev_layer << "    "
                      << node_ID[i] << "    "
                      << old_global_node_ID[i] << "    "
                      << new_node_ID[i] << "    "
                      << new_global_node_ID++ << "    "
                      << stimulation_frac[i] << "    "
                      << median_activation[i] << "    "
                      << median_activation_frac 
                      << endl;
         }
         outstream << endl;

         stimulation_frac.clear();
         old_global_node_ID.clear();
         node_ID.clear();
         new_node_ID.clear();
         local_node_ID = 0;
         median_activation.clear();
         quartile_activation.clear();

         min_median_activation = 1E10;
         max_median_activation = -min_median_activation;
      }
      prev_layer = curr_layer;

      old_global_node_ID.push_back(row_numbers.at(r).at(1));
      node_ID.push_back(row_numbers.at(r).at(2));
      new_node_ID.push_back(local_node_ID++);
      stimulation_frac.push_back(row_numbers.at(r).at(3));
      median_activation.push_back(row_numbers.at(r).at(4));
      quartile_activation.push_back(row_numbers.at(r).at(5));

      double curr_median_activation = median_activation.back();
      if(curr_median_activation > 0)
      {
         min_median_activation = basic_math::min(min_median_activation,
                                                 curr_median_activation);
         max_median_activation = basic_math::max(max_median_activation,
                                                 curr_median_activation);
      }
   } // loop over index r labeling rows in activations_filename

   filefunc::closefile(ordered_activations_filename, outstream);

   string banner="Exported ordered activations to "+
      ordered_activations_filename;
   outputfunc::write_banner(banner);


   exit(-1);

//   for(node_ids_iter = node_ids_map.begin(); node_ids_iter != 
//          node_ids_map.end(); node_ids_iter++)
//   {
//      int old_global_node_ID = node_ids_iter->first;
//      int new_global_node_ID = node_ids_iter->second;
//      cout << "old_global_node_ID = " << old_global_node_ID
//           << " new_global_node_ID = " << new_global_node_ID
//           << endl;
//   }
//   cout << "node_ids_map.size = " << node_ids_map.size() << endl;

// Import text file exported by program RAW_ACTIVATIONS which
// contains neural net pathways for particular test images:
   
   string montage_filename="./montage_cmds.dat";
   filefunc::ReadInfile(montage_filename);
   prev_layer = -1;
   int n_max = 5;
   string unix_cmd="";

   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(filefunc::text_line[i]);
      int layer = stringfunc::string_to_number(substrings[0]);

      if(layer == 0)
      {
         string imagechip_filename = substrings[1];
         string montage_filename = "layer_000.png";
         unix_cmd = "montage -tile 1x1 ";
         unix_cmd += imagechip_filename+" "+montage_filename;
         sysfunc::unix_command(unix_cmd);
      }
      else if (layer == max_layer_ID)
      {

// Export montage for layer max_layer_ID - 1:

         string montage_filename = "layer_"+
            stringfunc::integer_to_string(prev_layer,3)+".png";
         unix_cmd += " "+montage_filename;
         sysfunc::unix_command(unix_cmd);

// Now export montage for max_layer_ID layer:

         string class_label = substrings[1];
         double classification_score = stringfunc::string_to_number(
            substrings[2]);
         string imagechip_filename = substrings[3];
         montage_filename = "layer_"+stringfunc::integer_to_string(
            layer,3)+".png";

         unix_cmd = "montage -tile 1x1 ";
         unix_cmd += "-label '"+class_label+"  s="+
            stringfunc::number_to_string(classification_score,3)+"' ";
         unix_cmd += imagechip_filename+" "+montage_filename;
         sysfunc::unix_command(unix_cmd);

// Generate network montage from individual layer montages:

         unix_cmd = "montage -geometry +2+2 -tile 1x"
            +stringfunc::number_to_string(max_layer_ID+1);

         for(int layer = 0; layer <= max_layer_ID; layer++)
         {
            unix_cmd += " layer_" + stringfunc::integer_to_string(layer,3)+
               ".png";
         }
         
         string network_montage_filename=male_subnetwork_subdir;
         if(class_label == "female")
         {
            network_montage_filename=female_subnetwork_subdir;
         }
         
         network_montage_filename += "network_"+
            stringfunc::integer_to_string(i,4)+".png";
         unix_cmd += " "+network_montage_filename;
         sysfunc::unix_command(unix_cmd);

         string banner="Exported "+network_montage_filename;
         outputfunc::write_banner(banner);
      }
      else
      {
         if(layer != prev_layer)
         {
            if(layer >= 2)
            {
               string montage_filename = "layer_"+
                  stringfunc::integer_to_string(prev_layer,3)+".png";
               unix_cmd += " "+montage_filename;
               sysfunc::unix_command(unix_cmd);
            }
            unix_cmd = "montage -tile "+stringfunc::number_to_string(n_max)
               +"x1 ";
            prev_layer = layer;
         }
         
         int old_local_node_ID = stringfunc::string_to_number(substrings[1]);
         DUPLE duple(layer, old_local_node_ID);
         node_ids_iter = node_ids_map.find(duple);
         int new_local_node_ID = node_ids_iter->second;
         double renorm_node_activation = stringfunc::string_to_number(
            substrings[2]);
         string imagechip_filename = substrings[3];

         string chiplabel_str = " -label ";
         chiplabel_str += "'node "+stringfunc::number_to_string(
            new_local_node_ID)+"  a="+stringfunc::number_to_string(
               renorm_node_activation,3)+"' ";
         unix_cmd += chiplabel_str + imagechip_filename;
      }
   } // loop over index i labeling text lines

}

   
