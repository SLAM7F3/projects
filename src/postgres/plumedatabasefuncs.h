// ==========================================================================
// Header file for plumedatabasefunc namespace
// ==========================================================================
// Last modified on 8/1/12; 1/10/13; 1/15/13
// ==========================================================================

#ifndef PLUMEDATABASEFUNCS_H
#define PLUMEDATABASEFUNCS_H

#include <string>
#include <vector>
#include "math/threevector.h"

class gis_database;

namespace plumedatabasefunc
{

// Plume database insertion methods

   bool insert_camera_metadata(
      gis_database* gis_database_ptr,
      int camera_ID,int mission_ID,int status_flag,
      double focal_param,double u0,double v0,
      double azimuth,double elevation,double roll,const threevector& posn);
   std::string generate_insert_camera_metadata_SQL_command(
      int camera_ID,int mission_ID,int status_flag,
      double focal_param,double u0,double v0,
      double azimuth,double elevation,double roll,const threevector& posn);
   bool update_camera_metadata(
      gis_database* gis_database_ptr,int camera_ID,int mission_ID,
      double f,double u0,double v0,
      double azimuth,double elevation,double roll,
      double x_posn,double y_posn,double z_posn);
   std::string generate_update_camera_metadata_SQL_command(
      int camera_ID,int mission_ID,
      double f,double u0,double v0,
      double azimuth,double elevation,double roll,
      double x_posn,double y_posn,double z_posn);

   bool insert_photo_metadata(
      gis_database* gis_database_ptr,
      int photo_ID,int fieldtest_ID,int mission_ID,int slice_number,
      int sensor_ID,std::string date_stamp,double epoch_time,
      std::string URL,std::string thumbnail_URL,std::string mask_URL);
   std::string generate_insert_photo_metadata_SQL_command(
      int photo_ID,int fieldtest_ID,int mission_ID,int slice_number,
      int sensor_ID,std::string date_stamp,double epoch_time,
      std::string URL,std::string thumbnail_URL,std::string mask_URL);

// Plume database retrieval methods

   int retrieve_fieldtest_ID_given_mission_ID(
      gis_database* gis_database_ptr,int mission_ID);
   void retrieve_fieldtest_metadata_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,
      std::string& start_timestamp);
   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID,
      int& day_number,std::string& experiment_label);

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      std::vector<int>& photo_ID,
      std::vector<int>& sensor_ID,
      std::vector<std::string>& date_stamp,
      std::vector<double>& epoch_time,
      std::vector<std::string>& URL,
      std::vector<std::string>& thumbnail_URL,
      std::vector<std::string>& mask_URL);
   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      std::vector<int>& photo_ID,std::vector<int>& sensor_ID,
      std::vector<std::string>& URL,std::vector<std::string>& thumbnail_URL,
      std::vector<std::string>& mask_URL);
   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int slice_number,
      std::vector<int>& photo_ID,std::vector<int>& sensor_ID,
      std::vector<std::string>& URL,std::vector<std::string>& thumbnail_URL,
      std::vector<std::string>& mask_URL);

   std::string retrieve_photo_URL_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int slice_number,int sensor_ID);
   std::vector<std::string> retrieve_photo_URLs_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int sensor_ID);

   std::vector<int> retrieve_photo_slice_numbers_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID);
   double retrieve_slice_epoch_time_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID,
      int slice_number);

   std::string retrieve_mask_URL_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int slice_number,int sensor_ID);
   std::vector<std::string> retrieve_mask_URLs_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int sensor_ID);

   void retrieve_camera_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      std::vector<int>& camera_ID,std::vector<double>& focal_param,
      std::vector<double>& u0,std::vector<double>& v0,
      std::vector<double>& azimuth,std::vector<double>& elevation,
      std::vector<double>& roll,std::vector<double>& x_posn,
      std::vector<double>& y_posn,std::vector<double>& z_posn);
   bool retrieve_camera_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int camera_ID);

   bool retrieve_extremal_slice_IDs_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      int& min_slice_ID,int& max_slice_ID);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // plumedatabasefunc namespace

#endif // plumedatabasefuncs.h

