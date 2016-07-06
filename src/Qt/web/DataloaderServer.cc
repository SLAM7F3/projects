// ==========================================================================
// DATALOADERSERVER class file
// ==========================================================================
// Last updated on 9/11/10; 1/13/11; 1/18/11; 6/1/11
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "Qt/web/DataloaderServer.h"
#include "postgres/databasefuncs.h"
#include "image/imagefuncs.h"
#include "track/mover_funcs.h"
#include "video/photodbfuncs.h"
#include "video/photograph.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void DataloaderServer::allocate_member_objects()
{
}		       

void DataloaderServer::initialize_member_objects()
{
   clock_ptr=NULL;
   viewer_messenger_ptr=NULL;
   gps_tracksgroup_ptr=NULL;
   selected_fieldtest_ID=selected_mission_ID=selected_platform_ID=
      selected_sensor_ID=-1;
}

DataloaderServer::DataloaderServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
DataloaderServer::~DataloaderServer()
{
   if (gps_tracksgroup_ptr != NULL)
   {
      gps_tracksgroup_ptr->destroy_all_tracks();
   }
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function get_sensor_type() contains hard-wired sensor ID
// information from the Sensors table in the TOC database.  Given an
// input sensor_ID, this method returns its general type.  We wrote
// this little utility member function in order to distinguish GPS,
// MIDG and imagery data products.

DataloaderServer::SensorType DataloaderServer::get_sensor_type(
   int sensor_ID) const
{
//   cout << "inside DataloaderServer::get_sensor_type()" << endl;

/*

Sensors table in TOC database as of 8/19/10:

"1";"Microsoft webcam"
"2";"Logitech webcam"
"3";"D7 pancam"
"4";"Hockeypuck GPS device"
"5";"MIDG GPS/INS"
"9";"Canon powershot"
"10";"GPS camera"
"11";"Flip videocam"
"12";"Quad GPS/IMU"
"13";"Axis camera"
"14";"Garmin GPS"
"15";"Droid GPS"
"16";"Droid camera"
"17";"Pan-tilt camera"
"18";"FLIR"
"19";"Generic point and shoot"
*/

   if (sensor_ID==4)
   {
      return DataloaderServer::GPS;
   }
   else if (sensor_ID==5)
   {
      return DataloaderServer::MIDG;
   }
   else if (sensor_ID==10)
   {
      return DataloaderServer::GPSCAMIMAGE;
   }
   else if (sensor_ID==12)
   {
      return DataloaderServer::QUADGPS;
   }
   else if (sensor_ID==13)
   {
      return DataloaderServer::AXISIMAGE;
   }
   else if (sensor_ID==14)
   {
      return DataloaderServer::GARMINGPS;
   }
   else if (sensor_ID==15)
   {
      return DataloaderServer::DROIDGPS;
   }
   else if (sensor_ID==16)
   {
      return DataloaderServer::DROIDIMAGE;
   }
   else if (sensor_ID==1 || sensor_ID==2 || sensor_ID==3 || 
	    sensor_ID==9 || sensor_ID==10 || sensor_ID==11 || sensor_ID==13 ||
            sensor_ID==17 || sensor_ID==18 || sensor_ID==19)
   {
      return DataloaderServer::IMAGE;
   }
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray DataloaderServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
//   cout << "inside DataloaderServer:get() method" << endl;

   Q_UNUSED(responseHeader);

   doc.appendChild( response );

   URL_path=url.path().toStdString();
//   cout << "URL path = " << URL_path << endl;
   extract_KeyValue_pairs(url);

// In order to parse variable URLs (e.g. screen capture containing
// filenumber as a ratio), decompose URL into substrings separated by
// underscores:
 
   vector<string> URL_substrings=
      stringfunc::decompose_string_into_substrings(URL_path,"_");
//   for (int s=0; s<URL_substrings.size(); s++)
//   {
//      cout << "s = " << s
//           << " URL_substrings[s] = " << URL_substrings[s] << endl;
//   }

   string response_msg;

   if (URL_path=="/Update_Selector_Metadata/")
   {
      return update_selector_metadata();
   }

   else if (URL_path=="/Update_Fieldtest_Dropdown/")
   {
      return update_fieldtest_dropdown();
   }
   else if (URL_path=="/Update_Mission_Dropdown/")
   {
      return update_mission_dropdown();
   }
   else if (URL_path=="/Update_Platform_Dropdown/")
   {
      return update_platform_dropdown();
   }
   else if (URL_path=="/Update_Sensor_Dropdown/")
   {
      return update_sensor_dropdown();
   }

   else if (URL_path=="/Set_Fieldtest_Params/")
   {
      return set_fieldtest_params();
   }
   else if (URL_path=="/Set_Mission_Params/")
   {
      return set_mission_params();
   }

   else if (URL_path=="/Pick_Mission/")
   {
      pick_mission();
      return generate_JSON_response_to_picked_mission();
   }
   else if (URL_path=="/Select_Data_File/")
   {
      return select_data_file();
   }
   else if (URL_path=="/Upload_Data/")
   {
      return upload_data();
   }
   else if (URL_path=="/Upload_Thumbnails/")
   {
      return update_thumbnail_metadata_in_database();
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray DataloaderServer::post(const QUrl& url, const QByteArray& postData,
                             QHttpResponseHeader& responseHeader)
{
//   cout << "inside DataloaderServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   DataloaderServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;

/*
   if (URL_path=="/Set_Tour_Path/")
   {
      string error_msg;
      set_tour_path(postData,error_msg);
      ModeController_ptr->setState(ModeController::RUN_MOVIE);
      Grid_ptr->set_mask(0);

// Jump to starting photo position and then suppress ladar point cloud:

      PhotoTour* PhotoTour_ptr=PhotoToursGroup_ptr->get_PhotoTour_ptr(0);
      int starting_OBSFRUSTUM_ID=PhotoTour_ptr->get_starting_OBSFRUSTUM_ID();
      OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(
         starting_OBSFRUSTUM_ID,0);
      PhotoTour_ptr->set_prev_OBSFRUSTUM_ID(starting_OBSFRUSTUM_ID);
      suppress_ladar_point_cloud();

      return generate_JSON_response_to_tourpath_entry();
   }
*/

   return doc.toByteArray();
}

// ==========================================================================
// Dropdown update member functions
// ==========================================================================

// Member function update_selector_metadata()

QByteArray DataloaderServer::update_selector_metadata()
{
   cout << "inside DataloaderServer::update_selector_metadata()" << endl;

   vector<string> fieldtest_label,mission_label,platform_label;
   vector<int> fieldtest_ID,mission_ID,platform_ID;

// Retrieve correlated fieldtest, mission and platform label and ID
// information which are ordered chornologically (increasing):

   mover_func::retrieve_fieldtest_mission_platform_metadata_from_database(
      gis_database_ptr,fieldtest_label,fieldtest_ID,
      mission_label,mission_ID,platform_label,platform_ID);

// Store correlated metadata retrieved from database within STL maps:

   typedef map<int,vector<int> > ONE_ID_TO_MANY_ID_MAP;
   typedef map<int,int> ONE_ID_TO_ONE_ID_MAP;
   typedef map<int,string> ID_TO_LABEL_MAP;

   ONE_ID_TO_MANY_ID_MAP fieldtest_mission_IDs_map;
   ONE_ID_TO_ONE_ID_MAP mission_platform_IDs_map;
   ID_TO_LABEL_MAP fieldtest_label_map,mission_label_map,platform_label_map;
   
   for (int i=0; i<fieldtest_ID.size(); i++)
   {
      int curr_fieldtest_ID=fieldtest_ID[i];

      vector<int> curr_mission_IDs;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=fieldtest_mission_IDs_map.find(
         curr_fieldtest_ID);
      if (iter != fieldtest_mission_IDs_map.end())
      {
         curr_mission_IDs=iter->second;
      }
      curr_mission_IDs.push_back(mission_ID[i]);

      fieldtest_mission_IDs_map[curr_fieldtest_ID]=curr_mission_IDs;
      fieldtest_label_map[curr_fieldtest_ID]=fieldtest_label[i];
   } // loop over index i labeling fieldtest IDs
   
   for (int m=0; m<mission_ID.size(); m++)
   {
      int curr_mission_ID=mission_ID[m];
      int curr_platform_ID=platform_ID[m];
      mission_platform_IDs_map[curr_mission_ID]=curr_platform_ID;
      mission_label_map[curr_mission_ID]=mission_label[m];
   }

   for (int p=0; p<platform_ID.size(); p++)
   {
      int curr_platform_ID=platform_ID[p];
      platform_label_map[curr_platform_ID]=platform_label[p];
   } // loop over index i labeling platform IDs

   string json_string = "{ ";

// Write fieldtest_ID, fieldtest_label and corresponding mission IDs
// to output JSON string:

   json_string += " \"Fieldtest_ID_label_missionIDs\": [ ";   
   int n_fieldtests=fieldtest_ID.size();
   int prev_fieldtest_ID=-1;
   for (int i=0; i<n_fieldtests; i++)
   {
      int curr_fieldtest_ID=fieldtest_ID[i];

// Do not repeat fieldtest information within output JSON string more
// than once:

      if (curr_fieldtest_ID==prev_fieldtest_ID)
      {
         continue;
      }
      else
      {
         prev_fieldtest_ID=curr_fieldtest_ID;
      }

      ID_TO_LABEL_MAP::iterator label_iter=fieldtest_label_map.find(
         curr_fieldtest_ID);
      string curr_fieldtest_label=label_iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_fieldtest_ID)+","
         +"\""+curr_fieldtest_label+"\" ," ;

      json_string += " [ ";

      vector<int> curr_mission_IDs;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=fieldtest_mission_IDs_map.find(
         curr_fieldtest_ID);
      if (iter != fieldtest_mission_IDs_map.end())
      {
         curr_mission_IDs=iter->second;
      }
      int n_missions=curr_mission_IDs.size();

      cout << "Fieldtest ID = " << curr_fieldtest_ID << endl;
      for (int j=0; j<n_missions; j++)
      {
         int curr_mission_ID=curr_mission_IDs[j];
         json_string += stringfunc::number_to_string(curr_mission_ID);
         if (j < n_missions-1)
         {
            json_string += ",";
         }
      }
      json_string += " ] ] ";
      if (i < n_fieldtests-1)
      {
         json_string += ",";
      }
   } // loop over index i labeling fieldtest ID in chronological order

   json_string += " ], \n";

// Write mission_ID vs mission_label to output JSON string:

   json_string += " \"Mission_IDs_labels\": [ ";   
   int n_missions=mission_ID.size();
   for (int i=0; i<n_missions; i++)
   {
      int curr_mission_ID=mission_ID[i];
      ID_TO_LABEL_MAP::iterator label_iter=mission_label_map.find(
         curr_mission_ID);
      string curr_mission_label=label_iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_mission_ID)+",\""
         +curr_mission_label+"\" ]";
      if (i < n_missions-1)
      {
         json_string += ",";
      }
   } // loop over inde xi labeling mission ID in chronological order
   json_string += " ], \n";

// Write mission_ID vs platform_ID to output JSON string:

   json_string += " \"Mission_ID_vs_Platform_ID\": [ ";   
   int mission_counter=0;
   for (ONE_ID_TO_ONE_ID_MAP::iterator iter=mission_platform_IDs_map.begin();
        iter != mission_platform_IDs_map.end(); ++iter)
   {
      int curr_mission_ID=iter->first;
      int curr_platform_ID=iter->second;
      cout << "Mission ID = " << curr_mission_ID 
           << " Platform ID = " << curr_platform_ID << endl;

      json_string += " [ "
         +stringfunc::number_to_string(curr_mission_ID)+","
         +stringfunc::number_to_string(curr_platform_ID);
      json_string += " ] ";
      if (mission_counter < n_missions-1)
      {
         json_string += ",";
      }
      mission_counter++;
   }
   json_string += " ], \n";
      
// Write platform_ID and platform_label to output JSON string:

   json_string += " \"Platform_ID_label\": [ ";   
   int platform_counter=0;
   int n_platforms=platform_label_map.size();
   for (ID_TO_LABEL_MAP::iterator iter=platform_label_map.begin();
        iter != platform_label_map.end(); ++iter)
   {
      int curr_platform_ID=iter->first;
      string curr_platform_label=iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_platform_ID)+","+
         "\""+curr_platform_label+"\" ] ";

      if (platform_counter < n_platforms-1)
      {
         json_string += ",";
      }
      platform_counter++;
   }
   json_string += " ] \n";

   json_string += "} \n";

   cout << "Final update_selector_metadata json_string = " 
        << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function update_fieldtest_dropdown()

QByteArray DataloaderServer::update_fieldtest_dropdown()
{
   cout << "inside DataloaderServer::update_fieldtest_dropdown()" << endl;

   vector<string> fieldtest_label;
   vector<int> fieldtest_ID;
   mover_func::retrieve_fieldtest_metadata_from_database(
      gis_database_ptr,fieldtest_label,fieldtest_ID);
   
   string json_string = "{ ";

// Write fieldtest_IDs and fieldtest_labels to output JSON string:

   json_string += " \"Fieldtest_IDs_labels\": [ ";   
   int n_fieldtests=fieldtest_ID.size();
   for (int f=0; f<n_fieldtests; f++)
   {
      json_string += " [ "
         +stringfunc::number_to_string(fieldtest_ID[f])+",\""
         +fieldtest_label[f]+"\" ]";
      if (f < n_fieldtests-1)
      {
         json_string += ",";
      }
   }
   json_string += " ] \n";

   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function update_mission_dropdown()

QByteArray DataloaderServer::update_mission_dropdown()
{
   cout << "inside DataloaderServer::update_mission_dropdown()" << endl;

   bool just_mission_ID_flag=false;
   vector<string> mission_label,fieldtest_label,platform_label;
   vector<int> mission_ID,fieldtest_ID,platform_ID;
   mover_func::retrieve_mission_metadata_from_database(
      gis_database_ptr,mission_label,mission_ID,
      fieldtest_label,fieldtest_ID,platform_label,platform_ID,
      just_mission_ID_flag);
   
   string json_string = "{ ";

// Write mission_IDs, mission_labels, fieldtest_labels and
// platform_labels to output JSON string:

   json_string += " \"Mission_IDs_labels\": [ ";   
   int n_missions=mission_ID.size();
   for (int f=0; f<n_missions; f++)
   {
      json_string += " [ "
         +stringfunc::number_to_string(mission_ID[f])+",\""
         +mission_label[f]+"\" , \""
         +fieldtest_label[f]+"\" , \""
         +platform_label[f]+"\" ]";
      if (f < n_missions-1)
      {
         json_string += ",";
      }
   }
   json_string += " ] \n";

   json_string += "} \n";

   cout << "*************************************************" << endl;
   cout << "Final json_string = " << json_string << endl;
   cout << "*************************************************" << endl;
   
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function update_platform_dropdown()

QByteArray DataloaderServer::update_platform_dropdown()
{
   cout << "inside DataloaderServer::update_platform_dropdown()" << endl;

   vector<string> platform_label;
   vector<int> platform_ID;
   mover_func::retrieve_platform_metadata_from_database(
      gis_database_ptr,platform_label,platform_ID);
   
   string json_string = "{ ";

// Write platform_IDs and platform_labels to output JSON string:

   json_string += " \"Platform_IDs_labels\": [ ";   
   int n_platforms=platform_ID.size();
   for (int f=0; f<n_platforms; f++)
   {
      json_string += " [ "
         +stringfunc::number_to_string(platform_ID[f])+",\""
         +platform_label[f]+"\" ]";
      if (f < n_platforms-1)
      {
         json_string += ",";
      }
   }
   json_string += " ] \n";

   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());

}

// ---------------------------------------------------------------------
// Member function update_sensor_dropdown()

QByteArray DataloaderServer::update_sensor_dropdown()
{
   cout << "inside DataloaderServer::update_sensor_dropdown()" << endl;

   if (sensor_labels.size()==0)
   {
      mover_func::retrieve_sensor_metadata_from_database(
         gis_database_ptr,sensor_labels,sensor_IDs);
   }

// Write sensor_IDs and sensor_labels to output JSON string:

   string json_string = "{ \n";
   json_string += " \"Sensor_IDs_labels\": [ ";   
   int n_sensors=sensor_IDs.size();
   for (int f=0; f<n_sensors; f++)
   {
      json_string += " [ "
         +stringfunc::number_to_string(sensor_IDs[f])+",\""
         +sensor_labels[f]+"\" ]";
      if (f < n_sensors-1)
      {
         json_string += ",";
      }
   }
   json_string += " ] \n";

   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Metadata parameter manipulation member functions
// ==========================================================================

// Member function set_fieldtest_params()

QByteArray DataloaderServer::set_fieldtest_params()
{
   cout << "inside DataloaderServer::set_fieldtest_params()" << endl;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];
       
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="FieldtestSelectorID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
   }

   cout << "selected_fieldtest_ID = " << selected_fieldtest_ID << endl;
   string selected_fieldtest_date=mover_func::get_fieldtest_date(
      selected_fieldtest_ID,gis_database_ptr);

   string json_string = "{ ";
   json_string += " \"Fieldtest_Date\": ";   
   json_string += " \""+selected_fieldtest_date+"\" \n";
   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
QByteArray DataloaderServer::set_mission_params()
{
   cout << "inside DataloaderServer::set_mission_params()" << endl;

   int curr_fieldtest_ID,curr_platform_ID;
   int curr_SDcard_ID=-1;
   int curr_notebook_ID=-1;
   string start_mission_time,stop_mission_time;
   string pilot_name,copilot_name,courier_name;
   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];
       
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;
      if (key=="StartMissionTime")
      {
         start_mission_time=value;
      }
      else if (key=="FieldtestDropdownID")
      {
         curr_fieldtest_ID=stringfunc::string_to_number(value);
      }
      else if (key=="PlatformDropdownID")
      {
         curr_platform_ID=stringfunc::string_to_number(value);
      }
      else if (key=="SDcardID")
      {
         if (value.size() > 0)
            curr_SDcard_ID=stringfunc::string_to_number(value);
      }
      else if (key=="PilotName")
      {
         if (value.size() > 0) pilot_name=value;
      }
      else if (key=="CopilotName")
      {
         if (value.size() > 0) copilot_name=value;
      }
      else if (key=="CourierName")
      {
         if (value.size() > 0) courier_name=value;
      }
   }

   cout << "curr_platform_ID = " << curr_platform_ID << endl;
   cout << "curr_SDcard_ID = " << curr_SDcard_ID << endl;
   cout << "pilot name = " << pilot_name << endl;
   cout << "copilot name = " << copilot_name << endl;
   cout << "courier name = " << courier_name << endl;

// Retrieve fieldtest label and simple date corresponding to input
// fieldtest ID:

   vector<int> fieldtest_IDs;
   vector<string> fieldtest_labels;
   
   bool weekday_mon_day_flag=false;
   mover_func::retrieve_fieldtest_metadata_from_database(
      gis_database_ptr,fieldtest_labels,fieldtest_IDs,
      weekday_mon_day_flag);
   int index=-1;
   for (int j=0; j<fieldtest_IDs.size(); j++)
   {
      if (fieldtest_IDs[j]==curr_fieldtest_ID)
      {
         index=j;
      }
   }
   string curr_fieldtest_label=fieldtest_labels[index];
   cout << "curr_fieldtest_label = " << curr_fieldtest_label << endl;
   vector<string> substrings=
      stringfunc::decompose_string_into_substrings(curr_fieldtest_label);
   string curr_fieldtest_date=substrings[0];
   cout << "curr_fieldtest_date = " << curr_fieldtest_date << endl;

   start_mission_time=curr_fieldtest_date+" "+start_mission_time;
   stop_mission_time=curr_fieldtest_date+" "+stop_mission_time;
//   cout << "start_mission_time = " << start_mission_time << endl;
//   cout << "stop_mission_time = " << stop_mission_time << endl;
//   cout << "fieldtest_ID = " << curr_fieldtest_ID << endl;

   vector<int> platform_IDs;
   vector<string> platform_labels;
   mover_func::retrieve_platform_metadata_from_database(
      gis_database_ptr,platform_labels,platform_IDs);
   
// Retrieve platform label corresponding to input platform_ID:

   index=-1;
   for (int j=0; j<platform_IDs.size(); j++)
   {
      if (platform_IDs[j]==curr_platform_ID)
      {
         index=j;
      }
   }
   string curr_platform_label=platform_labels[index];
   cout << "curr_platform_label = " << curr_platform_label << endl;

   databasefunc::insert_mission(
      gis_database_ptr,start_mission_time,start_mission_time,
      curr_fieldtest_ID,curr_platform_label,curr_platform_ID,
      curr_SDcard_ID,pilot_name,copilot_name,courier_name);

// Retrieve mission_ID corresponding to newly generated mission:

   vector<int> mission_IDs;
   mover_func::retrieve_mission_metadata_from_database(
      gis_database_ptr,mission_IDs);
   int new_mission_ID=mission_IDs.back();

   return generate_JSON_response_to_new_mission_entry(new_mission_ID);
}

