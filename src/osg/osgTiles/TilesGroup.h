// ==========================================================================
// Header file for TilesGroup class.  
// ==========================================================================
// Last updated on 10/1/11; 10/2/11; 10/14/11
// ==========================================================================

#ifndef TILESGROUP_H
#define TILESGROUP_H

#include <iostream>
#include <string>
#include <vector>
#include "geometry/bounding_box.h"
#include "astro_geo/geopoint.h"
#include "math/threevector.h"
#include "math/twovector.h"
#include "image/TwoDarray.h"

class AnimationController;
class raster_parser;

class TilesGroup
{

  public:

   TilesGroup();
   ~TilesGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const TilesGroup& tg);

// Set & get member functions:

   void set_ladar_height_data_flag(bool flag);
   bool get_ladar_height_data_flag() const;
   bool get_new_latlong_points_inside_polygon_flag() const;
   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   void set_specified_UTM_zonenumber(int zone);
   int get_specified_UTM_zonenumber() const;
   void set_min_long(double min_longitude);
   double get_min_long() const;
   void set_max_long(double max_longitude);
   double get_max_long() const;
   void set_min_lat(double min_latitude);
   double get_min_lat() const;
   void set_max_lat(double max_latitude);
   double get_max_lat() const;
   void set_geotif_subdir(std::string subdir);
   void set_geotif_Ztiles_subdir(std::string subdir);
   std::string get_geotif_Ptiles_subdir() const;
   std::string get_webapps_outputs_subdir() const;
   void set_avg_LOS_png_files_ready_flag(bool flag);
   bool get_avg_LOS_png_files_ready_flag() const;
   const std::vector<std::string>& get_avg_LOS_png_filenames() const;
   const std::vector<geopoint>& get_png_lower_left_corners() const;
   const std::vector<geopoint>& get_png_upper_right_corners() const;
   void set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr);
   twoDarray* get_DTED_ztwoDarray_ptr();
   const twoDarray* get_DTED_ztwoDarray_ptr() const;
   void set_reduced_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr);
   twoDarray* get_reduced_DTED_ztwoDarray_ptr();
   const twoDarray* get_reduced_DTED_ztwoDarray_ptr() const;
   void set_tomcat_subdir(std::string subdir);
   int get_terrain_reduction_scale_factor();

// Line-of-sight tiling member functions:

   std::vector<threevector>& 
      individual_latlong_tiles_intercepting_ground_bbox(
         const geopoint& vertex0,double theta_min,double theta_max,
         bounding_box* ground_bbox_ptr);
   std::vector<threevector>& individual_latlong_tiles_intercepting_polygon(
      const geopoint& vertex1,const geopoint& vertex2,
      const geopoint& vertex3,const threevector& apex,double max_range);
   twoDarray*& load_DTED_subtiles_overlapping_polygon_into_ztwoDarray(
      const std::vector<threevector>& latlong_points_inside_polygon,
      double delta_x,double delta_y);
   twoDarray* generate_reduced_DTED_ztwoDarray();
   twoDarray* generate_subtile_twoDarray(
      double delta_x,double delta_y,
      geopoint& lower_left_corner,geopoint& upper_right_corner);
   twoDarray* generate_subtile_twoDarray(
      double delta_x,double delta_y,
      double x_lo,double x_hi,double y_lo,double y_hi);
   bool read_geotif_subtile_height_data(
      std::string geotif_filename,twoDarray* ztwoDarray_ptr);

   void update_avg_LOS_tiles(
      double delta_x,double delta_y,
      std::vector<threevector>& latlong_points_inside_triangle,
      twoDarray* DTED_ptwoDarray_ptr,
      AnimationController* AnimationController_ptr);
   void purge_tile_files();
   std::string export_avg_ground_bbox_LOS(
      int min_lon,int max_lon,int min_lat,int max_lat);
   std::string convert_avg_geotifs_from_greyscale_to_color(
      const std::vector<std::string>& avg_LOS_tif_filenames);

// Ladar height data member functions:

   std::string get_ladar_geotif_filename();
   twoDarray*& load_ladar_height_data_into_ztwoDarray();
   twoDarray*& load_ladar_height_data_into_ztwoDarray(
      std::string ladar_geotif_filename);
   twoDarray*& load_ladar_height_data_into_ztwoDarray(
      double delta_x,double delta_y);
   twoDarray*& load_ladar_height_data_into_ztwoDarray(
      std::string ladar_geotif_filename,double delta_x,double delta_y);

   void set_ladar_data_bbox(
      double xmin,double xmax,double ymin,double ymax,double dx,double dy);
   bool get_ladar_z_given_xy(double x,double y,double& z);

// SRTM height data member functions

   bool get_SRTM_nadir_z_given_lon_lat(double lon,double lat,double& z);
   twoDarray*& load_nine_SRTM_tiles_into_ztwoDarray(
      double longitude,double latitude,double delta_x,double delta_y,
      bool& some_data_imported_flag);
   bool estimate_SRTM_z_given_aerial_pt_and_ray(
      const geopoint& aerial_point,const threevector& r_hat,double& Z_ground);
   bool estimate_SRTM_ground_intercept_given_aerial_pt_and_ray(
      const geopoint& aerial_point,const threevector& r_hat,
      geopoint& ground_intercept_point);

