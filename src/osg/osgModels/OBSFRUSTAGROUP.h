// ==========================================================================
// Header file for OBSFRUSTAGROUP class
// ==========================================================================
// Last modified on 7/13/13; 8/15/13; 9/10/13
// ==========================================================================

#ifndef OBS_FRUSTAGROUP_H
#define OBS_FRUSTAGROUP_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osg2D/ArrowHUD.h"
#include "osg/osgGeometry/ArrowsGroup.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "math/fourvector.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "geometry/linesegment.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgModels/OBSFRUSTUM.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "image/TwoDarray.h"
#include "video/videofuncs.h"

class AnimationController;
class camera;
class MODELSGROUP;
class photogroup;
class WindowManager;

class OBSFRUSTAGROUP : public GeometricalsGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   OBSFRUSTAGROUP(
      Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr=NULL);
   OBSFRUSTAGROUP(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      AnimationController* AC_ptr,threevector* GO_ptr=NULL);
   virtual ~OBSFRUSTAGROUP();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const OBSFRUSTAGROUP& f);

// Set & get member functions:

   void set_enable_OBSFRUSTA_blinking_flag(bool flag);
   void set_cross_fading_flag(bool flag);
   void set_flashlight_mode_flag(bool flag);
   void set_display_Pyramids_flag(bool flag);
   void set_quasirandom_tour_flag(bool flag);
   void set_alpha_vary_selected_image_flag(bool flag);
   void set_erase_other_OBSFRUSTA_flag(bool flag);
   bool get_erase_other_OBSFRUSTA_flag() const;
   void set_jump_to_apex_flag(bool flag);

   int get_n_3D_rays() const;
   void set_FOV_excess_fill_factor(double factor);
   double get_FOV_excess_fill_factor() const;
   void set_groundplane_pi(const fourvector& pi);
   const fourvector& get_groundplane_pi() const;

   void set_z_ColorMap_ptr(ColorMap* cmap_ptr);
   OBSFRUSTUM* get_OBSFRUSTUM_ptr(int n) const;
   OBSFRUSTUM* get_ID_labeled_OBSFRUSTUM_ptr(int ID) const;
   MoviesGroup* get_MoviesGroup_ptr();
   std::vector<bounding_box>& get_photo_zplane_bboxes();
   const std::vector<bounding_box>& get_photo_zplane_bboxes() const;

   void set_ArrowHUD_ptr(ArrowHUD* AH_ptr);
   void set_ArrowsGroup_ptr(ArrowsGroup* AG_ptr);
   ArrowsGroup* get_ArrowsGroup_ptr();
   const ArrowsGroup* get_ArrowsGroup_ptr() const;
   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);
   PointCloudsGroup* get_PointCloudsGroup_ptr();
   const PointCloudsGroup* get_PointCloudsGroup_ptr() const;
   
   void set_PointCloud_ptr(PointCloud* cloud_ptr);
   PointCloud* get_PointCloud_ptr();
   const PointCloud* get_PointCloud_ptr() const;

   void set_ImageNumberHUD_ptr(ImageNumberHUD* ImageNumberHUD_ptr);
   ImageNumberHUD* get_ImageNumberHUD_ptr();
   const ImageNumberHUD* get_ImageNumberHUD_ptr() const;

   void set_WindowManager_ptr(WindowManager* WM_ptr);
   void set_PolyhedraGroup_ptr(PolyhedraGroup* PG_ptr);
   void set_PointsGroup_ptr(osgGeometry::PointsGroup* PG_ptr);
   osgGeometry::PointsGroup* get_PointsGroup_ptr();
   const osgGeometry::PointsGroup* get_PointsGroup_ptr() const;
   void set_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* PG_ptr);
   void set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr);
   twoDarray* get_DTED_ztwoDarray_ptr();
   void set_cameraid_xyz_map_ptr(videofunc::CAMERAID_XYZ_MAP* map_ptr);

   void set_photogroup_ptr(photogroup* pg_ptr);
   photogroup* get_photogroup_ptr();
   OBSFRUSTUM* get_selected_OBSFRUSTUM_ptr();
   int get_selected_OBSFRUSTUM_photo_ID();
   int get_selected_photo_OBSFRUSTUM_ID(int photo_ID);
   void set_prev_OBSFRUSTUM_framenumber(int framenumber);
   int get_prev_OBSFRUSTUM_framenumber() const;
   photograph* get_OBSFRUSTUM_photograph_ptr(int OBSFRUSTUM_ID);
   camera* get_OBSFRUSTUM_photo_camera_ptr(int OBSFRUSTUM_ID);

   OBSFRUSTUM* get_virtual_OBSFRUSTUM_ptr();
   const OBSFRUSTUM* get_virtual_OBSFRUSTUM_ptr() const;
   camera* get_virtual_camera_ptr();
   const camera* get_virtual_camera_ptr() const;

   void set_PARENT_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* O_ptr);
   void set_target_MODELSGROUP_ptr(MODELSGROUP* MG_ptr);
   MODELSGROUP* get_target_MODELSGROUP_ptr();
   const MODELSGROUP* get_target_MODELSGROUP_ptr() const;

   void set_play_OBSFRUSTA_as_movie_flag(bool flag);
   void set_project_frames_onto_zplane_flag(bool flag);

