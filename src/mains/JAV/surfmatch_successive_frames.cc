// =======================================================================
// Program SURFMATCH_SUCCESSIVE_FRAMES imports a set of image files.
// It imports SURF features previously extracted via program
// EXTRACT_SURF_FEATURES.  SURFMATCH_SUCCESSIVE_FRAMES performs
// tiepoint matching via fundamental matrix estimation and RANSAC on
// the consolidated sets of image features.  Successive image
// tiepoints and feature tracks labeled by unique IDs are exported to
// output text files.

//			./surfmatch_successive_frames                     

// =======================================================================
// Last updated on 11/5/13; 11/8/13; 11/17/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

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


   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
//   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   string SURF_keys_subdir=root_subdir+"SURF_keys/";

   string tiepoints_subdir=root_subdir+"tiepoints/";
   filefunc::dircreate(tiepoints_subdir);
   string ntiepoints_filename=tiepoints_subdir+"ntiepoints.dat";

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(images_subdir);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

// Determine minimal and maximal clip IDs:

   typedef map<int,vector<pair<int,string> > > CLIP_FRAME_MAP;

// independent int = clip_ID
// dependent STL vector holds pairs of photo_IDs and image filenames

   CLIP_FRAME_MAP clip_frame_map;
   CLIP_FRAME_MAP::iterator clip_frame_iter;

   for (int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      int image_ID=photograph_ptr->get_ID();
      string image_filename=photograph_ptr->get_filename();
      string image_prefix=filefunc::getprefix(image_filename);
      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         image_prefix,separator_chars);
      int clip_ID=stringfunc::string_to_number(substrings[1]);
      int frame_ID=stringfunc::string_to_number(substrings[3]);
      pair<int,string> P(image_ID,image_filename);

      clip_frame_iter=clip_frame_map.find(clip_ID);
      if (clip_frame_iter==clip_frame_map.end())
      {
         vector<pair<int,string> > V;
         V.push_back(P);
         clip_frame_map[clip_ID]=V;
      }
      else
      {
         clip_frame_iter->second.push_back(P);
      }
   } // loop over index i labeling input images

// We explicitly confirmed on 1/30/13 that the FLANN library yields
// noticeably better feature matching results than the older ANN
// library:

   bool FLANN_flag=true;
   image_matcher SIFT(photogroup_ptr,FLANN_flag);
   SIFT.set_sampson_error_flag(true);
   SIFT.set_perform_SOH_corner_angle_test_flag(false);
   SIFT.set_perform_Hamming_distance_test_flag(false);
   SIFT.set_perform_min_descriptor_entropy_test_flag(false);
   
// Note added on 2/10/13: "root-SIFT" matching appears to yield
// inferior results for Affine-SIFT features than conventional "SIFT"
// matching !

   SIFT.set_root_sift_matching_flag(false);
//   SIFT.set_root_sift_matching_flag(true);

   string features_subdir=root_subdir+"SURF_features/";
   filefunc::dircreate(features_subdir);

   timefunc::initialize_timeofday_clock();

// --------------------------------------------------------------------------
// Feature matching starts here:

   ofstream ntiepoints_stream;
   filefunc::openfile(ntiepoints_filename,ntiepoints_stream);
   ntiepoints_stream 
      << "# Image i Image j N_tiepoints N_duplicates sensor_separation_angle nonzero_tiepoint_blocks_frac filename_i   filename_j"
      << endl << endl;

   double max_ratio=0.7;    	// OK for GEO, Newswrap, Kermit images
//   double max_ratio=0.725;    	// SURF match 11/17/13 am
//      cout << "Enter max Lowe ratio:" << endl;
//      cin >> max_ratio;

   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;

//   double max_scalar_product=3E-3;
//   double max_scalar_product=1E-3;  // SURF match 11/17/13 am
   double max_scalar_product=1E-4;
//   double max_scalar_product=5E-5;
//      cout << "Enter max scalar product:" << endl;
//      cin >> max_scalar_product;

//   int max_n_good_RANSAC_iters=200;
   int max_n_good_RANSAC_iters=500;
//   cout << "Enter max_n_good_RANSAC_iters:" << endl;
//   cin >> max_n_good_RANSAC_iters;

   int min_n_features=15;
   int minimal_number_of_inliers=10;
//   int min_n_features=10;
//   int minimal_number_of_inliers=8;

// Import SURF features calculated via program EXTRACT_SURF_FEATURES:

   int counter=0;
   map_unionfind* map_unionfind_ptr=new map_unionfind();
   for (clip_frame_iter=clip_frame_map.begin(); clip_frame_iter != 
           clip_frame_map.end(); clip_frame_iter++)
   {
      double progress_frac=double(counter++)/clip_frame_map.size();
      outputfunc::print_elapsed_and_remaining_time(progress_frac);

      int clip_ID=clip_frame_iter->first;
      vector<pair<int,string> > image_ID_filename=clip_frame_iter->second;
      int n_curr_images=image_ID_filename.size();

      cout << "clip_ID = " << clip_ID 
           << " n_curr_images = " <<  n_curr_images << endl;

// Instantiate a map_union_find object to hold links between
// matching feature IDs across different images.  Then initialize
// *map_union_find_ptr with nodes corresponding to every feature
// extracted from every image:

      map_unionfind_ptr->purgeNodes();
      for (int i=0; i<n_curr_images; i++)
      {
         int image_ID=image_ID_filename[i].first;
         string image_filename=image_ID_filename[i].second;
//         cout << "image_ID = " << image_ID 
//              << " image filename = " << image_filename
//              << endl;

         vector<sift_detector::feature_pair> currimage_feature_info;
         SIFT.import_SURF_features(
            SURF_keys_subdir,image_filename,currimage_feature_info);
         SIFT.add_image_feature_info(image_ID,currimage_feature_info);
         SIFT.load_node_IDs(image_ID,map_unionfind_ptr);
      } // loop over index i labeling images within current clip      

// Match SURF features across pairs of successive images:

      for (int i=0; i<n_curr_images-1; i++)
      {
         int image_ID=image_ID_filename[i].first;
         SIFT.match_successive_image_features(
            image_ID,image_ID+1,
            sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_n_features,minimal_number_of_inliers,
            root_subdir,map_unionfind_ptr,ntiepoints_stream);

         if (image_ID+2 <= n_curr_images-1)
         {
            SIFT.match_successive_image_features(
               image_ID,image_ID+2,
               sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
               max_n_good_RANSAC_iters,min_n_features,
               minimal_number_of_inliers,
               root_subdir,map_unionfind_ptr,ntiepoints_stream);
         }
      } // loop over index i labeling input images

      SIFT.rename_feature_IDs(map_unionfind_ptr);

      int n_feature_tracks=0;
      for (int i=0; i<n_curr_images; i++)
      {
         int image_ID=image_ID_filename[i].first;
         n_feature_tracks += SIFT.export_feature_tracks(
            image_ID,features_subdir);
      }
      cout << "n_feature_tracks = " << n_feature_tracks << endl;

// Delete SURF features for current clip before processing next clip:

      for (int i=0; i<n_curr_images; i++)
      {
         int image_ID=image_ID_filename[i].first;
         SIFT.destroy_image_feature_info(image_ID);
      }

   } // loop over clip_frame_iter
   
   delete map_unionfind_ptr;   

   cout << "At end of program SURFMATCH_SUCCESSIVE_FRAMES" << endl;
   outputfunc::print_elapsed_time();
}
