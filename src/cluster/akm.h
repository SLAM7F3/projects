// ==========================================================================
// Header file for Approximate K-Means (AKM) class
// ==========================================================================
// Last modified on 6/19/13; 7/8/13; 8/30/13; 4/4/14
// ==========================================================================

#ifndef AKM_H
#define AKM_H

#include <map>
#include <vector>
#include <fastann/fastann.hpp>
#include <flann/flann.hpp>
#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"
#include <flann/io/hdf5.h>
#include "math/genmatrix.h"

class descriptor;

class akm
{
   
   typedef std::pair<descriptor*,descriptor*> feature_pair;

  public:

   akm(bool flann_flag=true);
   ~akm();
   akm(const akm& a);
   akm& operator= (const akm& a);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const akm& a);

// Set and get methods:

   void set_FLANN_flag(bool flag);
   void set_SIFT_descriptors(float* SIFT_descriptors);
   void set_SIFT_descriptors_matrix_ptr(flann::Matrix<float>* matrix_ptr);
   void set_SIFT_descriptors2_matrix_ptr(flann::Matrix<float>* matrix_ptr);
   void set_cluster_centers_matrix_ptr(flann::Matrix<float>* matrix_ptr);
   void set_inverse_sqrt_covar_matrix_ptr(genmatrix* inverse_sqrt_covar_ptr);
   double get_mu_n_features_per_cluster() const;
   double get_sigma_n_features_per_cluster() const;
   int get_K() const;

   std::vector<std::pair<int,int> >* get_initial_SIFT_matches_ptr();
   const std::vector<std::pair<int,int> >* get_initial_SIFT_matches_ptr() 
      const;

// Approximate K-means member functions:
  
   void reset_params(int n,int d,int k,int num_iters);
   void reset_params(int n,int d,int k);
   void randomly_initialize_cluster_centers();
   void randomly_initialize_FLANN_cluster_centers(
      const std::vector<int>& cluster_center_IDs);
   void randomly_initialize_FASTANN_cluster_centers(
      const std::vector<int>& cluster_center_IDs);
   void iteratively_refine_clusters();
   void iteratively_refine_FLANN_clusters();
   void iteratively_refine_FASTANN_clusters();

// Approximate SIFT feature matching member functions:

   bool load_SIFT_descriptors(
      const std::vector<feature_pair>* currimage_feature_info_ptr);
   void pushback_SIFT_descriptors_matrix_ptr();

   bool load_SIFT_descriptors(const std::vector<descriptor*>* D_ptrs_ptr);
   bool load_SIFT_descriptors(
      int n_total_features,int d_dims,const unsigned char* char_array,
      int n_skip);
   bool load_SIFT_descriptors(
      int currimage_ID,const std::vector<int>* image_IDs_ptr,
      const std::vector<descriptor*>* D_ptrs_ptr);
   void whiten_SIFT_descriptors();
   void whiten_SIFT_descriptors(
      const flann::Matrix<float>& inverse_sqrt_covar);

   bool load_SIFT_descriptors2(
      const std::vector<feature_pair>* nextimage_feature_info);
   bool load_SIFT_descriptors2(
      const std::vector<std::vector<feature_pair> >& currimage_feature_infos);

   void initialize_randomized_kdtree_index();
   void initialize_SIFT_feature_search(int i_forward,int j_backward);
   void initialize_backward_SIFT_feature_search();
   void initialize_forward_SIFT_feature_search();

   void find_only_forward_SIFT_matches(double max_distance_ratio);
   void find_only_backward_SIFT_matches(double max_distance_ratio);
   std::vector<std::pair<int,int> > find_bijective_SIFT_matches(
      double max_distance_ratio);
   void find_forward_SIFT_matches(double max_distance_ratio);
   void find_backward_SIFT_matches(double max_distance_ratio);

// Term frequency-inverse document frequency member functions:

   void clear_document_word_frequencies(int K);
   void clear_document_word_frequencies(int n_images,int K);
   void initialize_tfidf_sparse_matrix(int n_images,int K);

   int compute_image_words(
      int curr_image_ID,std::string image_filename,
      std::string image_words_subdir);
   void compute_term_frequencies(
      int curr_image_ID,std::string image_words_filename,
      std::string term_freqs_subdir);
   void compute_inverse_document_frequencies(
      int n_images,std::string sift_keys_subdir);
   void import_inverse_doc_freq_matrix(std::string sift_keys_subdir);

   void compute_tfidfs(
      int curr_image_ID,std::string term_freqs_filename,
      std::string tfidfs_subdir);
   
   void import_tfidfs(int curr_image_ID,std::string tfidfs_filename);
   std::string export_tfidf_sparse_matrix(std::string sift_keys_subdir);

   void compute_term_frequencies(int curr_image_ID);
   void compute_tfidfs_for_nplusone_image(
      int image_ID,std::string nplusone_subdir);
   double compare_Nplusone_image_with_archive_image(
      std::string tfidfs_filename,std::string& image_basename);


   void compute_tfidfs_for_single_image(int image_ID);
   void compute_tfidfs(unsigned int n_images);

   void compare_Nplusone_image_with_N_images(
      std::vector<int>& image_indices,std::vector<float>& dotproducts);

