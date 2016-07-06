// ========================================================================
// LOSSERVER header file
// ========================================================================
// Last updated on 5/19/11; 1/25/12; 1/30/12
// ========================================================================

#ifndef __LOSSERVER_H__
#define __LOSSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "geometry/bounding_box.h"
#include "astro_geo/geopoint.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgTiles/TilesGroup.h"

class LOSServer : public BasicServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   LOSServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~LOSServer();

// Set & get member functions:

   bool get_map_selected_flag() const;
   void set_specified_UTM_zonenumber(int zonenumber);
   int get_specified_UTM_zonenumber() const;
   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   void set_reset_AnimationController_start_stop_times_flag(bool flag);

   double get_longitude_lo() const;
   double get_longitude_hi() const;
   double get_latitude_lo() const;
   double get_latitude_hi() const;
   double get_skymap_longitude_lo() const;
   double get_skymap_latitude_lo() const;
   double get_skymap_longitude_hi() const;
   double get_skymap_latitude_hi() const;
   std::string get_map_countries_name() const;

   const std::vector<geopoint>& get_flightpath_geopoints() const;
   void set_Operations_ptr(Operations* O_ptr);
   void set_FlightPolyLinesGroup_ptr(PolyLinesGroup* FPLG_ptr);
   void set_Aircraft_MODELSGROUP_ptr(LOSMODELSGROUP* MG_ptr);
   void set_TilesGroup_ptr(TilesGroup* TG_ptr);
   void set_GroundTarget_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);
   void set_CompassHUD_ptr(CompassHUD* CH_ptr);
   MODEL* get_aircraft_MODEL_ptr();

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      const QUrl& url, QHttpResponseHeader& responseHeader);
   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

  private:

   bool map_selected_flag,northern_hemisphere_flag;
   bool reset_AnimationController_start_stop_times_flag;
   int specified_UTM_zonenumber,screenshot_counter;
   double meters_per_ft,meters_per_km,meters_per_nmi;
   double longitude_lo,longitude_hi,latitude_lo,latitude_hi;
   double skymap_longitude_lo,skymap_latitude_lo;
   double skymap_longitude_hi,skymap_latitude_hi;
   std::string map_countries_name;
   std::vector<geopoint> flightpath_geopoints;
   PolyLinesGroup* Flight_PolyLinesGroup_ptr;
   LOSMODELSGROUP* Aircraft_MODELSGROUP_ptr;
   MODEL* curr_MODEL_ptr;
   Movie* threatmap_Movie_ptr;
   TilesGroup* TilesGroup_ptr;
   SignPostsGroup* GroundTarget_SignPostsGroup_ptr;
   CompassHUD* CompassHUD_ptr;
   ImageNumberHUD* ImageNumberHUD_ptr;
   bounding_box* latlong_bbox_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// LOS movie functions:

   void display_ImageNumberHUD(bool flag);
   void display_movie_frame(int frame_step);
   void unmask_MODELs_and_rays();
   void play_movie();
   void broadcast_curr_thickclient_framenumber() const;
   void reset_AnimationController_start_stop_times(double flightpath_length);

// JSON response member functions:

   QByteArray generate_JSON_response_to_ROI_entry(
      bool valid_ROI_flag,std::string response_msg,
      double lower_left_longitude,double lower_left_latitude,
      double upper_right_longitude,double upper_right_latitude);
   QByteArray generate_JSON_response_to_flowfield_computation();
   QByteArray generate_JSON_response_to_circular_flightpath_entry(
      double center_longitude,double center_latitude,double orbit_radius,
      double score=0);
   QByteArray generate_JSON_response_to_flightpath_request(
      double flightpath_length,std::string error_msg);
   std::string generate_JSON_flight_distance_and_time_string(
      double flightpath_length,bool include_final_comma_flag);

// Parameter setting member functions:

   void set_bbox_corners(
      int k,double& lower_left_longitude,double& lower_left_latitude,
      double& upper_right_longitude,double& upper_right_latitude);
   void set_map();
   QByteArray set_ground_ROI_parameters(std::string& response_msg);
   bool set_ground_target_parameters(std::string& response_msg);
   void toggle_groundpath_entry();

   bool set_aircraft_parameters(std::string& response_msg);
   bool set_sensor_parameters(std::string& response_msg);
   void set_sensor_parameters(
      double min_ground_sensor_range,double max_ground_sensor_range,
      double horiz_FOV,std::string lobe_pattern);

// Sky map member functions:

   QByteArray compute_threat_map();
   void change_threat_map_alpha();
   void clear_threat_map();
   QByteArray compute_visibility_flowfield();
   std::string check_aircraft_altitude_wrt_ground(
      double skymap_longitude_lo,double skymap_latitude_lo,
      double skymap_longitude_hi,double skymap_latitude_hi);

