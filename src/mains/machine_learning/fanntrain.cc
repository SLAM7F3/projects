// ==========================================================================
// Program FANNTRAIN reads in some training data set in a format that
// can be handled by the Fast Artificial Neural Network (FANN)
// library.  It exports the network's structure to a JSON file which
// can be displayed via Michael Yee's Graph Viewer.  FANNTRAIN also
// calls FANN in order to train the network's weight and bias terms.
// ==========================================================================
// Last updated on 2/20/13
// ==========================================================================

#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <vector>
#include <fann.h>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

//   string dataset_basename="xor";
   string dataset_basename="mushroom";
   string dataset_filename=dataset_basename+".data";
   string neural_network_filename=dataset_basename+"_float.net";

// Initialize neural net structure:
   
   typedef map<int,pair<int,int> > NEURON_MAP;
// indep int var = neuron ID
// dependent int pair var = (layer,vertical index)
   NEURON_MAP neuron_map;
   NEURON_MAP::iterator iter;

   vector<int> n_real_nodes_per_layer,n_total_nodes_per_layer;

// XOR example:

//   n_real_nodes_per_layer.push_back(2);
//   n_real_nodes_per_layer.push_back(2);
//   n_real_nodes_per_layer.push_back(1);
//   int num_layers = n_real_nodes_per_layer.size();

// Mushroom example

   n_real_nodes_per_layer.push_back(125);
   n_real_nodes_per_layer.push_back(32);
   n_real_nodes_per_layer.push_back(2);
   int num_layers = n_real_nodes_per_layer.size();

   int neuron_ID=0;
   int max_total_nodes_per_layer=-1;
   unsigned int layers_array[num_layers];
   for (int l=0; l<num_layers; l++)
   {
      layers_array[l]=n_real_nodes_per_layer[l];
      if (l < num_layers-1)
      {
         n_total_nodes_per_layer.push_back(n_real_nodes_per_layer[l]+1);
      }
      else
      {
         n_total_nodes_per_layer.push_back(n_real_nodes_per_layer[l]);
      }
      max_total_nodes_per_layer=basic_math::max(
         max_total_nodes_per_layer,n_total_nodes_per_layer.back());
      
      for (int n=0; n<n_total_nodes_per_layer.back(); n++)
      {
         pair<int,int> P;
         P.first=l;
         P.second=n+1;
         neuron_map[neuron_ID++]=P;
      }
   } // loop over index l labeling layers
   int n_neurons=neuron_map.size();

   double scale_factor=sqrt(max_total_nodes_per_layer);

// Export JSON file containing node structure for neural net:

   string json_filename="neural_net.json";
   ofstream jsonstream;
   filefunc::openfile(json_filename,jsonstream);
   
   jsonstream << "{ " << endl;
   jsonstream << "  \"graph\": { " << endl;
   jsonstream << "     \"id\": 0, " << endl;
   jsonstream << "     \"edgedefault\": \"undirected\", " << endl;
   
   jsonstream << "     \"node\": [ " << endl;

   for (int neuron_ID=0; neuron_ID<n_neurons; neuron_ID++)
   {
      iter=neuron_map.find(neuron_ID);
      int curr_layer=iter->second.first;
      int vertical_index=iter->second.second;

      bool bias_flag=false;
      if (curr_layer < num_layers-1 && 
      vertical_index==n_total_nodes_per_layer[curr_layer]) bias_flag=true;

      double d_gy=1.0/(n_total_nodes_per_layer[curr_layer]+1);

      double gx=double(curr_layer)/double(num_layers);
      double gy=1-vertical_index*d_gy;

      gx *= scale_factor;
      gy *= scale_factor;

      jsonstream << "     {" << endl;
      jsonstream << "       \"id\": " << neuron_ID << "," << endl;
      jsonstream << "       \"data\": {" << endl;
      jsonstream << "         \"type\": \"NODE\"," << endl;
      jsonstream << "         \"gx\": " << gx << "," << endl;
      jsonstream << "         \"gy\": " << gy << "," << endl;

      colorfunc::Color layer_color=colorfunc::get_color(curr_layer);
      colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(layer_color);
      if (bias_flag)
      {
         colorfunc::HSV curr_hsv=colorfunc::RGB_to_hsv(curr_RGB);
         curr_hsv.second *= 0.5;
         curr_hsv.third *= 0.66;
         curr_RGB=colorfunc::hsv_to_RGB(curr_hsv);
      }
      

      jsonstream << "         \"rgbColor\": [" 
                 << curr_RGB.first << "," 
                 << curr_RGB.second << "," 
                 << curr_RGB.third << "]" << endl;
      jsonstream << "        }" << endl;
      jsonstream << "     }";
      if (neuron_ID < n_neurons-1) jsonstream << ",";
      jsonstream << endl;

   } // loop over neuron_ID
   jsonstream << "    ]," << endl;


