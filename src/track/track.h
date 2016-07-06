// ==========================================================================
// Header file for track class 
// ==========================================================================
// Last modified on 10/8/11; 10/9/11; 1/30/12; 4/5/14
// ==========================================================================

#ifndef TRACK_H
#define TRACK_H

class gis_database;
class Messenger;
class polyline;
class statevector;

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "geometry/linesegment.h"
#include "math/ltonevector.h"
#include "math/rpy.h"

class track
{

  public:

// Initialization, constructor and destructor methods:

   track(int id);
   track(const track& t);
   virtual ~track();
   track& operator= (const track& T);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const track& T);
 
// Set & get member functions:

   void set_broadcast_contents_flag(bool flag);
   bool get_broadcast_contents_flag() const;
   void set_spatially_fixed_flag(bool flag);
   bool get_spatially_fixed_flag() const;
   int get_ID() const;

// As of 1/22/09, we strongly suspect that Cylinder_ID is a redundant
// copy of label_ID.  It appears to be only used within the
// VideoServer class.

   void set_label_ID(int id);
   int get_label_ID() const;
   void set_Cylinder_ID(int id);
   int get_Cylinder_ID() const;
   void set_PolyLine_ID(int id);
   int get_PolyLine_ID() const;
   unsigned int size() const;
   void set_entityID(std::string id);
   std::string get_entityID() const;
   void set_entityType(std::string id);
   std::string get_entityType() const;
   void set_label(std::string l);
   std::string get_label() const;
   void set_description(std::string description);
   std::string get_description() const;

   void set_RGB_color(colorfunc::RGB curr_RGB);
   colorfunc::RGB get_RGB_color() const;

   bool pushback_time(double curr_t);
   void set_time(int i,double curr_t);
   double get_time(int i) const;
   void set_XYZ_coords(double curr_t,const threevector& r,
      bool temporally_sort_flag=false);
   bool get_XYZ_coords(double curr_t,threevector& r) const;

   void set_posn_velocity(
      double curr_t,const threevector& r,const threevector& v,
      bool temporally_sort_flag=false);
   void set_posn_velocity_GPSquality(
      double curr_t,const threevector& r,const threevector& v,
      const threevector& q);
   void set_posn_velocity_GPSquality_rpy(
      double curr_t,const threevector& r,const threevector& v,
      const threevector& q,const rpy& r_p_y);
   void set_posn_rpy(double curr_t,const threevector& r,const rpy& r_p_y);
   void set_posn_rpy_sensor_aer_fov(
      double curr_t,const threevector& r,const rpy& r_p_y,
      const threevector& sensor_aer,const twovector& sensor_hfov_vfov);
   void temporally_sort_posns_and_velocities();

   bool get_posn_velocity(double curr_t,threevector& r,threevector& v) const;

   void set_bbox(double curr_t,const bounding_box& curr_bbox);
   bool get_bbox(double curr_t,bounding_box& curr_bbox) const;
   void purge_all_values();

// Track coloring member functions:

   void push_back_velocity_color(const osg::Vec4& curr_color);
   std::vector<osg::Vec4>& get_velocity_colors();
   const std::vector<osg::Vec4>& get_velocity_colors() const;

   void push_back_segment_color(const osg::Vec4& curr_color);
   std::vector<osg::Vec4>& get_segment_colors();
   const std::vector<osg::Vec4>& get_segment_colors() const;

   void compute_track_color_fading();

// Track property member functions:

   double get_earliest_time() const;
   double get_latest_time() const;
   threevector get_earliest_posn() const;
   threevector get_latest_posn() const;

   const std::vector<double>& get_cumulative_lengths() const;
   std::vector<threevector>& get_posns();
   const std::vector<threevector>& get_posns() const;
   std::vector<threevector>& get_distinct_posns();
   std::vector<threevector>& get_velocities();
   const std::vector<threevector>& get_velocities() const;
   std::vector<threevector>* get_velocity_ptr();
   std::vector<threevector>& get_posns_with_distinct_velocities();
   std::vector<rpy>& get_rpys();
   const std::vector<rpy>& get_rpys() const;

   bool get_track_closed() const;
   bool get_latest_heading(threevector& u_hat);
   double max_temporal_gap();
   double temporal_duration();
   void close_track_test(double curr_t,double tau_coast);

   double total_length();
   double avg_speed();
   double median_altitude() const;
   void compute_distinct_posns();
   std::vector<threevector> compute_posns_with_distinct_directions();
   void compute_posns_with_distinct_velocities();
   void compute_average_velocities();

// Track manipulation member functions:

   void rescale_time_values(double scale_factor);
   void offset_time_values(double delta_t);

// Interpolation member functions:

   int get_track_point_index(double curr_t) const;
   bool get_interpolated_posn(double curr_t,threevector& interpolated_posn)
      const;
   bool get_interpolated_statevector(
      double curr_t,statevector& interpolated_statevector) const;
   bool get_interpolated_posn_rpy_sensor_aer_FOV(
      double curr_t,threevector& interpolated_posn,
      rpy& interpolated_rpy,threevector& sensor_aer,twovector& sensor_FOV) 
      const;

