// ========================================================================
// LOSTCLIENT header file.  This client class uses get() and post()
// member functions of Qt's QHttp class in order to transmit
// client requests to an LOSServer.
// ========================================================================
// Last updated on 1/25/12; 4/6/12
// ========================================================================

#ifndef __LOSTCLIENT_H__
#define __LOSTCLIENT_H__

#include <string>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>
#include <QtNetwork/QtNetwork>

class LOSTClient : public QObject
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   LOSTClient(std::string LOSTServer_URL,QObject* parent=NULL);
   ~LOSTClient();

// Set & get member functions:

   std::string& get_returned_output();

// LOSTClient communication member functions:

   void issue_get_request(std::string full_URL);
   void issue_get_request(std::string full_URL,std::string query_params);
   void issue_post_request(std::string full_URL,std::string query_params);
   std::string get_LOSTServer_response();

   protected slots:
        
      void httpResponseAvailable(const QHttpResponseHeader& response_header);
      void httpDoneReading(int,bool);

  protected:

  private:

   QHttp* http_ptr;
   std::string returned_output;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline std::string& LOSTClient::get_returned_output()
{
   return returned_output;
}

#endif // __LOSTCLIENT_H__
