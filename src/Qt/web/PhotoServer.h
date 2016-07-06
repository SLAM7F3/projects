// ========================================================================
// Header file for PHOTOSERVER class which enables thin-client
// communication with "photosynth" thick clients via HTTP get and post
// commands
// ========================================================================
// Last updated on 6/5/13; 7/24/13; 8/13/13; 10/31/13; 12/1/15
// ========================================================================

#ifndef __PHOTOSERVER_H__
#define __PHOTOSERVER_H__

#include <map>
#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "Qt/web/MessageServer.h"
#include "geometry/bounding_box.h"
#include "graphs/graph_hierarchy.h"
#include "osg/osgGrid/Grid.h"
#include "messenger/Messenger.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/PhotoToursGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgAnnotators/SignPostsGroup.h"

class fourvector;

class PhotoServer : public MessageServer
{
   Q_OBJECT
    
      public:

// DATA_IDS_MAP stores image existence bools as function of image datum IDs:

   typedef std::map<int,bool> DATA_IDS_MAP;

// Note: Avoid using port 8080 which is the default for TOMCAT.

   PhotoServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~PhotoServer();

// Set & get member functions:

   void set_JSON_filename(std::string filename);
   void set_specified_UTM_zonenumber(int zonenumber);
   int get_specified_UTM_zonenumber() const;
   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   double get_region_height() const;
   double get_region_width() const;

   void set_bounding_box_ptr(bounding_box* bb_ptr);
   void set_bundler_IO_subdir(std::string subdir);
   void set_Grid_ptr(Grid* G_ptr);
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* O_ptr);
   void set_SUBFRUSTAGROUP_ptr(OBSFRUSTAGROUP* SG_ptr);
   void set_PhotoToursGroup_ptr(PhotoToursGroup* PTG_ptr);
   void set_PhotoTour_PolyLinesGroup_ptr(PolyLinesGroup* PTPLG_ptr);
   void set_postgis_database_ptr(postgis_database* db_ptr);
   void set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);

// ActiveMQ message handling member functions:

   void issue_update_curr_node_message(
      int hierarchy_ID,int graph_level,int node_ID,int photo_ID,
      std::string topic,std::string client_name);
   void issue_update_curr_caption_message(
      int hierarchy_ID,const std::vector<int>& node_IDs,
      std::string caption,std::string topic);
   void issue_update_annotations_message(
      int n_annotations,int hierarchy_ID,const std::vector<int>& node_IDs,
      std::string topic);
   void issue_curr_backprojection_message(
      int hierarchy_ID,int node_ID,std::string topic,std::string client_name,
      const threevector& camera_posn,const threevector& r_hat,
      std::string label);
   void issue_best_path_message(
      int hierarchy_ID,int graph_level,std::vector<int>& node_IDs,
      std::string topic);
   void issue_clear_polygons_message(std::string topic);
   void issue_update_graph_polygons_message(
      const std::vector<twovector>& polygon_vertices,
      const fourvector& RGBA,std::string topic);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);
   virtual QByteArray post(const QUrl& url,const QByteArray& postData,
                           QHttpResponseHeader& responseHeader);

  private:

   bool northern_hemisphere_flag,ladar_point_cloud_suppressed_flag;
   int specified_UTM_zonenumber;
   double region_height,region_width;
   std::string bundler_IO_subdir,JSON_filename;
   bounding_box* bbox_ptr;

// curr_graph_hierarchy_ptr member acts as a cached variable:

   graph_hierarchy* curr_graph_hierarchy_ptr;

   Grid* Grid_ptr;
   OBSFRUSTAGROUP *OBSFRUSTAGROUP_ptr,*SUBFRUSTAGROUP_ptr;
   PhotoToursGroup* PhotoToursGroup_ptr;
   PolyLinesGroup* PhotoTour_PolyLinesGroup_ptr;
   postgis_database* postgis_database_ptr;
   SignPostsGroup* SignPostsGroup_ptr;

   DATA_IDS_MAP* data_ids_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Photo selection member functions:

   bool set_selected_image_ID(std::string& response_msg);

// Photo annotations member functions:

   QByteArray update_annotations();
   QByteArray retrieve_annotations();
   void select_annotated_images();
   void backproject_annotations();
   void clear_backprojections();

   bool set_points(std::string& response_msg);
   bool insert_3D_annotation(std::string& response_msg);
   bool set_regions(std::string& response_msg);
   bool delete_region(std::string& response_msg);

