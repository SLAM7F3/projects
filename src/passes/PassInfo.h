// ==========================================================================
// Header file for PassInfo class which holds non-pass information
// needed to display the data contents of a pass.
// ==========================================================================
// Last updated on 7/3/13; 8/5/13; 8/12/13
// ==========================================================================

#ifndef PASSINFO_H
#define PASSINFO_H

#include <iostream>
#include <string>
#include "math/basic_math.h"
#include "math/genmatrix.h"
#include "math/rpy.h"
#include "math/threevector.h"
#include "math/fourvector.h"
#include "datastructures/Triple.h"

class PassInfo
{

  public:

   PassInfo();
   PassInfo(const PassInfo& pi);
   ~PassInfo();

   PassInfo& operator= (const PassInfo& pi);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const PassInfo& p);

// Set & get methods:

   void set_portrait_mode_flag(bool flag);
   bool get_portrait_mode_flag() const;
   void set_ID(int i);
   int get_ID() const;
   void set_OSGsubPAT_ID(int i);
   int get_OSGsubPAT_ID() const;

   void set_start_argument_index(int i);
   void set_stop_argument_index(int i);
   int get_start_argument_index() const;
   int get_stop_argument_index() const;

   void set_height_colormap_number(int n);
   void set_prob_colormap_number(int n);
   int get_height_colormap_number() const;
   int get_prob_colormap_number() const;

   void set_independent_variable(int i);
   int get_independent_variable() const;

   void set_height_colormap_cyclic_fraction_offset(double f);
   double get_height_colormap_cyclic_fraction_offset() const;
   void set_prob_colormap_cyclic_fraction_offset(double f);
   double get_prob_colormap_cyclic_fraction_offset() const;
   void set_min_threshold(int i,double t);
   void set_max_threshold(int i,double t);
   double get_min_threshold(int i) const;
   double get_max_threshold(int i) const;

   void set_min_threshold_fraction(double f);
   void set_max_threshold_fraction(double f);
   double get_min_threshold_fraction() const;
   double get_max_threshold_fraction() const;
   
   void set_longitude_lo(double l);
   void set_longitude_hi(double l);
   void set_latitude_lo(double l);
   void set_latitude_hi(double l);
   void set_easting_lo(double l);
   void set_easting_hi(double l);
   void set_northing_lo(double l);
   void set_northing_hi(double l);
   void set_altitude(double a);

   double get_longitude_lo() const;
   double get_longitude_hi() const;
   double get_latitude_lo() const;
   double get_latitude_hi() const;

   double get_easting_lo() const;
   double get_easting_hi() const;
   double get_northing_lo() const;
   double get_northing_hi() const;
   double get_altitude() const;

   void set_UTM_zonenumber(int zone);
   void set_northern_hemisphere_flag(bool flag);
   int get_UTM_zonenumber() const;
   bool get_northern_hemisphere_flag() const;

   void set_elapsed_secs_since_epoch_lo(double e);
   void set_elapsed_secs_since_epoch_hi(double e);
   double get_elapsed_secs_since_epoch_lo() const;
   double get_elapsed_secs_since_epoch_hi() const;

   void set_height_offset(double h);
   double get_height_offset() const;

   void set_start_frame_ID(int ID);
   void set_stop_frame_ID(int ID);
   void set_photo_ID(int ID);
   void set_focal_length(double f);
   void set_Uaxis_focal_length(double f);
   void set_Vaxis_focal_length(double f);
   void set_U0(double U0);
   void set_V0(double V0);
   void set_pixel_skew_angle(double theta);
   void set_relative_az(double a);
   void set_relative_el(double e);
   void set_relative_roll(double r);
   void set_camera_longitude(double lon);
   void set_camera_latitude(double lat);
   void set_camera_x_posn(double x);
   void set_camera_y_posn(double y);
   void set_camera_z_posn(double z);

   void set_frustum_color(std::string color);
   void set_frustum_sidelength(double d);
   void set_downrange_distance(double d);
   void set_imageplane_x(double x);
   void set_imageplane_y(double y);
   void set_imageplane_z(double z);
   void set_imageplane_w(double w);

   void set_magnetic_yaw(double y);
   void set_filter_alpha_value(double alpha);

   void set_PostGIS_hostname(std::string input_name);
   void set_PostGIS_database_name(std::string input_name);
   void set_PostGIS_username(std::string input_name);
   void pushback_gispoints_tablename(std::string filename);
   void pushback_gislines_tablename(std::string filename);
   void pushback_gispolys_tablename(std::string filename);

   void set_package_subdir(std::string subdir);
   std::string get_package_subdir() const;
   void set_package_filename_prefix(std::string prefix);
   std::string get_package_filename_prefix() const;

