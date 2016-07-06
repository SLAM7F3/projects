// ==========================================================================
// Header file for stand-alone mover methods
// ==========================================================================
// Last updated on 9/7/10; 9/11/10; 9/18/10
// ==========================================================================

#ifndef MOVERFUNCS_H
#define MOVERFUNCS_H

#include <set>
#include <string>
#include <vector>
#include "astro_geo/geopoint.h"
#include "math/threevector.h"

class Clock;
class gis_database;
class polyline;
class track;
class tracks_group;

namespace mover_func
{

   double compute_relative_ROI_size(polyline* polyline_ptr);
   std::vector<threevector> extrude_bottom_ROI_face(
      polyline* polyline_ptr,double skeleton_height);

// Flat retrieval of table metadata from TOC database member
// functions:

   void retrieve_fieldtest_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& fieldtest_label,
      std::vector<int>& fieldtest_ID,bool weekday_mon_day_flag=true);
   std::string get_fieldtest_date(int fieldtest_ID,
	gis_database* gis_database_ptr);
   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,std::vector<int>& mission_ID);
   void retrieve_platform_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& platform_label,std::vector<int>& platform_ID);
   void retrieve_sensor_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& sensor_label,std::vector<int>& sensor_ID);

// Correlated retrieval of table metadata from TOC database member
// functions:

   void retrieve_fieldtest_mission_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& fieldtest_label,
      std::vector<int>& fieldtest_ID,
      std::vector<std::string>& mission_label,
      std::vector<int>& mission_ID);
   void retrieve_fieldtest_mission_platform_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& fieldtest_label,
      std::vector<int>& fieldtest_ID,
      std::vector<std::string>& mission_label,
      std::vector<int>& mission_ID,
      std::vector<std::string>& platform_label,
      std::vector<int>& platform_ID);

   void retrieve_correlated_fieldtest_mission_platform_sensor_metadata(
      gis_database* gis_database_ptr,std::string database_table_name,
      std::vector<std::string>& fieldtest_label,std::vector<int>& fieldtest_ID,
      std::vector<std::string>& mission_label,std::vector<int>& mission_ID,
      std::vector<std::string>& platform_label,std::vector<int>& platform_ID,
      std::vector<std::string>& sensor_label,std::vector<int>& sensor_ID);

   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& mission_label,std::vector<int>& mission_ID,
      std::vector<std::string>& fieldtest_label,std::vector<int>& fieldtest_ID,
      std::vector<std::string>& platform_label,std::vector<int>& platform_ID,
      bool just_mission_ID_flag);

// GPS track methods

   std::string generate_insert_track_point_SQL_command(
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      double secs_since_epoch,
      int fix_quality,int n_satellites,double horizontal_dilution,
      double longitude,double latitude,double altitude,
      double roll,double pitch,double yaw);

   void retrieve_track_points_metadata_from_database(
      bool daylight_savings_flag,gis_database* gis_database_ptr,
      int mission_ID,int sensor_ID,
      std::vector<int>& trackpoint_ID,std::vector<double>& elapsed_secs,
      std::vector<int>& fix_quality,std::vector<int>& n_satellites, 
      std::vector<double>& horiz_dilution,
      std::vector<double>& longitude,std::vector<double>& latitude,
      std::vector<double>& altitude,std::vector<double>& roll,
      std::vector<double>& pitch,std::vector<double>& yaw);
   void retrieve_track_points_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      std::vector<int>& trackpoint_ID,std::vector<std::string>& time_stamp,
      std::vector<int>& fix_quality,std::vector<int>& n_satellites,
      std::vector<double>& horiz_dilution,
      std::vector<double>& zposn,std::vector<double>& roll,
      std::vector<double>& pitch,std::vector<double>& yaw,
      std::vector<double>& longitude,std::vector<double>& latitude);

   tracks_group* retrieve_all_tracks_in_TOC_database(
      gis_database* gis_database_ptr,int selected_fieldtest_ID);

  std::vector<double> UTC_timestamp_to_secs_since_epoch(
      bool daylight_savings_flag,const std::vector<std::string>& time_stamp,
      const std::vector<double>& longitude,
      const std::vector<double>& latitude,
      const std::vector<double>& altitude);

// Peter's GPS, MIDG and Quad GPS log file parsing:

   void alpha_filter_raw_GPS_data(
      int curr_index,int starting_index,
      double& prev_filtered_longitude,double& prev_filtered_latitude,
      double raw_longitude,double raw_latitude,
      double& filtered_longitude,double& filtered_latitude);

   void parse_GPS_logfile(std::string logfilename,Clock& clock,
		          track* gps_track_ptr);
   bool insert_track_points(
      gis_database* gis_database_ptr,track* gps_track_ptr,
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID);

   void parse_insparse_output(
      std::string insparse_output_filename,Clock& clock,
      track* MIDG_track_ptr);

   void parse_QuadGPS_logfile(std::string quad_log_filename,
      Clock& clock,track* Quad_track_ptr);
   void parse_DroidGPS_logfile(std::string logfilename,Clock& clock,
      track* gps_track_ptr);
   void parse_GarminGPS_kmlfile(std::string kmlfilename,Clock& clock,
      track* gps_track_ptr);
   void generate_GPScamera_track(
      std::vector<double>& secs_elapsed,std::vector<geopoint>& geolocations,
      Clock& clock,std::string output_subdir,track* gps_track_ptr);

// ==========================================================================
// Inlined methods:
// ==========================================================================
   
} // mover_func namespace

#endif  // mover_funcs.h
