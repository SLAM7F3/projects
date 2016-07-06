// ==========================================================================
// Header file for sift_detector class
// ==========================================================================
// Last modified on 12/17/13; 12/23/13; 3/30/14; 4/3/14
// ==========================================================================

#ifndef SIFT_DETECTOR_H
#define SIFT_DETECTOR_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <osg/Image>

#include "datastructures/descriptor.h"
#include "structmotion/fundamental.h"
#include "math/ltquadruple.h"
#include "math/lttwovector.h"
#include "datastructures/Quadruple.h"
#include "image/TwoDarray.h"
#include "graphs/vptree.h"

#include <dlib/statistics.h>

class akm;
class ann_analyzer;
class camera;
class fundamental;
class homography;
class map_unionfind;
class photograph;
class photogroup;
class RGB_analyzer;
class texture_rectangle;

namespace cv
{
   class DMatch;
   class KeyPoint;
   class Mat;
}

class sift_detector
{

  public:

   typedef std::pair<descriptor*,descriptor*> feature_pair;

// To avoid circular dependence of sift_detector class upon
// FeaturesGroup class, we copy the following typedef from
// FeaturesGroup.h: 

   typedef std::map<int,std::vector<fourvector> > FEATURES_MAP;

// independent integer: feature_ID
// Dependent fourvectors: (pass_number,U,V,feature_index for curr pass)

   typedef std::map<int,bool> CANDIDATE_TIEPOINT_CURRFEATURE_IDS_MAP;
// independent int = curr image feature ID
// dependent bool : dummy var

   typedef std::map<int,std::pair<int,int> > 
      CANDIDATE_TIEPOINT_PAIRS_START_STOP_IMAGES_MAP;
// independent int = next image ID
// dependent int pair: start & stop candidate tiepoint IDs

   typedef std::map<quadruple,std::vector<feature_pair>,ltquadruple> 
      SOH_CORNER_DESCRIPTOR_MAP;

   typedef std::map<twovector,akm*,lttwovector> AKM_MAP;
// independent var: twovector containing image indices i and j
// dependent var: pointer to akm corresponding to images i and j

// independent quadruple = quantized subregion orientation histogram angles
// dependent STL vector contains SOH feature pairs

   sift_detector(photogroup* photogroup_ptr,bool FLANN_flag=true);
   sift_detector(const sift_detector& s);
   ~sift_detector();
   sift_detector& operator= (const sift_detector& s);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const sift_detector& s);

// Set and get member functions:

   void set_FLANN_flag(bool flag);
   void set_sampson_error_flag(bool flag);
   void set_root_sift_matching_flag(bool flag);
   void set_forward_feature_matching_flag(bool flag);
   bool get_forward_feature_matching_flag() const;
   void set_perform_SOH_corner_angle_test_flag(bool flag);
   void set_perform_Hamming_distance_test_flag(bool flag);
   void set_perform_min_descriptor_entropy_test_flag(bool flag);

   void set_num_threads(int n);
   void set_max_n_features_to_consider_per_image(unsigned int n);
   void set_min_allowed_U(double U);
   void set_max_allowed_U(double U);
   void set_min_allowed_V(double V);
   void set_max_allowed_V(double V);
   std::vector<std::vector<feature_pair> >& get_image_feature_info();
   std::vector<feature_pair>& get_image_feature_info(int i);
   std::vector<feature_pair>* get_image_feature_info_ptr(int i);

//   std::vector<unsigned long >* get_binary_descriptor_left_ptr(int i);
//   std::vector<unsigned long >* get_binary_descriptor_right_ptr(int i);
   std::vector<descriptor* >* get_Dbinary_ptrs_ptr(int i);
   vptree* get_vptree_ptr(int i);

   photogroup* get_photogroup_ptr();
   const photogroup* get_photogroup_ptr() const;
   fundamental* get_fundamental_ptr();
   const fundamental* get_fundamental_ptr() const;

   FEATURES_MAP& get_features_map();
   const FEATURES_MAP& get_features_map() const;
   akm* get_akm_ptr();
   const akm* get_akm_ptr() const;
   AKM_MAP* get_akm_map_ptr();
   const AKM_MAP* get_akm_map_ptr() const;

   std::vector<int>& get_inlier_tiepoint_ID();
   const std::vector<int>& get_inlier_tiepoint_ID() const;
   std::vector<twovector>& get_inlier_XY();
   const std::vector<twovector>& get_inlier_XY() const;
   std::vector<twovector>& get_inlier_UV();
   const std::vector<twovector>& get_inlier_UV() const;

   homography* get_homography_ptr();
   const homography* get_homography_ptr() const;

