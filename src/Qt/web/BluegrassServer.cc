// ==========================================================================
// BLUEGRASSSERVER class file
// ==========================================================================
// Last updated on 4/28/08; 4/30/08; 5/3/08; 6/2/08; 6/26/08
// ==========================================================================

#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "Qt/web/BluegrassServer.h"
#include "Qt/web/SKSDataServerInterfacer.h"
#include "general/stringfuncs.h"
#include "templates/mytemplates.h"
#include "Qt/web/WebClient.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void BluegrassServer::allocate_member_objects()
{
}		       

void BluegrassServer::initialize_member_objects()
{
   SKSDataServer_URL=HTMLServer_URL="";
   SKS_interface_ptr=NULL;
}

BluegrassServer::BluegrassServer(
   string host_IP_address,qint16 port, QObject* parent) :
   WebServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
BluegrassServer::~BluegrassServer()
{
   delete SKS_interface_ptr;
   server_ptr->close();
}

// ==========================================================================
// SKS DataServer communication member functions
// ==========================================================================

void BluegrassServer::establish_SKSDataServer_connection(string URL)
{
//   cout << "inside BluegrassServer::establish_SKSDataServer_connection()"
//        << endl;
//   cout << " URL = " << URL << endl;

   SKSDataServer_URL=URL;
//   SKSDataServer_IP="155.34.125.216";		// touchy dataserver
//   SKSDataServer_IP="155.34.135.168";		// dsherrill dataserver

   if (SKSDataServer_URL.size()==0)
   {
      cout << "!!!!!!!!!!!!!!!!!!" << endl;
      cout << "Error in BluegrassServer::establish_SKSDataServer_connection()" 
           << endl;
      cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;
      cout << "!!!!!!!!!!!!!!!!!!" << endl;
   }
//   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;
   SKS_interface_ptr=new SKSDataServerInterfacer(SKSDataServer_URL);
}		       

// ---------------------------------------------------------------------
// Member function query_SKS_DataServer takes in a query string and
// wraps it into an http GET command.  It then instantiates a
// WebClient object on the stack and sets its GET request.  After
// waiting for the SKS DataServer to return its http output with a
// terminal sentinel, this method strips out just the XML body of the
// SKS DataServer's message.  It finally loads the XML message into
// the SKS_interface's DOM.

string BluegrassServer::query_SKS_DataServer(string curr_query)
{
//   cout << "inside BluegrassServer::query_SKS_DataServer()" << endl;
//   cout << "query = " << curr_query << endl;
//   outputfunc::enter_continue_char();

   string GET_command="GET "+curr_query+" HTTP/1.0\r\n\r\n";

   string SKSDataServer_hostname=stringfunc::get_hostname_from_URL(
      SKSDataServer_URL);
   int SKSDataServer_portnumber=stringfunc::get_portnumber_from_URL(
      SKSDataServer_URL);
//   cout << "SKSDataServer_hostname = " << SKSDataServer_hostname
//        << " SKSDataServer_portnumber = " << SKSDataServer_portnumber
//        << endl;
 
   WebClient curr_DataClient(SKSDataServer_hostname,SKSDataServer_portnumber);
   curr_DataClient.set_GET_command(GET_command);

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the SKS DataServer.  Here we force the main Qt loop to continue
// processing until a tag indicating XML message completion has been
// received by the WebClient from the SKS DataServer:

   string XML_tag="</response>";
   while (!curr_DataClient.returned_output_contains_substring(XML_tag))
   {
      qApp->processEvents();
   }

   string XML_content=stringfunc::XML_content_between_tags(
      curr_DataClient.get_returned_output(),"response");
//   cout << "At end of BGS::query_SKS_Dataserver(), XML_content.size() = " 
//        << XML_content.size() << endl;

   SKS_interface_ptr->load_returned_XML_into_DOM(XML_content);
   return XML_content;
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray BluegrassServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
//   cout << "inside BluegrassServer:get()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );
   string URL_path;
   return get(doc,response,url,URL_path,responseHeader);
}

// ---------------------------------------------------------------------
QByteArray BluegrassServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
//   cout << "inside 2nd BluegrassServer:get() method" << endl;

   Q_UNUSED(responseHeader);

   doc.appendChild( response );
   URL_path=url.path().toStdString();
//   cout << "URL path = " << URL_path << endl;
    
