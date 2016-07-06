// ==========================================================================
// Plumedatabasefuncs namespace method definitions
// ==========================================================================
// Last modified on 7/9/12; 8/1/12; 1/10/13; 1/15/13; 4/5/14
// ==========================================================================

#include <iostream>
#include "astro_geo/Clock.h"
#include "postgres/plumedatabasefuncs.h"
#include "postgres/gis_database.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

namespace plumedatabasefunc
{

// ==========================================================================
// Plume database insertion methods
// ==========================================================================

// Method insert_camera_metadata() takes in metadata for a new entry
// within the sensors table of the plume database.  It
// inserts this metadata into *gis_database_ptr.

   bool insert_camera_metadata(
      gis_database* gis_database_ptr,
      int camera_ID,int mission_ID,int status_flag,
      double focal_param,double u0,double v0,
      double azimuth,double elevation,double roll,const threevector& posn)
   {
      cout << "inside plumedatabasefunc::insert_camera_metadata()" << endl;

      string curr_insert_command=
         plumedatabasefunc::generate_insert_camera_metadata_SQL_command(
            camera_ID,mission_ID,status_flag,focal_param,u0,v0,
            azimuth,elevation,roll,posn);
      cout << curr_insert_command << endl;
    
      vector<string> insert_commands;
      insert_commands.push_back(curr_insert_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method generate_insert_camera_metadata_SQL_command() takes in
// metadata associated with a single camera.  It generates and returns
// a string containing a SQL insert command needed to populate a row
// within the sensors table of the PLUME postgis database.

   string generate_insert_camera_metadata_SQL_command(
      int camera_ID,int mission_ID,int status_flag,
      double focal_param,double u0,double v0,
      double azimuth,double elevation,double roll,const threevector& posn)
   {
      cout << "inside plumedatabasefunc::generate_insert_camera_metadata_SQL_command()" << endl;

      string SQL_command="insert into sensors ";
      SQL_command += "(camera_ID,mission_ID,status_flag,focal_param,u0,v0,azimuth,elevation,roll,x_posn,y_posn,z_posn) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(camera_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(status_flag)+",";
      SQL_command += stringfunc::number_to_string(focal_param)+",";
      SQL_command += stringfunc::number_to_string(u0)+",";
      SQL_command += stringfunc::number_to_string(v0)+",";
      SQL_command += stringfunc::number_to_string(azimuth)+",";
      SQL_command += stringfunc::number_to_string(elevation)+",";
      SQL_command += stringfunc::number_to_string(roll)+",";
      SQL_command += stringfunc::number_to_string(posn.get(0))+",";
      SQL_command += stringfunc::number_to_string(posn.get(1))+",";
      SQL_command += stringfunc::number_to_string(posn.get(2));
      SQL_command += ");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method update_camera_metadata() takes in metadata for an existing
// entry within the sensors table of the plume database.  It
// inserts this metadata into *gis_database_ptr.

   bool update_camera_metadata(
      gis_database* gis_database_ptr,int camera_ID,int mission_ID,
      double f,double u0,double v0,
      double azimuth,double elevation,double roll,
      double x_posn,double y_posn,double z_posn)
   {
      cout << "inside plumedatabasefunc::update_camera_metadata()" << endl;
      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      string curr_update_command=
         plumedatabasefunc::generate_update_camera_metadata_SQL_command(
            camera_ID,mission_ID,f,u0,v0,azimuth,elevation,roll,
            x_posn,y_posn,z_posn);
      cout << "update cmd = " << curr_update_command << endl;

      vector<string> update_commands;
      update_commands.push_back(curr_update_command);
      gis_database_ptr->set_SQL_commands(update_commands);
      bool flag=gis_database_ptr->execute_SQL_commands();

      return flag;
   }

// ---------------------------------------------------------------------   
// Method generate_update_camera_metadata_SQL_command() takes in
// metadata associated with a single tripod camera.
// It generates and returns a string containing a SQL update
// command needed to populate a row within the sensors table
// of the plume postgis database.

   string generate_update_camera_metadata_SQL_command(
      int camera_ID,int mission_ID,
      double f,double u0,double v0,
      double azimuth,double elevation,double roll,
      double x_posn,double y_posn,double z_posn)
   {
//   cout << "inside plumedatabasefunc::generate_update_camera_metadata_SQL_command()" << endl;

      string SQL_command="update sensors ";
      SQL_command += "set focal_param="+stringfunc::number_to_string(f)+",";
      SQL_command += "u0="+stringfunc::number_to_string(u0)+",";
      SQL_command += "v0="+stringfunc::number_to_string(v0)+",";
      SQL_command += "azimuth="+stringfunc::number_to_string(azimuth)+",";
      SQL_command += "elevation="+stringfunc::number_to_string(elevation)+",";
      SQL_command += "roll="+stringfunc::number_to_string(roll)+",";
      SQL_command += "x_posn="+stringfunc::number_to_string(x_posn)+",";
      SQL_command += "y_posn="+stringfunc::number_to_string(y_posn)+",";
      SQL_command += "z_posn="+stringfunc::number_to_string(z_posn)+" ";
      SQL_command += "where camera_id="+stringfunc::number_to_string(
         camera_ID)+" ";
      SQL_command += "and mission_id="+stringfunc::number_to_string(
         mission_ID)+";";
      
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method insert_photo_metadata() takes in metadata for a new entry
// within the photos table of the plume database.  It
// inserts this metadata into *gis_database_ptr.

   bool insert_photo_metadata(
      gis_database* gis_database_ptr,
      int photo_ID,int fieldtest_ID,int mission_ID,int slice_number,
      int sensor_ID,string date_stamp,double epoch_time,
      string URL,string thumbnail_URL,string mask_URL)
   {
//      cout << "inside plumedatabasefunc::insert_photo_metadata()" << endl;

      string curr_insert_command=
         plumedatabasefunc::generate_insert_photo_metadata_SQL_command(
            photo_ID,fieldtest_ID,mission_ID,slice_number,sensor_ID,
            date_stamp,epoch_time,URL,thumbnail_URL,mask_URL);
      cout << curr_insert_command << endl;
    
      vector<string> insert_commands;
      insert_commands.push_back(curr_insert_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method generate_insert_photo_metadata_SQL_command() takes in
// metadata associated with a single photo.  It generates and returns
// a string containing a SQL insert command needed to populate a row
// within the photos table of the PLUME postgis database.

   string generate_insert_photo_metadata_SQL_command(
      int photo_ID,int fieldtest_ID,int mission_ID,int slice_number,
      int sensor_ID,string date_stamp,double epoch_time,
      string URL,string thumbnail_URL,string mask_URL)
   {
//   cout << "inside plumedatabasefunc::generate_insert_photo_metadata_SQL_command()" << endl;

      string SQL_command="insert into photos ";
      SQL_command += "(photo_ID,fieldtest_ID,mission_ID,slice_number,sensor_ID,time_stamp,epoch_time,URL,thumbnail_URL,mask_URL) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(photo_ID)+",";
      SQL_command += stringfunc::number_to_string(fieldtest_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(slice_number)+",";
      SQL_command += stringfunc::number_to_string(sensor_ID)+",";

      SQL_command += "'"+date_stamp+"',";
      SQL_command += stringfunc::number_to_string(epoch_time)+",";

      SQL_command += "'"+URL+"',";
      SQL_command += "'"+thumbnail_URL+"',";
      SQL_command += "'"+mask_URL+"'";
      SQL_command += ");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ==========================================================================
// Plume database retrieval methods
// ==========================================================================

// Method retrieve_fieldtest_ID_given_mission_ID()

   int retrieve_fieldtest_ID_given_mission_ID(
      gis_database* gis_database_ptr,int mission_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_fieldtest_ID_given_mission_ID()"
//           << endl;
      string curr_select_command = 
         "SELECT fieldtest_id from missions ";
      curr_select_command += "where mission_ID="+
         stringfunc::number_to_string(mission_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      if (field_array_ptr==NULL) return -1;

      int fieldtest_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return fieldtest_ID;
   }

// ---------------------------------------------------------------------   
// Method retrieve_fieldtest_metadata_from_database() retrieves start
// timestamp for the specified fieldtest.

   void retrieve_fieldtest_metadata_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,
      string& start_timestamp)
   {
//      cout << "inside plumedatabasefunc::retrieve_fieldtest_metadata_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT start_date from fieldtests ";
      curr_select_command += "where ID="+
         stringfunc::number_to_string(fieldtest_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      start_timestamp=field_array_ptr->get(0,0);
//      cout << "start_timestamp = " << start_timestamp << endl;
   }

// ---------------------------------------------------------------------   
// Method retrieve_mission_metadata_from_database() retrieves the day
// number and experiment label given input fieldtest and mission IDs.

   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID,
      int& day_number,string& experiment_label)
   {
//      cout << "inside plumedatabasefunc::retrieve_mission_metadata_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT day_number,experiment_label from missions ";
      curr_select_command += "where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID)+
         " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      day_number=stringfunc::string_to_number(field_array_ptr->get(0,0));
      experiment_label=field_array_ptr->get(0,1);
//      cout << "day_number = " << day_number
//           << " experiment_label = " << experiment_label << endl;
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_metadata_from_database()

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID,
      vector<int>& photo_ID,vector<int>& sensor_ID,
      vector<string>& timestamp,vector<double>& epoch_time,
      vector<string>& URL,vector<string>& thumbnail_URL,
      vector<string>& mask_URL)
   {
//      cout << "inside plumedatabasefunc::retrieve_photo_metadata_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT photo_ID,sensor_ID,time_stamp,epoch_time,URL,thumbnail_URL,mask_URL ";
      curr_select_command += "from photos where mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command == " AND fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_photo_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,0));
         int curr_sensor_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,1));
         string curr_timestamp=field_array_ptr->get(i,2);
         double curr_epoch_time=stringfunc::string_to_number(
            field_array_ptr->get(i,3));
         string curr_URL=field_array_ptr->get(i,4);
         string curr_thumbnail_URL=field_array_ptr->get(i,5);
         string curr_mask_URL=field_array_ptr->get(i,6);

         photo_ID.push_back(curr_photo_ID);
         sensor_ID.push_back(curr_sensor_ID);
         timestamp.push_back(curr_timestamp);
         epoch_time.push_back(curr_epoch_time);
         URL.push_back(curr_URL);
         thumbnail_URL.push_back(curr_thumbnail_URL);
         mask_URL.push_back(curr_mask_URL);
      }   
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_metadata_from_database()

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<int>& photo_ID,vector<int>& sensor_ID,
      vector<string>& URL,vector<string>& thumbnail_URL,
      vector<string>& mask_URL)
   {
//      cout << "inside plumedatabasefunc::retrieve_photo_metadata_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT photo_ID,sensor_ID,URL,thumbnail_URL,mask_URL ";
      curr_select_command += "from photos where mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " and epoch_time > "+stringfunc::number_to_string(
         start_epoch_time);
      curr_select_command += " and epoch_time < "+stringfunc::number_to_string(
         stop_epoch_time);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_photo_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,0));
         int curr_sensor_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,1));
         string curr_URL=field_array_ptr->get(i,2);
         string curr_thumbnail_URL=field_array_ptr->get(i,3);
         string curr_mask_URL=field_array_ptr->get(i,4);