// SIFT feature extraction member functions:

   void add_image_feature_info(
      int i,const std::vector<feature_pair>& currimage_feature_info);
   void destroy_image_feature_info(int i);
   void extract_SIFT_features(
      std::string sift_keys_subdir,bool delete_pgm_file_flag=true);
   void extract_SIFT_features(
      std::vector<std::string>& image_filenames,std::string sift_keys_subdir,
      bool delete_pgm_file_flag=true);
   void parallel_extract_SIFT_features(
      std::string sift_keys_subdir,bool delete_pgm_file_flag=true);
   bool compute_OpenCV_SIFT_features(
      std::string image_filename,
      std::vector<feature_pair>& currimage_feature_info);

   bool parse_Lowe_features(
      bool Lowe_SIFT_flag,int photo_xdim,int photo_ydim,
      std::string sift_keys_filename,
      std::vector<feature_pair>& currimage_feature_info,int image_ID=-1);
   bool parse_Lowe_descriptors(
      std::string sift_keys_filename,std::vector<descriptor*>* D_ptrs_ptr);
   bool parse_Lowe_descriptors(
      bool Lowe_SIFT_flag,std::string sift_keys_filename,
      std::string descriptors_filename);

   std::vector<std::string>& generate_SIFT_keyfiles(
      std::string sift_keys_subdir,bool delete_pgm_file_flag=true);
   std::string generate_Lowe_keyfile(
      std::string sift_keys_subdir,photograph* photograph_ptr,
      bool delete_pgm_file_flag=true);
   std::string generate_Lowe_keyfile(
      std::string sift_keys_subdir,std::string image_filename,
      bool delete_pgm_file_flag=true);
   void generate_Lowe_keyfile(
      bool delete_pgm_file_flag,
      std::string sift_keys_filename,std::string image_filename);
   void print_features(unsigned int n_features_to_print);

// FREAK feature extraction and matching member function:

   void detect_OpenCV_keypoints(
      std::string image_filename,std::vector<cv::KeyPoint>& keypoints);
   void detect_OpenCV_keypoints(
      std::string image1_filename,std::string image2_filename,
      std::vector<cv::KeyPoint>& keypoints1,
      std::vector<cv::KeyPoint>& keypoints2);

   void extract_OpenCV_FREAK_features(
      std::string image_filename,std::vector<cv::KeyPoint>& keypoints,
      cv::Mat& descriptors);
   void raw_match_OpenCV_FREAK_features(
      cv::Mat& descriptors1,cv::Mat& descriptors2,
      std::vector<cv::DMatch>& matches);
   void raw_match_OpenCV_FREAK_features(
      std::string image1_filename,std::string image2_filename,
      std::vector<cv::KeyPoint>& keypoints1,
      std::vector<cv::KeyPoint>& keypoints2,
      std::vector<cv::DMatch>& matches);
   void color_prune_FREAK_matches(
      std::vector<cv::KeyPoint>& keypoints_1,
      std::vector<cv::KeyPoint>& keypoints_2,
      std::vector<cv::DMatch>& matches,RGB_analyzer* RGB_analyzer_ptr,
      std::string image1_filename,std::string image2_filename,
      texture_rectangle* texture_rectangle1_ptr,
      texture_rectangle* texture_rectangle2_ptr);

// Harris corner detector member functions:

   std::vector<cv::KeyPoint> extract_harris_corners(
      double R_min,std::string image_filename,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      texture_rectangle* corners_texture_rectangle_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      twoDarray* xxderiv_twoDarray_ptr,twoDarray* xyderiv_twoDarray_ptr,
      twoDarray* yyderiv_twoDarray_ptr);

   std::vector<cv::KeyPoint> extract_harris_corners(
      double R_min,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* edges_texture_rectangle_ptr,
      texture_rectangle* corners_texture_rectangle_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      twoDarray* xxderiv_twoDarray_ptr,twoDarray* xyderiv_twoDarray_ptr,
      twoDarray* yyderiv_twoDarray_ptr);
   