// Display key/value items attached to URL:

   vector<pair<string,string> > KeyValue;

   typedef QPair<QString, QString> Pair;
   QList<Pair> items = url.queryItems();
   foreach( Pair item, items ) 
      {
         string key=item.first.toStdString();
         string value=item.second.toStdString();
//         cout << "key = " << key << " value = " << value << endl;

         pair<string,string> P(key,value);
         KeyValue.push_back(P);
      }

   if (URL_path=="/vehicle_tracks/")
   {
      return process_vehicle_track_queries(KeyValue);
   }
   else if (URL_path=="/vehicle_label/")
   {
      generate_dynamic_web_page(KeyValue);
   }
   else if (URL_path=="/SKSDataServer/track")
   {
   }
   
   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray BluegrassServer::post(
   const QUrl& url,const QByteArray& postData,
   QHttpResponseHeader& responseHeader)
{
//   cout << "inside BluegrassServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   BluegrassServer::get(doc,response,url,URL_path,responseHeader);

   if (URL_path=="/flightpath/")
   {
      QUrl tmp_url;
      QString tmp_qstring=tmp_url.fromPercentEncoding(postData);
      string post_data=tmp_qstring.toStdString();
//      cout << "post_data = " << post_data << endl;

/*
      QUrl decoded_url=tmp_url.fromEncoded(postData);
      string post_data2_str=decoded_url.toString().toStdString();
      cout << "post_data2_str = " << post_data2_str << endl;

      QString post_qstring(postData);
      QUrl url3(post_qstring);
      string url3_str=url3.toString().toStdString();
      cout << "post data 3 = " << url3_str << endl;
*/

      string XML_content=stringfunc::XML_content_between_tags(
         post_data,"polyline");
//      cout << "Initial XML content = " << XML_content << endl;
      XML_content=stringfunc::find_and_replace_char(XML_content,"+"," ");
//      cout << "Simplified XML content = " << XML_content << endl;
//      process_flight_path_queries(XML_content,doc,response);
   }
    
   return doc.toByteArray();
}

// ==========================================================================
// Bluegrass specific query member functions
// ==========================================================================

// ---------------------------------------------------------------------
// Member function process_vehicle_track_queries

QByteArray BluegrassServer::process_vehicle_track_queries(
   const vector<pair<string,string> >& KeyValue)
{
   cout << "inside BluegrassServer::process_vehicle_track_queries()" << endl;

   double min_longitude=stringfunc::string_to_number(KeyValue[0].second);
   double min_latitude=stringfunc::string_to_number(KeyValue[1].second);
   double max_longitude=stringfunc::string_to_number(KeyValue[2].second);
   double max_latitude=stringfunc::string_to_number(KeyValue[3].second);
   double t_start=stringfunc::string_to_number(KeyValue[4].second);
   double t_stop=stringfunc::string_to_number(KeyValue[5].second);

//   cout.precision(12);
//   cout << "min_longitude = " << min_longitude 
//        << " min_latitude =  " << min_latitude << endl;
//   cout << "max_longitude = " << max_longitude 
//        << " max_latitude =  " << max_latitude << endl;
//   cout << "t_start= " << t_start << " t_stop =  " << t_stop << endl;
   
   string query=SKS_interface_ptr->form_vehicle_tracks_query(
      min_longitude,min_latitude,max_longitude,max_latitude,t_start,t_stop);
   cout << "Vehicle track query = " << endl;
   cout << query << endl;
   string XML_content=query_SKS_DataServer(query);
   cout << "XML_content = " << XML_content << endl;
   return QByteArray(XML_content.c_str());
}

// ==========================================================================
// XML output member functions
// ==========================================================================

// Member function copy_XML_query_output

QDomDocument& BluegrassServer::copy_XML_query_output(string query)
{
   string XML_content=query_SKS_DataServer(query);
//   cout << "XML response = " << XML_content << endl;

   DOMParser parser;
   parser.read_XML_string_into_DOM(XML_content);
   return parser.get_doc();
}

// ---------------------------------------------------------------------
// Member function generate_dynamic_web_page

void BluegrassServer::generate_dynamic_web_page(
   const vector<pair<string,string> >& KeyValue)
{
//   cout << "inside BluegrassServer::generate_dynamic_web_page()" << endl;

// Execute http call to Michael Yee's launcher web applet to
// dynamically generate SAM web page.  Explicitly change ampersands
// within secondary dynamic_URL to %26 in order to avoid their being
// parsed within primary launch_URL:

   string mover_label=KeyValue[0].second;
//   cout << "mover_label = " << mover_label << endl;

//   string dynamic_URL="http://touchy/rco/vehicle.rhtml";
//   dynamic_URL += "?id="+mover_label;
   string dynamic_URL=Dynamic_WikiPage_URL+"?id="+mover_label;
//   cout << "dynamic_URL = " << dynamic_URL << endl;

   QUrl dynamic_QURL(QString(dynamic_URL.c_str()));

   QString launch_URL=
      QString("/LaunchService/launch?url=%1").
      arg(QString(dynamic_QURL.toEncoded()).replace("&","%26"));
   cout << "launch_URL = "<< launch_URL.toStdString() << endl;
         
   cout << "HTMLServer_URL = " << HTMLServer_URL << endl;
   if (HTMLServer_URL.size()==0)
   {
      cout << "Error in BluegrassServer::generate_dynamic_web_page()" << endl;
      cout << "HTMLServer_URL = " << HTMLServer_URL << endl;
      return;
   }
         
   string HTMLServer_IP=stringfunc::get_hostname_from_URL(HTMLServer_URL);
   int HTMLServer_portnumber=stringfunc::get_portnumber_from_URL(
      HTMLServer_URL);

//   cout << "HTMLServer_IP = " << HTMLServer_IP << endl;
//   cout << "HTMLServer_portnumber = " << HTMLServer_portnumber << endl;
   http_client.setHost(QString(HTMLServer_IP.c_str()),HTMLServer_portnumber);
   http_client.get(launch_URL);
}
