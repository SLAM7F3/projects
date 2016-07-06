// =======================================================================
// Program SIFTMATCH_SUCCESSIVE_FRAMES imports a set of image files.
// It extracts SIFT/ASIFT features via calls to the fast SIFT (affine
// SIFT) library.  SIFTMATCH_SUCCESSIVE_FRAMES performs tiepoint
// matching via fundamental matrix estimation and RANSAC on the
// consolidated sets of image features.  Successive image tiepoints
// and feature tracks labeled by unique IDs are exported to output
// text files.

/*

./siftmatch_successive_frames \
--newpass ./bundler/Korea/NK/images/frame-00001.jpg \
--newpass ./bundler/Korea/NK/images/frame-00002.jpg \
--newpass ./bundler/Korea/NK/images/frame-00003.jpg \
--newpass ./bundler/Korea/NK/images/frame-00004.jpg \
--newpass ./bundler/Korea/NK/images/frame-00005.jpg \
--image_list_filename ./bundler/Korea/NK/image_list.dat 

*/

// =======================================================================
// Last updated on 11/2/13; 11/3/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cluster/akm.h"
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
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

   bool serial_matching_flag=true;
//   bool serial_matching_flag=false;


   string ImageEngine_subdir="/data/ImageEngine/";

//   string root_subdir=ImageEngine_subdir+"NewsWrap/";
//   string root_subdir=ImageEngine_subdir+
//      "BostonBombing/Nightline_YouTube2/transcripted/";
//   string root_subdir=
//      "/data/ImageEngine/BostonBombing/clips_1_thru_133/clip34/";
//   string root_subdir=ImageEngine_subdir+"BostonBombing/clip3/";
//   string root_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";
//   string images_subdir=root_subdir;
//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";


//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";


   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(images_subdir);
   int n_images=photogroup_ptr->get_n_photos();

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

   string features_subdir=root_subdir+"SIFT_features/";
   filefunc::dircreate(features_subdir);

   timefunc::initialize_timeofday_clock();

// --------------------------------------------------------------------------
// Feature extraction starts here:

// Extract conventional SIFT features from each input image via
// Lowe's binary:

   string sift_keys_subdir=root_subdir+"sift_keys/";
//    filefunc::purge_files_in_subdir(sift_keys_subdir);

   bool delete_pgm_file_flag=false;
   SIFT.extract_SIFT_features(
      sift_keys_subdir,delete_pgm_file_flag);
//      image_filenames,sift_keys_subdir,delete_pgm_file_flag);

//   SIFT.set_num_threads(6);
//   SIFT.parallel_extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   outputfunc::print_elapsed_time();

   string asift_keys_subdir=root_subdir+"asift_keys/";
//   filefunc::purge_files_in_subdir(asift_keys_subdir);
//   SIFT.extract_ASIFT_features(asift_keys_subdir);

// On 5/22/13, observed nontrivial numbers of matching errors for
// kermit0-2 when ASIFT features are extracted in parallel!

//   SIFT.set_num_threads(2);
//   SIFT.parallel_extract_ASIFT_features(asift_keys_subdir);
   outputfunc::print_elapsed_time();

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

// --------------------------------------------------------------------------
// Feature matching starts here:

   string tiepoints_subdir=root_subdir+"tiepoints/";
   filefunc::dircreate(tiepoints_subdir);
   string ntiepoints_filename=tiepoints_subdir+"ntiepoints.dat";

   bool match_features_flag=!(filefunc::fileexist(ntiepoints_filename));
//   if (match_features_flag)
   {
      ofstream ntiepoints_stream;
      filefunc::openfile(ntiepoints_filename,ntiepoints_stream);
      ntiepoints_stream 
         << "# Image i Image j N_tiepoints N_duplicates sensor_separation_angle nonzero_tiepoint_blocks_frac filename_i   filename_j"
         << endl << endl;

//   double max_ratio=0.6;    	// OK for Kermit images
//   double max_ratio=0.7;    	// OK for GEO, Newswrap, Kermit images
      double max_ratio=0.725;    	// OK for GEO, Newswrap, Kermit images
//   double max_ratio=0.75;    	// OK for GEO, Newswrap, Kermit images
      cout << "Enter max Lowe ratio:" << endl;
//   cin >> max_ratio;

      double sqrd_max_ratio=sqr(max_ratio);
      double worst_frac_to_reject=0;

//   double max_scalar_product=1E-2;
//   double max_scalar_product=3E-3;
      double max_scalar_product=1E-3;

//   double max_scalar_product=5E-5;
//   double max_scalar_product=2E-5;
//   double max_scalar_product=1E-5;
      cout << "Enter max scalar product:" << endl;
//   cin >> max_scalar_product;

//   int max_n_good_RANSAC_iters=200;
      int max_n_good_RANSAC_iters=500;
//   cout << "Enter max_n_good_RANSAC_iters:" << endl;
//   cin >> max_n_good_RANSAC_iters;

      int min_n_features=10;
      int minimal_number_of_inliers=8;

// Match SIFT & ASIFT features across pairs of successive images:

      for (int i=0; i<n_images-1; i++)
      {
         SIFT.match_successive_image_features(
            i,i+1,sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_n_features,minimal_number_of_inliers,
            root_subdir,map_unionfind_ptr,ntiepoints_stream);
      } // loop over index i labeling input images

      SIFT.rename_feature_IDs(map_unionfind_ptr);

      int n_feature_tracks=0;
      for (int i=0; i<n_images; i++)
      {
         n_feature_tracks += SIFT.export_feature_tracks(i,features_subdir);
      }
      cout << "n_feature_tracks = " << n_feature_tracks << endl;

   } // match_feature_flag conditional
   delete map_unionfind_ptr;   

   cout << "At end of program SIFTMATCH_SUCCESSIVE_FRAMES" << endl;
   outputfunc::print_elapsed_time();
}