// Flight path member functions:

   QByteArray set_circular_path();
   double set_general_flightpath(
      const QByteArray& postData,std::string& error_msg);
   double generate_flightpath_from_waypoints(
      const std::vector<threevector>& V,std::string& error_msg);
   std::string check_flightpath_wrt_ground(polyline* flightpath_ptr);
   QByteArray set_automated_path();
   QByteArray generate_JSON_response_to_automated_flightpath_entry(
      double score);
   void clear_flightpath();

   QByteArray parse_clock_parameters(std::string& response_msg);
   QByteArray set_clock_parameters(
      double increment,std::string& response_msg);

// Raytracing member functions:

   bool set_raytracing_controls();
   void hide_aircraft_MODEL_and_OBSFRUSTA();
   void unhide_aircraft_MODEL_and_OBSFRUSTA();
   void set_aircraft_MODEL_and_OBSFRUSTA_masks(int mask_value);
   void clear_raytracing_results();
   QByteArray get_target_occlusion_fractions();
   QByteArray export_avg_occlusion_files();
   QByteArray generate_JSON_response_to_export_avg_occlusion_files(
      std::string geotif_filename,std::string nitf_filename);

   void cancel_calculation();

// PYXIS server member functions:

   void compute_ROI_visibility();
   void start_automatic_analysis();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline bool LOSServer::get_map_selected_flag() const
{
   return map_selected_flag;
}

inline void LOSServer::set_specified_UTM_zonenumber(int zonenumber)
{
   specified_UTM_zonenumber=zonenumber;
}

inline int LOSServer::get_specified_UTM_zonenumber() const
{
   return specified_UTM_zonenumber;
}

inline void LOSServer::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool LOSServer::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void LOSServer::set_reset_AnimationController_start_stop_times_flag(
   bool flag)
{
   reset_AnimationController_start_stop_times_flag=flag;
}


inline double LOSServer::get_longitude_lo() const
{
   return longitude_lo;
}

inline double LOSServer::get_longitude_hi() const
{
   return longitude_hi;
}

inline double LOSServer::get_latitude_lo() const
{
   return latitude_lo;
}

inline double LOSServer::get_latitude_hi() const
{
   return latitude_hi;
}

inline double LOSServer::get_skymap_longitude_lo() const
{
   return skymap_longitude_lo;
}

inline double LOSServer::get_skymap_longitude_hi() const
{
   return skymap_longitude_hi;
}

inline double LOSServer::get_skymap_latitude_lo() const
{
   return skymap_latitude_lo;
}

inline double LOSServer::get_skymap_latitude_hi() const
{
   return skymap_latitude_hi;
}

inline std::string LOSServer::get_map_countries_name() const
{
   return map_countries_name;
}

inline const std::vector<geopoint>& LOSServer::get_flightpath_geopoints() 
   const
{
   return flightpath_geopoints;
}

inline void LOSServer::set_Operations_ptr(Operations* O_ptr)
{
   BasicServer::set_Operations_ptr(O_ptr);
   ImageNumberHUD_ptr=Operations_ptr->get_ImageNumberHUD_ptr();
}

inline void LOSServer::set_FlightPolyLinesGroup_ptr(PolyLinesGroup* FPLG_ptr)
{
   Flight_PolyLinesGroup_ptr=FPLG_ptr;
}

inline void LOSServer::set_Aircraft_MODELSGROUP_ptr(LOSMODELSGROUP* MG_ptr)
{
   std::cout << "inside LOSServer::set_Aircraft_MODELSGROUP_ptr()" 
             << std::endl;
   Aircraft_MODELSGROUP_ptr=MG_ptr;
   std::cout << "Aircraft_MODELSGROUP_ptr = "
             << Aircraft_MODELSGROUP_ptr << std::endl;
}

inline void LOSServer::set_TilesGroup_ptr(TilesGroup* TG_ptr)
{
   TilesGroup_ptr=TG_ptr;
   latlong_bbox_ptr=new bounding_box(
      TilesGroup_ptr->get_min_long(),
      TilesGroup_ptr->get_max_long(),
      TilesGroup_ptr->get_min_lat(),
      TilesGroup_ptr->get_max_lat());
}

inline void LOSServer::set_GroundTarget_SignPostsGroup_ptr(
   SignPostsGroup* SPG_ptr)
{
   GroundTarget_SignPostsGroup_ptr=SPG_ptr;
}

inline void LOSServer::set_CompassHUD_ptr(CompassHUD* CH_ptr)
{
   CompassHUD_ptr=CH_ptr;
}

inline MODEL* LOSServer::get_aircraft_MODEL_ptr()
{
   if (Aircraft_MODELSGROUP_ptr->get_n_Graphicals() > 0)
   {
      return Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);
   }
   else
   {
      return NULL;
   }
}

#endif // __LOSSERVER_H__
