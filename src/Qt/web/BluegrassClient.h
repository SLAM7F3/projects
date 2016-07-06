// ========================================================================
// BLUEGRASSCLIENT header file
// ========================================================================
// Last updated on 8/28/08; 9/7/08; 9/21/08; 1/21/09; 1/22/09
// ========================================================================

#ifndef __BLUEGRASSCLIENT_H__
#define __BLUEGRASSCLIENT_H__

#include <string>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>
#include <QtNetwork/QtNetwork>
#include "geometry/bounding_box.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/PickHandlerCallbacks.h"

class PassInfo;
class PassesGroup;
class polyline;
class TrackList;
class tracks_group;

class BluegrassClient : public QObject, public PickHandlerCallbacks
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   BluegrassClient(std::string BluegrassServer_URL,QObject* parent=NULL);
   ~BluegrassClient();

// Set & get member functions:

   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);
   std::vector<bounding_box>& get_auto_nominated_bboxes();

// BluegrassClient communication member functions:

   void query_BluegrassServer(std::string curr_query);
   std::string get_BluegrassServer_response();
   std::string& get_returned_output();
   bool returned_output_contains_substring(std::string substring);

// BluegrassServer track retrieval member functions:

   std::string form_mover_tracks_query(PassInfo* passinfo_ptr);
   int retrieve_mover_tracks(
      std::string SKS_response,double secs_offset,double tracks_altitude,
      tracks_group* mover_tracks_ptr);
   int retrieve_mover_tracks(
      std::string SKS_response,int specified_UTM_zonenumber,
      const threevector& old_origin_offset,
      double mover_posns_rescaling_factor,
      const threevector& new_origin_offset,
      double secs_offset,double tracks_altitude,
      tracks_group* mover_tracks_ptr);
   TrackList* generate_tracks_from_parsed_XML(std::string XML_input);
   void eliminate_repeated_tracks(
      tracks_group* ROI_tracks_group_ptr,
      const std::vector<tracks_group*>& ROI_tracks_group_ptrs);

   std::vector<polyline*> generate_ROIs_from_parsed_XML(
      std::string XML_input,std::vector<std::vector<int> >& 
      label_IDs_for_slow_vehicles_passing_thru_ROIs);

   void display_selected_vehicle_webpage(std::string vehicle_label);

   protected slots:
        
      void httpResponseAvailable(const QHttpResponseHeader& response_header);

  protected:

  private:

   std::string returned_output;
   std::vector<bounding_box> auto_nominated_bboxes;
   EarthRegionsGroup* EarthRegionsGroup_ptr;
   QHttp* http_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void BluegrassClient::set_EarthRegionsGroup_ptr(
   EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;
}

inline std::vector<bounding_box>& BluegrassClient::get_auto_nominated_bboxes()
{
   return auto_nominated_bboxes;
}

#endif // __BLUEGRASSCLIENT_H__
