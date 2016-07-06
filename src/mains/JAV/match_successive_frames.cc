// =======================================================================
// Program MATCH_SUCCESSIVE_FRAMES imports a set of image files.  It
// first retrieves color histograms which are assumed to have been
// precalculated for all the input images.  MATCH_SUCCESSIVE_FRAMES
// computes color histogram dotproducts for successive image pairs.
// It further calculates image 4x4 image sector color histograms.
// Median sector dotproducts between neighboring image pairs are also
// stored.

// This program next extracts SIFT/ASIFT features via calls to the
// fast SIFT (affine SIFT) library.  Consolidated SIFT & ASIFT
// interest points and descriptors are exported to key files following
// Lowe's conventions. MATCH_SUCCESSIVE_FRAMES performs tiepoint
// matching via fundamental matrix estimation and RANSAC on the
// consolidated sets of image features.  Successive image tiepoints
// and feature tracks labeled by unique IDs are exported to output
// text files.  

// Empirically derived feature thresholds are used to define video
// shot boundaries.  Shot boundary detections are exported to an
// output text file.


/*

./match_successive_frames \
--newpass ./bundler/Korea/NK/images/frame-00001.jpg \
--newpass ./bundler/Korea/NK/images/frame-00002.jpg \
--newpass ./bundler/Korea/NK/images/frame-00003.jpg \
--newpass ./bundler/Korea/NK/images/frame-00004.jpg \
--newpass ./bundler/Korea/NK/images/frame-00005.jpg \
--image_list_filename ./bundler/Korea/NK/image_list.dat 

*/

// =======================================================================
// Last updated on 9/5/13; 9/7/13; 9/8/13; 10/8/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cluster/akm.h"
#include "video/descriptorfuncs.h"
#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "datastructures/map_unionfind.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/RGB_analyzer.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
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

   bool serial_matching_flag=true;
//   bool serial_matching_flag=false;

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
   string images_subdir=bundler_IO_subdir+"images/";
   string colorhist_subdir=images_subdir+"color_histograms/";

// Initialize RGB_analyzer and texture_rectangle objects:

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   string liberalized_color="";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(liberalized_color);
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

// Instantiate STL map to hold 4x4 sector color histograms:

   typedef map<int,vector<double>* > SECTOR_COLOR_HISTOGRAMS;
   SECTOR_COLOR_HISTOGRAMS prev_color_sector_histograms;
   SECTOR_COLOR_HISTOGRAMS::iterator iter;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

// Import color histograms for each image:

   string banner="Computing color histograms:";
   outputfunc::write_banner(banner);

   vector<double> prev_color_histogram,curr_color_histogram;
   vector<double> colorhist_dotproducts,median_color_sector_dotproducts;
   for (int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,20,n_images);
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(i);
      string basename=filefunc::getbasename(photo_ptr->get_filename());
      string prefix=stringfunc::prefix(basename);
      string color_histogram_filename=colorhist_subdir+prefix+".color_hist";

      prev_color_histogram.clear();
      double prev_colorhist_norm=0;
      for (int c=0; c<curr_color_histogram.size(); c++)
      {
         prev_color_histogram.push_back(curr_color_histogram[c]);
         prev_colorhist_norm += sqr(prev_color_histogram[c]);
      }
      prev_colorhist_norm=sqrt(prev_colorhist_norm);

      curr_color_histogram=filefunc::ReadInNumbers(color_histogram_filename);

      double color_dotproduct=0;         
      double curr_colorhist_norm=0;
      if (i > 0)
      {
         for (int c=0; c<curr_color_histogram.size(); c++)
         {
//            cout << "c = " << c << " color_hist[c] =  "
//                 << curr_color_histogram[c] << endl;
            curr_colorhist_norm += sqr(curr_color_histogram[c]);
            color_dotproduct += 
               curr_color_histogram[c]*prev_color_histogram[c];
         }
         curr_colorhist_norm=sqrt(curr_colorhist_norm);
         color_dotproduct /= (curr_colorhist_norm*prev_colorhist_norm);
         colorhist_dotproducts.push_back(color_dotproduct);
//         cout << "i = " << i 
//              << " color_dotproduct = " << color_dotproduct
//              << endl;
      }
      else
      {
//         colorhist_dotproducts.push_back(1.0);
      } // i > 0 conditional

