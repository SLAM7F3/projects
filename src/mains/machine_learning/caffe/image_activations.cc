// ========================================================================
// Program IMAGE_ACTIVATIONS 

//                         ./image_activations

// ========================================================================
// Last updated on 9/5/16; 9/7/16
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
   string imagechips_subdir = "./vis_facenet/node_images/";
   string activations_subdir = imagechips_subdir + "activations/";
   string image_activations_subdir = activations_subdir + "images/";
   string renorm_image_activations_subdir = image_activations_subdir 
      + "renormalized/";
   filefunc::dircreate(renorm_image_activations_subdir);
   string node_activations_subdir = activations_subdir + "nodes/";

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
         "# Layer  Old local  Old global   New local  New global  Pcum    R    G    B"
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
         double pcum = 0;
         if(node_activations_iter == node_activations_map.end())
         {
/*
            cout << "Error inside image_activations main program!" << endl;
            cout << "Should not have reached this point" << endl;
            cout << "layer_ID = " << layer_ID
                 << " old_local_node_ID = " << old_local_node_ID 
                 << endl;
            cout << "f = " << f 
                 << " of " << image_activation_filenames.size() - 1 << endl;
            cout << "image_activation_filenames[f] = " 
                 << image_activation_filenames[f] << endl;
            exit(-1);
*/
           //            r = g = b = 0.2;

            continue;
         }
         else
         {
            prob_distribution* prob_ptr = node_activations_iter->second;

            int bin = prob_ptr->get_bin_number(nonzero_activation);
            pcum = prob_ptr->get_pcum(bin);

// Map pcum = 1 --> bright red and pcum = 0 --> dark blue:

            double hue = 250 * ( 1 - pcum);
            double s = 1;
            double v = 0.4 * (1 + pcum);
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
                   << pcum << "      " 
                   << R << "      " 
                   << G << "      " 
                   << B << "      " 
                   << endl;

         const double min_pcum_threshold = 0.60;
         const double max_pcum_threshold = 0.75;
//         const double min_pcum_threshold = 0.75;
//         const double max_pcum_threshold = 0.85;
//         const double min_pcum_threshold = 0.85;
//         const double max_pcum_threshold = 0.99;
//         const double min_pcum_threshold = 0.99;
//         const double max_pcum_threshold = 1.00;
         if(layer_ID == max_layer_ID && pcum > min_pcum_threshold &&
            pcum < max_pcum_threshold)
         {
            cout << filefunc::getbasename(image_activation_filenames[f])
                 << "   pcum = " << pcum 
                 << "   class = " << class_label
                 << endl;
         }

      } // loop over index i labeling lines within image activations text file

      filefunc::closefile(output_filename, outstream);

   } // loop over index f labeling image activations files

   string banner=
      "Exported renormalized node activations for each test image to "+
      renorm_image_activations_subdir;
   outputfunc::write_banner(banner);
}

   