// Skymap generation member functions:

   twoDarray*& load_all_DTED_tiles(
      int n_extra_degs,double raytrace_cellsize,
      const std::vector<twovector>& target_posns);
   twoDarray*& load_all_DTED_tiles(
      int n_extra_degs,double raytrace_cellsize,
      const geopoint& lower_left_corner,const geopoint& upper_right_corner);
   twoDarray*& load_single_DTED_tile_into_ztwoDarray(
      double longitude,double latitude,double delta_x,double delta_y,
      bool& some_data_imported_flag);

  private:

   bool ladar_height_data_flag;
   bool northern_hemisphere_flag,avg_LOS_png_files_ready_flag;
   bool new_latlong_points_inside_polygon_flag;
   int min_long,max_long,min_lat,max_lat;
   int specified_UTM_zonenumber;
   int terrain_reduction_scale_factor;
   int prev_m,prev_n;
   double prev_lon,prev_lat;
   std::string geotif_subdir,geotif_Ztiles_subdir,geotif_Ptiles_subdir;
   std::string tomcat_subdir,webapps_outputs_subdir;
   std::vector<std::string> avg_LOS_png_filenames;
   std::vector<threevector> latlong_points_inside_polygon,
      prev_latlong_points_inside_polygon;
   std::vector<geopoint> png_lower_left_corners,png_upper_right_corners;
   twoDarray *DTED_ztwoDarray_ptr,*reduced_DTED_ztwoDarray_ptr;

   std::string prev_ladar_geotif_filename,prev_SRTM_geotif_filename;
   bounding_box ladar_data_bbox,SRTM_data_bbox;
   raster_parser* RasterParser_ptr;

   void latlong_bbox_corners_intercepting_polygon(
      const std::vector<threevector>& latlong_points_inside_polygon,
      geopoint& lower_left_corner,geopoint& upper_right_corner);
   bool read_geotif_subtiles_overlapping_polygon(
      const std::vector<threevector>& latlong_points_inside_polygon,
      twoDarray* ztwoDarray_ptr);

   void allocate_member_objects();
   void initialize_member_objects();
   bool get_FOB_Blessing_ladar_z_given_xy(double x,double y,double& z);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void TilesGroup::set_ladar_height_data_flag(bool flag)
{
   ladar_height_data_flag=flag;
}

inline bool TilesGroup::get_ladar_height_data_flag() const
{
   return ladar_height_data_flag;
}

inline bool TilesGroup::get_new_latlong_points_inside_polygon_flag() const
{
   return new_latlong_points_inside_polygon_flag;
}

inline void TilesGroup::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool TilesGroup::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void TilesGroup::set_specified_UTM_zonenumber(int zone)
{
   specified_UTM_zonenumber=zone;
}

inline int TilesGroup::get_specified_UTM_zonenumber() const
{
   return specified_UTM_zonenumber;
}

inline void TilesGroup::set_min_long(double min_longitude)
{
   min_long=min_longitude;
}

inline double TilesGroup::get_min_long() const
{
   return min_long;
}

inline void TilesGroup::set_max_long(double max_longitude)
{
   max_long=max_longitude;
}

inline double TilesGroup::get_max_long() const
{
   return max_long;
}

inline void TilesGroup::set_min_lat(double min_latitude)
{
   min_lat=min_latitude;
}

inline double TilesGroup::get_min_lat() const
{
   return min_lat;
}

inline void TilesGroup::set_max_lat(double max_latitude)
{
   max_lat=max_latitude;
}

inline double TilesGroup::get_max_lat() const
{
   return max_lat;
}

inline std::string TilesGroup::get_geotif_Ptiles_subdir() const
{
   return geotif_Ptiles_subdir;
}

inline std::string TilesGroup::get_webapps_outputs_subdir() const
{
   return webapps_outputs_subdir;
}

inline void TilesGroup::set_avg_LOS_png_files_ready_flag(bool flag) 
{
   avg_LOS_png_files_ready_flag=flag;
}

inline bool TilesGroup::get_avg_LOS_png_files_ready_flag() const
{
   return avg_LOS_png_files_ready_flag;
}

inline const std::vector<std::string>& TilesGroup::get_avg_LOS_png_filenames()
   const
{
   return avg_LOS_png_filenames;
}

inline const std::vector<geopoint>& TilesGroup::get_png_lower_left_corners() 
   const
{
   return png_lower_left_corners;
}

inline const std::vector<geopoint>& TilesGroup::get_png_upper_right_corners() 
   const
{
   return png_upper_right_corners;
}

inline void TilesGroup::set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr)
{
   DTED_ztwoDarray_ptr=ztwoDarray_ptr;
}

inline twoDarray* TilesGroup::get_DTED_ztwoDarray_ptr()
{
   return DTED_ztwoDarray_ptr;
}

inline const twoDarray* TilesGroup::get_DTED_ztwoDarray_ptr() const
{
   return DTED_ztwoDarray_ptr;
}

inline void TilesGroup::set_reduced_DTED_ztwoDarray_ptr(
   twoDarray* ztwoDarray_ptr)
{
   reduced_DTED_ztwoDarray_ptr=ztwoDarray_ptr;
}

inline twoDarray* TilesGroup::get_reduced_DTED_ztwoDarray_ptr()
{
   return reduced_DTED_ztwoDarray_ptr;
}

inline const twoDarray* TilesGroup::get_reduced_DTED_ztwoDarray_ptr() const
{
   return reduced_DTED_ztwoDarray_ptr;
}

inline void TilesGroup::set_tomcat_subdir(std::string subdir)
{
   tomcat_subdir=subdir;
   webapps_outputs_subdir=tomcat_subdir+"outputs/";
}

#endif // TilesGroup.h

