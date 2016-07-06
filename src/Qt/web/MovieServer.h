// ========================================================================
// Header file for MOVIESERVER class which enables thin-client
// communication with tech challenge thick clients via HTTP get and
// post commands
// ========================================================================
// Last updated on 12/22/11; 12/24/11; 12/25/11; 1/17/12
// ========================================================================

#ifndef __MOVIESERVER_H__
#define __MOVIESERVER_H__

#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "osg/CustomManipulator.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgFeatures/FeaturePickHandler.h"
#include "postgres/gis_database.h"
#include "messenger/Messenger.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinePickHandler.h"
#include "osg/osg2D/TOCHUD.h"

class texture_rectangle;

class MovieServer : public BasicServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   MovieServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~MovieServer();

// Set & get member functions:

   void set_gis_database_ptr(gis_database* gis_db_ptr);
   void set_CM_ptr(osgGA::CustomManipulator* CM_ptr);
   void set_starting_image_subdir(std::string subdir);
   bool get_movie_subdir_selected_flag() const;
   std::string get_movie_frames_subdir() const;
   std::string get_first_frame_filename() const;
   void set_viewer_messenger_ptr(Messenger* vm_ptr);
   void set_MoviesGroup_ptr(MoviesGroup* MG_ptr);
   void set_TOCHUD_ptr(TOCHUD* TH_ptr);
   TOCHUD* get_TOCHUD_ptr();
   const TOCHUD* get_TOCHUD_ptr() const;
   void set_FeaturesGroup_ptr(FeaturesGroup* FG_ptr);
   void set_FeaturePickHandler_ptr(FeaturePickHandler* FPH_ptr);
   void set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
   void set_PolyLinePickHandler_ptr(PolyLinePickHandler* PLPH_ptr);

   Movie* regenerate_Movie_to_display();


   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

  private:

   bool movie_subdir_selected_flag;
   int selected_fieldtest_ID,selected_mission_ID,selected_platform_ID,
      selected_sensor_ID;
   std::string starting_image_subdir;
   std::string movie_frames_subdir,first_frame_filename;

// Note added on 8/28/10: Following 2 STL vectors should eventually be
// replaced by 1 STL map

   std::vector<int> sensor_IDs;
   std::vector<std::string> sensor_labels;

   osgGA::CustomManipulator* CM_ptr;
   FeaturesGroup* FeaturesGroup_ptr;
   FeaturePickHandler* FeaturePickHandler_ptr;
   gis_database* gis_database_ptr;
   Messenger* viewer_messenger_ptr;
   MoviesGroup* MoviesGroup_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   PolyLinePickHandler* PolyLinePickHandler_ptr;
   texture_rectangle* texture_rectangle_ptr;
   TOCHUD* TOCHUD_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Movie selection member functions:

   QByteArray select_image_stills_subdir_via_GUI();
   QByteArray generate_JSON_response_to_stills_selection(int Nimages);
   QByteArray select_movie_frames_subdir_via_GUI();
   QByteArray generate_JSON_response_to_movie_selection(
      int min_photo_number,int max_photo_number,int Nimages);
   void set_movie_frames_subdir();
   void relate_frames_to_local_world_time();
   void reset_movie_params();

// Movie playback manipulation member functions:

   void goto_movie_start();
   void reverse_movie();
   void decrement_movie_frame();
   void pause_movie();
   void increment_movie_frame();
   void play_movie();
   void goto_movie_end();
   void broadcast_curr_image_URL();
   void increase_frame_rate();
   void decrease_frame_rate();

// TOC mission member functions:

   void pick_mission();
   std::string retrieve_sensor_label(int curr_sensor_ID);
   QByteArray generate_JSON_response_to_picked_mission(
      bool images_retrieved_flag);
   bool retrieve_movie_frames();
   void deduce_movie_params_from_frame_files(std::string movie_frames_subdir);
   QByteArray generate_JSON_response_to_frames_query();

// Video frame/time calibration member functions:

   int calibrate_frames_to_local_world_time();
   int extract_frame_geometries();
   std::string generate_JSON_response_to_frame_calibration(
      int n_calibrated_photos);
   void correlate_frame_numbers_and_world_times();

// Video frame triage member functions:

   QByteArray flag_frames();
   QByteArray generate_JSON_response_to_flagged_frames(
      int starting_flagged_frame,int stopping_flagged_frame,
      int flagged_frame_importance);
   QByteArray generate_JSON_response_to_importance_intervals();
   std::string generate_JSON_for_single_mission_importance_intervals(
      int mission_ID,int n_indent_spaces=0);
   QByteArray generate_JSON_for_multimission_importance_intervals();

   void annotate_current_frame();

