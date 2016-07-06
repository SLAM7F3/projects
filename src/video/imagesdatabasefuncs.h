// ==========================================================================
// Header file for imagesdatabasefunc namespace
// ==========================================================================
// Last modified on 8/13/13; 8/15/13; 10/29/13; 10/31/13
// ==========================================================================

#ifndef IMAGESDATABASEFUNCS_H
#define IMAGESDATABASEFUNCS_H

#include <string>
#include <vector>
#include "graphs/graphdbfuncs.h"
#include "math/threevector.h"
#include "datastructures/Triple.h"

class gis_database;
class node;

namespace imagesdatabasefunc
{
   typedef std::map<int,std::vector<std::string> > IMAGE_IDS_METADATA_MAP;
   typedef std::pair<std::string,std::string> STRING_PAIR;

   typedef std::map<int,std::vector<STRING_PAIR> > ATTRIBUTES_METADATA_MAP;
	// indep integer var = node ID
	// dependent var = STL vector of STRING_PAIRs

// Images database insertion methods

   bool insert_image_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int importance,
      std::string URL,int npx,int npy,
      std::string thumbnail_URL,int thumbnail_npx,int thumbnail_npy);
   std::string generate_insert_image_metadata_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int importance,
      std::string URL,int npx,int npy,
      std::string thumbnail_URL,int thumbnail_npx,int thumbnail_npy);
   std::string generate_insert_image_metadata_SQL_command(
      int campaign_ID,int mission_ID, int importance, 
      std::string URL,int npx,int npy,
      std::string thumbnail_URL,int thumbnail_npx,int thumbnail_npy);

   bool update_image_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,
      std::string UTC,double epoch);
   std::string generate_update_image_metadata_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,
      std::string UTC,double epoch);
   std::string generate_update_image_metadata_SQL_command_serialID(
      int campaign_ID,int mission_ID,int image_ID,
      std::string UTC,double epoch);
   bool update_image_sensor_ID(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int sensor_ID);

// Images database retrieval methods:

   bool images_in_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);
   bool image_in_database(gis_database* gis_database_ptr,int datum_ID);
   int get_image_ID(gis_database* gis_database_ptr,int datum_ID);

   int get_image_ID(
      gis_database* gis_database_ptr,int Hierarchy_ID,std::string URL);
   int get_datum_ID(
      gis_database* gis_database_ptr,int Hierarchy_ID,std::string URL);
   void get_image_metadata_given_URL(
      gis_database* gis_database_ptr,std::string URL,
      int& campaign_ID,int& mission_ID,int& image_ID,int& datum_ID);

   int get_image_ID(
      gis_database* gis_database_ptr,int campaign_ID,
      int mission_ID,std::string URL);
   int get_image_serial_ID(
      gis_database* gis_database_ptr,int campaign_ID,
      int mission_ID, std::string URL);
   bool get_campaign_mission_image_IDs(
      gis_database* gis_database_ptr,int datum_ID,
      int& campaign_ID,int& mission_ID,int& image_ID);
   bool get_campaign_mission_IDs(
      gis_database* gis_database_ptr,int Hierarchy_ID,
      int& campaign_ID,int& mission_ID);

   bool get_zeroth_image_npx_npy(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int& npx,int& npy);

   std::string get_image_URL(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID);
   std::string get_image_URL(
      gis_database* gis_database_ptr,int datum_ID,bool thumbnail_flag);
   std::string get_image_URL(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID);

   bool get_image_URLs(
      int campaign_ID,int mission_ID,gis_database* gis_database_ptr,
      std::vector<std::string>& URLs);
   bool get_image_URLs(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      std::vector<std::string>& URLs);
   bool get_image_URLs(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      std::vector<int>& campaign_IDs,std::vector<int>& mission_IDs,      
      std::vector<int>& image_IDs,std::vector<int>& datum_IDs,
      std::vector<std::string>& URLs);

   int get_datum_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID);
   bool retrieve_image_ID_URL_given_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int& image_ID,std::string& image_URL,std::string& thumbnail_URL);
   int get_node_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,int graph_ID);

   bool retrieve_particular_image_metadata(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int& campaign_ID,int& mission_ID,int& image_ID,double& epoch);
   bool retrieve_particular_image_metadata_from_database(
      gis_database* gis_database_ptr,int datum_ID,int& image_ID,
      int& importance,std::string& image_timestamp,double& image_epoch,
      std::string& image_URL,int& npx,int& npy,
      std::string& thumbnail_URL,int& thumbnail_npx,int& thumbnail_npy);
   std::string generate_retrieve_particular_image_SQL_command(int datum_ID);
   bool retrieve_particular_image_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::string image_URL,int& datum_ID,int& image_ID,
      int& npx,int& npy);

   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& image_IDs,std::vector<int>& datum_IDs,
      std::vector<std::string>& URLs);
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int HierarchyID,int GraphID,
      std::vector<int>& datum_IDs,std::vector<int>& image_IDs,
      std::vector<double>& epoch_times,
      std::vector<std::string>& thumbnail_URLs);
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int HierarchyID,int GraphID,
      std::vector<int>& node_IDs,std::vector<int>& image_IDs,
      std::vector<double>& epoch_times);
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int HierarchyID,int GraphID,
      std::vector<int>& node_IDs,
      std::vector<double>& epoch_times,
      std::vector<std::string>& image_URLs);