// Binary HDF5 file import/export member functions:

   std::string export_SIFT_features_in_HDF5_format(
      std::string sift_keys_filename,std::string hdf5_subdir,
      std::string output_file_prefix,
      const std::vector<descriptor*>* D_ptrs_ptr);
   void export_SIFT_features(
      std::vector<std::string>& sift_keys_filenames,
      std::string output_file_prefix,
      const std::vector<int>* image_IDs_ptr,
      const std::vector<descriptor*>* D_ptrs_ptr);
   void export_all_SIFT_features(
      std::string output_filename,std::string HDF5_label,
      const std::vector<descriptor*>* D_ptrs_ptr);

   int export_cluster_centers(
      int min_features_per_cluster,
      std::string output_file_prefix,std::string sift_keys_subdir);
   void export_partial_tfidf_and_multi_image_words(
      int n_processed_images,std::string sift_keys_subdir);
   void export_vocabulary(
      std::string output_file_prefix,std::string sift_keys_subdir,
      std::string& inverse_doc_freq_filename,std::string& vocab_filename);
   void import_vocabulary(
      std::string output_file_prefix,std::string sift_keys_subdir);

  private: 

   bool FLANN_flag;
   unsigned N,D,K;
   unsigned int n_iters;
   double cost_function;
   double mu_n_features_per_cluster,sigma_n_features_per_cluster;

   int *n_features_in_cluster,*image_word_count;
   int *multi_image_word_occurrence;
   float *term_frequency,*tfidf;

   flann::Matrix<float>* inverse_document_frequency_matrix_ptr;

   gmm::row_matrix< gmm::wsvector<float> >* tfidf_sparse_matrix_ptr;

   float *SIFT_descriptors,*cluster_centers,*new_cluster_centers;
   genmatrix* inverse_sqrt_covar_matrix_ptr;

   flann::Matrix<int>* indices_matrix_ptr;
   flann::Matrix<float>* dists_matrix_ptr;
   flann::Matrix<float>* SIFT_descriptors_matrix_ptr;
   flann::Matrix<float>* SIFT_descriptors2_matrix_ptr;
   flann::Matrix<float>* cluster_centers_matrix_ptr;
   flann::Matrix<float>* new_cluster_centers_matrix_ptr;

   unsigned int n_queries_times_nn;
   int* int_array_ptr;
   float* float_array_ptr;
   flann::Matrix<int>* indices_ptr;
   flann::Matrix<float>* dists_ptr;
   flann::Index<flann::L2<float> > *cluster_index_nn_ptr,
      *forward_nn_ptr,*backward_nn_ptr;

   std::vector<flann::Matrix<float>* > SIFT_descriptor_matrices_ptrs;

   typedef  flann::Index<flann::L2<float> > RANDOMIZED_KDTREE_INDEX;
   std::vector< RANDOMIZED_KDTREE_INDEX* > kdtree_index_ptrs;

   
   std::vector<std::pair<int,int> > initial_SIFT_matches;
   typedef std::map<int,int> FEATURE_MATCHES_MAP;
   FEATURE_MATCHES_MAP *forward_matches_map_ptr,*backward_matches_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const akm& a);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void akm::set_FLANN_flag(bool flag)
{
   FLANN_flag=flag;
}

inline void akm::set_SIFT_descriptors(float* SIFT_descriptors)
{
   this->SIFT_descriptors=SIFT_descriptors;
}

inline void akm::set_SIFT_descriptors_matrix_ptr(
   flann::Matrix<float>* matrix_ptr)
{
   SIFT_descriptors_matrix_ptr=matrix_ptr;
}

inline void akm::set_SIFT_descriptors2_matrix_ptr(
   flann::Matrix<float>* matrix_ptr)
{
   SIFT_descriptors2_matrix_ptr=matrix_ptr;
}

inline void akm::set_cluster_centers_matrix_ptr(
   flann::Matrix<float>* matrix_ptr)
{
   cluster_centers_matrix_ptr=matrix_ptr;
}

inline void akm::set_inverse_sqrt_covar_matrix_ptr(
   genmatrix* inverse_sqrt_covar_ptr)
{
   inverse_sqrt_covar_matrix_ptr=inverse_sqrt_covar_ptr;
}

inline double akm::get_mu_n_features_per_cluster() const
{
   return mu_n_features_per_cluster;
}

inline double akm::get_sigma_n_features_per_cluster() const
{
   return sigma_n_features_per_cluster;
}

inline int akm::get_K() const
{
   return K;
}

inline std::vector<std::pair<int,int> >* akm::get_initial_SIFT_matches_ptr()
{
   return &(initial_SIFT_matches);
}

inline const std::vector<std::pair<int,int> >* 
   akm::get_initial_SIFT_matches_ptr() const
{
   return &(initial_SIFT_matches);
}

#endif  // akm.h
