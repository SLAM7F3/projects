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
// Last updated on 9/12/16; 9/17/16; 9/18/16; 9/19/16
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
#include "numrec/nrfuncs.h"
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

double get_ran_factor()
{
   double ran_power = -1 + 2 * nrfunc::ran1();

//   double ran_factor = pow(1.2, ran_power);
//   double ran_factor = pow(1.5, ran_power);
//   double ran_factor = pow(2.0, ran_power);
   double ran_factor = pow(3.0, ran_power);

//   double ran_factor = 1;
   return ran_factor;
}


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
//      max_nodes_per_param_layer = 20;
//      max_nodes_per_param_layer = 25;
      max_nodes_per_param_layer = 257;
   }

   cout << "caffe_model_basename = " << caffe_model_basename << endl;
   cout << "Alexnet_flag = " << Alexnet_flag << endl;
   cout << "Googlenet_flag = " << Googlenet_flag << endl;
   cout << "VGG_flag = " << VGG_flag << endl;
   cout << "Resnet50_flag = " << Resnet50_flag << endl;
   cout << "Facenet_flag = " << Facenet_flag << endl;

   string scripts_subdir;
   string facenet_model_label;
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
      scripts_subdir="./vis_facenet2/";

      cout << "Enter facenet model label: (e.g. 2e, 2n, 2r, 2t, 2u)" << endl;
      cin >> facenet_model_label;
      string scripts_subsubdir = "model_"+facenet_model_label;
      filefunc::add_trailing_dir_slash(scripts_subsubdir);
      scripts_subdir += scripts_subsubdir;
   }

   filefunc::dircreate(scripts_subdir);
   string node_images_subdir=scripts_subdir+"node_images/";
   filefunc::dircreate(node_images_subdir);

   string all_scripts_filename=scripts_subdir+"run_all_scripts";
   ofstream all_scripts_stream;
   filefunc::openfile(all_scripts_filename, all_scripts_stream);

   int minor_layer_skip = -1;
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
      if(facenet_model_label == "2e")
      {
         minor_layer_skip = 2;       
         param_layer_names.push_back("conv1");
         param_layer_names.push_back("conv2");
         param_layer_names.push_back("conv3");
         param_layer_names.push_back("conv4");
         param_layer_names.push_back("fc5");
         param_layer_names.push_back("fc6");
         param_layer_names.push_back("fc7_faces");
      }
      else if (facenet_model_label == "2n" ||
               facenet_model_label == "2q" ||
               facenet_model_label == "2r" ||
               facenet_model_label == "2s" ||
               facenet_model_label == "2t")
      {
         minor_layer_skip = 6;       
         param_layer_names.push_back("conv1");
         param_layer_names.push_back("conv2");
         param_layer_names.push_back("conv3a");
         param_layer_names.push_back("conv3b");
         param_layer_names.push_back("conv4a");
         param_layer_names.push_back("conv4b");
         param_layer_names.push_back("fc5");
         param_layer_names.push_back("fc6");
         param_layer_names.push_back("fc7_faces");
      }
      else if (facenet_model_label == "2u")
      {
         minor_layer_skip = 6;       
         param_layer_names.push_back("conv1");
         param_layer_names.push_back("conv2");
         param_layer_names.push_back("conv3a");
         param_layer_names.push_back("conv3b");
         param_layer_names.push_back("conv4a");
         param_layer_names.push_back("conv4b");
         param_layer_names.push_back("fc5");
         param_layer_names.push_back("fc6_faces");
      }
      else
      {
         cout << "Unsupported facenet model label " << endl;
         exit(-1);
      }

//      init_scale = 15; // empirically reduced for 96x96 face images
      init_scale = 10; // empirically reduced for 96x96 face images model 2u
   }

   int n_major_layers = param_layer_names.size() + 1;
   for(unsigned int p = 0; p < param_layer_names.size(); p++)
   {
      cout << "Param layer p = " << p << " is named " << param_layer_names[p]
           << endl;
   }

   caffe_classifier classifier(test_prototxt_filename, caffe_model_filename,
                               n_major_layers, minor_layer_skip);

   vector<int> n_layer_nodes = classifier.get_n_param_layer_nodes();
   for(unsigned int n = 0; n < n_layer_nodes.size(); n++)
   {
      cout << "n = " << n << " n_layer_nodes = "
           << n_layer_nodes[n] << endl;
   }

   string cnn_vis_pathname = "/usr/local/python/cnn_vis.py";

   int layer_start = 1;   // We name very first layer as 1 rather than 0
//   int layer_stop =  1;
   int final_layer = param_layer_names.size();
   int layer_stop = final_layer;


// FAKE FAKE:  Sun Sep 18 at 12:57 pm 
//   layer_start = layer_stop = 5;   // conv4a
//   layer_start = layer_stop = 6;   // conv4b
   layer_start = layer_stop = 7;   // fc5
