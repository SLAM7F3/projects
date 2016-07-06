// ==========================================================================
// Header file for OBSFRUSTUM class
// ==========================================================================
// Last updated on 3/4/13; 3/12/13; 4/6/14; 6/7/14
// ==========================================================================

#ifndef OBSFRUSTUM_H
#define OBSFRUSTUM_H

#include <iostream>
#include <set>
#include <vector>
#include <osg/Group>
#include "osg/osgGeometry/ArrowsGroup.h"
#include "geometry/bounding_box.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osg2D/Movie.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PyramidsGroup.h"
#include "osg/osgTiles/ray_tracer.h"
#include "math/rotation.h"
#include "osg/osgTiles/TilesGroup.h"
#include "track/track.h"

class AnimationController;
class Arrow;
class ArrowsGroup;
class camera;
class fourvector;
class Messenger;
class MODEL;
class Pass;
class plane;
class PointCloud;
class PointFinder;
class polygon;
class PolyhedraGroup;
class pyramid;
class raster_parser;
class SignPost;
class SignPostsGroup;
class vertex;

class OBSFRUSTUM : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   OBSFRUSTUM(AnimationController* AC_ptr=NULL,int id=-1);
   OBSFRUSTUM(
      Pass* PI_ptr,double az_extent,double el_extent,
      threevector* grid_world_origin_ptr,AnimationController* AC_ptr=NULL,
      Movie* Movie_ptr=NULL,int id=-1);
   OBSFRUSTUM(
      Pass* PI_ptr,threevector* grid_world_origin_ptr,
      AnimationController* AC_ptr=NULL,Movie* Movie_ptr=NULL,int id=-1);

   virtual ~OBSFRUSTUM();
   friend std::ostream& operator<< (std::ostream& outstream,OBSFRUSTUM& o);

// Set & get methods:

   double get_az_extent() const;
   double get_el_extent() const;
   void set_rectangular_movie_flag(bool flag);
   bool get_rectangular_movie_flag() const;
   void set_virtual_camera_flag(bool flag);
   bool get_virtual_camera_flag() const;
   void set_display_camera_model_flag(
      double curr_t,int pass_number,bool flag);
   void set_portrait_mode_flag(bool flag);
   bool get_portrait_mode_flag() const;

   void set_display_ViewingPyramid_flag(bool flag);
   bool get_display_ViewingPyramid_flag() const;
   void set_display_ViewingPyramidAboveZplane_flag(bool flag);
   bool get_display_ViewingPyramidAboveZplane_flag() const;

   void set_movie_downrange_distance(double d);
   double get_movie_downrange_distance() const;
   void set_FOV_triangle_min_az(double min_az);
   void set_FOV_triangle_max_az(double max_az);

   void set_volume_alpha(double volume_alpha);
   double get_volume_alpha();
   double get_volume_alpha() const;

   PyramidsGroup* get_PyramidsGroup_ptr();
   const PyramidsGroup* get_PyramidsGroup_ptr() const;
   Pyramid* get_last_Pyramid_ptr(); 
   const Pyramid* get_last_Pyramid_ptr() const;

   void set_Movie_ptr(Movie* Movie_ptr);
   Movie* get_Movie_ptr();
   const Movie* get_Movie_ptr() const;
   void set_Movie_visibility_flag(bool flag);

   CylindersGroup* get_CylindersGroup_ptr();
   const CylindersGroup* get_CylindersGroup_ptr() const;
   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   const LineSegmentsGroup* get_LineSegmentsGroup_ptr() const;

   pyramid* get_viewing_pyramid_ptr();
   const pyramid* get_viewing_pyramid_ptr() const;
   pyramid* get_viewing_pyramid_above_zplane_ptr();
   const pyramid* get_viewing_pyramid_above_zplane_ptr() const;
   Pyramid* get_ViewingPyramid_ptr();
   const Pyramid* get_ViewingPyramid_ptr() const;
   Pyramid* get_ViewingPyramidAboveZplane_ptr();
   const Pyramid* get_ViewingPyramidAboveZplane_ptr() const;

   void set_TilesGroup_ptr(TilesGroup* TG_ptr);
   TilesGroup* get_TilesGroup_ptr();
   const TilesGroup* get_TilesGroup_ptr() const;

   void set_ground_bbox_ptr(bounding_box* ground_bbox_ptr);
   twoDarray* get_DTED_ptwoDarray_ptr() const;
   const std::vector<threevector>& get_corner_ray() const;
   std::vector<threevector>& get_projected_polyhedron_points();
   const std::vector<threevector>& get_projected_polyhedron_points() const;
   std::vector<threevector>& get_occluded_ray_impact_points();
   const std::vector<threevector>& get_occluded_ray_impact_points() const;

   bool get_ladar_height_data_flag() const;
   ray_tracer* get_ray_tracer_ptr();
   const ray_tracer* get_ray_tracer_ptr() const;

   ArrowsGroup* get_ArrowsGroup_ptr();
   const ArrowsGroup* get_ArrowsGroup_ptr() const;
   Arrow* get_Arrow_ptr(int a);
   const Arrow* get_Arrow_ptr(int a) const;

