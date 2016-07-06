// ==========================================================================
// Header file for MODELSGROUP class
// ==========================================================================
// Last modified on 10/9/11; 10/15/11; 2/1/12; 4/5/14
// ==========================================================================

#ifndef NEW_MODELSGROUP_H
#define NEW_MODELSGROUP_H

#include <iostream>
#include <osg/Group>
#include <osg/Node>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osgGeometry/ArrowsGroup.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODEL.h"
#include "track/movers_group.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "track/tracks_group.h"
#include "datastructures/Triple.h"
#include "osg/osgWindow/WindowManager.h"

class AnimationController;
class EarthRegion;
class geopoint;
class Movie;
class OBSFRUSTUM;
class Operations;
class PolyLinesGroup;
class PolyLinePickHandler;

class MODELSGROUP : public GeometricalsGroup, public AnnotatorsGroup
{

  public:

   enum AircraftModelType
      {
         predator,LiMIT,cessna
      };

// Initialization, constructor and destructor functions:

   MODELSGROUP(Pass* PI_ptr,threevector* GO_ptr=NULL,
               AnimationController* AC_ptr=NULL);
   MODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
               AnimationController* AC_ptr=NULL);
   MODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
               threevector* GO_ptr,AnimationController* AC_ptr=NULL);

   MODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
               PolyLinePickHandler* PLPH_ptr,
               threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
               AnimationController* AC_ptr=NULL);
   MODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
               PolyLinePickHandler* PLPH_ptr,
               threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
               Operations* O_ptr);

   virtual ~MODELSGROUP();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const MODELSGROUP& M);

// Set & get methods:

   void set_AircraftModelType(AircraftModelType model_type);
   AircraftModelType get_AircraftModelType() const;
   void set_generate_Predator_model_on_next_cycle_flag(bool flag);
   void set_alter_UAV_path_flag(bool flag);
   void set_continuously_plan_UAV_paths_flag(bool flag);
   void set_fade_UAV_track_color_flag(bool flag);
   void set_compute_passed_ground_targets_flag(bool flag);
   void set_long_initial_UAV_track_flag(bool flag);
   void set_compute_zface_height_flag(bool flag);
   void set_instantiate_OBSFRUSTAGROUP_flag(bool flag);
   void set_instantiate_OBSFRUSTUMPickHandler_flag(bool flag);
   void set_double_LiMIT_lobe_pattern_flag(bool flag);
   void set_loiter_at_track_end_flag(bool flag);

   void set_aircraft_altitude(double alt);
   double get_aircraft_altitude() const;
   void set_LiMIT_speed(double speed);
   double get_LiMIT_speed() const;

   void set_ground_bbox_ptr(bounding_box* ground_bbox_ptr);
   bounding_box* get_ground_bbox_ptr();

   void set_OBSFRUSTUM_az_extent(double delta_az);
   void set_OBSFRUSTUM_el_extent(double delta_el);
   void set_OBSFRUSTUM_roll(double roll);
   void set_OBSFRUSTUM_pitch(double pitch);

   void set_ID_for_path_to_alter(int ID);
   void set_n_future_repeats(unsigned int repeats);
   void set_model_filename(std::string filename);
   std::string get_model_filename() const;
   void set_tomcat_subdir(std::string subdir);
   std::string get_tomcat_subdir() const;
   MODEL* get_MODEL_ptr(int n) const;
   MODEL* get_ID_labeled_MODEL_ptr(int ID) const;
   std::vector<MODEL*> get_all_MODEL_ptrs() const;
   void set_ColorGeodeVisitor_ptr(ColorGeodeVisitor* CGV_ptr);

   tracks_group* get_tracks_group_ptr();
   const tracks_group* get_tracks_group_ptr() const;
   movers_group* get_movers_group_ptr();
   const movers_group* get_movers_group_ptr() const;
   std::vector<threevector>& get_even_path_point();
   const std::vector<threevector>& get_even_path_point() const;

   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);
   EarthRegionsGroup* get_EarthRegionsGroup_ptr();
   const EarthRegionsGroup* get_EarthRegionsGroup_ptr() const;
   void set_TilesGroup_ptr(TilesGroup* TG_ptr);

   void set_flightpath_fraction_offset(double frac_offset);
   PolyLinesGroup* get_Path_PolyLinesGroup_ptr();
   const PolyLinesGroup* get_Path_PolyLinesGroup_ptr() const;
   PolyLinePickHandler* get_Path_PolyLinePickHandler_ptr();
   const PolyLinePickHandler* get_Path_PolyLinePickHandler_ptr() const;

   void set_ModeController_ptr(ModeController* MC_ptr);
   void set_WindowManager_ptr(WindowManager* WCC_ptr);
   void set_ROI_PolyhedraGroup_ptr(PolyhedraGroup* PHG_ptr);
   void set_ArrowsGroup_ptr(ArrowsGroup* AG_ptr);
   ArrowsGroup* get_ArrowsGroup_ptr();
   const ArrowsGroup* get_ArrowsGroup_ptr() const;

   Messenger* get_viewer_Messenger_ptr();
   const Messenger* get_viewer_Messenger_ptr() const;
   void assign_MODELSGROUP_Messenger_ptrs();