// ---------------------------------------------------------------------
QByteArray DataloaderServer::generate_JSON_response_to_new_mission_entry(
   int new_mission_ID)
{
   cout << "inside DataloaderServer::generate_JSON_response_to_new_mission_entry()" << endl;

   string json_string = "{ ";
   json_string += " \"New_Mission_ID\": ";   
   json_string += stringfunc::number_to_string(new_mission_ID)+" \n";
   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// New data selection member functions
// ==========================================================================

// Member function pick_mission()

void DataloaderServer::pick_mission()
{
   cout << "inside DataloaderServer::pick_mission()" << endl;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

//      cout << "k = " << k 
//           << " Key = " << key
//           << " Value = " << value << endl;

      if (key=="FieldtestDropdownID" || key=="FieldtestSelectorID" ||
	      key=="FieldtestCalibratorID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
      else if (key=="MissionDropdownID" || key=="MissionSelectorID" ||
	      key=="MissionCalibratorID")
      {
         selected_mission_ID=stringfunc::string_to_number(value);
      }
      else if (key=="PlatformDropdownID" || key=="PlatformSelectorID" ||
	      key=="PlatformCalibratorID")
      {
         selected_platform_ID=stringfunc::string_to_number(value);
      }
      else if (key=="SensorDropdownID" || key=="SensorSelectorID" ||
	      key=="SensorCalibratorID")
      {
         selected_sensor_ID=stringfunc::string_to_number(value);
      }
   }

   cout << "selected_fieldtest_ID = " << selected_fieldtest_ID << endl;
   cout << "selected_mission_ID = " << selected_mission_ID << endl;
   cout << "selected_platform_ID = " << selected_platform_ID << endl;
   cout << "selected_sensor_ID = " << selected_sensor_ID << endl;
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_picked_mission()

QByteArray DataloaderServer::generate_JSON_response_to_picked_mission()
{
   cout << "inside Dataloaderserver::generate_JSON_response_to_picked_mission()"
        << endl;

   string json_string = "{ \"FieldtestID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_fieldtest_ID)
      +"\" , \n";
   json_string += "\"MissionID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_mission_ID)
      +"\" , \n";
   json_string += "\"PlatformID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_platform_ID)
      +"\" , \n";
   json_string += "\"SensorID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_sensor_ID)
      +"\" , \n";

   string selected_fieldtest_date=mover_func::get_fieldtest_date(
      selected_fieldtest_ID,gis_database_ptr);
   json_string += " \"FieldtestDate\": ";   
   json_string += " \""+selected_fieldtest_date+"\" , \n";

   string selected_sensor_label=retrieve_sensor_label(selected_sensor_ID);
   json_string += " \"SensorLabel\": ";   
   json_string += " \""+selected_sensor_label+"\" } \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_data_file()

QByteArray DataloaderServer::select_data_file()
{
   cout << "inside DataloaderServer::select_data_file()" << endl;

   data_filename=select_file_via_GUI();
   cout << "data_filename = " << data_filename << endl;

   string json_string = "{ ";
   json_string += " \"Data_filename\": \"";   
   json_string += data_filename+"\" \n";
   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_file_via_GUI() instantiates and pops open a
// Qt dialog window which allows a user to easily select a GPS track
// file to import.  The full path for the selected filename is
// returned by this method.

string DataloaderServer::select_file_via_GUI()
{
   cout << "inside DataloaderServer::select_file_via_GUI()" << endl;
   cout << "selected_sensor_ID = " << selected_sensor_ID << endl;

   SensorType curr_sensor_type=get_sensor_type(selected_sensor_ID);

   if (curr_sensor_type==GPS || curr_sensor_type==QUADGPS ||
       curr_sensor_type==DROIDGPS || curr_sensor_type==GARMINGPS)
   {
      return select_GPS_track_file_via_GUI();
   }
   else if (curr_sensor_type==MIDG)
   {
      return select_MIDG_track_file_via_GUI();
   }
   else if (curr_sensor_type==IMAGE || curr_sensor_type==DROIDIMAGE ||
	    curr_sensor_type==AXISIMAGE || curr_sensor_type==GPSCAMIMAGE)
   {
      return select_photo_file_via_GUI();
   }
}

// ---------------------------------------------------------------------
string DataloaderServer::select_GPS_track_file_via_GUI()
{
   
// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

   QWidget* window_ptr=new QWidget;
   window_ptr->move(835,0);
//   cout << "window_ptr->x() = " << window_ptr->x() << endl;
//   cout << "window_ptr->y() = " << window_ptr->y() << endl;
   window_ptr->setWindowTitle("GPS track data picker");

//   string starting_tracks_subdir="/data/tech_challenge/GPS_tracks/";
   string starting_tracks_subdir="/data/tech_challenge/field_tests/";
   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Select GPS Track File", starting_tracks_subdir.c_str(), 
   "Data Files (*.dat *.txt *.csv *.kml)");
   string GPS_track_filename=fileName.toStdString();
   cout << "Selected GPS track filename = " << GPS_track_filename << endl;
   return GPS_track_filename;
}

// ---------------------------------------------------------------------
string DataloaderServer::select_MIDG_track_file_via_GUI()
{
   cout << "inside DataloaderServer::select_MIDG_track_file_via_GUI()"
        << endl;

// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

   QWidget* window_ptr=new QWidget;
   window_ptr->move(835,0);
//   cout << "window_ptr->x() = " << window_ptr->x() << endl;
//   cout << "window_ptr->y() = " << window_ptr->y() << endl;
   window_ptr->setWindowTitle("MIDG data picker");

   string starting_image_subdir="/data/tech_challenge/field_tests/";
   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Select MIDG Data File", starting_image_subdir.c_str(), 
   "Data Files (*.midg)");
   string MIDG_filename=fileName.toStdString();
   cout << "Selected MIDG filename = " << MIDG_filename << endl;
   return MIDG_filename;
}

// ---------------------------------------------------------------------
string DataloaderServer::select_photo_file_via_GUI()
{
   QWidget* window_ptr=new QWidget;
   window_ptr->move(835,0);
//   cout << "window_ptr->x() = " << window_ptr->x() << endl;
//   cout << "window_ptr->y() = " << window_ptr->y() << endl;
   window_ptr->setWindowTitle("Photo picker");

   string starting_image_subdir="/data/tech_challenge/field_tests/";
   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Select Photo File", starting_image_subdir.c_str(), 
   "Photo Files (*.png *.PNG *.jpg *.JPG *.jpeg *.JPEG)");
   string photo_filename=fileName.toStdString();
   cout << "Selected photo filename = " << photo_filename << endl;
   return photo_filename;
}

// ==========================================================================
// New data upload member functions
// ==========================================================================

// Member function retrieve_sensor_label() takes in the ID for some
// sensor.  It performs a brute force search over all sensor labels
// for the counterpart to the input sensor ID.

string DataloaderServer::retrieve_sensor_label(int curr_sensor_ID)
{
//   cout << "inside DataloaderServer::retrieve_sensor_label()" << endl;

   if (sensor_labels.size()==0)
   {
      mover_func::retrieve_sensor_metadata_from_database(
         gis_database_ptr,sensor_labels,sensor_IDs);
   }

   string selected_sensor_label;
   for (int i=0; i<sensor_IDs.size(); i++)
   {
      if (sensor_IDs[i]==selected_sensor_ID)
      {
         selected_sensor_label=sensor_labels[i];
      }
   }

   return selected_sensor_label;
}

// ---------------------------------------------------------------------
// Member function upload_data()

QByteArray DataloaderServer::upload_data()
{
   cout << "inside DataloaderServer::upload_data()" << endl;
   bool data_uploaded_flag=false;

   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_clear_progress(progress_type);

   SensorType curr_sensor_type=get_sensor_type(selected_sensor_ID);
//   cout << "curr_sensor_type = " << curr_sensor_type << endl;
//   cout << "AXISIMAGE = " << AXISIMAGE << endl;

   if (curr_sensor_type==GPS)
   {
      data_uploaded_flag=insert_GPS_track_into_database();
   }
   else if (curr_sensor_type==MIDG)
   {
      data_uploaded_flag=insert_MIDG_track_into_database();
   }
   else if (curr_sensor_type==QUADGPS)
   {
      data_uploaded_flag=insert_QuadGPS_track_into_database();
   }
   else if (curr_sensor_type==DROIDGPS)
   {
      data_uploaded_flag=insert_DroidGPS_track_into_database();
   }
   else if (curr_sensor_type==GARMINGPS)
   {
      data_uploaded_flag=insert_GarminGPS_track_into_database();
   }
   else if (curr_sensor_type==IMAGE || curr_sensor_type==DROIDIMAGE ||
            curr_sensor_type==AXISIMAGE || curr_sensor_type==GPSCAMIMAGE)
   {
      data_uploaded_flag=insert_photos_into_database();
   }
   else
   {
      string banner="Upload data type is UNKNOWN!!!";
      outputfunc::write_big_banner(banner);
   }
   cout << "data_uploaded_flag = " << data_uploaded_flag << endl;

   viewer_messenger_ptr->broadcast_finished_progress(progress_type);

// Return JSON string confirming upload of new data labeled by
// current mission and current sensor IDs into database:

   string json_string = "{ ";
   json_string += " \"data_uploaded_flag\": ";   
   json_string += stringfunc::number_to_string(int(data_uploaded_flag))+",\n";
   json_string += " \"Mission_ID\": ";   
   json_string += stringfunc::number_to_string(selected_mission_ID)+",\n";
   json_string += " \"Sensor_ID\": ";   
   json_string += stringfunc::number_to_string(selected_sensor_ID)+", \n";

   string selected_sensor_label=retrieve_sensor_label(selected_sensor_ID);
   json_string += " \"Sensor_Label\": \""+selected_sensor_label+"\" \n";
   json_string += "} \n";
   
//   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function insert_GPS_track_into_database()

bool DataloaderServer::insert_GPS_track_into_database()
{
   cout << "inside DataloaderServer::insert_GPS_track_into_database()" << endl;

   double dataupload_progress=0.05;
   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   track* gps_track_ptr=gps_tracksgroup_ptr->generate_new_track(
      selected_mission_ID);

   dataupload_progress=0.20;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::parse_GPS_logfile(
      data_filename,*clock_ptr,gps_track_ptr);
   cout << "gps_track_ptr->size() = " << gps_track_ptr->size() << endl;

   dataupload_progress=0.50;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::insert_track_points(
      gis_database_ptr,gps_track_ptr,
      selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   return true;
}

// ---------------------------------------------------------------------
// Member function insert_MIDG_track_into_database()

bool DataloaderServer::insert_MIDG_track_into_database()
{
   cout << "inside DataloaderServer::insert_MIDG_track_into_database()" 
        << endl;

   double dataupload_progress=0.05;
   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   track* MIDG_track_ptr=gps_tracksgroup_ptr->generate_new_track(
      selected_mission_ID);
   mover_func::parse_insparse_output(data_filename,*clock_ptr,MIDG_track_ptr);
   cout << "MIDG_track_ptr->size() = " << MIDG_track_ptr->size() << endl;

   dataupload_progress=0.20;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::insert_track_points(
      gis_database_ptr,MIDG_track_ptr,
      selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   return true;
}

// ---------------------------------------------------------------------
// Member function insert_QuadGPS_track_into_database()

bool DataloaderServer::insert_QuadGPS_track_into_database()
{
   cout << "inside DataloaderServer::insert_QuadGPS_track_into_database()" 
        << endl;

   double dataupload_progress=0.02;
   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   track* Quad_track_ptr=gps_tracksgroup_ptr->generate_new_track(
      selected_mission_ID);
   mover_func::parse_QuadGPS_logfile(data_filename,*clock_ptr,Quad_track_ptr);
   cout << "Quad_track_ptr->size() = " << Quad_track_ptr->size() << endl;

   dataupload_progress=0.40;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::insert_track_points(
      gis_database_ptr,Quad_track_ptr,
      selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   return true;
}

// ---------------------------------------------------------------------
// Member function insert_DroidGPS_track_into_database()

bool DataloaderServer::insert_DroidGPS_track_into_database()
{
   cout << "inside DataloaderServer::insert_DroidGPS_track_into_database()" 
        << endl;

   double dataupload_progress=0.02;
   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   track* Droid_track_ptr=gps_tracksgroup_ptr->generate_new_track(
      selected_mission_ID);

   dataupload_progress=0.20;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::parse_DroidGPS_logfile(
	data_filename,*clock_ptr,Droid_track_ptr);
   cout << "Droid_track_ptr->size() = " << Droid_track_ptr->size() << endl;

   dataupload_progress=0.50;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::insert_track_points(
      gis_database_ptr,Droid_track_ptr,
      selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   cout << "At end of DataloaderServer::insert_DroidGPS_track_into_database()" 
        << endl;

   return true;
}

// ---------------------------------------------------------------------
// Member function insert_GarminGPS_track_into_database()

bool DataloaderServer::insert_GarminGPS_track_into_database()
{
   cout << "inside DataloaderServer::insert_GarminGPS_track_into_database()" 
        << endl;

   double dataupload_progress=0.02;
   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   track* Garmin_track_ptr=gps_tracksgroup_ptr->generate_new_track(
      selected_mission_ID);

   dataupload_progress=0.20;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::parse_GarminGPS_kmlfile(
	data_filename,*clock_ptr,Garmin_track_ptr);
   cout << "Garmin_track_ptr->size() = " << Garmin_track_ptr->size() << endl;

   dataupload_progress=0.50;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   mover_func::insert_track_points(
      gis_database_ptr,Garmin_track_ptr,
      selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   return true;
}

// ---------------------------------------------------------------------
// Member function insert_photos_into_database()

bool DataloaderServer::insert_photos_into_database()
{
   cout << "inside DataloaderServer::insert_photos_into_database()" << endl;

   string photo_subdir=filefunc::getdirname(data_filename);
//   cout << "photo_subdir = " << photo_subdir << endl;

   double dataupload_progress=0.02;
   string progress_type="data_uploading";
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   vector<string> photo_suffixes;
   photo_suffixes.push_back("jpg");
   photo_suffixes.push_back("JPG");
   photo_suffixes.push_back("jpeg");
   photo_suffixes.push_back("JPEG");
   photo_suffixes.push_back("png");
   photo_suffixes.push_back("PNG");
   
   vector<string> photo_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         photo_suffixes,photo_subdir);

   dataupload_progress=0.05;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   vector<int> xdim,ydim;
   vector<double> secs_elapsed;
   vector<string> calibrated_photo_filenames;
   vector<geopoint> geolocations;

   SensorType curr_sensor_type=get_sensor_type(selected_sensor_ID);
   int good_image_counter=0;
   for (int i=0; i<photo_filenames.size(); i++)
   {
//      cout << "DataloaderServer::insert_photos(), i = " << i 
//           << " photo_filename = " << photo_filenames[i] << endl;
    
      photograph curr_photo(photo_filenames[i]);
//      cout << curr_photo << endl;

//      cout << "curr_sensor_type = " << curr_sensor_type << endl;

      double elapsed_secs;
      if (curr_sensor_type==DROIDIMAGE)
      {
         elapsed_secs=photodbfunc::extract_droid_image_time_from_metadata_file(
            photo_filenames[i],curr_photo.get_clock());
      }
      else if (curr_sensor_type==GPSCAMIMAGE)
      {
         elapsed_secs=curr_photo.get_clock().
            secs_elapsed_since_reference_date();
         geolocations.push_back(curr_photo.get_geolocation());
      }
      else
      {
         elapsed_secs=curr_photo.get_clock().
            secs_elapsed_since_reference_date();
      }
      if (elapsed_secs < 0) elapsed_secs=0;

      string curr_image_filename=photo_filenames[i];
      if (curr_sensor_type==DROIDIMAGE || curr_sensor_type==AXISIMAGE ||
          curr_sensor_type==GPSCAMIMAGE)
      {

// Add image counter to end of droid image filenames:

         string new_image_filename=
            photodbfunc::append_counter_to_image_filenames(
               curr_image_filename,good_image_counter);
         calibrated_photo_filenames.push_back(new_image_filename);
      }
      else
      {
         calibrated_photo_filenames.push_back(curr_image_filename);
      }
      
      secs_elapsed.push_back(elapsed_secs);
      xdim.push_back(curr_photo.get_xdim());
      ydim.push_back(curr_photo.get_ydim());

//      cout.precision(12);
//      cout << "secs = " << secs_elapsed.back()
//           << " xdim = " << xdim.back() << " ydim = " << ydim.back()
//           << endl;
   } // loop over index i labeling input photos

//   cout << "calibrated_photo_filenames.size() = "
//        << calibrated_photo_filenames.size() << endl;

   dataupload_progress=0.10;
   viewer_messenger_ptr->broadcast_progress(
      dataupload_progress,progress_type);

   bool genuine_timestamp_flag=false;
   bool genuine_geolocation_flag=false;
   if (curr_sensor_type==DROIDIMAGE)
   {
      genuine_timestamp_flag=true;
   }
   else if (curr_sensor_type==GPSCAMIMAGE)
   {
      genuine_timestamp_flag=genuine_geolocation_flag=true;
      track* GPScam_track_ptr=gps_tracksgroup_ptr->generate_new_track(
         selected_mission_ID);
      string output_subdir=filefunc::getdirname(calibrated_photo_filenames[0]);

      mover_func::generate_GPScamera_track(
         secs_elapsed,geolocations,*clock_ptr,output_subdir,GPScam_track_ptr);

      cout << "GPScam_track_ptr->size() = " << GPScam_track_ptr->size() 
           << endl;

      mover_func::insert_track_points(
         gis_database_ptr,GPScam_track_ptr,
         selected_fieldtest_ID,selected_mission_ID,
         selected_platform_ID,selected_sensor_ID);
   }
   
   bool insert_flag=photodbfunc::insert_photo_metadata_into_database(
      gis_database_ptr,selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID,
      viewer_messenger_ptr,progress_type,
      genuine_timestamp_flag,secs_elapsed,
      genuine_geolocation_flag,geolocations,
      calibrated_photo_filenames,xdim,ydim);

   return insert_flag;
}

// ---------------------------------------------------------------------
// Member function update_thumbnail_metadata_in_database()

QByteArray DataloaderServer::update_thumbnail_metadata_in_database()
{
   cout << "inside DataloaderServer::update_thumbnail_metadata_in_database()" 
        << endl;

   string photo_subdir=filefunc::getdirname(data_filename);
   cout << "photo_subdir = " << photo_subdir << endl;
   string thumbnails_subdir=photo_subdir+"thumbnails/";
   cout << "thumbnails_subdir = " << thumbnails_subdir << endl;

   vector<string> photo_suffixes;
   photo_suffixes.push_back("jpg");
   photo_suffixes.push_back("JPG");
   photo_suffixes.push_back("jpeg");
   photo_suffixes.push_back("JPEG");
   photo_suffixes.push_back("png");
   photo_suffixes.push_back("PNG");
   
   vector<string> thumbnail_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         photo_suffixes,thumbnails_subdir);

   cout << "thumbnail_filenames.size() = " << thumbnail_filenames.size()
        << endl;

   vector<int> photo_IDs,xdim,ydim;
   vector<string> calibrated_thumbnail_filenames;

   photodbfunc::retrieve_photo_IDs_from_database(
      gis_database_ptr,selected_mission_ID,photo_IDs);

   SensorType curr_sensor_type=get_sensor_type(selected_sensor_ID);
   int good_image_counter=0;
   for (int i=0; i<thumbnail_filenames.size(); i++)
   {
//      cout << "curr_sensor_type = " << curr_sensor_type << endl;
      string curr_image_filename=thumbnail_filenames[i];
      if (curr_sensor_type==DROIDIMAGE || curr_sensor_type==AXISIMAGE ||
          curr_sensor_type==GPSCAMIMAGE)
      {

// Add image counter to end of droid image filenames:

         string new_image_filename=
            photodbfunc::append_counter_to_image_filenames(
               curr_image_filename,good_image_counter);
         calibrated_thumbnail_filenames.push_back(new_image_filename);
      }
      else
      {
         calibrated_thumbnail_filenames.push_back(curr_image_filename);
      }

      int width,height;
      imagefunc::get_image_width_height(curr_image_filename,width,height);
      xdim.push_back(width);
      ydim.push_back(height);

//      cout.precision(12);
//      cout << "secs = " << secs_elapsed.back()
//           << " xdim = " << xdim.back() << " ydim = " << ydim.back()
//           << endl;
   } // loop over index i labeling input photos

//   cout << "calibrated_thumbnail_filenames.size() = "
//        << calibrated_thumbnail_filenames.size() << endl;

   bool update_flag=photodbfunc::update_thumbnail_metadata_in_database(
      gis_database_ptr,photo_IDs,calibrated_thumbnail_filenames,xdim,ydim);

// Return JSON string confirming upload of thumbnails labeled by
// current mission and current sensor IDs into database:

   string json_string = "{ ";
   json_string += " \"data_uploaded_flag\": ";   
   json_string += stringfunc::number_to_string(int(update_flag))+",\n";
   json_string += " \"Mission_ID\": ";   
   json_string += stringfunc::number_to_string(selected_mission_ID)+",\n";
   json_string += " \"Sensor_ID\": ";   
   json_string += stringfunc::number_to_string(selected_sensor_ID)+", \n";

   string selected_sensor_label=retrieve_sensor_label(selected_sensor_ID);
   json_string += " \"Sensor_Label\": \""+selected_sensor_label+"\" \n";
   json_string += "} \n";
   
//   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}
