// ==========================================================================
// Header file for LOSMODELSGROUP class
// ==========================================================================
// Last modified on 5/4/12; 5/16/12; 5/19/13; 4/5/14
// ==========================================================================

#ifndef LOSMODELSGROUP_H
#define LOSMODELSGROUP_H

#include <iostream>
#include <map>

#include "postgres/gis_database.h"
#include "osg/osgModels/LOSMODEL.h"
#include "math/lttwovector.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/ReferenceFrameHUD.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "image/TwoDarray.h"

class LineSegment;
class LineSegmentsGroup;
class Messenger;
class Movie;
class MoviesGroup;
class texture_rectangle;

class LOSMODELSGROUP : public MODELSGROUP
{

  public:

   enum RAY_TYPE
   {
      single_air_to_multi_ground,multi_air_to_single_ground
   };

// Initialization, constructor and destructor functions:

   LOSMODELSGROUP(
      Pass* PI_ptr,threevector* GO_ptr=NULL,
      AnimationController* AC_ptr=NULL);
   LOSMODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
                  PolyLinePickHandler* PLPH_ptr,
                  threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
                  Operations* O_ptr);
   LOSMODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
                  PolyLinePickHandler* PLPH_ptr,
                  threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
                  Operations* O_ptr,MoviesGroup* MG_ptr);
   virtual ~LOSMODELSGROUP();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const LOSMODELSGROUP& M);

// Set & get methods:

   void set_altitude_dependent_MODEL_scale_flag(bool flag);
   void set_ladar_height_data_flag(bool flag);
   void set_raytrace_ground_targets_flag(bool flag);
   void set_reload_DTED_tiles_flag(bool flag);
   void set_update_dynamic_aircraft_MODEL_flag(bool flag);
   void set_fixed_aircraft_MODEL_orientation_flag(bool flag);
   void set_north_up_orientation_flag(bool flag);

   void set_min_raytrace_range(double r);
   void set_max_raytrace_range(double r);
   void set_max_ground_Z(double z);
   double get_raytrace_cellsize() const;
   void set_raytrace_progress(double progress);
   void set_ray_type(RAY_TYPE rt);
   RAY_TYPE get_ray_type() const;
   LOSMODEL* get_LOSMODEL_ptr(int n) const;
   LOSMODEL* get_ID_labeled_LOSMODEL_ptr(int ID) const;
   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   const LineSegmentsGroup* get_LineSegmentsGroup_ptr() const;
   void set_GroundTarget_SignPostsGroup_ptr(SignPostsGroup* GTSPG_ptr);
   void set_images_database_ptr(gis_database* db_ptr);
   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);
   void set_ReferenceFrameHUD_ptr(ReferenceFrameHUD* RFH_ptr);

// MODEL creation and manipulation methods:

   MODEL* generate_empty_MODEL(
      double min_raytrace_range,double max_raytrace_range);
   MODEL* generate_LiMIT_MODEL();
   MODEL* generate_LiMIT_MODEL_for_flight_PolyLine(
      PolyLine* flight_PolyLine_ptr);
   void set_altitude_dependent_MODEL_scale();
   void update_display();

// Model destruction methods:

// Model path creation methods:

// OBSFRUSTA generation member functions:

   void compute_OBSFRUSTUM_parameters(
      double min_ground_sensor_range,double max_ground_sensor_range,
      double horiz_FOV,int roll_sgn);

// Line-of-sight analysis member functions:

   void display_average_LOS_results();
   std::string compute_avg_LOS_ptwoDarray(std::string Ptiles_subdir);
   std::vector<std::string> find_avg_LOS_geotif_files(
      std::string geotif_Ptiles_subdir,
      ColorGeodeVisitor* ColorGeodeVisitor_ptr);
   void mark_current_FOV_midpoint();
   void mark_ground_intercept(
      const threevector& aerial_camera_posn,const threevector& r_hat,
      std::string label="");
   void export_average_occlusion_files(
      std::string dirname,std::string basename,
      std::string output_geotif_filename,std::string output_nitf_filename);
   
// Visibility skymap member functions:

   bool generate_target_visibility_skymaps(
      double lower_left_longitude,double lower_left_latitude,
      double upper_right_longitude,double upper_right_latitude);
   void clear_visibility_skymaps();

// Public automatic flight path planning member functions:

   double compute_optimal_clockwise_circular_flightpath(
      double& center_longitude,double& center_latitude,double& orbit_radius);
   double compute_optimal_clockwise_ellipse_params(
      double& center_longitude,double& center_latitude,
      double& ellipse_a,double& ellipse_b,double& ellipse_phi);
   double generate_ellipse_flightpath(
      double center_longitude,double center_latitude,
      double a,double b,double phi);
   double compute_optimal_linesegment_params(twovector& r1,twovector& r2);
   double generate_linesegment_flightpath(
      const twovector& r1,const twovector& r2);