// Affine-SIFT (ASIFT) member functions:
   
   void extract_ASIFT_features(std::string asift_keys_subdir);
   std::string extract_asift_descriptors(
      std::string image_filename,std::string asift_features_filename,
      std::string asift_features_subdir);
   void parallel_extract_ASIFT_features(std::string asift_keys_subdir);

// Oxford detector-descriptor feature extraction member functions:

   void extract_Oxford_features(
      std::string detector_name,std::string Oxford_keys_subdir,
      bool delete_pgm_file_flag=true);
   std::vector<std::string>& generate_detector_descriptor_keyfiles(
      std::string detector_name,std::string sift_keys_subdir,
      bool delete_pgm_file_flag=true);
   std::string generate_detector_descriptor_keyfile(
      std::string detector_name,std::string sift_keys_subdir,
      std::string image_filename,bool delete_pgm_file_flag=true);
   void parse_detector_descriptor_features(
      photograph* photograph_ptr,std::string sift_keys_filename,
      std::vector<feature_pair>& currimage_feature_info);
   void append_tiepoint_inliers(
      int i,int j,const std::vector<cv::KeyPoint>& keypoints1,
      const std::vector<cv::KeyPoint>& keypoints2,
      double max_scalar_product);

// Consolidated SIFT + Hessian affine feature tracking member functions:

   void import_consolidated_features(std::string bundler_IO_subdir);
   void parse_consolidated_features(
      std::string bundler_IO_subdir,photograph* photograph_ptr,
      std::vector<feature_pair>& currimage_feature_info);

// SIFT feature import member functions:

   std::vector<std::string>& import_compressed_sift_hdf5_filenames(
      std::string sift_keys_subdir);
   std::vector<std::string>& 
      import_sift_keys_filenames(std::string sift_keys_subdir);
   void import_D_ptrs(
      std::string sift_keys_subdir,
      std::vector<int>* image_IDs_ptr,std::vector<descriptor*>* D_ptrs_ptr);
   void import_D_ptrs(
      std::string sift_keys_filename,int image_ID,
      std::vector<descriptor*>* D_ptrs_ptr);
   genmatrix* compute_SIFT_features_covar_matrix_sqrt(
      const std::vector<descriptor*>* D_ptrs_ptr,
      std::string covar_sqrt_filename);
   genmatrix* import_SIFT_features_covar_matrix_sqrt(
      std::string covar_sqrt_filename);

// HOG feature extraction member functions:

   void extract_HOG_features(unsigned int n_rows,unsigned int n_columns);
   void extract_HOG_features(
      std::string image_filename,unsigned int n_columns,unsigned int n_rows,
      std::vector<feature_pair>& currimage_feature_info);

   void extract_CHOG_features(int n_requested_features);
   void extract_CHOG_features(
      std::string image_filename,int n_requested_features,
      std::vector<feature_pair>& currimage_feature_info);

// SURF feature extraction member functions:

   void extract_SURF_features(std::string SURF_keys_subdir);
   void extract_SURF_features(
      std::string SURF_keys_subdir,std::vector<std::string>& image_filenames);
   void extract_SURF_features(
      std::string SURF_keys_subdir,std::string image_filename);
   void import_SURF_features(
      std::string SURF_keys_subdir,std::string image_filename,
      std::vector<feature_pair>& currimage_feature_info);

// CHOG feature matching via fundamental matrix member functions:

   void identify_CHOG_feature_matches_via_fundamental_matrix(
      double max_ratio,double worst_frac_to_reject,
      double max_scalar_product);
   bool identify_CHOG_feature_matches_via_fundamental_matrix(
      int i,int j,double max_ratio,double worst_frac_to_reject,
      double max_scalar_product);
   void identify_CHOG_feature_matches_for_image_pair(
      const std::vector<feature_pair>& currimage_feature_info,
      const std::vector<feature_pair>& nextimage_feature_info,
      double max_scalar_product);