// OBSFRUSTUM creation and manipulation member functions:

   OBSFRUSTUM* generate_new_OBSFRUSTUM(
      Movie* Movie_ptr=NULL,int OSGsubPAT_ID=0,int ID=-1);
   OBSFRUSTUM* generate_new_OBSFRUSTUM(
      AnimationController* AC_ptr,int OSGsubPAT_ID=0,int ID=-1);
   OBSFRUSTUM* generate_new_OBSFRUSTUM(
      double az_extent,double el_extent,Movie* Movie_ptr=NULL,
      int OSGsubPAT_ID=0,int ID=-1);
   Movie* generate_Movie(std::string movie_filename,double alpha=1.0,
                         int Movie_ID=-1);
   OBSFRUSTUM* generate_movie_OBSFRUSTUM(
      std::string movie_filename,int OSGsubPAT_ID=0,double alpha=1.0,
      int movie_ID=-1,int ID=-1);
   void set_Movie_visibility_flag(bool flag);

   bool erase_OBSFRUSTUM(int n);
   bool unerase_OBSFRUSTUM(int n,bool display_Pyramids_flag=true);
   void erase_all_OBSFRUSTA();
   void unerase_all_OBSFRUSTA();
   void fast_erase_all_OBSFRUSTA();
   void fast_unerase_OBSFRUSTUM(int ID);
   void fast_unerase_OBSFRUSTUM_Movie(int ID);
   void destroy_OBSFRUSTUM(OBSFRUSTUM* curr_OBSFRUSTUM_ptr);
   void destroy_all_OBSFRUSTA();

// Circularly ordered OBSFRUSTA member functions:

   void read_circularly_ordered_OBSFRUSTA_IDs();
   void display_left_right_OBSFRUSTA_selection_symbols();
   bool move_to_right_OBSFRUSTUM_neighbor();
   bool move_to_left_OBSFRUSTUM_neighbor();

// OBSFRUSTA display member functions:

   void set_altitude_dependent_OBSFRUSTA_volume_alpha();