// Communications set/get member functions:

   void set_ActiveMQ_hostname(std::string input_name);
   std::string get_ActiveMQ_hostname() const;

   int get_start_frame_ID() const;
   int get_stop_frame_ID() const;
   int get_photo_ID() const;
   double get_focal_length() const;
   double get_Uaxis_focal_length() const;
   double get_Vaxis_focal_length() const;
   double get_U0() const;
   double get_V0() const;
   double get_pixel_skew_angle() const;
   double get_relative_az() const;
   double get_relative_el() const;
   double get_relative_roll() const;
   double get_camera_longitude() const;
   double get_camera_latitude() const;
   const threevector& get_camera_posn() const;

   std::string get_frustum_color() const;
   double get_frustum_sidelength() const;
   double get_downrange_distance() const;
   fourvector get_imageplane_pi() const;

   double get_magnetic_yaw() const;
   double get_filter_alpha_value() const;
   std::string get_PostGIS_hostname() const;
   std::string get_PostGIS_database_name() const;
   std::string get_PostGIS_username() const;
   const std::vector<std::string>& get_gispoints_tablenames() const;
   const std::vector<std::string>& get_gislines_tablenames() const;
   const std::vector<std::string>& get_gispolys_tablenames() const;

   void pushback_video_corner_vertex(const threevector& V);
   std::vector<threevector>& get_video_corner_vertices();
   const std::vector<threevector>& get_video_corner_vertices() const;
   threevector& get_video_corner_vertex(int c);
   const threevector& get_video_corner_vertex(int c) const;

   void pushback_bbox_top_left_corner(const threevector& V);
   void pushback_bbox_bottom_right_corner(const threevector& V);
   void pushback_bbox_color(std::string color);
   void pushback_bbox_label(std::string label);
   void pushback_bbox_label_color(std::string label_color);

   void set_ROI_skeleton_height(double height);
   void set_ROI_skeleton_color(std::string color);
   
   void pushback_frame_time(const twovector& curr_frame_time);
   void pushback_posn_orientation_frame(
      const Triple<threevector,rpy,int>& curr_posn_orientation_frame);

   std::vector<threevector>& get_bbox_top_left_corners();
   const std::vector<threevector>& get_bbox_top_left_corners() const;
   threevector& get_bbox_top_left_corner(int b);
   const threevector& get_bbox_top_left_corner(int b) const;

   std::vector<threevector>& get_bbox_bottom_right_corners();
   const std::vector<threevector>& get_bbox_bottom_right_corners() const;
   threevector& get_bbox_bottom_right_corner(int b);
   const threevector& get_bbox_bottom_right_corner(int b) const;

   std::vector<std::string>& get_bbox_colors();
   const std::vector<std::string>& get_bbox_colors() const;
   std::string get_bbox_color(int b);
   const std::string get_bbox_color(int b) const;
   std::vector<std::string>& get_bbox_labels();
   const std::vector<std::string>& get_bbox_labels() const;
   std::string get_bbox_label(int b);
   const std::string get_bbox_label(int b) const;
   std::string get_bbox_label_color(int b);
   const std::string get_bbox_label_color(int b) const;

   double get_ROI_skeleton_height() const;
   std::string get_ROI_skeleton_color() const;

   int get_n_frame_times() const;
   std::vector<twovector>& get_frame_times();
   const std::vector<twovector>& get_frame_times() const;
   twovector& get_frame_time(int i);
   const twovector& get_frame_time(int i) const;

   int get_n_posn_orientation_frames() const;
   std::vector<Triple<threevector,rpy,int> >& get_posn_orientation_frames();
   const std::vector<Triple<threevector,rpy,int> >& 
      get_posn_orientation_frames() const;
   Triple<threevector,rpy,int>& get_posn_orientation_frame(int i);
   const Triple<threevector,rpy,int>& get_posn_orientation_frame(int i) const;

   void set_projection_matrix(const genmatrix* proj_ptr);
   genmatrix* get_projection_matrix_ptr();
   const genmatrix* get_projection_matrix_ptr() const;

   void clear_params();

  private:

   bool portrait_mode_flag,northern_hemisphere_flag;
   int ID,OSGsubPAT_ID;
   int start_argument_index,stop_argument_index;
   int height_colormap_number,prob_colormap_number;
   int independent_var;
   int UTM_zonenumber;
   double height_colormap_cyclic_fraction_offset,
      prob_colormap_cyclic_fraction_offset;
   std::vector<double> min_threshold,max_threshold;
   double min_threshold_fraction,max_threshold_fraction;
   double longitude_lo,longitude_hi,latitude_lo,latitude_hi,altitude;
   double easting_lo,easting_hi,northing_lo,northing_hi;
   double elapsed_secs_since_epoch_lo,elapsed_secs_since_epoch_hi;

   int start_frame_ID,stop_frame_ID,photo_ID;
   double height_offset;
   double focal_length,Uaxis_focal_length,Vaxis_focal_length;
   double U0,V0,pixel_skew_angle;
   double relative_az,relative_el,relative_roll;
   double frustum_sidelength,downrange_distance,magnetic_yaw;
   double imageplane_x,imageplane_y,imageplane_z,imageplane_w;
   double filter_alpha_value;

   double camera_longitude,camera_latitude;
   threevector camera_posn;

   std::string frustum_color;
   std::string PostGIS_hostname,PostGIS_database_name,PostGIS_username;
   std::string package_subdir,package_filename_prefix;
   std::vector<std::string> gispoints_tablenames,
      gislines_tablenames,gispolys_tablenames;
   std::string ActiveMQ_hostname;
   std::vector<threevector> video_corner_vertices;
   std::vector<threevector> bbox_top_left_corners,bbox_bottom_right_corners;
   std::vector<std::string> bbox_colors,bbox_labels,bbox_label_colors;
   std::vector<twovector> frame_times;
   std::vector<Triple<threevector,rpy,int> > posn_orientation_frames;

   double ROI_skeleton_height;
   std::string ROI_skeleton_color;

   genmatrix* P_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PassInfo& pi);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void PassInfo::set_portrait_mode_flag(bool flag)
{
   portrait_mode_flag=flag;
}

