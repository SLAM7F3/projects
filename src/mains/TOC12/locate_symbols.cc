// ==========================================================================
// Program LOCATE_SYMBOLS is a variant of LOCATE_CHARS intended for
// TOC12 sign detection.

//				locate_symbols

// ==========================================================================
// Last updated on 9/6/12; 9/7/12; 9/13/12; 6/7/14
// ==========================================================================

#include  <fstream>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>
#include "dlib/svm.h"

#include "image/binaryimagefuncs.h"
#include "geometry/bounding_box.h"
#include "image/compositefuncs.h"
#include "video/connected_components.h"
#include "image/connectfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltquadruple.h"
#include "numrec/nrfuncs.h"
#include "datastructures/Quadruple.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

#include "color/colorfuncs.h"
#include "distance_transform/dtfuncs.h"
#include "geometry/plane.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::string;
using std::vector;

using namespace dlib;

int main(int argc, char* argv[])
{
   cout.precision(12);

   double min_RGB_diff=33;
   cout << "Enter min RGB difference:" << endl;
   cin >> min_RGB_diff;

//   const int K_Ng=512;
   const int K_Ng=1024;

   bool RGB_pixels_flag=false;
//   bool RGB_pixels_flag=true;
   cout << "RGB_pixels_flag = " << RGB_pixels_flag << endl;

//   int D=64*3;	// RGB color
//   if (!RGB_pixels_flag)
//   {
//      D=64;	// greyscale
//   }

// Hard-wire 9 TOC12 ppt symbol names:

   std::vector<string> symbol_names;
   symbol_names.push_back("biohazard");		// cyan
   symbol_names.push_back("eat");		// green
   symbol_names.push_back("gas");		// orange
   symbol_names.push_back("green_start");	// purple
   symbol_names.push_back("pink_radiation");    // white
   symbol_names.push_back("red_stop");		// blue
   symbol_names.push_back("skull");		// red
   symbol_names.push_back("water");		// yellow
   symbol_names.push_back("yellow_radiation");	// cream

   std::vector<string> symbol_color_strings;
   symbol_color_strings.push_back("cyan");
   symbol_color_strings.push_back("green");
   symbol_color_strings.push_back("orange");
   symbol_color_strings.push_back("purple");
   symbol_color_strings.push_back("white");
   symbol_color_strings.push_back("blue");
   symbol_color_strings.push_back("red");
   symbol_color_strings.push_back("yellow");
   symbol_color_strings.push_back("cream");
   
   std::vector<int> contour_color_indices;
   for (unsigned int s=0; s<symbol_color_strings.size(); s++)
   {
      contour_color_indices.push_back(colorfunc::get_color_index(
         symbol_color_strings[s]));
   }

// Form STL map which associates TOC12 ppt symbol names with their
// integer IDs:

   typedef std::map<string,int> SYMBOL_ID_MAP;
   SYMBOL_ID_MAP symbol_id_map;
   SYMBOL_ID_MAP::iterator symbol_id_iter;

   for (unsigned int s=0; s<symbol_names.size(); s++)
   {
      symbol_id_map[symbol_names[s]]=s;
   }

/*
// Query user to enter one particular TOC12 ppt symbol name:

   string symbol_name;
   cout << "Enter symbol name:" << endl;
   cout << "  biohazard,eat,gas,green_start,pink_radiation" << endl;
   cout << "  red_stop,skull,water,yellow_radiation" << endl;
   cin >> symbol_name;

   int selected_symbol_ID=-1;
   symbol_id_iter=symbol_id_map.find(symbol_name);
   if (symbol_id_iter==symbol_id_map.end())
   {
      cout << "No such TOC12 symbol name exists!" << endl;
      exit(-1);
   }
   else
   {
      selected_symbol_ID=symbol_id_iter->second;
   }
   cout << "Selected symbol_ID = " << selected_symbol_ID << endl;
*/

   string modified_images_subdir="./images/final_signs/modified_image_files/";
   string red_channel_subdir="./images/final_signs/red_channel/";
   string input_images_subdir=modified_images_subdir;
   if (!RGB_pixels_flag)
   {
      input_images_subdir=red_channel_subdir;
   }
//   cout << "input images subdir = " << input_images_subdir << endl;

   string symbol_name="";
   string synthetic_subdir=input_images_subdir+"synthetic_symbols/";
   string synthetic_symbols_subdir=synthetic_subdir+symbol_name+"/";
   string dictionary_subdir=synthetic_symbols_subdir;

// The svm functions use column vectors to contain a lot of the
// data on which they operate. So the first thing we do here is
// declare a convenient typedef.

// This typedef declares a matrix with K rows and 1 column.  It will
// be the object that contains each of our K dimensional samples. 

   const int K_shapes=11;
   typedef matrix<double, K_shapes, 1> shapes_sample_type;
   shapes_sample_type shapes_sample;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the gaussian kernel that
// can operate on our K-dim shapes_sample_type objects

   typedef radial_basis_kernel<shapes_sample_type> shapes_kernel_type;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   typedef probabilistic_decision_function<shapes_kernel_type> 
      shapes_probabilistic_funct_type;  
   typedef normalized_function<shapes_probabilistic_funct_type> 
      shapes_pfunct_type;
   shapes_pfunct_type shapes_pfunct;
   std::vector<shapes_pfunct_type*> shapes_pfuncts_ptrs;

// Import probabilistic classification function trained via SVM_SHAPE_ALL
// for all 9 TOC12 ppt symbol letters and brightness settings:

   string alphabet_subdir="./images/ppt_signs/alphabet/";
   string pfunction_filename=alphabet_subdir+"symbols_all_pfunct.dat";
   std::vector<string> pfunction_filenames;
   pfunction_filenames.push_back(pfunction_filename);

   shapes_pfunct_type* shapes_pfunct_ptr=new shapes_pfunct_type;
   ifstream fin2(pfunction_filename.c_str(),ios::binary);
   deserialize(*shapes_pfunct_ptr,fin2);
   shapes_pfuncts_ptrs.push_back(shapes_pfunct_ptr);



// Import probabilistic decision function generated by an SVM with a
// linear kernel on 10K symbol and non-symbol images:

   const int nineK=9*K_Ng;
   typedef matrix<double, nineK, 1> Ng_sample_type;
   typedef linear_kernel<Ng_sample_type> Ng_kernel_type;
   Ng_sample_type Ng_sample;

   typedef probabilistic_decision_function<Ng_kernel_type> 
      Ng_probabilistic_funct_type;  
   typedef normalized_function<Ng_probabilistic_funct_type> 
      Ng_pfunct_type;
   Ng_pfunct_type Ng_pfunct;

   string learned_funcs_subdir="./learned_functions/";
   string learned_Ng_pfunct_filename=learned_funcs_subdir;
   
   if (symbol_name=="radiation1")
   {
      learned_Ng_pfunct_filename += 
         "radiation1/symbols_Ng_pfunct_10000_82086.dat";
   }
   else if (symbol_name=="radiation2")
   {
      learned_Ng_pfunct_filename += 
         "radiation2/symbols_Ng_pfunct_10000_97748.dat";
   }
//   else if (symbol_name=="biohazard")
//   {
//      learned_Ng_pfunct_filename += 
//         "biohazard/symbols_Ng_pfunct_10000_97748.dat";
//   }
   else
   {
      learned_Ng_pfunct_filename="";
   }
   

   if (learned_Ng_pfunct_filename.size() > 0)
   {
      cout << "learned_Ng_pfunct_filename = "
           << learned_Ng_pfunct_filename << endl;
      ifstream fin6(learned_Ng_pfunct_filename.c_str(),ios::binary);
      deserialize(Ng_pfunct, fin6);
   }
   
//   double Ng_char_threshold=0.5;
   double Ng_char_threshold=0.9;
//   cout << "Enter Ng char threshold:" << endl;
//   cin >> Ng_char_threshold;

// Import dictionary trained on 10K symbol and non-symbol images:

   text_detector* text_detector_ptr=new text_detector(dictionary_subdir, RGB_pixels_flag);

   if (learned_Ng_pfunct_filename.size() > 0)
   {
      text_detector_ptr->import_inverse_sqrt_covar_matrix();
   }
   
   texture_rectangle* texture_rectangle_ptr=text_detector_ptr->
      get_texture_rectangle_ptr();

// ----------------------------------------------------------------------

   std::vector<polygon> particular_symbol_polygons;
   std::vector<std::vector<polygon> > all_symbol_polygons;

//   int color_channel_ID=-1;	
   int color_channel_ID=-2;	// TOC12 ppt symbols
//   cout << "Enter color channel ID:" << endl;
//   cout << "-2 = luminosity, -1 = value, 0 = saturation, 1 = red, 2 = green, 3 = blue" << endl;
//   cin >> color_channel_ID;
   
   timefunc::initialize_timeofday_clock();

   int index=0;
   int n_detections=0;
   int n_rejections=0;
   int n_Ng_rejections=0;
   int n_Ng_acceptances=0;

   connected_components* connected_components_ptr=NULL;
   connectfunc::create_extremal_region_pooled_memory();

   typedef Quadruple<int,int,int,int> BBOX_PIXEL_COORDS;
   typedef std::map<BBOX_PIXEL_COORDS,extremal_region*,ltquadruple > 
      BBOXES_MAP;
   BBOXES_MAP* bboxes_map_ptr=NULL;

   double shapes_prob_threshold=0.75;
//   double shapes_prob_threshold=0.9;
//   cout << "Enter shapes probability threshold:" << endl;
//   cin >> shapes_prob_threshold;

   int video_pass_number=1;
   cout << "Enter video pass number:" << endl;
   cin >> video_pass_number;
   string video_pass_str=stringfunc::integer_to_string(video_pass_number,2);

   string bboxes_subdir="./bboxes_"+video_pass_str+"/";
   filefunc::dircreate(bboxes_subdir);

// ***********************************************************************
// Outermost loop over all input images starts here:

   string image_subdir="./images/ppt_signs/videos/";
   image_subdir += "vid"+video_pass_str+"_frames/";
   cout << "image_subdir = " << image_subdir << endl;

   std::vector<string> image_filenames=filefunc::image_files_in_subdir(
      image_subdir);
   int n_images=image_filenames.size();

   int i_start=0;
   int i_stop=n_images;
   cout << "Enter starting image number to process:" << endl;
   cin >> i_start;

   cout << "i_start = " << i_start << endl;
//   i_stop=i_start+1;

   for (int image_counter=i_start; image_counter < i_stop; image_counter++)
   {
      string image_filename=image_filenames[image_counter];
      cout << "Processing image "+stringfunc::number_to_string(image_counter)+
         " of "+stringfunc::number_to_string(n_images) << endl;

      delete texture_rectangle_ptr;
      texture_rectangle_ptr=new texture_rectangle();
      text_detector_ptr->set_texture_rectangle_ptr(texture_rectangle_ptr);

// =======================================================================
// Loop over bright vs dark characters starts here:

//   for (int brightness_iter=0; brightness_iter < 1; brightness_iter++)
//   for (int brightness_iter=1; brightness_iter < 2; brightness_iter++)
      for (int brightness_iter=0; brightness_iter < 2; brightness_iter++)
      {
         bool invert_binary_values_flag;
         int threshold_start,threshold_stop,d_threshold;

         if (brightness_iter==0)	// bright chars dark background
         {
            invert_binary_values_flag=false;
            threshold_start=254;
            threshold_stop=1;
            d_threshold=-1;
         }
         else if (brightness_iter==1) // dark chars bright background
         {
            invert_binary_values_flag=true; 
            threshold_start=1;
            threshold_stop=254;
            d_threshold=1;
         }

         delete connected_components_ptr;   
         connected_components_ptr=new connected_components();
         connected_components_ptr->set_shapes_pfuncts_ptrs(
            &shapes_pfuncts_ptrs);
         connected_components_ptr->reset_image(
            image_filename,color_channel_ID,image_counter);
         connected_components::TREE_PTR tree_ptr=connected_components_ptr->
            get_tree_ptr();

// ----------------------------------------------------------------------
         for (int threshold=threshold_start; threshold != threshold_stop; 
              threshold += d_threshold)
         {
            int level=-1;
            if (!invert_binary_values_flag)
            {
               level=255-threshold;
            }
            else
            {
               level=threshold;
            }

            string region_type;
            if (brightness_iter==0)
            {
               region_type="bright_region";
            }
            else
            {
               region_type="dark_region";
            }
            int n_treenodes=0;
            int n_leafnodes=0;
            if (tree_ptr != NULL) n_treenodes=tree_ptr->size();
            if (tree_ptr != NULL) n_leafnodes=tree_ptr->get_n_leaf_nodes();

            if (level%10==0)
            {
               cout << region_type 
                    << "  level=" << level 
                    << "  n_nodes=" << n_treenodes
                    << "  n_leafnodes=" << n_leafnodes
                    << endl;
            }

            bool RLE_flag=true;
//            bool RLE_flag=false;
            bool export_connected_regions_flag=false;
//         bool export_connected_regions_flag=true;
            connected_components_ptr->compute_connected_components(
               index,threshold,level,RLE_flag,invert_binary_values_flag,
               export_connected_regions_flag);

// Compute extremal region shape probabilities for all 9 TOC12 ppt
// symbols:

            bool tight_skew_quartic_thresholds_flag=false;
            connected_components_ptr->compute_text_shape_probs(
               level,shapes_prob_threshold,0,1,
               tight_skew_quartic_thresholds_flag);
            
         } // loop over threshold index
// ----------------------------------------------------------------------

         double elapsed_time=timefunc::elapsed_timeofday_time();
         cout << "ELAPSED TIME = " << elapsed_time << endl;

// ......................................................................
         for (int symbol_ID=0; symbol_ID<1; symbol_ID++)
         {
            texture_rectangle_ptr->import_photo_from_file(image_filename);
            twoDarray* cc_twoDarray_ptr=new twoDarray(
               texture_rectangle_ptr->getWidth(),
               texture_rectangle_ptr->getHeight());

            std::vector<connected_components::TREENODE_PTR> 
               stable_region_node_ptrs=
               connected_components_ptr->identify_stable_extremal_regions(
                  symbol_ID);

            cout << "total n_nodes = " << tree_ptr->size() << endl;
            cout << "n_leaf nodes = " << tree_ptr->get_n_leaf_nodes() << endl;
            cout << "stable_region_node_ptrs.size() = "
                 << stable_region_node_ptrs.size() << endl;

// Instantiate and fill STL map with bounding box corner coordinates.
// Ignore any bbox candidate whose pixel width or pixel height are too
// small:

            delete bboxes_map_ptr;
            BBOXES_MAP* bboxes_map_ptr=new BBOXES_MAP;
            // *bboxes_map_ptr independent var = left bottom & top right 
//	     bbox corners
            // dependent var = extremal region ptr

            const unsigned int min_pixel_width=5;
            const unsigned int min_pixel_height=5;
            for (unsigned int n=0; n<stable_region_node_ptrs.size(); n++)
            {
               connected_components::TREENODE_PTR treenode_ptr=
                  stable_region_node_ptrs[n];
               extremal_region* extremal_region_ptr=
                  treenode_ptr->get_data_ptr();

               unsigned int left_pu,bottom_pv,right_pu,top_pv;
               extremal_region_ptr->get_bbox(
                  left_pu,bottom_pv,right_pu,top_pv);

//               cout << "right_pu-left_pu = " << right_pu-left_pu
//                    << " bottom_pv-top_pv = " << bottom_pv-top_pv
//                    << endl;

               if (right_pu-left_pu < min_pixel_width ||
                   top_pv-bottom_pv < min_pixel_height) continue;
               
               right_pu++;
               top_pv++;
         
               BBOX_PIXEL_COORDS bbox_corners(
                  left_pu,bottom_pv,right_pu,top_pv);
               BBOXES_MAP::iterator iter=bboxes_map_ptr->find(bbox_corners);
               if (iter==bboxes_map_ptr->end())
               {
                  (*bboxes_map_ptr)[bbox_corners]=extremal_region_ptr;

//               cout << "level = " << level
//                    << " char prob = " << basic_math::round(100*char_prob)
//                    << " bottom left: pu = " << left_pu
//                    << " pv = " << bottom_pv
//                    << " top right: pu = " << right_pu
//                    << " pv = " << top_pv << endl;
               }
            } // loop over index n labeling stable region nodes 
   
            cout << "Initially, bboxes_map_ptr->size() = " 
                 << bboxes_map_ptr->size() << endl;

// Merge bounding boxes whose sizes are nearly equal:
   
//            const int sqr_bbox_distance_toler=sqr(3);	// pixels
//            const int sqr_bbox_distance_toler=sqr(4);	// pixels
            const int sqr_bbox_distance_toler=sqr(6);	// pixels

            bool changed_flag=false;
            do 
            {
               changed_flag=false;

               for (BBOXES_MAP::iterator iter=bboxes_map_ptr->begin();
                    iter != bboxes_map_ptr->end(); iter++)
               {
                  BBOX_PIXEL_COORDS curr_bbox_corners=iter->first;
                  if (iter->second==NULL) continue;

                  int left_pu=curr_bbox_corners.first;
                  int bottom_pv=curr_bbox_corners.second;
                  int right_pu=curr_bbox_corners.third;
                  int top_pv=curr_bbox_corners.fourth;
      
                  for (BBOXES_MAP::iterator iter2=bboxes_map_ptr->begin();
                       iter2 != bboxes_map_ptr->end(); iter2++)
                  {
                     if (iter2==iter) continue;
                     if (iter->second==NULL) continue;
                     if (iter2->second==NULL) continue;
            
                     BBOX_PIXEL_COORDS next_bbox_corners=iter2->first;
                     double sqr_magnitude=
                        sqr(curr_bbox_corners.first-next_bbox_corners.first)+
                        sqr(curr_bbox_corners.second-next_bbox_corners.second)+
                        sqr(curr_bbox_corners.third-next_bbox_corners.third)+
                        sqr(curr_bbox_corners.fourth-next_bbox_corners.fourth);
                     if (sqr_magnitude > sqr_bbox_distance_toler) continue;

                     left_pu=basic_math::min(left_pu,next_bbox_corners.first);
                     bottom_pv=basic_math::min(
                        bottom_pv,next_bbox_corners.second);
                     right_pu=basic_math::max(
                        right_pu,next_bbox_corners.third);
                     top_pv=basic_math::max(top_pv,next_bbox_corners.fourth);
                     BBOX_PIXEL_COORDS new_bbox_corners(
                        left_pu,bottom_pv,right_pu,top_pv);
               
                     if (iter->second->get_pixel_area() > 
                     iter2->second->get_pixel_area())
                     {
                        (*bboxes_map_ptr)[new_bbox_corners]=iter->second;
                        iter2->second=NULL;
                     }
                     else
                     {
                        (*bboxes_map_ptr)[new_bbox_corners]=iter2->second;
                        iter->second=NULL;
                     }
                     changed_flag=true;
                  } // loop over iter2
               } // loop over iter
            }
            while (changed_flag);

            cout << "After bbox merging, bboxes_map_ptr->size() = " 
                 << bboxes_map_ptr->size() << endl;

// Prepare to compute Coates-Ng text character probabilities for all 
// surviving candidate bbox regions:

            for (BBOXES_MAP::iterator iter=bboxes_map_ptr->begin();
                 iter != bboxes_map_ptr->end(); iter++)
            {
               extremal_region* extremal_region_ptr=iter->second;
               if (extremal_region_ptr==NULL) continue;

// Compute fluctuations of red, green and blue colors within
// current extremal region.  "Letters" within TOC12 signs have nearly
// constant colorings.  So we reject any region if its color fluctations 
// are too large:

               double quartile_width_R,quartile_width_G,
                  quartile_width_B;
               videofunc::compute_RGB_fluctuations(
                  extremal_region_ptr->get_RLE_pixel_IDs(),
                  texture_rectangle_ptr,quartile_width_R,
                  quartile_width_G,quartile_width_B);
//               cout << "quartile_width_R = " << quartile_width_R 
//                    << " quartile_width_G = " << quartile_width_G
//                    << " quartile_width_B = " << quartile_width_B << endl;

//               double sqr_max_width=sqr(35);
               double sqr_max_width=sqr(50);
//               double sqr_max_width=sqr(255);
               double sqr_quartile_width=sqr(quartile_width_R)+
                  sqr(quartile_width_G)+
                  sqr(quartile_width_B);
               if (sqr_quartile_width > sqr_max_width) continue;

// Perform flood-filling around detected letters to find candidate
// bounding boxes for entire TOC12 signs:

               threevector median_interior_RGB=
                  texture_rectangle_ptr->find_interior_median_RGBs(
                     extremal_region_ptr);

               cc_twoDarray_ptr->clear_values();
               int output_value=255;
               extremal_region_ptr->run_length_decode(
                  cc_twoDarray_ptr,output_value);

               threevector median_perimeter_RGB;
               std::vector<twovector> perim_seeds;
               double perim_color_frac=
                  texture_rectangle_ptr->find_perimeter_seeds(
                     extremal_region_ptr,cc_twoDarray_ptr,perim_seeds,
                     median_perimeter_RGB);

               threevector RGB_difference(
                  median_interior_RGB-median_perimeter_RGB);

/*
               cout << "inter: R=" << median_interior_RGB.get(0)
                    << " G=" << median_interior_RGB.get(1)
                    << " B=" << median_interior_RGB.get(2)
                    << " perim: R=" << median_perimeter_RGB.get(0)
                    << " G=" << median_perimeter_RGB.get(1)
                    << " B=" << median_perimeter_RGB.get(2)
                    << endl;
*/

//               cout << "RGB_difference = " << RGB_difference
//                    << " perim_color_frac = " << perim_color_frac 
//                    << endl;

/*
               double median_interior_h,median_interior_s,median_interior_v;
               colorfunc::RGB_to_hsv(
                  median_interior_RGB.get(0)/255.0,
                  median_interior_RGB.get(1)/255.0,
                  median_interior_RGB.get(2)/255.0,
                  median_interior_h,median_interior_s,median_interior_v);

               double median_perim_h,median_perim_s,median_perim_v;
               colorfunc::RGB_to_hsv(
                  median_perimeter_RGB.get(0)/255.0,
                  median_perimeter_RGB.get(1)/255.0,
                  median_perimeter_RGB.get(2)/255.0,
                  median_perim_h,median_perim_s,median_perim_v);
               double v=basic_math::max(median_interior_v,median_perim_v);
               double renorm_RGB_difference=RGB_difference.magnitude()/v;
*/

               double lumin_interior=colorfunc::RGB_to_luminosity(
                  median_interior_RGB.get(0),
                  median_interior_RGB.get(1),
                  median_interior_RGB.get(2))/255.0;
               double lumin_perim=colorfunc::RGB_to_luminosity(
                  median_perimeter_RGB.get(0),
                  median_perimeter_RGB.get(1),
                  median_perimeter_RGB.get(2))/255.0;
               double lumin=basic_math::max(lumin_interior,lumin_perim);

               double renorm_RGB_difference=RGB_difference.magnitude()/lumin;

               cout << "renorm_RGB_difference = " << renorm_RGB_difference
                    << endl;

//               if (RGB_difference.magnitude() < 15) continue; 
               if (renorm_RGB_difference < min_RGB_diff) continue; 

/*
               RGB_difference=255*RGB_difference.unitvector();
               cout << "   renorm diff: R = " << RGB_difference.get(0)
                    << " G = " << RGB_difference.get(1)
                    << " B = " << RGB_difference.get(2)
                    << endl;

               threevector pink_rad_difference=threevector(255,0,255)-
                  threevector(0,0,255);
               
               double pink_rad_distance=
                  (RGB_difference-pink_rad_difference).magnitude();
//               cout << "pink_rad_distance = " << pink_rad_distance << endl;
*/

     
               const double min_perim_color_frac=0.175;
//               cout << "perim_color_frac = " << perim_color_frac << endl;
               if (perim_color_frac < min_perim_color_frac) continue;
               if (perim_seeds.size() < 4) continue;

// Display seed locations within clean version of original photo:

//            texture_rectangle_ptr->import_photo_from_file(image_filename);
//            for (int s=0; s<perim_seeds.size(); s++)
//            {
//               int px=perim_seeds[s].get(0);
//               int py=perim_seeds[s].get(1);
//               texture_rectangle_ptr->set_pixel_RGB_values(
//                  px,py,255,0,255);
//            }
//            string seeds_filename=bboxes_subdir+
//               "seeds_"+stringfunc::integer_to_string(
//                  extremal_region_ptr->get_ID(),3)+".jpg";
//            texture_rectangle_ptr->write_curr_frame(seeds_filename);

/*
// ...........................
// Flood-filling starts here

// Initial bbox corner coordinates for TOC12 letters:

               int init_left_pu,init_right_pu,init_top_pv,init_bottom_pv;
               extremal_region_ptr->
                  get_bbox(init_left_pu,init_top_pv,init_right_pu,
                  init_bottom_pv);

// If extremal region contains very few holes, flood-fill its exterior
// starting from seed locations.  Then compute bounding box
// surrounding flood-filled regions.  Otherwise, set symbol bounding
// box equal to current letter's bounding box:

               bounding_box* symbol_bbox_ptr=new bounding_box(
                  POSITIVEINFINITY,NEGATIVEINFINITY,
                  POSITIVEINFINITY,NEGATIVEINFINITY);
               if (extremal_region_ptr->get_n_holes() > 2)
               {
                  symbol_bbox_ptr->set_xy_bounds(
                     init_left_pu,init_right_pu,init_top_pv,init_bottom_pv);
               }
               else
               {
                  texture_rectangle_ptr->import_photo_from_file(
                     image_filename);
                  for (int s=0; s<perim_seeds.size(); s++)
                  {
//                     cout << "s = " << s << " perim_seeds.size() = "
//                          << perim_seeds.size() << endl;
                     
                     int seed_pu=perim_seeds[s].get(0);
                     int seed_pv=perim_seeds[s].get(1);
                     texture_rectangle_ptr->floodfill_color_region_bbox(
                        seed_pu,seed_pv,symbol_bbox_ptr);
                  }
               }
//               cout << "*symbol_bbox_ptr = " << *symbol_bbox_ptr << endl;

// Bbox corner coordinates for TOC12 symbol:

               int left_pu=symbol_bbox_ptr->get_xmin();
               int right_pu=symbol_bbox_ptr->get_xmax();
               int top_pv=symbol_bbox_ptr->get_ymin();
               int bottom_pv=symbol_bbox_ptr->get_ymax();

//               cout << "left_pu = " << left_pu << " right_pu = " << right_pu
//                    << " bottom_pv = " << bottom_pv << " top_pv = " << top_pv
//                    << endl;

// Reject cumulative bbox *symbol_bbox_ptr if it is too small or too large:

               double delta_pu=right_pu-left_pu;
               double delta_pv=bottom_pv-top_pv;
               double init_delta_pu=init_right_pu-init_left_pu;
               double init_delta_pv=init_bottom_pv-init_top_pv;
               if (delta_pu < 2 || delta_pv < 2) continue;

               const double max_magnification=5;
               if (delta_pu/init_delta_pu > max_magnification ||
                   delta_pv/init_delta_pv > max_magnification) continue;


// Flood-filling stops here
// ...........................

*/


/*
// Display decoded RLE pixel regions for all surviving extremal
// regions:

               texture_rectangle* ER_texture_rectangle_ptr=
                  new texture_rectangle();
               ER_texture_rectangle_ptr->import_photo_from_file(
                  image_filename);
               ER_texture_rectangle_ptr->
                  convert_single_twoDarray_to_three_channels(
                     cc_twoDarray_ptr,true);
               string ER_output_filename=bboxes_subdir+
                  "ER_"+stringfunc::integer_to_string(
                     extremal_region_ptr->get_ID(),3)+".jpg";
               ER_texture_rectangle_ptr->write_curr_frame(ER_output_filename);
               delete ER_texture_rectangle_ptr;
*/

// Bbox corner coords for individual letters:

               int left_pu=iter->first.first;
               int top_pv=iter->first.second;
               int right_pu=iter->first.third-1;
               int bottom_pv=iter->first.fourth-1;
  
//            cout << "left_pu = " << left_pu << " right_pu = " << right_pu
//                 << " bottom_pv = " << bottom_pv << " top_pv = " << top_pv
//                 << endl;
   
// Copy current bounding box chip into *qtwoDarray_ptr.  Then rescale
// chip's size so that new version stored in *qnew_twoDarray_ptr has
// height or width precisely equal to 32 pixels in size:

               int width=right_pu-left_pu+1;
               int height=bottom_pv-top_pv+1;
               double aspect_ratio=double(width)/double(height);

//            cout << "width = " << width << " height = " << height 
//                 << " aspect_ratio = " << aspect_ratio << endl;

               int new_width,new_height;
               if (aspect_ratio > 1)
               {
                  new_width=32;
                  new_height=new_width/aspect_ratio;
               }
               else
               {
                  new_height=32;
                  new_width=aspect_ratio*new_height;
               }
//            cout << "new_height = " << new_height
//                 << " new_width = " << new_width << endl;

               texture_rectangle_ptr->refresh_ptwoDarray_ptr();
               twoDarray* qtwoDarray_ptr=texture_rectangle_ptr->
                  export_sub_twoDarray(
                     left_pu,right_pu,top_pv,bottom_pv);
               qtwoDarray_ptr->init_coord_system(0,1,0,1);

               twoDarray* qnew_twoDarray_ptr=compositefunc::downsample(
                  new_width,new_height,qtwoDarray_ptr);
               delete qtwoDarray_ptr;

// For debugging purposes only, export *qnew_twoDarray_ptr as new JPG
// image chip:

//            texture_rectangle* subtexture_rectangle_ptr=new
//               texture_rectangle(new_width,new_height,1,3,NULL);
//            subtexture_rectangle_ptr->generate_blank_image_file(
//               new_width,new_height,"blank.jpg",0.5);
//            bool randomize_blue_values_flag=true;
//            subtexture_rectangle_ptr->
//               convert_single_twoDarray_to_three_channels(
//                  qnew_twoDarray_ptr,randomize_blue_values_flag);
//            string candidate_char_patches_subdir="./candidate_patches/";
//            filefunc::dircreate(candidate_char_patches_subdir);
//            string patch_filename=candidate_char_patches_subdir+
//               "candidate_char_patch_"+stringfunc::integer_to_string(
//                  candidate_char_counter++,3)+".jpg";
//            cout << "patch_filename = " << patch_filename << endl;
//            subtexture_rectangle_ptr->write_curr_frame(patch_filename);
//            delete subtexture_rectangle_ptr;

// Need to compute D=512-dim features for 8x8 patches within
// *qnew_twoDarray_ptr.  Pool features within 3x3 sectors into
// single 9xD vector.  Then compute probability rescaled chip
// corresponds to text character using learned Ng probability decision
// function...

               texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
               *(texture_rectangle_ptr->get_ptwoDarray_ptr()) = 
                  *qnew_twoDarray_ptr;
               delete qnew_twoDarray_ptr;
               
               text_detector_ptr->set_window_width(new_width);
               text_detector_ptr->set_window_height(new_height);

               bool perform_Ng_classification_flag=false;
//         bool perform_Ng_classification_flag=true;
               if (perform_Ng_classification_flag)
               {
                  double Ng_char_prob=0;
                  bool flag=text_detector_ptr->average_window_features(0,0);
                  if (flag)
                  {
                     float* window_histogram=text_detector_ptr->
                        get_nineK_window_descriptor();
                     for (int k=0; k<nineK; k++)
                     {
                        Ng_sample(k)=window_histogram[k];
//               cout << "k = " << k << " window_histogram[k] = "
//                    << window_histogram[k] << endl;
                     } // loop over index k labeling dictionary descriptors
                     Ng_char_prob=Ng_pfunct(Ng_sample);
                     cout << "Ng char probability = " << Ng_char_prob << endl
                          << endl;
//               outputfunc::enter_continue_char();
                  }
                  else
                  {
                     cout << "Averaged window features computation failed!" 
                          << endl;
                  }

// If we imported patch_filename into *texture_rectangle_ptr, we now
// need to reload image_filename into *texture_rectangle_ptr:

//            texture_rectangle_ptr->import_photo_from_file(image_filename);

                  if (Ng_char_prob < Ng_char_threshold)
                  {
                     n_Ng_rejections++;
                     continue;
                  }
                  else
                  {
                     n_Ng_acceptances++;
                  }
               } // perform_Ng_classification_flag conditional

               double left_u,top_v,right_u,bottom_v;
               texture_rectangle_ptr->get_uv_coords(
                  left_pu,top_pv,left_u,top_v);
               texture_rectangle_ptr->get_uv_coords(
                  right_pu,bottom_pv,right_u,bottom_v);
         
               std::vector<threevector> vertices;
               vertices.push_back(threevector(left_u,top_v));
               vertices.push_back(threevector(left_u,bottom_v));
               vertices.push_back(threevector(right_u,bottom_v));
               vertices.push_back(threevector(right_u,top_v));
               polygon bbox(vertices);
               particular_symbol_polygons.push_back(bbox);

               n_detections++;
            } // loop over *bboxes_map_ptr

            all_symbol_polygons.push_back(particular_symbol_polygons);

            cout << "symbol_ID = " << symbol_ID
                 << " particular_symbol_polygons.size() = " 
                 << particular_symbol_polygons.size()
                 << endl;
            particular_symbol_polygons.clear();

            delete cc_twoDarray_ptr;

         } // loop over symbol_ID index
// ......................................................................


         
      } // loop over brightness_iter
// =======================================================================

      cout << "n_detections = " << n_detections << endl;
      cout << "n_rejections = " << n_rejections << endl;
      cout << "n_Ng_acceptances = " << n_Ng_acceptances << endl;
      cout << "n_Ng_rejections = " << n_Ng_rejections << endl;
      double rejection_frac=
         double(n_rejections)/double(n_rejections+n_detections);
      cout << "R/(R+D) = " << rejection_frac << endl;

// Display all text character bounding boxes within a single,
// composite image:

      texture_rectangle* composite_texture_rectangle_ptr=
         new texture_rectangle();
      composite_texture_rectangle_ptr->import_photo_from_file(image_filename);

      string composite_output_filename=bboxes_subdir+
         "composite_char_bboxes_"+stringfunc::integer_to_string(
            image_counter,3)+".jpg";

      int polygons_counter=0;
      for (int symbol_ID=0; symbol_ID<1; symbol_ID++)
      {
         int thickness=0;
         std::vector<polygon> particular_symbol_polygons=
            all_symbol_polygons[polygons_counter++];
         cout << "symbol_ID = " << symbol_ID
              << " particular_symbol_polygons.size() = "
              << particular_symbol_polygons.size() << endl;
         
         videofunc::display_polygons(
            particular_symbol_polygons,
            composite_texture_rectangle_ptr,-1,thickness);
      } // loop over symbol_ID index
      all_symbol_polygons.clear();

      delete composite_texture_rectangle_ptr;

      double total_time=timefunc::elapsed_timeofday_time();
      cout << "TOTAL PROCESSING TIME = " << total_time << " secs = " 
           << total_time / 60.0 << " minutes" << endl;
      double avg_time_per_image=
         timefunc::elapsed_timeofday_time()/(image_counter-i_start+1);

      cout << "AVERAGE TIME PER IMAGE = " << avg_time_per_image << " secs"
           << endl;

   } // loop over image_counter index
// ***********************************************************************

   double avg_time_per_image=
      timefunc::elapsed_timeofday_time()/(i_stop-i_start);
   cout << "AVERAGE TIME PER IMAGE = " << avg_time_per_image << " secs"
        << endl;
   cout << "shapes_prob_threshold = " << shapes_prob_threshold << endl;

   delete connected_components_ptr;
   delete text_detector_ptr;
   delete bboxes_map_ptr;
   connectfunc::delete_extremal_region_pooled_memory();

}