// Candidate SIFT feature matching member functions:

   void prepare_all_SIFT_descriptors();
   void prepare_SIFT_descriptors(int i);
   void prepare_SIFT_descriptors(
      std::vector<feature_pair>* currimage_feature_info_ptr);
   void prepare_SIFT_descriptors2(
      std::vector<feature_pair>* nextimage_feature_info_ptr);
   void prepare_SIFT_descriptors(int i,int j,akm* curr_akm_ptr);

   bool identify_candidate_feature_matches_via_Lowe_ratio_test(
      int i,int j,double sqrd_max_ratio);
   bool identify_candidate_feature_matches_via_Lowe_ratio_test(
      int i,int jstart,int jstop,double sqrd_max_ratio);
   bool identify_candidate_feature_matches_via_Lowe_ratio_test_2(
      int i,int j,double sqrd_max_ratio);

   int identify_candidate_FLANN_feature_matches_for_image_pair(
      int& c_start,int& c_stop,
      const std::vector<feature_pair>* currimage_feature_info_ptr,
      const std::vector<feature_pair>* nextimage_feature_info_ptr,
      double max_distance_ratio);
   int identify_candidate_FLANN_feature_matches_for_image_pair(
      akm* curr_akm_ptr,double max_distance_ratio,
      const std::vector<feature_pair>* currimage_feature_info_ptr,
      const std::vector<feature_pair>* nextimage_feature_info_ptr,
      std::vector<feature_pair>& curr_candidate_tiepoint_pairs);

   void instantiate_FLANN_index(int i,int j_start,int j_stop);   
   void match_image_pair_features(
      int i,int j_start,int j_stop,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_scalar_product,
      int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
      int minimal_number_of_inliers,std::string bundler_IO_subdir,
      map_unionfind* map_unionfind_ptr,std::ofstream& ntiepoints_stream);
   void parallel_match_image_pair_features(
      bool export_fundamental_matrices_flag,
      int i,int j_start,int j_stop,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_scalar_product,
      int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
      int minimal_number_of_inliers,std::string bundler_IO_subdir,
      map_unionfind* map_unionfind_ptr,std::ofstream& ntiepoints_stream);
   void match_successive_image_features(
      int i,int j,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_scalar_product,
      int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
      int minimal_number_of_inliers,std::string bundler_IO_subdir,
      map_unionfind* map_unionfind_ptr,std::ofstream& ntiepoints_stream);

   void restricted_match_image_pair_features(
      int i,int j_start,int j_stop,
      std::vector<camera*>& camera_ptrs,photogroup* photogroup_ptr,
      double max_camera_angle_separation,
      double sqrd_max_ratio,double worst_frac_to_reject,
      double max_scalar_product,int max_n_good_RANSAC_iters,
      int min_candidate_tiepoints,int minimal_number_of_inliers,
      std::string bundler_IO_subdir,
      map_unionfind* map_unionfind_ptr,std::ofstream& ntiepoints_stream);
   void parallel_restricted_match_image_pair_features(
      bool export_fundamental_matrices_flag,
      int i,int j_start,int j_stop,
      std::vector<camera*>& camera_ptrs,photogroup* photogroup_ptr,
      double max_camera_angle_separation,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_scalar_product,
      int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
      int minimal_number_of_inliers,std::string bundler_IO_subdir,
      map_unionfind* map_unionfind_ptr,std::ofstream& ntiepoints_stream);

   void reorder_parallelized_tiepoints_file(std::string bundler_IO_subdir);