// Still imagery OBSFRUSTA generation member functions:

   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,bool multicolor_frusta_flag=true,
      bool thumbnails_flag=false);
   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,
      double frustum_sidelength,double movie_downrange_distance,
      bool multicolor_frusta_flag=true,bool thumbnails_flag=false);
   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,int n_final_photo,
      double frustum_sidelength,double movie_downrange_distance,
      bool multicolor_frusta_flag=true,bool thumbnails_flag=false);
   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,const threevector& camera_posn,
      double frustum_sidelength,double movie_downrange_distance,
      bool multicolor_frusta_flag=true,bool thumbnails_flag=false);
   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,int n_final_photo,
      const threevector& camera_posn,
      double frustum_sidelength,double movie_downrange_distance,
      bool multicolor_frusta_flag=true,bool thumbnails_flag=false);
   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,
      const threevector& global_camera_translation,
      double global_daz,double global_del,double global_droll,
      const threevector& rotation_origin,double local_spin_daz=0,
      colorfunc::Color OBSFRUSTUM_color=colorfunc::white,
      bool thumbnails_flag=false);
   void generate_still_imagery_frusta_for_photogroup(
      photogroup* photogroup_ptr,int n_still_images,
      const threevector& camera_posn,
      const threevector& global_camera_translation,
      double global_daz,double global_del,double global_droll,
      const threevector& rotation_origin,double local_spin_daz,
      double common_frustum_sidelength,double common_movie_downrange_distance,
      bool multicolor_frusta_flag=false,bool thumbnails_flag=false,
      colorfunc::Color OBSFRUSTUM_color=colorfunc::white);

   Movie* generate_second_image_plane(
      int OBSFRUSTUM_ID,std:: string second_photo_filename);

   int generate_still_imagery_frusta_from_projection_matrices(
      PassesGroup& passes_group,
      double Zplane_altitude=20.0,bool multicolor_frusta_flag=false,
      bool initially_mask_all_frusta_flag=true);
   int generate_still_imagery_frusta_from_projection_matrices(
      PassesGroup& passes_group,
      const threevector& relative_camera_translation,
      double Zplane_altitude=20.0,bool multicolor_frusta_flag=false,
      bool initially_mask_all_frusta_flag=true);
   bool extract_still_imagery_info(Pass* videopass_ptr);
   void reset_frustum_colors_based_on_Zcolormap();

   OBSFRUSTUM* generate_still_image_OBSFRUSTUM(
      int n_frustum,colorfunc::Color frustum_color,
      const threevector& relative_camera_translation,
      double Zplane_altitude=20.0,int OSGsubPAT_ID=0);
   OBSFRUSTUM* generate_virtual_camera_OBSFRUSTUM();

   OBSFRUSTUM* generate_test_OBSFRUSTUM(
      double z_ground,double sidelength,camera* camera_ptr);
   OBSFRUSTUM* generate_test_OBSFRUSTUM(
      double z_ground,double sidelength,
      const std::vector<threevector>& UV_corner_dir);

   void reset_to_common_imageplane();

// Subfrusta member functions:

   int get_new_subfrustum_ID() const;
   void set_new_subfrustum_ID(int ID);
   void set_subfrustum_color(colorfunc::Color c);
   void set_subfrustum_volume_alpha(double alpha);
   OBSFRUSTAGROUP* get_SUBFRUSTAGROUP_ptr();
   void set_subfrustum_bbox_ptr(bounding_box* bb_ptr);
   void set_subfrustum_downrange_distance(double distance);
   void pushback_subfrustum_bbox_ptr(bounding_box* bb_ptr);
   std::vector<bounding_box*>& get_subfrusta_bbox_ptrs();
   const std::vector<bounding_box*>& get_subfrusta_bbox_ptrs() const;
   void null_SUBFRUSTUM_ptr();
   OBSFRUSTUM* get_sub_FRUSTUM_ptr();
   const OBSFRUSTUM* get_sub_FRUSTUM_ptr() const;

   OBSFRUSTAGROUP* generate_SUBFRUSTAGROUP();
   void destroy_all_SUBFRUSTA();
   bool load_SUBFRUSTA();
   OBSFRUSTUM* generate_subfrustum(
      bounding_box* bbox_ptr,photograph* photograph_ptr,camera* camera_ptr);
   threevector insert_human_MODEL(
      OBSFRUSTUM* sub_FRUSTUM_ptr,camera* camera_ptr);
   void broadcast_human_MODEL_geoposn(const threevector& geoposn);
   void update_subfrusta(double z_ground);

