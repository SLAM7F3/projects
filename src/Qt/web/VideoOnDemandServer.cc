// ==========================================================================
// VIDEOONDEMANDSERVER class file
// ==========================================================================
// Last updated on 10/20/11; 10/21/11; 10/28/11
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "Qt/web/VideoOnDemandServer.h"
#include "postgres/databasefuncs.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void VideoOnDemandServer::allocate_member_objects()
{
}		       

void VideoOnDemandServer::initialize_member_objects()
{
   AVI_movie_counter=0;
   postgis_database_ptr=NULL;
}

VideoOnDemandServer::VideoOnDemandServer(
   string host_IP_address,qint16 port, QObject* parent) :
   MessageServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
VideoOnDemandServer::~VideoOnDemandServer()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray VideoOnDemandServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside VideoOnDemandServer:get() method" << endl;

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

   if (URL_path=="/Generate_AVI_Movie/")
   {
      Generate_AVI_movie(responseHeader,response_msg);
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray VideoOnDemandServer::post(
   const QUrl& url, const QByteArray& postData,
   QHttpResponseHeader& responseHeader)
{
//   cout << "inside VideoOnDemandServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   VideoOnDemandServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;
   return doc.toByteArray();
}

// ==========================================================================
// JSON response member functions
// ==========================================================================

// Member function Generate_AVI_movie()

bool VideoOnDemandServer::Generate_AVI_movie(
   QHttpResponseHeader& responseHeader,string& response_msg)
{
   cout << "inside VideoOnDemandServer::Generate_AVI_movie()" << endl;

   int n_args=KeyValue.size();

   int campaign_ID=-1;
   int mission_ID=-1;
   double start_time=-1;
   double stop_time=-1;
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="CampaignID")
      {
         campaign_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="MissionID")
      {
         mission_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="StartTime")
      {
         start_time=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="StopTime")
      {
         stop_time=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "campaign_ID = " << campaign_ID
//        << " mission_ID = " << mission_ID << endl;
//   cout << "start_time = " << start_time << endl;
//   cout << "stop_time = " << stop_time << endl;
   if (stop_time < start_time) return false;

   if (campaign_ID < 0 || mission_ID < 0)
   {
      if (!databasefunc::retrieve_mission_metadata_from_database(
         postgis_database_ptr,start_time,stop_time,
         campaign_ID,mission_ID))
      {
         return false;
      }
      else
      {
//         cout << "Inferred campaign and mission IDs" << endl;
      }
      
   } // campaign_ID < 0 || mission_ID < 0 conditional

   cout << "campaign_ID = " << campaign_ID
        << " mission_ID = " << mission_ID << endl;

// Retrieve FLIR imagery times and filename prefixes from Tstorm
// database corresponding to specified time interval:

   vector<double> epoch_time;
   vector<string> filename_stem;
   databasefunc::retrieve_aircraft_metadata_from_database(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_time,stop_time,epoch_time,filename_stem);

//   string input_imagery_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/thunderstorm/video_data/20110511/flight1/";

//   string input_imagery_subdir=		// ROBOT laptop
//      "/home/cho/programs/c++/svn/projects/src/mains/thunderstorm/video_data/";
   string input_imagery_subdir=		// BEAST server
      "/data_second_disk/EO/Thunderstorm4.0/Analog_Data/";
   input_imagery_subdir += "mission_"+stringfunc::integer_to_string(
      mission_ID,3)+"/";
//   cout << "input_imagery_subdir = " << input_imagery_subdir << endl;

   string image_suffix="jpg";
   string output_movie_filename_prefix="movie";
   string finished_movie_subdir="./";

   string AVI_movie_filename=videofunc::generate_FLIR_AVI_movie(
      input_imagery_subdir,image_suffix,AVI_movie_counter,
      output_movie_filename_prefix,finished_movie_subdir,
      start_time,stop_time,epoch_time,filename_stem);
//   cout << "AVI_movie_filename = " << AVI_movie_filename << endl;

   int AVI_filesize=filefunc::size_of_file_in_bytes(AVI_movie_filename);
//   cout << "AVI_filesize = " << AVI_filesize << endl;

   char* data_ptr=filefunc::ReadChars(AVI_movie_filename,AVI_filesize);
   QByteArray AVI_data_array(data_ptr,AVI_filesize);
   delete [] data_ptr;
//   cout << "AVI_data_array.size() = " << AVI_data_array.size() << endl;

   responseHeader.setContentType("video/avi");
   responseHeader.setValue("Content-Disposition","filename=movie.avi");
   responseHeader.setContentLength(AVI_data_array.size());

   QTcpSocket* socket_ptr = qobject_cast<QTcpSocket *>( sender() );

   QDataStream os( socket_ptr );
   QByteArray ResponseHeaderByteArray=
      responseHeader.toString().toLocal8Bit();
   int n_header_bytes=os.writeRawData(
      ResponseHeaderByteArray,ResponseHeaderByteArray.size());
   int n_data_bytes=os.writeRawData(
      AVI_data_array,AVI_data_array.size());

//   cout << "n_header_bytes = " << n_header_bytes
//        << " n_data_bytes = " << n_data_bytes << endl;
   
   write_text_content_to_socket_flag=false;

// Destroy local copy of AVI movie after it has been returned to
// client which placed the request:

   string unix_cmd="/bin/rm "+AVI_movie_filename;
   sysfunc::unix_command(unix_cmd);

   response_msg="AVI movie generated";
   return true;
}