/*
// FAKE FAKE:  Tues Dec 27, 2011 at 4:40 pm
// Next line for alg testing and viewgraphs only

//         if (curr_sensor_ID==20)
//         if (curr_sensor_ID==22)
//         if (curr_sensor_ID==20 || curr_sensor_ID==22)
//         if (curr_sensor_ID==20 || curr_sensor_ID==22 || curr_sensor_ID==25)
         {
         }
         else
         {
            continue;
         }
*/
     
         photo_ID.push_back(curr_photo_ID);
         sensor_ID.push_back(curr_sensor_ID);
         URL.push_back(curr_URL);
         thumbnail_URL.push_back(curr_thumbnail_URL);
         mask_URL.push_back(curr_mask_URL);
      }   
   }

// ---------------------------------------------------------------------   
// This overloaded version of retrieve_photo_metadata_from_database()
// returns photo metadata corresponding to a specified
// fieldtest, mission and time slice.

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int slice_number,
      vector<int>& photo_ID,vector<int>& sensor_ID,
      vector<string>& URL,vector<string>& thumbnail_URL,
      vector<string>& mask_URL)
   {
//      cout << "inside plumedatabasefunc::retrieve_photo_metadata_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT photo_ID,sensor_ID,URL,thumbnail_URL,mask_URL ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND slice_number="+
         stringfunc::number_to_string(slice_number);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_photo_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,0));
         int curr_sensor_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,1));
         string curr_URL=field_array_ptr->get(i,2);
         string curr_thumbnail_URL=field_array_ptr->get(i,3);
         string curr_mask_URL=field_array_ptr->get(i,4);

         photo_ID.push_back(curr_photo_ID);
         sensor_ID.push_back(curr_sensor_ID);
         URL.push_back(curr_URL);
         thumbnail_URL.push_back(curr_thumbnail_URL);
         mask_URL.push_back(curr_mask_URL);
      }   
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_URL_from_database() returns the photo URL
// corresponding to a specified fieldtest, mission, slice number and
// sensor:

   string retrieve_photo_URL_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int slice_number,int sensor_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_photo_URL_from_database()"