// Ground target visibility member functions:

//   bool compute_target_visibilities_along_flightpath(
//      MODEL* MODEL_ptr,std::vector<OBSFRUSTUM*>& OBSFRUSTA_ptrs,
//      bool reload_DTED_tiles_flag=true);
   bool compute_target_visibilities_along_flightpath(
      MODEL* MODEL_ptr,bool reload_DTED_tiles_flag=true);

   void clear_raytracing_results();
   void set_display_ImageNumberHUD(bool flag);
   int get_ground_target_visibility(double curr_time,int target_ID) const;
   int get_ground_target_visibility(double curr_time,int target_ID,
                                    threevector& occlusion_posn) const;
   void draw_colored_single_air_to_multi_ground_rays(
      bool tgts_prev_raytraced_flag,MODEL* MODEL_ptr);
   void draw_colored_multi_air_to_single_ground_rays(
      int target_ID,MODEL* MODEL_ptr);
   void purge_multi_air_to_single_ground_linesegments();
   double averaged_ground_target_visibility();

// SAM threat map member functions:

   Movie* visualize_SAM_threatmap(
      double SAM_range,
      double threatmap_longitude_lo,double threatmap_longitude_hi,
      double threatmap_latitude_lo,double threatmap_latitude_hi);
   void set_threatmap_corners(
      double threatmap_longitude_lo,double threatmap_longitude_hi,
      double threatmap_latitude_lo,double threatmap_latitude_hi,
      double threatmap_altitude);
   void generate_SAM_threatmap(
      double SAM_range,double threatmap_progress,std::string progress_type);
   void clear_SAM_threatmap();

   void test_skymap_interpolation();

// Ground target geolocation via raytraced visibility map comparisons:

   bool generate_target_visibility_omnimap();
   twoDarray* initialize_groundmap(
      const geopoint& lower_left_corner,const geopoint& upper_right_corner,
      int mdim,int ndim);
   twoDarray* initialize_groundmap(
      double groundmap_xlo,double groundmap_xhi,
      double groundmap_ylo,double groundmap_yhi,int mdim,int ndim);
   void raytrace_visibility_omnimap(
      const threevector& apex,double max_ground_Z,double max_raytrace_range,
      double min_raytrace_range,double ds,twoDarray* omni_twoDarray_ptr,
      twoDarray* DTED_ztwoDarray_ptr,twoDarray* reduced_DTED_ztwoDarray_ptr,
      OBSFRUSTUM* OBSFRUSTUM_ptr);
   
   void display_omni_occlusion(twoDarray* ptwoDarray_ptr);
   void export_visibility_omnimap(
      std::string visiblity_filename,const threevector& apex,
      twoDarray* ptwoDarray_ptr);
   twoDarray* import_visibility_omnimap(
      std::string visibility_filename,threevector& transmitter_posn);
   twoDarray* smear_visibility_omnimap(twoDarray* ptwoDarray_ptr);

   void fit_ground_target_position(
      const threevector& transmitter_posn,twoDarray* measured_twoDarray_ptr);

// Moving ground target member functions:

   void raytrace_moving_ground_target();
   void export_moving_ground_target_visibility_rays();
   void import_moving_ground_target_visibility_rays();

// Real-time manipulation of aircraft models

   void update_dynamic_aircraft_model();
   void update_dynamic_aircraft_model(
      const geopoint& aerial_point,const rpy& aircraft_RPY,
      const threevector& frustum_AER,const twovector& frustum_FOV);
   void toggle_between_earth_and_aircraft_reference_frames();

// PYXIS server member functions:

   void set_PYXIS_server_flag(bool flag);
   void set_PYXIS_output_subdir(std::string subdir);
   void set_PYXIS_output_basename(std::string basename);
   void start_ROI_visibility_computation();
   void export_PYXIS_ROI_visibility_files();

  protected:

  private:

   bool altitude_dependent_MODEL_scale_flag,ladar_height_data_flag;
   bool raytrace_ground_targets_flag,reload_DTED_tiles_flag,
      score_broadcasted_flag;
   bool update_dynamic_aircraft_MODEL_flag;
   bool fixed_aircraft_MODEL_orientation_flag,north_up_orientation_flag;
   bool PYXIS_server_flag;
   std::string PYXIS_output_subdir,PYXIS_output_basename;
   
   RAY_TYPE ray_type;
   double skymap_xlo,skymap_xhi,skymap_ylo,skymap_yhi,skymap_ds;
   double skymap_azlo,skymap_azhi,skymap_daz;
   double min_raytrace_range,max_raytrace_range,max_ground_Z;
   double endpoint_size_prefactor;

   double xc_min,xc_max,yc_min,yc_max,r_min,r_max,theta_min,theta_max;
   double a_min,a_max,b_min,b_max,phi_min,phi_max;
   double x1_min,x2_min,y1_min,y2_min;
   double x1_max,x2_max,y1_max,y2_max;
   double mu_xc,sigma_xc,mu_yc,sigma_yc,mu_r,sigma_r,mu_theta,sigma_theta;
   double mu_a,sigma_a,mu_b,sigma_b,mu_phi,sigma_phi;
   double mu_x1,sigma_x1,mu_y1,sigma_y1;
   double mu_x2,sigma_x2,mu_y2,sigma_y2;
   double raytrace_progress;

   double horiz_frustum_FOV,vert_frustum_FOV;