// Model creation and manipulation methods:

   MODEL* generate_new_Model(
      std::string model_filename,int OSGsubPAT_number=0,int ID=-1);
   MODEL* generate_new_Model(int OSGsubPAT_number=0,int ID=-1);

   OBSFRUSTUM* instantiate_OBSFRUSTUM(
      MODEL* MODEL_ptr,double az_extent,double el_extent,
      std::string movie_filename="",int OSGsubPAT_number=0);
   OBSFRUSTUM* instantiate_OBSFRUSTUM(
      MODEL* MODEL_ptr,double az_extent,double el_extent,Movie* Movie_ptr,
      int OSGsubPAT_number=0);
      
   OBSFRUSTUM* instantiate_second_OBSFRUSTUM(
      MODEL* MODEL_ptr,int OSGsubPAT_number=0);

   MODEL* move_z(int sgn);
   void change_scale(double scalefactor);

   void unmask_next_model();
   void toggle_model_mask(int OSGsubPAT_number);

// Model destruction methods:

   void destroy_all_MODELS();
   bool destroy_MODEL(int ID);
   bool destroy_last_created_MODEL();
   bool destroy_MODEL(MODEL* curr_MODEL_ptr);

// Model path creation methods:

   void record_waypoint();
   void finish_waypoint_entry();
   track* update_UAV_track(
      PolyLine* flight_PolyLine_ptr,MODEL* UAV_MODEL_ptr);
   track* update_UAV_track(
      int initial_framenumber,PolyLine* flight_PolyLine_ptr,
      MODEL* UAV_MODEL_ptr);
   void recolor_UAV_track(
      track* UAV_track_ptr,PolyLine* flight_PolyLine_ptr);

   void generate_racetrack_orbit(
      MODEL* curr_MODEL_ptr,double center_longitude,double center_latitude,
      double radius,double height_above_center,double omega,
      double phase_offset=0);
   void generate_racetrack_orbit(
      MODEL* curr_MODEL_ptr,const threevector& center_origin,double radius,
      double height_above_center,double omega,double phase_offset=0);
   void generate_racetrack_orbit(
      MODEL* MODEL_ptr,const std::vector<Triple<threevector,rpy,int> >& 
      sensor_posn_orientation_frames,
      AnimationController* movie_anim_controller_ptr);
   void generate_racetrack_orbit(
      const std::vector<threevector>& posn_reg,MODEL* MODEL_ptr);
   PolyLine* generate_circular_PolyLine_Path(
      double center_longitude,double center_latitude,double orbit_radius,
      int flightpath_sgn);
   void regularize_aircraft_posns_and_orientations(
      const std::vector<Triple<threevector,rpy,int> >& 
      sensor_posn_orientation_frames,
      AnimationController* movie_anim_controller_ptr,
      std::vector<threevector>& posn_reg,
      std::vector<threevector>& orientation_reg);