// Animation member functions:

   bool fly_to_entered_OBSFRUSTUM(
      int ID,int n_anim_steps=-1,double t_flight=-1);
   void flyto_camera_location(int ID,int n_anim_steps=-1,double t_flight=-1);
   std::vector<Movie*> find_Movies_in_OSGsubPAT();
   void pause_all_videos();
   void fly_out_from_OBSFRUSTUM(
      int ID,bool quasirandom_flyout_flag=false,
      double backoff_distance_factor=3);
   void compute_camera_flyout_posn_and_orientation(
      int ID,threevector& flyout_camera_posn,rotation& Rcamera_flyout,
      bool quasirandom_flyout_flag=false,double backoff_distance_factor=3);
   void compute_quasirandom_camera_flyout_posn_and_orientation(
      int ID,threevector& flyout_camera_posn,rotation& Rcamera_flyout);
   void finalize_after_flying_in_and_out();

   void load_hires_photo();
   void load_hires_photo(int OBSFRUSTUM_ID);
   void load_thumbnail_photo();
   void load_thumbnail_photo(int OBSFRUSTUM_ID);

   void cross_fade_photo_pair();
   void fade_away_photo(int OBSFRUSTUM_ID);
   void fade_in_photo(int OBSFRUSTUM_ID);

   void display_OBSFRUSTA_as_time_sequence();
   int find_OBSFRUSTUM_closest_to_screen_center();
   void alpha_vary_selected_image();
   bool select_and_alpha_vary_OBSFRUSTUM_closest_to_screen_center();
   bool deselect_OBSFRUSTUM();

// HAFB video3D member functions:

   OBSFRUSTUM* generate_HAFB_movie_OBSFRUSTUM(
      const std::vector<threevector>& aircraft_posn,double z_offset);
   void read_HAFB_frusta_info(
      std::string segments_filename,std::vector<double>& curr_time,
      std::vector<int>& segment_ID,std::vector<int>& pass_number,
      std::vector<threevector>& V1,std::vector<threevector>& V2,
      std::vector<colorfunc::Color>& color);
   void reconstruct_HAFB_corner_dirs(
      const std::vector<threevector>& plane_posn,
      const std::vector<threevector>& corner_posns,
      std::vector<threevector>& UV_corner_dir);

// 2D photo member functions:

   void compute_current_zplane_UV_bboxes();
   void reset_zface_color();
   void compute_tabular_rows_and_columns(
      int n_photos,int& n_rows,int& n_columns);
   void convert_screenspace_to_photo_coords(
      const threevector& UVW,int& OBSFRUSTUM_ID,twovector& video_UV);
   void convert_photo_to_screenspace_coords(
      int OBSFRUSTUM_ID,const twovector& video_UV,threevector& UVW);
   void zoom_virtual_camera(double angular_scale_factor);
   void load_new_photo(int OBSFRUSTUM_ID,std::string photo_filename);

// SignPost projection into image plane member functions:

   SignPost* generate_SignPost_at_imageplane_location(
      const twovector& UV,int OBSFRUSTUM_ID,
      SignPostsGroup* imageplane_SignPostsGroup_ptr);
   void project_SignPosts_into_imageplanes(
      SignPostsGroup* SignPostsGroup_ptr);
   void project_SignPosts_into_imageplanes(
      SignPostsGroup* SignPostsGroup_ptr,
      SignPostsGroup* imageplane_SignPostsGroup_ptr);
   void project_SignPosts_into_video_plane(
      SignPostsGroup* SignPostsGroup_ptr);

// Polyhedra projection into image plane member functions:

   void compute_photo_views_of_polyhedra_bboxes(photogroup* photogroup_ptr);
   bool project_Polyhedra_into_selected_photo(int selected_ID);

// Message handling member functions:

   bool blink_OBSFRUSTUM(int OBSFRUSTUM,double max_blink_period=10);
   void issue_select_vertex_message();

// Bundler member functions:

   void display_visible_reconstructed_XYZ_points(int selected_ID);
   void populate_image_vs_package_names_map(std::string packages_subdir);
   std::string get_package_filename(std::string image_filename);

// Raytracing member functions:

   void compute_D7_FOV(double& theta_min,double& theta_max);
   linesegment draw_ray_through_imageplane(
      int Arrow_ID,int OBSFRUSTUM_ID,const twovector& UV,
      double magnitude,colorfunc::Color color,double linewidth=1.0);
   threevector triangulate_rays(int Arrow_ID);

