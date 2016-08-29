// ==========================================================================
// Program GENERATE_CNN_VIS_SCRIPTS loops over all layers and nodes
// corresponding to some trained Caffe model.  For each node in the
// network, this program creates an executable script which calls
// Justin Johnson's "cnn_vis" python script.  We have empirically
// determined reasonable values for many of the parameters within
// Johnson's script which generate 96x96 image chips that maximally
// activate for each neuron in the network.  The image chips are
// initialized with gaussian noise.  Backprojection is used while
// holding the weights in the trained model fixed to reconstruct the
// chip which maximally activates a particular neuron.  All executable
// scripts which run the python code are exported to an output folder
// along wiht a single additional script that runs all of the
// generated scripts.

//   ./generate_cnn_vis_scripts                      
//    /data/caffe/faces/trained_models/test_160.prototxt                    
//    /data/caffe/faces/trained_models/Aug6_350K_96cap_T3/train_iter_702426.caffemodel 

// ==========================================================================
// Last updated on 8/20/16; 8/22/16; 8/24/16; 8/28/16
// ==========================================================================

#include  <algorithm>
#include  <fstream>
#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "math/basic_math.h"
#include "classification/caffe_classifier.h"
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
   timefunc::initialize_timeofday_clock();

   string test_prototxt_filename = argv[1];
   string caffe_model_filename = argv[2];

   bool Alexnet_flag = false;
   bool Googlenet_flag = false;
   bool VGG_flag = false;
   bool Resnet50_flag = false;
   bool Facenet_flag = false;
   string caffe_model_basename=filefunc::getbasename(caffe_model_filename);
   int max_nodes_per_param_layer = 25;

   if(caffe_model_basename == "bvlc_reference_caffenet.caffemodel")
   {
      Alexnet_flag = true;
//       max_nodes_per_param_layer = 25;
      max_nodes_per_param_layer = 1000;
   }
   else if(caffe_model_basename == "bvlc_googlenet.caffemodel")
   {
      Googlenet_flag = true;
      max_nodes_per_param_layer = 1000;
   }
   else if(caffe_model_basename == "VGG_ILSVRC_16_layers.caffemodel")
   {
      VGG_flag = true;
      max_nodes_per_param_layer = 1000;
//      max_nodes_per_param_layer = 25;
   }
   else if(caffe_model_basename == "ResNet-50-model.caffemodel")
   {
      Resnet50_flag = true;
      max_nodes_per_param_layer = 25;
//      max_nodes_per_param_layer = 1000;
   }
   else 
   {
      Facenet_flag = true;
      max_nodes_per_param_layer = 20;
//      max_nodes_per_param_layer = 25;
//      max_nodes_per_param_layer = 512;
   }

   cout << "caffe_model_basename = " << caffe_model_basename << endl;
   cout << "Alexnet_flag = " << Alexnet_flag << endl;
   cout << "Googlenet_flag = " << Googlenet_flag << endl;
   cout << "VGG_flag = " << VGG_flag << endl;
   cout << "Resnet50_flag = " << Resnet50_flag << endl;
   cout << "Facenet_flag = " << Facenet_flag << endl;

   caffe_classifier classifier(test_prototxt_filename, caffe_model_filename);

   string scripts_subdir;
   if(Alexnet_flag)
   {
      scripts_subdir="./vis_alexnet/";
   }
   else if(Googlenet_flag)
   {
      scripts_subdir="./vis_Googlenet/";
   }
   else if(VGG_flag)
   {
      scripts_subdir="./vis_VGG/";
   }
   else if(Resnet50_flag)
   {
      scripts_subdir="./vis_Resnet50/";
   }
   else if(Facenet_flag)
   {
//      scripts_subdir="./vis_facenet/";
      scripts_subdir="./vis_facenet2/";
      cout << "Enter subdir of vis_facenet2 into which image chips will be exported" << endl;
      string scripts_subsubdir;
      cin >> scripts_subsubdir;
      filefunc::add_trailing_dir_slash(scripts_subsubdir);
      scripts_subdir += scripts_subsubdir;
   }

   filefunc::dircreate(scripts_subdir);
   string node_images_subdir=scripts_subdir+"node_images/";
   filefunc::dircreate(node_images_subdir);

   string all_scripts_filename=scripts_subdir+"run_all_scripts";
   ofstream all_scripts_stream;
   filefunc::openfile(all_scripts_filename, all_scripts_stream);

   vector<int> n_layer_nodes = classifier.get_n_param_layer_nodes();
   for(unsigned int n = 0; n < n_layer_nodes.size(); n++)
   {
      cout << "n = " << n << " n_layer_nodes = "
           << n_layer_nodes[n] << endl;
   }

   double init_scale = 50;
   vector<string> param_layer_names;
   if (Alexnet_flag)
   {
      param_layer_names.push_back("conv1");
      param_layer_names.push_back("conv2");
      param_layer_names.push_back("conv3");
      param_layer_names.push_back("conv4");
      param_layer_names.push_back("conv5");
      param_layer_names.push_back("fc6");
      param_layer_names.push_back("fc7");
      param_layer_names.push_back("fc8");
   }
   else if (Googlenet_flag)
   {
      param_layer_names.push_back("loss3/classifier");
   }
   else if(VGG_flag)
   {
      param_layer_names.push_back("conv1_1");
      param_layer_names.push_back("conv1_2");
      param_layer_names.push_back("conv2_1");
      param_layer_names.push_back("conv2_2");
      param_layer_names.push_back("conv3_1");
      param_layer_names.push_back("conv3_2");
      param_layer_names.push_back("conv3_3");
      param_layer_names.push_back("conv4_1");
      param_layer_names.push_back("conv4_2");
      param_layer_names.push_back("conv4_3");
      param_layer_names.push_back("conv5_1");
      param_layer_names.push_back("conv5_2");
      param_layer_names.push_back("conv5_3");
      param_layer_names.push_back("fc6");
      param_layer_names.push_back("fc7");
      param_layer_names.push_back("fc8");
   }
   else if (Resnet50_flag)
   {
      param_layer_names.push_back("input");
      param_layer_names.push_back("conv1");
      param_layer_names.push_back("res2a");
      param_layer_names.push_back("res2b");
      param_layer_names.push_back("res2c");
      param_layer_names.push_back("res3a");
      param_layer_names.push_back("res3b");
      param_layer_names.push_back("res3c");
      param_layer_names.push_back("res3d");
      param_layer_names.push_back("res4a");
      param_layer_names.push_back("res4b");
      param_layer_names.push_back("res4c");
      param_layer_names.push_back("res4d");
      param_layer_names.push_back("res4e");
      param_layer_names.push_back("res4f");
      param_layer_names.push_back("res5a");
      param_layer_names.push_back("res5b");
      param_layer_names.push_back("res5c");
      param_layer_names.push_back("fc1000");
   }
   else if (Facenet_flag)
   {
      param_layer_names.push_back("conv1");
      param_layer_names.push_back("conv2");
      param_layer_names.push_back("conv3");
      param_layer_names.push_back("conv4");
      param_layer_names.push_back("fc5");
      param_layer_names.push_back("fc6");
      param_layer_names.push_back("fc7_faces");

/*
// Facenet1:
      param_layer_names.push_back("conv1a");
      param_layer_names.push_back("conv2a");
      param_layer_names.push_back("conv3a");
      param_layer_names.push_back("conv4a");
      param_layer_names.push_back("fc5");
      param_layer_names.push_back("fc6");
      param_layer_names.push_back("fc7_faces");
*/

      init_scale = 15; // empirically reduced for 96x96 face images
   }

   for(unsigned int p = 0; p < param_layer_names.size(); p++)
   {
      cout << "Param layer p = " << p << " is named " << param_layer_names[p]
           << endl;
   }
   outputfunc::enter_continue_char();

   string cnn_vis_pathname = "/usr/local/python/cnn_vis.py";

   int layer_start = 1;   // We name very first layer as 1 rather than 0
