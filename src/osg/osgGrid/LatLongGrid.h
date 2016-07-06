// ========================================================================
// Header file for LATLONGGRID class
// ========================================================================
// Last updated on 5/22/09; 5/24/09; 5/25/09; 6/6/09
// ========================================================================

#ifndef LATLONG_GRID_H
#define LATLONG_GRID_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include "osg/Custom3DManipulator.h"
#include "astro_geo/Ellipsoid_model.h"
#include "astro_geo/geopoint.h"
#include "osg/osgGrid/Grid.h"
#include "numerical/param_range.h"
#include "osg/osgGeometry/PolyLinesGroup.h"

class Clock;
class Pass;
class ViewFrustum;

class LatLongGrid : public Grid
{
  public:
        
   typedef std::map<threevector,std::pair<PolyLine*,int>,ltthreevector > 
      LINES_MAP;

   LatLongGrid(Pass* pass_ptr,osgGA::Custom3DManipulator* CM_3D_ptr=NULL);
   LatLongGrid(Pass* pass_ptr,int ndims,int ID=-1,
               osgGA::Custom3DManipulator* CM_3D_ptr=NULL);
   virtual ~LatLongGrid();
   friend std::ostream& operator<< (
      std::ostream& outstream,const LatLongGrid& g);

// Set & get member functions:

   void set_flat_grid_flag(bool flag);
   void set_dynamic_grid_flag(bool flag);
   bool get_dynamic_grid_flag() const;
   void set_depth_buffering_off_flag(bool flag);

   void set_world_origin_and_middle();
   void set_long_lat_middle(double long_middle,double lat_middle);
   double get_longitude_middle() const;
   double get_latitude_middle() const;
   param_range* get_longitude_param_range_ptr();
   const param_range* get_longitude_param_range_ptr() const;
   param_range* get_latitude_param_range_ptr();
   const param_range* get_latitude_param_range_ptr() const;

   double get_long_min() const;
   double get_long_start() const;
   double get_long_stop() const;
   double get_lat_start() const;
   double get_lat_stop() const;

   LINES_MAP* get_longitude_lines_map_ptr();
   LINES_MAP* get_latitude_lines_map_ptr();

   PolyLinesGroup* get_LongLinesGroup_ptr();
   PolyLinesGroup* get_LatLinesGroup_ptr();

   void set_delta_latitude(double delta);
   void set_delta_longitude(double delta);
   double get_delta_latitude() const;
   double get_delta_longitude() const;
   geopoint get_LOS_intersection_geopoint();
   const geopoint get_LOS_intersection_geopoint() const;
   virtual threevector get_north_hat();
   virtual const threevector get_north_hat() const;

   void set_init_linewidth(double il);
   void set_text_size_prefactor(double prefactor);

   Ellipsoid_model* get_Ellipsoid_model_ptr();
   void set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_3D_ptr);

// Grid initialization member functions:

   void initialize(
      int ZoneNumber,bool northern_flag,
      double min_east,double max_east,
      double min_north,double max_north,double min_Z);
   void initialize(
      double min_long,double max_long,
      double min_lat,double max_lat,double min_Z);
   virtual void update_grid();
   virtual void update_grid_text_color();
   void compute_relative_latlong_lines();

// Long-lat to UTM conversion member functions:

   double get_or_compute_UTM_to_latlong_gridlines_rot_angle();

// Dynamic grid computation member functions:

   void initialize_extremal_longitudes_and_latitudes();
   void compute_polyline_vertex_angular_separation(
      double& theta_start,double& theta_stop,
      double min_theta,double max_theta,int& n_theta_bins,double& d_theta);
   void angular_separation_between_lines(
      double log_eye_center_dist,int& n_precision,
      double& delta_theta,double& text_size,double& linewidth);
   bool compute_LOS_intersection_geopoint(
      const threevector& camera_ECI_posn,const threevector& camera_Zhat,
      Clock* Clock_ptr,double& camera_LOS_intercept_dist);
   std::vector<geopoint>& compute_FOV_intersection_geopoints(
      Clock* Clock_ptr);
   std::vector<geopoint>& compute_FOV_intersection_geopoints();

   void set_longitude_line_spacing(
      double log_eye_center_dist,
      int& n_long,int& n_lat,double& d_lat,
      int& n_precision,double& text_size,double& linewidth);
   void set_latitude_line_spacing(
      double log_eye_center_dist,
      int& n_lat,int& n_long,double& d_long,
      int& n_precision,double& text_size,double& linewidth);
   void adjust_endpoint_altitudes(
      double long1,double lat1,double& alt1,
      double long2,double lat2,double& alt2,
      double eye_alt,threevector& V1,threevector& V2);
   void update_earthline_text_size(
      double eye_alt,PolyLine* PolyLine_ptr,double text_size);
   void update_lines_map(
      int curr_iter,PolyLinesGroup* PolyLinesGroup_ptr,LINES_MAP* map_ptr);

   void redraw_long_lat_lines(double log_eye_alt,Clock* Clock_ptr);
   virtual void redraw_long_lat_lines(bool refresh_flag=false);
   void destroy_dynamic_grid_lines(
      PolyLinesGroup* PolyLinesGroup_ptr,LINES_MAP* map_ptr);
   virtual void destroy_dynamic_grid_lines();
   void toggle_dynamic_LongLatLines();
   void turn_off_dynamic_LongLatLines();
   void turn_on_dynamic_LongLatLines();

   geopoint ray_geopoint_intercept_on_grid(const threevector& r_hat);
   void compute_north_direction(double eye_alt);

  private:

   bool flat_grid_flag,depth_buffering_off_flag;
   bool northern_hemisphere_flag,dynamic_grid_flag;
   int UTM_zonenumber,ndigits_after_decimal;
   int curr_update_counter;
   double longitude_middle,latitude_middle;
   double UTM_grid_angle_wrt_LL_grid;
   double init_linewidth,text_size_prefactor;
   Pass* pass_ptr;
   Ellipsoid_model* Ellipsoid_model_ptr;
   osgGA::Custom3DManipulator* CM_3D_ptr;

   param_range *longitude_ptr,*latitude_ptr;
   std::vector<double> UTM_to_longitude_rot_angle,UTM_to_latitude_rot_angle;

   double delta_longitude,delta_latitude; 
   double long_min,long_start,long_stop,lat_start,lat_stop;
   threevector north_hat;
   LINES_MAP *longitude_lines_map_ptr,*latitude_lines_map_ptr;
   PolyLinesGroup *LongLinesGroup_ptr,*LatLinesGroup_ptr;
   geopoint LOS_intersection_geopoint;
   std::vector<geopoint> FOV_intersection_geopoint;

   void allocate_member_objects();
   void initialize_member_objects();

   void compute_long_lat_param_ranges(
      double min_long,double max_long,double min_lat,double max_lat);
   void crop_latlong_bounds();
   void set_text_character_sizes();
   osg::Vec4 get_dynamic_grid_color() const;

   void draw_longitude_lines(
      double log_eye_center_dist,double eye_alt,
      int curr_iter,double annotation_altitude,bool refresh_flag);
   void draw_latitude_lines(
      double log_eye_center_dist,double eye_alt,
      int curr_iter,double annotation_altitude,bool refresh_flag);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void LatLongGrid::set_dynamic_grid_flag(bool flag)
{
   dynamic_grid_flag=flag;
}

