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
// Last updated on 7/23/13; 7/27/13; 7/30/13; 8/25/13
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
#include "datastructures/map_unionfind.h"
#include "messenger/Messenger.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "image/pngfuncs.h"
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

   bool demo_flag=true;
   if (demo_flag)
   {
//      outputfunc::display_flow_diagram(
//         "./demo_flow_diagrams/","flow_diag_2.png");

      string broker_URL = "tcp://127.0.0.1:61616";
      string message_queue_channel_name="127.0.0.1";
      string message_sender_ID="MESSAGE_SENDER";
      bool include_sender_and_timestamp_info_flag=false;
      Messenger m(broker_URL,message_queue_channel_name,message_sender_ID,
                  include_sender_and_timestamp_info_flag);

      string command="DISPLAY_NEXT_FLOW_DIAGRAM";
      m.sendTextMessage(command);
   }
   timefunc::initialize_timeofday_clock(); 

   string matching_char;
   cout << "Enter 's' for serial feature matching or 'p' for parallel feature matching" << endl;
//   cin >> matching_char;

//   bool serial_matching_flag=true;
   bool serial_matching_flag=false;
//   if (matching_char=="p") serial_matching_flag=false;

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
   SIFT.set_forward_feature_matching_flag(true);
//   SIFT.set_forward_feature_matching_flag(false);
//   SIFT.set_max_n_features_to_consider_per_image(10000);

// Note added on 2/10/13: "root-SIFT" matching appears to yield
// inferior results for Affine-SIFT features than conventional "SIFT"
// matching !

   SIFT.set_root_sift_matching_flag(false);
//   SIFT.set_root_sift_matching_flag(true);

   string features_subdir=bundler_IO_subdir+"features/";
   filefunc::dircreate(features_subdir);

// --------------------------------------------------------------------------
// Feature extraction starts here:

// Extract conventional SIFT features from each input image via
// Lowe's binary:

   string sift_keys_subdir=bundler_IO_subdir+"sift_keys/";
   bool delete_pgm_file_flag=false;

   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
//   SIFT.set_num_threads(2);
//   SIFT.set_num_threads(4);
//   SIFT.parallel_extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   outputfunc::print_elapsed_time();

//   string asift_keys_subdir=bundler_IO_subdir+"asift_keys/";
//   SIFT.extract_ASIFT_features(asift_keys_subdir);
//   outputfunc::print_elapsed_time();

// Export consolidated sets of SIFT & ASIFT features to output keyfiles:

   string all_keys_subdir=bundler_IO_subdir+"all_keys/";
   filefunc::dircreate(all_keys_subdir);

   for (int i=0; i<n_images; i++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(i);

      string basename=filefunc::getbasename(photo_ptr->get_filename());
      string prefix=stringfunc::prefix(basename);
      string all_keys_filename=all_keys_subdir+prefix+".key";

//      cout << "all_keys_filename = " << all_keys_filename << endl;
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
         cout << "Exported "+all_keys_filename << endl;
      }
   } // loop over index i labeling images

// Instantiate a map_unionfind object to hold links between
// matching feature IDs across different images.  Then initialize
// *map_unionfind_ptr with nodes corresponding to every feature
// extracted from every image:

   map_unionfind* map_unionfind_ptr=new map_unionfind();

//   cout << "Loading node IDs into *map_unionfind_ptr" << endl;
   for (int i=0; i<n_images; i++)
   {
      SIFT.load_node_IDs(i,map_unionfind_ptr);
   }

//   cout << "Before computing descriptor component medians" << endl;
   SIFT.compute_descriptor_component_medians();
//   cout << "Before binary quantizing SIFT descriptors" << endl;
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

//   const double max_camera_angle_separation=45*PI/180;
   const double max_camera_angle_separation=60*PI/180;
//   const double max_camera_angle_separation=75*PI/180;
//   const double max_camera_angle_separation=82*PI/180;
//   const double max_camera_angle_separation=90*PI/180;

//   double max_Lowe_ratio=0.675;
//   double max_Lowe_ratio=0.68;
//   double max_Lowe_ratio=0.7;  // value as of 7/24
   double max_Lowe_ratio=0.725;	// affine constraint value
   double sqrd_max_Lowe_ratio=sqr(max_Lowe_ratio);
   double worst_frac_to_reject=0;
   double max_scalar_product=1E-3;	// affine constraint value
//   double max_scalar_product=2E-5;
//   double max_scalar_product=1E-5;
//   double max_scalar_product=5E-6;
// 		  double max_scalar_product=1E-6;  // val as of 7/24


//   int max_n_good_RANSAC_iters=200;
//   int max_n_good_RANSAC_iters=300;
//   int max_n_good_RANSAC_iters=350;
//   int max_n_good_RANSAC_iters=400;
   int max_n_good_RANSAC_iters=500;

   int min_candidate_tiepoints=35;
//   int min_candidate_tiepoints=50;
//   int min_candidate_tiepoints=60;

   int minimal_number_of_inliers=25;
//   int minimal_number_of_inliers=50;

// Match SIFT & ASIFT features across image pairs:

   cout << "Before matching SIFT features" << endl;
   int istart=0;
   int istop=n_images-1;
   for (int i=istart; i<=istop; i++)
   {
      int j_start=i+1;
      int j_stop=n_images;

      if (serial_matching_flag)
      {
         SIFT.restricted_match_image_pair_features(
            i,j_start,j_stop,camera_ptrs,
            photogroup_ptr,max_camera_angle_separation,
            sqrd_max_Lowe_ratio,worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_candidate_tiepoints,
            minimal_number_of_inliers,
            bundler_IO_subdir,map_unionfind_ptr,ntiepoints_stream);
      }
      else
      {

// As of 7/6/13, FLANN index construction is NOT thread-safe.
// So FLANN will generally return *different* candidate image tiepoint
// pairs each time we run program RESTRICTED_ASIFT in parallel
// threading mode...

//         bool export_fundamental_matrices_flag=true;
         bool export_fundamental_matrices_flag=false;
         SIFT.set_num_threads(6);
         SIFT.parallel_restricted_match_image_pair_features(
            export_fundamental_matrices_flag,
            i,j_start,j_stop,camera_ptrs,photogroup_ptr,
            max_camera_angle_separation,sqrd_max_Lowe_ratio,
            worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_candidate_tiepoints,
            minimal_number_of_inliers,
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

   cout << "At end of program RESTRICTED_ASIFT" << endl;
   outputfunc::print_elapsed_time();
}
