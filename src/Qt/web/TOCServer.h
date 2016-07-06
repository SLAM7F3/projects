// ========================================================================
// Header file for TOCSERVER class which enables thin-client
// communication with tech challenge thick clients via HTTP get and
// post commands
// ========================================================================
// Last updated on 9/12/10; 9/13/10; 9/20/10
// ========================================================================

#ifndef __TOCSERVER_H__
#define __TOCSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "astro_geo/Clock.h"
#include "postgres/gis_database.h"
#include "messenger/Messenger.h"

class G99VideoDisplay;
class homography;

class TOCServer : public BasicServer
{
   Q_OBJECT
    
      public:

   enum SensorType
   {
      GPS,IMAGE,GPS_AND_IMAGE
   };

// Note: Avoid using port 8080 which is the default for TOMCAT.

   TOCServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~TOCServer();

// Set & get member functions:

   void set_gis_database_ptr(gis_database* gis_db_ptr);
   void set_viewer_messenger_ptr(Messenger* vm_ptr);
   void set_metadata_messenger_ptr(Messenger* mdm_ptr);

   SensorType get_sensor_type(int sensor_ID) const;

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

// Image geoalignment member functions:

   std::string geoalign_image(
      double& min_easting, double& max_easting,
      double& min_northing,double& max_northing);
   std::string export_geoaligned_image(
      double new_xdim,double new_ydim,double min_easting,double max_easting,
      double min_northing,double max_northing,std::string input_imagePath,
      homography& H,const G99VideoDisplay& video);
   QByteArray generate_JSON_response_to_image_geoalignment(
      std::string output_image_path,
      double min_easting, double max_easting,
      double min_northing,double max_northing);

// Blue force tracking member functions:

   QByteArray display_GPS_track();

// Data selection member functions:

   QByteArray update_fieldtest_mission_platform_selector_dropdowns();
   QByteArray update_sensor_dropdown(TOCServer::SensorType);
   void pick_mission();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void TOCServer::set_gis_database_ptr(gis_database* gis_db_ptr)
{
   gis_database_ptr=gis_db_ptr;
}

inline void TOCServer::set_viewer_messenger_ptr(Messenger* vm_ptr)
{
   viewer_messenger_ptr=vm_ptr;
}

inline void TOCServer::set_metadata_messenger_ptr(Messenger* mdm_ptr)
{
   metadata_messenger_ptr=mdm_ptr;
}


#endif // __TOCSERVER_H__
