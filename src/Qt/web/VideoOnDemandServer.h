// ========================================================================
// Header file for VIDEOONDEMANDSERVER class which enables thin-client
// communication with "photosynth" thick clients via HTTP get and post
// commands
// ========================================================================
// Last updated on 10/20/11; 10/21/11
// ========================================================================

#ifndef __VIDEOONDEMAND_H__
#define __VIDEOONDEMAND_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/MessageServer.h"
#include "osg/osgGIS/postgis_database.h"

class VideoOnDemandServer : public MessageServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   VideoOnDemandServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~VideoOnDemandServer();

// Set & get member functions:

   void set_postgis_database_ptr(postgis_database* db_ptr);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

   bool Generate_AVI_movie(
      QHttpResponseHeader& responseHeader,std::string& response_msg);
   
  private:

   int AVI_movie_counter;
   postgis_database* postgis_database_ptr;
   QByteArray AVI_data_array;

   void allocate_member_objects();
   void initialize_member_objects();

// Dynamic JSON string generation via database query member functions:

   QByteArray generate_JSON_response_to_zeroth_node_ID(int zeroth_node_ID);
   QByteArray generate_JSON_response_to_invalid_entry(
      std::string response_msg);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void VideoOnDemandServer::set_postgis_database_ptr(
   postgis_database* db_ptr)
{
   postgis_database_ptr=db_ptr;
}

      

#endif // __VIDEOONDEMAND_H__
