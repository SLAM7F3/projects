// =======================================================================
// Program JUSTSIFT is a hand-altered version of ASIFTVID for matching
// only YouTube clip #2 frames against Nightline frames (with no
// matching within either clip).


// =======================================================================
// Last updated on 6/3/13
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

//   double hours_to_sleep=0.5/60.0;
   double hours_to_sleep=4;
   string sleeping_banner="Sleeping for "+stringfunc::number_to_string(
      hours_to_sleep)+" hours";
   outputfunc::write_big_banner(sleeping_banner);
   sleep(hours_to_sleep*3600);
   
// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;


// We explicitly confirmed on 1/30/13 that the FLANN library yields
// noticeably better feature matching results than the older ANN
// library:

   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);
   SIFT.set_sampson_error_flag(true);

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
//    filefunc::purge_files_in_subdir(sift_keys_subdir);

   bool delete_pgm_file_flag=false;
//   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   SIFT.set_num_threads(6);
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
//    filefunc::purge_files_in_subdir(asift_keys_subdir);
   SIFT.extract_ASIFT_features(asift_keys_subdir);

// On 5/22/13, observed nontrivial numbers of matching errors for
// kermit0-2 when ASIFT features are extracted in parallel!

//   SIFT.set_num_threads(2);
//   SIFT.parallel_extract_ASIFT_features(asift_keys_subdir);
   outputfunc::print_elapsed_time();

// Export consolidated sets of SIFT & ASIFT features to output keyfiles:

   string all_keys_subdir=bundler_IO_subdir+"all_keys/";
   filefunc::dircreate(all_keys_subdir);
//    filefunc::purge_files_in_subdir(all_keys_subdir);

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

/*
// Note added on 5/31/13: Inputs to binaryfunc::hamming_distance
// should be BINARY descriptors!!  Yet we CANT afford to hold copies
// of binary descriptors within member Dbinary_ptrs_ptr for large
// number of images!!  So we suspend all Hamming distance computations
// as of 5/31/13:


   SIFT.compute_descriptor_component_medians();
   SIFT.binary_quantize_SIFT_descriptors();
*/


/*
   SIFT.generate_VPtrees();
//   SIFT.match_binary_SIFT_descriptors();
   outputfunc::enter_continue_char();
*/

// --------------------------------------------------------------------------
// Feature matching starts here:

   string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
   filefunc::dircreate(tiepoints_subdir);
   string ntiepoints_filename=tiepoints_subdir+"ntiepoints.dat";
   ofstream ntiepoints_stream;
   filefunc::openfile(ntiepoints_filename,ntiepoints_stream);
   ntiepoints_stream 
      << "# Image i   Image j   N_tiepoints  N_duplicates filename_i   filename_j"
      << endl << endl;

   int istart=0;	// YouTube clip 2 starting frame
   int istop=108;	// YouTube clip 2 stopping frame

//   double max_ratio=0.6;    	// OK for Kermit images
   double max_ratio=0.65;    	// OK for GEO, Newswrap, Kermit images
//   double max_ratio=0.7;    	// OK for GEO, Newswrap, Kermit images
//   double max_ratio=0.725;    	// OK for GEO, Newswrap, Kermit images
//   double max_ratio=0.75;
//   cout << "Enter max Lowe ratio:" << endl;
//   cin >> max_ratio;

   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;

//   double max_scalar_product=1E-4;
//   double max_scalar_product=8E-5;
//   double max_scalar_product=7.5E-5;
//   double max_scalar_product=5E-5;
//   double max_scalar_product=2E-5;
//   double max_scalar_product=1E-5;
   double max_scalar_product=5E-6;
//   double max_scalar_product=3E-6;
//   cout << "Enter max scalar product:" << endl;
//   cin >> max_scalar_product;

//   int max_n_good_RANSAC_iters=200;
   int max_n_good_RANSAC_iters=500;

   int feature_track_ID=0;
   sift_detector::FEATURE_TRACK_IDS_MAP feature_track_ids_map;
   for (int i=istart; i<istop; i++)
   {

// Match SIFT & ASIFT features across image pairs:

      int j_start=istop+1;
      int j_stop=n_images;

//      SIFT.match_image_pair_features(
//         i,j_start,j_stop,sqrd_max_ratio,
//         worst_frac_to_reject,max_scalar_product,
//         max_n_good_RANSAC_iters,bundler_IO_subdir,
//         feature_track_ID,feature_track_ids_map,
//         ntiepoints_stream);

//      SIFT.set_num_threads(6);
      SIFT.set_num_threads(9);
      SIFT.parallel_match_image_pair_features(
         i,j_start,j_stop,sqrd_max_ratio,
         worst_frac_to_reject,max_scalar_product,
         max_n_good_RANSAC_iters,bundler_IO_subdir,
         feature_track_ID,feature_track_ids_map,
         ntiepoints_stream);

      SIFT.export_feature_tracks(i,features_subdir);

   } // loop over index i labeling input images

   filefunc::closefile(ntiepoints_filename,ntiepoints_stream);

   outputfunc::print_elapsed_time();
   int n_feature_tracks=feature_track_ids_map.size();
   cout << "n_feature_tracks = " << n_feature_tracks << endl;

//   outputfunc::enter_continue_char();

/*
// --------------------------------------------------------------------------
// Export matched features to output html file:

   if (n_feature_tracks > 0)
   {
      FeaturesGroup* FeaturesGroup_ptr=new FeaturesGroup(
         ndims,passes_group.get_pass_ptr(videopass_ID),NULL);
      FeaturesGroup_ptr->read_in_photo_features(
         photogroup_ptr,features_subdir);
//      bool output_only_multicoord_features_flag=false;
      bool output_only_multicoord_features_flag=true;
      FeaturesGroup_ptr->write_feature_html_file(
         photogroup_ptr,output_only_multicoord_features_flag);
      cout << "n_features = " << FeaturesGroup_ptr->get_n_Graphicals() << endl;
      delete FeaturesGroup_ptr;
   }
*/
 
   outputfunc::print_elapsed_time();
}