// Next instantiate FANN structures to hold and solve neural network:

// XOR example
//   const float desired_error = (const float) 0.001;
//   const unsigned int max_epochs = 500000;
//   const unsigned int epochs_between_reports = 1000;

// Mushroom example

//   const unsigned int num_layers = 3;
//   const unsigned int num_neurons_hidden = 32;

   const float desired_error = (const float) 0.0001;
   const unsigned int max_epochs = 300;
   const unsigned int epochs_between_reports = 10;

   struct fann *ann;
   struct fann_train_data *train_data, *test_data;

// XOR example:
   ann = fann_create_standard_array(num_layers, layers_array);

// Mushroom example:

   train_data = fann_read_train_from_file(dataset_filename.c_str());

//   ann = fann_create_standard(
//      num_layers,
//      train_data->num_input, num_neurons_hidden, train_data->num_output);


// XOR example
//   fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
//   fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

   fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC_STEPWISE);
   fann_set_activation_function_output(ann, FANN_SIGMOID_STEPWISE);

// XOR example
//   fann_train_on_file(
//      ann, dataset_filename.c_str(), 
//      max_epochs, epochs_between_reports, desired_error);

// Mushroom example:
   fann_train_on_data(ann, train_data, max_epochs, epochs_between_reports, 
   desired_error);
   
   cout << "Saving network" << endl;

   fann_save(ann, neural_network_filename.c_str());
//   fann_save(ann, "xor_float.net");
//   fann_save_to_fixed(ann, "xor_fixed.net");


   int n_layers=fann_get_num_layers(ann);
   cout << "n_layers = " << n_layers << endl;
   int n_nodes=fann_get_total_neurons(ann);
   cout << "n_nodes = " << n_nodes << endl;
   int n_links=fann_get_total_connections(ann);
   cout << "n_links = " << n_links << endl;

//   fann_print_connections(ann);

   fann_connection* connections=new fann_connection[n_links];
   fann_get_connection_array(ann,connections);


// Add links to output JSON file

   jsonstream << "     \"edge\": [ " << endl;

   for (int l=0; l<n_links; l++)
   {
      fann_connection curr_connection=connections[l];
      int source_ID=curr_connection.from_neuron;
      int target_ID=curr_connection.to_neuron;
      double weight=curr_connection.weight;
      
//      cout << "l = " << l 
//           << " source = " << source_ID
//           << " target = " << target_ID
//           << " weight = " << weight
//           << endl;

      jsonstream << "        {" << endl;
      jsonstream << "           \"source\": " << source_ID << "," << endl;
      jsonstream << "           \"target\": " << target_ID << "," << endl;
      jsonstream << "           \"data\": {" << endl;

// FAKE FAKE:  Weds Feb 20, 2013 at 5 pm
// Reset weight to unity for GraphViewer display purposes

      weight=1;
      jsonstream << "             \"edge_weight\": " << weight << "," << endl;

      colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(colorfunc::white);
            
      jsonstream << "             \"rgbColor\":  [" 
                 << curr_RGB.first << "," 
                 << curr_RGB.second << "," 
                 << curr_RGB.third << "]" 
                 << endl;
      jsonstream << "           }" << endl;
      jsonstream << "        }";
      if (l < n_links-1) jsonstream << ",";
      jsonstream << endl;
   }

   jsonstream << "    ]" << endl;


   jsonstream << "  }" << endl;
   jsonstream << "}" << endl;

   filefunc::closefile(json_filename,jsonstream);


   fann_print_parameters(ann);
   
   fann_destroy(ann);


}