// Frustum initialization and construction methods:

   pyramid* generate_viewing_pyramid_ptr();
   pyramid* generate_viewing_pyramid_above_zplane_ptr();
   void instantiate_OSG_Pyramids();
   void generate_Pyramid_geodes();

   void generate_or_reset_viewing_pyramid(
      const threevector& camera_posn,
      const std::vector<threevector>& UV_corner_dir,double sidelength);
   void initialize_frustum_with_movie(
      double& frustum_sidelength,double movie_downrange_distance,
      bool portrait_mode_flag=false);
   double compute_pyramid_sidelength(double movie_downrange_distance);
   double compute_movie_downrange_distance(double pyramid_sidelength);

   pyramid* compute_viewing_pyramid_above_Zplane(
      double z,pyramid* pyramid_ptr);

   void build_current_frustum(
      double curr_t,int pass_number,
      const threevector& camera_posn,const threevector& v_hat,
      double yaw,double pitch,double z_base_face);
   void build_current_frustum(
      double curr_t,int pass_number,const threevector& camera_posn,
      const threevector& n_hat,double z_base_face);
   void build_current_frustum(
      double curr_t,int pass_number,const threevector& camera_posn,
      double phi,double thetay,double z_base_face);

   bool compute_corner_rays(
      const threevector& v_hat,
      double horiz_FOV,double vert_FOV,double roll,double pitch);
   bool compute_corner_rays(
      double alpha,double beta,double roll,double pitch,
      const threevector& v_hat);
   void build_current_frustum(
      double curr_t,int pass_number,
      double roll,double pitch,double z_base_face,
      const threevector& apex_posn,const threevector& v_hat);
   bool build_current_frustum(
      double curr_t,int pass_number,
      const threevector& camera_posn,const threevector& camera_v_hat,
      double alpha,double beta,double roll,double pitch,double z_base_face);

   void build_current_frustum(
      double curr_t,int pass_number,
      const threevector& apex_posn,const std::vector<threevector>& 
      base_vertices);
   void build_current_frustum(
      double curr_t,int pass_number,genmatrix* curr_P_ptr,
      double frustum_sidelength,double filter_alpha_value,
      bool temporally_filter_flag);

   void build_OBSFRUSTUM(
      double curr_t,int pass_number,
      double frustum_sidelength,double movie_downrange_distance,
      const threevector& camera_world_posn,
      const std::vector<threevector>& corner_ray,
      colorfunc::Color OBSFRUSTUM_color,double volume_alpha=0);
   void build_OBSFRUSTUM(
      double curr_t,int pass_number,double Zplane_altitude,
      const threevector& camera_world_posn,
      const std::vector<threevector>& corner_ray,
      colorfunc::Color OBSFRUSTUM_color,double volume_alpha=0);
   void build_OBSFRUSTUM(
      double curr_t,int pass_number,double Zplane_altitude);
   void build_OBSFRUSTUM(
      double curr_t,int pass_number,
      double frustum_sidelength,double movie_downrange_distance,
      const threevector& camera_world_posn,double az,double el,double roll,
      colorfunc::Color OBSFRUSTUM_color,double volume_alpha=0);

   void reset_OBSFRUSTUM(
      double curr_t,int pass_number,
      double frustum_sidelength,double movie_downrange_distance,
      double az,double el,double roll,camera* camera_ptr,
      colorfunc::Color OBSFRUSTUM_color,double vol_alpha=0);
   void reset_OBSFRUSTUM(
      double curr_t,int pass_number,
      double frustum_sidelength,double movie_downrange_distance,
      const threevector& camera_world_posn,
      const std::vector<threevector>& corner_ray,
      colorfunc::Color OBSFRUSTUM_color,double vol_alpha=0);
   std::vector<threevector> reorient_camera_corner_rays(
      double az,double el,double roll,camera* camera_ptr);