inline bool PassInfo::get_portrait_mode_flag() const
{
   return portrait_mode_flag;
}

inline void PassInfo::set_ID(int i)
{
   ID=i;
}

inline int PassInfo::get_ID() const
{
   return ID;
}

inline void PassInfo::set_OSGsubPAT_ID(int i)
{
   OSGsubPAT_ID=i;
}

inline int PassInfo::get_OSGsubPAT_ID() const
{
   return OSGsubPAT_ID;
}

inline void PassInfo::set_start_argument_index(int i) 
{
   start_argument_index=i;
}

inline void PassInfo::set_stop_argument_index(int i) 
{
   stop_argument_index=i;
}

inline int PassInfo::get_start_argument_index() const
{
   return start_argument_index;
}

inline int PassInfo::get_stop_argument_index() const
{
   return stop_argument_index;
}

inline void PassInfo::set_height_colormap_number(int n)
{
   height_colormap_number=n;
}

inline void PassInfo::set_prob_colormap_number(int n)
{
   prob_colormap_number=n;
}

inline int PassInfo::get_height_colormap_number() const
{
   return basic_math::max(0,height_colormap_number);
}

inline int PassInfo::get_prob_colormap_number() const
{
   return basic_math::max(0,prob_colormap_number);
}

inline void PassInfo::set_independent_variable(int i)
{
   independent_var=i;
}