// Aircraft MODEL generation member functions:

   MODEL* generate_Cessna_MODEL(
      int& OSGsubPAT_number,double model_scalefactor=5.0,
      colorfunc::Color model_color=colorfunc::blue);
   MODEL* generate_predator_MODEL(
      std::string model_filename="",double predator_speed=100,
      double model_scalefactor=1.0);
   MODEL* generate_predator_MODEL(
      int& OSGsubPAT_number,std::string model_filename="",
      double predator_speed=100,double model_scalefactor=1.0);
   MODEL* generate_LiMIT_MODEL(
      int& OSGsubPAT_number,bool use_AWACS_model_flag=true);
   MODEL* generate_LiMIT_MODEL();
   MODEL* generate_LiMIT_MODEL_for_flight_PolyLine(
      PolyLine* flight_PolyLine_ptr);

   void generate_predator_racetrack_orbit(
      int n_total_frames,MODEL* predator_MODEL_ptr);
   void generate_predator_racetrack_orbit(
      int n_total_frames,double racetrack_center_longitude,
      double racetrack_center_latitude,MODEL* predator_MODEL_ptr);
   void generate_elliptical_LiMIT_racetrack_orbit(
      int n_total_frames,const geopoint& racetrack_center,
      MODEL* LiMIT_ptr);
   void generate_elliptical_LiMIT_racetrack_orbit(
      int n_total_frames,const threevector& racetrack_center,
      MODEL* LiMIT_ptr);

// OBSFRUSTA generation member functions:

   OBSFRUSTUM* instantiate_predator_OBSFRUSTUM(
      int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
      double predator_az_extent,double predator_el_extent,
      std::string movie_filename="",double zface_offset=100);
   OBSFRUSTUM* instantiate_predator_OBSFRUSTUM(
      int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
      double predator_az_extent,double predator_el_extent,
      Movie* Movie_ptr,double zface_offset=100);

   OBSFRUSTUM* generate_predator_OBSFRUSTUM(
      int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
      double predator_az_extent,double predator_el_extent,
      std::string movie_filename="",double zface_offset=100);
   OBSFRUSTUM* generate_predator_OBSFRUSTUM(
      int first_framenumber,int last_framenumber,
      int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
      double predator_az_extent,double predator_el_extent,
      std::string movie_filename="",double zface_offset=100);
   OBSFRUSTUM* generate_predator_OBSFRUSTUM(
      int first_framenumber,int last_framenumber,
      int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
      double predator_az_extent,double predator_el_extent,
      Movie* Movie_ptr,double zface_offset=100);

   OBSFRUSTUM* instantiate_LiMIT_FOV_OBSFRUSTUM(
      int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
      double FOV_az_extent,double FOV_el_extent);
   OBSFRUSTUM* generate_LiMIT_FOV_OBSFRUSTUM(
      int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
      double FOV_az_extent,double FOV_el_extent);
   OBSFRUSTUM* generate_LiMIT_FOV_OBSFRUSTUM(
      int first_framenumber,int last_framenumber,
      int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
      double FOV_az_extent,double FOV_el_extent);
   void set_LiMIT_FOV_OBSFRUSTUM_colors(
      OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,double volume_alpha);
   void set_altitude_dependent_OBSFRUSTA_volume_alpha(
      std::vector<OBSFRUSTUM*>& OBSFRUSTUM_ptrs);
   void set_altitude_dependent_Polyhedron_alpha(Polyhedron* Polyhedron_ptr);

   OBSFRUSTUM* generate_LiMIT_instantaneous_dwell_OBSFRUSTUM(
      int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
      OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,EarthRegion* region_ptr);

   OBSFRUSTUM* instantiate_MODEL_FOV_OBSFRUSTUM(
      int MODEL_OSGsubPAT_number,MODEL* MODEL_ptr,
      double FOV_az_extent,double FOV_el_extent);
   OBSFRUSTUM* generate_MODEL_FOV_OBSFRUSTUM(
      int MODEL_OSGsubPAT_number,MODEL* MODEL_ptr,
      double FOV_az_extent,double FOV_el_extent);
   OBSFRUSTUM* generate_MODEL_FOV_OBSFRUSTUM(
      int MODEL_OSGsubPAT_number,MODEL* MODEL_ptr,
      int first_framenumber,int last_framenumber,
      double FOV_az_extent,double FOV_el_extent);
//   void display_average_LOS_results();

// HAFB video3D methods:

   MODEL* generate_HAFB_video_pass_model(
      double z_rot_angle,const threevector& first_frame_aircraft_posn,
      AnimationController* HAFB_AnimationController_ptr);
//   MODEL* generate_cessna_model(
//      double z_rot_angle,const threevector& first_frame_aircraft_posn,
//      AnimationController* HAFB_AnimationController_ptr);
   void read_HAFB_plane_info(
      std::vector<threevector>& plane_posn,
      std::vector<threevector>& plane_attitude);
   OBSFRUSTUM* instantiate_HAFB_video_OBSFRUSTUM(
      MODEL* MODEL_ptr,const std::vector<threevector>& aircraft_posn,
      double z_offset,int OSGsubPAT_number=0);

