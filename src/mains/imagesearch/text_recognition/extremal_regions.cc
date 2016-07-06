// ==========================================================================
// Program EXTREMAL_REGIONS imports the binary and probabilistic
// linear classifier functions exported by program SVM_SHAPE.  It
// first computes connected components within binary thresholded
// versions of an input image for thresholds ranging from 255 down to
// 0.  Following Neumann and Matas, "Real-time scene text localization
// and recognition", CVPR 2012, we compute aspect_ratio, compactness,
// n_holes and median horizontal crossings for each candidate extremal
// region.  These features are mapped into a probability that the
// candidate corresponds to a text character.  

// Entropies for surviving extremal regions are subsequently
// calculated.  If the entropy does not lie within a specified
// interval, the candidate region is discarded.  

// Finally, bounding boxes are placed around surviving candidate text
// character extremal regions.  

//			extremal_regions

// ==========================================================================
// Last updated on 6/22/12; 6/23/12; 6/24/12; 6/25/12
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

   const int K_shapes=4;
   typedef matrix<double, K_shapes, 1> shapes_sample_type;
   shapes_sample_type curr_sample;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the gaussian kernel that
// can operate on our K-dim shapes_sample_type objects

   typedef radial_basis_kernel<shapes_sample_type> kernel_type;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   typedef decision_function<kernel_type> dec_funct_type;
   typedef normalized_function<dec_funct_type> funct_type;
   funct_type learned_funct;

   string learned_funcs_subdir="./learned_functions/";
   string learned_binary_filename=learned_funcs_subdir+
      "shapes_bifunc_1555_3131.dat";
   cout << "shapes_binary_filename = " << learned_binary_filename << endl;
   ifstream fin(learned_binary_filename.c_str(),ios::binary);
   deserialize(learned_funct, fin);

   typedef probabilistic_decision_function<kernel_type> 
      probabilistic_funct_type;  
   typedef normalized_function<probabilistic_funct_type> pfunct_type;
   pfunct_type learned_pfunct;

   string learned_pfunct_filename=learned_funcs_subdir+
      "shapes_pfunct_1555_3131.dat";
   cout << "learned_pfunct_filename = " << learned_binary_filename << endl;
   ifstream fin2(learned_pfunct_filename.c_str(),ios::binary);
   deserialize(learned_pfunct, fin2);

// Import dictionary trainined on 50K text and non-text images:

   const int K_Ng=512;
   typedef matrix<double, K_Ng, 1> linear_sample_type;
   typedef linear_kernel<linear_sample_type> linear_kernel_type;

   typedef decision_function<linear_kernel_type> linear_dec_funct_type;
   typedef normalized_function<linear_dec_funct_type> linear_funct_type;
   linear_funct_type learned_Ng_funct;

   typedef probabilistic_decision_function<linear_kernel_type> 
      linear_probabilistic_funct_type;  
   typedef normalized_function<linear_probabilistic_funct_type> 
      linear_pfunct_type;
   linear_pfunct_type learned_Ng_pfunct;

   string dictionary_subdir="./training_data/dictionary/";
   text_detector* text_detector_ptr=new text_detector(K_Ng,dictionary_subdir);
   text_detector_ptr->import_mean_and_covar_matrices();
   text_detector_ptr->import_dictionary();
//   text_detector_ptr->initialize_window_features_maps();

   string learned_Ng_binary_filename=learned_funcs_subdir+
      "learned_function_50000.dat";
   cout << "learned_Ng_binary_filename = "
        << learned_Ng_binary_filename << endl;
   ifstream fin3(learned_Ng_binary_filename.c_str(),ios::binary);
   deserialize(learned_Ng_funct, fin3);

   string learned_Ng_pfunct_filename=learned_funcs_subdir+
      "learned_pfunct_50000.dat";
   cout << "learned_Ng_pfunct_filename = "
        << learned_Ng_pfunct_filename << endl;
   ifstream fin4(learned_Ng_pfunct_filename.c_str(),ios::binary);
   deserialize(learned_Ng_pfunct, fin4);
 

