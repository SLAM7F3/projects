// =========================================================================
// Header file for ground extraction functions.
// =========================================================================
// Last modified on 11/1/07; 11/10/07; 11/11/07
// =========================================================================

#ifndef GROUNDFUNCS_H
#define GROUNDFUNCS_H

#include <set>
#include <vector>
#include "math/threevector.h"
class parallelogram;
class polygon;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace groundfunc
{
   twoDarray* extract_ground(
      double xextent,double yextent,double correlation_distance,
      parallelogram const *data_bbox_ptr,double local_threshold_frac,
      twoDarray const *ztwoDarray_ptr);
   void generate_threshold_centers( 
      double xextent,double yextent,double correlation_distance,
      double local_threshold_frac,twoDarray const *ztwoDarray_ptr,
      std::vector<double>& threshold_intensity,
      std::vector<twovector>& centers_posn);
   void generate_threshold_centers( 
      int mbins,int nbins,double correlation_distance,
      double local_threshold_frac,
      double zground_min,double minimal_fill_frac,
      twoDarray const *ztwoDarray_ptr,
      std::vector<double>& threshold_intensity,
      std::vector<twovector>& centers_posn);
   void generate_threshold_field( 
      double correlation_distance,parallelogram const *data_bbox_ptr,
      const std::vector<double>& threshold_intensity,
      const std::vector<twovector>& centers_posn,
      twoDarray const *ztwoDarray_ptr,twoDarray* zthreshold_twoDarray_ptr);
   bool erode_strong_gradient_regions(
      double grad_magnitude_threshold,
      twoDarray* ztwoDarray_ptr,twoDarray const *grad_mag_twoDarray_ptr,
      twoDarray const *grad_phase_twoDarray_ptr);

// Ooozing methods:

   void identify_ground_seed_pixels(twoDarray const *ztwoDarray_ptr,
                                    twoDarray* ground_seeds_twoDarray_ptr);
   void find_low_local_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray* groundmask_twoDarray_ptr,
      double max_gradient_magnitude_lo=0.7,int n_recursion_iters=5,
      double nonground_sentinel_value=1.0);
   void local_ground_ooze(
      unsigned int px,unsigned int py,twoDarray const *ztwoDarray_ptr,
      twoDarray* zhilo_twoDarray_ptr,
      double max_gradient_magnitude_lo,
      const std::vector<double>& delta_s,int& nchanges,
      std::vector<std::pair<int,int> >* new_pixel_posns_ptr);
   twoDarray* find_low_local_pixels(
      const std::vector<threevector>& groundpoint_XYZ,
      twoDarray const *ztwoDarray_ptr,double max_gradient_magnitude_lo,
      int n_recursion_iters,parallelogram* data_bbox_ptr=NULL);
   twoDarray* find_low_local_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr);

// Ground flattening methods

   twoDarray* generate_zground_twoDarray(
     int iter,twoDarray const *ztwoDarray_ptr,
     twoDarray const *groundmask_twoDarray_ptr);
   twoDarray* generate_thresholded_flattened_height_map(
      twoDarray const *ztwoDarray_ptr,twoDarray* zthreshold_twoDarray_ptr,
      double cutoff_height);
   void refine_ground_mask_seeds(
      double cutoff_height,twoDarray* zflattened_twoDarray_ptr,
      twoDarray* groundmask_twoDarray_ptr);
   void eliminate_ground_outliers(
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *zthreshold_twoDarray_ptr,
      twoDarray* groundmask_twoDarray_ptr,double max_delta_height);
   void eliminate_ground_outliers(
      int n_filter_size,twoDarray const *ztwoDarray_ptr,
      twoDarray* groundmask_twoDarray_ptr,double max_delta_height);

   twoDarray* completely_flatten_ground(
      const twoDarray* ztwoDarray_ptr,const twoDarray* zhilo_twoDarray_ptr);
   twoDarray* generate_gaussian_weights_filter(double ds);
   twoDarray* interpolate_lo_asphalt_heights(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr,
      twoDarray const *zlo_asphalt_twoDarray_ptr);
   
// Ground planarity testing methods

   void compute_local_planarity(
      polygon& poly,TwoDarray<threevector> const *normal_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* zplanar_twoDarray_ptr);
   void compute_planarity_in_bbox(
      double bbox_width,double bbox_length,
      TwoDarray<threevector> const *normal_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray const *zhilo_twoDarray_ptr,
      twoDarray* zplanar_twoDarray_ptr);
   void faster_compute_planarity_in_bbox(
      double bbox_width,double bbox_length,
      TwoDarray<threevector> const *normal_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* zplanar_twoDarray_ptr);
   double integrate_dotproducts(
      int min_px,int max_px,int min_py,int max_py,
      double curr_normal_x,double curr_normal_y,double curr_normal_z,
      double normal_magnitude[],double& n_neighboring_normals,
      TwoDarray<threevector> const *normal_twoDarray_ptr);

// Surface feature enhancement methods:

   twoDarray* exaggerate_surface_image(
      double z1,double z2,twoDarray const *ztwoDarray_ptr);
   void average_down_small_height_fluctuations(
      twoDarray const *ztwoDarray_ptr,twoDarray* zsmoothed_twoDarray_ptr);
   void remove_individual_height_outliers(
      twoDarray const *ztwoDarray_ptr,twoDarray* zsmoothed_twoDarray_ptr);
   void remove_isolated_pixels(twoDarray* ztwoDarray_ptr);
}

#endif // groundfuncs.h