// Store ground target visibility (-1 = target lies outside sensor's
// field of regard, 0 = target is occluded , 1 = target is visible)
// and occlusion position as a function of twovector (time, target ID)
// within *target_visibility_map_ptr:

   typedef std::map<twovector,std::pair<int,threevector>, lttwovector> 
      TARGET_VISIBILITY_MAP;
   TARGET_VISIBILITY_MAP* target_visibility_map_ptr;

   geopoint lower_left_threatmap_corner,upper_right_threatmap_corner;
   LineSegmentsGroup* LineSegmentsGroup_ptr; // holds raytracing segments
   Messenger* cancel_messenger_ptr;
   MoviesGroup* MoviesGroup_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   gis_database* images_database_ptr;
   ReferenceFrameHUD* ReferenceFrameHUD_ptr;
   SignPostsGroup* GroundTarget_SignPostsGroup_ptr;
   texture_rectangle* threat_texture_rectangle_ptr;
   twoDarray* threatmap_twoDarray_ptr;

// Store ground target skymaps as a function of twovector 
// (azimuthal heading index, target ID) within *target_skymap_map_ptr:

   typedef std::map<twovector,twoDarray*, lttwovector> TARGET_SKYMAP_MAP;
   TARGET_SKYMAP_MAP* target_skymap_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const LOSMODELSGROUP& M);

   void initialize_skymaps(
      unsigned int n_ground_targets,
      const geopoint& lower_left_corner,const geopoint& upper_right_corner,
      twoDarray*& skymap_twoDarray_ptr,
      twoDarray*& skymap_Xsum_twoDarray_ptr,
      twoDarray*& skymap_Ysum_twoDarray_ptr);
   void load_heightfields(
      const std::vector<twovector>& target_posns,
      twoDarray*& DTED_ztwoDarray_ptr,twoDarray*& reduced_DTED_ztwoDarray_ptr,
      twoDarray*& DTED_ptwoDarray_ptr);
   void load_heightfields(
      const geopoint& lower_left_corner,const geopoint& upper_right_corner,
      twoDarray*& DTED_ztwoDarray_ptr,twoDarray*& reduced_DTED_ztwoDarray_ptr,
      twoDarray*& DTED_ptwoDarray_ptr);
   void write_out_ground_target_posns();
   void write_out_skymap_text_files(
      double theta_deg,twoDarray* skymap_twoDarray_ptr);
   void write_out_individual_target_skymap_text_files();
   void write_out_skymap_PNG_files(
      MODEL* MODEL_ptr,double theta_deg,twoDarray* skymap_twoDarray_ptr,
      twoDarray* skymap_phase_twoDarray_ptr);
   void export_skymap(
      const std::vector<twovector>& target_posns,double theta,
      twoDarray* skymap_twoDarray_ptr);
   void accumulate_skymap_flowfield(
      double theta,twoDarray* skymap_twoDarray_ptr,
      twoDarray* skymap_Xsum_twoDarray_ptr,
      twoDarray* skymap_Ysum_twoDarray_ptr);
   void compute_average_skymap_flowfield(
      twoDarray* skymap_Xsum_twoDarray_ptr,
      twoDarray* skymap_Ysum_twoDarray_ptr,
      twoDarray* skymap_twoDarray_ptr,twoDarray* skymap_phase_twoDarray_ptr);
   void export_flowfield_geocoords();

