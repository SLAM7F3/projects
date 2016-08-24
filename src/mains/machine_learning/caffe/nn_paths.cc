// ========================================================================
// Program NN_PATHS 

// ./nn_paths 
// /data/caffe/faces/trained_models/test_96.prototxt                     
// /data/caffe/faces/trained_models/Aug2_184K_T3/train_iter_200000.caffemodel 
// /data/caffe/faces/image_chips/testing/Jul30_and_31_96x96

// ========================================================================
// Last updated on 8/16/16; 8/23/16; 8/24/16
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
   string subnetwork_subdir = "./vis_facenet/node_images/subnetworks/";
   filefunc::dircreate(subnetwork_subdir);
   string male_subnetwork_subdir=subnetwork_subdir+"male/";
   string female_subnetwork_subdir=subnetwork_subdir+"female/";
   filefunc::dircreate(male_subnetwork_subdir);
   filefunc::dircreate(female_subnetwork_subdir);

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

   int istart=0;
//   int istop = 500;
   int istop = n_images;
   string unix_cmd;

   typedef std::map<DUPLE, vector<double>, ltduple> NODE_ACTIVATIONS_MAP;
// independent DUPLE contains (local ID for node in layer L, layer ID)
// dependent vector<double> contains node activations for all testing images
   NODE_ACTIVATIONS_MAP node_activations_map;
//   NODE_ACTIVATIONS_MAP female_node_activations_map, male_node_activations_map;
   NODE_ACTIVATIONS_MAP::iterator node_activations_iter;

//   const double strong_activation_threshold = 0.2;
   const double strong_activation_threshold = 0.25;

   vector<string> blob_names;
   blob_names.push_back("conv1a");
   blob_names.push_back("conv2a");
   blob_names.push_back("conv3a");
   blob_names.push_back("conv4a");
   blob_names.push_back("fc5");
   blob_names.push_back("fc6");
   blob_names.push_back("fc7_faces");
   unsigned int n_layers = blob_names.size();

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

      int true_label = -1;
      string true_gender_label = substrings[0];
      if(true_gender_label == "non")
      {
         true_label = 0;
      }
      else if(true_gender_label == "male")
      {
         true_label = 1;
      }
      else if(true_gender_label == "female")
      {
         true_label = 2;
      }

      string classification_gender_label;
      if(classification_label == 0)
      {
         classification_gender_label = "non";
      }
      else if(classification_label == 1)
      {
         classification_gender_label = "male";
      }
      else if(classification_label == 2)
      {
         classification_gender_label = "female";
      }

      cout << "true_gender_label = " << true_gender_label << endl;
//      cout << "classification_gender_label = " << classification_gender_label
//           << " classification score = " << classification_score
//      << endl;
      int n_strong_activations = 0;

// Periodically update median node activation information:

      if(i > istart && i%50 == 0)
      {
         string activations_filename=imagechips_subdir+"activations_"
            +stringfunc::integer_to_string(i,4)+".dat";
         ofstream activations_stream;
         filefunc::openfile(activations_filename, activations_stream);
         activations_stream << 
            "# layer  node  stim_frac  median  quartile   mean     sigma"
                            << endl;
         activations_stream << "n_images = " << i << endl;
         activations_stream << endl;

         for(node_activations_iter = node_activations_map.begin();
             node_activations_iter != node_activations_map.end();
             node_activations_iter++)
         {
            DUPLE curr_duple = node_activations_iter->first;
            vector<double> curr_activations = node_activations_iter->second;
      
// Count fraction of test images which fire current node:

            int n_stimulations = 0;
            for(unsigned int j = 0; j < curr_activations.size(); j++)
            {
               if(curr_activations[j] > 0) n_stimulations++;
            }
            double stimulation_frac = double(n_stimulations)/(i-istart+1);

            double mu, sigma;
            mathfunc::mean_and_std_dev(curr_activations, mu, sigma);

            double median, quartile_width;
            mathfunc::median_value_and_quartile_width(
               curr_activations, median, quartile_width);
            activations_stream << curr_duple.first << "   "
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

      }
      
      for(unsigned int layer = 0; layer < n_layers; layer++)
      {
         vector<int> node_IDs;
         vector<double> node_activations;
         int n_tiny_values = classifier.retrieve_layer_activations(
            blob_names[layer], node_IDs, node_activations);
         int n_layer_nodes = node_activations.size();
         for(int n = 0; n < n_layer_nodes; n++)
         {
            DUPLE curr_duple(layer, node_IDs[n]);
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

         unix_cmd = "montage -tile "+stringfunc::number_to_string(n_max)+"x1 ";
         for(int n = 0; n < n_max; n++)
         {
//            cout << "     Node ID = " << node_IDs[n]
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
               stringfunc::integer_to_string(node_IDs[n],3)+".png";
            string chip_filename = imagechips_subdir+layer_name+"/"+
               chip_basename;

            string chiplabel_str = " -label ";
            if(layer == n_layers - 1)
            {
               chiplabel_str += "'"+true_gender_label+" c="+
                  stringfunc::number_to_string(classification_score,3)+"' ";
            }
            else if(renorm_node_activations.size() > 0)
            {
               chiplabel_str += "' node "+stringfunc::number_to_string(
                  node_IDs[n])+"  a="+stringfunc::number_to_string(
                  renorm_node_activations[n],3)+"' ";
            }
            unix_cmd += chiplabel_str+" "+chip_filename+" ";
         } // loop over index n 
         

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
         unix_cmd += " "+montage_filename;
//         cout << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);

      } // loop over layer index
       
      if(classification_label == 0) continue;
      if(classification_score < 0.95) continue;
      if(n_strong_activations < 3) continue;

      unix_cmd = "montage -geometry +2+2 -tile 1x"
         +stringfunc::number_to_string(n_layers+1)
         +" "+image_filename+" ";

      vector<string> layer_montage_filenames;
      for(unsigned int layer = 0; layer < n_layers; layer++)
      {
         layer_montage_filenames.push_back(
            "layer_"+
            stringfunc::integer_to_string(layer+1,3)+".png");
         unix_cmd += layer_montage_filenames.back()+" ";
      }

      string network_montage_filename=male_subnetwork_subdir;
      if(true_gender_label == "female")
      {
         network_montage_filename=female_subnetwork_subdir;
      }
      
      network_montage_filename += "network_"+
         stringfunc::integer_to_string(i,4)+".png";
      unix_cmd += network_montage_filename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      string banner="Exported "+network_montage_filename;
      outputfunc::write_banner(banner);

      for(unsigned int layer = 0; layer < n_layers; layer++)
      {
         filefunc::deletefile(layer_montage_filenames[layer]);
      }

   } // loop over index i labeling input image tiles

   cout << "n_images = " << n_images << endl;  // 4536
   cout << "test.prototxt = " << test_prototxt_filename << endl;
   cout << "trained caffe model = " << caffe_model_filename << endl;
   cout << "input_images_subdir = " << input_images_subdir << endl;
   
}

   