// Frustum drawing methods:
   
   virtual void set_color(const colorfunc::Color& c);
   virtual void set_color(const osg::Vec4& color);
   void set_color(const osg::Vec4& color,double VolumeAlpha);
   void set_color(
      const colorfunc::Color& SideEdgeColor,
      const colorfunc::Color& ZplaneEdgeColor,
      const colorfunc::Color& VolumeColor,double VolumeAlpha);
   void set_color(
      const osg::Vec4& side_edge_color,const osg::Vec4& zplane_edge_color,
      const osg::Vec4& volume_color);

   void set_typical_pyramid_edge_widths();

// Time-dependent translation & rotation member functions:

   void translate(
      double curr_t,int pass_number,const threevector& trans);
   void absolute_position(
      double curr_t,int pass_number,const threevector& absolute_posn);
   void absolute_position_and_orientation(
      double curr_t,int pass_number,const threevector& absolute_posn,
      const threevector& new_Uhat,const threevector& new_Vhat);
   void absolute_position_and_orientation(
      double curr_t,int pass_number,
      const threevector& absolute_position,double az,double el,double roll);
   void set_relative_Movie_window();
   void set_relative_Movie_window(
      Movie* movie_ptr,camera* camera_ptr,double downrange_distance);
   void orient_camera_model(
      double curr_t,int pass_number,
      const std::vector<threevector>& UV_corner_dir);

// Footprint member functions:

   polygon GMTI_dwell_frustum_footprint(
      const threevector& V,const threevector& G,
      double range_extent,double crossrange_extent,double z_base_face);
//   polygon reconstruct_footprint(double curr_t,int pass_number);

// Projection & backprojection member functions:

   double estimate_z_ground(
      PointCloud* PointCloud_ptr,unsigned int n_frac_bins=300,
      bool compute_avg_flag=false);
   double estimate_z_ground(
      PointFinder* PointFinder_ptr,unsigned int n_frac_bins=300,
      bool compute_avg_flag=false);
   void backproject_pixels_into_second_imageplane(
      OBSFRUSTUM* OBSFRUSTUM_ptr,double overlap_hue=0);
   void project_SignPosts_into_imageplane(SignPostsGroup* SignPostsGroup_ptr);
   SignPost* generate_SignPost_at_imageplane_location(
      const twovector& UV,SignPostsGroup* SignPostsGroup_ptr,
      double curr_t,int passnumber,double size,double height_multiplier,
      int ID=-1);
   void project_curr_track_points(Movie::TRACKS_MAP* tracks_map_ptr,
      const fourvector& groundplane_pi);

   bool project_Polyhedra_into_imageplane(
      PolyhedraGroup* PolyhedraGroup_ptr,
      osgGeometry::PolygonsGroup* PolygonsGroup_ptr,
      twoDarray* DTED_ztwoDarray_ptr);

   bool new_project_Polyhedra_into_imageplane(
      PolyhedraGroup* PolyhedraGroup_ptr,
      osgGeometry::PolygonsGroup* PolygonsGroup_ptr,
      twoDarray* DTED_ztwoDarray_ptr);

   double project_polyhedron_point_cloud_into_imageplane(
      polyhedron* polyhedron_ptr,camera* camera_ptr,
      twoDarray* DTED_ztwoDarray_ptr);
   double project_polyhedron_point_cloud_into_imageplane(
      polyhedron* polyhedron_ptr,camera* camera_ptr,
      twoDarray* DTED_ztwoDarray_ptr,bounding_box& UVW);
   double integrate_projected_polyhedron_faces_area(
      polyhedron* polyhedron_ptr);
   double calculate_imageplane_score(
      double integrated_points_visibility,double integrated_proj_face_area,
      const bounding_box& UVW);
   
// Empire State Building video member functions:

   void draw_ESB_street_rays();
   void crop_ESB_region_voxels(PointCloud* PointCloud_ptr);
   void project_ground_info_into_imageplane(PointCloud* PointCloud_ptr);

