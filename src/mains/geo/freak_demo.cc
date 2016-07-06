// =======================================================================
// Progrma FREAK_DEMO is a playground for experimenting with the FREAK
// feature detector which is implemented in OpenCV 2.4.
// =======================================================================
// Last updated on 9/26/12; 9/28/12
// =======================================================================

//  demo.cpp
//
// Here is an example on how to use the descriptor presented in the
// following paper:
// A. Alahi, R. Ortiz, and P. Vandergheynst. FREAK: Fast Retina
// Keypoint. In IEEE Conference on Computer Vision and Pattern
// Recognition, 2012.
//
//	Copyright (C) 2011-2012  Signal processing laboratory 2, EPFL,
//	Kirell Benzi (kirell.benzi@epfl.ch),
//	Raphael Ortiz (raphael.ortiz@a3.epfl.ch),
//	Alexandre Alahi (alexandre.alahi@epfl.ch)
//	and Pierre Vandergheynst (pierre.vandergheynst@epfl.ch)

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <opencv2/features2d/features2d.hpp>

#include "general/filefuncs.h"
#include "video/RGB_analyzer.h"
#include "video/sift_detector.h"
#include "video/texture_rectangle.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table();

   texture_rectangle* texture_rectangle_1_ptr=new texture_rectangle();
   texture_rectangle* texture_rectangle_2_ptr=new texture_rectangle();

   bool FLANN_flag=true;
   sift_detector SIFT(photogroup_ptr,FLANN_flag);

   
   string sift_keys_subdir="./sift_keys/";
   SIFT.extract_SIFT_features(sift_keys_subdir);

   const int n_min_quadrant_features=1;		
   double max_ratio=0.7;
   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;
   double max_scalar_product=0.001;

   SIFT.identify_candidate_feature_matches_via_fundamental_matrix(
      n_min_quadrant_features,sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product);

   fundamental* fundamental_ptr=SIFT.get_fundamental_ptr();
   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();

   cout.precision(12);
   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;
   cout << "fundamental rank = " << F_ptr->rank() << endl;
   cout << "Det(F) = " << F_ptr->determinant() << endl;

// Next extract and rudimentarily match FREAK features:

   vector<cv::KeyPoint> keypoints1,keypoints2;
   vector<cv::DMatch> matches;

   photograph* photo1_ptr=photogroup_ptr->get_photograph_ptr(0);
   photograph* photo2_ptr=photogroup_ptr->get_photograph_ptr(1);
   string image1_filename=photo1_ptr->get_filename();
   string image2_filename=photo2_ptr->get_filename();
//   cout << "image1_filename = " << image1_filename << endl;
//   cout << "image2_filename = " << image2_filename << endl;

   SIFT.raw_match_OpenCV_FREAK_features(
      image1_filename,image2_filename,keypoints1,keypoints2,matches);
   SIFT.color_prune_FREAK_matches(
      keypoints1,keypoints2,matches,RGB_analyzer_ptr,
      image1_filename,image2_filename,
      texture_rectangle_1_ptr,texture_rectangle_2_ptr);
   SIFT.append_FREAK_inliers(
      keypoints1,keypoints2,3*max_scalar_product);

   SIFT.rename_feature_IDs(0,1);
   SIFT.export_feature_tracks(0);
   SIFT.export_feature_tracks(1);

   delete RGB_analyzer_ptr;
   delete texture_rectangle_1_ptr;
   delete texture_rectangle_2_ptr;

// Export matched features to output html file:

   FeaturesGroup* FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(videopass_ID),NULL);

   string features_subdir="./features/";
   FeaturesGroup_ptr->read_in_photo_features(photogroup_ptr,features_subdir);
   bool output_only_multicoord_features_flag=true;
   FeaturesGroup_ptr->write_feature_html_file(
      photogroup_ptr,output_only_multicoord_features_flag);


    
}
