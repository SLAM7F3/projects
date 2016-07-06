// ==========================================================================
// Header file for databasefunc namespace
// ==========================================================================
// Last modified on 10/27/11; 10/28/11; 10/31/11
// ==========================================================================

#ifndef DATABASEFUNCS_H
#define DATABASEFUNCS_H

#include <string>
#include <vector>
#include "math/threevector.h"

class gis_database;

namespace databasefunc
{

// Manipulation methods for world map annotations table in TOC
// database 

   bool insert_world_annotation(
      gis_database* gis_database_ptr,int fieldtest_ID,double secs_since_epoch,
      std::string username,std::string label,
      std::string description,std::string color,int importance,
      double longitude,double latitude,double altitude);
   std::string generate_insert_world_annotation_SQL_command(
      int fieldtest_ID,double secs_since_epoch,
      std::string username,std::string label,
      std::string description,std::string color,int importance,
      double longitude,double latitude,double altitude);
   int get_world_annotation_ID(
      gis_database* gis_database_ptr,
      std::string username,std::string label,std::string description);
   void get_all_world_map_annotations(
      gis_database* gis_database_ptr,int selected_fieldtest_ID,
      std::vector<int>& annotation_IDs,
      std::vector<std::string>& creation_times,
      std::vector<std::string>& event_times,
      std::vector<std::string>& usernames,
      std::vector<std::string>& labels,
      std::vector<std::string>& descriptions,
      std::vector<std::string>& colors,
      std::vector<int>& importances,
      std::vector<threevector>& llas);

   bool update_world_annotation(
      gis_database* gis_database_ptr,int annotation_ID,double secs_since_epoch,
      std::string username,std::string label,std::string description,
      std::string color,int importance,double longitude,double latitude,
      double altitude);
   std::string generate_update_world_annotation_SQL_command(
      int annotation_ID,double secs_since_epoch,
      std::string username,std::string label,std::string description,
      std::string color,int importance,double longitude,double latitude,
      double altitude);

   bool delete_world_annotation(gis_database* gis_database_ptr,
   	int annotation_ID);

   void retrieve_world_annotations_from_database(
      gis_database* gis_database_ptr,int fieldtest_id,
      std::vector<int>& annotation_ID,std::vector<std::string>& time_stamp,
      std::vector<std::string>& username,std::vector<std::string>& label,
      std::vector<std::string>& description,std::vector<std::string>& color,
      std::vector<int>& importance,std::vector<double>& zposn,
      std::vector<double>& longitude,std::vector<double>& latitude);

// TOC database manipulation methods

   bool insert_fieldtest(
      gis_database* gis_database_ptr,std::string fieldtest_date,
      std::string brief_label,std::string description);
   std::string generate_insert_fieldtest_SQL_command(
      std::string fieldtest_date,std::string brief_label,
      std::string description);
   double get_fieldtest_time(
      gis_database* gis_databse_ptr,int fieldtest_ID);

   int insert_mission(
      gis_database* gis_database_ptr,
      std::string start_mission_time,std::string stop_mission_time,
      int fieldtest_ID,std::string platform_label,int platform_ID,
      int SDcard_ID,std::string pilot_name,std::string copilot_name,
      std::string courier_name);
   std::string generate_insert_mission_SQL_command(
      std::string start_mission_time,std::string stop_mission_time,
      int fieldtest_ID,std::string platform_label,int platform_ID,
      int SDcard_ID,std::string pilot_name,std::string copilot_name,
      std::string courier_name);
   bool insert_platform(
      gis_database* gis_database_ptr,std::string description);
   std::string generate_insert_platform_SQL_command(std::string description);
   bool insert_sensor(
      gis_database* gis_database_ptr,std::string description);
   std::string generate_insert_sensor_SQL_command(std::string description);

// FLIR database insertion methods

   bool insert_aircraft_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int frame_ID,
      double epoch_time,std::string frame_prefix,
      double longitude,double latitude,double altitude,
      double yaw,double pitch,double roll);
   std::string generate_insert_aircraft_metadata_SQL_command(
      int campaign_ID,int mission_ID,int frame_ID,
      double epoch_time,std::string frame_prefix,
      double longitude,double latitude,double altitude,
      double yaw,double pitch,double roll);
   bool insert_camera_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int frame_ID,
      double horiz_FOV,double vert_FOV,
      double camera_az,double camera_el,double camera_roll);
   std::string generate_insert_camera_metadata_SQL_command(
      int campaign_ID,int mission_ID,int frame_ID,
      double horiz_FOV,double vert_FOV,
      double camera_az,double camera_el,double camera_roll);

// FLIR database retrieval methods

   void retrieve_campaign_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,
      std::string& campaign_name,int& UTM_zonenumber,
      bool& northern_hemisphere_flag,
      std::string& DTED_map_name,double& map_min_lon,double& map_max_lon,
      double& map_min_lat,double& map_max_lat);

   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int& flight_number,std::string& start_date);
   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,double epoch_time,
      int& campaign_ID,int& mission_ID);
   bool retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,
      double epoch_start_time,double epoch_stop_time,
      int& campaign_ID,int& mission_ID);
   void retrieve_mission_start_stop_times_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double& start_time,double& stop_time);

   void retrieve_aircraft_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& frame_ID,
      std::vector<double>& epoch_time,
      std::vector<std::string>& frame_prefix,
      std::vector<double>& longitude,
      std::vector<double>& latitude,
      std::vector<double>& altitude,
      std::vector<double>& roll,
      std::vector<double>& pitch,
      std::vector<double>& yaw);
   void retrieve_aircraft_metadata_from_database(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,double start_time,double stop_time,
      std::vector<int>& frame_ID,
      std::vector<double>& epoch_time,
      std::vector<std::string>& frame_prefix,
      std::vector<double>& longitude,
      std::vector<double>& latitude,
      std::vector<double>& altitude,
      std::vector<double>& roll,
      std::vector<double>& pitch,
      std::vector<double>& yaw);
   void retrieve_aircraft_metadata_from_database(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,double start_time,double stop_time,
      std::vector<double>& epoch_time,std::vector<std::string>& frame_prefix);
   void retrieve_extremal_frame_IDs_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_time,double stop_time,
      int& first_frame_ID,int& last_frame_ID);

   void retrieve_camera_metadata_from_database(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,
      std::vector<double>& horiz_FOV,std::vector<double>& vert_FOV,
      std::vector<double>& camera_az,std::vector<double>& camera_el,
      std::vector<double>& camera_roll);
   void retrieve_camera_metadata_from_database(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,double start_time,double stop_time,
      std::vector<double>& horiz_FOV,std::vector<double>& vert_FOV,
      std::vector<double>& camera_az,std::vector<double>& camera_el,
      std::vector<double>& camera_roll);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // databasefunc namespace

#endif // databasefuncs.h

