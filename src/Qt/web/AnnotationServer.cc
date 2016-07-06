// ==========================================================================
// ANNOTATIONSERVER class file
// ==========================================================================
// Last updated on 9/30/10; 10/19/10; 1/18/11; 6/1/11
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

#include "Qt/web/AnnotationServer.h"
#include "postgres/databasefuncs.h"
#include "astro_geo/geopoint.h"
#include "geometry/homography.h"
#include "track/mover_funcs.h"
#include "templates/mytemplates.h"

#include "video/G99VideoDisplay.h"
#include "video/photoannotationdbfuncs.h"
#include "video/photodbfuncs.h"
#include "image/raster_parser.h"
#include "image/TwoDarray.h"
#include "video/videofuncs.h"


using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void AnnotationServer::allocate_member_objects()
{
}		       

void AnnotationServer::initialize_member_objects()
{
   clock.current_local_time_and_UTC();
   viewer_messenger_ptr=NULL;
   metadata_messenger_ptr=NULL;
   gis_database_ptr=NULL;
}

AnnotationServer::AnnotationServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
AnnotationServer::~AnnotationServer()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray AnnotationServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside AnnotationServer:get() method" << endl;

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

   if (URL_path=="/Get_World_Annotations/")
   {
      return get_world_annotations();
   }
   else if (URL_path=="/Insert_World_Annotation/")
   {
      insert_world_annotation();
   }
   else if (URL_path=="/Update_World_Annotation/")
   {
      update_world_annotation();
   }
   else if (URL_path=="/Delete_World_Annotation/")
   {
      delete_world_annotation();
   }

   else if (URL_path=="/Get_Photo_Annotations/")
   {
      return get_particular_photo_annotations();
   }
   else if (URL_path=="/Get_All_Photo_Annotations/")
   {
      return get_all_photo_annotations();
   }
   else if (URL_path=="/Insert_Photo_Annotation/")
   {
      insert_photo_annotation();
   }
   else if (URL_path=="/Update_Photo_Annotation/")
   {
      update_photo_annotation();
   }
   else if (URL_path=="/Delete_Photo_Annotation/")
   {
      delete_photo_annotation();
   }
   else if (URL_path=="/Get_Multimission_Photo_Annotations/")
   {
      return generate_JSON_for_multimission_photo_annotations();
   }
   else if (URL_path=="/Get_Photos_For_Mission/")
   {
      return get_photos_for_mission();
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray AnnotationServer::post(const QUrl& url, const QByteArray& postData,
                             QHttpResponseHeader& responseHeader)
{
//   cout << "inside AnnotationServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   AnnotationServer::get(doc,response,url,URL_path,responseHeader);

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
// Map annotation member functions
// ==========================================================================

// Member function get_world_annotations() generates and returns a
// JSON string containing metadata for all current entries within the
// world_annotations table of the TOC database.

QByteArray AnnotationServer::get_world_annotations()
{
//   cout << "inside AnnotationServer::get_world_annotations()" << endl;

   int n_args=KeyValue.size();
   int selected_fieldtest_ID=-1;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="FieldtestID")
      {
         selected_fieldtest_ID=
            stringfunc::string_to_number(KeyValue[k].second); 
      }
   }
   
   vector<int> annotation_IDs,importances;
   vector<string> creation_times,event_times,usernames,
      labels,descriptions,colors;
   vector<threevector> llas;

   databasefunc::get_all_world_map_annotations(
      gis_database_ptr,selected_fieldtest_ID,annotation_IDs,creation_times,
      event_times,usernames,labels,descriptions,colors,importances,llas);

   int n_annotations=annotation_IDs.size();

   string json_string = "{ \"n_annotations\": "+stringfunc::number_to_string(
      n_annotations);
   if (n_annotations > 0)
   {
      json_string += ",";
   }
   json_string += " \n";

   string key;
   for (int i=0; i<n_annotations; i++)
   {
      string istring=stringfunc::number_to_string(i);
      key="ID_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(annotation_IDs[i])+", \n";

      key="creation_time_"+istring;
      json_string += "\""+key+"\": "
         +"\""+creation_times[i]+"\" , \n";

      key="event_time_"+istring;
      json_string += "\""+key+"\": "
         +"\""+event_times[i]+"\" , \n";

      key="username_"+istring;
      json_string += "\""+key+"\": "
         +"\""+usernames[i]+"\" , \n";

      key="label_"+istring;
      json_string += "\""+key+"\": "
         +"\""+labels[i]+"\" , \n";

// FAKE FAKE:  Thurs Sep 30 at 9:04 am

// On 9/30/10, we learned the hard way that output JSON strings must
// not contain carriage returns if they are to be parsed correctly
// within the thin client.  So we explicitly replace any \n
// character with a space in the descriptions field:

      string cleaned_description=stringfunc::find_and_replace_char(
         descriptions[i],"\n"," ");

      key="description_"+istring;
      json_string += "\""+key+"\": "
         +"\""+cleaned_description+"\" , \n";

      key="color_"+istring;
      json_string += "\""+key+"\": "
         +"\""+colors[i]+"\" , \n";
      
      key="importance_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(importances[i])+", \n";

      key="longitude_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(llas[i].get(0),7)+", \n";

      key="latitude_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(llas[i].get(1),7)+", \n";

      key="altitude_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(llas[i].get(2));

      if (i < n_annotations-1) json_string += ",";
      json_string += "\n";
   } // loop over index i labeling world annotations
   
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function insert_world_annotation()

bool AnnotationServer::insert_world_annotation()
{
//   cout << "inside AnnotationServer::insert_world_annotation()" << endl;

   int n_args=KeyValue.size();
   int selected_fieldtest_ID,importance;
   string username,label,description,color;
   double longitude=0;
   double latitude=0;
   double altitude=0;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="FieldtestID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(
            KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="UserName")
      {
         username=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Label")
      {
         label=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Description")
      {
         description=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Color")
      {
         color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Importance")
      {
         importance=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="Longitude")
      {
         longitude=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Latitude")
      {
         latitude=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Altitude")
      {
         altitude=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   double secs_since_epoch=clock.secs_elapsed_since_reference_date();
//   cout << "secs_since_epoch = " << secs_since_epoch << endl;

   bool insert_flag=databasefunc::insert_world_annotation(
      gis_database_ptr,selected_fieldtest_ID,
      secs_since_epoch,username,label,description,color,importance,
      longitude,latitude,altitude);

   int curr_annotation_ID=databasefunc::get_world_annotation_ID(
      gis_database_ptr,username,label,description);
   
   if (viewer_messenger_ptr != NULL)
   {
      broadcast_world_annotation(
         curr_annotation_ID,username,label,description,color,importance,
         longitude,latitude,altitude);
   } 

   return insert_flag;
}

// ---------------------------------------------------------------------
// Member function update_world_annotation()

bool AnnotationServer::update_world_annotation()
{
//   cout << "inside AnnotationServer::update_world_annotation()" << endl;

   int curr_annotation_ID=-1;
   int n_args=KeyValue.size();
   int importance=-1;
   string username,label,description,color;
   double longitude=NEGATIVEINFINITY;
   double latitude=NEGATIVEINFINITY;
   double altitude=NEGATIVEINFINITY;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="WorldAnnotationID")
      {
         curr_annotation_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="UserName")
      {
         username=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Label")
      {
         label=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Description")
      {
         description=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Color")
      {
         color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Importance")
      {
         importance=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="Longitude")
      {
         longitude=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Latitude")
      {
         latitude=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Altitude")
      {
         altitude=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   double secs_since_epoch=clock.secs_elapsed_since_reference_date();
//   cout << "secs_since_epoch = " << secs_since_epoch << endl;

   bool update_flag=databasefunc::update_world_annotation(
      gis_database_ptr,curr_annotation_ID,
      secs_since_epoch,username,label,description,color,importance,
      longitude,latitude,altitude);

   if (viewer_messenger_ptr != NULL)
   {
      broadcast_world_annotation(
         curr_annotation_ID,username,label,description,color,importance,
         longitude,latitude,altitude);
   } 

   return update_flag;
}

// ---------------------------------------------------------------------
// Member function delete_world_annotation()

bool AnnotationServer::delete_world_annotation()
{
//   cout << "inside AnnotationServer::delete_world_annotation()" << endl;

   int annotation_ID=-1;
   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="WorldAnnotationID")
      {
         annotation_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   }

   bool flag=databasefunc::delete_world_annotation(gis_database_ptr,
      annotation_ID);

   if (viewer_messenger_ptr != NULL)
   {
      broadcast_world_annotation_deletion(annotation_ID);
   } 

   return flag;
}

// ---------------------------------------------------------------------
// Member function broadcast_world_annotation() emits an ActiveMQ
// message containing annotation metadata so that all thin clients can
// be updated.

void AnnotationServer::broadcast_world_annotation(
   int curr_annotation_ID,string username,string label,string description,
   string color,int importance,double longitude,double latitude,
   double altitude)
{
//   cout << "inside AnnotationServer::broadcast_world_annotation()" << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_WORLD_ANNOTATION";	

   key="ID";
   value=stringfunc::number_to_string(curr_annotation_ID);
   properties.push_back(Messenger::Property(key,value));

   key="Username";
   value=username;
   if (value.size() > 0)
      properties.push_back(Messenger::Property(key,value));

   key="Label";
   value=label;
   if (value.size() > 0)
      properties.push_back(Messenger::Property(key,value));

   key="Description";
   value=description;
   if (value.size() > 0)
      properties.push_back(Messenger::Property(key,value));

   key="Color";
   value=color;
   properties.push_back(Messenger::Property(key,value));

   key="Importance";
   value=stringfunc::number_to_string(importance);
   properties.push_back(Messenger::Property(key,value));

   key="Longitude";
   value=stringfunc::number_to_string(longitude,8);
   properties.push_back(Messenger::Property(key,value));

   key="Latitude";
   value=stringfunc::number_to_string(latitude,8);
   properties.push_back(Messenger::Property(key,value));

   key="Altitude";
   value=stringfunc::number_to_string(altitude);
   properties.push_back(Messenger::Property(key,value));

   viewer_messenger_ptr->broadcast_subpacket(command,properties);
}

// ---------------------------------------------------------------------
// Member function broadcast_world_annotation_deletion() emits an ActiveMQ
// message containing annotation deletion metadata so that all thin
// clients can be updated.

void AnnotationServer::broadcast_world_annotation_deletion(
   int curr_annotation_ID)
{
//   cout << "inside AnnotationServer::broadcast_annotation_deletion()" << endl;

   string command,key,value;
   vector<Messenger::Property> properties;

   command="DELETE_WORLD_ANNOTATION";	

   key="ID";
   value=stringfunc::number_to_string(curr_annotation_ID);
   properties.push_back(Messenger::Property(key,value));

   viewer_messenger_ptr->broadcast_subpacket(command,properties);
}

// ==========================================================================
// Photo annotation member functions
// ==========================================================================

// Member function get_particular_photo_annotations() generates and
// returns a JSON string containing metadata for the entries within
// the photo_annotations table of the TOC database for a particular
// photo.

QByteArray AnnotationServer::get_particular_photo_annotations()
{
//   cout << "inside AnnotationServer::get_particular_photo_annotations()" << endl;

   int n_args=KeyValue.size();
   int selected_photo_ID=-1;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="PhotoID")
      {
         selected_photo_ID=
            stringfunc::string_to_number(KeyValue[k].second); 
      }
   }

   vector<int> annotation_IDs,importances;
   vector<string> creation_times,usernames,labels,descriptions,colors;
   vector<twovector> UVs;

   photoannotationdbfunc::get_particular_photo_annotations(
      gis_database_ptr,selected_photo_ID,annotation_IDs,creation_times,
      usernames,labels,descriptions,colors,importances,UVs);

   int n_annotations=annotation_IDs.size();
   string json_string = "{ \"n_annotations\": "+stringfunc::number_to_string(
      n_annotations);
   if (n_annotations > 0)
   {
      json_string += ",";
   }
   json_string += " \n";

   string key;
   for (int i=0; i<n_annotations; i++)
   {
      string istring=stringfunc::number_to_string(i);
      key="ID_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(annotation_IDs[i])+", \n";

      key="creation_time_"+istring;
      json_string += "\""+key+"\": "
         +"\""+creation_times[i]+"\" , \n";

      key="username_"+istring;
      json_string += "\""+key+"\": "
         +"\""+usernames[i]+"\" , \n";

      key="label_"+istring;
      json_string += "\""+key+"\": "
         +"\""+labels[i]+"\" , \n";

      key="description_"+istring;
      json_string += "\""+key+"\": "
         +"\""+descriptions[i]+"\" , \n";

      key="color_"+istring;
      json_string += "\""+key+"\": "
         +"\""+colors[i]+"\" , \n";
      
      key="importance_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(importances[i])+", \n";

      key="U_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(UVs[i].get(0),7)+", \n";

      key="V_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(UVs[i].get(1),7);

      if (i < n_annotations-1) json_string += ",";
      json_string += "\n";
   } // loop over index i labeling world annotations
   
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function get_all_photo_annotations() generates and returns a
// JSON string containing metadata for all current entries within the
// photo_annotations table of the TOC database.

QByteArray AnnotationServer::get_all_photo_annotations()
{
//   cout << "inside AnnotationServer::get_all_photo_annotations()" << endl;

   int n_args=KeyValue.size();
   int selected_fieldtest_ID=-1;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="FieldtestID")
      {
         selected_fieldtest_ID=
            stringfunc::string_to_number(KeyValue[k].second); 
      }
   }

   vector<int> annotation_IDs,importances;
   vector<string> creation_times,usernames,labels,descriptions,colors;
   vector<twovector> UVs;

   photoannotationdbfunc::get_all_photo_annotations(
      gis_database_ptr,selected_fieldtest_ID,annotation_IDs,creation_times,
      usernames,labels,descriptions,colors,importances,UVs);

   int n_annotations=annotation_IDs.size();

   string json_string = "{ \"n_annotations\": "+stringfunc::number_to_string(
      n_annotations);
   if (n_annotations > 0)
   {
      json_string += ",";
   }
   json_string += " \n";

   string key;
   for (int i=0; i<n_annotations; i++)
   {
      string istring=stringfunc::number_to_string(i);
      key="ID_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(annotation_IDs[i])+", \n";

      key="creation_time_"+istring;
      json_string += "\""+key+"\": "
         +"\""+creation_times[i]+"\" , \n";

      key="username_"+istring;
      json_string += "\""+key+"\": "
         +"\""+usernames[i]+"\" , \n";

      key="label_"+istring;
      json_string += "\""+key+"\": "
         +"\""+labels[i]+"\" , \n";

      key="description_"+istring;
      json_string += "\""+key+"\": "
         +"\""+descriptions[i]+"\" , \n";

      key="color_"+istring;
      json_string += "\""+key+"\": "
         +"\""+colors[i]+"\" , \n";
      
      key="importance_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(importances[i])+", \n";

      key="U_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(UVs[i].get(0),7)+", \n";

      key="V_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(UVs[i].get(1),7);

      if (i < n_annotations-1) json_string += ",";
      json_string += "\n";
   } // loop over index i labeling world annotations
   
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
bool AnnotationServer::insert_photo_annotation()
{
   cout << "inside AnnotationServer::insert_photo_annotation()" << endl;

   int n_args=KeyValue.size();
   int fieldtest_ID,photo_ID,importance;
   string username,label,description,color;
   double U=0;
   double V=0;
   for (int k=0; k<n_args; k++)
   {
      cout << "k = " << k 
           << " KeyValue[k].first = " << KeyValue[k].first
           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="FieldtestID")
      {
         fieldtest_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         photo_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="UserName")
      {
         username=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Label")
      {
         label=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Description")
      {
         description=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Color")
      {
         color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Importance")
      {
         importance=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="U")
      {
         U=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="V")
      {
         V=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   int photo_counter,imp;
   string caption,photo_timestamp,photo_filename;
   photodbfunc::retrieve_particular_photo_metadata_from_database(
      gis_database_ptr,photo_ID,caption,photo_timestamp,photo_filename,
      photo_counter,imp);
   bool UTC_flag=true;

// Time associated with photo that is being annotated:

   double elapsed_secs=clock.timestamp_string_to_elapsed_secs(
      photo_timestamp,UTC_flag);

// Current time at which user inserted annotation:

   double secs_since_epoch=clock.secs_elapsed_since_reference_date();
//   cout << "secs_since_epoch = " << secs_since_epoch << endl;

   bool insert_flag=photoannotationdbfunc::insert_photo_annotation(
      gis_database_ptr,fieldtest_ID,photo_ID,
      elapsed_secs,
      username,label,description,color,importance,U,V);

   int curr_photo_annotation_ID=photoannotationdbfunc::get_photo_annotation_ID(
      gis_database_ptr,username,label,description);
   
   if (viewer_messenger_ptr != NULL)
   {
      broadcast_photo_annotation(
         curr_photo_annotation_ID,fieldtest_ID,photo_ID,username,label,
         description,color,importance,U,V);
   } 

   return insert_flag;
}

// ---------------------------------------------------------------------
// Member function update_photo_annotation()

bool AnnotationServer::update_photo_annotation()
{
//   cout << "inside AnnotationServer::update_photo_annotation()" << endl;

   int curr_annotation_ID=-1;
   int fieldtest_ID,photo_ID;
   int n_args=KeyValue.size();
   int importance=-1;
   string username,label,description,color;
   double U,V;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="PhotoAnnotationID")
      {
         curr_annotation_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="FieldtestID")
      {
         fieldtest_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         photo_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="UserName")
      {
         username=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Label")
      {
         label=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Description")
      {
         description=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Color")
      {
         color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Importance")
      {
         importance=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="U")
      {
         U=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="V")
      {
         V=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   double secs_since_epoch=clock.secs_elapsed_since_reference_date();
//   cout << "secs_since_epoch = " << secs_since_epoch << endl;

   bool update_flag=photoannotationdbfunc::update_photo_annotation(
      gis_database_ptr,curr_annotation_ID,
      secs_since_epoch,username,label,description,color,importance,U,V);

   if (viewer_messenger_ptr != NULL)
   {
      broadcast_photo_annotation(
         curr_annotation_ID,fieldtest_ID,photo_ID,
         username,label,description,color,importance,U,V);
   } 

   return update_flag;
}

// ---------------------------------------------------------------------
// Member function delete_photo_annotation()

bool AnnotationServer::delete_photo_annotation()
{
//   cout << "inside AnnotationServer::delete_photo_annotation()" << endl;

   int annotation_ID=-1;
   int photo_ID;
   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="PhotoAnnotationID")
      {
         annotation_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         photo_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   }

   bool flag=photoannotationdbfunc::delete_photo_annotation(gis_database_ptr,
      annotation_ID);

   if (viewer_messenger_ptr != NULL)
   {
      broadcast_photo_annotation_deletion(annotation_ID,photo_ID);
   } 

   return flag;
}

// ---------------------------------------------------------------------
// Member function broadcast_photo_annotation() emits an ActiveMQ
// message containing photo annotation metadata so that all thin
// clients can
// be updated.

void AnnotationServer::broadcast_photo_annotation(
   int curr_annotation_ID,int fieldtest_ID,int photo_ID,
   string username,string label,string description,
   string color,int importance,double U,double V)
{
//   cout << "inside AnnotationServer::broadcast_photo_annotation()" << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_PHOTO_ANNOTATION";	

   key="ID";
   value=stringfunc::number_to_string(curr_annotation_ID);
   properties.push_back(Messenger::Property(key,value));

   key="FieldtestID";
   value=stringfunc::number_to_string(fieldtest_ID);
   properties.push_back(Messenger::Property(key,value));

   key="PhotoID";
   value=stringfunc::number_to_string(photo_ID);
   properties.push_back(Messenger::Property(key,value));

   key="Username";
   value=username;
   if (value.size() > 0)
      properties.push_back(Messenger::Property(key,value));

   key="Label";
   value=label;
   if (value.size() > 0)
      properties.push_back(Messenger::Property(key,value));

   key="Description";
   value=description;
   if (value.size() > 0)
      properties.push_back(Messenger::Property(key,value));

   key="Color";
   value=color;
   properties.push_back(Messenger::Property(key,value));

   key="Importance";
   value=stringfunc::number_to_string(importance);
   properties.push_back(Messenger::Property(key,value));

   key="U";
   value=stringfunc::number_to_string(U,8);
   properties.push_back(Messenger::Property(key,value));

   key="V";
   value=stringfunc::number_to_string(V,8);
   properties.push_back(Messenger::Property(key,value));

   viewer_messenger_ptr->broadcast_subpacket(command,properties);
}

// ---------------------------------------------------------------------
// Member function broadcast_photo_annotation_deletion() emits an ActiveMQ
// message containing annotation deletion metadata so that all thin
// clients can be updated.

void AnnotationServer::broadcast_photo_annotation_deletion(
   int curr_annotation_ID,int photo_ID)
{
//   cout << "inside AnnotationServer::broadcast_photo_annotation_deletion()" << endl;

   string command,key,value;
   vector<Messenger::Property> properties;

   command="DELETE_PHOTO_ANNOTATION";	

   key="ID";
   value=stringfunc::number_to_string(curr_annotation_ID);
   properties.push_back(Messenger::Property(key,value));

   key="PhotoID";
   value=stringfunc::number_to_string(photo_ID);
   properties.push_back(Messenger::Property(key,value));

   viewer_messenger_ptr->broadcast_subpacket(command,properties);
}


// ---------------------------------------------------------------------
// Member function generate_JSON_for_multimission_photo_annotations()
// generates and returns a JSON string containing startFrame,
// stopFrame and importance triples which cover all frames within all
// missions for the currently selected fieldtest.  This triple
// information will be displayed as a SIMILE timeline by Diane Staheli.

QByteArray AnnotationServer::generate_JSON_for_multimission_photo_annotations()
{
   cout << "inside MovieServer::generate_JSON_for_multimission_photo_annotations()"
        << endl;

   int selected_fieldtest_ID;
   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

//      cout << "k = " << k 
//           << " Key = " << key
 //          << " Value = " << value << endl;

      if (key=="FieldtestID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
   }

   vector<int> fieldtest_ID,mission_ID,platform_ID;
   vector<string> fieldtest_label,mission_label,platform_label;
   mover_func::retrieve_fieldtest_mission_platform_metadata_from_database(
      gis_database_ptr,fieldtest_label,fieldtest_ID,
      mission_label,mission_ID,platform_label,platform_ID);

// Need to count number of field tests which match
// selected_fieldtest_ID for final JSON comma placing purposes:

   int n_matching_fieldtests=0;
   for (int i=0; i<fieldtest_ID.size(); i++)
   {
      if (fieldtest_ID[i] == selected_fieldtest_ID) n_matching_fieldtests++;
   }
   cout << "n_matching_fieldtests = " << n_matching_fieldtests << endl;
      
   int fieldtest_counter=0;
   string json_string = "{ \n";
   json_string += "  'missions': [ \n";
   for (int i=0; i<fieldtest_ID.size(); i++)
   {
      if (fieldtest_ID[i] != selected_fieldtest_ID) continue;

      json_string += "    {\n";
      json_string += "       'fieldtest': '"+fieldtest_label[i]+"', \n";
      json_string += "       'mission': '"+mission_label[i]+"', \n";
      json_string += "       'platform': '"+platform_label[i]+"', \n";

      json_string += "       'annotations': [ \n";
      int n_indent_spaces=11;
      json_string += generate_JSON_for_single_mission_photo_annotations(
         mission_ID[i],n_indent_spaces);
      json_string += "       ]\n";
      json_string += "    }";
      if (fieldtest_counter < n_matching_fieldtests-1)
      {
         json_string += ",";
      }
      json_string += "\n";

      fieldtest_counter++;
   } // loop over index i labeling correlated fieldtest, mission, platform 
     //	labels

   json_string += "  ] \n";
   json_string += "} \n";
   
   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_for_single_mission_photo_annotations()
// generates and returns a JSON string containing startFrame,
// stopFrame and importance triples which cover all frames within the
// current video clip.  This triple information will be displayed as
// a SIMILE timeline by Diane Staheli.

string AnnotationServer::generate_JSON_for_single_mission_photo_annotations(
   int mission_ID,int n_indent_spaces)
{
//   cout << "inside MovieServer::generate_JSON_for_single_mission_photo_annotations()"
//        << endl;
//   cout << "mission_ID = " << mission_ID << endl;

   string white_space;
   for (int n=0; n<n_indent_spaces; n++)
   {
      white_space += " ";
   }

   vector<int> annotation_IDs,photo_IDs,importances;
   vector<string> photo_times,usernames,labels,descriptions,colors;
   vector<twovector> UVs;
   
   photoannotationdbfunc::get_fieldtest_photo_annotations(
      gis_database_ptr,mission_ID,
      annotation_IDs,photo_IDs,
      photo_times,usernames,labels,descriptions,
      colors,importances,UVs);

   string json_string;   
//   cout << "photo_importance_intervals.size() = "
//        << photo_importance_intervals.size() << endl;

   Clock clock;

   for (int i=0; i<annotation_IDs.size(); i++)
   {
      int photo_ID=photo_IDs[i];

//      cout << "i = " << i << " importance = " << importance << endl;

      int framenumber,importance;
      string caption,timestamp,filename;
      photodbfunc::retrieve_particular_photo_metadata_from_database(
         gis_database_ptr,photo_ID,caption,timestamp,
         filename,framenumber,importance);

      bool UTC_flag=false;
      double start_time=-1;
      if (timestamp.size() > 0 && timestamp != "NULL")
      {
//         cout << "start_timestamp = " << timestamp << endl;
         start_time=clock.timestamp_string_to_elapsed_secs(
            timestamp,UTC_flag);
      }

      json_string += white_space+"   { \n";
      json_string += white_space+"      'startFrame': '"+
         stringfunc::number_to_string(framenumber)+"', \n";
      json_string += white_space+"      'startTime': '"+
         stringfunc::number_to_string(start_time)+"', \n";
      json_string += white_space+"      'importance': '"+
         stringfunc::number_to_string(importance)+"' \n";
      json_string += white_space+"   }";
      if (i < annotation_IDs.size()-1) json_string += ",";
      json_string += "\n";
   }  // loop over index i labeling photo annotations
   
   return json_string;
}

// ==========================================================================
// Photo retrieval member functions
// ==========================================================================

QByteArray AnnotationServer::get_photos_for_mission()
{
   cout << "inside AnnotationServer::get_photos_for_mission()" << endl;

   int selected_mission_ID=-1;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

      cout << "k = " << k 
           << " Key = " << key
           << " Value = " << value << endl;

      if (key=="MissionID")
      {
         selected_mission_ID=stringfunc::string_to_number(value);
      }
   }

   int n_photos=0;
   vector<int> photo_IDs,photo_framenumbers;
   vector<string> photo_timestamps,photo_urls;
   vector<twovector> photo_npxnpys,photo_lonlats;
   string color_hexstr;
   if (selected_mission_ID > 0)
   {
      colorfunc::Color mission_color=colorfunc::get_color(
         selected_mission_ID%15);
      colorfunc::RGB mission_RGB=colorfunc::get_RGB_values(mission_color);
      color_hexstr=colorfunc::RGB_to_RRGGBB_hex(mission_RGB);
      photodbfunc::retrieve_photo_metadata_from_database(
         gis_database_ptr,selected_mission_ID,
         photo_IDs,photo_timestamps,photo_urls,photo_npxnpys,photo_lonlats,
         photo_framenumbers);
      n_photos=photo_IDs.size();
   }

//   cout << "photo_IDs.size() = " << photo_IDs.size() << endl;
//   cout << "photo_timestamps.size() = " << photo_timestamps.size() << endl;
//   cout << "photo_urls.size() = " << photo_urls.size() << endl;
//   cout << "photo_lonlats.size() = " << photo_lonlats.size() << endl;

   string json_string = "{ \"n_photos\": "+stringfunc::number_to_string(
      n_photos);
   if (n_photos > 0)
   {
      json_string += ",";
   }
   json_string += " \n";

   string key;
   for (int i=0; i<n_photos; i++)
   {
      int npx=photo_npxnpys[i].get(0);
      int npy=photo_npxnpys[i].get(1);

      double curr_longitude=photo_lonlats[i].get(0);
      double curr_latitude=photo_lonlats[i].get(1);
//      cout << "lon = " << curr_longitude
//           << " lat = " << curr_latitude << endl;

      string istring=stringfunc::number_to_string(i);

      key="ID_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(photo_IDs[i])+", \n";

      key="timestamp_"+istring;
      json_string += "\""+key+"\": "
         +"\""+photo_timestamps[i]+"\" , \n";

      key="url_"+istring;
      json_string += "\""+key+"\": "
         +"\""+photo_urls[i]+"\" , \n";

      key="npx_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(npx)+" , \n";

      key="npy_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(npy)+" , \n";

      key="longitude_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(curr_longitude)+" , \n";

      key="latitude_"+istring;
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(curr_latitude)+" , \n";

      key="framenumber_"+istring;
      int true_framenumber=photo_framenumbers[i]+
         AnimationController_ptr->get_frame_counter_offset();
      json_string += "\""+key+"\": "
         +stringfunc::number_to_string(true_framenumber)+", \n";

      key="color_"+istring;
      json_string += "\""+key+"\": "
         +"\""+color_hexstr+"\" ";
      
      if (i < n_photos-1) json_string += ",";
      json_string += "\n";

   } // loop over index i labeling photos
   
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}