/*
  string image_subdir=
  "/media/66368D22368CF3F9/text_recognition/training_data/icdar03/words/renorm_words/";
  string image_filename=image_subdir+"00397.jpg"; 
  // light "C.C.T.V." against dark background
  */

   cout << "0: light C against dark background" << endl;
   cout << "1: light FAVOURITE against dark background" << endl;
   cout << "2: light WWW against dark background" << endl;
   cout << "3: light library against dark background" << endl;
   cout << "4: light library against dark background" << endl;
   cout << "5: light open against dark background" << endl;
   cout << "6: light ROGERS against red background" << endl;
   cout << "7: kermit000" << endl;

   int image_number=1;
   cout << "Enter image number:" << endl;
   cin >> image_number;

   string image_subdir;
   if (image_number==0)
   {
      image_subdir=
         "/media/66368D22368CF3F9/text_recognition/training_data/char_intensity/bright_chars_dark_background/";
   }
   else if (image_number >= 1 && image_number <= 5)
   {
      image_subdir=
         "/media/66368D22368CF3F9/text_recognition/training_data/icdar03/words/test/2/";
//    image_subdir="./training_data/icdar03/words/test/2/";
   }
   else if (image_number >=6 && image_number <= 6)
   {
      image_subdir=
         "/media/66368D22368CF3F9/text_recognition/training_data/icdar03/words/test/3/";
   }
   else if (image_number >= 7 && image_number <= 7)
   {
      image_subdir="/data/ImageEngine/kermit/";
   }
   
   string image_filename;
   if (image_number==0)
   {
      image_filename=image_subdir+"00285.jpg"; // light C/dark bkgnd
   }
   else if (image_number==1)
   {
      image_filename=image_subdir+"120.jpg"; // light FAVOURITE/dark bkgnd
   }
   else if (image_number==2)
   {
      image_filename=image_subdir+"122.jpg"; // light www on dark bkgnd
   }
   else if (image_number==3)
   {
      image_filename=image_subdir+"152.jpg"; // light library on dark bkgnd
   }
   else if (image_number==4)
   {
      image_filename=image_subdir+"157.jpg"; // light library on dark bkgnd
   }
   else if (image_number==5)
   {
      image_filename=image_subdir+"158.jpg"; // light open on dark bkgnd
   }
   else if (image_number==6)
   {
      image_filename=image_subdir+"227.jpg"; // 
   }
   else if (image_number==7)
   {
      image_filename=image_subdir+"kermit000.jpg"; // 
   }

//   string image_subdir="./training_data/stores/";
//   string image_filename=image_subdir+
//      "interior-design-for-a-supermarket2.jpg";
//      "Store-Front-5.jpg";
//     "p604192-Winnipeg-The_Garden_Room_store_front.jpg";
//   "reduced_store_front_4.jpg";
   
//   string image_subdir="./training_data/text_database/";
//   string image_filename=image_subdir+"text_img0251.png";

//   string image_subdir="./training_data/street_signs/";
//   string image_filename=image_subdir+"P6180940.JPG";

   int n_entropy_rejections=0;
   double min_entropy=0.55;
   double max_entropy=0.925;

   double min_entropy_in=0.2;
   double max_entropy_in=0.85;

   double min_entropy_out=0.32;
   double max_entropy_out=0.88;

   double min_sigma_all=0;
   double max_sigma_all=105;

   double min_sigma_in=0;
   double max_sigma_in=60;

   double min_sigma_out=0;
   double max_sigma_out=60;

   std::vector<double> entropies;
//   cout << "Enter min entropy:" << endl;
//   cin >> min_entropy;
//   cout << "Enter max entropy:" << endl;
//   cin >> max_entropy;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

// ----------------------------------------------------------------------
// Outermost loop over bright vs dark characters starts here:

   int index=0;

//   for (int brightness_iter=0; brightness_iter < 1; brightness_iter++)
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
         cout << "brightness iter = " << brightness_iter 
              << " threshold = " << threshold << endl;
         int level=255-threshold;

