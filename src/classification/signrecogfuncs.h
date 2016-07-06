// ==========================================================================
// Header file for signrecogfunc namespace
// ==========================================================================
// Last modified on 11/2/12; 11/3/12; 11/4/12
// ==========================================================================

#ifndef SIGNRECOGFUNCS_H
#define SIGNRECOGFUNCS_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "image/extremal_regions_group.h"

class camera;
class connected_components;
class RGB_analyzer;
class text_detector;
class texture_rectangle;

namespace signrecogfunc
{
   typedef struct 
   {
         std::string symbol_name,sign_hue,bbox_color;
         std::vector<std::string> colors_to_find;
         double min_aspect_ratio,max_aspect_ratio;
         double min_compactness,max_compactness;
         int min_n_holes,max_n_holes;
         int min_n_crossings,max_n_crossings;
         int min_n_significant_holes,max_n_significant_holes;
         bool black_interior_flag,white_interior_flag,purple_interior_flag;
         double min_gradient_mag,max_bbox_hue_frac,Ng_threshold;
   } SIGN_PROPERTIES;

   const int K_Ng=1024;
   const int nineK=9*K_Ng;

   typedef dlib::matrix<double, nineK, 1> Ng_sample_type;
   typedef dlib::linear_kernel<Ng_sample_type> Ng_kernel_type;
   typedef dlib::probabilistic_decision_function<Ng_kernel_type> 
      Ng_probabilistic_funct_type;  
   typedef dlib::normalized_function<Ng_probabilistic_funct_type> 
      Ng_pfunct_type;

   typedef std::map<int,int> GRADSTEP_MAP;

// Image file manipulation methods:

   std::string generate_timestamped_archive_subdir(
      std::string input_images_subdir);
   std::string archive_all_but_latest_image_files(
      std::string input_images_subdir,std::string archived_images_subdir);

// PointGrey camera specific methods:

   void get_PointGrey_calibration_params(
      int PointGrey_camera_ID,
      int& Npu,int& Npv,double& f_pixels,
      double& cu_pixels,double& cv_pixels,double& k2,double& k4);
   void initialize_PointGrey_camera_params(
      int PointGrey_camera_ID,camera* camera_ptr);
   std::string radially_undistort_PointGrey_image(
      int PointGrey_camera_ID,
      std::string image_filename,texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* undistorted_texture_rectangle_ptr);
   bool detect_corrupted_PointGrey_image(
      std::string image_filename,texture_rectangle* texture_rectangle_ptr);
   std::string crop_white_border(std::string orig_image_filename);

// TOC12 sign initialization methods:

   std::vector<SIGN_PROPERTIES> initialize_sign_properties();
   std::vector<text_detector*> 
      import_Ng_probabilistic_classification_functions(
         const std::vector<SIGN_PROPERTIES>& sign_properties,
         std::vector<Ng_pfunct_type>& Ng_pfuncts);

// 

   std::string resize_input_image(
      std::string orig_image_filename,int& xdim,int& ydim);
   void reset_texture_image(
      std::string image_filename,texture_rectangle* texture_rectangle_ptr);
   void quantize_colors(
      RGB_analyzer* RGB_analyzer_ptr,
      SIGN_PROPERTIES& curr_sign_properties,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* selected_colors_texture_rectangle_ptr,
      texture_rectangle* binary_texture_rectangle_ptr);
   void compute_edgemaps(
      int curr_sign_ID,int sign_ID_start,
      SIGN_PROPERTIES& curr_sign_properties,
      SIGN_PROPERTIES& prev_sign_properties,
      texture_rectangle* edges_texture_rectangle_ptr,
      GRADSTEP_MAP& black_gradstep_map,GRADSTEP_MAP& white_gradstep_map);
   void find_hot_black_edges(
      int xdim,std::vector<std::pair<int,int> >& black_pixels,
      GRADSTEP_MAP& black_gradstep_map,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr);
   void find_hot_black_edges(
      int xdim,int flood_R,int flood_G,int flood_B,
      int black_flood_R,int black_flood_G,int black_flood_B,
      GRADSTEP_MAP& black_gradstep_map,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr);
   void find_hot_white_edges(
      int xdim,std::vector<std::pair<int,int> >& white_pixels,
      GRADSTEP_MAP& white_gradstep_map,
      texture_rectangle* quantized_texture_rectangle_ptr,      
      texture_rectangle* white_grads_texture_rectangle_ptr);

