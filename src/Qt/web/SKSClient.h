// ========================================================================
// SKSCLIENT header file
// ========================================================================
// Last updated on 5/3/08
// ========================================================================

#ifndef __SKSCLIENT_H__
#define __SKSCLIENT_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>
#include <QtNetwork/QtNetwork>
#include "Qt/web/WebServer.h"

class SKSDataServerInterfacer;
class WebClient;

class SKSClient : public QObject
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   SKSClient(std::string SKSDataServer_URL,QObject* parent=NULL);
   ~SKSClient();

// Set & get member functions:

// SKS DataServer communication member functions:

   void post_query(std::string URL_path,std::string query_params);
	
   void query_SKSDataServer(std::string URL_subpath,std::string query_params);
   std::string get_SKSDataServer_response();
   std::string& get_returned_output();
   bool returned_output_contains_substring(std::string substring);

   protected slots:

      void httpResponseAvailable(const QHttpResponseHeader& response_header);

  protected:

// Bluegrass specific query member functions:

   QString process_vehicle_track_queries(
      const std::vector<std::pair<std::string,std::string> >& KeyValue);

  private:

   std::string SKSDataServer_URL,returned_output;
   QHttp* http_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // __SKSCLIENT_H__