// Compute sector color histograms:

      int n_changed_sectors=0;
      vector<double> sector_dotproducts;
      for (int index=0; index<16; index++)
      {
         int row=index/4;
         int column=index%4;
//         cout << "index = " << index << " row = " << row
//              << " column = " << column << endl;
         
         vector<double> sector_color_histogram=
            descriptorfunc::compute_sector_color_histogram(
               4,4,row,column,photo_ptr->get_filename(),texture_rectangle_ptr,
               RGB_analyzer_ptr);

         if (i==0)
         {
            vector<double>* color_sector_histogram_ptr=new vector<double>;
            for (int c=0; c<sector_color_histogram.size(); c++)
            {
               color_sector_histogram_ptr->push_back(
                  sector_color_histogram[c]);
//               cout << "c = " << c << " color_histogram[c] = " 
//                    << color_sector_histogram_ptr->at(c) << endl;
            }
            prev_color_sector_histograms[index]=color_sector_histogram_ptr;
            sector_dotproducts.push_back(1);
         }
         else
         {
            iter=prev_color_sector_histograms.find(index);
            vector<double>* prev_color_histogram_ptr=iter->second;
            
            double dotproduct=0;
            double curr_normsq=0;
            double prev_normsq=0;
            for (int c=0; c<sector_color_histogram.size(); c++)
            {
               double prev_coeff=prev_color_histogram_ptr->at(c);
               double curr_coeff=sector_color_histogram[c];
               (*prev_color_histogram_ptr)[c]=curr_coeff;
               
               dotproduct += curr_coeff*prev_coeff;
               curr_normsq += sqr(curr_coeff);
               prev_normsq += sqr(prev_coeff);
            }
            
            dotproduct /= (sqrt(curr_normsq)*sqrt(prev_normsq));
            sector_dotproducts.push_back(dotproduct);
         } // i==0 conditional

//         cout << "index = " << index << " sector dotproduct = " 
//              << sector_dotproducts.back() << endl;

      } // loop over index labeling 16 sectors within current video frame

      if (i > 0)
      {
         median_color_sector_dotproducts.push_back(
            mathfunc::median_value(sector_dotproducts));
      }
      
//      cout << "i = " << i 
//           << " colorhist_dotproduct = " << colorhist_dotproducts[i]
//           << " median color sector dotproduct = "
//           << median_color_sector_dotproducts[i] << endl;

   } // loop over index i labeling images

   cout << "colorhist_dotproducts.size() = "
        << colorhist_dotproducts.size() << endl;
   cout << "median_color_sector_dotproducts.size() = "
        << median_color_sector_dotproducts.size() << endl;

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

   string asift_keys_subdir=bundler_IO_subdir+"asift_keys/";
//   filefunc::purge_files_in_subdir(asift_keys_subdir);
//   SIFT.extract_ASIFT_features(asift_keys_subdir);

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
         banner="Exported "+all_keys_filename;
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

// --------------------------------------------------------------------------
// Feature matching starts here:

   string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
   filefunc::dircreate(tiepoints_subdir);
   string ntiepoints_filename=tiepoints_subdir+"ntiepoints.dat";

   bool match_features_flag=!(filefunc::fileexist(ntiepoints_filename));
   if (match_features_flag)
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
            i,sqrd_max_ratio,worst_frac_to_reject,max_scalar_product,
            max_n_good_RANSAC_iters,min_n_features,minimal_number_of_inliers,
            bundler_IO_subdir,map_unionfind_ptr,ntiepoints_stream);
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

// Parse ntiepoints file and search for temporal discontinuities in
// both numbers of extracted tiepoints and nonzero block fractions:

   vector<vector<string> > row_substrings=
      filefunc::ReadInSubstrings(ntiepoints_filename);
   vector<double> n_tiepoints,nonzero_block_fracs;
   vector<string> curr_frame,next_frame;
   for (int r=0; r<row_substrings.size(); r++)
   {
      n_tiepoints.push_back(
         stringfunc::string_to_number(row_substrings[r].at(2)));
      nonzero_block_fracs.push_back(
         stringfunc::string_to_number(row_substrings[r].at(5)));
      curr_frame.push_back(row_substrings[r].at(6));
      next_frame.push_back(row_substrings[r].at(7));
   }

