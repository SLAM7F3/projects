// ==========================================================================
// SKSCLIENT class file
// ==========================================================================
// Last updated on 5/3/08
// ==========================================================================

#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "Qt/web/SKSClient.h"
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
void SKSClient::allocate_member_objects()
{
   http_ptr=new QHttp(this);
}		       

void SKSClient::initialize_member_objects()
{
   returned_output="";
}

SKSClient::SKSClient(string SKSDataServer_URL,QObject* parent) 
{
   cout << "inside SKSClient constructor()" << endl;
   
   allocate_member_objects();
   initialize_member_objects();

   this->SKSDataServer_URL=SKSDataServer_URL;
   string SKSDataServer_IP=stringfunc::get_hostname_from_URL(
      SKSDataServer_URL);
   int SKSDataServer_portnumber=stringfunc::get_portnumber_from_URL(
      SKSDataServer_URL);
   cout << "SKSDataServer_IP = " << SKSDataServer_IP << endl;
   cout << "SKS portnumber = " << SKSDataServer_portnumber << endl;
//   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;

   http_ptr->setHost(QString(SKSDataServer_IP.c_str()),
                     SKSDataServer_portnumber);
//   connect( http_ptr, SIGNAL(requestFinished(int id,bool error) ), 
   connect( http_ptr, SIGNAL(readyRead(const QHttpResponseHeader&) ), 
            this, SLOT(httpResponseAvailable(const QHttpResponseHeader&)) );
}

// ---------------------------------------------------------------------
SKSClient::~SKSClient()
{
}

// ==========================================================================
// SKS DataServer communication member functions
// ==========================================================================


void SKSClient::post_query(string URL_path,string query_params)
{
   cout << "inside SKSClient::post_query()" << endl;
   cout << "URL_path = " << URL_path 
        << " query_params = " << query_params << endl;

   returned_output="";
   http_ptr->get(QString(URL_path.c_str()));
//   http_ptr->post(QString(URL_path.c_str()),QByteArray(query_params.c_str()));

   cout << "At end of SKSClient::post_query()" << endl;
}

// ---------------------------------------------------------------------
// Member function query_SKSDataServer takes in a URL subpath along
// with a set of query parameters.  After lots of trial & error
// experimentation, Delsey and Peter found on 5/3/08 that using a Qt
// post command with header information explicitly entered separately
// from the main post body works with the SKSDataServer.  

void SKSClient::query_SKSDataServer(string URL_subpath,string query_params)
{
   cout << "inside SKSClient::query_SKSDataServer()" << endl;
   cout << "URL_subpath = " << URL_subpath 
        << " query_params = " << query_params << endl;

   returned_output="";

   QHttpRequestHeader header("POST",QString(URL_subpath.c_str()));
   header.setValue("Host",QString(SKSDataServer_URL.c_str()));
   header.setContentType("application/x-www-form-urlencoded");
   cout << "header = " << header.toString().toStdString() << endl;

   QByteArray request_array(query_params.c_str());

   cout << "request_array = "
        << QString(request_array).toStdString() << endl;

   http_ptr->request(header,request_array);

   cout << "At end of SKSClient::query_SKSDataServer()" << endl;
}

// ---------------------------------------------------------------------
// After waiting for the SKSDataServer to return its http output with
// a terminal sentinel, member function get_SKSDataServer_response()
// strips out just the XML body of the SKSDataServer's message.  It
// finally loads the XML message into the SKS_interface's DOM.

std::string SKSClient::get_SKSDataServer_response()
{
//   cout << "inside SKSClient::get_SKSDataServer_response()" << endl;

   string banner="Retrieving information from SKS Data Server:";
   outputfunc::write_banner(banner);

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the WebServer.  Here we force the main Qt loop to continue
// processing until a tag indicating XML message completion has been
// received by the WebClient from the WebServer:

   string response_tag1="</response>";
   string response_tag2="<response/>";
   while (! (returned_output_contains_substring(response_tag1) ||
             returned_output_contains_substring(response_tag2)) )
   {
//      cout << "returned_output.size() = " << returned_output.size() << endl;
//      cout << "returned_output = " << returned_output << endl;

      int n_iters=25;
      for (int n=0; n<n_iters; n++)
      {
         qApp->processEvents();
      }
   }

   string XML_content=stringfunc::XML_content_between_tags(
      get_returned_output(),"response");
//   cout << "At end of SKSClient::query_SKSDataServer()" << endl;
//   cout << "XML_content = " << XML_content << endl;
   return XML_content;
}

// ---------------------------------------------------------------------
void SKSClient::httpResponseAvailable( 
   const QHttpResponseHeader& response_header)
{
//   cout << "inside SKSClient::httpResponseAvailable()" << endl;

   QByteArray output=http_ptr->readAll();
   returned_output += string(output.data());
//   cout << "returned_output = " << returned_output << endl;
}

// ---------------------------------------------------------------------
string& SKSClient::get_returned_output()
{
   return returned_output;
}

// ---------------------------------------------------------------------
bool SKSClient::returned_output_contains_substring(string substring)
{
//   cout << "inside SKSClient::returned_output_contains_substring()" << endl;
   int posn=stringfunc::first_substring_location(returned_output,substring);
//   cout << "posn = " << posn << endl;
   return (posn >= 0);
}