// Inlier feature identification via fundamental matrix member
// functions:

   bool identify_inlier_matches_via_fundamental_matrix(
      int i,int j,std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      fundamental* fundamental_ptr,
      double worst_frac_to_reject,double max_scalar_product,
      int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
      int minimal_number_of_inliers,
      bool ignore_triple_roots_flag=true);
   bool compute_fundamental_matrix_via_RANSAC(
      const std::vector<twovector>& tiepoint_UV,
      const std::vector<twovector>& tiepoint_UVmatch,
      std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      fundamental* curr_fundamental_ptr,
      double max_scalar_product,double worst_frac_to_reject,
      bool ignore_triple_roots_flag,int max_n_good_RANSAC_iters,
      int minimal_number_of_inliers,
      double& min_RANSAC_cost,int thread_i,int thread_j);

   bool compute_seven_point_fundamental_matrix(
      const std::vector<int>& tiepoint_indices,
      const std::vector<twovector>& tiepoint_UV,
      const std::vector<twovector>& tiepoint_UVmatch,
      std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      fundamental* curr_fundamental_ptr,
      double max_scalar_product,bool ignore_triple_roots_flag,
      double& min_RANSAC_cost,int thread_i,int thread_j);
   bool compute_seven_point_fundamental_matrix(
      dlib::random_subset_selector<feature_pair>& tiepoint_samples,
      fundamental* curr_fundamental_ptr,int thread_i,int thread_j);
   bool compute_candidate_fundamental_matrix();

   int identify_inliers_via_fundamental_matrix(
      double max_scalar_product,
      const std::vector<twovector>& tiepoint_UV,
      const std::vector<twovector>& tiepoint_UVmatch,
      const std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      fundamental* curr_fundamental_ptr,double& min_RANSAC_cost,
      int thread_i,int thread_j);
   void store_tiepoint_twovectors(
      const std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      std::vector<twovector>& tiepoint_UV,
      std::vector<twovector>& tiepoint_UVmatch);
   int recover_inlier_tiepoints();
   int compute_inlier_fundamental_matrix(
      double worst_frac_to_reject,
      std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      fundamental* curr_fundamental_ptr,int thread_i,int thread_j);

// Inlier feature identification via affine transformation member
// functions:

   int identify_tiepoint_inliers_via_affine_transformation(
      unsigned int n_affine_RANSAC_iters,
      const std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      std::vector<feature_pair>& affine_inlier_tiepoint_pairs);

// Subregion orientation histogram member functions:

   void check_tiepoint_corner_angles(int i,int j);
   int count_SOH_corner_angle_matches(
      descriptor* curr_D_ptr,descriptor* next_D_ptr);
   double compute_SOH_angle(int i,int j,descriptor* D_ptr);
   void quantize_SOH_corner_angles(
      feature_pair& current_feature_pair,quadruple& current_quadruple);
   void quantize_currimage_feature_info(
      std::vector<feature_pair>& currimage_feature_info);
   void quantize_cumimage_feature_info(
      std::vector<feature_pair>& cumimage_feature_info);
   int identify_candidate_SOH_feature_matches_for_image_pair(
      const std::vector<feature_pair>& currimage_feature_info,
      const std::vector<feature_pair>& nextimage_feature_info);

   void renormalize_quadruple_indices(quadruple& curr_quadruple);
   std::vector<quadruple> quadruple_neighborhood(
      quadruple& curr_quadruple);
   int curr_feature_SOH_neighborhood(
      quadruple& curr_quadruple,
      std::vector<feature_pair>& curr_feature_neighborhood_info);
   int cum_feature_SOH_neighborhood(
      quadruple& cum_quadruple,
      std::vector<feature_pair>& cum_feature_neighborhood_info);

// SIFT feature matching via homography member functions:

   void identify_candidate_feature_matches_via_homography(
      int n_min_quadrant_features,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_sqrd_delta);
   bool identify_candidate_feature_matches_via_homography(
      int i,int jstart,int jstop,
      int n_min_quadrant_features,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_sqrd_delta);

   bool identify_inlier_matches_via_homography(
      int i,int j,int max_n_good_RANSAC_iters,
      double worst_frac_to_reject,double max_sqrd_delta);
   bool identify_inlier_matches_via_homography(
      int i,int j,int max_n_good_RANSAC_iters,
      double worst_frac_to_reject,double max_sqrd_delta,
      std::vector<feature_pair>& curr_candidate_tiepoint_pairs);
   bool compute_homography_via_RANSAC(
      std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      int max_n_good_RANSAC_iters,double max_sqrd_delta);
   void compute_four_point_homography(std::vector<feature_pair>& 
      curr_candidate_tiepoint_pairs);
   void identify_inliers_via_homography(
      std::vector<feature_pair>& curr_candidate_tiepoint_pairs,
      double max_sqrd_delta,bool print_flag=true,
      bool feature_ray_matching_flag=false);
   void compute_inlier_homography(
      double worst_frac_to_reject,bool feature_ray_matching_flag=false);

   std::pair<int,int> maximal_number_forward_feature_matches(
      int i,int jstart,int jstop,double sqrd_max_ratio);
   int count_candidate_forward_feature_matches(
      const std::vector<feature_pair>& currimage_feature_info,
      const std::vector<feature_pair>& nextimage_feature_info,
      double sqrd_max_ratio);
   std::vector<int> identify_overlapping_images(
      int i,int jstart,int jstop,int max_n_feature_matches,
      double frac_max_n_feature_matches,double sqrd_max_ratio);

   void identify_candidate_feature_ray_matches(
      int i,const std::vector<int>& overlapping_image_indices,
      double sqrd_max_ratio);


   void homography_match_image_pair_features(
      int i,int j,int max_n_good_RANSAC_iters,double sqrd_max_ratio,
      double worst_frac_to_reject,double max_sqrd_delta,
      std::string features_subdir);