// Message handling member functions:

   virtual bool parse_next_message_in_queue(message& curr_message);

// Multi-UAV C2 member functions:

   bool broadcast_sensor_and_target_statevectors();
   void broadcast_sensor_statevectors();
   bool broadcast_add_track_to_GoogleEarth_channel(
      track* track_ptr,bool compute_posns_with_distinct_dirs_flag=true);
   bool broadcast_add_tracks_to_GoogleEarth_channel(
      const std::vector<track*> track_ptrs);
   bool broadcast_delete_tracks_to_GoogleEarth_channel(
      const std::vector<int> track_IDs);
   colorfunc::Color UAV_flight_path_color(int UAV_ID);
   void alter_UAV_Path_PolyLine();
   std::vector<int> purge_UAV_MODELS_and_tracks();

// RTPS ROI Polyhedra member functions 

   void destroy_ground_bbox();
   void set_ground_bbox(
      double lower_left_longitude,double lower_left_latitude,
      double upper_right_longitude,double upper_right_latitude);

// Human model member functions:

   MODEL* generate_man_MODEL(int& OSGsubPAT_number);

// Real-time MODEL updating member functions:

   void update_Aircraft_MODEL(
      threevector posn,double roll,double pitch,double yaw,
      MODEL* Aircraft_MODEL_ptr);
   void generate_Aircraft_MODEL_track_and_mover(MODEL* Aircraft_MODEL_ptr);

  protected:

   bool LiMIT_FOV_OBSFRUSTUM_flag;
   bool alter_UAV_path_flag;
   unsigned int n_future_repeats;
   double LiMIT_speed,aircraft_altitude;
   double OBSFRUSTUM_az_extent,OBSFRUSTUM_el_extent;
   double OBSFRUSTUM_roll,OBSFRUSTUM_pitch;
   EarthRegionsGroup* EarthRegionsGroup_ptr;
   ColorGeodeVisitor* ColorGeodeVisitor_ptr;
   Messenger* viewer_Messenger_ptr;
   PolyhedraGroup* ROI_PolyhedraGroup_ptr;
   PolyLinesGroup* Path_PolyLinesGroup_ptr;

   AircraftModelType aircraft_model_type;
   bool compute_skymap_flag;
   bool double_LiMIT_lobe_pattern_flag;
   bool generate_Predator_model_on_next_cycle_flag;

   std::string tomcat_subdir,webapps_outputs_subdir;

   ArrowsGroup* ArrowsGroup_ptr;
   bounding_box* ground_bbox_ptr;
   Operations* Operations_ptr;
   TilesGroup* TilesGroup_ptr;
   tracks_group* tracks_group_ptr;

   void add_flight_path_arrows(
      PolyLine* flight_PolyLine_ptr,double scale_factor=1);

  private:

   bool continuously_plan_UAV_paths_flag;
   bool fade_UAV_track_color_flag;
   bool compute_passed_ground_targets_flag,long_initial_UAV_track_flag;
   bool compute_zface_height_flag;
   bool instantiate_OBSFRUSTAGROUP_flag;
   bool instantiate_OBSFRUSTUMPickHandler_flag;
   bool loiter_at_track_end_flag;
   unsigned int model_counter;
   int ID_for_path_to_alter;
   double Predator_speed,aircraft_speed;
   double prev_UAV_path_planning_time;
   double flightpath_fraction_offset;
   std::string model_filename;
   std::vector<threevector> candidate_waypoint,waypoint,even_path_point;

   ModeController* ModeController_ptr;
   PolyLinePickHandler* Path_PolyLinePickHandler_ptr;

   movers_group* movers_group_ptr;
   WindowManager* WindowManager_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void initialize_Path_PolyLinesGroup();
   void initialize_Path_PolyLinePickHandler();
   void docopy(const MODELSGROUP& M);

   bool broadcast_sensor_and_target_statevectors(
      EarthRegion* curr_EarthRegion_ptr);
   PolyLine* generate_flight_PolyLine_among_ROIs(
      MODEL* UAV_MODEL_ptr,const std::vector<threevector>& ROI_posns,
      PolyLine*& Path_PolyLine_ptr);
   MODEL* generate_Predator_Model_for_flight_PolyLine(
      PolyLine* flight_PolyLine_ptr,bool broadcast_UAV_track_flag=true);
   MODEL* generate_aircraft_MODEL_for_flight_PolyLine(
      PolyLine* flight_PolyLine_ptr,bool broadcast_UAV_track_flag=true);

   track* generate_task_assignments(
      int UAV_ID,const std::vector<int>& ROI_IDs);
   double compute_altitude_dependent_alpha(
      double zmin,double zmax,double alpha_max);

   void update_display();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void MODELSGROUP::set_AircraftModelType(AircraftModelType model_type)
{
   aircraft_model_type=model_type;
}