//         bool export_connected_regions_flag=false;
         bool export_connected_regions_flag=true;
         connected_components_ptr->compute_connected_components(
            index,threshold,level,invert_binary_values_flag,
            export_connected_regions_flag);


         graph* graph_ptr=connected_components_ptr->get_graph_ptr(level);
         int n_nodes=graph_ptr->get_n_nodes();
         for (int n=0; n<n_nodes; n++)
         {
            node* node_ptr=graph_ptr->get_ordered_node_ptr(n);
         
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

            const double shapes_prob_threshold=0.75;
            if (char_prob < shapes_prob_threshold)
            {
//            graph_ptr->delete_node(node_ptr);
               node_ptr->set_centrality(-1);
            }
            else
            {

// Record extremal region's character probability within current
// node's centrality field:

               node_ptr->set_centrality(char_prob);
            }

         } // loop over index n labeling current graph nodes
      } // loop over threshold index

// Loop over all graphs within graph hierarchy.  Ignore any extremal
// region whose parent char probability is higher than its own:

      int initial_n_nodes_sum=0;
      int n_deleted_nodes=0;

      int level_stop=255-threshold_stop;
      int level_start=255-threshold_start;

      cout << "level_start = " << level_start
           << " level_stop = " << level_stop << endl;

      bool changed_flag=false;
/*
      do 
      {
         changed_flag=false;
   
         for (int level=level_start; level<level_stop; level++)
         {
            graph* graph_ptr=connected_components_ptr->get_graph_ptr(level);
            graph* parent_graph_ptr=connected_components_ptr->get_graph_ptr(
               level+1);

            int n_nodes=graph_ptr->get_n_nodes();
            initial_n_nodes_sum += n_nodes;
//      cout << "level = " << level << " n_nodes = " << n_nodes << endl;
            for (int n=0; n<n_nodes; n++)
            {
               node* node_ptr=graph_ptr->get_ordered_node_ptr(n);
//         cout << "node_ptr = " << node_ptr << endl;
               if (node_ptr==NULL) continue;
               double char_prob=node_ptr->get_centrality();
               if (char_prob < 0) continue;
         
               int parent_ID=node_ptr->get_parent_ID();
//         cout << "parent_ID = " << parent_ID << endl;
               node* parent_node_ptr=parent_graph_ptr->get_node_ptr(parent_ID);
               if (parent_node_ptr==NULL) continue;
               double parent_char_prob=parent_node_ptr->get_centrality();
               if (parent_char_prob < 0) continue;

               if (parent_char_prob > char_prob)
               {
//            graph_ptr->delete_node(node_ptr);
                  node_ptr->set_centrality(-1);
                  n_deleted_nodes++;
               }
               else
               {
//            parent_graph_ptr->delete_node(parent_node_ptr);
                  parent_node_ptr->set_centrality(-1);
                  n_deleted_nodes++;
               }
               changed_flag=true;

            } // loop over index n labeling nodes within *graph_ptr
         } // loop over level index
      }
      while (changed_flag);
*/

      int final_n_nodes_sum=initial_n_nodes_sum-n_deleted_nodes;

      cout << "Initial n_nodes sum = " << initial_n_nodes_sum << endl;
      cout << "Final n_nodes sum = " << final_n_nodes_sum << endl;

