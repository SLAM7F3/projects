// ========================================================================
// Program RAW_ACTIVATIONS imports a trained FACENET caffe model.  It
// also takes in testing images which the model has never seen before.
// For each FACENET neuron, we count how many input test images cause
// it to fire.  We also record the mean and median values of its
// non-zero activations.  Activation statistics for each node in each
// layer are periodically exported to output text files within
// activations_subdir.  Any test image which has zero activation for
// all nodes within a network layer is copied into a subdirectory of
// zero_activations_subdir.

// RAW_ACTIVATIONS exports text files for each input test image
// containing activation values for all nodes which fired.  It also
// exports a text file containing the minimum and maximum nonzero
// activation values for each layer.

// ./raw_activations
// /data/caffe/faces/trained_models/test_96.prototxt                     
// /data/caffe/faces/trained_models/Aug2_184K_T3/train_iter_200000.caffemodel 
// /data/caffe/faces/image_chips/testing/Jul30_and_31_96x96

// ========================================================================
// Last updated on 8/25/16; 8/26/16; 9/5/16; 9/7/16
// ========================================================================

#include "classification/caffe_classifier.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::cerr;
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
   timefunc::initialize_timeofday_clock();

   string test_prototxt_filename   = argv[1];
   string caffe_model_filename = argv[2];
   caffe_classifier classifier(test_prototxt_filename, caffe_model_filename);

   string input_images_subdir = argv[3];
   filefunc::add_trailing_dir_slash(input_images_subdir);

   string imagechips_subdir = "./vis_facenet/node_images/";
   string subnetwork_subdir = imagechips_subdir + "subnetworks/";
   filefunc::dircreate(subnetwork_subdir);
   string activations_subdir = imagechips_subdir + "activations/";
   filefunc::dircreate(activations_subdir);
   string node_activations_subdir = activations_subdir + "nodes/";
   filefunc::dircreate(node_activations_subdir);
   string image_activations_subdir = activations_subdir + "images/";
   filefunc::dircreate(image_activations_subdir);
   string zero_activations_subdir = image_activations_subdir + 
      "zero_activations/";
   filefunc::dircreate(zero_activations_subdir);

// Save min and max nonzero activations for each layer calculated
// across all test images and for all nodes within each layer in
// following STL map

   typedef std::map<int, twovector> EXTREMAL_ACTIVATIONS_MAP;
// independent int = layer ID
// dependent twovector holds min and max activations for all nodes
// within a layer

   EXTREMAL_ACTIVATIONS_MAP extremal_activations_map;
   EXTREMAL_ACTIVATIONS_MAP::iterator extremal_activations_iter;

// Following blob names are appropriate for Facenet 1 only !!!

   vector<string> blob_names;
   blob_names.push_back("conv1a");
   blob_names.push_back("conv2a");
   blob_names.push_back("conv3a");
   blob_names.push_back("conv4a");
   blob_names.push_back("fc5");
   blob_names.push_back("fc6");
   blob_names.push_back("fc7_faces");
   unsigned int n_layers = blob_names.size();

// Following RGB mean values are appropriate for Facenet 1 only !!!

   double Bmean = 104.008;
   double Gmean = 116.669;
   double Rmean = 122.675;
   classifier.set_mean_bgr(Bmean, Gmean, Rmean);

   classifier.add_label("non");
   classifier.add_label("male");
   classifier.add_label("female");
   classifier.add_label("unsure");

   bool search_all_children_dirs_flag = false;
//   bool search_all_children_dirs_flag = true;
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      input_images_subdir, search_all_children_dirs_flag);
   int n_images = image_filenames.size();
   vector<int> shuffled_image_indices = mathfunc::random_sequence(n_images);
   cout << "Total number of test images = " << n_images << endl;
   exit(-1);

   int istart=0;
//   int istop = 500;
   int istop = n_images;

   string montage_filename="montage_cmds.dat";
   ofstream montagestream;
   filefunc::openfile(montage_filename, montagestream);

   typedef std::map<DUPLE, vector<double>, ltduple> NODE_ACTIVATIONS_MAP;
