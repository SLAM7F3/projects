// Note added on 5/9/13: Need to move PNG and PGM files in
// bundler_IO_subdir/images to their own subdirs!

// =======================================================================
// Program RESTRICTED_ASIFT is a variant of program ASIFTVID.  It
// performs expensive feature matching only between pairs of aerial
// video frames whose hardware estimates for camera orientations
// indicate that the separation angle between image planes is less
// than some threshold value.

// Need to move PNG and PGM files in bundler_IO_subdir/images/ into
// their own newly created subdirs.

// =======================================================================
// Last updated on 6/2/13; 6/13/13; 6/14/13
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
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

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
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   vector<camera*> camera_ptrs;
   for (int n=0; n<n_images; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
   }

// We explicitly confirmed on 1/30/13 that the FLANN library yields
// noticeably better feature matching results than the older ANN
// library:

   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);
   SIFT.set_sampson_error_flag(true);
   SIFT.set_hamming_distance_comparison_flag(true);
//   SIFT.set_max_n_features_to_consider_per_image(10000);

// Note added on 2/10/13: "root-SIFT" matching appears to yield
// inferior results for Affine-SIFT features than conventional "SIFT"
// matching !

   SIFT.set_root_sift_matching_flag(false);
//   SIFT.set_root_sift_matching_flag(true);

   string features_subdir=bundler_IO_subdir+"features/";
   filefunc::dircreate(features_subdir);

   timefunc::initialize_timeofday_clock();

// --------------------------------------------------------------------------
// Feature extraction starts here:

// Extract conventional SIFT features from each input image via
// Lowe's binary:

   string sift_keys_subdir=bundler_IO_subdir+"sift_keys/";
   bool delete_pgm_file_flag=false;
//   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   SIFT.set_num_threads(9);
   SIFT.parallel_extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   outputfunc::print_elapsed_time();

// In Feb 2013, we empirically found that the Oxford feature detectors
// yielded the following number of tiepoint matches for two pairs of
// Pass 15 GEO video frames:

/*
// 2548; 2788
   string haraff_keys_subdir=bundler_IO_subdir+"haraff_keys/";
   SIFT.extract_Oxford_features(
      "haraff",haraff_keys_subdir,delete_pgm_file_flag);

// 2722; 2944
   string harlap_keys_subdir=bundler_IO_subdir+"harlap_keys/";
   SIFT.extract_Oxford_features(
      "harlap",harlap_keys_subdir,delete_pgm_file_flag);

// 3147; 3319
   string heslap_keys_subdir=bundler_IO_subdir+"heslap_keys/";
   SIFT.extract_Oxford_features(
      "heslap",heslap_keys_subdir,delete_pgm_file_flag);
*/

/*
// 5179; 5405
   string harhes_keys_subdir=bundler_IO_subdir+"harhes_keys/";
   SIFT.extract_Oxford_features(
      "harhes",harhes_keys_subdir,delete_pgm_file_flag);
*/

/*
// 6455; 6336
   string sedgelap_keys_subdir=bundler_IO_subdir+"sedgelap_keys/";
   delete_pgm_file_flag=true;
   SIFT.extract_Oxford_features(
      "sedgelap",sedgelap_keys_subdir,delete_pgm_file_flag);
*/

   string asift_keys_subdir=bundler_IO_subdir+"asift_keys/";
   SIFT.extract_ASIFT_features(asift_keys_subdir);

   outputfunc::print_elapsed_time();

// Export consolidated sets of SIFT & ASIFT features to output keyfiles:

   string all_keys_subdir=bundler_IO_subdir+"all_keys/";
   filefunc::dircreate(all_keys_subdir);

   for (int i=0; i<n_images; i++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(i);

      string basename=filefunc::getbasename(photo_ptr->get_filename());
      string prefix=stringfunc::prefix(basename);
      string all_keys_filename=all_keys_subdir+prefix+".key";

      cout << "all_keys_filename = " << all_keys_filename << endl;
      if (filefunc::fileexist(all_keys_filename))
      {
         cout << "all_keys_filename = " << all_keys_filename
              << " already exists in " << all_keys_subdir << endl;
      }
      else
      {
         SIFT.export_features_to_Lowe_keyfile(
            photo_ptr->get_ydim(),all_keys_filename,
            SIFT.get_image_feature_info(i));
         string banner="Exported "+all_keys_filename;
         outputfunc::write_banner(banner);
      }
   } // loop over index i labeling images

// Note added on 5/31/13: Inputs to binaryfunc::hamming_distance
// should be BINARY descriptors!!  Yet we CANT afford to hold copies
// of binary descriptors within member Dbinary_ptrs_ptr for large
// number of images!!  So we suspend all Hamming distance computations
// as of 5/31/13:

   SIFT.compute_descriptor_component_medians();
   SIFT.binary_quantize_SIFT_descriptors();

