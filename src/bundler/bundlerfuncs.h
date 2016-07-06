// =========================================================================
// Header file for stand-alone bundler functions.
// =========================================================================
// Last modified on 6/13/13; 7/28/13; 9/10/13
// =========================================================================

#ifndef BUNDLERFUNCS_H
#define BUNDLERFUNCS_H

#include <map>
#include <string> 

class camera;
class FeaturesGroup;
class genmatrix;
class PassesGroup;
class photograph;
class photogroup;
class rotation;
class threevector;

namespace bundlerfunc
{

   std::string generate_list_tmp_file(std::string bundler_IO_subdir);
   std::string generate_trivial_bundle_dot_out_file(std::string list_tmp_file);

   void read_in_pointcloud_and_photos(
      std::string subdir,PassesGroup& passes_group,
      int photo_number_step,int cloudpass_ID,
      std::string& bundler_IO_subdir,
      std::string& image_sizes_filename,std::string& xyz_points_filename);

   void scale_translate_rotate_bundler_XYZ(
      threevector& bundler_xyz,
      double fitted_world_to_bundler_distance_ratio,
      const threevector& fitted_bundler_trans,
      double global_az,double global_el,double global_roll,
      const threevector& rotation_origin);
   void scale_translate_rotate_bundler_XYZ(
      threevector& bundler_xyz,
      double fitted_world_to_bundler_distance_ratio,
      const threevector& fitted_bundler_trans,const rotation& global_R,
      const threevector& rotation_origin);

   void rotate_scale_translate_bundler_XYZ(
      threevector& bundler_xyz,const threevector& bundler_COM,
      double fitted_world_to_bundler_distance_ratio,
      const threevector& fitted_bundler_trans,const rotation& global_R);

// GPS georegistration methods

   void read_left_right_points(
      std::string left_points_filename,std::string right_points_filename,
      std::vector<threevector>& left_points,
      std::vector<threevector>& right_points);
   void subtract_left_right_COMs(
      std::vector<threevector>& left_points,
      std::vector<threevector>& right_points,
      threevector& left_COM,threevector& right_COM);
   genmatrix* form_symmetric_N_matrix(
      std::vector<threevector>& left_points,
      std::vector<threevector>& right_points);
   void fit_rotation_angles(
      genmatrix* N_ptr,double& az,double& el,double& roll);
   double fit_scale_factor(
      const std::vector<threevector>& left_points,
      const std::vector<threevector>& right_points);
   threevector fit_translation_and_reset_left_right_points(
      std::vector<threevector>& left_points,
      std::vector<threevector>& right_points,
      const threevector& right_COM,const threevector& left_COM,
      const rotation& R,double scale);
   void RANSAC_fit_rotation_translation_scale(
      std::vector<threevector>& left_points,
      std::vector<threevector>& right_points,
      threevector& left_COM,threevector& right_COM,
      double& az,double& el,double& roll,double& scale,
      threevector& trans_Horn,threevector& trans_Peter,
      double& median_residual_dist,double& quartile_width);
   void fit_rot_trans_scale_params(
      std::vector<threevector>& left_points,
      std::vector<threevector>& right_points,
      threevector& left_COM,threevector& right_COM,
      double& az,double& el,double& roll,double& scale,threevector& trans_Horn,
      threevector& trans_Peter);
   double compute_avg_residual(
      const std::vector<threevector>& left_points,
      const std::vector<threevector>& right_points,
      std::vector<threevector>& transformed_left_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn);
   std::vector<int> identify_left_right_pair_inliers(
      const std::vector<threevector>& left_points,
      const std::vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,double max_residual_dist);
   std::vector<int> identify_left_right_pair_outliers(
      const std::vector<threevector>& left_points,
      const std::vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,double max_residual_dist);
   double inlier_left_right_pair_fraction(
      const std::vector<threevector>& left_points,
      const std::vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,double max_residual_dist);
   void median_left_right_residual_distance(
      const std::vector<threevector>& left_points,
      const std::vector<threevector>& right_points,
      double az,double el,double roll,double scale,
      const threevector& trans_Horn,
      double& median_distance,double& quartile_width);

// Radial undistortion methods:

   std::string radial_undistort(
      photograph* photo_ptr,std::string undistorted_images_subdir);
   std::string radial_undistort(
      camera* camera_ptr,std::string photo_filename,
      std::string undistorted_images_subdir);
   void generate_undistorted_bundle_file(
      std::string bundle_filename,std::string undistorted_images_subdir);

// Bundler.out file export methods:

   void export_bundle_file(
      std::string bundle_filename,photogroup* photogroup_ptr,
      FeaturesGroup* FeaturesGroup_ptr);
   void export_bundle_file(
      std::string bundle_filename,photogroup* photogroup_ptr,
      FeaturesGroup* FeaturesGroup_ptr,
      std::map<int,bool>* feature_ids_to_ignore_map_ptr);
   void export_bundle_file(
      std::string bundle_filename,photogroup* photogroup_ptr,
      const std::vector<threevector>& XYZ);

// Z_ground finding methods:

   double extract_Zground(std::string bundler_IO_subdir);

}


#endif // bundlerfuncs.h