// independent DUPLE contains (layer ID, local ID for node in layer L)
// dependent vector<double> contains node activations for all testing images
   NODE_ACTIVATIONS_MAP node_activations_map;
//   NODE_ACTIVATIONS_MAP female_node_activations_map, male_node_activations_map;
   NODE_ACTIVATIONS_MAP::iterator node_activations_iter;

//   const double strong_activation_threshold = 0.2;
   const double strong_activation_threshold = 0.25;

   for(int i = istart; i < istop; i++)
   {
      outputfunc::update_progress_and_remaining_time(
         i, 500, (istop-istart));
      int image_ID = shuffled_image_indices[i];
      
      string orig_image_filename = image_filenames[image_ID];
      string image_filename = orig_image_filename;
      string image_basename = filefunc::getbasename(image_filename);
      cout << "i = " << i 
//           << " image_basename = " << image_basename 
           << " image_filename = " << image_filename
           << endl;

      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(image_basename,"_");

      unsigned int input_img_width, input_img_height;
      imagefunc::get_image_width_height(
         orig_image_filename, input_img_width, input_img_height);

      if(input_img_width != 96 || input_img_height != 96)
      {
         cout << "Error" << endl;
         cout << "input_img_width = " << input_img_width << endl;
         cout << "input_img_height = " << input_img_height << endl;
      }

      texture_rectangle curr_image(image_filename, NULL);
      classifier.rgb_img_to_bgr_fvec(curr_image);
      classifier.generate_dense_map();

      int classification_label = classifier.get_classification_result();
      double classification_score=classifier.get_classification_score();

      string true_gender_label = substrings[0];
//      cout << "true_gender_label = " << true_gender_label << endl;
      int n_strong_activations = 0;

// Periodically update node activation frequency and non-zero value
// information:

      if(i > istart && i%50 == 0)
      {
         string activations_filename=activations_subdir+"activations_"
            +stringfunc::integer_to_string(i,4)+".dat";
         ofstream activations_stream;
         filefunc::openfile(activations_filename, activations_stream);
         activations_stream << "#  n_images = " << i << endl;
         activations_stream << endl;

         activations_stream << 
"# Layer  Global node  Local node  Stimulation  Median        Quartile_width  Mean            Sigma" << endl;
         activations_stream << 
"#  ID       ID            ID      fraction     activation    activation      activation      activation" << endl;
         activations_stream << endl;

// Recall global node IDs 0, 1 and 2 are reserved for the RGB channels
// within the input imagery data layer 0:

         int n_RGB_channels = 3;
         int global_node_counter = n_RGB_channels;

         for(node_activations_iter = node_activations_map.begin();
             node_activations_iter != node_activations_map.end();
             node_activations_iter++)
         {
            DUPLE curr_duple = node_activations_iter->first;
            vector<double> curr_activations = node_activations_iter->second;
      
// Count fraction of test images which fire current node:

            vector<double> nonzero_activations;
            for(unsigned int j = 0; j < curr_activations.size(); j++)
            {
               if(curr_activations[j] > 0)
               {
                  nonzero_activations.push_back(curr_activations[j]);
               }
            }
            double stimulation_frac = double(nonzero_activations.size())/
               (i-istart+1);

            double mu = 0, sigma = 0;
            double median = 0, quartile_width = 0;

            if(nonzero_activations.size() > 0)
            {
               mathfunc::mean_and_std_dev(nonzero_activations, mu, sigma);
               mathfunc::median_value_and_quartile_width(
                  nonzero_activations, median, quartile_width);
            }

// Recall layer 0 = input imagery data layer:

            int curr_layer = curr_duple.first + 1;
            activations_stream << curr_layer << "   "
                               << global_node_counter++ << "   "
                               << curr_duple.second << "   "
                               << stimulation_frac << " \t"
                               << median << " \t\t"
                               << quartile_width << " \t\t"
                               << mu << " \t\t"
                               << sigma << " \t\t"
                               << endl;
         } // loop over node activations iterator

         filefunc::closefile(activations_filename, activations_stream);
         string banner="Exported "+activations_filename;
         outputfunc::write_banner(banner);
      } // i > istart && i%50 == 0 conditional

      vector<string> montage_lines;
      montage_lines.push_back("0  "+image_filename);

// Open text file for current image into which we save all nonzero
// node activation information:

      string currimage_activations_filename=image_activations_subdir+
         "image_activations_"+stringfunc::integer_to_string(i,4)+".dat";
      ofstream image_activations_stream;
      filefunc::openfile(
         currimage_activations_filename, image_activations_stream);
      image_activations_stream << image_filename << endl << endl;

      double softmax_denom = 0;
      for(unsigned int layer = 0; layer < n_layers; layer++)
      {
         vector<int> local_node_IDs;
         vector<double> node_activations;

         bool sort_activations_flag = true;
         if(layer == n_layers - 1)
         {
            sort_activations_flag = false;
         }
         
         int n_tiny_values = classifier.retrieve_layer_activations(
            blob_names[layer], local_node_IDs, node_activations,
            sort_activations_flag);
         int n_layer_nodes = node_activations.size();
         double layer_nonzero_activations_frac = 
            double(n_layer_nodes - n_tiny_values) / n_layer_nodes;
         image_activations_stream << endl;
         image_activations_stream << "#  " << layer_nonzero_activations_frac
                                  << " of nodes in layer " << layer+1
                                  << " have nonzero activations " 
                                  << endl << endl;

         double min_nonzero_activation = 1E10;
         double max_nonzero_activation = - min_nonzero_activation;
         bool zero_activations_flag = true;
         for(int n = 0; n < n_layer_nodes; n++)
         {

// Export nonzero node activations for current image to output text
// file:

            if(node_activations[n] > 0)
            {
               image_activations_stream << layer+1 << "  "
                                        << local_node_IDs[n] << "  "
                                        << node_activations[n] << endl;
               min_nonzero_activation = basic_math::min(
                  min_nonzero_activation, node_activations[n]);
               max_nonzero_activation = basic_math::max(
                  max_nonzero_activation, node_activations[n]);
               zero_activations_flag = false;
            }

            if(layer == n_layers - 1)
            {
               softmax_denom += exp(node_activations[n]);
            }

            DUPLE curr_duple(layer, local_node_IDs[n]);
            node_activations_iter = node_activations_map.find(curr_duple);
            if(node_activations_iter == node_activations_map.end())
            {
               vector<double> curr_node_activations;
               curr_node_activations.push_back(node_activations[n]);
               node_activations_map[curr_duple] = curr_node_activations;
            }
            else
            {
               node_activations_iter->second.push_back(node_activations[n]);
            }
         } // loop over index n labeling nodes within current layer

// If current image has zero activation for all nodes within current
// layer, copy it into a subdirectory of zero_activations_subdir:

         if(zero_activations_flag)
         {
            string zero_activations_subsubdir = zero_activations_subdir+
               stringfunc::number_to_string(layer+1)+"/";
            filefunc::dircreate(zero_activations_subsubdir);
            string unix_cmd = "cp "+image_filename+" "+
               zero_activations_subsubdir;
            sysfunc::unix_command(unix_cmd);
         }

         extremal_activations_iter = extremal_activations_map.find(
            layer+1);
         if(extremal_activations_iter == extremal_activations_map.end())
         {
            extremal_activations_map[layer+1] = 
               twovector(min_nonzero_activation, max_nonzero_activation);
         }
         else
         {
            double prev_min_nonzero_activation = 
               extremal_activations_iter->second.get(0);
            double prev_max_nonzero_activation = 
               extremal_activations_iter->second.get(1);
            min_nonzero_activation = basic_math::min(
               min_nonzero_activation, prev_min_nonzero_activation);
            max_nonzero_activation = basic_math::max(
               max_nonzero_activation, prev_max_nonzero_activation);
            extremal_activations_iter->second.put(0,min_nonzero_activation);
            extremal_activations_iter->second.put(1,max_nonzero_activation);
         }

// Convert raw node activations into relative fractions:

         double denom = 0;
         for(int n = 0; n < n_layer_nodes; n++)
         {
            denom += node_activations[n];
         }

         vector<double> renorm_node_activations;
         for(int n = 0; denom > 0 && n < n_layer_nodes; n++)
         {
            renorm_node_activations.push_back(node_activations[n] / denom);
         }
         
         string layer_name = blob_names[layer];
//         cout << "Layer = " << layer_name << endl;
//         unsigned n_max = 4;
         int n_max = 5;
         if(n_layer_nodes < n_max) n_max = n_layer_nodes;
         if(layer == n_layers - 1) n_max = 1;

         for(int n = 0; n < n_max; n++)
         {
//            cout << "     Local Node ID = " << local_node_IDs[n]
//                 << " Activation = " << node_activations[n];
            if(renorm_node_activations.size() > 0)
            {
//               cout << " Renormalized activation = " 
//                    << renorm_node_activations[n];
               if(layer < blob_names.size() - 1 &&
                  renorm_node_activations[n] > strong_activation_threshold)
               {
                  n_strong_activations++;
//                  cout << "  Strong activation ";
               }
            }
//            cout << endl;
            string chip_basename = layer_name+"_"+
               stringfunc::integer_to_string(local_node_IDs[n],3)+".png";
            string chip_filename = imagechips_subdir+layer_name+"/"+
               chip_basename;

            string chiplabel_str = " -label ";
            if(layer == n_layers - 1)
            {
               chiplabel_str += "'"+true_gender_label+" c="+
                  stringfunc::number_to_string(classification_score,3)+"' ";
               string curr_line = stringfunc::number_to_string(layer+1)+"  "
                  +true_gender_label+"  "
                  +stringfunc::number_to_string(classification_score)+"  "
                  +chip_filename;
               montage_lines.push_back(curr_line);
            }
            else if(renorm_node_activations.size() > 0)
            {
               chiplabel_str += "' node "+stringfunc::number_to_string(
                  local_node_IDs[n])+"  a="+stringfunc::number_to_string(
                  renorm_node_activations[n],3)+"' ";
               string curr_line = stringfunc::number_to_string(layer+1)+"  "
                  +stringfunc::number_to_string(local_node_IDs[n])+"  "
                  +stringfunc::number_to_string(renorm_node_activations[n])
                  +"  "+chip_filename;
               montage_lines.push_back(curr_line);
            }
         } // loop over index n labeling activated neurons' image chips
         
// Compute statistics for current layer's node activations:

         double mu_activation, sigma_activation;
         mathfunc::mean_and_std_dev(
            node_activations, mu_activation, sigma_activation);
         double median_activation, quartile_width;
         mathfunc::median_value_and_quartile_width(
            node_activations, median_activation, quartile_width);
//         cout << "     Activations: median=" << median_activation
//              << " quartile_width=" << quartile_width 
//              << "   mu=" << mu_activation 
//              << " sigma=" << sigma_activation << endl;

         string montage_filename = "layer_"+
            stringfunc::integer_to_string(layer+1,3)+".png";
      } // loop over layer index

// Compute softmax class probabilities from final layer:

      image_activations_stream << endl;
      image_activations_stream << "#  Softmax class probabilities" 
                               << endl << endl;

      bool sort_activations_flag = false;
      int layer = n_layers - 1;
      vector<int> local_node_IDs;
      vector<double> node_activations;
      int n_tiny_values = classifier.retrieve_layer_activations(
         blob_names[layer], local_node_IDs, node_activations, 
         sort_activations_flag);
      int n_layer_nodes = node_activations.size();
      for(int n = 0; n < n_layer_nodes; n++)
      {
         double softmax_prob = exp(node_activations[n]) / softmax_denom;
         string class_label = "non-face";
         if(n == 1)
         {
            class_label = "male face";
         }
         else if(n == 2)
         {
            class_label = "female face";
         }
         image_activations_stream << n << "   "
                                  << softmax_prob << "   # "
                                  << class_label
                                  << endl;
      }

      filefunc::closefile(
         currimage_activations_filename, image_activations_stream);

      string banner="Exported "+currimage_activations_filename;
      outputfunc::write_banner(banner);
       
      if(classification_label == 0) continue;
      if(classification_score < 0.95) continue;
      if(n_strong_activations < 3) continue;

// Check whether there exists at least one image chip corresponding to
// each Facnet layer 0 through n_layers:

      unsigned int n_represented_layers = 0;
      for(unsigned int layer = 0; layer <= n_layers; layer++)
      {
         for(unsigned int m = 0; m < montage_lines.size(); m++)
         {
            vector<string> substrings = 
               stringfunc::decompose_string_into_substrings(montage_lines[m]);
            if(layer == stringfunc::string_to_number(substrings[0]))
            {
               n_represented_layers++;
               break;

            }
         } // loop over index m
      } // loop over index layer

      if(n_represented_layers == n_layers+1)
      {
         for(unsigned int m = 0; m < montage_lines.size(); m++)
         {
            montagestream << montage_lines[m] << endl;
         }
         montagestream << endl;
      }
   } // loop over index i labeling input test images

   cout << "n_images = " << n_images << endl;  // 4536
   cout << "test.prototxt = " << test_prototxt_filename << endl;
   cout << "trained caffe model = " << caffe_model_filename << endl;
   cout << "input_images_subdir = " << input_images_subdir << endl;

   filefunc::closefile(montage_filename, montagestream);
   string banner="Exported "+montage_filename;
   outputfunc::write_banner(banner);