inline int PassInfo::get_independent_variable() const
{
   return independent_var;
}

inline void PassInfo::set_height_colormap_cyclic_fraction_offset(double f)
{
   height_colormap_cyclic_fraction_offset=f;
}

inline double PassInfo::get_height_colormap_cyclic_fraction_offset() const
{
   return height_colormap_cyclic_fraction_offset;
}

inline void PassInfo::set_prob_colormap_cyclic_fraction_offset(double f)
{
   prob_colormap_cyclic_fraction_offset=f;
}

inline double PassInfo::get_prob_colormap_cyclic_fraction_offset() const
{
   return prob_colormap_cyclic_fraction_offset;
}

inline void PassInfo::set_min_threshold(int i,double t)
{
   min_threshold[i]=t;
}

inline void PassInfo::set_max_threshold(int i,double t)
{
   max_threshold[i]=t;
}

inline void PassInfo::set_min_threshold_fraction(double f)
{
   min_threshold_fraction=f;
}

inline void PassInfo::set_max_threshold_fraction(double f)
{
   max_threshold_fraction=f;
}

inline double PassInfo::get_min_threshold_fraction() const
{
   return min_threshold_fraction;
}

inline double PassInfo::get_max_threshold_fraction() const
{
   return max_threshold_fraction;
}

inline void PassInfo::set_longitude_lo(double l)
{
   longitude_lo=l;
}

inline void PassInfo::set_longitude_hi(double l)
{
   longitude_hi=l;
}

inline void PassInfo::set_latitude_lo(double l)
{
   latitude_lo=l;
}

inline void PassInfo::set_latitude_hi(double l)
{
   latitude_hi=l;
}

inline void PassInfo::set_easting_lo(double l)
{
   easting_lo=l;
}

inline void PassInfo::set_easting_hi(double l)
{
   easting_hi=l;
}

inline void PassInfo::set_northing_lo(double l)
{
   northing_lo=l;
}

inline void PassInfo::set_northing_hi(double l)
{
   northing_hi=l;
}

inline void PassInfo::set_altitude(double a)
{
   altitude=a;
}

inline double PassInfo::get_longitude_lo() const
{
   return longitude_lo;
}

inline double PassInfo::get_longitude_hi() const
{
   return longitude_hi;
}

inline double PassInfo::get_latitude_lo() const
{
   return latitude_lo;
}

inline double PassInfo::get_latitude_hi() const
{
   return latitude_hi;
}

inline double PassInfo::get_easting_lo() const
{
   return easting_lo;
}

inline double PassInfo::get_easting_hi() const
{
   return easting_hi;
}

inline double PassInfo::get_northing_lo() const
{
   return northing_lo;
}

inline double PassInfo::get_northing_hi() const
{
   return northing_hi;
}

inline double PassInfo::get_altitude() const
{
   return altitude;
}

inline void PassInfo::set_UTM_zonenumber(int zone)
{
   UTM_zonenumber=zone;
}

inline void PassInfo::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline int PassInfo::get_UTM_zonenumber() const
{
   return UTM_zonenumber;
}

inline bool PassInfo::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void PassInfo::set_elapsed_secs_since_epoch_lo(double e)
{
   elapsed_secs_since_epoch_lo=e;
}

inline void PassInfo::set_elapsed_secs_since_epoch_hi(double e)
{
   elapsed_secs_since_epoch_hi=e;
}

inline double PassInfo::get_elapsed_secs_since_epoch_lo() const
{
   return elapsed_secs_since_epoch_lo;
}

inline double PassInfo::get_elapsed_secs_since_epoch_hi() const
{
   return elapsed_secs_since_epoch_hi;
}

inline void PassInfo::set_height_offset(double h) 
{
   height_offset=h;
}

inline double PassInfo::get_height_offset() const
{
   return height_offset;
}

inline void PassInfo::set_start_frame_ID(int ID)
{
   start_frame_ID=ID;
}