// UAV OBSFRUSTA member functions:

   void set_UAV_OBSFRUSTUM_colorings(int UAV_ID);

// DTED raytracing member functions:

   void compute_latlong_pnts_inside_footprint(
      double max_raytrace_range,double raytrace_cellsize,
      std::vector<threevector>& latlong_points_inside_polygon);
   void compute_latlong_pnts_inside_footprint(
      double max_raytrace_range,double raytrace_cellsize,
      double& theta_min,double& theta_max,threevector& apex,
      polygon& zplane_triangle,
      std::vector<threevector>& latlong_points_inside_polygon);
   twoDarray* load_DTED_height_data(
      double max_raytrace_range,double raytrace_cellsize);
   twoDarray* load_DTED_height_data(
      double max_raytrace_range,double raytrace_cellsize,
      double& theta_min,double& theta_max,threevector& apex,
      std::vector<threevector>& latlong_points_inside_polygon,
      polygon& zplane_triangle);
   twoDarray* load_DTED_height_data(
      double raytrace_cellsize,
      const std::vector<threevector>& latlong_points_inside_polygon);

   double raytrace_occluded_ground_region(
      double min_raytrace_range,double max_raytrace_range,
      double raytrace_cellsize,
      unsigned int& n_total_rays,unsigned int& n_occluded_rays);

   int raytrace_ground_targets(
      const std::vector<twovector>& target_posns,double max_ground_Z,
      double max_raytrace_range,double min_raytrace_range,double ds,
      twoDarray* DTED_ztwoDarray_ptr,twoDarray* DTED_ptwoDarray_ptr,
      twoDarray* reduced_DTED_ztwoDarray_ptr,
      std::vector<std::pair<int,threevector> >& target_tracing_result);
   int raytrace_ground_targets(
      const std::vector<twovector>& target_posns,double max_ground_Z,
      double max_raytrace_range,double min_raytrace_range,double ds,
      std::vector<std::pair<int,threevector> >& target_tracing_result);
   int omni_raytrace_ground_targets(
      const threevector& obs_posn,const std::vector<twovector>& target_posns,
      double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
      double ds,twoDarray* DTED_ztwoDarray_ptr,
      twoDarray* reduced_DTED_ztwoDarray_ptr,
      std::vector<std::pair<int,threevector> >& target_tracing_result);

   bool import_DTED_ptwoDarray_contents(
      raster_parser& RasterParser,std::string geotif_Ptiles_subdir);
   void extract_viewfrustum_components(
      bool northern_hemisphere_flag,int specified_UTM_zonenumber,
      threevector& apex,bounding_box& zplane_face_bbox,
      polygon& zplane_triangle,
      std::vector<geopoint>& triangle_vertices,
      double max_raytrace_range,double& theta_min,double& theta_max);
   void export_DTED_ptwoDarray_contents(
      raster_parser& RasterParser,std::string geotif_Ptiles_subdir,
      bool northern_hemisphere_flag,int specified_UTM_zonenumber);
   void compute_occlusion_frac_vs_maxrange(const threevector& apex);

// OBSFRUSTUM orientation member functions:

   rotation rotation_about_LOS();
   rotation rotation_about_LOS(bool& image_rotated_by_ninety_degrees_flag);

// Ray drawing member functions:

   Arrow* draw_ray_through_imageplane(
      int Arrow_ID,const twovector& UV,double magnitude,
      colorfunc::Color color,double linewidth=1.0);

  protected:

  private:

   bool rectangular_movie_flag,virtual_camera_flag;
   bool display_camera_model_flag,viewing_pyramid_above_zplane_exists;
   bool portrait_mode_flag;
   bool display_ViewingPyramid_flag,display_ViewingPyramidAboveZplane_flag;
   double z_ground;
   double az_extent,el_extent;
   double movie_downrange_distance,volume_alpha;
   double prev_ll_longitude,prev_ll_latitude,prev_ur_longitude,
      prev_ur_latitude;
   double FOV_triangle_min_az,FOV_triangle_max_az;
   threevector* grid_world_origin_ptr;
   std::vector<threevector> corner_ray,projected_polyhedron_points,
      occluded_ray_impact_points;

   ArrowsGroup* ArrowsGroup_ptr;
   twoDarray* DTED_ptwoDarray_ptr;
   Pass* pass_ptr;
   bounding_box* ground_bbox_ptr;
   CylindersGroup* CylindersGroup_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   Movie* Movie_ptr;
   MODEL* camera_model_ptr;
   pyramid *viewing_pyramid_ptr,*viewing_pyramid_above_zplane_ptr;
   PyramidsGroup* PyramidsGroup_ptr;
   Pyramid *ViewingPyramid_ptr,*ViewingPyramidAboveZplane_ptr;
   ray_tracer* ray_tracer_ptr;
   TilesGroup* TilesGroup_ptr;

   int prev_i_start,prev_i_stop;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const OBSFRUSTUM& f);

   void draw_rays_thru_imageplane_features_to_worldplane(
      const std::vector<int>& track_IDs,const std::vector<twovector>& UV,
      const plane& P);
   std::string get_geotiff_filename(std::string geotif_Ptiles_subdir);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double OBSFRUSTUM::get_az_extent() const
{
   return az_extent;
}