//           << endl;
      string curr_select_command = "SELECT URL ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND slice_number="+
         stringfunc::number_to_string(slice_number);
      curr_select_command += " AND sensor_ID="+
         stringfunc::number_to_string(sensor_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      string photo_URL="";
      if (field_array_ptr != NULL)
      {
         photo_URL=field_array_ptr->get(0,0);
      }
      return photo_URL;
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_URLs_from_database() returns an STL vector
// containing chronologically ordered photo URLs corresponding to a
// specified fieldtest, mission and sensor:

   vector<string> retrieve_photo_URLs_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int sensor_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_photo_URLs_from_database()"
//           << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;
//      cout << "fieldtest_ID = " << fieldtest_ID
//           << " mission_ID = " << mission_ID
//           << " sensor_ID = " << sensor_ID << endl;

      string curr_select_command = "SELECT URL ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND sensor_ID="+
         stringfunc::number_to_string(sensor_ID);
      curr_select_command += " ORDER BY epoch_time";
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);
//      cout << "field_array_ptr = " << field_array_ptr << endl;

      vector<string> photo_URLs;
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         photo_URLs.push_back(field_array_ptr->get(i,0));
      }
      return photo_URLs;
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_slice_numbers_from_database() returns an STL
// vector containing chronologically ordered photo time slice numbers
// corresponding to a specified fieldtest and mission:

   vector<int> retrieve_photo_slice_numbers_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_photo_slice_numbers_from_database()"