inline MODELSGROUP::AircraftModelType MODELSGROUP::get_AircraftModelType() 
   const
{
   return aircraft_model_type;
}

inline void MODELSGROUP::set_generate_Predator_model_on_next_cycle_flag(
   bool flag)
{
   generate_Predator_model_on_next_cycle_flag=flag;
}

inline void MODELSGROUP::set_alter_UAV_path_flag(bool flag)
{
   alter_UAV_path_flag=flag;
}

inline void MODELSGROUP::set_continuously_plan_UAV_paths_flag(bool flag)
{
   continuously_plan_UAV_paths_flag=flag;
}

inline void MODELSGROUP::set_fade_UAV_track_color_flag(bool flag)
{
   fade_UAV_track_color_flag=flag;
}

inline void MODELSGROUP::set_compute_passed_ground_targets_flag(bool flag)
{
   compute_passed_ground_targets_flag=flag;
}

inline void  MODELSGROUP::set_long_initial_UAV_track_flag(bool flag)
{
   long_initial_UAV_track_flag=flag;
}

inline void MODELSGROUP::set_compute_zface_height_flag(bool flag)
{
   compute_zface_height_flag=flag;
}

inline void MODELSGROUP::set_instantiate_OBSFRUSTAGROUP_flag(bool flag)
{
   instantiate_OBSFRUSTAGROUP_flag=flag;
}

inline void MODELSGROUP::set_instantiate_OBSFRUSTUMPickHandler_flag(bool flag)
{
   instantiate_OBSFRUSTUMPickHandler_flag=flag;
}

inline void MODELSGROUP::set_double_LiMIT_lobe_pattern_flag(bool flag)
{
   double_LiMIT_lobe_pattern_flag=flag;
}

inline void MODELSGROUP::set_loiter_at_track_end_flag(bool flag)
{
   loiter_at_track_end_flag=flag;
}

inline void MODELSGROUP::set_aircraft_altitude(double alt)
{
   aircraft_altitude=alt;
   initialize_Path_PolyLinesGroup();
}

inline double MODELSGROUP::get_aircraft_altitude() const
{
   return aircraft_altitude;
}

inline void MODELSGROUP::set_LiMIT_speed(double speed)
{
   LiMIT_speed=speed;
}

inline double MODELSGROUP::get_LiMIT_speed() const
{
   return LiMIT_speed;
}

// --------------------------------------------------------------------------

inline void MODELSGROUP::set_ground_bbox_ptr(bounding_box* ground_bbox_ptr)
{
   this->ground_bbox_ptr=ground_bbox_ptr;
}

inline bounding_box* MODELSGROUP::get_ground_bbox_ptr()
{
   return ground_bbox_ptr;
}

// --------------------------------------------------------------------------
inline void MODELSGROUP::set_OBSFRUSTUM_az_extent(double delta_az)
{
   OBSFRUSTUM_az_extent=delta_az;
}

inline void MODELSGROUP::set_OBSFRUSTUM_el_extent(double delta_el)
{
   OBSFRUSTUM_el_extent=delta_el;
}

inline void MODELSGROUP::set_OBSFRUSTUM_roll(double roll)
{
   OBSFRUSTUM_roll=roll;
}

inline void MODELSGROUP::set_OBSFRUSTUM_pitch(double pitch)
{
   OBSFRUSTUM_pitch=pitch;
}

// --------------------------------------------------------------------------
inline void MODELSGROUP::set_ID_for_path_to_alter(int ID)
{
   ID_for_path_to_alter=ID;
}

inline void MODELSGROUP::set_n_future_repeats(unsigned int repeats)
{
   n_future_repeats=repeats;
}