// RANSAC member functions:

   void identify_inlier_feature_ray_matches(
      int n_min_quadrant_features,
      double worst_frac_to_reject,double max_sqrd_delta);
   bool compute_candidate_homography(
      double bbox_area,bool feature_ray_matching_flag=false);

   int compute_n_iters(double eps,int n_tiepoints=4);

   void rename_feature_IDs(int i,int j);
   void rename_feature_IDs(
      int i,int j,const std::vector<feature_pair>& curr_inlier_tiepoint_pairs);

// map_unionfind member functions:

//   map_unionfind::DUPLE get_node_ID(int i,int F_ID) const;
//   int get_feature_ID(double node_ID) const;
//   int get_image_number(double node_ID) const;
   void load_node_IDs(
      int i,map_unionfind* map_unionfind_ptr);
   void link_matching_node_IDs(
      int i,int j,const std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      map_unionfind* map_unionfind_ptr);
   void rename_feature_IDs(map_unionfind* map_unionfind_ptr);

// Feature export member functions:

   std::string export_feature_tiepoints(
      int i,int j,const std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      std::string tiepoints_subdirk,photogroup* curr_photogroup_ptr,
      int n_duplicate_features,map_unionfind* map_unionfind_ptr,
      std::ofstream& ntiepoints_stream) const;
   std::string export_feature_tiepoints(
      int i,int j,const std::vector<feature_pair>& curr_inlier_tiepoint_pairs,
      std::string tiepoints_subdirk,photogroup* curr_photogroup_ptr,
      int n_duplicate_features,double sensor_separation_angle,
      map_unionfind* map_unionfind_ptr,std::ofstream& ntiepoints_stream) 
      const;
   int export_feature_tracks(int i);
   int export_feature_tracks(int i,std::string features_subdir);
   int export_feature_tracks(
      int i,std::string features_subdir,std::vector<feature_pair>* 
      currimage_feature_info_ptr);
   int export_all_feature_tracks(
      int i,std::string features_filename,
      std::vector<feature_pair>* currimage_feature_info_ptr);
   void generate_features_map();
   void generate_features_map(int i);
   void export_fundamental_matrix(
      fundamental* curr_fundamental_ptr,
      std::string bundler_IO_subdir,int i,int j);
   void export_homography_matrix(std::string bundler_IO_subdir,int i,int j);
   void export_features_to_Lowe_keyfile(
      int photo_ydim,std::string output_keyfilename,
      const std::vector<feature_pair>& currimage_feature_info);

// Horn's relative orientation and baseline determination member functions

   std::vector<std::pair<threevector,threevector> >
      convert_matching_binocular_features_into_rays(
         camera* cameraL_ptr,camera* cameraR_ptr);

// 3x3 projection matrix member functions:
   
   double compute_ray_feature_homography(
      std::vector<feature_pair>* feature_ray_ptr,
      double input_frac_to_use=1.0,bool check_ray_feature_homography=false);
   double compute_ray_feature_homography(
      const std::vector<twovector>& UV,const std::vector<threevector>& nhat,
      double input_frac_to_use=1.0,bool check_ray_projection_flag=false);
   void compute_projection_matrix(photograph* photograph_ptr);
   void compute_projection_matrix(
      const threevector& camera_world_posn,genmatrix* P_ptr);
   void write_projection_package_file(
      double frustum_sidelength,double downrange_distance,
      std::string output_package_subdir,photograph* photograph_ptr);

