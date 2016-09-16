// ========================================================================
// Program RENORM_IMAGE_ACTIVATIONS imports extremal nonzero
// activation values for each layer output by program RAW_ACTIVATIONS.
// (The minimum activation values are all very close to 0.)  It also
// imports nonzero ordered activation values for every Facenet node
// calculated for each test image by program ORDERED_ACTIVATIONS.
// RENORM_IMAGE_ACTIVATIONS effectively rescales the ordered
// activation values to fractions ranging between 0 and 1.  It then
// assigns hues and values based upon these fractions so that
// fractionally large activations are colored bright and hot while
// fractionally small activations are colored dim and cold.  The
// renormalized activation fractions and RGB values are exported to a
// new set of text files for each test image.

//                      ./renorm_image_activations

// ========================================================================
// Last updated on 9/5/16; 9/7/16; 9/8/16; 9/14/16
// ========================================================================

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
#include "math/lttriple.h"
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
   string facenet_model_label = "2e";
   cout << "Enter facenet model label: (e.g. 2e, 2n, 2r)" << endl;
//   cin >> facenet_model_label;

   string network_subdir = "./vis_facenet/network/";
   string base_activations_subdir = network_subdir + "activations/";
   string activations_subdir = base_activations_subdir + "model_"+
      facenet_model_label+"/";
   string image_activations_subdir = activations_subdir + "images/";
   string renorm_image_activations_subdir = image_activations_subdir 
      + "renormalized/";
   filefunc::dircreate(renorm_image_activations_subdir);
   string node_activations_subdir = activations_subdir + "nodes/";

// First import min and max activation values for all nodes within
// each layer:

   typedef std::map<int, twovector> EXTREMAL_ACTIVATIONS_MAP;
// independent int = layer ID
// dependent twovector holds min and max activations for all nodes
// within a layer

   EXTREMAL_ACTIVATIONS_MAP extremal_activations_map;
   EXTREMAL_ACTIVATIONS_MAP::iterator extremal_activations_iter;

   string extremal_activations_filename = activations_subdir + 
      "extremal_layer_activations.dat";
   vector< vector<double> > row_values = filefunc::ReadInRowNumbers(
      extremal_activations_filename);
   
   for(unsigned int r = 0; r < row_values.size(); r++)
   {
      int layer_ID = row_values[r].at(0);
      double min_activation = row_values[r].at(1);
      double max_activation = row_values[r].at(2);
      extremal_activations_map[layer_ID] = 
         twovector(min_activation, max_activation);
   }

   string ordered_activations_filename = activations_subdir+
      "ordered_activations.dat";
   vector< vector<double> > row_numbers = filefunc::ReadInRowNumbers(
      ordered_activations_filename);

   typedef std::map<DUPLE, triple, ltduple> OLD_NEW_NODE_IDS_MAP;
// independent DUPLE contains (layer ID, old local ID for node in layer L)
// dependent TRIPLE contains (old global node ID, new local node ID, new global node ID)

   OLD_NEW_NODE_IDS_MAP old_new_node_ids_map;
   OLD_NEW_NODE_IDS_MAP::iterator old_new_node_ids_iter;

   int max_layer_ID = 0;
   for(unsigned int r = 0; r < row_numbers.size(); r++)
   {
      vector<double> curr_row = row_numbers[r];
      DUPLE old_duple(curr_row[0], curr_row[1]);
      triple new_triple(curr_row[2], curr_row[3], curr_row[4]);
      old_new_node_ids_map[old_duple] = new_triple;
      max_layer_ID = basic_math::max(max_layer_ID, int(curr_row[0]));
   }
   cout << "max_layer_ID = " << max_layer_ID << endl;

   typedef std::map<DUPLE, prob_distribution*, ltduple> NODE_ACTIVATIONS_MAP;
