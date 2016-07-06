// ========================================================================
// Header file for LADARSERVER class which enables thin-client
// communication with tech challenge thick clients via HTTP get and
// post commands
// ========================================================================
// Last updated on 1/18/12; 4/4/12; 6/28/12
// ========================================================================

#ifndef __LADARSERVER_H__
#define __LADARSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/BasicServer.h"
#include "astro_geo/Clock.h"
#include "osg/CustomManipulator.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgFeatures/FeaturePickHandler.h"
#include "postgres/gis_database.h"
#include "osg/osgGrid/Grid.h"
#include "messenger/Messenger.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinePickHandler.h"
#include "osg/osgRegions/RegionPolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgAnnotators/SignPostPickHandler.h"

class LadarServer : public BasicServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   LadarServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~LadarServer();

// Set & get member functions:

   void set_gis_database_ptr(gis_database* gis_db_ptr);
   void set_viewer_messenger_ptr(Messenger* vm_ptr);
   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);
   void set_Grid_ptr(Grid* G_ptr);
   void set_CM_ptr(osgGA::CustomManipulator* CM_ptr);
   void set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);
   void set_SignPostPickHandler_ptr(SignPostPickHandler* SPPH_ptr);
   void set_FeaturesGroup_ptr(FeaturesGroup* FG_ptr);
   void set_FeaturePickHandler_ptr(FeaturePickHandler* FPH_ptr);
   void set_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* PG_ptr);
   void set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
   void set_PolyLinePickHandler_ptr(PolyLinePickHandler* PLPH_ptr);
   void set_photogroup_ptr(photogroup* photogroup_ptr);
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   void set_RegionPolyLinesGroup_ptr(RegionPolyLinesGroup* RPLG_ptr);
   void set_ROIPickHandler_ptr(RegionPolyLinePickHandler* RPLPH_ptr);

   std::vector<std::string> open_input_file_dialog();

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

  private:

   int pointcolor_dependence_counter;
   gis_database* gis_database_ptr;
   Clock clock;
   Messenger* viewer_messenger_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   Grid* Grid_ptr;
   osgGA::CustomManipulator* CM_ptr;
   FeaturesGroup* FeaturesGroup_ptr;
   FeaturePickHandler* FeaturePickHandler_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   photogroup* photogroup_ptr;
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   PolyLinePickHandler* PolyLinePickHandler_ptr;
   RegionPolyLinesGroup* ROIsGroup_ptr;
   RegionPolyLinePickHandler* ROIPickHandler_ptr;
   SignPostsGroup* SignPostsGroup_ptr;
   SignPostPickHandler* SignPostPickHandler_ptr;
//    QWidget* window_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Thin client message parsing member functions:

   void reset_cloud_params();
   void toggle_grid_display();
   void reset_virtual_camera();
   void export_TDP_file();
   void increase_point_size();
   void decrease_point_size();

   void set_maxz_threshold_percentage();
   void set_minz_threshold_percentage();
   void set_maxp_threshold_percentage();
   void set_minp_threshold_percentage();
   void set_pointcolor_dependence(int pointcolor_dependence);

   void set_colormap_offset_percentage();
   void set_intensity_magnification_factor();
   void reset_height_colormap(int c);

// Feature manipulation member functions:

   QByteArray insert_world_feature();
   QByteArray update_world_feature();

   int set_selected_Feature_ID() const;
   void drag_world_feature();
   QByteArray stop_drag_world_feature();
   QByteArray move_up_world_feature();
   QByteArray move_down_world_feature();

   QByteArray delete_world_feature();
   QByteArray purge_world_features();
   void increase_feature_size();
   void decrease_feature_size();
   QByteArray export_features();
//   QByteArray generate_JSON_response_to_feature_export(
//      std::string output_filename);
   QByteArray generate_JSON_response_to_geometrical_export(
      std::string output_filename);
   QByteArray import_features();

   QByteArray generate_JSON_response_to_feature_event();
   QByteArray generate_JSON_response_to_feature_event(
      std::string input_filename);