// Instantiate and fill STL map with bounding box corner coordinates:

      typedef std::map<fourvector,node*,ltfourvector > NODES_MAP;
      NODES_MAP* nodes_map_ptr=new NODES_MAP;
      // *nodes_map_ptr independent var = left bottom & top right bbox corners
      // dependent var = node ptr

      for (int level=level_start; level<level_stop; level++)
      {
         graph* graph_ptr=connected_components_ptr->get_graph_ptr(level);
         int n_nodes=graph_ptr->get_n_nodes();
//      cout << "level = " << level << " n_nodes = " << n_nodes << endl;
         for (int n=0; n<n_nodes; n++)
         {
            node* node_ptr=graph_ptr->get_ordered_node_ptr(n);
            if (node_ptr==NULL) continue;
            double char_prob=node_ptr->get_centrality();
//         cout << "n = " << n << " char_prob = " << char_prob << endl;
            if (char_prob < 0) continue;

            int left_bottom_pu=node_ptr->get_Uposn();
            int left_bottom_pv=node_ptr->get_Vposn();
            int right_top_pu=node_ptr->get_U2posn()+1;
            int right_top_pv=node_ptr->get_V2posn()+1;

// Reject any bounding box whose pixel width or height is less than
// some reasonable threshold values:

            const int min_pixel_width=2;
            const int min_pixel_height=4;

            int pixel_width=right_top_pu-left_bottom_pu;
            int pixel_height=right_top_pv-left_bottom_pv;
            cout << "w = " << pixel_width << " h = " << pixel_height << endl;

            if (pixel_width < min_pixel_width) continue;
            if (pixel_height < min_pixel_height) continue;

            fourvector bbox_corners(
               left_bottom_pu,left_bottom_pv,right_top_pu,right_top_pv);
            NODES_MAP::iterator iter=nodes_map_ptr->find(bbox_corners);
            if (iter==nodes_map_ptr->end())
            {
               (*nodes_map_ptr)[bbox_corners]=node_ptr;

               cout << "level = " << level
                    << " char prob = " << basic_math::round(100*char_prob)
                    << " bottom left: pu = " << left_bottom_pu
                    << " pv = " << left_bottom_pv
                    << " top right: pu = " << right_top_pu
                    << " pv = " << right_top_pv << endl;
            }
         } // loop over index n labeling nodes for current graph
      } // loop over level index
      
      delete connected_components_ptr;   
      cout << "Initially, nodes_map_ptr->size() = " << nodes_map_ptr->size()
           << endl;

// Merge bounding boxes whose sizes are nearly equal:
   
//   const int bbox_distance_toler=1;	// pixels
      const int bbox_distance_toler=4;	// pixels

      do 
      {
         changed_flag=false;

         for (NODES_MAP::iterator iter=nodes_map_ptr->begin();
              iter != nodes_map_ptr->end(); iter++)
         {
            fourvector curr_bbox_corners=iter->first;
            if (iter->second==NULL) continue;

            double left_bottom_pu=curr_bbox_corners.get(0);
            double left_bottom_pv=curr_bbox_corners.get(1);
            double right_top_pu=curr_bbox_corners.get(2);
            double right_top_pv=curr_bbox_corners.get(3);
      
            for (NODES_MAP::iterator iter2=nodes_map_ptr->begin();
                 iter2 != nodes_map_ptr->end(); iter2++)
            {
               if (iter2==iter) continue;
               if (iter2->second==NULL) continue;
            
               fourvector next_bbox_corners=iter2->first;
               if ( (curr_bbox_corners-next_bbox_corners).magnitude() > 
               bbox_distance_toler) continue;

               left_bottom_pu=basic_math::min(
                  left_bottom_pu,next_bbox_corners.get(0));
               left_bottom_pv=basic_math::min(
                  left_bottom_pv,next_bbox_corners.get(1));
               right_top_pu=basic_math::max(
                  right_top_pu,next_bbox_corners.get(2));
               right_top_pv=basic_math::max(
                  right_top_pv,next_bbox_corners.get(3));
               fourvector new_bbox_corners
                  (left_bottom_pu,left_bottom_pv,right_top_pu,right_top_pv);
               (*nodes_map_ptr)[new_bbox_corners]=iter->second;
               iter2->second=NULL;
               changed_flag=true;
            } // loop over iter2
         } // loop over iter
      }
      while (changed_flag);