inline void PassInfo::set_stop_frame_ID(int ID)
{
   stop_frame_ID=ID;
}

inline void PassInfo::set_photo_ID(int ID)
{
   photo_ID=ID;
}

inline void PassInfo::set_focal_length(double f)
{
   focal_length=f;
}

inline void PassInfo::set_Uaxis_focal_length(double f)
{
   Uaxis_focal_length=f;
}

inline void PassInfo::set_Vaxis_focal_length(double f)
{
   Vaxis_focal_length=f;
}

inline void PassInfo::set_U0(double U0)
{
   this->U0=U0;
}

inline void PassInfo::set_V0(double V0)
{
   this->V0=V0;
}

inline void PassInfo::set_pixel_skew_angle(double theta)
{
   pixel_skew_angle=theta; // pixel_skew_angle measured in radians
}

inline void PassInfo::set_relative_az(double a)
{
   relative_az=a;
}

inline void PassInfo::set_relative_el(double e)
{
   relative_el=e;
}

inline void PassInfo::set_relative_roll(double r)
{
   relative_roll=r;
}

inline void PassInfo::set_camera_longitude(double lon)
{
   camera_longitude=lon;
}

inline void PassInfo::set_camera_latitude(double lat)
{
   camera_latitude=lat;
}

inline void PassInfo::set_camera_x_posn(double x)
{
   camera_posn.put(0,x);
}

inline void PassInfo::set_camera_y_posn(double y)
{
   camera_posn.put(1,y);
}

inline void PassInfo::set_camera_z_posn(double z)
{
   camera_posn.put(2,z);
}

inline void PassInfo::set_frustum_color(std::string color)
{
   frustum_color=color;
}

inline void PassInfo::set_frustum_sidelength(double d)
{
   frustum_sidelength=d;
}

inline void PassInfo::set_downrange_distance(double d)
{
   downrange_distance=d;
}

inline void PassInfo::set_imageplane_x(double x)
{
   imageplane_x=x;
}

inline void PassInfo::set_imageplane_y(double y)
{
   imageplane_y=y;
}

inline void PassInfo::set_imageplane_z(double z)
{
   imageplane_z=z;
}

inline void PassInfo::set_imageplane_w(double w)
{
   imageplane_w=w;
}

inline void PassInfo::set_magnetic_yaw(double y)
{
   magnetic_yaw=y;
}

inline void PassInfo::set_filter_alpha_value(double alpha)
{
   filter_alpha_value=alpha;
}

inline void PassInfo::set_PostGIS_hostname(std::string input_name)
{
   PostGIS_hostname=input_name;
}

inline void PassInfo::set_PostGIS_database_name(std::string input_name)
{
   PostGIS_database_name=input_name;
}

inline void PassInfo::set_PostGIS_username(std::string input_name)
{
   PostGIS_username=input_name;
}

inline void PassInfo::pushback_gispoints_tablename(std::string filename)
{
   gispoints_tablenames.push_back(filename);
}

inline void PassInfo::pushback_gislines_tablename(std::string filename)
{
   gislines_tablenames.push_back(filename);
}

inline void PassInfo::pushback_gispolys_tablename(std::string filename)
{
   gispolys_tablenames.push_back(filename);
}


inline void PassInfo::set_package_subdir(std::string subdir)
{
   package_subdir=subdir;
}

inline std::string PassInfo::get_package_subdir() const
{
   return package_subdir;
}

inline void PassInfo::set_package_filename_prefix(std::string prefix)
{
   package_filename_prefix=prefix;
}

inline std::string PassInfo::get_package_filename_prefix() const
{
   return package_filename_prefix;
}



inline void PassInfo::set_ActiveMQ_hostname(std::string input_name)
{
   ActiveMQ_hostname=input_name;
}

inline int PassInfo::get_start_frame_ID() const
{
   return start_frame_ID;
}

inline int PassInfo::get_stop_frame_ID() const
{
   return stop_frame_ID;
}

inline int PassInfo::get_photo_ID() const
{
   return photo_ID;
}