// Binary SIFT member functions:

   std::vector<int>& compute_descriptor_component_medians();
   void binary_quantize_SIFT_descriptors();
   void generate_VPtrees();
   void match_binary_SIFT_descriptors();
   void check_hamming_distances(int i,int j);
   void check_binary_matches(int i,int j);

  protected:

   std::vector<int> inlier_tiepoint_ID;
   std::vector<twovector> inlier_XY,inlier_UV;

  private: 

   bool FLANN_flag,sampson_error_flag,root_sift_matching_flag;
   bool forward_feature_matching_flag;
   bool perform_SOH_corner_angle_test_flag,perform_Hamming_distance_test_flag,
      perform_min_descriptor_entropy_test_flag;
   int num_threads;
   unsigned int max_n_features_to_consider_per_image;
   unsigned int f_dims,d_dims;
   unsigned int n_images;
   int max_n_inliers,n_iters;
   int feature_counter,n_duplicate_features;
   std::vector<int> image_feature_indices;
   double min_allowed_U,max_allowed_U,min_allowed_V,max_allowed_V;

// Binary SIFT members:

   int d_index_1,d_index_2;
   std::vector<int> median_d_values;
//   std::vector<std::vector<unsigned long> > 
//      binary_descriptor_left,binary_descriptor_right;
   std::vector<std::vector<descriptor* > > Dbinary_ptrs;
   std::vector<vptree* > vptree_ptrs;