inline void LatLongGrid::set_flat_grid_flag(bool flag)
{
   flat_grid_flag=flag;
}

inline bool LatLongGrid::get_dynamic_grid_flag() const
{
   return dynamic_grid_flag;
}

inline void LatLongGrid::set_depth_buffering_off_flag(bool flag)
{
   depth_buffering_off_flag=flag;
}

inline const param_range* LatLongGrid::get_longitude_param_range_ptr() const
{
   return longitude_ptr;
}

inline param_range* LatLongGrid::get_longitude_param_range_ptr() 
{
   return longitude_ptr;
}

inline const param_range* LatLongGrid::get_latitude_param_range_ptr() const
{
   return latitude_ptr;
}

inline param_range* LatLongGrid::get_latitude_param_range_ptr() 
{
   return latitude_ptr;
}

inline void LatLongGrid::set_long_lat_middle(
   double long_middle,double lat_middle)
{
   longitude_middle=long_middle;
   latitude_middle=lat_middle;
}

inline double LatLongGrid::get_longitude_middle() const
{
   return longitude_middle;
}

inline double LatLongGrid::get_latitude_middle() const
{
   return latitude_middle;
}

inline double LatLongGrid::get_long_min() const
{
   return long_min;
}

inline double LatLongGrid::get_long_start() const
{
   return long_start;
}

inline double LatLongGrid::get_long_stop() const
{
   return long_stop;
}

inline double LatLongGrid::get_lat_start() const
{
   return lat_start;
}

inline double LatLongGrid::get_lat_stop() const
{
   return lat_stop;
}

inline LatLongGrid::LINES_MAP* LatLongGrid::get_longitude_lines_map_ptr()
{
   return longitude_lines_map_ptr;
}

inline LatLongGrid::LINES_MAP* LatLongGrid::get_latitude_lines_map_ptr()
{
   return latitude_lines_map_ptr;
}

inline PolyLinesGroup* LatLongGrid::get_LongLinesGroup_ptr()
{
   return LongLinesGroup_ptr;
}

inline PolyLinesGroup* LatLongGrid::get_LatLinesGroup_ptr()
{
   return LatLinesGroup_ptr;
}

inline void LatLongGrid::set_delta_latitude(double delta)
{
   delta_latitude=delta;
}

inline void LatLongGrid::set_delta_longitude(double delta)
{
   delta_longitude=delta;
}

inline double LatLongGrid::get_delta_longitude() const
{
   return delta_longitude;
}

inline double LatLongGrid::get_delta_latitude() const
{
   return delta_latitude;
}

inline geopoint LatLongGrid::get_LOS_intersection_geopoint()
{
   return LOS_intersection_geopoint;
}

inline const geopoint LatLongGrid::get_LOS_intersection_geopoint() const
{
   return LOS_intersection_geopoint;
}

inline threevector LatLongGrid::get_north_hat()
{
   return north_hat;
}

inline const threevector LatLongGrid::get_north_hat() const
{
   return north_hat;
}

inline void LatLongGrid::set_init_linewidth(double il)
{
   init_linewidth=il;
}

inline void LatLongGrid::set_text_size_prefactor(double prefactor)
{
   text_size_prefactor=prefactor;
}

inline Ellipsoid_model* LatLongGrid::get_Ellipsoid_model_ptr()
{
   return Ellipsoid_model_ptr;
}

inline void LatLongGrid::set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_3D_ptr)
{
   this->CM_3D_ptr=CM_3D_ptr;
}



#endif
