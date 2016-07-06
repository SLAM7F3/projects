// =======================================================================
// Program ASIFTVID imports a set of image files.  It extracts SIFT
// and ASIFT features via calls to Lowe's SIFT binary and the affine
// SIFT library.  Consolidated SIFT & ASIFT interest points and
// descriptors are exported to key files following Lowe's conventions.  
// ASIFTVID next performs tiepoint matching via fundamental matrix
// estimation and RANSAC on the consolidated sets of image features.
// Fundamental matrices and feature tracks labeled by unique IDs are
// exported to output text files.  Finally, an html file is generated
// which lists features containing at least 2 tiepoint
// correspondences.

/*

./asiftvid \
--newpass ./bundler/GEO/pass04/images/20110730_110715.193.jpg \
--newpass ./bundler/GEO/pass04/images/20110730_110735.193.jpg \
--newpass ./bundler/GEO/pass04/images/20110730_110755.193.jpg \
--newpass ./bundler/GEO/pass04/images/20110730_110815.193.jpg 

*/

// =======================================================================
// Last updated on 7/10/13; 7/11/13; 7/23/13; 8/25/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <opencv2/features2d/features2d.hpp>

#include "cluster/akm.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "datastructures/map_unionfind.h"
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

   string matching_char;
   cout << "Enter 's' for serial feature matching or 'p' for parallel feature matching" << endl;
   cin >> matching_char;

   bool serial_matching_flag=true;
//   bool serial_matching_flag=false;
   if (matching_char=="p") serial_matching_flag=false;

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
   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
//   SIFT.set_num_threads(6);
//   SIFT.parallel_extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
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
//   filefunc::purge_files_in_subdir(asift_keys_subdir);
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

// Instantiate a map_union_find object to hold links between
// matching feature IDs across different images.  Then initialize
// *map_union_find_ptr with nodes corresponding to every feature
// extracted from every image:

   map_unionfind* map_unionfind_ptr=new map_unionfind();
   for (int i=0; i<n_images; i++)
   {
      SIFT.load_node_IDs(i,map_unionfind_ptr);
   }

   SIFT.compute_descriptor_component_medians();
   SIFT.binary_quantize_SIFT_descriptors();

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
      << "# Image i Image j N_tiepoints N_duplicates sensor_separation_angle filename_i   filename_j"
      << endl << endl;



//   double max_ratio=0.6;    	// OK for Kermit images
//   double max_ratio=0.7;    	// OK for GEO, Newswrap, Kermit images
   double max_ratio=0.725;    	// OK for GEO, Newswrap, Kermit images
//   double max_ratio=0.75;    	// OK for GEO, Newswrap, Kermit images
   cout << "Enter max Lowe ratio:" << endl;
   cin >> max_ratio;

   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;

//   double max_scalar_product=1E-2;
//   double max_scalar_product=3E-3;
   double max_scalar_product=1E-3;

//   double max_scalar_product=5E-5;
//   double max_scalar_product=2E-5;
//   double max_scalar_product=1E-5;
   cout << "Enter max scalar product:" << endl;
   cin >> max_scalar_product;

//   int max_n_good_RANSAC_iters=200;
   int max_n_good_RANSAC_iters=500;
//   cout << "Enter max_n_good_RANSAC_iters:" << endl;
//   cin >> max_n_good_RANSAC_iters;

//   int min_n_features=50;
//   int minimal_number_of_inliers=40;

   int min_n_features=10;
   int minimal_number_of_inliers=8;

// Match SIFT & ASIFT features across image pairs:

   for (int i=0; i<n_images; i++)
   {
      int j_start=i+1;
      int j_stop=n_images;

      if (serial_matching_flag)
      {
         SIFT.match_image_pair_features(
            i,j_start,j_stop,
            sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_n_features,minimal_number_of_inliers,
            bundler_IO_subdir,map_unionfind_ptr,ntiepoints_stream);
      }
      else
      {

// As of 7/6/13, FLANN index construction is NOT thread-safe.
// So FLANN will generally return *different* candidate image tiepoint
// pairs each time we run program ASIFTVID in parallel threading mode...

         bool export_fundamental_matrices_flag=true;
         SIFT.set_num_threads(6);
         SIFT.parallel_match_image_pair_features(
            export_fundamental_matrices_flag,
            i,j_start,j_stop,
            sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_n_features,minimal_number_of_inliers,
            bundler_IO_subdir,map_unionfind_ptr,ntiepoints_stream);
      }

   } // loop over index i labeling input images

   if (!serial_matching_flag)
      SIFT.reorder_parallelized_tiepoints_file(bundler_IO_subdir);

   SIFT.rename_feature_IDs(map_unionfind_ptr);

   int n_feature_tracks=0;
   for (int i=0; i<n_images; i++)
   {
      n_feature_tracks += SIFT.export_feature_tracks(i,features_subdir);
   }
   cout << "n_feature_tracks = " << n_feature_tracks << endl;
   
   delete map_unionfind_ptr;
   outputfunc::print_elapsed_time();
//   outputfunc::enter_continue_char();

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

   cout << "At end of program ASIFTVID" << endl;
   outputfunc::print_elapsed_time();
}