inline double PassInfo::get_focal_length() const
{
   return focal_length;
}

inline double PassInfo::get_Uaxis_focal_length() const
{
   return Uaxis_focal_length;
}

inline double PassInfo::get_Vaxis_focal_length() const
{
   return Vaxis_focal_length;
}

inline double PassInfo::get_U0() const
{
   return U0;
}

inline double PassInfo::get_V0() const
{
   return V0;
}

inline double PassInfo::get_pixel_skew_angle() const
{
   return pixel_skew_angle;
}

inline double PassInfo::get_relative_az() const
{
   return relative_az;
}

inline double PassInfo::get_relative_el() const
{
   return relative_el;
}

inline double PassInfo::get_relative_roll() const
{
   return relative_roll;
}

inline double PassInfo::get_camera_longitude() const
{
   return camera_longitude;
}

inline double PassInfo::get_camera_latitude() const
{
   return camera_latitude;
}

inline const threevector& PassInfo::get_camera_posn() const
{
   return camera_posn;
}

inline std::string PassInfo::get_frustum_color() const
{
   return frustum_color;
}

inline double PassInfo::get_frustum_sidelength() const
{
   return frustum_sidelength;
}

inline double PassInfo::get_downrange_distance() const
{
   return downrange_distance;
}

inline fourvector PassInfo::get_imageplane_pi() const
{
   return fourvector(imageplane_x,imageplane_y,imageplane_z,imageplane_w);
}

inline double PassInfo::get_magnetic_yaw() const
{
   return magnetic_yaw;
}

inline double PassInfo::get_filter_alpha_value() const
{
   return filter_alpha_value;
}

inline std::string PassInfo::get_PostGIS_hostname() const
{
   return PostGIS_hostname;
}

inline std::string PassInfo::get_PostGIS_database_name() const
{
   return PostGIS_database_name;
}

inline std::string PassInfo::get_PostGIS_username() const
{
   return PostGIS_username;
}

inline const std::vector<std::string>& PassInfo::get_gispoints_tablenames() 
   const
{
   return gispoints_tablenames;
}

inline const std::vector<std::string>& PassInfo::get_gislines_tablenames() 
   const
{
   return gislines_tablenames;
}

inline const std::vector<std::string>& PassInfo::get_gispolys_tablenames() 
   const
{
   return gispolys_tablenames;
}

inline std::string PassInfo::get_ActiveMQ_hostname() const
{
   return ActiveMQ_hostname;
}

inline genmatrix* PassInfo::get_projection_matrix_ptr() 
{
   return P_ptr;
}

inline const genmatrix* PassInfo::get_projection_matrix_ptr() const
{
   return P_ptr;
}

inline void PassInfo::pushback_video_corner_vertex(const threevector& V)
{
   video_corner_vertices.push_back(V);
}

inline std::vector<threevector>& PassInfo::get_video_corner_vertices()
{
   return video_corner_vertices;
}

inline const std::vector<threevector>& PassInfo::get_video_corner_vertices() const
{
   return video_corner_vertices;
}

inline threevector& PassInfo::get_video_corner_vertex(int c)
{
   return video_corner_vertices[c];
}

inline const threevector& PassInfo::get_video_corner_vertex(int c) const
{
   return video_corner_vertices[c];
}

inline void PassInfo::pushback_bbox_top_left_corner(const threevector& V)
{
   bbox_top_left_corners.push_back(V);
}

inline void PassInfo::pushback_bbox_bottom_right_corner(const threevector& V)
{
   bbox_bottom_right_corners.push_back(V);
}

inline void PassInfo::pushback_bbox_color(std::string color)
{
   bbox_colors.push_back(color);
}

inline void PassInfo::pushback_bbox_label(std::string label)
{
   bbox_labels.push_back(label);
}

inline void PassInfo::pushback_bbox_label_color(std::string color)
{
   bbox_label_colors.push_back(color);
}

inline void PassInfo::set_ROI_skeleton_height(double height)
{
   ROI_skeleton_height=height;
}

