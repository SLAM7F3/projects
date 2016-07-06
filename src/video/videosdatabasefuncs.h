// ==========================================================================
// Header file for videosdatabasefunc namespace
// ==========================================================================
// Last modified on 10/31/13; 11/1/13; 11/4/13
// ==========================================================================

#ifndef VIDEOSDATABASEFUNCS_H
#define VIDEOSDATABASEFUNCS_H

#include <map>
#include <string>
#include <vector>
#include "video/imagesdatabasefuncs.h"

class gis_database;

namespace videosdatabasefunc
{

// Videos database insertion methods

   bool insert_video_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,
      int clip_ID,int frame_ID,
      std::string video_URL,std::string transcript_URL);
   std::string generate_insert_video_metadata_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int clip_ID,int frame_ID,
      std::string video_URL,std::string transcript_URL);
   bool update_keyframe_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,int keyframe_ID);
   std::string generate_update_keyframe_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int keyframe_ID);
      
// Videos database retrieval methods:

   void get_clip_and_frame_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,int& clip_ID,int& frame_ID);
   int get_image_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int clip_ID,int frame_ID);
   void get_image_and_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int clip_ID,int frame_ID,int& image_ID,int& keyframe_ID);

   bool get_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& image_IDs,std::vector<int>& keyframe_IDs);
   std::map<int,std::vector<int> >* get_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID);

   bool get_keyframe_and_data_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& keyframe_IDs,std::vector<int>& data_IDs);
   bool get_keyframe_and_node_IDs(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int campaign_ID,int mission_ID,
      std::vector<int>& keyframe_IDs,std::vector<int>& node_IDs);
   bool get_distinct_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      std::vector<int>& keyframe_IDs);
   imagesdatabasefunc::ATTRIBUTES_METADATA_MAP* 
      retrieve_all_video_keyframes(
         gis_database* gis_database_ptr,int graph_hierarchy_ID,
         int campaign_ID,int mission_ID);

} // videosdatabasefunc namespace

#endif // videosdatabasefuncs.h