   void compute_connected_components(
      std::string binary_quantized_filename,
      SIGN_PROPERTIES& curr_sign_properties,
      std::vector<extremal_region*>& extremal_region_ptrs,
      std::vector<extremal_region*>& inverse_extremal_region_ptrs);

// Region tests methods:

   bool dominant_hue_mismatch(
      std::string sign_hue,std::string lookup_map_name,
      extremal_region* extremal_region_ptr,RGB_analyzer* RGB_analyzer_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr);
   bool small_region_bbox(
      unsigned int left_pu,unsigned int bottom_pv,
      unsigned int right_pu,unsigned int top_pv);
   bool large_region_bbox(
      int left_pu,int bottom_pv,int right_pu,int top_pv,int xdim,int ydim);
   int minimal_significant_hole_count(
      unsigned int left_pu,unsigned int right_pu,
      unsigned int bottom_pv,unsigned int top_pv,
      std::vector<extremal_region*> inverse_extremal_region_ptrs);
   bool minimal_n_hole_pixels(
      unsigned int left_pu,unsigned int right_pu,
      unsigned int bottom_pv,unsigned int top_pv,
      const std::vector<std::pair<int,int> >& black_pixels,
      const std::vector<std::pair<int,int> >& white_pixels,
      SIGN_PROPERTIES& curr_sign_properties);
   int n_purple_hole_pixels(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr);
   bool identify_purple_interior_pixels(
      std::string image_filename,
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr,
      SIGN_PROPERTIES& curr_sign_properties);
   double classify_dominant_colored_pixels(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* selected_colors_texture_rectangle_ptr,
      SIGN_PROPERTIES& curr_sign_properties);

// Ng classification methods

   void compute_bbox_chip_dimensions(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      int& new_width,int& new_height);
   void generate_bbox_chip(
      std::string symbol_name,int new_width,int new_height,
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr);
   double Ng_classification_prob(
      int curr_sign_ID,text_detector* text_detector_ptr,
      const std::vector<Ng_pfunct_type>& Ng_pfuncts);

// User interface methods

   void generate_bbox_polygons(
      int left_pu,int right_pu,int bottom_pv,int top_pv,
      texture_rectangle* texture_rectangle_ptr,
      SIGN_PROPERTIES& curr_sign_properties,
      std::vector<polygon>& bbox_polygons,
      std::vector<int>& bbox_color_indices);
   void export_bbox_polygons(
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* text_texture_rectangle_ptr,
      std::vector<polygon>& bbox_polygons,
      const std::vector<int>& bbox_color_indices,
      std::vector<std::string>& bbox_symbol_names);
   void print_processing_time(int image_counter);

//  Black & white TOC12 sign recognition methods

   void flood_fill_blackish_pixels(
      int xdim,int ydim,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* binary_texture_rectangle_ptr);
   void compute_bright_MSERs(
      std::string image_filename,extremal_regions_group& regions_group,
      texture_rectangle* edges_texture_rectangle_ptr);
   void link_bright_MSERs_and_black_regions(
      int xdim,int ydim,int flood_R,int flood_G,int flood_B,
      twoDarray* bright_cc_borders_twoDarray_ptr,
      twoDarray* black_cc_twoDarray_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      extremal_regions_group& regions_group);
   void form_bright_MSER_bbox_polygons(
      int xdim,int ydim,extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* coalesced_bright_region_map_ptr,
      texture_rectangle* texture_rectangle_ptr,
      std::vector<polygon>& bright_bbox_polygons);
   void form_flooded_black_region_bbox_polygons(
      int xdim,int ydim,extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr,
      texture_rectangle* texture_rectangle_ptr,
      std::vector<polygon>& dark_bbox_polygons);
   void form_bright_MSER_and_flooded_black_region_bbox_polygons(
      int xdim,int ydim,int curr_sign_ID,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr,
      SIGN_PROPERTIES& curr_sign_properties,
      texture_rectangle* texture_rectangle_ptr,
      std::vector<text_detector*>& text_detector_ptrs,
      const std::vector<Ng_pfunct_type>& Ng_pfuncts,
      std::vector<polygon>& bbox_polygons,
      std::vector<int>& bbox_color_indices);