// 3D viewer controls member functions:

   bool set_cloud_point_size(std::string& response_msg);
   bool toggle_grid(std::string& response_msg);
   bool set_photo_opacity(std::string& response_msg);
   bool set_photo_zoom(std::string& response_msg);
   bool reset_to_default_view();

// Phototour member functions:

   void set_tour_path(const QByteArray& postData,std::string& error_msg);
   void rewind_tourpath();
   void play_tour();
   void pause_tour();
   void step_tour_forward();
   void step_tour_backward();
   void clear_tourpath();
   QByteArray generate_JSON_response_to_tourpath_entry();
   std::string generate_JSON_tourpath_distance(
      double tourpath_length,bool include_final_comma_flag);
   void suppress_ladar_point_cloud();

// Dynamic JSON string generation via database query member functions:

   bool get_node_geoposns(std::string& response_msg);
   bool get_node_metadata(std::string& response_msg);
   bool get_node_neighbors(std::string& response_msg);
   int get_requested_node_ID();

   QByteArray get_photo_by_hierarchy_and_datum_IDs();
   QByteArray get_photo_by_hierarchy_and_datum_IDs(
      int hierarchy_ID,int graph_level,int datum_ID,std::string topic,
      std::string client_name="");
   QByteArray get_photo_by_URL();
   QByteArray get_photo_by_hierarchy_level_node();
   QByteArray get_neighbor_thumbnails();
   QByteArray get_sibling_thumbnails();
   QByteArray get_children_thumbnails();
   QByteArray get_parent_thumbnail();

   QByteArray generate_JSON_response_to_requested_photo(
      int hierarchy_ID,int graph_level,int node_ID,int datum_ID,
      std::string caption,std::string photo_timestamp,
      std::string photo_URL,int npx,int npy,
      std::string thumbnail_URL,int thumbnail_npx,int thumbnail_npy,
      int importance);
   QByteArray generate_JSON_response_to_requested_thumbnails(
      std::vector<int>& image_IDs,std::vector<std::string>& URLs,
      std::vector<std::string>& thumbnail_URLs);
   QByteArray generate_JSON_response_to_zeroth_node_ID(int zeroth_node_ID);
   QByteArray generate_JSON_response_to_invalid_entry(
      std::string response_msg);

// Camera query handling member functions:

   QByteArray get_camera_metadata();
   QByteArray generate_JSON_response_to_requested_camera_metadata(
      int hierarchy_ID);
   QByteArray generate_JSON_response_to_requested_camera_metadata(
      int hierarchy_ID,
      const std::vector<int>& image_ID,const std::vector<string>& URL,
      const std::vector<double>& FOV_u,const std::vector<double>& FOV_v,
      const std::vector<double>& U0,const std::vector<double>& V0,
      const std::vector<double>& az,const std::vector<double>& el,
      const std::vector<double>& roll,const std::vector<double>& camera_lon,
      const std::vector<double>& camera_lat,
      const std::vector<double>& camera_alt,
      const std::vector<double>& frustum_sidelength);

// Graph query handling member functions:

   QByteArray get_graph_hierarchy();
   QByteArray generate_all_graph_hierarchy_IDs_JSON_response();
   QByteArray generate_JSON_response_to_requested_graph_hierarchy(
      int hierarchy_ID,std::string description,int n_graphs,int n_levels,
      int n_connected_components,
      const std::vector<int>& graph_ID,const std::vector<int>& graph_level,
      const std::vector<int>& parent_graph_ID,const std::vector<int>& n_nodes,
      const std::vector<int>& n_links);
   QByteArray get_graph();
   void reconstruct_graph_hierarchy_only_if_necessary(int graph_hierarchy_ID);
   QByteArray generate_JSON_response_to_requested_graph(
      int hierarchy_ID,int graph_ID,bool get_nodes_flag,bool get_edges_flag,
      bool get_annotations_flag,const std::vector<int>& incident_node_IDs);
   QByteArray update_hierarchy_level_dropdowns();

   QByteArray get_zeroth_node_given_level();
   void redraw_graph_geometries();

// Connected graph components member functions:

   QByteArray get_connected_graph_components();
   QByteArray generate_JSON_response_to_connected_components(
      int hierarchy_ID,
      const std::vector<int>& graph_IDs,const std::vector<int>& levels,
      const std::vector<int>& connected_component_IDs,
      const std::vector<int>& n_nodes);

// Best path query handling member functions:

   QByteArray find_best_path();
   QByteArray generate_JSON_response_to_best_path(
      const std::vector<int>& path_node_IDs);

