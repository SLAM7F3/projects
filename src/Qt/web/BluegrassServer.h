// ========================================================================
// BLUEGRASSSERVER header file
// ========================================================================
// Last updated on 4/28/08; 4/29/08; 6/2/08; 6/26/08
// ========================================================================

#ifndef __BLUEGRASSSERVER_H__
#define __BLUEGRASSSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>
#include "Qt/web/WebServer.h"

class SKSDataServerInterfacer;
class WebClient;

class BluegrassServer : public WebServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.
   BluegrassServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   BluegrassServer(
      std::string host_IP_address, qint16 port,
      WebClient* WebClient_ptr,QObject* parent=NULL);
   ~BluegrassServer();

// Set & get member functions:

   void set_HTMLServer_URL(std::string URL);
   void set_Dynamic_WikiPage_URL(std::string URL);

// SKS DataServer communication member functions:
    
   void establish_SKSDataServer_connection(std::string URL);
   std::string query_SKS_DataServer(std::string curr_query);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      const QUrl& url, QHttpResponseHeader& responseHeader);
   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url, const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

// Bluegrass specific query member functions:

   QByteArray process_vehicle_track_queries(
      const std::vector<std::pair<std::string,std::string> >& KeyValue);
   void generate_dynamic_web_page(
      const std::vector<std::pair<std::string,std::string> >& KeyValue);

// XML output member functions:
   
   QDomDocument& copy_XML_query_output(std::string query);

  private:

   std::string SKSDataServer_URL,HTMLServer_URL,Dynamic_WikiPage_URL;
   SKSDataServerInterfacer* SKS_interface_ptr;
   QHttp http_client;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void BluegrassServer::set_HTMLServer_URL(std::string URL)
{
   HTMLServer_URL=URL;
}

inline void BluegrassServer::set_Dynamic_WikiPage_URL(std::string URL)
{
   Dynamic_WikiPage_URL=URL;
}

#endif // __BLUEGRASSSERVER_H__
