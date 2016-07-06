// ==========================================================================
// LOSTCLIENT class file
// ==========================================================================
// Last updated on 1/25/12; 4/6/12
// ==========================================================================

#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "Qt/web/LOSTClient.h"
#include "Qt/web/DOMParser.h"
#include "general/stringfuncs.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void LOSTClient::allocate_member_objects()
{
   http_ptr=new QHttp(this);
}		       

void LOSTClient::initialize_member_objects()
{
   returned_output="";
}

LOSTClient::LOSTClient(string LOSTServer_URL,QObject* parent) 
{
//   cout << "inside LOSTClient constructor" << endl;
//   cout << "LOSTServer_URL = " << LOSTServer_URL << endl;
   allocate_member_objects();
   initialize_member_objects();

   string LOSTServer_IP=stringfunc::get_hostname_from_URL(LOSTServer_URL);
   int LOSTServer_portnumber=stringfunc::get_portnumber_from_URL(
      LOSTServer_URL);

//   cout << "LOSTServer_IP = " << LOSTServer_IP << endl;
//   cout << "LOSTServer_portnumber = " << LOSTServer_portnumber
//        << endl;

   http_ptr->setHost(QString(LOSTServer_IP.c_str()),
                     LOSTServer_portnumber);
   connect( http_ptr, SIGNAL(readyRead(const QHttpResponseHeader&) ), 
            this, SLOT(httpResponseAvailable(const QHttpResponseHeader&)) );
   connect( http_ptr, SIGNAL(requestFinished(int,bool)), 
            this, SLOT(httpDoneReading(int,bool)) );
}

// ---------------------------------------------------------------------
void LOSTClient::httpResponseAvailable( 
   const QHttpResponseHeader& response_header)
{
//   cout << "inside LOSTClient::httpResponseAvailable()" << endl;
}

// ---------------------------------------------------------------------
void LOSTClient::httpDoneReading(int ID,bool error)
{
   cout << "inside LOSTClient::httpDoneReading()" << endl;

   QByteArray output=http_ptr->readAll();
   returned_output += string(output.data());
}

// ---------------------------------------------------------------------
LOSTClient::~LOSTClient()
{
}

// ==========================================================================
// LOST specific query member functions
// ==========================================================================

// Member function issue_get_request() submit a query to
// http_ptr->get().

void LOSTClient::issue_get_request(string full_URL)
{
//   cout << "inside LOSTClient::issue_get_request()" << endl;
   string query_params="";
   issue_get_request(full_URL,query_params);
}

void LOSTClient::issue_get_request(string full_URL,string query_params)
{
//   cout << "inside LOSTClient::issue_get_request()" << endl;
   
   string query=full_URL;
   if (query_params.size() > 0)
   {
      query += "?"+query_params;
   }
//   cout << "query = " << query << endl;
   
   returned_output="";
   QString requestPath=QString(query.c_str());
//   cout << "requestPath = " << requestPath.toStdString() << endl;
   http_ptr->get( requestPath );
}

void LOSTClient::issue_post_request(string full_URL,string query_params)
{
   cout << "inside LOSTClient::issue_post_request()" << endl;
   
   returned_output="";
   QString requestPath=QString(full_URL.c_str());
//   cout << "requestPath = " << requestPath.toStdString() << endl;
   http_ptr->post( QString(full_URL.c_str()) , 
                   QByteArray(query_params.c_str()) );
}


/*
// ---------------------------------------------------------------------
// After waiting for the LOSTServer to return its http output
// with a terminal sentinel, member function
// get_LOSTServer_response() strips out just the XML body of the
// LOSTServer's message.  It finally loads the XML message into
// the SKS_interface's DOM.

string LOSTClient::get_LOSTServer_response()
{
//   cout << "inside LOSTClient::get_LOSTServer_response()" << endl;

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the WebServer.  Here we force the main Qt loop to continue
// processing until a tag indicating XML message completion has been
// received by the WebClient from the WebServer:

   string response_tag1="</response>";
   string response_tag2="<response/>";
   int counter=0;
   while (! (returned_output_contains_substring(response_tag1) ||
             returned_output_contains_substring(response_tag2)) )
   {
      int n_iters=25;
      for (int n=0; n<n_iters; n++)
      {
         qApp->processEvents();
      }
      if (counter%5000==0)
      {
         cout << counter/5000 << " " << flush;
      }
      counter++;
   }
   cout << endl;

   string XML_content=stringfunc::XML_content_between_tags(
      get_returned_output(),"response");
//   cout << "XML_content = " << XML_content << endl;
//   cout << "XML_content.size() = " << XML_content.size() << endl;
   return XML_content;
}

// ---------------------------------------------------------------------
bool LOSTClient::returned_output_contains_substring(string substring)
{
//   cout << "inside LOSTClient::returned_output_contains_substring()" << endl;
   int posn=stringfunc::first_substring_location(returned_output,substring);
//   cout << "posn = " << posn << endl;
   return (posn >= 0);
}
*/