// PolyLine manipulation member functions:

   QByteArray insert_world_polyline();
   QByteArray update_world_polyline();

   int set_selected_PolyLine_ID() const;
   void drag_world_polyline();
   QByteArray stop_drag_world_polyline();
   QByteArray move_up_world_polyline();
   QByteArray move_down_world_polyline();
   QByteArray double_world_polyline_vertices();
   QByteArray delete_world_polyline();
   QByteArray purge_world_polylines();
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
   int unselect_world_vertex() const;
   QByteArray update_world_vertex();
   void drag_world_vertex();
   QByteArray stop_drag_world_vertex();
   void regenerate_PolyLine(
      int selected_PolyLine_ID,int selected_vertex_ID,
      const threevector& new_vertex_posn,bool delete_vertex_flag=false);
   void regenerate_PolyLine_wo_vertex(
      int selected_PolyLine_ID,int selected_vertex_ID);
   QByteArray move_up_world_vertex();
   QByteArray move_down_world_vertex();
   QByteArray move_world_vertex(double dz);
   QByteArray delete_world_vertex();

   void increase_polyline_vertex_size();
   void decrease_polyline_vertex_size();
   void change_polyline_vertex_size(double factor);
   QByteArray generate_JSON_response_to_polyline_vertex_event();

// Plane manipulation member functions:

   QByteArray insert_world_plane();
   void draw_world_plane(const std::vector<threevector>& V);
   QByteArray export_planes();
   QByteArray import_planes();

// Region of Interest member functions:

   QByteArray insert_world_ROI();
   int set_selected_ROI_ID() const;
   QByteArray delete_world_ROI();
   QByteArray generate_JSON_response_to_ROI_event();

// Annotation manipulation member functions:

   QByteArray insert_world_annotation();
   QByteArray update_world_annotation();

   int set_selected_SignPost_ID() const;

   void drag_world_annotation();
   QByteArray stop_drag_world_annotation();
   QByteArray move_up_world_annotation();
   QByteArray move_down_world_annotation();

   QByteArray delete_world_annotation();
   void increase_annotation_size();
   void decrease_annotation_size();
   void export_annotations();
   QByteArray import_annotations();

   QByteArray generate_JSON_response_to_annotation_event();

// Simulated camera member functions:

   void insert_simulated_camera();
   QByteArray shoot_simulated_photo();
   void set_alpha_percentage();
   void set_alpha_percentage(double alpha);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void LadarServer::set_gis_database_ptr(gis_database* gis_db_ptr)
{
   gis_database_ptr=gis_db_ptr;
}

inline void LadarServer::set_viewer_messenger_ptr(Messenger* vm_ptr)
{
   viewer_messenger_ptr=vm_ptr;
}

inline void LadarServer::set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}

inline void LadarServer::set_Grid_ptr(Grid* G_ptr)
{
   Grid_ptr=G_ptr;
}

inline void LadarServer::set_CM_ptr(osgGA::CustomManipulator* CM_ptr)
{
   this->CM_ptr=CM_ptr;
}

inline void LadarServer::set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr)
{
   SignPostsGroup_ptr=SPG_ptr;
}

inline void LadarServer::set_SignPostPickHandler_ptr(
	SignPostPickHandler* SPPH_ptr)
{
   SignPostPickHandler_ptr=SPPH_ptr;
}

inline void LadarServer::set_FeaturesGroup_ptr(FeaturesGroup* FG_ptr)
{
   FeaturesGroup_ptr=FG_ptr;
}

inline void LadarServer::set_FeaturePickHandler_ptr(
   FeaturePickHandler* FPH_ptr)
{
   FeaturePickHandler_ptr=FPH_ptr;
}

inline void LadarServer::set_PolygonsGroup_ptr(
   osgGeometry::PolygonsGroup* PG_ptr)
{
   PolygonsGroup_ptr=PG_ptr;
}

inline void LadarServer::set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr)
{
   PolyLinesGroup_ptr=PLG_ptr;
}

inline void LadarServer::set_PolyLinePickHandler_ptr(
   PolyLinePickHandler* PH_ptr)
{
   PolyLinePickHandler_ptr=PH_ptr;
}

inline void LadarServer::set_photogroup_ptr(photogroup* photogroup_ptr)
{
   this->photogroup_ptr=photogroup_ptr;
}

inline void LadarServer::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr)
{
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline void LadarServer::set_RegionPolyLinesGroup_ptr(
   RegionPolyLinesGroup* RPG_ptr)
{
   ROIsGroup_ptr=RPG_ptr;
}

inline void LadarServer::set_ROIPickHandler_ptr(
   RegionPolyLinePickHandler* RPH_ptr)
{
   ROIPickHandler_ptr=RPH_ptr;
}

#endif // __LADARSERVER_H__