// SIFT matches query handling member functions:

   QByteArray get_SIFT_matches();
   QByteArray generate_JSON_response_to_SIFT_matches(
      const std::vector<twovector>& feature_matches);

// Image caption member functions:

   QByteArray edit_image_caption();
   QByteArray generate_JSON_response_to_image_caption(
      std::string image_caption);

// Image time member functions:

   QByteArray median_image_time();
   QByteArray selected_image_time();
   QByteArray generate_JSON_response_to_requested_image_time(
      double image_epoch);

   QByteArray extremal_image_times();
   QByteArray generate_JSON_response_to_extremal_image_times(
      double starting_epoch,double stopping_epoch);
   QByteArray get_image_times();
   QByteArray get_nearby_image_times();
   QByteArray generate_JSON_response_to_image_times(
      int HierarchyID,int GraphID,
      const std::vector<int>& datum_IDs,const std::vector<int>& image_IDs,
      const std::vector<double>& epoch_times,
      const std::vector<std::string>& thumbnail_URLs);
   QByteArray select_temporal_neighbor();
   QByteArray generate_JSON_response_to_temporal_neighbor(
      int temporal_neighbor_node_ID,double new_epoch);

// Image geocoordinates member functions

   QByteArray get_image_geocoordinates();
   QByteArray generate_JSON_response_to_image_geocoordinates(
      int UTM_zonenumber,std::string northern_hemisphere_flag,
      const std::vector<int>& datum_IDs,
      const std::vector<threevector>& camera_posns);

// Image attribute member functions:

   QByteArray get_image_attributes();
   QByteArray generate_JSON_response_to_image_attributes(
      std::vector<std::string>& attribute_keys,
      std::vector<std::vector<std::string> >& attribute_values);
   void select_image_attributes();
   void clear_color_mapper();
   void clear_color_mapper(std::string topic,std::string client_name);

// Image coloring member functions:

   QByteArray generate_color_histogram();
   QByteArray generate_JSON_response_to_color_histogram(
      std::string quantized_image_filename,int npx,int npy,
      const std::vector<double>& color_histogram,
      const std::vector<std::string>& color_labels);
   QByteArray restore_original_colors();
   void select_dominant_colors();

// Image human faces member functions:

   void select_human_faces();
   QByteArray display_face_circles();
   QByteArray generate_JSON_response_to_human_faces(
      int n_detected_faces,string human_faces_image_filename,int npx,int npy);
   QByteArray get_n_detected_faces();
   QByteArray generate_JSON_response_to_n_faces(int n_detected_faces);

// Video keyframe member functions:

   void display_video_keyframes();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void PhotoServer::set_JSON_filename(std::string filename)
{
   JSON_filename=filename;
}

inline void PhotoServer::set_specified_UTM_zonenumber(int zonenumber)
{
   specified_UTM_zonenumber=zonenumber;
}

inline int PhotoServer::get_specified_UTM_zonenumber() const
{
   return specified_UTM_zonenumber;
}

inline void PhotoServer::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool PhotoServer::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline double PhotoServer::get_region_height() const
{
   return region_height;
}

inline double PhotoServer::get_region_width() const
{
   return region_width;
}

inline void PhotoServer::set_bounding_box_ptr(bounding_box* bb_ptr)
{
   bbox_ptr=bb_ptr;
}

inline void PhotoServer::set_bundler_IO_subdir(std::string subdir)
{
   bundler_IO_subdir=subdir;
}

inline void PhotoServer::set_Grid_ptr(Grid* G_ptr)
{
   Grid_ptr=G_ptr;
}

inline void PhotoServer::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* O_ptr)
{
   OBSFRUSTAGROUP_ptr=O_ptr;
} 

inline void PhotoServer::set_SUBFRUSTAGROUP_ptr(OBSFRUSTAGROUP* SG_ptr)
{
   SUBFRUSTAGROUP_ptr=SG_ptr;
} 

inline void PhotoServer::set_PhotoToursGroup_ptr(PhotoToursGroup* PTG_ptr)
{
   PhotoToursGroup_ptr=PTG_ptr;
}

inline void PhotoServer::set_PhotoTour_PolyLinesGroup_ptr(
   PolyLinesGroup* PTPLG_ptr)
{
   PhotoTour_PolyLinesGroup_ptr=PTPLG_ptr;
}

inline void PhotoServer::set_postgis_database_ptr(postgis_database* db_ptr)
{
   postgis_database_ptr=db_ptr;
}

inline void PhotoServer::set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr)
{
   SignPostsGroup_ptr=SPG_ptr;
}


#endif // __PHOTOSERVER_H__
