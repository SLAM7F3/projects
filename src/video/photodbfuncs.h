// ==========================================================================
// Header file for photodbfunc namespace
// ==========================================================================
// Last modified on 7/26/11; 7/27/11; 7/29/11; 4/5/14
// ==========================================================================

#ifndef PHOTODBFUNCS_H
#define PHOTODBFUNCS_H

#include <map>
#include <string>
#include <vector>
#include "astro_geo/geopoint.h"
#include "graphs/graphdbfuncs.h"

class Clock;
class gis_database;
class graph;
class graph_hierarchy;
class Messenger;
class node;
class photograph;
class photogroup;
class sift_feature;
class track;
class tracks_group;

namespace photodbfunc
{
   typedef std::map<int,std::vector<std::string> > PHOTO_IDS_METADATA_MAP;

// JSON file export methods

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,std::string bundler_IO_subdir,
      std::vector<int>& photo_ID,std::vector<int>& photo_importance,
      std::vector<int>& npx,std::vector<int>& npy,
      std::vector<int>& thumbnail_npx,std::vector<int>& thumbnail_npy,
      std::vector<std::string>& image_filenames,
      std::vector<std::string>& photo_timestamp,
      std::vector<std::string>& photo_URL,
      std::vector<std::string>& thumbnail_URL,
      std::vector<double>& zposn,std::vector<double>& azimuth,
      std::vector<double>& elevation,std::vector<double>& roll,
      std::vector<double>& focal_param,
      std::vector<double>& longitude,std::vector<double>& latitude);
   photogroup* generate_photogroup_from_database(
      gis_database* gis_database_ptr,std::string bundler_IO_subdir);
   void export_JSON_files(
      graph_hierarchy& graphs_pyramid,photogroup* photogroup_ptr,
      std::string bundler_IO_subdir);
   void write_graph_json_file(graph* graph_ptr,photogroup* photogroup_ptr,
      std::string json_filename);

   std::string output_node_GraphML(
      int n_indent,node* node_ptr,photograph* photograph_ptr,
      bool terminal_node_flag);

   void write_geolocation_JSON_file(
      std::vector<int>& photo_ID,std::vector<double>& longitude,
      std::vector<double>& latitude,std::string json_filename);
   std::string generate_geolocation_JSON_string(
      std::vector<int>& photo_ID,std::vector<double>& longitude,
      std::vector<double>& latitude);
   std::string output_geolocation_GraphML(
      unsigned int n_indent,int photo_ID,double longitude,double latitude,
      bool terminal_node_flag);

   void write_metadata_JSON_file(
      int requested_photo_ID,
      std::vector<int>& photo_ID,std::vector<int>& npx,std::vector<int>& npy,
      std::vector<std::string>& image_filenames,
      std::vector<std::string>& image_timestamps,
      std::vector<double>& longitude,std::vector<double>& latitude,
      std::vector<double>& zposn,std::vector<double>& azimuth,
      std::vector<double>& elevation,
      std::vector<double>& roll,std::string json_filename);
   std::string generate_metadata_JSON_string(
      int requested_photo_ID,
      std::vector<int>& photo_ID,std::vector<int>& npx,std::vector<int>& npy,
      std::vector<std::string>& image_filenames,
      std::vector<std::string>& image_timestamps,
      std::vector<double>& longitude,std::vector<double>& latitude,
      std::vector<double>& zposn,std::vector<double>& azimuth,
      std::vector<double>& elevation,std::vector<double>& roll);
   std::string output_metadata_GraphML(
      int n_indent,int photo_ID,int npx,int npy,
      std::string image_filename,std::string photo_timestamp,
      double longitude,double latitude,double zposn,
      double azimuth,double elevation,double roll,
      bool terminal_node_flag);

// Database metadata insertion methods

   std::string generate_insert_photo_SQL_command(
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      int photo_counter,bool genuine_timestamp_flag,double secs_since_epoch,
      bool genuine_geolocation_flag,const geopoint& geolocation,
      std::string URL,int npx,int npy,int importance);
   std::string generate_insert_photo_SQL_command(
      int photo_ID,int time_stamp,std::string photo_URL,
      int xdim,int ydim,
      std::string thumbnail_URL,int thumbnail_xdim,int thumbnail_ydim,
      double longitude,double latitude,double altitude,
      double az,double el,double roll,double focal_param);

   bool insert_photo_metadata_into_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      Messenger* messenger_ptr,std::string progress_type,
      bool genuine_timestamp_flag,const std::vector<double>& secs_elapsed,
      bool genuine_geolocation_flag,
      const std::vector<geopoint>& geolocations,
      const std::vector<std::string>& photo_filenames,
      const std::vector<int>& xdim,const std::vector<int>& ydim);