// Sub-orientation histogram members:

   int max_quadruple_index;
   double SOH_binsize;
   std::vector<double> sin_orientation,cos_orientation;

   std::vector<std::vector<feature_pair> > image_feature_info;
   std::vector<feature_pair> candidate_tiepoint_pairs,
      inlier_tiepoint_pairs;
   std::vector<feature_pair> UR_candidate_tiepoint_pairs,
      UL_candidate_tiepoint_pairs,LL_candidate_tiepoint_pairs,
      LR_candidate_tiepoint_pairs;

   photogroup* photogroup_ptr;
   std::vector<std::string> sift_keys_filenames;
   ann_analyzer *ANN_ptr,*inverse_ANN_ptr;
   akm* akm_ptr;
   AKM_MAP* akm_map_ptr;

   homography* H_ptr;
   fundamental* fundamental_ptr;

   FEATURES_MAP features_map;
   CANDIDATE_TIEPOINT_CURRFEATURE_IDS_MAP 
      candidate_tiepoint_currfeature_ids_map;
   CANDIDATE_TIEPOINT_CURRFEATURE_IDS_MAP::iterator
      candidate_tiepoint_currfeature_ids_iter;

   CANDIDATE_TIEPOINT_PAIRS_START_STOP_IMAGES_MAP 
      candidate_tiepoint_pairs_start_stop_images_map;

   SOH_CORNER_DESCRIPTOR_MAP curr_SOH_corner_descriptor_map,
      cum_SOH_corner_descriptor_map;
   SOH_CORNER_DESCRIPTOR_MAP::iterator curr_SOH_corner_iter,
      cum_SOH_corner_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const sift_detector& p);

   void destroy_allocated_features_for_specified_image(
      std::vector<feature_pair>* currimage_feature_info_ptr);
   void initialize_ANN_analyzers();

   double bin_features_into_quadrants(int n_min_quadrant_features);
   int identify_candidate_feature_matches_for_image_pair(
      const std::vector<feature_pair>& currimage_feature_info,
      const std::vector<feature_pair>& nextimage_feature_info,
      double sqrd_max_ratio);
   void refine_inliers_identification(
      double max_sqrd_delta,bool feature_ray_matching_flag=false);
   void refine_inliers_identification_via_fundamental_matrix(
      double max_scalar_product);

   twovector recover_UV_from_F(descriptor* F_ptr);
   threevector recover_nhat_from_F(descriptor* F_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void sift_detector::set_FLANN_flag(bool flag)
{
   FLANN_flag=flag;
}

inline void sift_detector::set_sampson_error_flag(bool flag)
{
   sampson_error_flag=flag;
}

inline void sift_detector::set_root_sift_matching_flag(bool flag)
{
   root_sift_matching_flag=flag;
}

inline void sift_detector::set_forward_feature_matching_flag(bool flag)
{
   forward_feature_matching_flag=flag;
}

inline bool sift_detector::get_forward_feature_matching_flag() const
{
   return forward_feature_matching_flag;
}

inline void sift_detector::set_perform_SOH_corner_angle_test_flag(bool flag)
{
   perform_SOH_corner_angle_test_flag=flag;
}

inline void sift_detector::set_perform_Hamming_distance_test_flag(bool flag)
{
   perform_Hamming_distance_test_flag=flag;
}

inline void sift_detector::set_perform_min_descriptor_entropy_test_flag(
   bool flag)
{
   perform_min_descriptor_entropy_test_flag=flag;
}

inline void sift_detector::set_num_threads(int n)
{
   num_threads=n;
}

inline void sift_detector::set_max_n_features_to_consider_per_image(
   unsigned int n)
{
   max_n_features_to_consider_per_image=n;
}

inline void sift_detector::set_min_allowed_U(double U)
{
   min_allowed_U=U;
}

inline void sift_detector::set_max_allowed_U(double U)
{
   max_allowed_U=U;
}

inline void sift_detector::set_min_allowed_V(double V)
{
   min_allowed_V=V;
}

inline void sift_detector::set_max_allowed_V(double V)
{
   max_allowed_V=V;
}

inline std::vector<std::vector<sift_detector::feature_pair> >& 
sift_detector::get_image_feature_info()
{
   return image_feature_info;
}

inline std::vector<sift_detector::feature_pair>& 
sift_detector::get_image_feature_info(int i)
{
   return image_feature_info.at(i);
}

inline std::vector<sift_detector::feature_pair>*
sift_detector::get_image_feature_info_ptr(int i)
{
   return &(image_feature_info.at(i));
}

inline twovector sift_detector::recover_UV_from_F(descriptor* F_ptr)
{
   return twovector(F_ptr->get(1),F_ptr->get(2));
}

inline threevector sift_detector::recover_nhat_from_F(descriptor* F_ptr)
{
   return threevector(F_ptr->get(6),F_ptr->get(7),F_ptr->get(8));
}

inline photogroup* sift_detector::get_photogroup_ptr()
{
   return photogroup_ptr;
}

inline const photogroup* sift_detector::get_photogroup_ptr() const
{
   return photogroup_ptr;
}

inline fundamental* sift_detector::get_fundamental_ptr()
{
   return fundamental_ptr;
}

inline const fundamental* sift_detector::get_fundamental_ptr() const
{
   return fundamental_ptr;
}

inline sift_detector::FEATURES_MAP& sift_detector::get_features_map()
{
   return features_map;
}

inline const sift_detector::FEATURES_MAP& 
sift_detector::get_features_map() const
{
   return features_map;
}

inline akm* sift_detector::get_akm_ptr()
{
   return akm_ptr;
}

inline const akm* sift_detector::get_akm_ptr() const
{
   return akm_ptr;
}

inline sift_detector::AKM_MAP* sift_detector::get_akm_map_ptr()
{
   return akm_map_ptr;
}

inline const sift_detector::AKM_MAP* sift_detector::get_akm_map_ptr() const
{
   return akm_map_ptr;
}

inline std::vector<int>& sift_detector::get_inlier_tiepoint_ID()
{
   return inlier_tiepoint_ID;
}

inline const std::vector<int>& sift_detector::get_inlier_tiepoint_ID() const
{
   return inlier_tiepoint_ID;
}

inline std::vector<twovector>& sift_detector::get_inlier_XY()
{
   return inlier_XY;
}

inline const std::vector<twovector>& sift_detector::get_inlier_XY() const
{
   return inlier_XY;
}

inline std::vector<twovector>& sift_detector::get_inlier_UV()
{
   return inlier_UV;
}

inline const std::vector<twovector>& sift_detector::get_inlier_UV() const
{
   return inlier_UV;
}

inline homography* sift_detector::get_homography_ptr()
{
   return H_ptr;
}

inline const homography* sift_detector::get_homography_ptr() const
{
   return H_ptr;
}



#endif  // sift_detector.h