// Temporal neighbor retrieval methods

   int retrieve_sensor_metadata_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int node_ID);
   int retrieve_beginning_temporal_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      double& new_epoch);
   int retrieve_prev_temporal_neighbor_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int sensor_ID,double& new_epoch);
   int retrieve_next_temporal_neighbor_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int sensor_ID,double& new_epoch);
   int retrieve_ending_temporal_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      double& new_epoch);

   bool retrieve_particular_image_epoch(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      double& epoch);

// SIFT matching database insertion methods

   std::string generate_insert_sift_match_SQL_command(
      int campaign_ID1,int mission_ID1,int image_ID1,int feature_ID1,
      int campaign_ID2,int missoin_ID2,int image_ID2,int feature_ID2);

// Image graph database querying methods

   std::string write_graph_json_string(
      gis_database* gis_database_ptr,int hierarchy_ID,graph* graph_ptr,
      bool get_nodes_flag,bool get_edges_flag,bool get_annotations_flag,
      const std::vector<int>& incident_node_IDs);
   std::string write_nodes_json_string(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID);
   void add_node_attributes_to_json_string(
      int curr_node_ID,std::string& json_string,
      ATTRIBUTES_METADATA_MAP* image_annotations_map_ptr,
      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr,
      ATTRIBUTES_METADATA_MAP* color_histograms_map_ptr,
      ATTRIBUTES_METADATA_MAP* human_faces_map_ptr,
      ATTRIBUTES_METADATA_MAP* video_keyframes_map_ptr);
   bool retrieve_nodes_metadata(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      std::vector<int>& node_ID,std::vector<double>& epoch,
      std::vector<std::string>& URL,std::vector<int>& npx,
      std::vector<int>& npy,std::vector<std::string>& thumbnail_URL,
      std::vector<int>& thumbnail_npx,
      std::vector<int>& thumbnail_npy,std::vector<int>& parent_node_ID,
      std::vector<double>& gx,std::vector<double>& gy,
      std::vector<double>& gx2,std::vector<double>& gy2,
      std::vector<double>& relative_size,std::vector<std::string>& color,
      std::vector<std::string>& label);

// Image time querying methods

   int retrieve_campaign_UTM_zonenumber(
      gis_database* gis_database_ptr,int campaign_ID);
   double retrieve_median_image_time(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);
   double retrieve_image_time(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID);
   void retrieve_extremal_image_times(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double& starting_epoch,double& stopping_epoch);
   void retrieve_image_times(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<twovector>& epoch_IDs);
   bool retrieve_images_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<int>& datum_IDs,std::vector<int>& image_IDs,
      std::vector<double>& epoch_times,
      std::vector<std::string>& thumbnail_URLs);

   bool retrieve_image_minutes_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<double>& distinct_epoch_minutes);
   bool retrieve_image_hours_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<double>& distinct_epoch_hours);
   bool retrieve_image_days_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<double>& distinct_epoch_days);
   bool retrieve_image_months_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<double>& distinct_epoch_months);
   bool retrieve_image_years_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<double>& distinct_epoch_years);

   bool retrieve_closest_time_image(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double input_epoch_time,int& datum_ID,int& image_ID,
      double& epoch,std::string& thumbnail_URL);

