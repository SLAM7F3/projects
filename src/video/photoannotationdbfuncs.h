// ==========================================================================
// Header file for photoannotationdbfunc namespace
// ==========================================================================
// Last modified on 1/18/11
// ==========================================================================

#ifndef PHOTOANNOTATIONDBFUNCS_H
#define PHOTOANNOTATIONDBFUNCS_H

#include <string>
#include <vector>
#include "color/colorfuncs.h"

class gis_database;

namespace photoannotationdbfunc
{

// Manipulation methods for photo annotations table in TOC database:

   bool insert_photo_annotation(
      gis_database* gis_database_ptr,int fieldtest_ID,int photo_ID,
      double secs_since_epoch,std::string username,std::string label,
      std::string description,std::string color,int importance,
      double U,double V);
   std::string generate_insert_photo_annotation_SQL_command(
      int fieldtest_ID,int photo_ID,double secs_since_epoch,
      std::string username,std::string label,
      std::string description,std::string color,
      int importance,double U,double V);

   int get_photo_annotation_ID(
      gis_database* gis_database_ptr,
      std::string username,std::string label,std::string description);
   void get_particular_photo_annotations(
      gis_database* gis_database_ptr,int photo_ID,
      std::vector<int>& annotation_IDs,
      std::vector<std::string>& creation_times,
      std::vector<std::string>& usernames,std::vector<std::string>& labels,
      std::vector<std::string>& descriptions,
      std::vector<std::string>& colors,
      std::vector<int>& importances,std::vector<twovector>& UVs);
   void get_fieldtest_photo_annotations(
      gis_database* gis_database_ptr,int fieldtest_ID,
      std::vector<int>& annotation_IDs,std::vector<int>& photo_IDs,
      std::vector<std::string>& photo_times,
      std::vector<std::string>& usernames,
      std::vector<std::string>& labels,
      std::vector<std::string>& descriptions,
      std::vector<std::string>& colors,
      std::vector<int>& importances,
      std::vector<twovector>& UVs);
   void get_all_photo_annotations(
      gis_database* gis_database_ptr,int selected_fieldtest_ID,
      std::vector<int>& annotation_IDs,
      std::vector<std::string>& creation_times,
      std::vector<std::string>& usernames,
      std::vector<std::string>& labels,
      std::vector<std::string>& descriptions,
      std::vector<std::string>& colors,
      std::vector<int>& importances,
      std::vector<twovector>& UVs);

   bool update_photo_annotation(
      gis_database* gis_database_ptr,
      int annotation_ID,double secs_since_epoch,
      std::string username,std::string label,std::string description,
      std::string color,int importance,double U,double V);
   std::string generate_update_photo_annotation_SQL_command(
      int annotation_ID,double secs_since_epoch,
      std::string username,std::string label,std::string description,
      std::string color,int importance,double U,double V);

   bool delete_photo_annotation(
      gis_database* gis_database_ptr,int annotation_ID);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // photoannotationdbfunc namespace

#endif // photoannotationdbfuncs.h