//   layer_start = layer_stop = 8;   // fc6
//   layer_start = layer_stop = 9;   // fc7
   
   cout << "layer_start = " << layer_start << endl;
   cout << "layer_stop = " << layer_stop << endl;

   outputfunc::enter_continue_char();

// For progress reporting purposes, first count total number of nodes
// whose images will be generated via backprojection:

   int n_total_nodes_to_process = 0;
   for(int layer = layer_stop; layer >= layer_start; layer--)
   {
      int layer_index = layer - 1;
      int max_iters = 1;


      if(layer == final_layer && Facenet_flag)
      {
//         max_iters = 100;
         max_iters = 150;
      }
      n_total_nodes_to_process += max_iters * n_layer_nodes[layer_index];
   }
   
   int node_counter = 0;
   classifier.set_minor_layer_skip(minor_layer_skip);
   for(int layer = layer_stop; layer >= layer_start; layer--)
   {
      int layer_index = (layer - 1) * minor_layer_skip;

      int max_iters = 1;
      if(layer == final_layer && Facenet_flag)
      {
//         max_iters = 100;
         max_iters = 150;
      }

      for(int iter = 0; iter < max_iters; iter++)
      {
         int start_node = 0;
         int stop_node = n_layer_nodes[layer_index];
//         int start_node = 1; // Use to extract just male/female chips for
//                             //  final fully connected layer from Facenet2
         cout << "layer = " << layer
              << " layer_index = " << layer_index
              << " n_layer_nodes = " << n_layer_nodes[layer_index]
              << endl;

         int node_skip = 1;
         for(int node = start_node; node < stop_node; node += node_skip)
         {
//            outputfunc::update_progress_and_remaining_time(
//               node_counter++, 100, n_total_nodes_to_process);
            
            if(node > max_nodes_per_param_layer) continue;

            string orig_layer_name=
               param_layer_names[layer_index/minor_layer_skip];
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

/*
// Sep 19, 2016 parameters (yields mediocre results for all
// layers except fc6_faces within 6-conv layer model 2u with batch
// normalization):

            outstream << "--initialization_scale="
//                      << init_scale * get_ran_factor() << " \\" << endl;
                      << init_scale  << " \\" << endl;
            double init_blur = 5;
            outstream << "--initialization_blur="
//                      << init_blur * get_ran_factor() << " \\" << endl;
                      << init_blur << " \\" << endl;
            int num_steps = 120;
            outstream << "--num_steps="
//                      << int(num_steps * get_ran_factor()) << " \\" << endl;
                      << int(num_steps) << " \\" << endl;
            outstream << "--batch_size=1 \\" << endl;
            outstream << "--output_iter=100 \\" << endl;
//            double learning_rate = 2.0;
            double learning_rate = 5;
            outstream << "--learning_rate="
                      << learning_rate * get_ran_factor() << " \\" << endl;
            outstream << "--learning_rate_decay_iter=40 \\" << endl;
            outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
            outstream << "--decay_rate=0.95 \\" << endl;  
//            double alpha = 3.0;
//            double alpha = 1.0;
            double alpha = 0.4;
            outstream << "--alpha="
                      << alpha * get_ran_factor() << " \\" << endl;  
            double p_reg = 5E-6;
            outstream << "--p_reg="
                      << p_reg << " \\" << endl;
//                      << p_reg * get_ran_factor() << " \\" << endl;
            outstream << "--p_scale=1.0 \\" << endl;
            double beta = 1.7;
            outstream << "--beta="
                      << beta << " \\" << endl;
//                      << beta * get_ran_factor() << " \\" << endl;
            double tv_reg = 1E-5;
            outstream << "--tv_reg=" 
                      << tv_reg * get_ran_factor() << " \\" << endl;
            outstream << "--tv_reg_step_iter=40 \\" << endl;
            outstream << "--tv_reg_step=1E-4 \\" << endl;
            outstream << "--num_sizes=1 \\" << endl;
            outstream << "--iter_behavior=print" << endl;
*/

/*
// Sep 19, 2016 parameters (yields mediocre results for final faces
// layer of 6-conv layer model 2u with batch normalization):

            outstream << "--initialization_scale="
                      << init_scale  << " \\" << endl;
            double init_blur = 5;
            outstream << "--initialization_blur="
                      << init_blur << " \\" << endl;
            int num_steps = 120;
            outstream << "--num_steps="
                      << int(num_steps) << " \\" << endl;
            outstream << "--batch_size=1 \\" << endl;
            outstream << "--output_iter=100 \\" << endl;
            double learning_rate = 1;
            outstream << "--learning_rate="
//                      << learning_rate << " \\" << endl;
                      << learning_rate * get_ran_factor() << " \\" << endl;
            outstream << "--learning_rate_decay_iter=4000 \\" << endl;
            outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
            outstream << "--decay_rate=0.95 \\" << endl;  
            double alpha = 1.6;
            outstream << "--alpha="
//                      << alpha  << " \\" << endl;  
                      << alpha * get_ran_factor() << " \\" << endl;  
            double p_reg = 1E-5;
            outstream << "--p_reg="
//                      << p_reg  << " \\" << endl;
                      << p_reg * get_ran_factor() << " \\" << endl;
            outstream << "--p_scale=1.0 \\" << endl;
            double beta = 2;
            outstream << "--beta="
//                      << beta << " \\" << endl;
                      << beta * get_ran_factor() << " \\" << endl;
            double tv_reg = 2.5E-5;
            outstream << "--tv_reg=" 
//                      << tv_reg << " \\" << endl;
                      << tv_reg * get_ran_factor() << " \\" << endl;
            outstream << "--tv_reg_step_iter=40 \\" << endl;
            outstream << "--tv_reg_step=1E-4 \\" << endl;
            outstream << "--num_sizes=1 \\" << endl;
            outstream << "--iter_behavior=print" << endl;
*/


/*
// Sep 18, 2016 parameters (yields decent results for all layers
// except fc7_faces within 6-conv layer model 2t with batch normalization):

            outstream << "--initialization_scale="
                      << init_scale  << " \\" << endl;
//                      << init_scale * get_ran_factor() << " \\" << endl;
            double init_blur = 5;
            outstream << "--initialization_blur="
                      << init_blur << " \\" << endl;
//                      << init_blur * get_ran_factor() << " \\" << endl;
            int num_steps = 120;
            outstream << "--num_steps="
                      << int(num_steps) << " \\" << endl;
//                      << int(num_steps * get_ran_factor()) << " \\" << endl;
            outstream << "--batch_size=1 \\" << endl;
            outstream << "--output_iter=100 \\" << endl;
//            double learning_rate = 2.0;
            double learning_rate = 3.6;
            outstream << "--learning_rate="
                      << learning_rate << " \\" << endl;
//                      << learning_rate * get_ran_factor() << " \\" << endl;
            outstream << "--learning_rate_decay_iter=4000 \\" << endl;
            outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
            outstream << "--decay_rate=0.95 \\" << endl;  
//            double alpha = 3.0;
            double alpha = 1.0;
            outstream << "--alpha="
                      << alpha  << " \\" << endl;  
//                      << alpha * get_ran_factor() << " \\" << endl;  
//            double p_reg = 1E-5;
            double p_reg = 3.7E-6;
            outstream << "--p_reg="
                      << p_reg << " \\" << endl;
//                      << p_reg * get_ran_factor() << " \\" << endl;
            outstream << "--p_scale=1.0 \\" << endl;
//            double beta = 2.25;
            double beta = 1.2;
            outstream << "--beta="
                      << beta << " \\" << endl;
//                      << beta * get_ran_factor() << " \\" << endl;
            double tv_reg = 2E-5;
            outstream << "--tv_reg=" 
                      << tv_reg << " \\" << endl;
//                      << tv_reg * get_ran_factor() << " \\" << endl;
            outstream << "--tv_reg_step_iter=4000 \\" << endl;
            outstream << "--tv_reg_step=1E-4 \\" << endl;
            outstream << "--num_sizes=1 \\" << endl;
            outstream << "--iter_behavior=print" << endl;
*/


// Sep 18, 2016 parameters (yields good results for final faces
// layer of 6-conv layer model 2t with batch normalization):

            outstream << "--initialization_scale="
                      << init_scale << " \\" << endl;
            outstream << "--initialization_blur=5 \\" << endl;
            outstream << "--num_steps=120 \\" << endl;
            outstream << "--batch_size=1 \\" << endl;
            outstream << "--output_iter=100 \\" << endl;
            outstream << "--learning_rate=2.0 \\" << endl;
            outstream << "--learning_rate_decay_iter=40 \\" << endl;
            outstream << "--learning_rate_decay_fraction=0.5 \\" << endl;
            outstream << "--decay_rate=0.95 \\" << endl;  
            outstream << "--alpha=2.0 \\" << endl;  
            outstream << "--p_reg=1E-5 \\" << endl;
            outstream << "--p_scale=1.0 \\" << endl;
            outstream << "--beta=2.25 \\" << endl;
            outstream << "--tv_reg=2E-5 \\" << endl;
            outstream << "--tv_reg_step_iter=40 \\" << endl;
            outstream << "--tv_reg_step=1E-4 \\" << endl;


/*
// Aug 18 parameters (yields decent results for 4-conv & 6-conv layer
// models without batch normalization):

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