inline double OBSFRUSTUM::get_el_extent() const
{
   return el_extent;
}

inline void OBSFRUSTUM::set_rectangular_movie_flag(bool flag)
{
   rectangular_movie_flag=flag;
}

inline bool OBSFRUSTUM::get_rectangular_movie_flag() const
{
   return rectangular_movie_flag;
}

inline void OBSFRUSTUM::set_virtual_camera_flag(bool flag)
{
   virtual_camera_flag=flag;
}

inline bool OBSFRUSTUM::get_virtual_camera_flag() const
{
   return virtual_camera_flag;
}

inline void OBSFRUSTUM::set_portrait_mode_flag(bool flag)
{
   portrait_mode_flag=flag;
}

inline bool OBSFRUSTUM::get_portrait_mode_flag() const
{
   return portrait_mode_flag;
}

inline void OBSFRUSTUM::set_display_ViewingPyramid_flag(bool flag)
{
   display_ViewingPyramid_flag=flag;
}

inline bool OBSFRUSTUM::get_display_ViewingPyramid_flag() const
{
   return display_ViewingPyramid_flag;
}

inline void OBSFRUSTUM::set_display_ViewingPyramidAboveZplane_flag(bool flag)
{
   display_ViewingPyramidAboveZplane_flag=flag;
}

inline bool OBSFRUSTUM::get_display_ViewingPyramidAboveZplane_flag() const
{
   return display_ViewingPyramidAboveZplane_flag;
}

inline void OBSFRUSTUM::set_movie_downrange_distance(double d)
{
   movie_downrange_distance=d;
}

inline double OBSFRUSTUM::get_movie_downrange_distance() const
{
   return movie_downrange_distance;
}

inline void OBSFRUSTUM::set_FOV_triangle_min_az(double min_az)
{
   FOV_triangle_min_az=min_az;
}

inline void OBSFRUSTUM::set_FOV_triangle_max_az(double max_az)
{
   FOV_triangle_max_az=max_az;
}

// Not sure if we can get rid of next method...it's called by 
// ObsFrustaGroup::generate_HAFB_movie_OBSFRUSTUM()

inline void OBSFRUSTUM::set_Movie_ptr(Movie* Movie_ptr)
{
   this->Movie_ptr=Movie_ptr;
}

inline Movie* OBSFRUSTUM::get_Movie_ptr()
{
   return Movie_ptr;
}

inline const Movie* OBSFRUSTUM::get_Movie_ptr() const
{
   return Movie_ptr;
}

inline CylindersGroup* OBSFRUSTUM::get_CylindersGroup_ptr()
{
   return CylindersGroup_ptr;
}

inline const CylindersGroup* OBSFRUSTUM::get_CylindersGroup_ptr() const
{
   return CylindersGroup_ptr;
}

inline LineSegmentsGroup* OBSFRUSTUM::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

inline const LineSegmentsGroup* OBSFRUSTUM::get_LineSegmentsGroup_ptr() const
{
   return LineSegmentsGroup_ptr;
}

inline PyramidsGroup* OBSFRUSTUM::get_PyramidsGroup_ptr()
{
   return PyramidsGroup_ptr;
}

inline const PyramidsGroup* OBSFRUSTUM::get_PyramidsGroup_ptr() const
{
   return PyramidsGroup_ptr;
}

