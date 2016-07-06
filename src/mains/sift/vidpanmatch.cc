// ==========================================================================
// Program VIDPANMATCH reads in a set of input ground digital photos.


// ==========================================================================
// Last updated on 1/18/09; 1/19/09; 1/20/09; 2/7/09
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/sift_detector.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// ************************************************************************
// String constant declarations for video clips:

   string video_filename_prefix="boston_skyline_clip_one_";
   string video_frames_subdir=
//      "/data/EO/ground_photos/2009/Boston/MIT_fieldtrip_Jan6_2009/videos/Boston_skyline/frames/";
      "/media/disk-1/Boston/MIT_fieldtrip_Jan6_2009/videos/Boston_skyline/frames/";
   string output_package_subdir=
      "./packages/Boston_skyline_Jan6_2009/georegistered_video_frames/";

//   string video_filename_prefix="MIT_dome_Oct15_2008_";
//   string video_frames_subdir=
//      "/media/disk-1/Boston/Green_bldg_field_trip_Oct15_2008/video_clips/frames/";
//   string output_package_subdir="./packages/MIT_dome_clip_one/";


//   string video_filename_prefix="lobby7_clip_one_";
//   string video_frames_subdir=
//      "/media/disk-2/Boston/MIT_fieldtrip_Jan6_2009/videos/Lobby7/frames/";
//      "/media/disk-1/Boston/MIT_fieldtrip_Jan6_2009/videos/Lobby7/frames/";
//   string output_package_subdir=
//      "./packages/Lobby7_Jan6_2009/georegistered_lobby7_clip_one/";

// ************************************************************************

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();

   sift_detector SIFT(photogroup_ptr);

// Start loop over video images here:
 
   int video_start_frame=324;
//   int video_start_frame=1300;

//   int video_stop_frame=1000;	

//   int video_stop_frame=1661;	// Jan 6 Lobby video clip one
//   int video_stop_frame=2433; // Oct 15 MIT dome video clip one
   int video_stop_frame=2500; // Jan 6 Boston skyline video clip one
   int video_frame_step=1;

   int i_maximal_overlap=-1;
   photograph* video_image_ptr=photogroup_ptr->get_photograph_ptr(n_photos-1);
   for (int v=video_start_frame; v<=video_stop_frame; v += video_frame_step)
   {
      const int max_digits=4;
      string photograph_number_str=stringfunc::integer_to_string(
         v,max_digits);
      
      string curr_video_filename=video_filename_prefix+photograph_number_str+
         ".png";

      string banner="Processing video frame "+photograph_number_str;
      outputfunc::write_banner(banner);

//      string curr_video_filename="lobby7_clip_one_0068.png";
//      string curr_video_filename="lobby7_clip_one_0368.png";
//      string curr_video_filename="lobby7_clip_one_0868.png";
//      string curr_video_filename="lobby7_clip_one_1268.png";
      string video_pathname=video_frames_subdir+curr_video_filename;
      video_image_ptr->set_filename(video_pathname);

// If v==video_start_frame, extract features for all static panoramic
// plus final dynamic video image.  Otherwise, only extract features
// in current dynamic video image:

      if (v==video_start_frame)
      {
         SIFT.extract_SIFT_features();
      }
      else
      {
         SIFT.destroy_allocated_features_for_specified_image(
            SIFT.get_image_feature_info_ptr(n_photos-1));

         vector<sift_detector::feature_pair>* video_frame_feature_info_ptr=
            SIFT.get_image_feature_info_ptr(n_photos-1);
         video_frame_feature_info_ptr->clear();
//         const double horiz_scale_factor=0.5;
//         const double vert_scale_factor=0.5;
         const double horiz_scale_factor=1.0;
         const double vert_scale_factor=1.0;
         SIFT.extract_SIFT_features(
            video_image_ptr,*video_frame_feature_info_ptr,
            horiz_scale_factor,vert_scale_factor);
      }

//   SIFT.print_features(3);

// Strict parameters OK for Lobby 7 example shot on Jan 6, 2009:

   const int n_min_quadrant_features=10;	// 3D panoramas
   const double sqrd_max_ratio=sqr(0.5);	// 3D panoramas
   const double worst_frac_to_reject=0.33;
   const double max_sqrd_delta=sqr(0.0025);

/*
      const int n_min_quadrant_features=9;	
      const double sqrd_max_ratio=sqr(0.55);      
      const double worst_frac_to_reject=0.25;
      const double max_sqrd_delta=sqr(0.0020);
*/

/*
  const int n_min_quadrant_features=5;		// video/still matching
  const double sqrd_max_ratio=sqr(0.7);	// video/still matching
  const double worst_frac_to_reject=0.10;
  const double max_sqrd_delta=sqr(0.005);
*/
      int jstart=0;
      int jstop=n_photos-2;
      
      pair<int,int> max_feature_matches=
         SIFT.maximal_number_forward_feature_matches(
            n_photos-1,jstart,jstop,sqrd_max_ratio);
      int max_n_feature_matches=max_feature_matches.first;
      i_maximal_overlap=max_feature_matches.second;
      
      cout << "max_n_feature_matches = " << max_n_feature_matches << endl;
      cout << "i_maximal_overlap = " << i_maximal_overlap << endl;

      const double frac_max_n_feature_matches=0.5;
      vector<int> overlapping_images=
         SIFT.identify_overlapping_images(
            n_photos-1,jstart,jstop,max_n_feature_matches,
            frac_max_n_feature_matches,sqrd_max_ratio);
      for (int k=0; k<overlapping_images.size(); k++)
      {
         cout << "k = " << k << " overlapping_images[k] = "
              << overlapping_images[k] << endl;
      }

      SIFT.identify_candidate_feature_ray_matches(
         n_photos-1,overlapping_images,sqrd_max_ratio);
      SIFT.identify_inlier_feature_ray_matches(
         n_min_quadrant_features,worst_frac_to_reject,max_sqrd_delta);

      SIFT.compute_projection_matrix(video_image_ptr);

//      const double frustum_sidelength=180;	// meters  Lobby7 
//      const double downrange_distance=25;	// meters  Lobby7

      const double frustum_sidelength=1000;	// meters  Boston skyline
      const double downrange_distance=500;	// meters  Boston skyline

      SIFT.write_projection_package_file(
         frustum_sidelength,downrange_distance,
         output_package_subdir,video_image_ptr);

   } // loop over index v labeling dynamic video frames

   delete photogroup_ptr;
}

   