// Virtual OBSFRUSTUM creation & manipulation member functions:

   OBSFRUSTUM* physical_OBSFRUSTUM_pointing_closest_to_virtual_OBSFRUSTUM(
      double z_ground);
   photograph* instantiate_virtual_photo(
      double horiz_FOV,double vert_FOV,double frustum_sidelength,
      double blank_grey_level=0.5);
   OBSFRUSTUM* generate_virtual_OBSFRUSTUM(photograph* photograph_ptr);
   OBSFRUSTUM* update_virtual_OBSFRUSTUM(
      const threevector& camera_world_posn,
      double az,double el,double roll,double frustum_sidelength);
   OBSFRUSTUM* update_virtual_OBSFRUSTUM(
      double az,double el,double roll,double frustum_sidelength);
   void update_virtual_OBSFRUSTUM(
      const threevector& camera_world_posn,
      double az,double el,double roll,double frustum_sidelength,
      double z_ground);
   void compute_virtual_OBSFRUSTA_for_circular_staring_orbit(
      const threevector& ground_target_posn,
      double virtual_camera_height_above_ground);
   std::string reproject_physical_image_into_virtual_OBSFRUSTUM(
      OBSFRUSTUM* physical_OBSFRUSTUM_ptr,double z_ground);
   void export_virtual_camera_package(std::string virtual_image_filename);
   void virtual_tour_from_animation_path(
      std::string path_filename,double z_ground);

// OBSFRUSTUM adjustment member functions:

   OBSFRUSTUM* adjust_frustum_angles(double d_az,double d_el,double d_roll);
   OBSFRUSTUM* adjust_frustum_angles(
      int OBSFRUSTUM_ID,double d_az,double d_el,double d_roll);
   
  protected:

  private:

   bool enable_OBSFRUSTA_blinking_flag,cross_fading_flag;
   bool flashlight_mode_flag;
   bool display_Pyramids_flag;
   bool quasirandom_tour_flag,before_flyin_flag,before_flyout_flag;
   bool alpha_vary_selected_image_flag;
   bool erase_other_OBSFRUSTA_flag,OBSFRUSTA_reset_flag;
   bool jump_to_apex_flag,play_OBSFRUSTA_as_movie_flag;
   bool project_frames_onto_zplane_flag;
   int frustum_color_counter,new_subfrustum_ID;
   int virtual_frame_counter,n_3D_rays;
   double filter_alpha_value,FOV_excess_fill_factor;
   double subfrustum_downrange_distance,subfrustum_volume_alpha;
   colorfunc::Color subfrustum_color;
   fourvector groundplane_pi;

   ArrowHUD* ArrowHUD_ptr;
   ArrowsGroup* ArrowsGroup_ptr;
   bounding_box* subfrustum_bbox_ptr;
   std::vector<bounding_box*> subfrusta_bbox_ptrs;
   camera* virtual_camera_ptr;
   ColorMap* z_ColorMap_ptr;
   ImageNumberHUD* ImageNumberHUD_ptr;
   MODELSGROUP* target_MODELSGROUP_ptr;
   MoviesGroup* MoviesGroup_ptr;
   OBSFRUSTUM *sub_FRUSTUM_ptr,*virtual_OBSFRUSTUM_ptr;
   OBSFRUSTAGROUP *PARENT_OBSFRUSTAGROUP_ptr,*SUBFRUSTAGROUP_ptr;
   photogroup* photogroup_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   PointCloud* PointCloud_ptr;
   osgGeometry::PointsGroup* PointsGroup_ptr;
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   twoDarray* DTED_ztwoDarray_ptr; // just pointer, not object
   WindowManager* WindowManager_ptr;

   videofunc::CAMERAID_XYZ_MAP* cameraID_xyz_map_ptr;

   typedef std::map<int,int> PHOTO_VS_OBSFRUSTUM_ID_MAP;
   PHOTO_VS_OBSFRUSTUM_ID_MAP photo_obsfrustum_IDs_map;
   typedef std::map<std::string,std::string> IMAGE_VS_PACKAGE_NAMES_MAP;
   IMAGE_VS_PACKAGE_NAMES_MAP* image_vs_package_names_map_ptr;

// Member variables for still image OBSFRUSTA:

   int n_still_images;
   colorfunc::Color frustum_color;
   std::vector<bool> portrait_mode_flags;
   std::vector<std::string> still_movie_filenames,frustum_colors;
   std::vector<double> frustum_sidelengths,downrange_distances;
   std::vector<genmatrix*> P_ptrs;
   threevector static_camera_world_posn;

