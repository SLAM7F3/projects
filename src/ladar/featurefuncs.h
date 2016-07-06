// ==========================================================================
// Header file for feature extraction methods
// ==========================================================================
// Last modified on 9/30/09; 10/1/09; 10/3/09
// ==========================================================================

#ifndef FEATUREFUNCS_H
#define FEATUREFUNCS_H

#include <set>
#include <vector>
#include "datastructures/datapoint.h"
#include "math/threevector.h"

class contour;
template <class T> class Hashtable;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class parallelogram;
class polygon;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace featurefunc
{
   extern const double low_tree_sentinel_value;
   extern const double tree_sentinel_value;  
   extern const double building_sentinel_value;  
   extern const double grass_sentinel_value;  
   extern const double road_sentinel_value;  
   extern const double shadow_sentinel_value;  

// General feature extraction methods:

   void mark_pixels_in_list(
      double intensity_value,linkedlist const *currlist_ptr,
      twoDarray *ftwoDarray_ptr);
   twoDarray* update_feature_map(
      twoDarray const *features_twoDarray_ptr,
      twoDarray const *p_refined_roof_twoDarray_ptr);
   void refine_tree_features(
      twoDarray const *ztwoDarray_ptr,twoDarray* features_twoDarray_ptr);
   bool feature_nearby(
      double radius,const threevector& posn,double feature_value,
      const twoDarray* features_twoDarray_ptr);
   void remove_isolated_height_outliers_from_feature_map(
      parallelogram const *data_bbox_ptr,twoDarray* ztwoDarray_ptr,
      twoDarray const *ftwoDarray_ptr);
   twoDarray* cull_feature_pixels(
      double feature_value,twoDarray const *ftwoDarray_ptr);
   twoDarray* cull_feature_pixels(
      double feature_value,double null_value,twoDarray const *ftwoDarray_ptr);
   twoDarray* cull_feature_pixels(
      double feature_value,twoDarray const *ztwoDarray_ptr,
      twoDarray const *ftwoDarray_ptr);
   void transfer_feature_pixels(
      double feature_value,twoDarray const *ftwoDarray_ptr,
      twoDarray *fnew_twoDarray_ptr);
   void transfer_feature_pixels(
      double feature_value,double new_feature_value,
      twoDarray const *ftwoDarray_ptr,twoDarray *fnew_twoDarray_ptr);
   void recolor_feature_pixels(
      double old_feature_value,double new_feature_value,
      twoDarray* ftwoDarray_ptr);
   void recolor_feature_pixels(
      double old_feature_value,double new_feature_value_lo,
      double new_feature_value_hi,double z_cutoff,
      twoDarray const *ztwoDarray_ptr,twoDarray* ftwoDarray_ptr);
   double abs_gradient_contour_integral(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,contour const *contour_ptr);

// Road and grass extraction methods:

   twoDarray* distinguish_road_and_grass_pixels(
      int n_iters,std::string imagedir,double p_tall_sentinel_value,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr);
   twoDarray* generate_binary_asphalt_image(
      twoDarray const *features_twoDarray_ptr);
   void density_filter_road_content(
      double filter_diameter,double fill_frac_threshold,
      twoDarray* ptwoDarray_ptr);
   twoDarray* locate_road_seeds(
      std::string imagedir,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   twoDarray* classify_road_grass_pixels(
      double p_tall_sentinel_value,twoDarray const *ptwoDarray_ptr);
   void remove_feature_holes_from_feature_map(
      double feature_sentinel_value,
      twoDarray* features_twoDarray_ptr,twoDarray const *ztwoDarray_ptr,
      std::string particular_feature_name,std::string imagedir);
   twoDarray* fill_feature_islands(
      twoDarray const *ftwoDarray_ptr,double feature_sentinel_value);

// Tree and building extraction methods:

   twoDarray* distinguish_tree_from_bldg_pixels(
      int n_iters,std::string imagedir,double p_low_sentinel_value,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr,
      twoDarray const *norm_zfluc_twoDarray_ptr);
   twoDarray* locate_building_clusters(
      std::string imagedir,
      twoDarray *ptwoDarray_ptr,twoDarray const *ztwoDarray_ptr);

   std::vector<std::pair<int,std::vector<double> > >*
      detect_tiered_roofs(
      Hashtable<linkedlist*> const *connected_hashtable_ptr,
      twoDarray const *ztwoDarray_ptr);
   twoDarray* refine_building_extraction(
      std::string imagedir,
      std::vector<std::pair<int,std::vector<double> > >* tiered_bldg_ptr,
      Hashtable<linkedlist*> const *connected_hashtable_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* ptwoDarray_ptr);
   linkedlist* locate_rooftop_seed_pixels(
      int n,std::vector<std::pair<int,std::vector<double> > >* 
      tiered_bldg_ptr,linkedlist* currlist_ptr,
      twoDarray const *ztwoDarray_ptr);
   linkedlist* median_height_seed_voxels(
      double height_tolerance,linkedlist* voxel_list_ptr,
      twoDarray const *ztwoDarray_ptr);
   void compute_building_bbox(
      linkedlist* curr_list_ptr,twoDarray const *ztwoDarray_ptr,
      const double bbox_tolerance_distance,
      double& minimum_x,double& minimum_y,double& maximum_x,double& maximum_y,
      double& x_center,double& y_center,double& char_radius,
      unsigned int& min_px,unsigned int& min_py,
      unsigned int& max_px,unsigned int& max_py);
   void ooze_rooftop_pixels(
      int n_rooftop,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      linkedlist* seed_list_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray* ptwoDarray_ptr,std::string imagedir);

   twoDarray* ooze_rooftop_pixels(
      double max_radius,const threevector& rooftop_posn,
      twoDarray const *ztwoDarray_ptr,double& max_roof_z);
   twoDarray* construct_rooftop_binary_mask(
      const threevector& rooftop_posn,twoDarray const *ztwoDarray_ptr,
      threevector& COM,double& theta,double& max_roof_z);


   double accumulate_binary_rooftop_info(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      twoDarray const *ztwoDarray_ptr,twoDarray const *tmp_ptwoDarray_ptr,
      twoDarray* p_refined_roof_twoDarray_ptr);
   void subtract_oozed_pixels(
      linkedlist* currlist_ptr,twoDarray const *p_refined_roof_twoDarray_ptr);
   void clean_ancillary_building_parts(
      int n_rooftop,std::string imagedir,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double z_primary_roof_COM,twoDarray const *ztwoDarray_ptr,
      polygon const *convexhull_ptr,twoDarray* p_ancillary_roof_twoDarray_ptr,
      Hashtable<linkedlist*>* connected_bldgparts_hashtable_ptr);
   double pixels_average_height(
      linkedlist const *pixels_list_ptr,twoDarray const *ztwoDarray_ptr);
   linkedlist* locate_ancillary_bldg_part_seeds(
      polygon const *hull_ptr,linkedlist const *pixels_list_ptr,
      twoDarray const *ztwoDarray_ptr);
   void accumulate_building_parts(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      twoDarray const *pconnected_components_twoDarray_ptr,
      twoDarray *p_refined_roof_twoDarray_ptr);
   void compute_building_COM_locations(
      Hashtable<linkedlist*> const *connected_hashtable_ptr,
      twoDarray *ptwoDarray_ptr);
   twoDarray* classify_tree_building_pixels(
      double p_tall_sentinel_value,twoDarray const *ptwoDarray_ptr,
      twoDarray const *ztall_twoDarray_ptr);
   double tree_pixels_outside_contour(
      twoDarray const *features_twoDarray_ptr,contour const *contour_ptr);

// Feature clustering & contour methods:

   Hashtable<linkedlist*>* extract_feature_clusters(
      double min_footprint_area,double feature_sentinel_value,
      std::string feature_type,std::string imagedir,
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr);
   Hashtable<linkedlist*>* connected_feature_components(
      double min_footprint_area,std::string feature_type,std::string imagedir,
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *pbinary_twoDarray_ptr);
   void generate_feature_clusters_network(
      Hashtable<linkedlist*>* connected_feature_components_hashtable_ptr);
   void fit_deformable_contours_around_feature_clusters(
      std::string feature_type,std::string imagedir,
      std::string features_filenamestr,twoDarray const *ztwoDarray_ptr,
      Hashtable<linkedlist*>* connected_feature_components_hashtable_ptr);
   contour* generate_feature_cluster_contour(
      int n_cluster,linkedlist const *pixel_list_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* fmask_twoDarray_ptr);
   contour* generate_feature_cluster_contour(
      double delta_s,double znull,linkedlist const *pixel_list_ptr,
      twoDarray const *fmask_twoDarray_ptr);
   void rasterize_feature_cluster_contour(
      int n_cluster,std::string imagedir,const contour& c,
      twoDarray const *ztwoDarray_ptr,twoDarray* fmask_twoDarray_ptr);

// Car detection methods:

   twoDarray* subtract_genuine_ground_asphalt_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr,
      twoDarray const *ooze_asphalt_twoDarray_ptr);
   void color_local_height_bumps(
      twoDarray const *zbump_twoDarray_ptr,
      twoDarray* features_twoDarray_ptr,double annotation_value);
}

#endif // featurefuncs.h