//           << endl;
      string curr_select_command = "SELECT DISTINCT slice_number ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " ORDER BY slice_number";
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      vector<int> slice_numbers;
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         slice_numbers.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,0)));
      }
      return slice_numbers;
   }

// ---------------------------------------------------------------------   
// Method retrieve_slice_epoch_time_from_database() returns the epoch
// time corresponding to the photos within the photos table of the
// plume database specified by the input slice_number.

   double retrieve_slice_epoch_time_from_database(
      gis_database* gis_database_ptr,int fieldtest_ID,int mission_ID,
      int slice_number)
   {
//      cout << "inside plumedatabasefunc::retrieve_slice_epoch_time_from_database()"
//           << endl;
      string curr_select_command = "SELECT DISTINCT epoch_time ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND slice_number="+
         stringfunc::number_to_string(slice_number);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      double epoch_time=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      return epoch_time;
   }

// ---------------------------------------------------------------------   
// Method retrieve_mask_URL_from_database() return the unique mask URL
// corresponding to a specified fieldtest, mission, time slice
// and sensor:

   string retrieve_mask_URL_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int slice_number,int sensor_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_mask_URL_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT mask_URL ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND slice_number="+
         stringfunc::number_to_string(slice_number);
      curr_select_command += " AND sensor_ID="+
         stringfunc::number_to_string(sensor_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      string curr_mask_URL="";
      if (field_array_ptr != NULL)
      {
         curr_mask_URL=field_array_ptr->get(0,0);
      }
      return curr_mask_URL;
   }

// ---------------------------------------------------------------------   
// Method retrieve_mask_URLs_from_database() returns an STL vector
// containing chronologically ordered mask URLs corresponding to a
// specified fieldtest, mission and sensor:

   vector<string> retrieve_mask_URLs_from_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int sensor_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_mask_URLs_from_database()"
