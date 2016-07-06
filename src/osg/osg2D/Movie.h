// ========================================================================
// Movie class provides functionality for displaying G99
// movie files.
// ========================================================================
// Last updated on 2/27/11; 3/1/11; 3/12/13; 4/1/14
// ========================================================================

#ifndef MOVIE_H
#define MOVIE_H

#include <iostream>
#include <map>
#include <vector>
#include <osg/Geode>
#include <osg/PositionAttitudeTransform>
#include "video/camera.h"
#include "osg/Custom3DManipulator.h"
#include "video/G99VideoDisplay.h"
#include "osg/osgGraphicals/Graphical.h"
#include "geometry/plane.h"
#include "video/photograph.h"
#include "math/threevector.h"
#include "track/track.h"

class AnimationController;
class genmatrix;
class homography;
class polygon;

class Movie : public G99VideoDisplay, public Graphical
{   
  public:

   typedef std::map<int,track*> TRACKS_MAP;


   typedef std::map<int,std::pair<double,std::string> > ANNOTATIONS_MAP;
// independent int = video frame number
// Dependent pair holds epoch time plus string annotation 

   Movie(const int p_ndims,std::string filename,int id,double alpha,
         AnimationController* AC_ptr,bool hide_backface_flag,
         osgGA::Custom3DManipulator* CM_ptr=NULL);
   Movie(const int p_ndims,texture_rectangle* texture_rectangle_ptr,
         int id,double alpha,osgGA::Custom3DManipulator* CM_ptr=NULL);
   virtual ~Movie();
   void destroy_cameras_for_images();
   void clear_camera_ptrs();
   
// Set & get member functions:

   void set_dynamic_window_flag(bool flag);
   bool get_dynamic_window_flag() const;
   void set_warp_onto_imageplane_flag(bool flag);
   bool get_warp_onto_imageplane_flag() const;
   void set_fadeout_start_time(double t);
   void set_fadein_start_time(double t);
   void set_watch_subdirectory(std::string subdir);
   std::string get_watch_subdirectory() const;

   void set_frame_origin(const threevector& origin);
   threevector& get_frame_origin();

   void set_video_corner_UTM_coords(std::vector<threevector>& V);
   std::vector<threevector>& get_video_corner_UTM_coords();
   const std::vector<threevector>& get_video_corner_UTM_coords() const;

   void set_camera_ptr(camera* c_ptr);
   camera* get_camera_ptr(int imagenumber);
   camera* get_camera_ptr();
   osg::Geode* getGeode() const;
   plane* get_imageplane_ptr();
   const plane* get_imageplane_ptr() const;
   void set_photograph_ptr(photograph* photo_ptr);
   photograph* get_photograph_ptr();
   const photograph* get_photograph_ptr() const;

   TRACKS_MAP* get_tracks_map_ptr();
   const TRACKS_MAP* get_tracks_map_ptr() const;
   ANNOTATIONS_MAP* get_annotations_map_ptr();
   const ANNOTATIONS_MAP* get_annotations_map_ptr() const;

   double get_absolute_altitude(double curr_t,int pass_number);

   const threevector& get_bottom_left_XYZ() const;
   const threevector& get_top_left_XYZ() const;
   const threevector& get_top_right_XYZ() const;
   const threevector& get_bottom_right_XYZ() const;

// Camera member functions:

   std::string enter_camera_params_filename();
   void plot_roll_pitch_yaw_vs_time();
   void read_camera_params_for_sequence();

// FOV member functions:

   void set_FOV_scale_attitude_posn(
      const std::vector<double>& curr_time,
      const std::vector<int>& pass_number,const std::vector<threevector>& V1,
      const threevector& grid_origin);
   void approx_project_onto_worldspace_grid(
      double curr_t,int pass_number,
      const threevector& origin_proj,const threevector& Umax_proj,
      const threevector& Vmax_proj,const threevector& grid_origin);
   void project_onto_worldspace_grid(
      double curr_t,int pass_number,const threevector& grid_origin);

// Rotation, scaling and translation member functions:

   void rotate_scale(
      double curr_t,int pass_number,
      const threevector& Uaxis,const threevector& Vaxis);
   void rotate_scale(
      double curr_t,int pass_number,const genmatrix& Rimageplane,
      const twovector& UVsize_in_meters);
   threevector translate(
      double curr_t,int pass_number,const twovector& meters_per_pixel,
      const twovector& center_shift,const twovector& trans,
      double radial_displacement);
   osg::Quat retrieve_camera_quaternion(
      double curr_t,int pass_number,bool align_Vhat_with_zhat=true);
   void transform_UV_to_XYZ_coords(double curr_t,int pass_number);
   void remap_window_corners(
      double rho,const threevector& camera_XYZ,const threevector& khat,
      const std::vector<threevector>& UV_corner_dir);
   void remap_window_corners(
      const threevector& camera_XYZ,plane* image_plane_ptr,
      const std::vector<threevector>& UV_corner_dir);
   void remap_window_corners();
   void remap_orthographic_window_corners(
      double curr_t,int pass_number,
      double k,double u0,double v0,double rho,
      const threevector& uhat,const threevector& vhat);
//   void update_movie_window(double curr_t,int pass_number);
   void set_CM3D_viewpoint(double curr_t,int pass_number);

// Intensity content manipulation member functions:

   void null_region_outside_poly(const polygon& poly);

// Average background image generation member functions:

   void generate_average_background_image();

// Georegistration member functions:

   bool georegister_subtexture_corners(
      double min_longitude,double max_longitude,
      double min_latitude,double max_latitude);
   bool export_current_subframe(
      double min_longitude,double max_longitude,
      double min_latitude,double max_latitude,
      bool draw_central_bbox_flag=false,int n_horiz_output_pixels=-1);