// Automatic flight path planning member functions

   void generate_circle_orbit_param_samples(
      int iter,int n_samples,
      std::vector<double>& xc_vars,std::vector<double>& yc_vars,
      std::vector<double>& r_vars);
   void generate_ellipse_orbit_param_samples(
      int iter,int n_samples,
      std::vector<double>& xc_vars,std::vector<double>& yc_vars,
      std::vector<double>& a_vars,std::vector<double>& b_vars,
      std::vector<double>& phi_vars);
   void generate_linesegment_param_samples(
      int iter,int n_samples,
      std::vector<double>& x1_vars,std::vector<double>& y1_vars,
      std::vector<double>& x2_vars,std::vector<double>& y2_vars);

   void initialize_cross_entropy_distribution_params(
      double xmin,double xmax,double& mu,double& sigma);
   std::vector<double> generate_random_vars(
      unsigned int n_samples,double mu,double sigma,double xmin,double xmax);
   double candidate_circle_orbit_score(double xc,double yc,double r);
   double candidate_ellipse_orbit_score(
      double xc,double yc,double a,double b,double phi);
   double candidate_linesegment_score(
      double x1,double y1,double x2,double y2);

   double get_interpolated_groundtarget_visibility_flag(
      double x,double y,double az_heading,int g);

   virtual bool parse_next_message_in_queue(message& curr_message);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void LOSMODELSGROUP::set_altitude_dependent_MODEL_scale_flag(bool flag)
{
   altitude_dependent_MODEL_scale_flag=flag;
}

inline void LOSMODELSGROUP::set_ladar_height_data_flag(bool flag)
{
   ladar_height_data_flag=flag;
   endpoint_size_prefactor=0.2;
   set_min_raytrace_range(0);
   set_max_raytrace_range(500*1000);
}

inline void LOSMODELSGROUP::set_raytrace_ground_targets_flag(bool flag)
{
   raytrace_ground_targets_flag=flag;
}

inline void LOSMODELSGROUP::set_reload_DTED_tiles_flag(bool flag)
{
   reload_DTED_tiles_flag=flag;
}

inline void LOSMODELSGROUP::set_update_dynamic_aircraft_MODEL_flag(bool flag)
{
   update_dynamic_aircraft_MODEL_flag=flag;
}

inline void LOSMODELSGROUP::set_fixed_aircraft_MODEL_orientation_flag(
   bool flag)
{
   fixed_aircraft_MODEL_orientation_flag=flag;
}

inline void LOSMODELSGROUP::set_north_up_orientation_flag(bool flag)
{
   north_up_orientation_flag=flag;
}

inline void LOSMODELSGROUP::set_PYXIS_server_flag(bool flag)
{
   PYXIS_server_flag=flag;
}

inline void LOSMODELSGROUP::set_PYXIS_output_subdir(string subdir)
{
   PYXIS_output_subdir=subdir;
}

inline void LOSMODELSGROUP::set_PYXIS_output_basename(string basename)
{
   PYXIS_output_basename=basename;
}

// --------------------------------------------------------------------------
inline void LOSMODELSGROUP::set_max_raytrace_range(double r)
{
   max_raytrace_range=r;
}

inline void LOSMODELSGROUP::set_min_raytrace_range(double r)
{
   min_raytrace_range=r;
}

inline void LOSMODELSGROUP::set_max_ground_Z(double z)
{
   max_ground_Z=z;
}

inline void LOSMODELSGROUP::set_raytrace_progress(double progress)
{
   raytrace_progress=progress;
}

inline void LOSMODELSGROUP::set_ray_type(LOSMODELSGROUP::RAY_TYPE rt)
{
   ray_type=rt;
}

inline LOSMODELSGROUP::RAY_TYPE LOSMODELSGROUP::get_ray_type() const
{
   return ray_type;
}

// --------------------------------------------------------------------------
inline LOSMODEL* LOSMODELSGROUP::get_LOSMODEL_ptr(int n) const
{
   return dynamic_cast<LOSMODEL*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline LOSMODEL* LOSMODELSGROUP::get_ID_labeled_LOSMODEL_ptr(int ID) const
{
   return dynamic_cast<LOSMODEL*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline void LOSMODELSGROUP::set_images_database_ptr(gis_database* db_ptr)
{
   images_database_ptr=db_ptr;
}

// --------------------------------------------------------------------------
inline LineSegmentsGroup* LOSMODELSGROUP::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

inline const LineSegmentsGroup* LOSMODELSGROUP::get_LineSegmentsGroup_ptr() const
{
   return LineSegmentsGroup_ptr;
}

// --------------------------------------------------------------------------
inline void LOSMODELSGROUP::set_GroundTarget_SignPostsGroup_ptr(
   SignPostsGroup* GTSPG_ptr)
{
   GroundTarget_SignPostsGroup_ptr=GTSPG_ptr;
}

// --------------------------------------------------------------------------
inline void LOSMODELSGROUP::set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}

// --------------------------------------------------------------------------
inline void LOSMODELSGROUP::set_ReferenceFrameHUD_ptr(
   ReferenceFrameHUD* RFH_ptr)
{
   ReferenceFrameHUD_ptr=RFH_ptr;
}


#endif // LOSLOSMODELSGROUP.h