// Track segment member functions:

   std::vector<linesegment>* get_segment_ptr();
   const std::vector<linesegment>* get_segment_ptr() const;
   void compute_segments(bool twoD_flag=false);
   unsigned int segment_number(double frac);
   void segment_point(double frac,threevector& curr_point);
   void interpolated_posns(double ds,std::vector<threevector>& interp_posn);
   double closest_approach_time(const threevector& POI);

// Track intersection member functions:

   bool intercepts_bbox(const bounding_box& bbox);
   bool intercepts_bbox_polyline(const polyline* polyline_ptr);

// ActiveMQ broadcast member functions:

   bool broadcast_statevector(
      double secs_elapsed_since_epoch,Messenger* Messenger_ptr,
      colorfunc::Color curr_color=colorfunc::white,std::string ID_label="")
      const;
   bool broadcast_statevector(
      double secs_elapsed_since_epoch,
      const threevector* grid_origin_ptr,Messenger* Messenger_ptr,
      colorfunc::Color curr_color=colorfunc::white,std::string ID_label="")
      const;
   void broadcast_statevector(
      const statevector& curr_statevector,const threevector* grid_origin_ptr,
      Messenger* Messenger_ptr,colorfunc::Color curr_color=colorfunc::white,
      std::string ID_label="") const;

// SQL input/output member functions:

   void write_SQL_insert_track_commands(
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      std::string SQL_track_points_filename);
   std::vector<std::string> generate_SQL_insert_track_commands(
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID);
   
  private: 

   bool track_closed_flag,broadcast_contents_flag,spatially_fixed_flag;
   int ID,label_ID,Cylinder_ID,PolyLine_ID;
   double length;
   colorfunc::RGB RGB_color;
   std::string entityID,entityType,label,description;

   typedef std::map<double,int,ltonevector> TIME_MAP; 
   // first member = time, second member = index

   std::vector<double> t;
   TIME_MAP* time_map_ptr;
   std::vector<double> cumulative_lengths;
   std::vector<threevector> posn,distinct_posn,posns_with_distinct_velocities;
   std::vector<threevector> velocity;
   std::vector<rpy> roll_pitch_yaw;
   std::vector<threevector> sensor_az_el_roll;
   std::vector<twovector> sensor_FOV;
   std::vector<bounding_box> bbox;
   std::vector<linesegment> segment;

// GPS track quality values:

   std::vector<int> fix_quality,n_satellites;
   std::vector<double> horiz_dilution;

   std::vector<osg::Vec4> velocity_colors,segment_colors;

   void allocate_member_objects(); 
   void initialize_member_objects();
   void docopy(const track& T);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void track::set_broadcast_contents_flag(bool flag)
{
   broadcast_contents_flag=flag;
}

inline bool track::get_broadcast_contents_flag() const
{
   return broadcast_contents_flag;
}

inline void track::set_spatially_fixed_flag(bool flag)
{
   spatially_fixed_flag=flag;
}

inline bool track::get_spatially_fixed_flag() const
{
   return spatially_fixed_flag;
}

inline int track::get_ID() const
{
   return ID;
}

inline void track::set_label_ID(int id)
{
   label_ID=id;
}

inline int track::get_label_ID() const
{
   return label_ID;
}

inline void track::set_Cylinder_ID(int id)
{
   Cylinder_ID=id;
}

inline int track::get_Cylinder_ID() const
{
   return Cylinder_ID;
}

inline void track::set_PolyLine_ID(int id)
{
   PolyLine_ID=id;
}

inline int track::get_PolyLine_ID() const
{
   return PolyLine_ID;
}

inline unsigned int track::size() const
{
   return time_map_ptr->size();
}

inline void track::set_entityID(std::string id)
{
   entityID=id;
}

inline std::string track::get_entityID() const
{
   return entityID;
}

inline void track::set_entityType(std::string type)
{
   entityType=type;
}

inline std::string track::get_entityType() const
{
   return entityType;
}

inline void track::set_label(std::string l)
{
   label=l;
}

inline std::string track::get_label() const
{
   return label;
}

inline void track::set_description(std::string description)
{
   this->description=description;
}

inline std::string track::get_description() const
{
   return description;
}

inline void track::set_RGB_color(colorfunc::RGB curr_RGB)
{
   RGB_color=curr_RGB;
}

inline colorfunc::RGB track::get_RGB_color() const
{
   return RGB_color;
}

inline bool track::get_track_closed() const
{
   return track_closed_flag;
}

inline std::vector<linesegment>* track::get_segment_ptr() 
{
   return &segment;
}

inline const std::vector<linesegment>* track::get_segment_ptr() const
{
   return &segment;
}

inline void track::push_back_velocity_color(const osg::Vec4& curr_color)
{
   velocity_colors.push_back(curr_color);
}

inline std::vector<osg::Vec4>& track::get_velocity_colors() 
{
   return velocity_colors;
}

inline const std::vector<osg::Vec4>& track::get_velocity_colors() const
{
   return velocity_colors;
}

inline void track::push_back_segment_color(const osg::Vec4& curr_color)
{
   segment_colors.push_back(curr_color);
}

inline std::vector<osg::Vec4>& track::get_segment_colors() 
{
   return segment_colors;
}

inline const std::vector<osg::Vec4>& track::get_segment_colors() const
{
   return segment_colors;
}


#endif  // math/track.h