// Export nonzero activations for each node within entire network to
// individual text files:

   int n_nonzero_activations = 0;
   for(node_activations_iter = node_activations_map.begin();
       node_activations_iter != node_activations_map.end();
       node_activations_iter++)
   {
      DUPLE curr_duple = node_activations_iter->first;
      int layer_ID = curr_duple.first + 1;
      int local_node_ID = curr_duple.second;

      string node_activations_filename=node_activations_subdir+
         "node_activations_"+stringfunc::number_to_string(layer_ID)+"_"+
         stringfunc::integer_to_string(local_node_ID,3)+".dat";
      ofstream node_activations_stream;
      filefunc::openfile(node_activations_filename, node_activations_stream);
      
      vector<double> curr_activations = node_activations_iter->second;
      vector<double> nonzero_activations;
      for(unsigned int j = 0; j < curr_activations.size(); j++)
      {
         if(curr_activations[j] > 0)
         {
            node_activations_stream << curr_activations[j] << "  ";
            n_nonzero_activations++;
            if(n_nonzero_activations%10 == 0)
            {
               node_activations_stream << endl;
            }
         }
      }
      node_activations_stream << endl;
      filefunc::closefile(node_activations_filename, node_activations_stream);
   } // loop over node activations iterator

   banner="Exported all nonzero activations for all network nodes to "+
      node_activations_subdir;
   outputfunc::write_banner(banner);

// Export extremal nonzero activation values to output text file:

   string extremal_activations_filename=activations_subdir+
      "extremal_layer_activations.dat";
   ofstream extremal_stream;
   filefunc::openfile(extremal_activations_filename, extremal_stream);

   extremal_stream << "# ====================================================="
                   << endl;
   extremal_stream << "# Layer  Min non-zero   Max non-zero" << endl;
   extremal_stream << "#   ID    activation     activation" << endl;
   extremal_stream << "# ====================================================="
                   << endl << endl;

   for(extremal_activations_iter = extremal_activations_map.begin();
       extremal_activations_iter != extremal_activations_map.end();
       extremal_activations_iter++)
   {
      int layer = extremal_activations_iter->first;
      double min_nonzero_activation = extremal_activations_iter->second.get(0);
      double max_nonzero_activation = extremal_activations_iter->second.get(1);
      
      extremal_stream << layer << "   "
                      << min_nonzero_activation << "   "
                      << max_nonzero_activation 
                      << endl;
   }
   
   banner="Exported extremal activation values per layer to "+
      extremal_activations_filename;
   outputfunc::write_banner(banner);
   filefunc::closefile(extremal_activations_filename, extremal_stream);
}

   
