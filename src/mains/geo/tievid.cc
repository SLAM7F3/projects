// =======================================================================
// Program TIEVID extracts 2D tiepoint pairs using Lowe's SIFT
// binary and the OpenCV FREAK detector.  It also returns the 4
// closest features within the XY image plane to entered sets of X & Y
// coordinates.
// =======================================================================
// Last updated on 11/12/12; 12/2/12; 12/3/12; 12/16/12
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <opencv2/features2d/features2d.hpp>

#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "math/lttwovector.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/RGB_analyzer.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "graphs/vptree.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

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
   image_matcher SIFT(photogroup_ptr,FLANN_flag);

   string sift_keys_subdir="./sift_keys/";
   string features_subdir="./features/";
   filefunc::dircreate(features_subdir);

// First extract conventional SIFT features via Lowe's binary:

   SIFT.extract_SIFT_features(sift_keys_subdir);

   const int n_min_quadrant_features=1;		
   double max_ratio=0.7;
   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;
   double max_scalar_product=0.001;

   SIFT.identify_candidate_feature_matches_via_fundamental_matrix(
      n_min_quadrant_features,sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product);

// Locate OpenCV keypoints using multiple detectors (sift, surf,
// star, mser and GFTT):

   vector<cv::KeyPoint> keypoints1,keypoints2;

   int i=0;
   int j=1;

   photograph* photo1_ptr=photogroup_ptr->get_photograph_ptr(i);
   photograph* photo2_ptr=photogroup_ptr->get_photograph_ptr(j);
   string image1_filename=photo1_ptr->get_filename();
   string image2_filename=photo2_ptr->get_filename();
//   cout << "image1_filename = " << image1_filename << endl;
//   cout << "image2_filename = " << image2_filename << endl;

   SIFT.detect_OpenCV_keypoints(
      image1_filename,image2_filename,keypoints1,keypoints2);

/*
// Extract Harris corners:

   double R_min;
   cout << "Enter threshold R_min:" << endl;
   cin >> R_min;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* edges_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* corners_texture_rectangle_ptr=new texture_rectangle();

   twoDarray* xderiv_twoDarray_ptr=NULL;
   twoDarray* yderiv_twoDarray_ptr=NULL;
   twoDarray* xxderiv_twoDarray_ptr=NULL;
   twoDarray* xyderiv_twoDarray_ptr=NULL;
   twoDarray* yyderiv_twoDarray_ptr=NULL;

   vector<cv::KeyPoint> corner_keypoints1=SIFT.extract_harris_corners(
      R_min,image1_filename,texture_rectangle_ptr,
      edges_texture_rectangle_ptr,corners_texture_rectangle_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      xxderiv_twoDarray_ptr,xyderiv_twoDarray_ptr,yyderiv_twoDarray_ptr);

   string corners_filename="corners1.jpg";
   corners_texture_rectangle_ptr->write_curr_frame(corners_filename);

   vector<cv::KeyPoint> corner_keypoints2=SIFT.extract_harris_corners(
      R_min,image2_filename,texture_rectangle_ptr,
      edges_texture_rectangle_ptr,corners_texture_rectangle_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      xxderiv_twoDarray_ptr,xyderiv_twoDarray_ptr,yyderiv_twoDarray_ptr);

   corners_filename="corners2.jpg";
   corners_texture_rectangle_ptr->write_curr_frame(corners_filename);

   delete texture_rectangle_ptr;
   delete edges_texture_rectangle_ptr;
   delete corners_texture_rectangle_ptr;

   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   delete xxderiv_twoDarray_ptr;
   delete xyderiv_twoDarray_ptr;
   delete yyderiv_twoDarray_ptr;

   for (int i=0; i<corner_keypoints1.size(); i++)
   {
      keypoints1.push_back(corner_keypoints1[i]);
   }
   for (int i=0; i<corner_keypoints2.size(); i++)
   {
      keypoints2.push_back(corner_keypoints2[i]);
   }

   cout << "keypoints1.size() = " << keypoints1.size()
        << " keypoints2.size() = " << keypoints2.size() << endl;
*/

// Next extract and rudimentarily match FREAK descriptors at 2D OpenCV
// keypoints:

   cout << "Extracting FREAK features at OpenCV keypoints:" << endl;

   vector<cv::DMatch> matches;
   SIFT.raw_match_OpenCV_FREAK_features(
      image1_filename,image2_filename,keypoints1,keypoints2,matches);
   SIFT.color_prune_FREAK_matches(
      keypoints1,keypoints2,matches,RGB_analyzer_ptr,
      image1_filename,image2_filename,
      texture_rectangle_1_ptr,texture_rectangle_2_ptr);
   SIFT.append_tiepoint_inliers(
      i,j,keypoints1,keypoints2,max_scalar_product);

   SIFT.compute_inlier_fundamental_matrix(0);

   SIFT.rename_feature_IDs(i,j);
   SIFT.export_feature_tracks(i);
   SIFT.export_feature_tracks(j);

   delete RGB_analyzer_ptr;
   delete texture_rectangle_1_ptr;
   delete texture_rectangle_2_ptr;

// Export matched features to output html file:

   FeaturesGroup* FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(videopass_ID),NULL);

   FeaturesGroup_ptr->read_in_photo_features(photogroup_ptr,features_subdir);
   bool output_only_multicoord_features_flag=true;
   FeaturesGroup_ptr->write_feature_html_file(
      photogroup_ptr,output_only_multicoord_features_flag);

// Return closest features within XY image plane corresponding to some
// entered set of X & Y coordinates:

   SIFT.recover_inlier_tiepoints();
   SIFT.fill_feature_coords_ID_maps();
   SIFT.generate_VP_trees();

   while (true)
   {
      double X,Y;
      cout << "Enter x coordinate:" << endl;
      cin >> X;
      cout << "Enter y coordinate:" << endl;
      cin >> Y;
      SIFT.find_nearest_XY_features(X,Y);
   }


}

