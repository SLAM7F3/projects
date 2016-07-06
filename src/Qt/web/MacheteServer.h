// ========================================================================
// Header file for MACHETESERVER class which enables thin-client
// communication with thick clients via HTTP get and post commands.
// ========================================================================
// Last updated on 4/12/12; 4/25/12; 4/27/12
// ========================================================================

#ifndef __MACHETESERVER_H__
#define __MACHETESERVER_H__

#include <map>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "astro_geo/Clock.h"
#include "Qt/web/MessageServer.h"
#include "geometry/bounding_box.h"
#include "osg/osgGIS/postgis_database.h"

class LOSTClient;

class MacheteServer : public MessageServer
{
   Q_OBJECT
    
      public:

// DATA_IDS_MAP stores image existence bools as function of image datum IDs:

   typedef std::map<int,bool> DATA_IDS_MAP;

// Note: Avoid using port 8080 which is the default for TOMCAT.

   MacheteServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~MacheteServer();

// Set & get member functions:

   void set_specified_UTM_zonenumber(int zonenumber);
   int get_specified_UTM_zonenumber() const;
   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   double get_region_height() const;
   double get_region_width() const;

   void set_bounding_box_ptr(bounding_box* bb_ptr);
   void set_clock_ptr(Clock* c_ptr);
   void set_postgis_database_ptr(postgis_database* db_ptr);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

  private:

   bool northern_hemisphere_flag;
   int specified_UTM_zonenumber;
   double region_height,region_width;
   bounding_box* bbox_ptr;
   Clock* clock_ptr;
   postgis_database* postgis_database_ptr;

   DATA_IDS_MAP* data_ids_map_ptr;
   LOSTClient* LOSTClient_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Data importing member functions:

   QByteArray find_mounted_disks();
   QByteArray parse_mounted_disks_JSON(std::string& JSON_response);
   QByteArray select_raw_data_files(std::string& response_msg);
   QByteArray import_raw_data_files(::string& response_msg);

// Data processing member functions:

   QByteArray find_files_to_process();
   QByteArray parse_files_to_process_JSON(std::string& JSON_response);

// Get and post member functions:

   std::string Server_module_get_query(
      std::string Server_URL,std::string request);
   std::string Server_module_post_query(
      std::string Server_URL,std::string query_params);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void MacheteServer::set_specified_UTM_zonenumber(int zonenumber)
{
   specified_UTM_zonenumber=zonenumber;
}

inline int MacheteServer::get_specified_UTM_zonenumber() const
{
   return specified_UTM_zonenumber;
}

inline void MacheteServer::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool MacheteServer::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline double MacheteServer::get_region_height() const
{
   return region_height;
}

inline double MacheteServer::get_region_width() const
{
   return region_width;
}

inline void MacheteServer::set_bounding_box_ptr(bounding_box* bb_ptr)
{
   bbox_ptr=bb_ptr;
}

inline void MacheteServer::set_clock_ptr(Clock* c_ptr)
{
   clock_ptr=c_ptr;
}

inline void MacheteServer::set_postgis_database_ptr(postgis_database* db_ptr)
{
   postgis_database_ptr=db_ptr;
}


#endif // __MACHETESERVER_H__