// Member variables for 2D photos:

   std::vector<bounding_box> photo_zplane_bboxes;

// Member variables for dynamic videos:

   int prev_OBSFRUSTUM_framenumber;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const OBSFRUSTAGROUP& f);

   void initialize_new_OBSFRUSTUM(
      OBSFRUSTUM* OBSFRUSTUM_ptr,Movie* Movie_ptr,int OSGsubPAT_ID=0);

   void set_next_frustum_color(int n_frustum);
   bool reset_OBSFRUSTUM_from_package_file(OBSFRUSTUM* OBSFRUSTUM_ptr);

   virtual bool parse_next_message_in_queue(message& curr_message);
   void update_display();

   void update_subfrustum();
   void compute_other_visible_imageplanes();
   void update_OBSFRUSTUM_using_Movie_info(
      OBSFRUSTUM* OBSFRUSTUM_ptr,const std::vector<threevector>& vertices);

   void project_curr_frame_onto_zplane();
   void conduct_quasirandom_tour();

   OBSFRUSTUM* generate_still_image_frustum_for_photograph(
      photograph* photograph_ptr,const threevector& camera_posn,
      const threevector& global_camera_translation,
      double global_daz,double global_del,double global_droll,
      const threevector& rotation_origin,
      double common_frustum_sidelength,double common_movie_downrange_distance,
      colorfunc::Color OBSFRUSTUM_color,bool thumbnails_flag);
   OBSFRUSTUM* generate_still_image_frustum_for_photograph(
      photograph* photograph_ptr,const threevector& camera_posn,
      const threevector& global_camera_translation,
      double global_daz,double global_del,double global_droll,
      const threevector& rotation_origin,double local_spin_daz,
      double common_frustum_sidelength,double common_movie_downrange_distance,
      colorfunc::Color OBSFRUSTUM_color,bool thumbnails_flag,
      Movie* input_Movie_ptr=NULL);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void OBSFRUSTAGROUP::set_enable_OBSFRUSTA_blinking_flag(bool flag)
{
   enable_OBSFRUSTA_blinking_flag=flag;
}

inline void OBSFRUSTAGROUP::set_cross_fading_flag(bool flag)
{
   cross_fading_flag=flag;
}

inline void OBSFRUSTAGROUP::set_flashlight_mode_flag(bool flag)
{
   flashlight_mode_flag=flag;
}

inline void OBSFRUSTAGROUP::set_display_Pyramids_flag(bool flag)
{
   display_Pyramids_flag=flag;
}

inline void OBSFRUSTAGROUP::set_quasirandom_tour_flag(bool flag)
{
   quasirandom_tour_flag=flag;
}

inline void OBSFRUSTAGROUP::set_alpha_vary_selected_image_flag(bool flag)
{
   alpha_vary_selected_image_flag=flag;
}

inline void OBSFRUSTAGROUP::set_erase_other_OBSFRUSTA_flag(bool flag)
{
   erase_other_OBSFRUSTA_flag=flag;
}

inline bool OBSFRUSTAGROUP::get_erase_other_OBSFRUSTA_flag() const
{
   return erase_other_OBSFRUSTA_flag;
}

inline void OBSFRUSTAGROUP::set_jump_to_apex_flag(bool flag)
{
   jump_to_apex_flag=flag;
}