// Export temporal locations of shot boundaries to output text file:
   
   string shot_boundaries_filename=tiepoints_subdir+"shot_boundaries.dat";
   ofstream shot_boundaries_stream;
   filefunc::openfile(shot_boundaries_filename,shot_boundaries_stream);
   shot_boundaries_stream << 
      "# curr_filename next_filename  N_tiepoints  tiepoint_ratio nonzero_tiepoint_blocks_frac" << endl << endl;

   for (int r=1; r<curr_frame.size()-1; r++)
   {
      double curr_tiepoint_ratio=0;
      if (n_tiepoints[r-1] > 0 && n_tiepoints[r] > 0)
      {
         double delta_n_tiepoints=fabs(n_tiepoints[r]-n_tiepoints[r-1]);
         curr_tiepoint_ratio=delta_n_tiepoints/
            basic_math::min(n_tiepoints[r-1],n_tiepoints[r]);
      }
      double curr_prev_colordotproduct=colorhist_dotproducts[r];
      double curr_prev_median_colorsector_dotproduct=
         median_color_sector_dotproducts[r];

      bool shot_boundary_detected_flag=false;

// If significant dropoff or increase in number of SIFT feature
// matches along with at least some color change occur, declare shot
// boundary:

      if ( curr_tiepoint_ratio < 0.4 &&
//          nonzero_block_fracs[r] < 0.4 &&
          curr_prev_colordotproduct < 0.9) shot_boundary_detected_flag=true;

      if ( curr_tiepoint_ratio > 3 &&
//          nonzero_block_fracs[r] > 0.8 &&
          curr_prev_colordotproduct < 0.9) shot_boundary_detected_flag=true;

      if ( curr_tiepoint_ratio > 2 &&
//          nonzero_block_fracs[r] > 0.8 &&
          curr_prev_colordotproduct < 0.5 &&
         curr_prev_median_colorsector_dotproduct < 0.5) 
         shot_boundary_detected_flag=true;

// If major color jump occurs and SIFT matches are confined to
// relatively small portion of image plane (e.g. text ticker region),
// declare shot boundary:

      if (nonzero_block_fracs[r] < 0.3 &&
          curr_prev_colordotproduct < 0.52 &&
         curr_prev_median_colorsector_dotproduct < 0.5) 
         shot_boundary_detected_flag=true;

// If major dropoff in SIFT feature matches along with non-negligible
// color change occurs, declare shot boundary (maybe fade):

      if (curr_tiepoint_ratio < 0.1 &&
          curr_prev_colordotproduct < 0.91 &&
          curr_prev_median_colorsector_dotproduct < 0.81) 
         shot_boundary_detected_flag=true;

      if (curr_tiepoint_ratio < 0.1 &&
          curr_prev_colordotproduct < 0.94 &&
          curr_prev_median_colorsector_dotproduct < 0.78) 
         shot_boundary_detected_flag=true;

      if (curr_tiepoint_ratio < 0.1 &&
          curr_prev_colordotproduct < 0.95 &&
          curr_prev_median_colorsector_dotproduct < 0.75) 
         shot_boundary_detected_flag=true;

// If nontrivial changes in SIFT feature matches, nonzero_block_fracs
// and color occurs, declare shot boundary (maybe fade):

      if (curr_tiepoint_ratio < 0.666 && 
          nonzero_block_fracs[r] > 0.3 &&
          curr_prev_colordotproduct < 0.8) shot_boundary_detected_flag=true;

// If a large number of SIFT features match between two frames and
// they are spread out over a significant fraction of the image plane,
// declare NO shot boundary:

      if (n_tiepoints[r] > 400 &&
          nonzero_block_fracs[r] > 0.5) shot_boundary_detected_flag=false;


      if (shot_boundary_detected_flag)
      {
         shot_boundaries_stream 
            << "------------------------------------------------------------"
            << endl;
      }

      shot_boundaries_stream 
         << curr_frame[r] << "  "
         << next_frame[r] << "  "
         << n_tiepoints[r] << "  "
         << curr_tiepoint_ratio << "   "
         << nonzero_block_fracs[r] << "   "
         << colorhist_dotproducts[r] << "   "
         << curr_prev_median_colorsector_dotproduct 
         << endl;

   }
   filefunc::closefile(shot_boundaries_filename,shot_boundaries_stream);

   banner="Exported shot boundaries to "+shot_boundaries_filename;
   outputfunc::write_big_banner(banner);

   cout << "At end of program MATCH_SUCCESSIVE_FRAMES" << endl;
   outputfunc::print_elapsed_time();
}
