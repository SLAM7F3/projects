// ==========================================================================
// MACHETESERVER class file
// ==========================================================================
// Last updated on 4/12/12; 4/25/12; 4/27/12
// ==========================================================================

#include <algorithm>
#include <iostream>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "graphs/cppJSON.h"
#include "graphs/jsonfuncs.h"
#include "Qt/web/LOSTClient.h"
#include "ladar/machetedbfuncs.h"
#include "Qt/web/MacheteServer.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void MacheteServer::allocate_member_objects()
{
   data_ids_map_ptr=new DATA_IDS_MAP;
}		       

void MacheteServer::initialize_member_objects()
{
   bbox_ptr=NULL;
   clock_ptr=NULL;
   postgis_database_ptr=NULL;
   LOSTClient_ptr=NULL;
   region_height=region_width=-1;
}

MacheteServer::MacheteServer(
   string host_IP_address,qint16 port, QObject* parent) :
   MessageServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
MacheteServer::~MacheteServer()
{
   delete data_ids_map_ptr;
   delete LOSTClient_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray MacheteServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside MacheteServer:get() method" << endl;

   Q_UNUSED(responseHeader);

   doc.appendChild( response );

   URL_path=url.path().toStdString();
   cout << "URL path = " << URL_path << endl;
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
   if (URL_path=="/Update_Messenger_Topic_Name/")
   {
      update_messenger_topic_name(response_msg);
   }
   else if (URL_path=="/Find_Mounted_Disks/")
   {
      return find_mounted_disks();
   }
   else if (URL_path=="/Select_Raw_Data_Files/")
   {
      return select_raw_data_files(response_msg);
   }
   else if (URL_path=="/Import_Raw_Data_Files/")
   {
      return import_raw_data_files(response_msg);
   }
   else if (URL_path=="/Find_Files_To_Process/")
   {
      return find_files_to_process();
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray MacheteServer::post(const QUrl& url, const QByteArray& postData,
                               QHttpResponseHeader& responseHeader)
{
//   cout << "inside MacheteServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   MacheteServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;
   if (URL_path=="/Set_Tour_Path/")
   {
   }

   return doc.toByteArray();
}

// ==========================================================================
// Data importing member functions
// ==========================================================================

// Member function find_mounted_disks()

QByteArray MacheteServer::find_mounted_disks()
{
   cout << "inside MacheteServer::find_mounted_disks()" << endl;
   
//   string Server_URL="172.25.191.241:4567";
   string Server_URL="gear:4567";
   string request="/disks";
   string JSON_response=Server_module_get_query(Server_URL,request);
   return parse_mounted_disks_JSON(JSON_response);
}

// ---------------------------------------------------------------------
// Member function parse_mounted_disks_JSON()

QByteArray MacheteServer::parse_mounted_disks_JSON(string& JSON_response)
{
//   cout << "inside MacheteServer::parse_mounted_disks_JSON()" << endl;
//   cout << "JSON_response = " << JSON_response << endl;

   cppJSON* cppJSON_ptr=new cppJSON();
   cJSON* root_ptr=cppJSON_ptr->parse_json(JSON_response);
   cppJSON_ptr->generate_JSON_tree();
   cppJSON_ptr->extract_key_value_pairs(root_ptr);

   vector<int> num_files;
   vector<string> paths,ids;

   int n_JSON_objects=cppJSON_ptr->get_n_objects();
//   cout << "Number JSON objects = " << n_JSON_objects << endl;
   for (int n=0; n<n_JSON_objects; n++)
   {
      vector<cppJSON::KEY_VALUE_PAIR> key_value_pairs=
         cppJSON_ptr->get_object_key_value_pairs(n);
      for (int k=0; k<key_value_pairs.size(); k++)
      {
         cppJSON::KEY_VALUE_PAIR curr_key_value_pair=key_value_pairs[k];
//         cout << "k = " << k 
//              << " key = " << curr_key_value_pair.first
//              << " value = " << curr_key_value_pair.second << endl;

         string key=curr_key_value_pair.first;
         string value=curr_key_value_pair.second;
         if (key=="num_files")
         {
            num_files.push_back(stringfunc::string_to_number(value));
         }
         else if (key=="path")
         {
            paths.push_back(value);
         }
         else if (key=="id")
         {
            ids.push_back(value);
         }
      } // loop over index k
   } // loop over index n 

   for (int i=0; i<num_files.size(); i++)
   {
      cout << "i = " << i
           << " num_files = " << num_files[i]
           << " path = " << paths[i]
           << " id = " << ids[i] << endl;
   }
   delete cppJSON_ptr;

   return QByteArray(JSON_response.c_str());
}

// ---------------------------------------------------------------------
// Member function select_raw_data_files()

QByteArray MacheteServer::select_raw_data_files(string& response_msg)
{
   cout << "inside MacheteServer::select_raw_data_files()" << endl;

   int n_args=KeyValue.size();
   string topic,disk_ID;
   for (int k=0; k<n_args; k++)
   {
      cout << "k = " << k
           << " KeyValue.first = " << KeyValue[k].first
           << " KeyValue.second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="DiskID")
      {
         disk_ID=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << " disk_ID = " << disk_ID << endl;
   cout << "topic = " << topic << endl;

   string Server_URL="gear:4567";
   string request="/disk/"+disk_ID;
   string JSON_response=Server_module_get_query(Server_URL,request);

   cout << "JSON_response from Dave's server = "
        << JSON_response << endl;

   return QByteArray(JSON_response.c_str());
}

/*
   string starting_image_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/falconI/data_files/";
   cout << "starting_image_subdir = " << starting_image_subdir
        << endl;
   
   string window_title="Select ladar data files";
   string file_types="Raw ladar files (*.raw)";
   vector<string> input_filenames=BasicServer::open_input_file_dialog(
      window_title,starting_image_subdir,file_types);

   int pass_ID=0;
   string file_type="raw";

   clock_ptr->set_time_based_on_local_computer_clock();
   double insertion_epoch=clock_ptr->secs_elapsed_since_reference_date();
   string insertion_UTC=clock_ptr->YYYY_MM_DD_H_M_S();
   for (int i=0; i<input_filenames.size(); i++)
   {
      string file_URL=input_filenames[i];
      machetedbfunc::insert_file_metadata(
         postgis_database_ptr,campaign_ID,sortie_ID,pass_ID,
         file_type,file_URL,insertion_epoch,insertion_UTC);
   }

// Broadcast data importing progress:

   string progress_type="DataImport";
   get_Messenger_ptr(topic)->broadcast_progress(0.02,progress_type);

   response_msg="";
   return true;
}
*/

// ---------------------------------------------------------------------
// Member function import_raw_data_files()

QByteArray MacheteServer::import_raw_data_files(string& response_msg)
{
   cout << "inside MacheteServer::import_raw_data_files()" << endl;

   int n_args=KeyValue.size();
   int campaign_ID,sortie_ID;
   string topic,files_to_import;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k
//           << " KeyValue.first = " << KeyValue[k].first
//           << " KeyValue.second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="Campaign")
      {
         campaign_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Sortie")
      {
         sortie_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="FilesToImport")
      {
         files_to_import=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << "campaign_ID = " << campaign_ID 
        << " sortie_ID = " << sortie_ID 
        << " files_to_import = " << files_to_import << endl;
   cout << "topic = " << topic << endl;

// As of 4/25/12, we assume files_to_import is a concatenated string
// of raw data filenames separated by commas.  This string ends with a
// terminal comma:

   string separator_chars=",";
   vector<string> import_filenames=
      stringfunc::decompose_string_into_substrings(
         files_to_import,separator_chars);

// Add entries into data_files table of MACHETE database for each
// imported raw ladar file:

   int pass_ID=0;
   string file_type="raw";

   clock_ptr->set_time_based_on_local_computer_clock();
   double insertion_epoch=clock_ptr->secs_elapsed_since_reference_date();
   string insertion_UTC=clock_ptr->YYYY_MM_DD_H_M_S();
   for (int i=0; i<import_filenames.size(); i++)
   {
      string file_URL=import_filenames[i];
      machetedbfunc::insert_file_metadata(
         postgis_database_ptr,campaign_ID,sortie_ID,pass_ID,
         file_type,file_URL,insertion_epoch,insertion_UTC);
   }

// Broadcast data importing progress:

   string progress_type="DataImport";
   get_Messenger_ptr(topic)->broadcast_progress(0.02,progress_type);

// Pass JSON array containing import filenames to importer module:

   string JSON_array="[ \n";
   for (int i=0; i<import_filenames.size(); i++)
   {
      JSON_array += "\""+import_filenames[i]+"\"";
      if (i < import_filenames.size()-1) JSON_array += ",";
   }
   JSON_array += " \n";
   JSON_array += "] \n";

   cout << "JSON_array = " << JSON_array << endl;

//   string Server_URL="gear:4567/import";
   string Server_URL="/import";
   string JSON_response=Server_module_post_query(Server_URL,JSON_array);

   cout << "JSON_response from Dave's server = "
        << JSON_response << endl;
   return QByteArray(JSON_response.c_str());
}

// ==========================================================================
// Data processing member functions
// ==========================================================================

// Member function find_files_to_process()

QByteArray MacheteServer::find_files_to_process()
{
   cout << "inside MacheteServer::find_files_to_process()" << endl;
   
   string Server_URL="gear:4567";
   string request="/files_to_process";
   string JSON_response=Server_module_get_query(Server_URL,request);
   cout << "JSON_response = " << JSON_response << endl;
   return QByteArray(JSON_response.c_str());
}

// ==========================================================================
// Get and post member functions
// ==========================================================================

// Member function Server_module_get_query()

string MacheteServer::Server_module_get_query(string Server_URL,string request)
{
   if (LOSTClient_ptr==NULL)
   {
      LOSTClient_ptr=new LOSTClient(Server_URL);
   }
//   cout << "LOSTClient_ptr = " << LOSTClient_ptr << endl;
   
   LOSTClient_ptr->issue_get_request(request);

   bool response_returned_flag=false;
   string JSON_response;
   while(!response_returned_flag)
   {

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the WebServer.  Here we force the main Qt loop to continue
// processing until the server's entire response to the get request
// has been received by the LOSTClient:

      qApp->processEvents();

      JSON_response=LOSTClient_ptr->get_returned_output();
      if (JSON_response.size() > 0)
      {
         response_returned_flag=true;
      }
//      cout << response_returned_flag << " " << flush;
   }
//   cout << "JSON_response = " << JSON_response << endl;   

   return JSON_response;
}

// ---------------------------------------------------------------------
// Member function Server_module_post_query()

string MacheteServer::Server_module_post_query(
   string Server_URL,string query_params)
{
   if (LOSTClient_ptr==NULL)
   {
      LOSTClient_ptr=new LOSTClient(Server_URL);
   }
//   cout << "LOSTClient_ptr = " << LOSTClient_ptr << endl;
   
   LOSTClient_ptr->issue_post_request(Server_URL,query_params);

   bool response_returned_flag=false;
   string JSON_response;
   while(!response_returned_flag)
   {

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the WebServer.  Here we force the main Qt loop to continue
// processing until the server's entire response to the get request
// has been received by the LOSTClient:

      qApp->processEvents();

      JSON_response=LOSTClient_ptr->get_returned_output();
      if (JSON_response.size() > 0)
      {
         response_returned_flag=true;
      }
//      cout << response_returned_flag << " " << flush;
   }
//   cout << "JSON_response = " << JSON_response << endl;   

   return JSON_response;
}
