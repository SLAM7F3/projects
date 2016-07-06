// ========================================================================
// VIDEOSERVER header file
// ========================================================================
// Last updated on 6/26/08; 5/12/09; 5/14/09
// ========================================================================

#ifndef __VIDEOSERVER_H__
#define __VIDEOSERVER_H__

#include <set>
#include <string>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osg2D/Movie.h"
#include "track/tracks_group.h"
#include "Qt/web/WebServer.h"

class VideoServer : public WebServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   VideoServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~VideoServer();

// Set & get member functions:

   void set_Movie_ptr(Movie* M_ptr);
   void set_tracks_group_ptr(tracks_group* tg_ptr);
   void set_CylindersGroup_ptr(CylindersGroup* CG_ptr);
   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      const QUrl& url, QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url, const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

// XML output member functions:
   
   QDomDocument& copy_XML_query_output(std::string query);

  private:

   int n_horiz_output_pixels;
   std::string URL_path;
   CylindersGroup* CylindersGroup_ptr;
   EarthRegionsGroup* EarthRegionsGroup_ptr;
   Movie* Movie_ptr;
   tracks_group* tracks_group_ptr;

   QByteArray PNG_data_array;

// iPhone photo metadata:

   double photo_longitude,photo_latitude,photo_altitude;
   double photo_horizontal_uncertainty,photo_vertical_uncertainty;
   double photo_time;

   void allocate_member_objects();
   void initialize_member_objects();

// Video specific query member functions:

   bool extract_chip_corresponding_to_bbox();
   bool extract_chip_corresponding_to_center_and_radius();
   bool extract_chip_centered_on_track();

   bool extract_chip_given_geopoint_and_radius(
      const geopoint& center_posn,double radius,
      bool draw_central_bbox_flag);

// iPhone photo handling member functions:

   void read_input_photo(const QByteArray& postData);
   
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void VideoServer::set_Movie_ptr(Movie* M_ptr)
{
   Movie_ptr=M_ptr;
}

inline void VideoServer::set_tracks_group_ptr(tracks_group* tg_ptr)
{
   tracks_group_ptr=tg_ptr;
}

inline void VideoServer::set_CylindersGroup_ptr(CylindersGroup* CG_ptr)
{
   CylindersGroup_ptr=CG_ptr;
}

inline void VideoServer::set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;
}


#endif // __VIDEOSERVER_H__