//           << endl;
      string curr_select_command = 
         "SELECT mask_URL ";
      curr_select_command += "from photos where fieldtest_ID="+
         stringfunc::number_to_string(fieldtest_ID);
      curr_select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND sensor_ID="+
         stringfunc::number_to_string(sensor_ID);
      curr_select_command += " ORDER BY epoch_time";
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      vector<string> mask_URLs;
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         mask_URLs.push_back(field_array_ptr->get(i,0));
      }
      return mask_URLs;
   }

// ---------------------------------------------------------------------   
// Method retrieve_camera_metadata_from_database() returns az, el and
// roll angles in radians.

   void retrieve_camera_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      vector<int>& camera_ID,vector<double>& focal_param,
      vector<double>& u0,vector<double>& v0,
      vector<double>& azimuth,vector<double>& elevation,
      vector<double>& roll,vector<double>& x_posn,vector<double>& y_posn,
      vector<double>& z_posn)
   {
      string curr_select_command = 
         "SELECT camera_ID,focal_param,u0,v0,azimuth,elevation,roll,x_posn,y_posn,z_posn ";
      curr_select_command += " from sensors where mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " order by camera_id";

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_camera_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,0));
         double curr_focal_param=stringfunc::string_to_number(
            field_array_ptr->get(i,1));
         double curr_u0=stringfunc::string_to_number(
            field_array_ptr->get(i,2));
         double curr_v0=stringfunc::string_to_number(
            field_array_ptr->get(i,3));

         double curr_az=stringfunc::string_to_number(
            field_array_ptr->get(i,4))*PI/180;
         double curr_el=stringfunc::string_to_number(
            field_array_ptr->get(i,5))*PI/180;
         double curr_roll=stringfunc::string_to_number(
            field_array_ptr->get(i,6))*PI/180;

         double curr_x=stringfunc::string_to_number(
            field_array_ptr->get(i,7));
         double curr_y=stringfunc::string_to_number(
            field_array_ptr->get(i,8));
         double curr_z=stringfunc::string_to_number(
            field_array_ptr->get(i,9));

         camera_ID.push_back(curr_camera_ID);
         focal_param.push_back(curr_focal_param);
         u0.push_back(curr_u0);
         v0.push_back(curr_v0);
         azimuth.push_back(curr_az);
         elevation.push_back(curr_el);
         roll.push_back(curr_roll);
         x_posn.push_back(curr_x);
         y_posn.push_back(curr_y);
         z_posn.push_back(curr_z);
      } // loop over index i
   }

// ---------------------------------------------------------------------   
// This overloaded version of retrieve_camera_metadata_from_database()
// tests if a row in the sensors table corresponding to the input mission
// and camera IDs already exists.  If so, this boolean method returns
// true.

   bool retrieve_camera_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int camera_ID)
   {
//      cout << "inside plumedatabasefunc::retrieve_camera_metadata_from_database()" << endl;

      string curr_select_command = "SELECT * from sensors ";
      curr_select_command += " WHERE mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " AND camera_ID="+
         stringfunc::number_to_string(camera_ID);
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

//      cout << "field_array_ptr = " << field_array_ptr << endl;
      if (field_array_ptr==NULL) return false;

      return (field_array_ptr->get_mdim() > 0);
   }

// ---------------------------------------------------------------------   
// Method retrieve_extremal_slice_IDs_from_database() searches the
// photos table in the plume database for the min and max time slice
// numbers.

   bool retrieve_extremal_slice_IDs_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      int& min_slice_ID,int& max_slice_ID)
   {
      string curr_select_command = "SELECT distinct slice_number from photos ";
      curr_select_command += " WHERE mission_ID="+
         stringfunc::number_to_string(mission_ID);
      curr_select_command += " ORDER BY slice_number";
//      cout << "curr_select_cmd = " << curr_select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_command);

//      cout << "field_array_ptr = " << field_array_ptr << endl;
      if (field_array_ptr==NULL) return false;

      min_slice_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      int mdim=field_array_ptr->get_mdim();
      max_slice_ID=stringfunc::string_to_number(
         field_array_ptr->get(mdim-1,0));
      return true;
   }


} // plumedatabasefunc namespace

