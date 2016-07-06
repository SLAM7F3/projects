// ==========================================================================
// Program REGION_STATS


//				region_stats

// ==========================================================================
// Last updated on 6/22/12; 6/23/12; 6/24/12
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>
#include "dlib/svm.h"

#include "image/binaryimagefuncs.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

#include "math/fourvector.h"
#include "graphs/graph.h"
#include "graphs/graph_hierarchy.h"
#include "datastructures/Linkedlist.h"
#include "math/ltfourvector.h"
#include "graphs/node.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::string;
using std::vector;

using namespace dlib;

int main(int argc, char* argv[])
{
   cout.precision(12);

// The svm functions use column vectors to contain a lot of the
// data on which they operate. So the first thing we do here is
// declare a convenient typedef.

// This typedef declares a matrix with K rows and 1 column.  It will
// be the object that contains each of our K dimensional samples. 

   const int K=4;
   typedef matrix<double, K, 1> sample_type;
   sample_type curr_sample;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the linear kernel that
// can operate on our K-dim sample_type objects

   typedef radial_basis_kernel<sample_type> kernel_type;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   typedef decision_function<kernel_type> dec_funct_type;
   typedef normalized_function<dec_funct_type> funct_type;
   funct_type learned_funct;

   string learned_funcs_subdir="./learned_functions/";
   string learned_binary_filename=learned_funcs_subdir+
      "learned_bifunc_1555_3131.dat";
   cout << "learned_binary_filename = " << learned_binary_filename << endl;
   ifstream fin(learned_binary_filename.c_str(),ios::binary);
   deserialize(learned_funct, fin);

   typedef probabilistic_decision_function<kernel_type> 
      probabilistic_funct_type;  
   typedef normalized_function<probabilistic_funct_type> pfunct_type;
   pfunct_type learned_pfunct;

   string learned_pfunct_filename=learned_funcs_subdir+
      "learned_pfunct_1555_3131.dat";
   cout << "learned_pfunct_filename = " << learned_binary_filename << endl;
   ifstream fin2(learned_pfunct_filename.c_str(),ios::binary);
   deserialize(learned_pfunct, fin2);

// ----------------------------------------------------------------------

   int imagenumber_start=30000;
   int imagenumber_stop=50000;
   int n_images=imagenumber_stop-imagenumber_start;

//   bool text_flag=true;
   bool text_flag=false;
   
   string output_filename="text_randomness.dat";
   if (!text_flag) output_filename="nontext_randomness.dat";
   
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   if (text_flag)
   {
      outstream << "# Features derived from " << n_images 
                << " text training examples" << endl;
   }
   else
   {
      outstream << "# Features derived from " << n_images 
                << " non-text training examples" << endl;
   }
   outstream << endl;
   
   outstream << "# image   sigma_all sigma_in  sigma_out  S_all   S_in     S_out" << endl;
   outstream << endl;

   string image_subdir="./training_data/renorm_chars/";
   if (!text_flag)
   {
      image_subdir="./training_data/nontext_images/";
   }


   for (int imagenumber=imagenumber_start; imagenumber<imagenumber_stop;
        imagenumber++)
   {
      string image_filename;
      if (text_flag)
      {
         image_filename=image_subdir+
            stringfunc::integer_to_string(imagenumber,5)+".jpg";
      }
      else
      {
         image_filename=image_subdir+"nontext_"+
            stringfunc::integer_to_string(imagenumber,6)+".jpg";
      }

      int index=0;
      int counter=0;
      double sigma_all=0;
      double sigma_interior=0;
      double sigma_exterior=0;
      double entropy_all=0;
      double entropy_interior=0;
      double entropy_exterior=0;

      for (int brightness_iter=0; brightness_iter < 2; brightness_iter++)
      {
         bool invert_binary_values_flag=false; // bright chars dark background
         if (brightness_iter==1)
         {
            invert_binary_values_flag=true; // dark chars bright background
         }

         connected_components* connected_components_ptr=
            new connected_components();
         connected_components_ptr->reset_image(image_filename);

         int threshold_start=254;
         int threshold_stop=1;
         int d_threshold=-1;
         for (int threshold=threshold_start; threshold >= threshold_stop; 
              threshold += d_threshold)
         {
//         cout << "brightness iter = " << brightness_iter 
//              << " threshold = " << threshold << endl;

            int level=255-threshold;
//         bool export_connected_regions_flag=false;
            bool export_connected_regions_flag=true;
            connected_components_ptr->compute_connected_components(
               index,threshold,level,invert_binary_values_flag,
               export_connected_regions_flag);

            graph* graph_ptr=connected_components_ptr->get_graph_ptr(level);
            int n_nodes=graph_ptr->get_n_nodes();

            if (n_nodes != 1) continue;

            node* node_ptr=graph_ptr->get_ordered_node_ptr(0);
         
            int ID=node_ptr->get_ID();
            int pixel_area=node_ptr->get_data_ID();
            int pixel_perimeter=node_ptr->get_source_ID();
            int euler_number=node_ptr->get_representative_child_ID();
            int median_horiz_crossings=node_ptr->get_color_ID();
            double left_bottom_pu=node_ptr->get_Uposn();
            double left_bottom_pv=node_ptr->get_Vposn();
            double right_top_pu=node_ptr->get_U2posn()+1;
            double right_top_pv=node_ptr->get_V2posn()+1;
//         cout << "node: ID = " << ID
//              << " area = " << pixel_area
//              << " perim = " << pixel_perimeter
//              << " euler = " << euler_number
//              << " horiz_crossings = " << median_horiz_crossings << endl;

//         cout << "left_bottom: pu = " << left_bottom_pu
//              << " pv = " << left_bottom_pv << endl;
//         cout << "right_top: pu = " << right_top_pu
//              << " pv = " << right_top_pv << endl;
         
            double aspect_ratio=(right_top_pu-left_bottom_pu)/(
               right_top_pv-left_bottom_pv);
            double compactness=sqrt(pixel_area)/double(pixel_perimeter);
            int n_holes=1-euler_number;

//         cout << "  n = " << n << " node_ID = " << ID << endl;
//         cout << "  aspect_ratio = " << aspect_ratio << endl;
//         cout << "  compactness = " << compactness << endl;
//         cout << "  n_holes = " << n_holes << endl;
//         cout << "  median_horiz_crossings = " << median_horiz_crossings
//              << endl;

            curr_sample(0)=aspect_ratio;
            curr_sample(1)=compactness;
            curr_sample(2)=n_holes;
            curr_sample(3)=median_horiz_crossings;

            double char_class=learned_funct(curr_sample);
            double char_prob=learned_pfunct(curr_sample);
//         cout << "      char_class = " << char_class
//              << " char_prob = " << char_prob << endl << endl;

            if (char_prob < 0.75) continue;

            double curr_sigma_all,curr_sigma_interior,curr_sigma_exterior;
            if (!connected_components_ptr->
            compute_interior_exterior_std_devs(
               curr_sigma_all,curr_sigma_interior,curr_sigma_exterior)) 
               continue;

            double curr_entropy_all,curr_entropy_interior,
               curr_entropy_exterior;
            if (!connected_components_ptr->
            compute_interior_exterior_entropies(
               curr_entropy_all,curr_entropy_interior,curr_entropy_exterior)) 
               continue;

            sigma_all += curr_sigma_all;
            sigma_interior += curr_sigma_interior;
            sigma_exterior += curr_sigma_exterior;
            entropy_all += curr_entropy_all;
            entropy_interior += curr_entropy_interior;
            entropy_exterior += curr_entropy_exterior;
            counter++;

//            cout.precision(5);
//            cout << "image_filename = " << image_filename << endl;
//            cout << "   brightness iter = " << brightness_iter 
//                 << " threshold = " << threshold 
//                 << " char_prob = " << char_prob 
//                 << " r_in = " << ratio_interior
//                 << " r_out = " << ratio_exterior 
//                 << " S_in = " << curr_entropy_interior
//                 << " S_out = " << curr_entropy_exterior 
//                 << endl;
//            cout << endl;

         } // loop over threshold index

         delete connected_components_ptr;

      } // loop over brightness_iter

      const int min_counter=2;
      if (counter < min_counter) continue;

      sigma_all /= counter;
      sigma_interior /= counter;
      sigma_exterior /= counter;
      entropy_all /= counter;
      entropy_interior /= counter;
      entropy_exterior /= counter;

      cout << "image_filename = " << image_filename << endl;
      cout << "sigma_in = " << sigma_interior
           << " sigma_out = " << sigma_exterior
           << " S_in = " << entropy_interior
           << " S_out = " << entropy_exterior << endl;
      cout << endl;

      outstream << filefunc::getbasename(image_filename) << "  "
                << sigma_all << "  "
                << sigma_interior << "  "
                << sigma_exterior << "  "
                << entropy_all << "  "
                << entropy_interior << "  "
                << entropy_exterior 
                << endl;

   } // loop over imagenumber index

   filefunc::closefile(output_filename,outstream);

   string banner="Standard deviation and entropy features exported to "+
      output_filename;
   outputfunc::write_big_banner(banner);
}