inline void PassInfo::set_ROI_skeleton_color(std::string color)
{
   ROI_skeleton_color=color;
}

inline void PassInfo::pushback_frame_time(const twovector& curr_frame_time)
{
   frame_times.push_back(curr_frame_time);
}

inline void PassInfo::pushback_posn_orientation_frame(
   const Triple<threevector,rpy,int>& curr_posn_orientation_frame)
{
   posn_orientation_frames.push_back(curr_posn_orientation_frame);
}

inline std::vector<threevector>& PassInfo::get_bbox_top_left_corners()
{
   return bbox_top_left_corners;
}

inline const std::vector<threevector>& PassInfo::get_bbox_top_left_corners() const
{
   return bbox_top_left_corners;
}

inline threevector& PassInfo::get_bbox_top_left_corner(int b)
{
   return bbox_top_left_corners[b];
}

inline const threevector& PassInfo::get_bbox_top_left_corner(int b) const
{
   return bbox_top_left_corners[b];
}


inline std::vector<threevector>& PassInfo::get_bbox_bottom_right_corners()
{
   return bbox_bottom_right_corners;
}

inline const std::vector<threevector>& PassInfo::get_bbox_bottom_right_corners() const
{
   return bbox_bottom_right_corners;
}

inline threevector& PassInfo::get_bbox_bottom_right_corner(int b)
{
   return bbox_bottom_right_corners[b];
}

inline const threevector& PassInfo::get_bbox_bottom_right_corner(int b) const
{
   return bbox_bottom_right_corners[b];
}

inline std::vector<std::string>& PassInfo::get_bbox_colors()
{
   return bbox_colors;
}

inline const std::vector<std::string>& PassInfo::get_bbox_colors() const
{
   return bbox_colors;
}

inline std::string PassInfo::get_bbox_color(int b)
{
   return bbox_colors[b];
}

inline const std::string PassInfo::get_bbox_color(int b) const
{
   return bbox_colors[b];
}

inline std::vector<std::string>& PassInfo::get_bbox_labels()
{
   return bbox_labels;
}

inline const std::vector<std::string>& PassInfo::get_bbox_labels() const
{
   return bbox_labels;
}

inline std::string PassInfo::get_bbox_label(int b)
{
   return bbox_labels[b];
}

inline const std::string PassInfo::get_bbox_label(int b) const
{
   return bbox_labels[b];
}

inline std::string PassInfo::get_bbox_label_color(int b)
{
   return bbox_label_colors[b];
}

inline const std::string PassInfo::get_bbox_label_color(int b) const
{
   return bbox_label_colors[b];
}

inline double PassInfo::get_ROI_skeleton_height() const
{
   return ROI_skeleton_height;
}

inline std::string PassInfo::get_ROI_skeleton_color() const
{
   return ROI_skeleton_color;
}

inline int PassInfo::get_n_frame_times() const
{
   return frame_times.size();
}

inline std::vector<twovector>& PassInfo::get_frame_times()
{
   return frame_times;
}

inline const std::vector<twovector>& PassInfo::get_frame_times() const
{
   return frame_times;
}

inline twovector& PassInfo::get_frame_time(int i)
{
   return frame_times[i];
}

inline const twovector& PassInfo::get_frame_time(int i) const
{
   return frame_times[i];
}

inline int PassInfo::get_n_posn_orientation_frames() const
{
   return posn_orientation_frames.size();
}

inline std::vector<Triple<threevector,rpy,int> >& 
PassInfo::get_posn_orientation_frames()
{
   return posn_orientation_frames;
}

inline const std::vector<Triple<threevector,rpy,int> >& 
PassInfo::get_posn_orientation_frames() const
{
   return posn_orientation_frames;
}

inline Triple<threevector,rpy,int>& PassInfo::get_posn_orientation_frame(
   int i)
{
   return posn_orientation_frames[i];
}

inline const Triple<threevector,rpy,int>& 
PassInfo::get_posn_orientation_frame(int i) const
{
   return posn_orientation_frames[i];
}


#endif // Pass.h

