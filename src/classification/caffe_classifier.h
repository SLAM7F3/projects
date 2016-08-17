// ==========================================================================
// Header file for caffe_classifier class 
// ==========================================================================
// Last modified on 8/1/16; 8/2/16; 8/16/16; 8/17/16
// ==========================================================================

#ifndef CAFFE_CLASSIFIER_H
#define CAFFE_CLASSIFIER_H

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <string>
#include <vector>

#include "color/colorfuncs.h"

class texture_rectangle;

class caffe_classifier
{   
  public:

// Initialization, constructor and destructor functions:

   caffe_classifier(
      const std::string& deploy_prototxt_filename,
      const std::string& trained_caffe_model_filename);

   ~caffe_classifier();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const caffe_classifier& dt);

// Set & get member functions:

   void set_segmentation_flag(bool flag);
   void set_mean_bgr(double bmean, double gmean, double rmean);
   void add_label(std::string curr_label);
   std::string get_label_name(int n);
   int get_classification_result() const;
   double get_classification_score() const;
   unsigned int get_n_labels() const;
   std::vector<int>& get_n_param_layer_nodes();
   const std::vector<int>& get_n_param_layer_nodes() const;

   void rgb_img_to_bgr_fvec(texture_rectangle& curr_img);
   void generate_dense_map();
   void generate_dense_map_data_blob();
   std::string display_segmentation_labels(texture_rectangle& curr_img,
                                           std::string output_subdir);
   std::string display_segmentation_scores(texture_rectangle& curr_img,
                                           std::string output_subdir);
   void retrieve_layer_activations(std::string blob_name);
   void cleanup_memory();

  private: 

   bool segmentation_flag;
   int num_data_channels_;
   int input_img_xdim, input_img_ydim;
   int classification_result;
   double classification_score;
   std::string test_prototxt_filename, trained_caffe_model_filename;
   std::vector<int> n_param_layer_nodes;
   std::vector<std::string> n_param_layer_names;

   caffe::shared_ptr<caffe::Net<float> > net_;
   cv::Size input_geometry_;
   cv::Mat mean_;
   std::vector<std::string> labels_;
   colorfunc::RGB mean_bgr;
   float *feature_descriptor;
   texture_rectangle *label_tr_ptr, *score_tr_ptr;
   texture_rectangle *cc_tr_ptr;


   void load_trained_network();
   void print_network_metadata();

   void retrieve_classification_results(const caffe::Blob<float>* result_blob);
   void export_segmentation_mask(const caffe::Blob<float>* result_blob);

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const caffe_classifier& C);


// Deprecated member functions:

//   void SetMean(const std::string& mean_file);
//   std::vector<float> Predict(const cv::Mat& img);
//   void WrapInputLayer(std::vector<cv::Mat>* input_channels);
//   void Preprocess(const cv::Mat& img,
//                   std::vector<cv::Mat>* input_channels);
//   std::vector<std::pair<std::string, float> > Classify(
//      const cv::Mat& img, int N = 5);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void caffe_classifier::set_segmentation_flag(bool flag)
{
   segmentation_flag = flag;
}

inline void caffe_classifier::set_mean_bgr(
   double bmean, double gmean, double rmean)
{
   mean_bgr.first = bmean;
   mean_bgr.second = gmean;
   mean_bgr.third = rmean;
}

inline void caffe_classifier::add_label(std::string curr_label)
{
   labels_.push_back(curr_label);
}

inline std::string caffe_classifier::get_label_name(int n)
{
   return labels_.at(n);
}

inline int caffe_classifier::get_classification_result() const
{
   return classification_result;
}

inline double caffe_classifier::get_classification_score() const
{
   return classification_score;
}

inline unsigned int caffe_classifier::get_n_labels() const
{
   return labels_.size();
}

inline std::vector<int>& caffe_classifier::get_n_param_layer_nodes()
{
   return n_param_layer_nodes;
}

inline const std::vector<int>& caffe_classifier::get_n_param_layer_nodes() const
{
   return n_param_layer_nodes;
}



#endif  // caffe_classifier.h