// --------------------------------------------------------------------------
// Feature matching starts here:

   string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
   filefunc::dircreate(tiepoints_subdir);
   string ntiepoints_filename=tiepoints_subdir+"ntiepoints.dat";
   ofstream ntiepoints_stream;
   filefunc::openfile(ntiepoints_filename,ntiepoints_stream);
   ntiepoints_stream 
      << "# Image i Image j N_tiepoints N_duplicates sensor_separation_angle filename_i   filename_j"
      << endl << endl;

   int istart=0;
   int istop=n_images;

//   const double max_camera_angle_separation=45*PI/180;
   const double max_camera_angle_separation=60*PI/180;
//   const double max_camera_angle_separation=75*PI/180;
//   const double max_camera_angle_separation=82*PI/180;
//   const double max_camera_angle_separation=90*PI/180;

   const int n_min_quadrant_features=1;		
//   double max_ratio=0.675;
   double max_ratio=0.7;
//   double max_ratio=0.725;
   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;
//   double max_scalar_product=0.001;
//   double max_scalar_product=0.000333;
//   double max_scalar_product=1E-4;
//   double max_scalar_product=8E-5;
//   double max_scalar_product=2E-5;
   double max_scalar_product=1E-5;
//   double max_scalar_product=2E-6;
//   double max_scalar_product=1E-6;

//   int max_n_good_RANSAC_iters=100;
   int max_n_good_RANSAC_iters=200;

// On 5/3/13, processing 57 frames from GEO/20120105_1402
// yielded n_feature_tracks=142926 in 16.37 minutes on BEAST
// with max_ratio=0.7, max_scalar_product=1E-5, 
// max_n_good_RANSAC_iters=200
// FLANN parameters: fsp.checks=96, fsp.cores=0, flann::KDTreeIndexParams(4).  
// Qualitatively reasonable reconstruction w/ var focal params.


// On 4/30/13, processing 57 frames from GEO/20120105_1402
// yielded n_feature_tracks=139620 in 19.91 minutes on BEAST
// with max_ratio=0.7, max_scalar_product=1E-5, 
// max_n_good_RANSAC_iters=200
// FLANN parameters: fsp.checks=128, fsp.cores=0, flann::KDTreeIndexParams(4).  
// n_reconstructed_3D_pnts = 16866 in 1.4 mins
// Acceptable (though flawed) initial reconstruction w/o variable
// focal lenghts.  Qualitatively reasonable reconstruction w/ variable
// focal params.

// On 4/30/13, processing 57 frames from GEO/20120105_1402
// yielded n_feature_tracks=145371 in 13.48 minutes on BEAST
// with max_ratio=0.7, max_scalar_product=1E-5, 
// max_n_good_RANSAC_iters=200
// FLANN parameters: fsp.checks=64, fsp.cores=0, flann::KDTreeIndexParams(4).  

// n_reconstructed_3D_pnts = 17927 in 1.57 mins
// Poor reconstruction!

   int feature_track_ID=0;
   sift_detector::FEATURE_TRACK_IDS_MAP feature_track_ids_map;
   sift_detector::IMAGE_VS_FEATURES_MAP image_vs_features_map;

   for (int i=istart; i<istop; i++)
   {
      int j_start=i+1;
      int j_stop=n_images;

      SIFT.restricted_match_image_pair_features(
         i,j_start,j_stop,camera_ptrs,
         photogroup_ptr,max_camera_angle_separation,
         sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
         max_n_good_RANSAC_iters,bundler_IO_subdir,
         feature_track_ID,feature_track_ids_map,image_vs_features_map,
         ntiepoints_stream);

//      SIFT.set_num_threads(9);
//      SIFT.parallel_restricted_match_image_pair_features(
//         i,j_start,j_stop,camera_ptrs,photogroup_ptr,
//         max_camera_angle_separation,sqrd_max_ratio,
//         worst_frac_to_reject,max_scalar_product,
//         max_n_good_RANSAC_iters,bundler_IO_subdir,
//         feature_track_ID,feature_track_ids_map,image_vs_features_map,
//         ntiepoints_stream);

      SIFT.export_feature_tracks_for_image(
         i,features_subdir,image_vs_features_map);

   } // loop over index i labeling input images

   filefunc::closefile(ntiepoints_filename,ntiepoints_stream);

   outputfunc::print_elapsed_time();
   int n_feature_tracks=feature_track_ids_map.size();
   cout << "n_feature_tracks = " << n_feature_tracks << endl;
   cout << "At end of restricted_asift.cc" << endl;
   outputfunc::print_elapsed_time();
}