   bool export_current_subframe(
      double min_longitude,double max_longitude,
      double min_latitude,double max_latitude,std::string output_subdir,
      bool draw_central_bbox_flag=false,int n_horiz_output_pixels=-1);
   bool export_current_subframe(
      double min_longitude,double max_longitude,
      double min_latitude,double max_latitude,
      std::string output_subdir,std::string output_filename,
      bool draw_central_bbox_flag=false,int n_horiz_output_pixels=-1);
   void export_current_frame();

// 2D movie superposition member functions:
   
   std::vector<threevector> construct_video_frame_corners() const;
   std::vector<threevector> compute_video_frame_corners_in_panorama(
      int video_width,int video_height,int panorama_width,int panorama_height,
      homography* curr_H_ptr,const std::vector<threevector>& 
      video_frame_corners);
   void superpose_video_frame_on_panorama(
      const std::vector<threevector>& panorama_frame_coords);
   threevector imageplane_location(const twovector& UV);

// Photograph manipulation member functions:

   void reset_displayed_photograph(std::string photo_filename,bool twoD_flag);
   bool warp_photo_onto_imageplane(const plane& imageplane);
   bool warp_photo_onto_imageplane(const plane& imageplane,camera* camera_ptr);
   bool time_dependent_fade_out(double total_fadeout_time);
   bool time_dependent_fade_out(double begin_fall_time,double end_fall_time);
   bool time_dependent_fade_in(double total_fadein_time);
   bool time_dependent_fade_in(double begin_rise_time,double end_rise_time);
   
   bool import_next_to_latest_photo();

// Tracking member functions:

   TRACKS_MAP* generate_ESB_car_tracks();
   TRACKS_MAP* generate_Luke_blob_tracks(int p);

// Video annotation member functions:

   void annotate_curr_frame();
   void export_video_annotations();

  protected:

  private:

   bool dynamic_window_flag,warp_onto_imageplane_flag;
   double fadein_start_time,fadeout_start_time;
   std::string watch_subdirectory;
   threevector frame_origin;
   threevector bottom_left_XYZ,top_left_XYZ,top_right_XYZ,bottom_right_XYZ;
   std::vector<threevector> video_corner_UTM_coords;

   plane* imageplane_ptr;
   photograph* photograph_ptr;
   std::vector<camera*> camera_ptrs;
   TRACKS_MAP* tracks_map_ptr;
   ANNOTATIONS_MAP* annotations_map_ptr;
   osg::ref_ptr<osg::Geode> geode_refptr;
   osg::ref_ptr<osgGA::Custom3DManipulator> CM_3D_refptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void instantiate_cameras_for_images();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Movie::set_dynamic_window_flag(bool flag)
{
   dynamic_window_flag=flag;
}

inline bool Movie::get_dynamic_window_flag() const
{
   return dynamic_window_flag;
}

inline void Movie::set_warp_onto_imageplane_flag(bool flag)
{
   warp_onto_imageplane_flag=flag;
}

inline bool Movie::get_warp_onto_imageplane_flag() const
{
   return warp_onto_imageplane_flag;
}

inline void Movie::set_fadeout_start_time(double t)
{
   fadeout_start_time=t;
}

inline void Movie::set_fadein_start_time(double t)
{
   fadein_start_time=t;
}

inline void Movie::set_watch_subdirectory(std::string subdir)
{
   watch_subdirectory=subdir;
}

inline std::string Movie::get_watch_subdirectory() const
{
   return watch_subdirectory;
}

inline void Movie::set_frame_origin(const threevector& origin)
{
   frame_origin=origin;
}

inline threevector& Movie::get_frame_origin() 
{
   return frame_origin;
}

inline void Movie::set_video_corner_UTM_coords(std::vector<threevector>& V)
{
   video_corner_UTM_coords.clear();
   video_corner_UTM_coords=V;
}

inline std::vector<threevector>& Movie::get_video_corner_UTM_coords()
{
   return video_corner_UTM_coords;
}

inline const std::vector<threevector>& Movie::get_video_corner_UTM_coords() 
   const
{
   return video_corner_UTM_coords;
}

inline osg::Geode* Movie::getGeode() const
{ 
   return geode_refptr.get(); 
}

inline void Movie::set_photograph_ptr(photograph* photo_ptr)
{
   photograph_ptr=photo_ptr;
}

inline photograph* Movie::get_photograph_ptr()
{
   return photograph_ptr;
}

inline const photograph* Movie::get_photograph_ptr() const
{
   return photograph_ptr;
}

inline plane* Movie::get_imageplane_ptr()
{
   return imageplane_ptr;
}

inline const plane* Movie::get_imageplane_ptr() const
{
   return imageplane_ptr;
}

inline Movie::TRACKS_MAP* Movie::get_tracks_map_ptr()
{
   return tracks_map_ptr;
}

inline const Movie::TRACKS_MAP* Movie::get_tracks_map_ptr() const
{
   return tracks_map_ptr;
}

inline Movie::ANNOTATIONS_MAP* Movie::get_annotations_map_ptr()
{
   return annotations_map_ptr;
}

inline const Movie::ANNOTATIONS_MAP* Movie::get_annotations_map_ptr() const
{
   return annotations_map_ptr;
}

inline const threevector& Movie::get_bottom_left_XYZ() const
{
   return bottom_left_XYZ;
}

inline const threevector& Movie::get_top_left_XYZ() const
{
   return top_left_XYZ;
}

inline const threevector& Movie::get_top_right_XYZ() const
{
   return top_right_XYZ;
}

inline const threevector& Movie::get_bottom_right_XYZ() const
{
   return bottom_right_XYZ;
}

#endif
