// ========================================================================
// Header file for ANNOTATIONSERVER class which enables thin-client
// communication with tech challenge thick clients via HTTP get and
// post commands
// ========================================================================
// Last updated on 9/17/10; 9/21/10; 10/19/10
// ========================================================================

#ifndef __ANNOTATIONSERVER_H__
#define __ANNOTATIONSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "astro_geo/Clock.h"
#include "postgres/gis_database.h"
#include "messenger/Messenger.h"

class AnnotationServer : public BasicServer
{
   Q_OBJECT
    
      public:

   enum SensorType
   {
      GPS,IMAGE,GPS_AND_IMAGE
   };

// Note: Avoid using port 8080 which is the default for TOMCAT.

   AnnotationServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~AnnotationServer();

// Set & get member functions:

   void set_gis_database_ptr(gis_database* gis_db_ptr);
   void set_viewer_messenger_ptr(Messenger* vm_ptr);
   void set_metadata_messenger_ptr(Messenger* mdm_ptr);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

  private:

   gis_database* gis_database_ptr;
   Clock clock;
   Messenger *viewer_messenger_ptr,*metadata_messenger_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Map annotation member functions:

   QByteArray get_world_annotations();
   bool insert_world_annotation();
   bool update_world_annotation();
   bool delete_world_annotation();
   void broadcast_world_annotation(
      int curr_annotation_ID,std::string username,std::string label,
      std::string description,std::string color,int importance,
      double longitude,double latitude,double altitude);
   void broadcast_world_annotation_deletion(int curr_annotation_ID);

// Photo annotation member functions:

   QByteArray get_particular_photo_annotations();
   QByteArray get_all_photo_annotations();
   bool insert_photo_annotation();
   bool update_photo_annotation();
   bool delete_photo_annotation();
   void broadcast_photo_annotation(
      int curr_annotation_ID,int fieldtest_ID,int photo_ID,
      std::string username,std::string label,
      std::string description,std::string color,int importance,
      double U,double V);
   void broadcast_photo_annotation_deletion(
      int curr_annotation_ID,int photo_ID);
   
   QByteArray generate_JSON_for_multimission_photo_annotations();
   std::string generate_JSON_for_fieldtest_photo_annotations(
      int fieldtest_ID,int n_indent_spaces);
   std::string generate_JSON_for_single_mission_photo_annotations(
      int mission_ID,int n_indent_spaces);

// Photo retrievel member functions:

   QByteArray get_photos_for_mission();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void AnnotationServer::set_gis_database_ptr(gis_database* gis_db_ptr)
{
   gis_database_ptr=gis_db_ptr;
}

inline void AnnotationServer::set_viewer_messenger_ptr(Messenger* vm_ptr)
{
   viewer_messenger_ptr=vm_ptr;
}

inline void AnnotationServer::set_metadata_messenger_ptr(Messenger* mdm_ptr)
{
   metadata_messenger_ptr=mdm_ptr;
}


#endif // __ANNOTATIONSERVER_H__
