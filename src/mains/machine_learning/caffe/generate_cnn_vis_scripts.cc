// ==========================================================================
// Program GENERATE_CNN_VIS_SCRIPTS

//			   generate_cnn_vis_scripts

// ==========================================================================
// Last updated on 8/15/16
// ==========================================================================

#include  <algorithm>
#include  <fstream>
#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;


// ==========================================================================

int main(int argc, char* argv[])
{
   string scripts_subdir="./cnn_vis_scripts/";
   filefunc::dircreate(scripts_subdir);
   string node_images_subdir=scripts_subdir+"node_images/";
   filefunc::dircreate(node_images_subdir);

   string all_scripts_filename=scripts_subdir+"run_all_scripts";
   ofstream all_scripts_stream;
   filefunc::openfile(all_scripts_filename, all_scripts_stream);

   vector<int> n_layer_nodes;
   n_layer_nodes.push_back(256);
   n_layer_nodes.push_back(256);
   n_layer_nodes.push_back(3);

   vector<string> layer_names;
   layer_names.push_back("fc5");
   layer_names.push_back("fc6");
   layer_names.push_back("fc7_faces");
   
   string cnn_vis_pathname = "/usr/local/python/cnn_vis.py";


   int layer_start = 5;
   int layer_stop = 7;
   for(int layer = layer_stop; layer >= layer_start; layer--)
   {
      int layer_index = layer - layer_start;
      for(int node = 0; node < n_layer_nodes[layer_index]; node++)
      {
         string curr_layer_name=layer_names[layer_index];
         string curr_node_name=stringfunc::integer_to_string(node,3);
         string curr_script_basename="run_"+
            curr_layer_name+"_"+curr_node_name;
         string curr_script_filename=scripts_subdir+curr_script_basename;
         string curr_node_img_filename="./node_images/"+
            curr_layer_name+"_"+curr_node_name+".png";
         ofstream outstream;

         filefunc::openfile(curr_script_filename, outstream);

         outstream << "/usr/local/anaconda/bin/python \\" << endl;
         outstream << cnn_vis_pathname << " \\" << endl;
         outstream << "--deploy_txt=/data/caffe/faces/trained_models/test_160.prototxt \\" << endl;
         outstream << "--caffe_model=/data/caffe/faces/trained_models/Aug6_350K_96cap_T3/train_iter_702426.caffemodel \\" << endl;
         outstream << "--image_type=amplify_neuron \\" << endl;
         outstream << "--target_layer="+curr_layer_name+" \\" << endl;
         outstream << "--target_neuron="+stringfunc::number_to_string(node)
            +" \\" << endl;
         outstream << "--output_file="+curr_node_img_filename+" \\" << endl;
         outstream << "--rescale_image \\" << endl;
         outstream << "--gpu=0   \\" << endl;
         outstream << "--num_steps=1000 \\" << endl;
         outstream << "--batch_size=25 \\" << endl;
         outstream << "--output_iter=1000 \\" << endl;
         outstream << "--learning_rate=0.1 \\" << endl;
         outstream << "--learning_rate_decay_iter=100 \\" << endl;
         outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
         outstream << "--decay_rate=0.9 \\" << endl;
         outstream << "--alpha=2.0 \\" << endl;
         outstream << "--p_reg=0.0001 \\" << endl;
         outstream << "--beta=2.5 \\" << endl;
         outstream << "--tv_reg=0.00001 \\" << endl;
         outstream << "--num_sizes=1 \\" << endl;
         outstream << "--iter_behavior=print" << endl;
         filefunc::closefile(curr_script_filename, outstream);
         filefunc::make_executable(curr_script_filename);

         all_scripts_stream << "./" << curr_script_basename << endl;
      } // loop over node index
   } // loop over layer index

   filefunc::closefile(all_scripts_filename, all_scripts_stream);
   filefunc::make_executable(all_scripts_filename);

   string banner="Exported cnn_vis scripts to "+scripts_subdir;
   outputfunc::write_banner(banner);
   banner="Exported script to run all layers/nodes to "+
      all_scripts_filename;
   outputfunc::write_banner(banner);
}

