// ========================================================================
// WEBSERVER header file
// ========================================================================
// Last updated on 8/3/10; 1/18/11; 2/16/12
// ========================================================================

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <curl/curl.h>
#include <string>
#include <QtNetwork/QtNetwork>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

class WebServer : public QObject
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   WebServer(std::string host_IP, qint16 port, QObject* parent = NULL );
   ~WebServer();
   void setup_initial_signal_slot_connections();

// Set & get member functions:

   bool get_Server_listening_flag() const;
   std::string get_server_URL_prefix() const;
   std::string get_tomcat_URL_prefix() const;

// Get & post member functions:

   virtual bool isPathValid(const QString& path);
   virtual QByteArray get(
      const QUrl& url,QHttpResponseHeader& responseHeader);
   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      QHttpResponseHeader& responseHeader);
   void extract_KeyValue_pairs(const QUrl& url);
   virtual QByteArray post(const QUrl& url, const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

   protected slots:
        
   void incomingConnection();
   void readSocket();
        
  protected:
        
   int n_keys;
   bool Server_listening_flag;
   bool write_text_content_to_socket_flag;
   CURL* curl_ptr;
   QTcpServer* server_ptr;
   QHostAddress host_address;
   std::string host_IP_address,tomcat_URL_prefix;
   int port_number;
   std::vector<std::pair<std::string,std::string> > KeyValue;
   std::vector<std::string> Key;
   std::vector<std::string> Value;

   QMap<QTcpSocket*, QHttpRequestHeader>  _pending;
   std::string content_type;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline bool WebServer::get_Server_listening_flag() const
{
   return Server_listening_flag;
}


#endif // WEBSERVER_H