inline int OBSFRUSTAGROUP::get_n_3D_rays() const
{
   return n_3D_rays;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_FOV_excess_fill_factor(double factor)
{
   FOV_excess_fill_factor=factor;
}

inline double OBSFRUSTAGROUP::get_FOV_excess_fill_factor() const
{
   return FOV_excess_fill_factor;
}

inline void OBSFRUSTAGROUP::set_groundplane_pi(const fourvector& pi)
{
   groundplane_pi=pi;
}

inline const fourvector& OBSFRUSTAGROUP::get_groundplane_pi() const
{
   return groundplane_pi;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_z_ColorMap_ptr(ColorMap* cmap_ptr) 
{
   z_ColorMap_ptr=cmap_ptr;
}

// --------------------------------------------------------------------------
inline OBSFRUSTUM* OBSFRUSTAGROUP::get_OBSFRUSTUM_ptr(int n) const
{
//   cout << "inside OBSFRUSTAGROUP::get_OBSFRUSTUM_ptr(), n = " << n << endl;
//   OBSFRUSTUM* OBSFRUSTUM_ptr=dynamic_cast<OBSFRUSTUM*>(get_Graphical_ptr(n));
//   cout << "returned OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
//   return OBSFRUSTUM_ptr;

   return dynamic_cast<OBSFRUSTUM*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline OBSFRUSTUM* OBSFRUSTAGROUP::get_ID_labeled_OBSFRUSTUM_ptr(
   int ID) const
{
   return dynamic_cast<OBSFRUSTUM*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline MoviesGroup* OBSFRUSTAGROUP::get_MoviesGroup_ptr() 
{
   return MoviesGroup_ptr;
}

// --------------------------------------------------------------------------
inline std::vector<bounding_box>& OBSFRUSTAGROUP::get_photo_zplane_bboxes() 
{
   return photo_zplane_bboxes;
}

inline const std::vector<bounding_box>& OBSFRUSTAGROUP::get_photo_zplane_bboxes() const
{
   return photo_zplane_bboxes;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_ArrowHUD_ptr(ArrowHUD* AH_ptr)
{
   ArrowHUD_ptr=AH_ptr;
}

inline void OBSFRUSTAGROUP::set_ArrowsGroup_ptr(ArrowsGroup* AG_ptr)
{
   ArrowsGroup_ptr=AG_ptr;
}

inline ArrowsGroup* OBSFRUSTAGROUP::get_ArrowsGroup_ptr()
{
   return ArrowsGroup_ptr;
}

inline const ArrowsGroup* OBSFRUSTAGROUP::get_ArrowsGroup_ptr() const
{
   return ArrowsGroup_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_PointCloudsGroup_ptr(
   PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}

inline PointCloudsGroup* OBSFRUSTAGROUP::get_PointCloudsGroup_ptr()
{
   return PointCloudsGroup_ptr;
}

inline const PointCloudsGroup* OBSFRUSTAGROUP::get_PointCloudsGroup_ptr() 
   const
{
   return PointCloudsGroup_ptr;
}


inline void OBSFRUSTAGROUP::set_PointCloud_ptr(PointCloud* cloud_ptr)
{
   PointCloud_ptr=cloud_ptr;
}

inline PointCloud* OBSFRUSTAGROUP::get_PointCloud_ptr()
{
   return PointCloud_ptr;
}

inline const PointCloud* OBSFRUSTAGROUP::get_PointCloud_ptr() const
{
   return PointCloud_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_ImageNumberHUD_ptr(
   ImageNumberHUD* ImageNumberHUD_ptr)
{
   this->ImageNumberHUD_ptr=ImageNumberHUD_ptr;
}

inline ImageNumberHUD* OBSFRUSTAGROUP::get_ImageNumberHUD_ptr()
{
   return ImageNumberHUD_ptr;
}

inline const ImageNumberHUD* OBSFRUSTAGROUP::get_ImageNumberHUD_ptr() const
{
   return ImageNumberHUD_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_WindowManager_ptr(WindowManager* WM_ptr)
{
   WindowManager_ptr=WM_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_PolyhedraGroup_ptr(PolyhedraGroup* PG_ptr)
{
   PolyhedraGroup_ptr=PG_ptr;
}

inline void OBSFRUSTAGROUP::set_PointsGroup_ptr(
   osgGeometry::PointsGroup* PG_ptr)
{
   PointsGroup_ptr=PG_ptr;
}

inline osgGeometry::PointsGroup* OBSFRUSTAGROUP::get_PointsGroup_ptr()
{
   return PointsGroup_ptr;
}

inline const osgGeometry::PointsGroup* OBSFRUSTAGROUP::get_PointsGroup_ptr() 
   const
{
   return PointsGroup_ptr;
}

inline void OBSFRUSTAGROUP::set_PolygonsGroup_ptr(
   osgGeometry::PolygonsGroup* PG_ptr)
{
   PolygonsGroup_ptr=PG_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr)
{
   DTED_ztwoDarray_ptr=ztwoDarray_ptr;
}

inline twoDarray* OBSFRUSTAGROUP::get_DTED_ztwoDarray_ptr()
{
   return DTED_ztwoDarray_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_cameraid_xyz_map_ptr(
   videofunc::CAMERAID_XYZ_MAP* map_ptr)
{
   cameraID_xyz_map_ptr=map_ptr;
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_photogroup_ptr(photogroup* pg_ptr)
{
   photogroup_ptr=pg_ptr;
}

inline photogroup* OBSFRUSTAGROUP::get_photogroup_ptr()
{
   return this->photogroup_ptr;
}

// --------------------------------------------------------------------------
inline void OBSFRUSTAGROUP::set_prev_OBSFRUSTUM_framenumber(int framenumber)
{
   prev_OBSFRUSTUM_framenumber=framenumber;
}

inline int OBSFRUSTAGROUP::get_prev_OBSFRUSTUM_framenumber() const
{
   return prev_OBSFRUSTUM_framenumber;
}

// --------------------------------------------------------------------------

inline OBSFRUSTUM* OBSFRUSTAGROUP::get_virtual_OBSFRUSTUM_ptr()
{
   return virtual_OBSFRUSTUM_ptr;
}

inline const OBSFRUSTUM* OBSFRUSTAGROUP::get_virtual_OBSFRUSTUM_ptr() const
{
   return virtual_OBSFRUSTUM_ptr;
}

inline camera* OBSFRUSTAGROUP::get_virtual_camera_ptr()
{
   return virtual_camera_ptr;
}

inline const camera* OBSFRUSTAGROUP::get_virtual_camera_ptr() const
{
   return virtual_camera_ptr;
}

// --------------------------------------------------------------------------
// Subfrusta set/get methods

inline int OBSFRUSTAGROUP::get_new_subfrustum_ID() const
{
   return new_subfrustum_ID;
}

inline void OBSFRUSTAGROUP::set_new_subfrustum_ID(int ID)
{
   new_subfrustum_ID=ID;
}

inline void OBSFRUSTAGROUP::set_subfrustum_color(colorfunc::Color c)
{
   subfrustum_color=c;
}

inline void OBSFRUSTAGROUP::set_subfrustum_volume_alpha(double alpha)
{
   subfrustum_volume_alpha=alpha;
}

inline OBSFRUSTAGROUP* OBSFRUSTAGROUP::get_SUBFRUSTAGROUP_ptr()
{
   return SUBFRUSTAGROUP_ptr;
}

inline void OBSFRUSTAGROUP::set_subfrustum_bbox_ptr(bounding_box* bb_ptr)
{
   subfrustum_bbox_ptr=bb_ptr;
}

inline void OBSFRUSTAGROUP::set_subfrustum_downrange_distance(double distance)
{
   subfrustum_downrange_distance=distance;
}

inline void OBSFRUSTAGROUP::pushback_subfrustum_bbox_ptr(bounding_box* bb_ptr)
{
   subfrusta_bbox_ptrs.push_back(bb_ptr);
}

inline std::vector<bounding_box*>& OBSFRUSTAGROUP::get_subfrusta_bbox_ptrs()
{
   return subfrusta_bbox_ptrs;
}

inline const std::vector<bounding_box*>& OBSFRUSTAGROUP::get_subfrusta_bbox_ptrs() const
{
   return subfrusta_bbox_ptrs;
}

inline void OBSFRUSTAGROUP::null_SUBFRUSTUM_ptr()
{
   sub_FRUSTUM_ptr=NULL;
}

inline OBSFRUSTUM* OBSFRUSTAGROUP::get_sub_FRUSTUM_ptr()
{
   return sub_FRUSTUM_ptr;
}

inline const OBSFRUSTUM* OBSFRUSTAGROUP::get_sub_FRUSTUM_ptr() const
{
   return sub_FRUSTUM_ptr;
}

inline void OBSFRUSTAGROUP::set_play_OBSFRUSTA_as_movie_flag(bool flag)
{
   play_OBSFRUSTA_as_movie_flag=flag;
}

inline void OBSFRUSTAGROUP::set_project_frames_onto_zplane_flag(bool flag)
{
   project_frames_onto_zplane_flag=flag;
}


#endif // OBS_FRUSTAGROUP.h