// Image attributes methods

   bool insert_image_attribute(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      std::string key,std::string value);
   std::string generate_insert_image_attribute_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      std::string key,std::string value);
   bool update_image_attribute(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      std::string key,std::string value);
   bool retrieve_image_attributes_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::string key,std::vector<int>& image_IDs,std::vector<int>& datum_IDs,
      std::vector<std::string>& values);
   std::string generate_retrieve_image_attributes_SQL_command(
      int campaign_ID,int mission_ID,std::string key);
   bool retrieve_particular_image_attributes_from_database(
      gis_database* gis_database_ptr,int datum_ID,
      std::vector<std::string>& keys,std::vector<std::string>& values);
   bool retrieve_image_attribute_keys_values_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<std::string>& keys,
      std::vector<std::vector<std::string> >& key_values);

   ATTRIBUTES_METADATA_MAP* retrieve_all_attributes(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);

// Image attributes metadata methods

   bool image_attribute_key_exists(
      gis_database* gis_database_ptr,std::string attribute_key);
   bool image_attribute_value_exists(
      gis_database* gis_database_ptr,std::string attribute_value);

// Sensor metadata methods

   bool insert_sensor_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int sensor_ID,
      int metadata_ID,int image_ID,int datum_ID,int status,
      double X,double Y,double Z,double az,double el,double roll,
      double FOV_U,double FOV_V,double f,double U0,double V0);
   bool retrieve_particular_sensor_posn_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::string image_URL,threevector& XYZ_posn);

   bool retrieve_all_sensor_posns_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<threevector>& XYZ_posns);
   bool retrieve_all_sensor_posns_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& datum_IDs,std::vector<threevector>& XYZ_posns);

   bool retrieve_particular_sensor_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,threevector& posn,threevector& az_el_roll,
      threevector& f_u0_v0);
   bool retrieve_particular_sensor_hfov_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,double& horiz_fov);

   bool threeD_sensor_metadata_in_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);
   bool retrieve_sensor_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& image_ID,std::vector<std::string>& URL,
      std::vector<double>& FOV_u,std::vector<double>& FOV_v,
      std::vector<double>& U0,std::vector<double>& V0,
      std::vector<double>& az,std::vector<double>& el,
      std::vector<double>& roll,std::vector<double>& camera_lon,
      std::vector<double>& camera_lat,std::vector<double>& camera_alt,
      std::vector<double>& frustum_sidelength);
   bool retrieve_sensor_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& image_ID,std::vector<std::string>& URL,
      std::vector<double>& U0,std::vector<double>& V0,
      std::vector<double>& camera_lon,std::vector<double>& camera_lat);

// Platform metadata methods

   bool insert_platform_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int platform_ID,
      int metadata_ID,int image_ID,std::string image_prefix,
      std::string UTC,double epoch,
      double lon,double lat,double alt,
      double yaw,double pitch,double roll);
   bool retrieve_particular_platform_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,threevector& platform_lla,threevector& platform_rpy);

// World region metadata methods

   bool retrieve_world_region_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,
      int& UTM_zonenumber,std::string& northern_hemisphere_flag);

// Color histogram metadata methods

   bool insert_image_color_histogram(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      double red_frac,double orange_frac,double yellow_frac,
      double green_frac,double blue_frac,double purple_frac,
      double black_frac,double white_frac,double grey_frac,double brown_frac,
      std::string primary_color,std::string secondary_color,
      std::string tertiary_color);
   ATTRIBUTES_METADATA_MAP* retrieve_all_image_color_histograms(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);
   std::string generate_retrieve_image_colorings_SQL_command(
      int campaign_ID,int mission_ID,std::string primary_color);
   bool retrieve_dominant_image_colors_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::string primary_color,
      std::vector<int>& node_IDs,std::vector<int>& datum_IDs);

// Image faces metadata methods

   bool insert_image_face(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      double center_u,double center_v,double radius);
   ATTRIBUTES_METADATA_MAP* retrieve_all_human_faces(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);
   bool retrieve_detected_face_circles_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<twovector>& center,std::vector<double>& radius);
   bool retrieve_detected_face_circles_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      std::vector<twovector>& center,std::vector<double>& radius);

// Image annotations methods

   bool insert_image_annotations(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      const std::vector<double>& U,const std::vector<double>& V,
      const std::vector<std::string>& label);
   bool retrieve_image_annotations_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      std::vector<twovector>& UV,std::vector<std::string>& label);
   ATTRIBUTES_METADATA_MAP* retrieve_all_image_annotations(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);

// Connected component methods

   bool get_connected_component_image_URLs(
      gis_database* gis_database_ptr,int hierarchy_ID,
      int connected_component_ID,std::vector<std::string>& image_URLs);


} // imagesdatabasefunc namespace

#endif // imagesdatabasefuncs.h