inline Pyramid* OBSFRUSTUM::get_last_Pyramid_ptr()
{
   return PyramidsGroup_ptr->get_last_Pyramid_ptr();
}

inline const Pyramid* OBSFRUSTUM::get_last_Pyramid_ptr() const
{
   return PyramidsGroup_ptr->get_last_Pyramid_ptr();
}

inline pyramid* OBSFRUSTUM::get_viewing_pyramid_ptr()
{
   return viewing_pyramid_ptr;
}

inline const pyramid* OBSFRUSTUM::get_viewing_pyramid_ptr() const
{
   return viewing_pyramid_ptr;
}

inline pyramid* OBSFRUSTUM::get_viewing_pyramid_above_zplane_ptr()
{
   return viewing_pyramid_above_zplane_ptr;
}

inline const pyramid* OBSFRUSTUM::get_viewing_pyramid_above_zplane_ptr() const
{
   return viewing_pyramid_above_zplane_ptr;
}

inline Pyramid* OBSFRUSTUM::get_ViewingPyramid_ptr()
{
   return ViewingPyramid_ptr;
}

inline const Pyramid* OBSFRUSTUM::get_ViewingPyramid_ptr() const
{
   return ViewingPyramidAboveZplane_ptr;
}

inline Pyramid* OBSFRUSTUM::get_ViewingPyramidAboveZplane_ptr()
{
   return ViewingPyramidAboveZplane_ptr;
}

inline void OBSFRUSTUM::set_TilesGroup_ptr(TilesGroup* TG_ptr)
{
   TilesGroup_ptr=TG_ptr;
   ray_tracer_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
}

inline TilesGroup* OBSFRUSTUM::get_TilesGroup_ptr()
{
   return TilesGroup_ptr;
}

inline const TilesGroup* OBSFRUSTUM::get_TilesGroup_ptr() const
{
   return TilesGroup_ptr;
}

inline void OBSFRUSTUM::set_volume_alpha(double a)
{
   volume_alpha=a;
}

inline double OBSFRUSTUM::get_volume_alpha()
{
   return volume_alpha;
}

inline double OBSFRUSTUM::get_volume_alpha() const
{
   return volume_alpha;
}

inline void OBSFRUSTUM::set_ground_bbox_ptr(bounding_box* ground_bbox_ptr)
{
   this->ground_bbox_ptr=ground_bbox_ptr;
}

inline twoDarray* OBSFRUSTUM::get_DTED_ptwoDarray_ptr() const
{
   return DTED_ptwoDarray_ptr;
}

inline const std::vector<threevector>& OBSFRUSTUM::get_corner_ray() const
{
   return corner_ray;
}

inline std::vector<threevector>& OBSFRUSTUM::get_projected_polyhedron_points()
{
   return projected_polyhedron_points;
}

inline const std::vector<threevector>& 
OBSFRUSTUM::get_projected_polyhedron_points() const
{
   return projected_polyhedron_points;
}

inline std::vector<threevector>& OBSFRUSTUM::get_occluded_ray_impact_points()
{
   return occluded_ray_impact_points;
}

inline const std::vector<threevector>& 
OBSFRUSTUM::get_occluded_ray_impact_points() const
{
   return occluded_ray_impact_points;
}

inline bool OBSFRUSTUM::get_ladar_height_data_flag() const
{
   return TilesGroup_ptr->get_ladar_height_data_flag();
}

inline ray_tracer* OBSFRUSTUM::get_ray_tracer_ptr()
{
   return ray_tracer_ptr;
}

inline const ray_tracer* OBSFRUSTUM::get_ray_tracer_ptr() const
{
   return ray_tracer_ptr;
}

inline ArrowsGroup* OBSFRUSTUM::get_ArrowsGroup_ptr()
{
   return ArrowsGroup_ptr;
}

inline const ArrowsGroup* OBSFRUSTUM::get_ArrowsGroup_ptr() const
{
   return ArrowsGroup_ptr;
}

inline Arrow* OBSFRUSTUM::get_Arrow_ptr(int a)
{
   return ArrowsGroup_ptr->get_Arrow_ptr(a);
}

inline const Arrow* OBSFRUSTUM::get_Arrow_ptr(int a) const
{
   return ArrowsGroup_ptr->get_Arrow_ptr(a);
}

#endif // OBSFRUSTUM.h