   void extract_black_connected_components(
      std::string binary_quantized_filename,
      connected_components* connected_components_ptr,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr);
   void generate_linked_bright_MSERs_and_black_regions(
      int xdim,int ydim,int black_flood_R,int black_flood_G,int black_flood_B,
      std::string image_filename,std::string binary_quantized_filename,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      GRADSTEP_MAP& black_gradstep_map,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP*& coalesced_bright_region_map_ptr,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr);
   void merge_bright_MSER_and_flooded_black_region_bboxes(
      int xdim,int ydim,
      std::string image_filename,std::string binary_quantized_filename,
      texture_rectangle* binary_texture_rectangle_ptr,
      texture_rectangle* black_flooded_texture_rectangle_ptr,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr,
      GRADSTEP_MAP& black_gradstep_map,
      extremal_regions_group& regions_group,
      extremal_regions_group::ID_REGION_MAP* black_regions_map_ptr);

//  Colored TOC12 sign recognition methods

   void find_hot_edges(
      int xdim,SIGN_PROPERTIES& curr_sign_properties,
      std::vector<std::pair<int,int> >& black_pixels,
      std::vector<std::pair<int,int> >& white_pixels,
      GRADSTEP_MAP& black_gradstep_map,GRADSTEP_MAP& white_gradstep_map,
      texture_rectangle* black_grads_texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* white_grads_texture_rectangle_ptr);

   void form_colored_sign_bbox_polygons(
      int curr_sign_ID,
      std::vector<std::pair<int,int> >& black_pixels,
      std::vector<std::pair<int,int> >& white_pixels,
      std::vector<extremal_region*>& extremal_region_ptrs,
      std::vector<extremal_region*>& inverse_extremal_region_ptrs,
      SIGN_PROPERTIES& curr_sign_properties,
      RGB_analyzer* RGB_analyzer_ptr,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      std::vector<text_detector*>& text_detector_ptrs,
      const std::vector<Ng_pfunct_type>& Ng_pfuncts,
      std::vector<polygon>& bbox_polygons,
      std::vector<int>& bbox_color_indices);

   void form_colored_sign_bbox_polygons(
      int curr_sign_ID,std::string symbol_name,std::string image_filename,
      std::vector<extremal_region*>& extremal_region_ptrs,
      std::vector<extremal_region*>& inverse_extremal_region_ptrs,
      SIGN_PROPERTIES& curr_sign_properties,
      RGB_analyzer* RGB_analyzer_ptr,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* quantized_texture_rectangle_ptr,
      texture_rectangle* selected_texture_rectangle_ptr,
      std::vector<text_detector*>& text_detector_ptrs,
      const std::vector<Ng_pfunct_type>& Ng_pfuncts,
      std::vector<polygon>& bbox_polygons,
      std::vector<int>& bbox_color_indices,
      std::vector<std::string>& bbox_symbol_names);

// 3D relative position methods

   threevector compute_relative_bbox_position(
      camera* camera_ptr,double diagonal_corner_separation,
      polygon& bbox_polygon);
   std::vector<threevector> compute_relative_bbox_positions(
      camera* camera_ptr,double diagonal_corner_separation,
      std::vector<polygon>& bbox_polygons);

// Tank sign recognition methods

   void compute_connected_components_for_tank_sign_image(
      int xdim,int ydim,std::string image_filename,
      extremal_regions_group::ID_REGION_MAP* bright_regions_map_ptr);
   void count_colored_pixels(
      int pu,int pv,texture_rectangle* texture_rectangle_ptr,
      int& n_red_pixels,int& n_yellow_pixels,int& n_cyan_pixels,
      int& n_purple_pixels,
      int& red_pu,int& red_pv,int& yellow_pu,int& yellow_pv,
      int& cyan_pu,int& cyan_pv,int& purple_pu,int& purple_pv);
   void draw_colored_cell_COMs(
      const twovector& red_COM,const twovector& yellow_COM,
      const twovector& cyan_COM,const twovector& purple_COM,
      texture_rectangle* texture_rectangle_ptr,
      std::vector<polygon>& RYCP_polygons);
   void compute_tank_posn_rel_to_camera(
      std::vector<polygon>& RYCP_polygons,camera* camera_ptr,
      threevector& tank_posn_rel_to_camera);
   bool search_for_colored_checkerboard_in_bright_region(
      int xdim,extremal_region* region_ptr,
      std::vector<polygon>& RYCP_polygons,camera* camera_ptr,
      texture_rectangle* texture_rectangle_ptr,
      threevector& tank_posn_rel_to_camera);

} // signrecogfunc namespace

#endif // signrecogfuncs.h

