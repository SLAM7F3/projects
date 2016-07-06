// ========================================================================
// Header file for DATALOADERSERVER class which enables thin-client
// communication with tech challenge thick clients via HTTP get and
// post commands
// ========================================================================
// Last updated on 9/5/10; 9/7/10; 1/13/11
// ========================================================================

#ifndef __DATALOADERSERVER_H__
#define __DATALOADERSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "astro_geo/Clock.h"
#include "postgres/gis_database.h"
#include "messenger/Messenger.h"
#include "track/tracks_group.h"

class DataloaderServer : public BasicServer
{
   Q_OBJECT
    
      public:

   enum SensorType
   {
      GPS,MIDG,QUADGPS,GARMINGPS,DROIDGPS,
      IMAGE,DROIDIMAGE,AXISIMAGE,GPSCAMIMAGE
   };

// Note: Avoid using port 8080 which is the default for TOMCAT.

   DataloaderServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~DataloaderServer();

// Set & get member functions:

   void set_clock_ptr(Clock* c_ptr);
   void set_gis_database_ptr(gis_database* gis_db_ptr);
   void set_gps_tracksgroup_ptr(tracks_group* tg_ptr);
   void set_viewer_messenger_ptr(Messenger* vm_ptr);
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

   int selected_fieldtest_ID,selected_mission_ID,
      selected_platform_ID,selected_sensor_ID;
   std::string fieldtest_date,data_filename;

// Note added on 8/28/10: Following 2 STL vectors should eventually be
// replaced by 1 STL map

   std::vector<int> sensor_IDs;
   std::vector<std::string> sensor_labels;

   gis_database* gis_database_ptr;
   Clock* clock_ptr;
   Messenger* viewer_messenger_ptr;
   tracks_group* gps_tracksgroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Dropdown update member functions:

   QByteArray update_selector_metadata();
   QByteArray update_fieldtest_dropdown();
   QByteArray update_mission_dropdown();
   QByteArray update_platform_dropdown();
   QByteArray update_sensor_dropdown();

// Metadata parameter manipulation member functions:

   QByteArray set_fieldtest_params();
   QByteArray set_mission_params();
   QByteArray generate_JSON_response_to_new_mission_entry(int new_mission_ID);

// New data selection member functions:

   void pick_mission();
   QByteArray generate_JSON_response_to_picked_mission();
   QByteArray select_data_file();

   std::string select_file_via_GUI();
   std::string select_GPS_track_file_via_GUI();
   std::string select_MIDG_track_file_via_GUI();
   std::string select_photo_file_via_GUI();

// New data upload member functions:

   std::string retrieve_sensor_label(int curr_sensor_ID);
   QByteArray upload_data();
   bool insert_GPS_track_into_database();
   bool insert_MIDG_track_into_database();
   bool insert_QuadGPS_track_into_database();
   bool insert_DroidGPS_track_into_database();
   bool insert_GarminGPS_track_into_database();
   bool insert_photos_into_database();

   QByteArray update_thumbnail_metadata_in_database();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void DataloaderServer::set_clock_ptr(Clock* c_ptr)
{
   clock_ptr=c_ptr;
}

inline void DataloaderServer::set_gis_database_ptr(gis_database* gis_db_ptr)
{
   gis_database_ptr=gis_db_ptr;
}

inline void DataloaderServer::set_gps_tracksgroup_ptr(tracks_group* tg_ptr)
{
   gps_tracksgroup_ptr=tg_ptr;
}

inline void DataloaderServer::set_viewer_messenger_ptr(Messenger* vm_ptr)
{
   viewer_messenger_ptr=vm_ptr;
}

#endif // __DATALOADERSERVER_H__
