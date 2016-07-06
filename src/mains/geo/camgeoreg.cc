// =======================================================================
// Program CAMGEOREG imports a georegistered TIF file generated from a
// Google Earth screen shot plus ground control points.  It also takes
// in an aerial video frame. CAMGEOREG extracts SIFT and ASIFT
// features via calls to Lowe's SIFT binary and the affine SIFT
// library.  Consolidated SIFT & ASIFT interest points and
// descriptors are exported to key files following Lowe's conventions.  
// CAMGEOREG next performs tiepoint matching via homography
// estimation and RANSAC on the consolidated sets of image features.
// The best-fit homography matrix and feature tracks labeled by unique
// IDs are exported to output text files.

// =======================================================================
// Last updated on 3/9/13; 4/8/13; 4/9/13; 7/2/13; 4/6/14
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <opencv2/features2d/features2d.hpp>
#include "video/camerafuncs.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "video/image_matcher.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "image/raster_parser.h"
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
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string packages_subdir=bundler_IO_subdir+"packages/";

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

// We explicitly confirmed on 1/30/13 that the FLANN library yields
// noticeably better feature matching results than the older ANN
// library:

   bool FLANN_flag=true;
//   bool FLANN_flag=false;
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

// ==========================================================================
// Instantiate RasterParser object to hold georegistration information
// for Google Earth screen shot:

   string EO_geotif_filename=photogroup_ptr->get_photograph_ptr(0)->
      get_filename();
   cout << "EO_geotif_filename = " << EO_geotif_filename << endl;
   string videoframe_filename=photogroup_ptr->get_photograph_ptr(1)->
      get_filename();
   cout << "videoframe_filename = " << videoframe_filename << endl;

   texture_rectangle* EO_geotif_texture_rectangle_ptr=new texture_rectangle(
      EO_geotif_filename,NULL);
   texture_rectangle* videoframe_texture_rectangle_ptr=new texture_rectangle(
      videoframe_filename,NULL);

   raster_parser RasterParser;
   RasterParser.open_image_file(EO_geotif_filename);
         
   int n_channels=RasterParser.get_n_channels();
   cout << "n_channels = " << n_channels << endl;

   twoDarray *RtwoDarray_ptr,*GtwoDarray_ptr,*BtwoDarray_ptr;
   double E_min,E_max,N_min,N_max;

   for (int channel_ID=0; channel_ID<n_channels; channel_ID++)
   {
      RasterParser.fetch_raster_band(channel_ID);
      
      if (n_channels==3)
      {
//               cout << "channel_ID = " << channel_ID << endl;
         if (channel_ID==0)
         {
            RtwoDarray_ptr=RasterParser.get_RtwoDarray_ptr();
//                  cout << "RtwoDarray_ptr = " << RtwoDarray_ptr << endl;
            RasterParser.read_raster_data(RtwoDarray_ptr);

            E_min=RtwoDarray_ptr->get_xlo();
            E_max=RtwoDarray_ptr->get_xhi();
            N_min=RtwoDarray_ptr->get_ylo();
            N_max=RtwoDarray_ptr->get_yhi();
         }
         else if (channel_ID==1)
         {
            GtwoDarray_ptr=RasterParser.get_GtwoDarray_ptr();
//            cout << "GtwoDarray_ptr = " << GtwoDarray_ptr << endl;
            RasterParser.read_raster_data(GtwoDarray_ptr);
         }
         else if (channel_ID==2)
         {
            BtwoDarray_ptr=RasterParser.get_BtwoDarray_ptr();
//            cout << "BtwoDarray_ptr = " << BtwoDarray_ptr << endl;
            RasterParser.read_raster_data(BtwoDarray_ptr);
         }
      } // n_channels conditional
   } // loop over channel_ID labeling RGB channels

   RasterParser.close_image_file();

   cout << "E_min = " << E_min << " E_max = " << E_max << endl;
   cout << "N_min = " << N_min << " N_max = " << N_max << endl;
   
// --------------------------------------------------------------------------
// Feature extraction starts here:

// Extract conventional SIFT features from each input image via
// Lowe's binary:

   string sift_keys_subdir=bundler_IO_subdir+"sift_keys/";
   bool delete_pgm_file_flag=false;
   SIFT.extract_SIFT_features(sift_keys_subdir,delete_pgm_file_flag);
   outputfunc::print_elapsed_time();

// In Feb 2013, we empirically found that the Oxford feature detectors
// yielded the following number of tiepoint matches for two pairs of
// Pass 15 GEO video frames:

/*
   string haraff_keys_subdir=bundler_IO_subdir+"haraff_keys/";
   SIFT.extract_Oxford_features(
      "haraff",haraff_keys_subdir,delete_pgm_file_flag);

   string harlap_keys_subdir=bundler_IO_subdir+"harlap_keys/";
   SIFT.extract_Oxford_features(
      "harlap",harlap_keys_subdir,delete_pgm_file_flag);

   string heslap_keys_subdir=bundler_IO_subdir+"heslap_keys/";
   SIFT.extract_Oxford_features(
      "heslap",heslap_keys_subdir,delete_pgm_file_flag);

   string harhes_keys_subdir=bundler_IO_subdir+"harhes_keys/";
   SIFT.extract_Oxford_features(
      "harhes",harhes_keys_subdir,delete_pgm_file_flag);

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

// --------------------------------------------------------------------------
// Feature matching starts here:

   int istart=0;
   int istop=n_images;
   int n_feature_tracks=0;

//    const int n_min_quadrant_features=1;		
//   double max_ratio=0.7;    	// OK for GEO
//   double max_ratio=0.8;    	// OK for GE-HAFB frame comparison
   double max_ratio=0.87;    	// OK for GE-MIT frame comparison
//   cout << "Enter max Lowe ratio:" << endl;
//   cin >> max_ratio;

   double sqrd_max_ratio=sqr(max_ratio);
   double worst_frac_to_reject=0;

// SIFT tiepoint inlier identification becomes LESS stringent as
// max_sqrd_delta increases.

// For GE4 - HAFB frame 279 matching, 
//	max_ratio=0.75 and max_sqrd_delta=sqr(0.05) yields 55 good matches
//	max_ratio=0.80 and max_sqrd_delta=sqr(0.025) yields 100 reasonable
//		 matches

// For GE0a - MIT_May25_clip07_0005 matching
// 	max_ratio = 0.8 and max_sqrd_delta=sqr(0.035) yields 26 OK matches
// 	max_ratio = 0.8 and max_sqrd_delta=sqr(0.025) yields 26 OK matches

   double sqrt_max_sqrd_delta=0.015;
   cout << "Enter sqrt(max_sqrd_delta):" << endl;
//   cin >> sqrt_max_sqrd_delta;
   double max_sqrd_delta=sqr(sqrt_max_sqrd_delta);

   int max_n_good_RANSAC_iters=200;
   for (int i=istart; i<istop; i++)
   {
      for (int j=i+1; j<n_images; j++)
      {
         outputfunc::print_elapsed_time();
         cout << "i = " << i << " j = " << j << endl;

// Match SIFT & ASIFT features across image pairs:

         string banner="Matching SIFT/ASIFT features:";
         outputfunc::write_big_banner(banner);

         SIFT.identify_candidate_feature_matches_via_Lowe_ratio_test(
            i,j,j,sqrd_max_ratio);

         if (!SIFT.identify_inlier_matches_via_homography(
            i,j,max_n_good_RANSAC_iters,worst_frac_to_reject,max_sqrd_delta)) 
            continue;

         SIFT.export_homography_matrix(bundler_IO_subdir,i,j);
         SIFT.rename_feature_IDs(i,j); // Rename tiepoint pair labels

      } // loop over index j labeling input images

      n_feature_tracks += SIFT.export_feature_tracks(i,features_subdir);

   } // loop over index i labeling input images

   cout << "n_feature_tracks = " << n_feature_tracks << endl;
   if (n_feature_tracks==0) exit(-1);

// --------------------------------------------------------------------------
// Export matched features to output html file:

   FeaturesGroup* FeaturesGroup_ptr=new FeaturesGroup(
      ndims,passes_group.get_pass_ptr(videopass_ID),NULL);
   FeaturesGroup_ptr->read_in_photo_features(
      photogroup_ptr,features_subdir);
   bool output_only_multicoord_features_flag=true;
   FeaturesGroup_ptr->write_feature_html_file(
      photogroup_ptr,output_only_multicoord_features_flag);
   cout << "n_features = " << FeaturesGroup_ptr->get_n_Graphicals() << endl;
   delete FeaturesGroup_ptr;

// =======================================================================

// Recover XY (warped GE image) and UV (aerial video frame) inlier
// tiepoint coordinates:

   SIFT.recover_inlier_tiepoints();
   vector<int> inlier_ID=SIFT.get_inlier_tiepoint_ID();
   vector<twovector> inlier_XY=SIFT.get_inlier_XY();
   vector<twovector> inlier_UV=SIFT.get_inlier_UV();
   vector<twovector> inlier_EN;

   homography* H_ptr=SIFT.get_homography_ptr();

   int width=EO_geotif_texture_rectangle_ptr->getWidth();
   int height=EO_geotif_texture_rectangle_ptr->getHeight();

   double U_min=0;
   double U_max=double(width)/double(height);
   double V_min=0;
   double V_max=1;
   bool world_to_image_flag=true;


// Generate alpha-blended composites of aerial video frame geoaligned
// to warped GE image:

   double alpha_start=0.0;
   double alpha_stop=1.0;
//   double alpha_stop=2.0;
//   double d_alpha=0.05;
   double d_alpha=0.25;

   double alpha=alpha_start;
   while (alpha <= alpha_stop)
   {
      string rectified_image_filename=bundler_IO_subdir+
         "orthorectified_videoframe_"+stringfunc::number_to_string(alpha,2)
         +".jpg";

      camerafunc::orthorectify_image(
         videoframe_texture_rectangle_ptr,width,height,
         EO_geotif_filename,alpha,
         U_min,U_max,V_min,V_max,*H_ptr,rectified_image_filename,
         world_to_image_flag);
      alpha += d_alpha;
   }

// Recover aerial video camera parameters from homography:

   photograph* photo_i_ptr=photogroup_ptr->get_photograph_ptr(0);
   photograph* photo_j_ptr=photogroup_ptr->get_photograph_ptr(1);
   cout << "image i = " << photo_i_ptr->get_filename() << endl;
   cout << "image j = " << photo_j_ptr->get_filename() << endl;
         
   camera* camera_i_ptr=photo_i_ptr->get_camera_ptr();
   camera* camera_j_ptr=photo_j_ptr->get_camera_ptr();
         
//   double u0_i=camera_i_ptr->get_u0();
//   double v0_i=camera_i_ptr->get_v0();
   double u0_j=camera_j_ptr->get_u0();
   double v0_j=camera_j_ptr->get_v0();
         
   double f,az,el,roll;
   threevector camera_posn;
   genmatrix P(3,4);
   H_ptr->compute_camera_params_from_zplane_homography(
      u0_j,v0_j,f,az,el,roll,camera_posn,P);

   double aspect_ratio=u0_j/v0_j;
   double FOV_u,FOV_v;
   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      f,aspect_ratio,FOV_u,FOV_v);

   cout << "f = " << f << endl;
   cout << "az = " << az*180/PI << endl;
   cout << "el = " << el*180/PI << endl;
   cout << "roll = " << roll*180/PI << endl;
   cout << "camera_posn = " << camera_posn << endl;
   cout << "FOV_u = " << FOV_u*180/PI << endl;
   cout << "FOV_v = " << FOV_v*180/PI << endl;

   camera_j_ptr->set_f(f);
   camera_j_ptr->set_Rcamera(az,el,roll);
   camera_j_ptr->set_world_posn(camera_posn);
   camera_j_ptr->construct_seven_param_projection_matrix();
   cout << "*camera_j_ptr = " << *camera_j_ptr << endl;

// Compute corner ray intercepts in UTM geocoords:

   unsigned int px,py;
   double easting,northing;
   cout.precision(10);

   double ground_Z=0;
   vector<threevector> corner_ray_intercepts=camera_j_ptr->
      corner_ray_intercepts_with_zplane(ground_Z);
   vector<threevector> world_corner_ray_intercepts;
   for (int c=0; c<corner_ray_intercepts.size(); c++)
   {
      EO_geotif_texture_rectangle_ptr->get_pixel_coords(
         corner_ray_intercepts[c].get(0),
         corner_ray_intercepts[c].get(1),px,py);
      RtwoDarray_ptr->pixel_to_point(px,py,easting,northing);
      cout << "c = " << c << " corner ray intercept = "
           << corner_ray_intercepts[c] << endl;
      cout << "Easting = " << easting << " northing = " << northing
           << endl << endl;
      world_corner_ray_intercepts.push_back(threevector(easting,northing,0));
   }

   double avg_world_to_XY_ratio=0;
   for (int c=0; c<4; c++)
   {
      int next_c=modulo(c+1,4);
      double XY_dist=
         (corner_ray_intercepts[next_c]-corner_ray_intercepts[c]).magnitude();
      double world_dist=
         (world_corner_ray_intercepts[next_c]-world_corner_ray_intercepts[c]).
         magnitude();
      double world_to_XY_ratio=world_dist/XY_dist;
      cout << "c = " << c << " world_to_XY_ratio = "
           << world_to_XY_ratio << endl;
      avg_world_to_XY_ratio += 0.25*world_to_XY_ratio;
   }
   cout << "Average world_to_XY ratio = " << avg_world_to_XY_ratio << endl;

// Transform aerial video camera position to UTM geocoordinates:

   EO_geotif_texture_rectangle_ptr->get_pixel_coords(
      camera_posn.get(0),camera_posn.get(1),px,py);
   RtwoDarray_ptr->pixel_to_point(px,py,easting,northing);

   double aircraft_altitude=avg_world_to_XY_ratio*camera_posn.get(2);
   cout << "Aircraft altitude (meters) = " << aircraft_altitude << endl;
   camera_posn=threevector(easting,northing,aircraft_altitude);
   camera_j_ptr->set_world_posn(camera_posn);   
   cout << "camera geoposn = " << camera_posn << endl;

// Export package file for aerial video camera:

   string package_filename=packages_subdir+"aerial_vid.pkg";
   int photo_ID=1;
   double frustum_sidelength=1000;	// meters
   camera_j_ptr->write_camera_package_file(
      package_filename,photo_ID,videoframe_filename,frustum_sidelength);
   string banner="Exported "+package_filename;
   outputfunc::write_big_banner(banner);

   delete EO_geotif_texture_rectangle_ptr;
   delete videoframe_texture_rectangle_ptr;
   
   outputfunc::print_elapsed_time();
}