// independent DUPLE contains (layer ID, local ID for node in layer L)
// dependent prob_distribution contains pointer to dynamically instantiated
// probability distribution for node's nonzero activations

   NODE_ACTIVATIONS_MAP node_activations_map;
   NODE_ACTIVATIONS_MAP::iterator node_activations_iter;

   vector<string> node_activation_filenames=filefunc::files_in_subdir(
      node_activations_subdir);

   vector<double> n_nonzero_activations;
   for(unsigned int f = 0; f < node_activation_filenames.size(); f++)
   {
      string curr_basename = filefunc::getbasename(
         node_activation_filenames[f]);
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_basename, "_.");
      int layer_ID = stringfunc::string_to_number(substrings[2]);
      int local_node_ID = stringfunc::string_to_number(substrings[3]);
      DUPLE curr_duple(layer_ID, local_node_ID);
      
      vector<double> nonzero_activations = filefunc::ReadInNumbers(
         node_activation_filenames[f]);
      if(nonzero_activations.size() > 0)
      {
         int nbins = 100;
         prob_distribution* curr_prob=new prob_distribution(
            nonzero_activations, nbins, 0);
         node_activations_map[curr_duple] = curr_prob;
      }

   } // loop over index f labeling node activations files

   cout << "node_activations_map.size() = "
        << node_activations_map.size() << endl;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("dat");
   vector<string> image_activation_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes, image_activations_subdir);
   for(unsigned int f = 0; f < image_activation_filenames.size(); f++)
   {
      filefunc::ReadInfile(image_activation_filenames[f]);
      string image_filename = filefunc::text_line[0];
      string image_basename = filefunc::getbasename(image_filename);
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         image_basename, "_");
      string class_label = substrings[0]+"_"+substrings[1];
      
      string output_filename=renorm_image_activations_subdir+
         filefunc::getbasename(image_activation_filenames[f]);
      ofstream outstream;
      filefunc::openfile(output_filename, outstream);
      outstream << image_filename << endl << endl;

      outstream << 
         "# Layer  Old local  Old global   New local  New global  Frac    R    G    B"
                << endl;
      outstream << 
        "#  ID     node ID    node ID       node ID    node ID         "
                << endl << endl;

      for(unsigned int i = 1; i < filefunc::text_line.size(); i++)
      {
         vector<double> curr_vals = stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         int layer_ID = curr_vals[0];
         int old_local_node_ID = curr_vals[1];
         double nonzero_activation = curr_vals[2];
         
         DUPLE old_duple(layer_ID, old_local_node_ID);
         node_activations_iter = node_activations_map.find(old_duple);

         double r, g, b;
         double frac = 0;
         if(node_activations_iter == node_activations_map.end())
         {
/*
            cout << "Error inside image_activations main program!" << endl;
            cout << "Should not have reached this point" << endl;
            exit(-1);
*/
            continue;
         }
         else
         {
/*
            prob_distribution* prob_ptr = node_activations_iter->second;
            int bin = prob_ptr->get_bin_number(nonzero_activation);
            pcum = prob_ptr->get_pcum(bin);
            frac = pcum;
*/

// Map pcum = 1 --> bright red and pcum = 0 --> dark blue:

            extremal_activations_iter = extremal_activations_map.find(
               layer_ID);
            if(extremal_activations_iter == extremal_activations_map.end())
            {
               frac = 1;
            }
            else
            {
               double min_activation = 0;
//               double min_activation = extremal_activations_iter->
//                  second.get(0);
               double max_activation = extremal_activations_iter->
                  second.get(1);
               frac = (nonzero_activation - min_activation) /
                  (max_activation - min_activation);
               
               if(frac < 0 || frac > 1)
               {
                  cout << "layer_ID = " << layer_ID
                       << " nonzero_act = " << nonzero_activation
                       << " min_act = " << min_activation
                       << " max_act = " << max_activation
                       << " frac = " << frac
                       << endl;

                  cout << "layer_ID = " << layer_ID
                       << " old_local_node_ID = " << old_local_node_ID 
                       << endl;
                  cout << "f = " << f 
                       << " of " << image_activation_filenames.size() - 1 
                       << endl;
                  cout << "image_activation_filenames[f] = " 
                       << image_activation_filenames[f] << endl;

                  outputfunc::enter_continue_char();
               }
            }

//            double hue = 250 * (1 - frac);  // unbiased

// As of 9/7/16, we intentionally bias hues towards warmer tones:
            double hue = 250  - 375 * frac;


            if(hue < 0) hue = 0;

            double s = 1;
            double v = 0.4 * (1 + frac);
            colorfunc::hsv_to_RGB(hue, s, v, r, g, b);
         }

         int R = 255 * r;
         int G = 255 * g;
         int B = 255 * b;

         old_new_node_ids_iter = old_new_node_ids_map.find(old_duple);
         triple new_triple = old_new_node_ids_iter->second;
         int old_global_node_ID = new_triple.first;
         int new_local_node_ID = new_triple.second;
         int new_global_node_ID = new_triple.third;

         outstream << layer_ID << "       "
                   << old_local_node_ID << "       "
                   << old_global_node_ID << "       "
                   << new_local_node_ID << "       "
                   << new_global_node_ID << "       "
                   << frac << "      " 
                   << R << "      " 
                   << G << "      " 
                   << B << "      " 
                   << endl;

         const double min_frac_threshold = 0.15;
         const double max_frac_threshold = 0.50;
//         const double min_frac_threshold = 0.50;
//         const double max_frac_threshold = 0.75;
//         const double min_frac_threshold = 0.75;
//         const double max_frac_threshold = 0.85;
//         const double min_frac_threshold = 0.85;
//         const double max_frac_threshold = 0.99;
//         const double min_frac_threshold = 0.90;
//         const double max_frac_threshold = 1.00;
         if(layer_ID == max_layer_ID && frac > min_frac_threshold &&
            frac < max_frac_threshold)
         {
            if(class_label == "male_face")
            {
               cout << filefunc::getbasename(image_activation_filenames[f])
                    << "   frac = " << frac 
                    << "   class = " << class_label
                    << endl;
            }
         }

      } // loop over index i labeling lines within image activations text file

      filefunc::closefile(output_filename, outstream);

   } // loop over index f labeling image activations files

   string banner=
      "Exported renormalized node activations for each test image to "+
      renorm_image_activations_subdir;
   outputfunc::write_banner(banner);
}

   