// Database metadata retrieval methods

   std::string generate_retrieve_photos_SQL_command(int mission_ID);
   std::string generate_retrieve_photos_SQL_command(
      int mission_ID,int sensor_ID);
   std::string generate_retrieve_photo_IDs_SQL_command(int mission_ID);
   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      std::vector<int>& photo_IDs,std::vector<std::string>& 
      photo_filenames,std::vector<int>& photo_framenumbers);
   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      std::vector<int>& photo_IDs,std::vector<std::string>& 
      photo_filenames,std::vector<int>& photo_framenumbers,
      std::vector<std::string>& photo_timestamps);

   void retrieve_photo_IDs_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      std::vector<int>& photo_IDs);
   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      std::vector<int>& photo_IDs,
      std::vector<std::string>& photo_timestamps);
   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      std::vector<int>& photo_IDs,
      std::vector<std::string>& photo_timestamps,
      std::vector<std::string>& photo_urls,
      std::vector<twovector>& photo_npxnpys,
      std::vector<twovector>& photo_lonlats,
      std::vector<int>& photo_framenumbers);
   PHOTO_IDS_METADATA_MAP* retrieve_photo_metadata_for_graph(
      gis_database* gis_database_ptr,int graph_ID);
   PHOTO_IDS_METADATA_MAP* retrieve_photo_URLs_vs_node_IDs(
      gis_database* gis_database_ptr,int graph_ID);
   
   std::string generate_retrieve_particular_photo_SQL_command(int photo_ID);
   bool retrieve_particular_photo_metadata_from_database(
      gis_database* gis_database_ptr,int photo_ID,std::string& caption,
      std::string& photo_timestamp,std::string& photo_URL,
      int& photo_counter,int& importance);
   bool retrieve_particular_photo_metadata_from_database(
      gis_database* gis_database_ptr,int photo_ID,
      std::string& photo_timestamp,std::string& photo_URL,int& npx,int& npy,
      std::string& thumbnail_URL,int& thumbnail_npx,int& thumbnail_npy,
      int& importance,int& photo_counter);
   bool retrieve_photo_ID_URL_given_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int& photo_ID,std::string& photo_URL,std::string& thumbnail_URL);
   int retrieve_graph_hierarchy_for_particular_photo(
      gis_database* gis_database_ptr,int photo_ID);
   std::string retrieve_photo_subdir_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID);

// Database metadata update methods

   bool update_photo_timestamps_in_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      const std::vector<int>& photo_IDs,
      const std::vector<std::string>& photo_timestamps);
   bool update_thumbnail_metadata_in_database(
      gis_database* gis_database_ptr,const std::vector<int>& photo_IDs,
      const std::vector<std::string>& thumbnail_filenames,
      const std::vector<int>& xdim,const std::vector<int>& ydim);
   std::string generate_photo_thumbnail_SQL_command(
      int photo_ID,std::string thumbnail_URL,int npx,int npy);
   bool update_photo_importance_in_database(
      gis_database* gis_database_ptr,std::string URL,int importance);
   std::vector<fourvector> compute_photo_importance_intervals(
      gis_database* gis_database_ptr,int mission_ID);

   int fuse_photo_and_gps_metadata(
      gis_database* gis_database_ptr,int selected_mission_ID,
      tracks_group* tracks_group_ptr);
   bool update_photo_geometries_in_database(
      gis_database* gis_database_ptr,
      const std::vector<int>& photo_IDs,
      const std::vector<double>& photo_lons,
      const std::vector<double>& photo_lats,
      const std::vector<double>& photo_alts);

// TOC specific methods

   double extract_droid_image_time_from_metadata_file(
      std::string image_filename,Clock& clock);
   std::string append_counter_to_image_filenames(
      std::string curr_image_filename,int& good_image_counter);
   bool photo_in_database(gis_database* gis_database_ptr,int photo_ID);
   int get_photo_ID(
      gis_database* gis_database_ptr,std::string URL);
   std::string get_photo_URL(
      gis_database* gis_database_ptr,int photo_ID,bool thumbnail_flag);
//   std::string get_thumbnail_URL(
//      gis_database* gis_database_ptr,int photo_ID);
   twovector get_photo_dims(gis_database* gis_database_ptr,int photo_ID);

// Graph database querying methods

   std::string write_graph_json_string(
      gis_database* gis_database_ptr,int hierarchy_ID,graph* graph_ptr,
      bool get_nodes_flag,bool get_edges_flag,
      const std::vector<int>& incident_node_IDs);
   std::string write_node_json_string(
      int n_indent,int graph_hierarchy_ID,node* node_ptr,
      gis_database* gis_database_ptr,
      PHOTO_IDS_METADATA_MAP* photo_ids_metadata_map_ptr,
      graphdbfunc::NODE_IDS_LABELS_MAP* node_ids_labels_map_ptr,
      bool terminal_node_flag);

// SIFT features and matches insertion methods

   std::string generate_insert_sift_feature_SQL_command(
      sift_feature* sift_feature_ptr);
   std::string generate_insert_sift_match_SQL_command(
      int photo_ID1,int feature_ID1,int photo_ID2,int feature_ID2);

// SIFT features and matches retrieval methods

   std::string generate_retrieve_SIFT_matches_SQL_command(
      int photo_ID1,int photo_ID2);
   bool retrieve_SIFT_matches_from_database(
      gis_database* gis_database_ptr,int photo_ID1,int photo_ID2,
      std::vector<twovector>& feature_matches);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // photodbfunc namespace

#endif // photodbfuncs.h

