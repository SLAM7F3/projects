// =========================================================================
// Header file for stand-alone ALIRT data manipulation functions.
// =========================================================================
// Last modified on 4/16/07; 10/26/07; 2/5/11
// =========================================================================

#ifndef LADARFUNCS_H
#define LADARFUNCS_H

#include <fstream>
#include <iostream>
#include <vector>
#include <osg/Array>
#include "math/constants.h"
#include "datastructures/datapoint.h"
#include "math/threevector.h"
#include "threeDgraphics/xyzpfuncs.h"

class geopoint;
template <class T> class Hashtable;
class ladarimage;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class parallelogram;
class polygon;
class prob_distribution;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace ladarfunc
{

// Flight path methods:

   std::string public_flight_path_filename(
      std::string& public_xyz_filenamestr);
   void shift_HAFB_to_Greenwich_origin(std::vector<threevector>& XYZ);
   void rotate_xy_coords(
      double theta,const threevector& origin,std::vector<threevector>& XYZ);

// Ladar data bounding box methods:

   void draw_data_bbox(
      parallelogram const *data_bbox_ptr,twoDarray* ztwoDarray_ptr);
   void null_data_outside_bbox(
      parallelogram const *data_bbox_ptr,twoDarray* ztwoDarray_ptr);
   void fake_compute_data_bbox(
      twoDarray const *ztwoDarray_ptr,parallelogram* data_bbox_ptr);
   void crop_data_inside_bbox(
      double elimination_frac,parallelogram const *data_bbox_ptr,
      twoDarray* ftwoDarray_ptr);
   void color_points_near_data_bbox(
      double dist_from_bbox,polygon* data_bbox_ptr,twoDarray* ftwoDarray_ptr);
   polygon* xy_data_bbox(
      twoDarray const *ztwoDarray_ptr,double delta_x,double delta_y);
   
// Height image cleaning methods:

   void median_fill_image(int niters,int nsize,twoDarray* ztwoDarray_ptr);
   void median_fill_image(int niters,int nsize,parallelogram* bbox_ptr,
                          twoDarray* ztwoDarray_ptr);
   void median_fill_image(int nsize,parallelogram* bbox_ptr,
                          twoDarray* ztwoDarray_ptr);
   int remove_isolated_outliers(
      parallelogram const *bbox_ptr,twoDarray* ztwoDarray_ptr);
   int remove_isolated_outliers(
      double feature_value,parallelogram const *bbox_ptr,
      twoDarray* ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr,
      int xsize,int ysize,int min_nonnull_pnts,
      double min_height_separation,double max_variance_factor);
   void threshold_zimage(
      double zmin,double zmax,twoDarray* ztwoDarray_ptr);
   void null_imagetwo_bright_pixels_in_imageone(
      double zthreshold,twoDarray const *ztwoDarray2_ptr,
      twoDarray* ztwoDarray1_ptr);
   void compute_xyzp_distributions(
      ladarimage* xyzimage_ptr,const std::vector<threevector>& XYZ,
      const std::vector<double>& p);
   void mark_tall_clusters(
      twoDarray const *ztwoDarray_ptr,twoDarray const *zcluster_twoDarray_ptr,
      twoDarray* ftwoDarray_ptr,double tall_object_null_value);
   void generate_xyzpfile_with_tall_object_info(
      std::string tall_objects_filenamestr,parallelogram const *data_bbox_ptr,
      double zmax,twoDarray* ztwoDarray_ptr,
      twoDarray const *zcluster_twoDarray_ptr,double tall_object_null_value);
   double compute_voxel_median_height(
      linkedlist* voxel_list_ptr,twoDarray const *ztwoDarray_ptr);
   double compute_z_distribution(
      std::string imagedir,twoDarray const *ztwoDarray_ptr);
   void mark_snowflake_points(
      double min_z,double max_z,double delta_z,std::vector<threevector>& XYZ);
   void mark_belowground_snowflake_points(
      double min_zdensity,double min_z,double max_z,double delta_z,
      osg::Vec3Array* vertices_ptr,double ceiling_min_z=POSITIVEINFINITY);

// Height gradient computation methods:

   void compute_x_y_deriv_fields(
      double spatial_resolution,parallelogram* bbox_ptr,
      twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      double min_distance_to_border=3,double null_value=xyzpfunc::null_value);
   void threshold_gradient_phase_field(
      double phi_min,double phi_max,
      twoDarray const *gradient_phase_twoDarray_ptr,
      twoDarray* phase_threshold_twoDarray_ptr);

   twoDarray* binary_subtract(
      twoDarray const *zsurviving_connected_binary_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr);

// Connected components methods:

   Hashtable<linkedlist*>* connect_height_components(
      double abs_zthreshold,double min_projected_area,
      twoDarray const *ztwoDarray_ptr);
   twoDarray* compute_hulls_surrounding_components(
      Hashtable<linkedlist*>* connected_hashtable_ptr,
      twoDarray *ztwoDarray_ptr);
   twoDarray* connect_binary_components(
      double min_projected_area,twoDarray const *zbinary_twoDarray_ptr);
   Hashtable<linkedlist*>* generate_connected_binary_components_hashtable(
      double min_projected_area,twoDarray const *zbinary_twoDarray_ptr);
   linkedlist* retrieve_connected_pixel_list(
      int n,Hashtable<linkedlist*> const *connected_hashtable_ptr);

// Intensity image cleaning methods:

   void set_pixel_intensity_values_to_sentinel(
      bool ignore_tall_objects,double z_threshold,double p_sentinel,
      twoDarray const *ztwoDarray_ptr,twoDarray* ptwoDarray_ptr);
   void probability_filter(
      int nsize,
      twoDarray const *ptwoDarray_ptr,twoDarray const *pmedian_twoDarray_ptr,
      twoDarray *pfiltered_twoDarray_ptr,double irrelevant_intensity);
   twoDarray* merge_hi_lo_intensity_images(
      double z_threshold,twoDarray const *ztwoDarray_ptr,
      twoDarray* p_hi_twoDarray_ptr,twoDarray* p_lo_twoDarray_ptr);
   void increase_intensities_for_large_height_fluctuations(
      double p_threshold,double norm_zfluc_threshold,double p_new_min,
      twoDarray const *ptwoDarray_ptr,
      twoDarray const *norm_zfluc_twoDarray_ptr,
      twoDarray* pnew_twoDarray_ptr);

// Drawing & coloring methods:

   void draw_xy_coordinate_system(
      std::string xyzp_filename,double annotation_value,
      twoDarray const *ztwoDarray_ptr,double z_coord_system=-10);
   twoDarray* recolor_feature_heights_for_RGB_colormap(
      twoDarray const *ftwoDarray_ptr);

// ALIRT tile methods:

   void AlirtTileLabelToLonLat(std::string tile_label,double& lon,double& lat);
   std::string LonLatToAlirtTileLabel(double& lon,double& lat);
   std::vector<std::string> AlirtTilesInBbox(
      const geopoint& lower_left,const geopoint& upper_right);

}

#endif // ladarfuncs.h