inline void MODELSGROUP::set_model_filename(std::string filename)
{
   model_filename=filename;
}

inline std::string MODELSGROUP::get_model_filename() const
{
   return model_filename;
}

inline std::string MODELSGROUP::get_tomcat_subdir() const
{
   return tomcat_subdir;
}

// --------------------------------------------------------------------------
inline MODEL* MODELSGROUP::get_MODEL_ptr(int n) const
{
   return dynamic_cast<MODEL*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline MODEL* MODELSGROUP::get_ID_labeled_MODEL_ptr(int ID) const
{
   return dynamic_cast<MODEL*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline void MODELSGROUP::set_ColorGeodeVisitor_ptr(ColorGeodeVisitor* CGV_ptr)
{
   ColorGeodeVisitor_ptr=CGV_ptr;
}

// --------------------------------------------------------------------------
inline tracks_group* MODELSGROUP::get_tracks_group_ptr()
{
   return tracks_group_ptr;
}

inline const tracks_group* MODELSGROUP::get_tracks_group_ptr() const
{
   return tracks_group_ptr;
}

// --------------------------------------------------------------------------
inline void MODELSGROUP::set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;
}

inline EarthRegionsGroup* MODELSGROUP::get_EarthRegionsGroup_ptr()
{
   return EarthRegionsGroup_ptr;
}

inline const EarthRegionsGroup* MODELSGROUP::get_EarthRegionsGroup_ptr() const
{
   return EarthRegionsGroup_ptr;
}

inline void MODELSGROUP::set_TilesGroup_ptr(TilesGroup* TG_ptr)
{
   TilesGroup_ptr=TG_ptr;
}

// --------------------------------------------------------------------------
inline movers_group* MODELSGROUP::get_movers_group_ptr()
{
   return movers_group_ptr;
}

inline const movers_group* MODELSGROUP::get_movers_group_ptr() const
{
   return movers_group_ptr;
}

// --------------------------------------------------------------------------
inline std::vector<threevector>& MODELSGROUP::get_even_path_point()
{
   return even_path_point;
}

inline const std::vector<threevector>& MODELSGROUP::get_even_path_point() const
{
   return even_path_point;
}

// --------------------------------------------------------------------------
inline void MODELSGROUP::set_flightpath_fraction_offset(double frac_offset)
{
   flightpath_fraction_offset=frac_offset;
}

inline PolyLinesGroup* MODELSGROUP::get_Path_PolyLinesGroup_ptr()
{
   return Path_PolyLinesGroup_ptr;
}

inline const PolyLinesGroup* MODELSGROUP::get_Path_PolyLinesGroup_ptr() const
{
   return Path_PolyLinesGroup_ptr;
}

inline PolyLinePickHandler* MODELSGROUP::get_Path_PolyLinePickHandler_ptr()
{
   return Path_PolyLinePickHandler_ptr;
}

inline const PolyLinePickHandler* MODELSGROUP::get_Path_PolyLinePickHandler_ptr() 
   const
{
   return Path_PolyLinePickHandler_ptr;
}

// --------------------------------------------------------------------------
inline void MODELSGROUP::set_ModeController_ptr(ModeController* MC_ptr)
{
   ModeController_ptr=MC_ptr;
}

inline void MODELSGROUP::set_WindowManager_ptr(WindowManager* WCC_ptr)
{
   WindowManager_ptr=WCC_ptr;
}

inline void MODELSGROUP::set_ROI_PolyhedraGroup_ptr(PolyhedraGroup* PHG_ptr)
{
   ROI_PolyhedraGroup_ptr=PHG_ptr;
}

inline void MODELSGROUP::set_ArrowsGroup_ptr(ArrowsGroup* AG_ptr)
{
   ArrowsGroup_ptr=AG_ptr;
}

inline ArrowsGroup* MODELSGROUP::get_ArrowsGroup_ptr()
{
   return ArrowsGroup_ptr;
}

inline const ArrowsGroup* MODELSGROUP::get_ArrowsGroup_ptr() const
{
   return ArrowsGroup_ptr;
}

// --------------------------------------------------------------------------
inline Messenger* MODELSGROUP::get_viewer_Messenger_ptr()
{
   return viewer_Messenger_ptr;
}

inline const Messenger* MODELSGROUP::get_viewer_Messenger_ptr() const
{
   return viewer_Messenger_ptr;
}

#endif // MODELSGROUP.h