// Compute image entropies within candidate text character bounding
// boxes:

      texture_rectangle_ptr->import_photo_from_file(image_filename);
      texture_rectangle_ptr->convert_color_image_to_greyscale();

      std::vector<double> gradient_mag_avgs,entropies;
      for (NODES_MAP::iterator iter=nodes_map_ptr->begin();
           iter != nodes_map_ptr->end(); iter++)
      {
         node* node_ptr=iter->second;
         if (node_ptr==NULL) continue;
         fourvector bbox_corners=iter->first;

         int left_top_pu=bbox_corners.get(0);
         int left_top_pv=bbox_corners.get(1);
         int right_bottom_pu=bbox_corners.get(2);
         int right_bottom_pv=bbox_corners.get(3);

         bool filter_intensities_flag=true;
         bool greyscale_flag=true;
         double entropy=texture_rectangle_ptr->compute_image_entropy(
            left_top_pu,right_bottom_pu+1,left_top_pv,right_bottom_pv+1,
            filter_intensities_flag,greyscale_flag);
         entropies.push_back(entropy);
         node_ptr->set_relative_size(entropy);
         cout << "in main, entropy = " << entropy << endl;
         cout << "entropies.size() = " << entropies.size() << endl;
      } // loop over *nodes_map_ptr

      int bbox_counter=0;
      for (NODES_MAP::iterator iter=nodes_map_ptr->begin();
           iter != nodes_map_ptr->end(); iter++)
      {
         node* node_ptr=iter->second;
         if (node_ptr==NULL) continue;

         double curr_entropy=node_ptr->get_relative_size();
         if (curr_entropy < min_entropy || curr_entropy > max_entropy)
         {
            n_entropy_rejections++;
            continue;
         }

         fourvector bbox_corners=iter->first;
         int left_top_pu=bbox_corners.get(0);
         int left_top_pv=bbox_corners.get(1);
         int right_bottom_pu=bbox_corners.get(2);
         int right_bottom_pv=bbox_corners.get(3);

         texture_rectangle_ptr->import_photo_from_file(image_filename);

         double left_top_u,left_top_v,right_bottom_u,right_bottom_v;
         texture_rectangle_ptr->get_uv_coords(
            left_top_pu,left_top_pv,left_top_u,left_top_v);
         texture_rectangle_ptr->get_uv_coords(
            right_bottom_pu,right_bottom_pv,right_bottom_u,right_bottom_v);

         std::vector<threevector> vertices;
         vertices.push_back(threevector(left_top_u,left_top_v));
         vertices.push_back(threevector(left_top_u,right_bottom_v));
         vertices.push_back(threevector(right_bottom_u,right_bottom_v));
         vertices.push_back(threevector(right_bottom_u,left_top_v));
         polygon bbox(vertices);
         std::vector<polygon> polygons;
         polygons.push_back(bbox);

         string bboxes_subdir="./bboxes/";
         filefunc::dircreate(bboxes_subdir);
         string output_filename=bboxes_subdir+"bbox_bright_chars";
         if (brightness_iter==1) output_filename=bboxes_subdir+
                                    "bbox_dark_chars";
      
         output_filename += stringfunc::integer_to_string(bbox_counter++,3)+
            ".jpg";
         int contour_color_index=1;
         int line_thickness=2;
         videofunc::display_polygons(
            polygons,texture_rectangle_ptr,output_filename,
            contour_color_index,line_thickness);

//      cout << "char bbox counter = " << bbox_counter-1
//           << " bottom right: U = " << right_bottom_u
//           << " V = " << right_bottom_v
//           << " top left: U = " << left_top_u
//           << " V = " << left_top_v
//           << endl;
      } // loop over *nodes_map_ptr
      delete nodes_map_ptr;


// As of 6/22/2012, we are unable to move the next few lines which
// compute and display the entropy distribution outside the
// brightness_iter for loop.  We do not know why entropies.size()==0
// immediately after the end of this for loop...

      cout << "entropies.size() = " << entropies.size() << endl;
      if (brightness_iter==1)
      {
         prob_distribution prob_entropy(entropies,100);
         prob_entropy.writeprobdists(false);
      }
   } // loop over brightness_iter
// ----------------------------------------------------------------------

   cout << "n_entropy_rejections = " << n_entropy_rejections << endl;

   delete texture_rectangle_ptr;
}