// Feature manipulation member functions:

   QByteArray insert_image_feature();
   QByteArray update_image_feature();

   int set_selected_Feature_ID() const;
   void drag_image_feature();
   QByteArray stop_drag_image_feature();

   QByteArray delete_image_feature();
   QByteArray purge_image_features();
   void increase_feature_size();
   void decrease_feature_size();
   QByteArray export_features();
   QByteArray generate_JSON_response_to_geometrical_export(
      std::string output_filename);
   QByteArray import_features();
   QByteArray generate_JSON_response_to_feature_event();
   QByteArray generate_JSON_response_to_feature_event(
      std::string input_filename,bool successful_import_flag=true);

// PolyLine manipulation member functions:

   QByteArray insert_image_polyline();
   QByteArray update_image_polyline();

   int set_selected_PolyLine_ID() const;
   void drag_image_polyline();
   QByteArray stop_drag_image_polyline();
   void scale_image_polyline();
   QByteArray stop_scale_image_polyline();
   void rotate_image_polyline();
   QByteArray stop_rotate_image_polyline();

   QByteArray regenerate_image_PolyLine();
   QByteArray double_image_polyline_vertices();
   QByteArray delete_image_polyline();
   QByteArray purge_image_polylines();
   void increase_polyline_size();
   void decrease_polyline_size();
   QByteArray export_polylines();
   QByteArray import_polylines();

   QByteArray generate_JSON_response_to_polyline_event();
   QByteArray generate_JSON_response_to_polyline_event(
      std::string input_filename);

// PolyLine vertex manipulation member functions:

   osgGeometry::PointsGroup* get_selected_PolyLine_vertices() const;
   void get_selected_PolyLine_and_vertex_IDs(
      int& selected_PolyLine_ID,int& selected_vertex_ID) const;
   void unselect_all_PolyLine_vertices() const;
   int set_selected_Vertex_ID() const;
   int unselect_image_vertex() const;
   QByteArray update_image_vertex();
   void drag_image_vertex();
   QByteArray stop_drag_image_vertex();
   QByteArray move_image_vertex(double dz);
   void regenerate_PolyLine(
      int selected_PolyLine_ID,int selected_vertex_ID,
      const threevector& new_vertex_posn,bool delete_vertex_flag=false);
   void regenerate_PolyLine_wo_vertex(
      int selected_PolyLine_ID,int selected_vertex_ID);
   QByteArray delete_image_vertex();

   void increase_polyline_vertex_size();
   void decrease_polyline_vertex_size();
   void change_polyline_vertex_size(double factor);
   QByteArray generate_JSON_response_to_polyline_vertex_event();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void MovieServer::set_gis_database_ptr(gis_database* gis_db_ptr)
{
   gis_database_ptr=gis_db_ptr;
}

inline void MovieServer::set_CM_ptr(osgGA::CustomManipulator* CM_ptr)
{
   this->CM_ptr=CM_ptr;
}

inline void MovieServer::set_starting_image_subdir(std::string subdir)
{
   starting_image_subdir=subdir;
}

inline bool MovieServer::get_movie_subdir_selected_flag() const
{
   return movie_subdir_selected_flag;
}

inline std::string MovieServer::get_movie_frames_subdir() const
{
   return movie_frames_subdir;
}

inline std::string MovieServer::get_first_frame_filename() const
{
   return first_frame_filename;
}

inline void MovieServer::set_viewer_messenger_ptr(Messenger* vm_ptr)
{
   viewer_messenger_ptr=vm_ptr;
}

inline void MovieServer::set_MoviesGroup_ptr(MoviesGroup* MG_ptr)
{
   MoviesGroup_ptr=MG_ptr;
}

inline void MovieServer::set_TOCHUD_ptr(TOCHUD* TH_ptr)
{
   TOCHUD_ptr=TH_ptr;
}

inline TOCHUD* MovieServer::get_TOCHUD_ptr()
{
   return TOCHUD_ptr;
}

inline const TOCHUD* MovieServer::get_TOCHUD_ptr() const
{
   return TOCHUD_ptr;
}

inline void MovieServer::set_FeaturesGroup_ptr(FeaturesGroup* FG_ptr)
{
   FeaturesGroup_ptr=FG_ptr;
}

inline void MovieServer::set_FeaturePickHandler_ptr(
   FeaturePickHandler* FPH_ptr)
{
   FeaturePickHandler_ptr=FPH_ptr;
}

inline void MovieServer::set_PolyLinesGroup_ptr(PolyLinesGroup* FG_ptr)
{
   PolyLinesGroup_ptr=FG_ptr;
}

inline void MovieServer::set_PolyLinePickHandler_ptr(
   PolyLinePickHandler* FPH_ptr)
{
   PolyLinePickHandler_ptr=FPH_ptr;
}


#endif // __MOVIESERVER_H__