//   int layer_stop =  1;
   int final_layer = param_layer_names.size();

   int layer_stop = final_layer;
   cout << "layer_start = " << layer_start << endl;
   cout << "layer_stop = " << layer_stop << endl;

   outputfunc::enter_continue_char();

// For progress reporting purposes, first count total number of nodes
// whose images will be generated via backprojection:

   int n_total_nodes_to_process = 0;
   for(int layer = layer_stop; layer >= layer_start; layer--)
   {
      int layer_index = layer - layer_start;
      int max_iters = 1;
      if(layer == final_layer && Facenet_flag)
      {
         max_iters = 5;
//         max_iters = 30;
      }
      n_total_nodes_to_process += max_iters * n_layer_nodes[layer_index];
   }
   
   int node_counter = 0;
   for(int layer = layer_stop; layer >= layer_start; layer--)
   {
      int layer_index = layer - layer_start;

      int max_iters = 1;
      if(layer == final_layer && Facenet_flag)
      {
         max_iters = 5;
//         max_iters = 30;
      }
      
      for(int iter = 0; iter < max_iters; iter++)
      {
         for(int node = 0; node < n_layer_nodes[layer_index]; node++)
         {
            outputfunc::update_progress_and_remaining_time(
               node_counter++, 10, n_total_nodes_to_process);

            if(node > max_nodes_per_param_layer) continue;

            string orig_layer_name=param_layer_names[layer_index];
            string curr_layer_name = stringfunc::find_and_replace_char(
               orig_layer_name, "/", "_");
            
            string curr_node_name=stringfunc::integer_to_string(node,3);
            string curr_script_basename="run_"+
               curr_layer_name+"_"+curr_node_name;
            if(iter > 0)
            {
               curr_script_basename += "_"+
                  stringfunc::integer_to_string(iter,2);
            }
            
            string curr_script_filename=scripts_subdir+curr_script_basename;
            string curr_img_subdir = node_images_subdir + 
               curr_layer_name+"/";
            filefunc::dircreate(curr_img_subdir);
            curr_img_subdir = "./node_images/"+curr_layer_name+"/";
            string curr_node_img_basename=
               curr_layer_name+"_"+curr_node_name;
            if(iter > 0)
            {
               curr_node_img_basename += "_"+
                  stringfunc::integer_to_string(iter,2);
            }
            curr_node_img_basename += ".png";

            string curr_node_img_filename=curr_img_subdir + 
               curr_node_img_basename;
            ofstream outstream;

            filefunc::openfile(curr_script_filename, outstream);

            outstream << "/usr/local/anaconda/bin/python \\" << endl;
            outstream << cnn_vis_pathname << " \\" << endl;
            outstream << "--deploy_txt=" << test_prototxt_filename 
                      << " \\" << endl;
            outstream << "--caffe_model=" << caffe_model_filename 
                      << " \\" << endl;
            outstream << "--image_type=amplify_neuron \\" << endl;
            outstream << "--target_layer="+orig_layer_name+" \\" << endl;
            outstream << "--target_neuron="+stringfunc::number_to_string(node)
               +" \\" << endl;
            outstream << "--output_file="+curr_node_img_filename+" \\" << endl;
            outstream << "--rescale_image \\" << endl;
            outstream << "--gpu=0   \\" << endl;

// Aug 18 parameters:

            outstream << "--initialization_scale="
                      << init_scale << " \\" << endl;
					// empirically optimized
            outstream << "--initialization_blur=5 \\" << endl;
					// empirically optimized
            outstream << "--num_steps=60 \\" << endl;
					// empirically optimized
            outstream << "--batch_size=1 \\" << endl;
            outstream << "--output_iter=100 \\" << endl;
            outstream << "--learning_rate=0.8 \\" << endl;
			                // empirically optimized
            outstream << "--learning_rate_decay_iter=20 \\" << endl;
            outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
            outstream << "--decay_rate=0.95 \\" << endl;  
					// empirically optimized
            outstream << "--alpha=4.0 \\" << endl;  
					// empirically optimized
            outstream << "--p_reg=0.0001 \\" << endl;
            outstream << "--beta=2.5 \\" << endl;
            outstream << "--tv_reg=0.00001 \\" << endl;
					// empirically optimized
            outstream << "--tv_reg_step_iter=25 \\" << endl;
					// empirically optimized
            outstream << "--tv_reg_step=0.00010 \\" << endl;
					// empirically optimized
            outstream << "--num_sizes=1 \\" << endl;
            outstream << "--iter_behavior=print" << endl;


/*
// Aug 17 parameters:
            outstream << "--num_steps=100 \\" << endl;
					// empirically optimized
            outstream << "--batch_size=25 \\" << endl;
            outstream << "--output_iter=100 \\" << endl;
            outstream << "--learning_rate=0.1 \\" << endl;
            outstream << "--learning_rate_decay_iter=100 \\" << endl;
            outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
            outstream << "--decay_rate=0.95 \\" << endl;  
					// empirically optimized
            outstream << "--alpha=4.0 \\" << endl;  
					// empirically optimized
            outstream << "--p_reg=0.0001 \\" << endl;
            outstream << "--beta=2.5 \\" << endl;
            outstream << "--tv_reg=0.00001 \\" << endl;
					// empirically optimized
            outstream << "--tv_reg_step_iter=25 \\" << endl;
					// empirically optimized
            outstream << "--tv_reg_step=0.00010 \\" << endl;
					// empirically optimized
            outstream << "--num_sizes=1 \\" << endl;
            outstream << "--iter_behavior=print" << endl;
*/

            filefunc::closefile(curr_script_filename, outstream);
            filefunc::make_executable(curr_script_filename);

            all_scripts_stream << "./" << curr_script_basename << endl;

         } // loop over node index
      } // loop over iter index
   } // loop over layer index

   filefunc::closefile(all_scripts_filename, all_scripts_stream);
   filefunc::make_executable(all_scripts_filename);

   string banner="Exported cnn_vis scripts to "+scripts_subdir;
   outputfunc::write_banner(banner);
   banner="Exported script to run all layers/nodes to "+
      all_scripts_filename;
   outputfunc::write_banner(banner);
}

