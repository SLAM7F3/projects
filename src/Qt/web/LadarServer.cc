// ==========================================================================
// LADARSERVER class file
// ==========================================================================
// Last updated on 1/18/12; 4/4/12; 6/28/12
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include <QtGui/QFileDialog>

#include "Qt/web/LadarServer.h"
#include "video/camerafuncs.h"
#include "geometry/geometry_funcs.h"
#include "templates/mytemplates.h"
#include "geometry/plane.h"
#include "geometry/polyline.h"
#include "math/prob_distribution.h"
#include "video/texture_rectangle.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void LadarServer::allocate_member_objects()
{

   
}		       

void LadarServer::initialize_member_objects()
{
   clock.current_local_time_and_UTC();
   viewer_messenger_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   Grid_ptr=NULL;
   CM_ptr=NULL;
   SignPostsGroup_ptr=NULL;
   SignPostPickHandler_ptr=NULL;
   FeaturesGroup_ptr=NULL;
   FeaturePickHandler_ptr=NULL;
   PolygonsGroup_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   PolyLinePickHandler_ptr=NULL;
   photogroup_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   ROIsGroup_ptr=NULL;
   ROIPickHandler_ptr=NULL;
}

LadarServer::LadarServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
LadarServer::~LadarServer()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

vector<string> LadarServer::open_input_file_dialog()
{
//   cout << "inside LadarServer::open_input_file_dialog()" << endl;

   window_ptr->move(835,0);

   string window_title="Open Ladar Data File";
   string starting_image_subdir=
	"/home/cho/programs/c++/svn/projects/src/mains/falconI/PuertoRico/sortie-237/";
//	"/home/cho/programs/c++/svn/projects/src/mains/";

   if (!filefunc::direxist(starting_image_subdir))
   {
      starting_image_subdir="/home/cho/programs/c++/svn/projects/src/mains/";
   }
   string file_types="Image Files (*.osga *.tdp)";
   return BasicServer::open_input_file_dialog(
      window_title,starting_image_subdir,file_types);
   
/*

//   cout << "starting_image_subdir = "
//        << starting_image_subdir << endl;

   QStringList inputfileNames = QFileDialog::getOpenFileNames(
      window_ptr,
      "Open Ladar Data File", starting_image_subdir.c_str(), 
      "Image Files (*.osga *.tdp)");
   cout << "inputfileNames.size() = " << inputfileNames.size() << endl;

   vector<string> input_filenames;
   for (int i=0; i<inputfileNames.size(); i++)
   {
      input_filenames.push_back(
         string(inputfileNames.at(i).toLocal8Bit().constData()));
//      cout << "i = " << i 
//           << " inputfileName = " << input_filenames[i] << endl;
   }
   return input_filenames;
*/
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray LadarServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside LadarServer:get() method" << endl;

   Q_UNUSED(responseHeader);

   doc.appendChild( response );

   URL_path=url.path().toStdString();
   cout << "URL path = " << URL_path << endl;
   extract_KeyValue_pairs(url);

// In order to parse variable URLs (e.g. screen capture containing
// filenumber as a ratio), decompose URL into substrings separated by
// underscores:

   vector<string> URL_substrings=
      stringfunc::decompose_string_into_substrings(URL_path,"_");
//   for (int s=0; s<URL_substrings.size(); s++)
//   {
//      cout << "s = " << s
//           << " URL_substrings[s] = " << URL_substrings[s] << endl;
//   }

   string response_msg;

   if (URL_path=="/RESET_CLOUD_PARAMS/")
   {
      reset_cloud_params();
   }
   else if (URL_path=="/INCREASE_POINT_SIZE/")
   {
      increase_point_size();
   }
   else if (URL_path=="/DECREASE_POINT_SIZE/")
   {
      decrease_point_size();
   }
   else if (URL_path=="/TOGGLE_GRID_DISPLAY/")
   {
      toggle_grid_display();
   }
   else if (URL_path=="/RESET_VIRTUAL_CAMERA/")
   {
      reset_virtual_camera();
   }
   else if (URL_path=="/EXPORT_TDP_FILE/")
   {
      export_TDP_file();
   }
   else if (URL_path=="/SET_MAXZ_THRESHOLD_PERCENTAGE/")
   {
      set_maxz_threshold_percentage();
   }
   else if (URL_path=="/SET_MINZ_THRESHOLD_PERCENTAGE/")
   {
      set_minz_threshold_percentage();
   }
   else if (URL_path=="/SET_MAXP_THRESHOLD_PERCENTAGE/")
   {
      set_maxp_threshold_percentage();
   }
   else if (URL_path=="/SET_MINP_THRESHOLD_PERCENTAGE/")
   {
      set_minp_threshold_percentage();
   }

   else if (URL_path=="/HEIGHT_POINTCOLOR_DEPENDENCE/")
   {
      set_pointcolor_dependence(2);
   }
   else if (URL_path=="/INTENSITY_POINTCOLOR_DEPENDENCE/")
   {
      set_pointcolor_dependence(3);
   }
   else if (URL_path=="/FUSED_POINTCOLOR_DEPENDENCE/")
   {
      set_pointcolor_dependence(1);
   }
   else if (URL_path=="/smallhuevalue/")
   {
      reset_height_colormap(2);
   }
   else if (URL_path=="/largehuevalue/")
   {
      reset_height_colormap(4);
   } 
   else if (URL_path=="/reverselargehuevalue/")
   {
      reset_height_colormap(14);
   }
  else if (URL_path=="/jet/")
   {
      reset_height_colormap(0);
   }
   else if (URL_path=="/grey/")
   {
      reset_height_colormap(6);
   }
   else if (URL_path=="/purehue/")
   {
      reset_height_colormap(7);
   }
   else if (URL_path=="/wrap1/")
   {
      reset_height_colormap(8);
   }
   else if (URL_path=="/wrap2/")
   {
      reset_height_colormap(9);
   }
   else if (URL_path=="/wrap4/")
   {
      reset_height_colormap(11);
   }
   else if (URL_path=="/wrap8/")
   {
      reset_height_colormap(12);
   }
   else if (URL_path=="/SET_COLORMAP_OFFSET_PERCENTAGE/")
   {
      set_colormap_offset_percentage();
   }
   else if (URL_path=="/SET_INTENSITY_MAGNIFICATION_FACTOR/")
   {
      set_intensity_magnification_factor();
   }

// 3D features

   else if (URL_path=="/INSERT_WORLD_FEATURE/")
   {
      return insert_world_feature();
   }
   else if (URL_path=="/SELECT_WORLD_FEATURE/")
   {
      int feature_ID=set_selected_Feature_ID();
      double max_blink_period=3;      // secs
      FeaturesGroup_ptr->blink_Geometrical(feature_ID,max_blink_period);
      return generate_JSON_response_to_feature_event();
   }
   else if (URL_path=="/UNSELECT_WORLD_FEATURE/")
   {
      FeaturesGroup_ptr->set_selected_Graphical_ID(-1);
   }
   else if (URL_path=="/UPDATE_WORLD_FEATURE/")
   {
      return update_world_feature();
   }
   else if (URL_path=="/START_DRAG_WORLD_FEATURE/")
   {
      drag_world_feature();
   }
   else if (URL_path=="/STOP_DRAG_WORLD_FEATURE/")
   {
      return stop_drag_world_feature();
   }
   else if (URL_path=="/MOVE_UP_WORLD_FEATURE/")
   {
      return move_up_world_feature();
   }
   else if (URL_path=="/MOVE_DOWN_WORLD_FEATURE/")
   {
      return move_down_world_feature();
   }
   else if (URL_path=="/DELETE_WORLD_FEATURE/")
   {
      return delete_world_feature();
   }
   else if (URL_path=="/PURGE_WORLD_FEATURES/")
   {
      return purge_world_features();
   }
   else if (URL_path=="/INCREASE_FEATURE_SIZE/")
   {
      increase_feature_size();
   }
   else if (URL_path=="/DECREASE_FEATURE_SIZE/")
   {
      decrease_feature_size();
   }
   else if (URL_path=="/EXPORT_FEATURES/")
   {
      return export_features();
   }
   else if (URL_path=="/IMPORT_FEATURES/")
   {
      return import_features();
   }

// 3D polylines

   else if (URL_path=="/LABEL_POLYLINES_BY_LENGTH/")
   {
      PolyLinesGroup_ptr->set_ID_labels_flag(false);
      PolyLinesGroup_ptr->reset_labels();
   }
   else if (URL_path=="/LABEL_POLYLINES_BY_ID/")
   {
      PolyLinesGroup_ptr->set_ID_labels_flag(true);
      PolyLinesGroup_ptr->reset_labels();
   }
   else if (URL_path=="/INSERT_WORLD_POLYLINE/")
   {
      return insert_world_polyline();
   }
   else if (URL_path=="/SELECT_WORLD_POLYLINE/")
   {
      int polyline_ID=set_selected_PolyLine_ID();
      double max_blink_period=3;      // secs
      PolyLinesGroup_ptr->blink_Geometrical(polyline_ID,max_blink_period);
      return generate_JSON_response_to_polyline_event();
   }
   else if (URL_path=="/UNSELECT_WORLD_POLYLINE/")
   {
      PolyLinesGroup_ptr->set_selected_Graphical_ID(-1);
      unselect_all_PolyLine_vertices();
      return generate_JSON_response_to_polyline_event();
   }
   else if (URL_path=="/UPDATE_WORLD_POLYLINE/")
   {
      return update_world_polyline();
   }
   else if (URL_path=="/START_DRAG_WORLD_POLYLINE/")
   {
      drag_world_polyline();
   }
   else if (URL_path=="/STOP_DRAG_WORLD_POLYLINE/")
   {
      return stop_drag_world_polyline();
   }
   else if (URL_path=="/MOVE_UP_WORLD_POLYLINE/")
   {
      return move_up_world_polyline();
   }
   else if (URL_path=="/MOVE_DOWN_WORLD_POLYLINE/")
   {
      return move_down_world_polyline();
   }
   else if (URL_path=="/DOUBLE_WORLD_POLYLINE_VERTICES/")
   {
      return double_world_polyline_vertices();
   }
   else if (URL_path=="/DELETE_WORLD_POLYLINE/")
   {
      return delete_world_polyline();
   }
   else if (URL_path=="/PURGE_WORLD_POLYLINES/")
   {
      return purge_world_polylines();
   }
   else if (URL_path=="/INCREASE_POLYLINE_SIZE/")
   {
      increase_polyline_size();
   }
   else if (URL_path=="/DECREASE_POLYLINE_SIZE/")
   {
      decrease_polyline_size();
   }
   else if (URL_path=="/EXPORT_POLYLINES/")
   {
      return export_polylines();
   }
   else if (URL_path=="/IMPORT_POLYLINES/")
   {
      return import_polylines();
   }

// 3D world planes

   else if (URL_path=="/INSERT_WORLD_PLANE/")
   {
      return insert_world_plane();
   }
   else if (URL_path=="/EXPORT_PLANES/")
   {
      return export_planes();
   }
   else if (URL_path=="/IMPORT_PLANES/")
   {
      return import_planes();
   }

// 3D world vertices

   else if (URL_path=="/SELECT_WORLD_VERTEX/")
   {
      set_selected_Vertex_ID();
   }
   else if (URL_path=="/UNSELECT_WORLD_VERTEX/")
   {
      unselect_world_vertex();
   }
   else if (URL_path=="/UPDATE_WORLD_VERTEX/")
   {
      return update_world_vertex();
   }
   else if (URL_path=="/START_DRAG_WORLD_VERTEX/")
   {
      drag_world_vertex();
   }
   else if (URL_path=="/STOP_DRAG_WORLD_VERTEX/")
   {
      return stop_drag_world_vertex();
   }
   else if (URL_path=="/MOVE_UP_WORLD_VERTEX/")
   {
      return move_up_world_vertex();
   }
   else if (URL_path=="/MOVE_DOWN_WORLD_VERTEX/")
   {
      return move_down_world_vertex();
   }
   else if (URL_path=="/DELETE_WORLD_VERTEX/")
   {
      return delete_world_vertex();
   }
   else if (URL_path=="/INCREASE_POLYLINE_VERTEX_SIZE/")
   {
      increase_polyline_vertex_size();
   }
   else if (URL_path=="/DECREASE_POLYLINE_VERTEX_SIZE/")
   {
      decrease_polyline_vertex_size();
   }

// 3D Regions of Interest

   else if (URL_path=="/INSERT_WORLD_ROI/")
   {
      return insert_world_ROI();
   }
   else if (URL_path=="/SELECT_WORLD_ROI/")
   {
      set_selected_ROI_ID();
      int top_polyline_ID=ROIsGroup_ptr->get_selected_Graphical_ID()+1;
      double max_blink_period=3;      // secs
      ROIsGroup_ptr->blink_Geometrical(top_polyline_ID,max_blink_period);
      return generate_JSON_response_to_ROI_event();
   }
   else if (URL_path=="/DELETE_WORLD_ROI/")
   {
      return delete_world_ROI();
   }

// 3D annotations

   else if (URL_path=="/INSERT_WORLD_ANNOTATION/")
   {
      return insert_world_annotation();
   }
   else if (URL_path=="/SELECT_WORLD_ANNOTATION/")
   {
      int SignPost_ID=set_selected_SignPost_ID();
      double max_blink_period=3;      // secs
      SignPostsGroup_ptr->blink_Geometrical(SignPost_ID,max_blink_period);
      return generate_JSON_response_to_annotation_event();
   }
   else if (URL_path=="/UNSELECT_WORLD_ANNOTATION/")
   {
      SignPostsGroup_ptr->set_selected_Graphical_ID(-1);
   }
   else if (URL_path=="/UPDATE_WORLD_ANNOTATION/")
   {
      return update_world_annotation();
   }
   else if (URL_path=="/START_DRAG_WORLD_ANNOTATION/")
   {
      drag_world_annotation();
   }
   else if (URL_path=="/STOP_DRAG_WORLD_ANNOTATION/")
   {
      return stop_drag_world_annotation();
   }
   else if (URL_path=="/MOVE_UP_WORLD_ANNOTATION/")
   {
      return move_up_world_annotation();
   }
   else if (URL_path=="/MOVE_DOWN_WORLD_ANNOTATION/")
   {
      return move_down_world_annotation();
   }
   else if (URL_path=="/DELETE_WORLD_ANNOTATION/")
   {
      return delete_world_annotation();
   }
   else if (URL_path=="/INCREASE_ANNOTATION_SIZE/")
   {
      increase_annotation_size();
   }
   else if (URL_path=="/DECREASE_ANNOTATION_SIZE/")
   {
      decrease_annotation_size();
   }
   else if (URL_path=="/EXPORT_ANNOTATIONS/")
   {
      export_annotations();
   }
   else if (URL_path=="/IMPORT_ANNOTATIONS/")
   {
      return import_annotations();
   }

   else if (URL_path=="/INSERT_VIRTUAL_CAMERA/")
   {
      insert_simulated_camera();
   }
   else if (URL_path=="/SHOOT_SIMULATED_PHOTO/")
   {
      return shoot_simulated_photo();
   }
   else if (URL_path=="/SET_ALPHA_PERCENTAGE/")
   {
      set_alpha_percentage();
   }

   else if (URL_path=="/Capture_Viewer_Screen/")
   {
      return capture_viewer_screen();
   }
   else if (URL_path=="/Start_Recording_Movie/")
   {
      cout << "Start recording movie pushed" << endl;
      Start_Recording_Movie();
   }
   else if (URL_path=="/Stop_Recording_Movie/")
   {
      cout << "Stop recording movie pushed" << endl;
      Stop_Recording_Movie();
      return generate_AVI_movie();
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray LadarServer::post(const QUrl& url, const QByteArray& postData,
                             QHttpResponseHeader& responseHeader)
{
//   cout << "inside LadarServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   LadarServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;

   return doc.toByteArray();
}

// ==========================================================================
// Thin client message parsing member functions
// ==========================================================================

// Member function reset_cloud_params()

void LadarServer::reset_cloud_params()
{
   cout << "inside LadarServer::reset_cloud_params()" << endl;

   if (PointCloudsGroup_ptr==NULL) return;
   
   int point_size=0;

   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="PointSize")
      {
         point_size=stringfunc::string_to_number(value);
      }
   } // loop over index k labeling key-value pairs

   cout << "point_size = " << point_size << endl;
   PointCloudsGroup_ptr->set_pt_size(point_size);
}

// ---------------------------------------------------------------------
void LadarServer::toggle_grid_display()
{
//   cout << "inside LadarServer::toggle_grid_display()" << endl;
  
   if (Grid_ptr==NULL) return;
   Grid_ptr->toggle_mask();
}

// ---------------------------------------------------------------------
void LadarServer::reset_virtual_camera()
{
//   cout << "inside LadarServer::reset_virtual_camera()" << endl;
   if (CM_ptr==NULL) return;
   CM_ptr->reset_view_to_home();
}

// ---------------------------------------------------------------------
void LadarServer::export_TDP_file()
{
   cout << "inside LadarServer::export_TDP_file()" << endl;

   PointCloud* cloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);
//   string output_tdpfilename="output";
//   cloud_ptr->write_TDP_file(output_tdpfilename);
   bool set_origin_to_zeroth_xyz_flag=true;
   cloud_ptr->write_output_file(
      "output",set_origin_to_zeroth_xyz_flag);
}

// ---------------------------------------------------------------------
void LadarServer::increase_point_size()
{
//   cout << "inside LadarServer::increase_point_size()" << endl;

   if (PointCloudsGroup_ptr==NULL) return;

   PointCloudsGroup_ptr->set_pt_size(
      PointCloudsGroup_ptr->get_pt_size()+1);
}

// ---------------------------------------------------------------------
void LadarServer::decrease_point_size()
{
//   cout << "inside LadarServer::decrease_point_size()" << endl;

   if (PointCloudsGroup_ptr==NULL) return;

   PointCloudsGroup_ptr->set_pt_size(
      PointCloudsGroup_ptr->get_pt_size()-1);
}

// ---------------------------------------------------------------------
void LadarServer::set_maxz_threshold_percentage()
{
//   cout << "inside LadarServer::set_maxz_threshold_percentage()" << endl;

   if (PointCloudsGroup_ptr->get_n_Graphicals()==0) return;

   int n_args=KeyValue.size();
   double max_zthreshold_percentage=100;
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="maxZThreshold")
      {
         max_zthreshold_percentage=stringfunc::string_to_number(value);
      }
   }

   double zmin=PointCloudsGroup_ptr->get_xyz_bbox().zMin();
   double zmax=PointCloudsGroup_ptr->get_xyz_bbox().zMax();
   double curr_max_zthreshold=
      zmin+0.01*max_zthreshold_percentage*(zmax-zmin);
//   cout << "curr_max_zthreshold = " << curr_max_zthreshold << endl;

   PointCloudsGroup_ptr->set_max_threshold(curr_max_zthreshold);
   PointCloudsGroup_ptr->broadcast_cloud_params();
}

// ---------------------------------------------------------------------
void LadarServer::set_minz_threshold_percentage()
{
//   cout << "inside LadarServer::set_minz_threshold_percentage()" << endl;

   if (PointCloudsGroup_ptr->get_n_Graphicals()==0) return;

   int n_args=KeyValue.size();
   double min_zthreshold_percentage=0;
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="minZThreshold")
      {
         min_zthreshold_percentage=stringfunc::string_to_number(value);
      }
   }

   double zmin=PointCloudsGroup_ptr->get_xyz_bbox().zMin();
   double zmax=PointCloudsGroup_ptr->get_xyz_bbox().zMax();
   double curr_min_zthreshold=
      zmin+0.01*min_zthreshold_percentage*(zmax-zmin);
//   cout << "curr_min_zthreshold = " << curr_min_zthreshold << endl;

   PointCloudsGroup_ptr->set_min_threshold(curr_min_zthreshold);
   PointCloudsGroup_ptr->broadcast_cloud_params();
}

// ---------------------------------------------------------------------
void LadarServer::set_maxp_threshold_percentage()
{
   cout << "inside LadarServer::set_maxp_threshold_percentage()" << endl;

   if (PointCloudsGroup_ptr->get_n_Graphicals()==0) return;

   int n_args=KeyValue.size();
   double max_pthreshold_percentage=100;
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="maxPThreshold")
      {
         max_pthreshold_percentage=stringfunc::string_to_number(value);
      }
   }
//   cout << "max_pthreshold_percentage = " << max_pthreshold_percentage << endl;

   double pmin=PointCloudsGroup_ptr->get_min_value(3);
   double pmax=PointCloudsGroup_ptr->get_max_value(3);
   double curr_max_pthreshold=
      pmin+0.01*max_pthreshold_percentage*(pmax-pmin);
   cout << "curr_max_pthreshold = " << curr_max_pthreshold << endl;

   PointCloudsGroup_ptr->set_max_prob_threshold(curr_max_pthreshold);
   PointCloudsGroup_ptr->broadcast_cloud_params();
}

// ---------------------------------------------------------------------
void LadarServer::set_minp_threshold_percentage()
{
   cout << "inside LadarServer::set_minp_threshold_percentage()" << endl;

   if (PointCloudsGroup_ptr->get_n_Graphicals()==0) return;

   int n_args=KeyValue.size();
   double min_pthreshold_percentage=0;
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="minPThreshold")
      {
         min_pthreshold_percentage=stringfunc::string_to_number(value);
      }
   }
//   cout << "min_pthreshold_percentage = " << min_pthreshold_percentage << endl;

   double pmin=PointCloudsGroup_ptr->get_min_value(3);
   double pmax=PointCloudsGroup_ptr->get_max_value(3);
   double curr_min_pthreshold=
      pmin+0.01*min_pthreshold_percentage*(pmax-pmin);
   cout << "curr_min_pthreshold = " << curr_min_pthreshold << endl;

   PointCloudsGroup_ptr->set_min_prob_threshold(curr_min_pthreshold);
   PointCloudsGroup_ptr->broadcast_cloud_params();
}

// ---------------------------------------------------------------------
void LadarServer::set_pointcolor_dependence(int pointcolor_dependence)
{
//   cout << "inside LadarServer::set_pointcolor_dependence()" << endl;

   if (pointcolor_dependence==2)
   {
      PointCloudsGroup_ptr->set_dependent_coloring_var(2); // z-coloring
   }
   else if (pointcolor_dependence==3)
   {
      PointCloudsGroup_ptr->set_dependent_coloring_var(3); // p-coloring
   }
   else if (pointcolor_dependence==1)
   {
      PointCloudsGroup_ptr->set_dependent_coloring_var(1); // z & p-coloring
   }
   PointCloudsGroup_ptr->reload_all_colors();
}

// ---------------------------------------------------------------------
void LadarServer::set_colormap_offset_percentage()
{
   cout << "inside LadarServer::set_colormap_offset_percentage()" << endl;

   double colormap_frac_offset=0;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="colormapOffset")
      {
         colormap_frac_offset=0.01*stringfunc::string_to_number(value);
      }
   }
   PointCloudsGroup_ptr->set_heightmap_cyclic_colormap_offset(
      colormap_frac_offset);
}

// ---------------------------------------------------------------------
void LadarServer::set_intensity_magnification_factor()
{
   cout << "inside LadarServer::set_intensity_magnification_factor()" << endl;

   double intensity_magnification=1.0;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="IntensityMagnification")
      {
         intensity_magnification=stringfunc::string_to_number(value);
      }
   }
   cout << "intensity_magnification = "
        << intensity_magnification << endl;
   
   PointCloudsGroup_ptr->get_ColorGeodeVisitor_ptr()->
      set_probabilities_magnification(intensity_magnification);

   PointCloud* cloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);
   cloud_ptr->get_p_ColorMap_ptr()->IncrementUpdateIndex();
}

// ---------------------------------------------------------------------
void LadarServer::reset_height_colormap(int colormap_ID)
{
//   cout << "inside LadarServer::reset_height_colormap()" << endl;
//   cout << "colormap_ID = " << colormap_ID << endl;
   PointCloudsGroup_ptr->set_height_color_map(colormap_ID);
   PointCloudsGroup_ptr->set_prob_color_map(colormap_ID);
   PointCloudsGroup_ptr->reload_all_colors();
}

// ==========================================================================
// Feature manipulation member functions
// ==========================================================================

QByteArray LadarServer::insert_world_feature()
{
   cout << "inside LadarServer::insert_world_feature()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_FEATURE);

   FeaturePickHandler_ptr->set_enable_pick_flag(true);

   while (!FeaturesGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in LadarServer" << endl;
      WindowManager_ptr->process();
   }
   FeaturesGroup_ptr->set_Geometricals_updated_flag(false);

   ModeController_ptr->setState(ModeController::VIEW_DATA);
   FeaturePickHandler_ptr->set_enable_pick_flag(false);

   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::update_world_feature()
{
   cout << "inside LadarServer::update_world_feature()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_FEATURE);

   int existing_feature_ID,new_feature_ID,n_feature_coords=0;
   double X,Y,Z;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="existing_feature_ID")
      {
         existing_feature_ID=stringfunc::string_to_number(value);
      }
      else if (key=="new_feature_ID")
      {
         new_feature_ID=stringfunc::string_to_number(value);
      }
      else if (key=="feature_X")
      {
         if (value.size() > 0)
         {
            X=stringfunc::string_to_number(value);
            n_feature_coords++;
         }
      }
      else if (key=="feature_Y")
      {
         if (value.size() > 0)
         {
            Y=stringfunc::string_to_number(value);
            n_feature_coords++;
         }
      }
      else if (key=="feature_Z")
      {
         if (value.size() > 0)
         {
            Z=stringfunc::string_to_number(value);
            n_feature_coords++;
         }
      }
   }
   
//   cout << "existing_feature_ID = " << existing_feature_ID << endl;
   Feature* Feature_ptr=FeaturesGroup_ptr->get_ID_labeled_Feature_ptr(
      existing_feature_ID);
//   cout << "new_feature_ID = " << new_feature_ID << endl;
   Feature* new_Feature_ptr=FeaturesGroup_ptr->get_ID_labeled_Feature_ptr(
      new_feature_ID);
   if (new_Feature_ptr != NULL)
   {
      cout << "Another feature with ID=new_feature_ID already exists!"
           << endl;
   }
   else
   {
      FeaturesGroup_ptr->renumber_Graphical(Feature_ptr,new_feature_ID);
      FeaturesGroup_ptr->set_selected_Graphical_ID(new_feature_ID);
      Feature_ptr->set_ID(new_feature_ID);
      Feature_ptr->reset_text_label();
   }

//   cout << "n_feature_coords = " << n_feature_coords << endl;
   if (n_feature_coords < 3)
   {
   }
   else
   {
      threevector posn(X,Y,Z);   
      Feature_ptr->set_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),
         FeaturesGroup_ptr->get_passnumber(),posn);
   }

   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
int LadarServer::set_selected_Feature_ID() const
{
   cout << "inside LadarServer::set_selected_Feature_ID()" << endl;

   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="feature_ID")
      {
         int Feature_ID=stringfunc::string_to_number(value);
         FeaturesGroup_ptr->set_selected_Graphical_ID(Feature_ID);
//         cout << "Selected Feature_ID = " 
//              << FeaturesGroup_ptr->get_selected_Graphical_ID() << endl;
         return Feature_ID;
      }
   } // loop over index k labeling key-value pairs

   FeaturesGroup_ptr->set_selected_Graphical_ID(-1);
   return -1;
}

// ---------------------------------------------------------------------
void LadarServer::drag_world_feature()
{
//   cout << "inside LadarServer::drag_world_feature()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_FEATURE);
   FeaturePickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray LadarServer::stop_drag_world_feature()
{
//   cout << "inside LadarServer::stop_drag_world_feature()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   FeaturePickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_up_world_feature()
{
//   cout << "inside LadarServer::move_up_world_feature()" << endl;
//   int Feature_ID=get_selected_Feature_ID();

   set_selected_Feature_ID();
   FeaturesGroup_ptr->move_z(1);
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_down_world_feature()
{
//   cout << "inside LadarServer::move_down_world_feature()" << endl;
   set_selected_Feature_ID();
   FeaturesGroup_ptr->move_z(-1);
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::delete_world_feature()
{
//   cout << "inside LadarServer::delete_world_feature()" << endl;

   set_selected_Feature_ID();
   FeaturesGroup_ptr->destroy_feature();
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::purge_world_features()
{
//   cout << "inside LadarServer::purge_world_features()" << endl;

   FeaturesGroup_ptr->destroy_all_Features();
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
void LadarServer::decrease_feature_size()
{
   cout << "inside LadarServer::decrease_feature_size()" << endl;
   FeaturesGroup_ptr->change_size(0.5);
}

void LadarServer::increase_feature_size()
{
   cout << "inside LadarServer::increase_feature_size()" << endl;
   FeaturesGroup_ptr->change_size(2.0);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::export_features()
{
   cout << "inside LadarServer::export_features()" << endl;
   string output_filename=FeaturesGroup_ptr->save_feature_info_to_file();
   cout << "Feature information written to "+output_filename << endl;
   return generate_JSON_response_to_geometrical_export(output_filename);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_geometrical_export()

QByteArray LadarServer::generate_JSON_response_to_geometrical_export(
   string output_filename)
{
   cout << "Inside LadarServer::generate_JSON_response_to_geometrical_export()"
        << endl;
   
   string json_string = "{  \n";
   json_string += " \"Export_filename\": \""+output_filename+"\"";
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
QByteArray LadarServer::import_features()
{
   cout << "inside LadarServer::import_features()" << endl;
   string input_filename=FeaturesGroup_ptr->read_info_from_file();
   cout << "input_filename = " << input_filename << endl;
   return generate_JSON_response_to_feature_event(input_filename);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_feature_event()

QByteArray LadarServer::generate_JSON_response_to_feature_event()
{
   string input_filename="";
   return generate_JSON_response_to_feature_event(input_filename);
}

QByteArray LadarServer::generate_JSON_response_to_feature_event(
   string input_filename)
{
   cout << "Inside LadarServer::generate_JSON_response_to_feature_event()"
        << endl;
   
   string json_string = "{  \n";

   json_string += " \"Features\": [ ";

   int n_Features=FeaturesGroup_ptr->get_n_Graphicals();
   cout << "n_Features = " << n_Features << endl;
   for (int n=0; n<n_Features; n++)
   {
      Feature* Feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(n);
      int ID=Feature_ptr->get_ID();
      threevector Feature_posn;
      Feature_ptr->get_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),
         FeaturesGroup_ptr->get_passnumber(),Feature_posn);

      json_string += 
         " ["+stringfunc::number_to_string(ID)+","
	 +stringfunc::number_to_string(Feature_posn.get(0))+","
         +stringfunc::number_to_string(Feature_posn.get(1))+","
         +stringfunc::number_to_string(Feature_posn.get(2))+" ]";
      if (n < n_Features-1) json_string += ",";
   } // loop over index n labeling Features
   
   json_string += " ], \n ";
   json_string += " \"Import_filename\": \""+input_filename+"\",";
   json_string += " \"SelectedFeatureID\": "+
      stringfunc::number_to_string(
         FeaturesGroup_ptr->get_selected_Graphical_ID())+" \n";
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// PolyLine manipulation member functions
// ==========================================================================

QByteArray LadarServer::insert_world_polyline()
{
   cout << "inside LadarServer::insert_world_polyline()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_POLYLINE);

   PolyLinePickHandler_ptr->set_disable_input_flag(false);
   ROIPickHandler_ptr->set_disable_input_flag(true);

   while (!PolyLinesGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in LadarServer" << endl;
      WindowManager_ptr->process();
   }
   PolyLinesGroup_ptr->set_Geometricals_updated_flag(false);
   PolyLinesGroup_ptr->set_selected_Graphical_ID(
      PolyLinesGroup_ptr->get_most_recently_added_ID());

   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->
      get_ID_labeled_PolyLine_ptr(
         PolyLinesGroup_ptr->get_most_recently_added_ID());

// Label PolyLine with either its ID or its length in meters:

   if (PolyLinesGroup_ptr->get_ID_labels_flag())
   {
      PolyLine_ptr->generate_PolyLine_ID_label();
   }
   else
   {
      PolyLine_ptr->generate_PolyLine_length_label();
   } 

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::update_world_polyline()
{
   cout << "inside LadarServer::update_world_polyline()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);

   int existing_PolyLine_ID,new_PolyLine_ID;
   double X,Y,Z;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="existing_polyline_ID")
      {
         existing_PolyLine_ID=stringfunc::string_to_number(value);
      }
      else if (key=="new_polyline_ID")
      {
         new_PolyLine_ID=stringfunc::string_to_number(value);
      }
   }

//   cout << "existing_PolyLine_ID = " << existing_PolyLine_ID << endl;
   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(
      existing_PolyLine_ID);
//   cout << "new_PolyLine_ID = " << new_PolyLine_ID << endl;
   PolyLine* new_PolyLine_ptr=PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(
      new_PolyLine_ID);

   if (new_PolyLine_ptr != NULL)
   {
      cout << "Another PolyLine with ID=new_PolyLine_ID already exists!"
           << endl;
   }
   else
   {
      PolyLinesGroup_ptr->renumber_Graphical(PolyLine_ptr,new_PolyLine_ID);
      PolyLinesGroup_ptr->set_selected_Graphical_ID(new_PolyLine_ID);
      PolyLine_ptr->set_ID(new_PolyLine_ID);
   }

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
int LadarServer::set_selected_PolyLine_ID() const
{
   cout << "inside LadarServer::set_selected_PolyLine_ID()" << endl;

   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="polyline_ID")
      {
         int PolyLine_ID=stringfunc::string_to_number(value);
         PolyLinesGroup_ptr->set_selected_Graphical_ID(PolyLine_ID);
         cout << "Selected PolyLine_ID = " 
              << PolyLinesGroup_ptr->get_selected_Graphical_ID() << endl;
         return PolyLine_ID;
      }
   } // loop over index k labeling key-value pairs

   PolyLinesGroup_ptr->set_selected_Graphical_ID(-1);
   return -1;
}

// ---------------------------------------------------------------------
void LadarServer::drag_world_polyline()
{
//   cout << "inside LadarServer::drag_world_polyline()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);
   PolyLinePickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray LadarServer::stop_drag_world_polyline()
{
//   cout << "inside LadarServer::stop_drag_world_polyline()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   PolyLinePickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_up_world_polyline()
{
//   cout << "inside LadarServer::move_up_world_polyline()" << endl;

   int PolyLine_ID=set_selected_PolyLine_ID();
   PolyLinesGroup_ptr->move_z(1);
   PolyLinesGroup_ptr->set_selected_Graphical_ID(PolyLine_ID);
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_down_world_polyline()
{
//   cout << "inside LadarServer::move_down_world_polyline()" << endl;
   int PolyLine_ID=set_selected_PolyLine_ID();
   PolyLinesGroup_ptr->move_z(-1);
   PolyLinesGroup_ptr->set_selected_Graphical_ID(PolyLine_ID);
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::double_world_polyline_vertices()
{
//   cout << "inside LadarServer::double_world_polyline_vertices()" << endl;
   int selected_PolyLine_ID=set_selected_PolyLine_ID();
   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(
      selected_PolyLine_ID);
   if (PolyLine_ptr != NULL)
   {
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      int n_vertices=polyline_ptr->get_n_vertices();
      vector<threevector> doubled_vertices;
      for (int n=0; n<n_vertices-1; n++)
      {
         threevector curr_vertex=polyline_ptr->get_vertex(n);
         threevector next_vertex=polyline_ptr->get_vertex(n+1);
         threevector intermediate_vertex=0.5*(curr_vertex+next_vertex);
         doubled_vertices.push_back(curr_vertex);
         doubled_vertices.push_back(intermediate_vertex);
      }
      doubled_vertices.push_back(polyline_ptr->get_last_vertex());

      PolyLine_ptr=PolyLinesGroup_ptr->regenerate_PolyLine(
         doubled_vertices,PolyLine_ptr,PolyLine_ptr->get_permanent_color(),
         PolyLine_ptr->get_selected_color());
      PolyLinesGroup_ptr->set_selected_Graphical_ID(selected_PolyLine_ID);

      if (PolyLinesGroup_ptr->get_ID_labels_flag())
      {
         PolyLine_ptr->generate_PolyLine_ID_label();
      }
      else
      {
         PolyLine_ptr->generate_PolyLine_length_label();
      }

   } // PolyLine_ptr != NULL conditional

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::delete_world_polyline()
{
//   cout << "inside LadarServer::delete_world_polyline()" << endl;

   set_selected_PolyLine_ID();
   PolyLinesGroup_ptr->destroy_PolyLine();
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::purge_world_polylines()
{
//   cout << "inside LadarServer::purge_world_polylines()" << endl;

   PolyLinesGroup_ptr->destroy_all_PolyLines();
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
void LadarServer::decrease_polyline_size()
{
//   cout << "inside LadarServer::decrease_polyline_size()" << endl;
   PolyLinesGroup_ptr->change_size(0.5);
}

void LadarServer::increase_polyline_size()
{
//   cout << "inside LadarServer::increase_polyline_size()" << endl;
   PolyLinesGroup_ptr->change_size(2.0);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::export_polylines()
{
   cout << "inside LadarServer::export_polylines()" << endl;
   string output_filename=PolyLinesGroup_ptr->save_info_to_file();
   cout << "Polyline information written to "+output_filename << endl;
   return generate_JSON_response_to_geometrical_export(output_filename);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::import_polylines()
{
   cout << "inside LadarServer::import_polylines()" << endl;
   string input_filename=
      PolyLinesGroup_ptr->reconstruct_polylines_from_file_info();
   return generate_JSON_response_to_polyline_event(input_filename);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_polyline_event() returns
// a JSON string containing all PolyLine IDs, the selected PolyLine's ID

QByteArray LadarServer::generate_JSON_response_to_polyline_event()
{
   string input_filename="";
   return generate_JSON_response_to_polyline_event(input_filename);
}

QByteArray LadarServer::generate_JSON_response_to_polyline_event(
   string input_filename)
{
   cout << "Inside LadarServer::generate_JSON_response_to_polyline_event()"
        << endl;
   
   string json_string = "{  \n";

   json_string += " \"PolyLineIDs\": [ ";
   int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
//   cout << "n_PolyLines = " << n_PolyLines << endl;

// After PolyLines are manipulated (e.g. some vertex posn altered),
// PolyLinesGroup doesn't necessarily contain PolyLines in the order
// that they were created.  So we explicitly reorder the PolyLines so
// that they do coincide with creation order:

   vector<int> PolyLine_IDs;
   vector<PolyLine*> ordered_PolyLine_ptrs;
   for (int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
      int ID=PolyLine_ptr->get_ID();
      PolyLine_IDs.push_back(ID);
      ordered_PolyLine_ptrs.push_back(PolyLine_ptr);
   } // loop over index n labeling PolyLines
   
   templatefunc::Quicksort(PolyLine_IDs,ordered_PolyLine_ptrs);

   for (int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=ordered_PolyLine_ptrs[n];
      int ID=PolyLine_ptr->get_ID();
      json_string += stringfunc::number_to_string(ID);
      if (n < n_PolyLines-1) json_string += ",";
   } // loop over index n labeling PolyLines
   json_string += " ], \n ";

   json_string += " \"PolyLineLengthLabels\": [ ";
   for (int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=ordered_PolyLine_ptrs[n];
      string length_label=PolyLine_ptr->get_length_label();
      json_string += "\""+length_label+"\"";
      if (n < n_PolyLines-1) json_string += ",";
   } // loop over index n labeling PolyLines
   json_string += " ], \n ";

   json_string += " \"Import_filename\": \""+input_filename+"\",";

   int selected_PolyLine_ID=PolyLinesGroup_ptr->get_selected_Graphical_ID();
//   cout << "selected_PolyLine_ID = " 
//        << selected_PolyLine_ID << endl;
   json_string += " \"selected_PolyLine_ID\": "+
      stringfunc::number_to_string(selected_PolyLine_ID);
   if (selected_PolyLine_ID >= 0)
   {
      json_string += ",";
   }
   json_string += " \n";

   if (selected_PolyLine_ID >= 0)
   {
      PolyLine* PolyLine_ptr=
         PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(selected_PolyLine_ID);
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      int n_vertices=polyline_ptr->get_n_vertices();

      json_string += " \"SelectedPolyLineVertices\": [ ";
      for (int n=0; n<n_vertices; n++)
      {
         threevector curr_vertex=polyline_ptr->get_vertex(n);
         json_string += 
            " ["+stringfunc::number_to_string(n)+","
            +stringfunc::number_to_string(curr_vertex.get(0))+","
            +stringfunc::number_to_string(curr_vertex.get(1))+","
            +stringfunc::number_to_string(curr_vertex.get(2))+" ]";
         if (n < n_vertices-1) json_string += ",";
      } // loop over index n labeling vertices of selected PolyLine
      json_string += " ], \n ";

      osgGeometry::PointsGroup* PointsGroup_ptr=PolyLine_ptr->
         get_PointsGroup_ptr();
      int selected_vertex_ID=
         PointsGroup_ptr->get_selected_Graphical_ID();
      json_string += " \"selected_vertex_ID\": "+
         stringfunc::number_to_string(selected_vertex_ID)+" \n";
   } // selected_PolyLine_ID >= 0 conditional

   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// PolyLine vertex manipulation member functions
// ==========================================================================

osgGeometry::PointsGroup* LadarServer::get_selected_PolyLine_vertices() const
{
//   cout << "inside LadarServer::get_selected_PolyLine_vertices()" << endl;

   int selected_PolyLine_ID=PolyLinesGroup_ptr->get_selected_Graphical_ID();
//   cout << "selected_PolyLine_ID = " << selected_PolyLine_ID << endl;
   
   if (selected_PolyLine_ID==-1) return NULL;
   
   PolyLine* PolyLine_ptr=
      PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(selected_PolyLine_ID);
   osgGeometry::PointsGroup* PointsGroup_ptr=PolyLine_ptr->
      get_PointsGroup_ptr();
   return PointsGroup_ptr;
}

// ---------------------------------------------------------------------
void LadarServer::get_selected_PolyLine_and_vertex_IDs(
   int& selected_PolyLine_ID,int& selected_vertex_ID) const
{
//   cout << "inside LadarServer::get_selected_PolyLine_and_vertex_IDs()" 
//        << endl;

   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="polyline_ID")
      {
         selected_PolyLine_ID=stringfunc::string_to_number(value);
      }
      else if (key=="vertex_ID")
      {
         selected_vertex_ID=stringfunc::string_to_number(value);
      }
   }
   
//   cout << "Selected PolyLine_ID = " << selected_PolyLine_ID << endl;
//   cout << "Selected Vertex_ID = " << selected_vertex_ID << endl;
}

// ---------------------------------------------------------------------
void LadarServer::unselect_all_PolyLine_vertices() const
{
//   cout << "inside LadarServer::unselect_all_PolyLine_vertices()" << endl;

   int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
   for (int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
      osgGeometry::PointsGroup* PointsGroup_ptr=PolyLine_ptr->
         get_PointsGroup_ptr();
      PointsGroup_ptr->set_selected_Graphical_ID(-1);
   }
}

// ---------------------------------------------------------------------
int LadarServer::set_selected_Vertex_ID() const
{
//   cout << "inside LadarServer::set_selected_Vertex_ID()" << endl;

   unselect_all_PolyLine_vertices();

   int selected_PolyLine_ID,selected_vertex_ID;
   get_selected_PolyLine_and_vertex_IDs(
      selected_PolyLine_ID,selected_vertex_ID);

   if (selected_PolyLine_ID==-1) return -1;
   if (selected_vertex_ID==-1) return -1;

// Next select vertex from selected PolyLine:

   osgGeometry::PointsGroup* PointsGroup_ptr=get_selected_PolyLine_vertices();
   PointsGroup_ptr->set_selected_Graphical_ID(selected_vertex_ID);
//   cout << "Selected Point ID = " 
//        << PointsGroup_ptr->get_selected_Graphical_ID() << endl;
   return selected_vertex_ID;
}

// ---------------------------------------------------------------------
int LadarServer::unselect_world_vertex() const
{
//   cout << "inside LadarServer::unselect_world_vertex()" << endl;

   osgGeometry::PointsGroup* PointsGroup_ptr=get_selected_PolyLine_vertices();
   PointsGroup_ptr->set_selected_Graphical_ID(-1);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::update_world_vertex()
{
//   cout << "inside LadarServer::update_world_vertex()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;

   int selected_PolyLine_ID,selected_vertex_ID,n_vertex_coords=0;
   double X,Y,Z;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="polyline_ID")
      {
         selected_PolyLine_ID=stringfunc::string_to_number(value);
      }
      else if (key=="vertex_ID")
      {
         selected_vertex_ID=stringfunc::string_to_number(value);
      }
      else if (key=="vertex_X")
      {
         if (value.size() > 0)
         {
            X=stringfunc::string_to_number(value);
            n_vertex_coords++;
         }
      }
      else if (key=="vertex_Y")
      {
         if (value.size() > 0)
         {
            Y=stringfunc::string_to_number(value);
            n_vertex_coords++;
         }
      }
      else if (key=="vertex_Z")
      {
         if (value.size() > 0)
         {
            Z=stringfunc::string_to_number(value);
            n_vertex_coords++;
         }
      }
   }
   
//   cout << "Selected PolyLine_ID = " << selected_PolyLine_ID << endl;
//   cout << "Selected Vertex_ID = " << selected_vertex_ID << endl;
   osgGeometry::PointsGroup* PointsGroup_ptr=get_selected_PolyLine_vertices();
   
   osgGeometry::Point* Point_ptr=PointsGroup_ptr->get_ID_labeled_Point_ptr(
      selected_vertex_ID);

   if (n_vertex_coords==3)
   {
      threevector new_vertex_posn(X,Y,Z);   
      regenerate_PolyLine(
         selected_PolyLine_ID,selected_vertex_ID,new_vertex_posn);
   }

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
// Member function regenerate_PolyLine() takes in a new location for
// some vertex within some PolyLine.

void LadarServer::regenerate_PolyLine(
   int selected_PolyLine_ID,int selected_vertex_ID,
   const threevector& new_vertex_posn,bool delete_vertex_flag)
{
//   cout << "inside LadarServer::regenerate_PolyLine()" << endl;
   
   if (selected_PolyLine_ID==-1) return;
   if (selected_vertex_ID==-1) return;

   PolyLine* PolyLine_ptr=
      PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(selected_PolyLine_ID);
   if (PolyLine_ptr==NULL) return;
   
   polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
   vector<threevector> vertices;
   int n_vertices=polyline_ptr->get_n_vertices();
//   cout << "n_vertices = " << n_vertices << endl;
   for (int v=0; v<n_vertices; v++)
   {
      threevector curr_vertex=polyline_ptr->get_vertex(v);
      bool add_vertex_flag=true;
      if (v==selected_vertex_ID)
      {

// Do not allow user to delete vertex when PolyLine has only 2 vertices:

         if (delete_vertex_flag && n_vertices == 2)
         {
            add_vertex_flag=true;
         }
         else if (delete_vertex_flag && n_vertices > 2)
         {
            add_vertex_flag=false;
         }
         else
         {
            curr_vertex=new_vertex_posn;
         }
      }
      
      if (add_vertex_flag) vertices.push_back(curr_vertex);
//         cout << "v = " << v << " vertices[v] = " << vertices.back() << endl;
   }

   PolyLine_ptr=PolyLinesGroup_ptr->regenerate_PolyLine(
      vertices,PolyLine_ptr,PolyLine_ptr->get_permanent_color(),
      PolyLine_ptr->get_selected_color());
   PolyLinesGroup_ptr->set_selected_Graphical_ID(selected_PolyLine_ID);

   if (PolyLinesGroup_ptr->get_ID_labels_flag())
   {
      PolyLine_ptr->generate_PolyLine_ID_label();
   }
   else
   {
      PolyLine_ptr->generate_PolyLine_length_label();
   }
      
// Reset selected vertex within selected PolyLine:

   if (delete_vertex_flag && n_vertices > 2)
   {
   }
   else
   {
      osgGeometry::PointsGroup* PointsGroup_ptr=
         get_selected_PolyLine_vertices();
      PointsGroup_ptr->set_selected_Graphical_ID(selected_vertex_ID);
   }
}

void LadarServer::regenerate_PolyLine_wo_vertex(
   int selected_PolyLine_ID,int selected_vertex_ID)
{
   threevector new_vertex_posn(Zero_vector);
   bool delete_vertex_flag=true;
   regenerate_PolyLine(selected_PolyLine_ID,selected_vertex_ID,
		       new_vertex_posn,delete_vertex_flag);
}

// ---------------------------------------------------------------------
void LadarServer::drag_world_vertex()
{
//   cout << "inside LadarServer::drag_world_vertex()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE_VERTEX);
   PolyLinePickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray LadarServer::stop_drag_world_vertex()
{
//   cout << "inside LadarServer::stop_drag_world_vertex()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   PolyLinePickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   move_world_vertex(0);
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_up_world_vertex()
{
//   cout << "inside LadarServer::move_up_world_vertex()" << endl;
   return move_world_vertex(1);
}

QByteArray LadarServer::move_down_world_vertex()
{
//   cout << "inside LadarServer::move_down_world_vertex()" << endl;
   return move_world_vertex(-1);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_world_vertex(double dz)
{
//   cout << "inside LadarServer::move_world_vertex(), dz = " << dz << endl;

   int selected_PolyLine_ID,selected_vertex_ID;
   get_selected_PolyLine_and_vertex_IDs(
      selected_PolyLine_ID,selected_vertex_ID);

   osgGeometry::PointsGroup* PointsGroup_ptr=get_selected_PolyLine_vertices();
   osgGeometry::Point* Point_ptr=PointsGroup_ptr->get_ID_labeled_Point_ptr(
      selected_vertex_ID);

   threevector vertex_posn;
   Point_ptr->get_UVW_coords(PointsGroup_ptr->get_curr_t(),
	   PointsGroup_ptr->get_passnumber(),vertex_posn);

   vertex_posn.put(2,vertex_posn.get(2)+dz);
   regenerate_PolyLine(selected_PolyLine_ID,selected_vertex_ID,vertex_posn);

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::delete_world_vertex()
{
//   cout << "inside LadarServer::delete_world_vertex()" << endl;

   int selected_PolyLine_ID,selected_vertex_ID;
   get_selected_PolyLine_and_vertex_IDs(
      selected_PolyLine_ID,selected_vertex_ID);
   regenerate_PolyLine_wo_vertex(
      selected_PolyLine_ID,selected_vertex_ID);

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
void LadarServer::increase_polyline_vertex_size()
{
   change_polyline_vertex_size(2);
}

void LadarServer::decrease_polyline_vertex_size()
{
   change_polyline_vertex_size(0.5);
}

void LadarServer::change_polyline_vertex_size(double factor)
{
//   cout << "inside LadarServer::change_polyline_vertex_size()" << endl;

   PolyLinesGroup_ptr->set_Pointsize_scalefactor(
      factor*PolyLinesGroup_ptr->get_Pointsize_scalefactor());
   PolyLinesGroup_ptr->set_textsize_scalefactor(
      factor*PolyLinesGroup_ptr->get_textsize_scalefactor());
   int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
   for (int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
      osgGeometry::PointsGroup* PointsGroup_ptr=PolyLine_ptr->
         get_PointsGroup_ptr();
      PointsGroup_ptr->change_size(factor);

      for (int t=0; t<PolyLine_ptr->get_n_text_messages(); t++)
      {
         osgText::Text* text_ptr=PolyLine_ptr->get_text_ptr(t);
         PolyLine_ptr->change_text_size(text_ptr,factor);
      }
   } // loop over index n labeling PolyLines
}

// ==========================================================================
// Region of Interest member functions
// ==========================================================================

QByteArray LadarServer::insert_world_ROI()
{
//   cout << "inside LadarServer::insert_world_ROI()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_POLYLINE);

   PolyLinePickHandler_ptr->set_disable_input_flag(true);
   ROIPickHandler_ptr->set_disable_input_flag(false);

   while (!ROIsGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in LadarServer" << endl;
      WindowManager_ptr->process();
   }
   ROIsGroup_ptr->set_Geometricals_updated_flag(false);

//   ROIsGroup_ptr->set_fixed_label_to_ROI_ID(
//      ROIsGroup_ptr->get_next_unused_ID(),label);
   ROIsGroup_ptr->set_selected_Graphical_ID(
      ROIsGroup_ptr->get_most_recently_added_ID());

   return generate_JSON_response_to_ROI_event();
}

// ---------------------------------------------------------------------
int LadarServer::set_selected_ROI_ID() const
{
   cout << "inside LadarServer::set_selected_ROI_ID()" << endl;

   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="ROI_ID")
      {
         int ROI_ID=stringfunc::string_to_number(value);
         ROIsGroup_ptr->set_selected_Graphical_ID(2*ROI_ID);
         cout << "Selected ROI_ID = " << ROI_ID << endl;
         return ROI_ID;
      }
   } // loop over index k labeling key-value pairs

   ROIsGroup_ptr->set_selected_Graphical_ID(-1);
   return -1;
}

// ---------------------------------------------------------------------
QByteArray LadarServer::delete_world_ROI()
{
   cout << "inside LadarServer::delete_world_polyline()" << endl;

   int bottom_polyline_ID=ROIsGroup_ptr->get_selected_Graphical_ID();
   int top_polyline_ID=bottom_polyline_ID+1;
   
   ROIsGroup_ptr->destroy_PolyLine();
   ROIsGroup_ptr->set_selected_Graphical_ID(top_polyline_ID);
   ROIsGroup_ptr->destroy_PolyLine();
   return generate_JSON_response_to_ROI_event();
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_ROI_event() returns
// a JSON string containing all ROI IDs, the selected PolyLine's ID

QByteArray LadarServer::generate_JSON_response_to_ROI_event()
{
   cout << "Inside LadarServer::generate_JSON_response_to_ROI_event()"
        << endl;
   
   string json_string = "{  \n";

   int n_RegionPolyLines=ROIsGroup_ptr->get_n_Graphicals();
   int n_ROIs=n_RegionPolyLines/2;
   cout << "n_ROIs = " << n_ROIs << endl;

   json_string += " \"ROIIDs\": [ ";

// After PolyLines are manipulated (e.g. some vertex posn altered),
// PolyLinesGroup doesn't necessarily contain PolyLines in the order
// that they were created.  So we explicitly reorder the PolyLines so
// that they do coincide with creation order:

   vector<int> RegionPolyLine_IDs;
   vector<PolyLine*> ordered_RegionPolyLine_ptrs;
   for (int n=0; n<n_RegionPolyLines; n++)
   {
      PolyLine* RegionPolyLine_ptr=ROIsGroup_ptr->get_PolyLine_ptr(n);
      int ID=RegionPolyLine_ptr->get_ID();
      RegionPolyLine_IDs.push_back(ID);
      ordered_RegionPolyLine_ptrs.push_back(RegionPolyLine_ptr);
   } // loop over index n labeling RegionPolyLines
   
   templatefunc::Quicksort(RegionPolyLine_IDs,ordered_RegionPolyLine_ptrs);

   for (int n=0; n<n_RegionPolyLines; n += 2)
   {
      PolyLine* RegionPolyLine_ptr=ordered_RegionPolyLine_ptrs[n];
      int ROI_ID=RegionPolyLine_ptr->get_ID()/2;
      json_string += stringfunc::number_to_string(ROI_ID);
      if (n < n_RegionPolyLines-2) json_string += ",";
   } // loop over index n labeling RegionPolyLines
   json_string += " ], \n ";

/*
   json_string += " \"PolyLineLengthLabels\": [ ";
   for (int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=ordered_PolyLine_ptrs[n];
      string length_label=PolyLine_ptr->get_length_label();
      json_string += "\""+length_label+"\"";
      if (n < n_PolyLines-1) json_string += ",";
   } // loop over index n labeling PolyLines
   json_string += " ], \n ";
*/

   int selected_PolyLine_ID=ROIsGroup_ptr->get_selected_Graphical_ID();
   cout << "selected_PolyLine_ID = " << selected_PolyLine_ID << endl;

   json_string += " \"selected_ROI_ID\": "+
      stringfunc::number_to_string(selected_PolyLine_ID/2);
   if (selected_PolyLine_ID >= 0)
   {
      json_string += ",";
   }
   json_string += " \n";

   if (selected_PolyLine_ID >= 0)
   {
      PolyLine* PolyLine_ptr=
         ROIsGroup_ptr->get_ID_labeled_PolyLine_ptr(selected_PolyLine_ID);
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      int n_vertices=polyline_ptr->get_n_vertices();

      json_string += " \"SelectedBottomPolyLineVertices\": [ ";
      for (int n=0; n<n_vertices; n++)
      {
         threevector curr_vertex=polyline_ptr->get_vertex(n);
         json_string += 
            " ["+stringfunc::number_to_string(n)+","
            +stringfunc::number_to_string(curr_vertex.get(0))+","
            +stringfunc::number_to_string(curr_vertex.get(1))+","
            +stringfunc::number_to_string(curr_vertex.get(2))+" ]";
         if (n < n_vertices-1) json_string += ",";
      } // loop over index n labeling vertices of selected PolyLine
      json_string += " ], \n ";

      osgGeometry::PointsGroup* PointsGroup_ptr=PolyLine_ptr->
         get_PointsGroup_ptr();
      int selected_vertex_ID=
         PointsGroup_ptr->get_selected_Graphical_ID();
      json_string += " \"selected_vertex_ID\": "+
         stringfunc::number_to_string(selected_vertex_ID)+" \n";
   } // selected_PolyLine_ID >= 0 conditional

   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Annotation manipulation member functions
// ==========================================================================

QByteArray LadarServer::insert_world_annotation()
{
   cout << "inside LadarServer::insert_world_annotation()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_ANNOTATION);

   string label;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="new_annotLabel")
      {
         label=value;
      }
   }

   cout << "label = " << label << endl;
   SignPostsGroup_ptr->set_fixed_label_to_SignPost_ID(
      SignPostsGroup_ptr->get_next_unused_ID(),label);

   while (!SignPostsGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in LadarServer" << endl;
      WindowManager_ptr->process();
   }
   SignPostsGroup_ptr->set_Geometricals_updated_flag(false);

   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::update_world_annotation()
{
   cout << "inside LadarServer::update_world_annotation()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_ANNOTATION);

   int SignPost_ID,n_annotation_coords=0;
   double X,Y,Z;
   string label;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
      cout << "k = " << k 
           << " key = " << key
           << " value = " << value << endl;

      if (key=="annotation_ID")
      {
         SignPost_ID=stringfunc::string_to_number(value);
      }
      else if (key=="annotation_label")
      {
         label=value;
      }
      else if (key=="annotation_X")
      {
         if (value.size() > 0)
         {
            X=stringfunc::string_to_number(value);
            n_annotation_coords++;
         }
      }
      else if (key=="annotation_Y")
      {
         if (value.size() > 0)
         {
            Y=stringfunc::string_to_number(value);
            n_annotation_coords++;
         }
      }
      else if (key=="annotation_Z")
      {
         if (value.size() > 0)
         {
            Z=stringfunc::string_to_number(value);
            n_annotation_coords++;
         }
      }
   }
   
   cout << "SignPost_ID = " << SignPost_ID << endl;
   cout << "label = " << label << endl;

   SignPost* SignPost_ptr=SignPostsGroup_ptr->get_ID_labeled_SignPost_ptr(
      SignPost_ID);
   SignPost_ptr->set_label(label);


   cout << "n_annotation_coords = " << n_annotation_coords << endl;
   if (n_annotation_coords < 3)
   {
/*
   while (!SignPostsGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in LadarServer" << endl;
      WindowManager_ptr->process();
   }
   SignPostsGroup_ptr->set_Geometricals_updated_flag(false);
*/

   }
   else
   {
      threevector posn(X,Y,Z);   
      SignPost_ptr->set_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),posn);
   }

   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
int LadarServer::set_selected_SignPost_ID() const
{
   cout << "inside LadarServer::set_selected_SignPost_ID()" << endl;

   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="annotation_ID")
      {
         int SignPost_ID=stringfunc::string_to_number(value);
         SignPostsGroup_ptr->set_selected_Graphical_ID(SignPost_ID);
//         cout << "Selected SignPost_ID = " 
//              << SignPostsGroup_ptr->get_selected_Graphical_ID() << endl;
         return SignPost_ID;
      }
   } // loop over index k labeling key-value pairs

   SignPostsGroup_ptr->set_selected_Graphical_ID(-1);
   return -1;
}

// ---------------------------------------------------------------------
void LadarServer::drag_world_annotation()
{
//   cout << "inside LadarServer::drag_world_annotation()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_ANNOTATION);
   SignPostPickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray LadarServer::stop_drag_world_annotation()
{
//   cout << "inside LadarServer::stop_drag_world_annotation()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   SignPostPickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_up_world_annotation()
{
//   cout << "inside LadarServer::move_up_world_annotation()" << endl;
//   int SignPost_ID=get_selected_SignPost_ID();

   set_selected_SignPost_ID();
   SignPostsGroup_ptr->move_z(1);
   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::move_down_world_annotation()
{
//   cout << "inside LadarServer::move_down_world_annotation()" << endl;
   set_selected_SignPost_ID();
   SignPostsGroup_ptr->move_z(-1);
   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
QByteArray LadarServer::delete_world_annotation()
{
//   cout << "inside LadarServer::delete_world_annotation()" << endl;

   set_selected_SignPost_ID();
   SignPostsGroup_ptr->destroy_SignPost();
   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
void LadarServer::decrease_annotation_size()
{
   cout << "inside LadarServer::decrease_annotation_size()" << endl;
   SignPostsGroup_ptr->change_size(0.5);
}

void LadarServer::increase_annotation_size()
{
   cout << "inside LadarServer::increase_annotation_size()" << endl;
   SignPostsGroup_ptr->change_size(2.0);
}

// ---------------------------------------------------------------------
void LadarServer::export_annotations()
{
   cout << "inside LadarServer::export_annotations()" << endl;
   string output_filename=SignPostsGroup_ptr->save_info_to_file();
   cout << "Annotation information written to " << output_filename << endl;
}

QByteArray LadarServer::import_annotations()
{
   cout << "inside LadarServer::import_annotations()" << endl;
   SignPostsGroup_ptr->read_info_from_file();
   return generate_JSON_response_to_annotation_event();
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_annotation_event()

QByteArray LadarServer::generate_JSON_response_to_annotation_event()
{
   cout << "Inside LadarServer::generate_JSON_response_to_annotation_event()"
        << endl;
   
   string json_string = "{  \n";

   json_string += " \"SignPosts\": [ ";

   int n_SignPosts=SignPostsGroup_ptr->get_n_Graphicals();
   cout << "n_SignPosts = " << n_SignPosts << endl;
   for (int n=0; n<n_SignPosts; n++)
   {
      SignPost* SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(n);
      int ID=SignPost_ptr->get_ID();
      threevector SignPost_posn;
      SignPost_ptr->get_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),SignPost_posn);
      string SignPost_label=SignPost_ptr->get_label();

      json_string += 
         " ["+stringfunc::number_to_string(ID)+","
	 +stringfunc::number_to_string(SignPost_posn.get(0))+","
         +stringfunc::number_to_string(SignPost_posn.get(1))+","
         +stringfunc::number_to_string(SignPost_posn.get(2))+", \""
         +SignPost_label+"\" ]";
      if (n < n_SignPosts-1) json_string += ",";
   } // loop over index n labeling SignPosts
   
   json_string += " ], \n ";
   json_string += " \"SelectedSignPostID\": "+
      stringfunc::number_to_string(
         SignPostsGroup_ptr->get_selected_Graphical_ID())+" \n";
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Simulated camera member functions
// ==========================================================================

void LadarServer::insert_simulated_camera()
{
//   cout << "inside LadarServer::insert_simulated_camera()" << endl;

   double X,Y,Z,az,el,roll;
   double horiz_FOV,vert_FOV,frustum_sidelength;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="camera_X")
      {
         X=stringfunc::string_to_number(value);
      }
      else if (key=="camera_Y")
      {
         Y=stringfunc::string_to_number(value);
      }
      else if (key=="camera_Z")
      {
         Z=stringfunc::string_to_number(value);
      }
      else if (key=="camera_az")
      {
         az=stringfunc::string_to_number(value);
      }
      else if (key=="camera_el")
      {
         el=stringfunc::string_to_number(value);
      }
      else if (key=="camera_roll")
      {
         roll=stringfunc::string_to_number(value);
      }
      else if (key=="horiz_FOV")
      {
         horiz_FOV=stringfunc::string_to_number(value);
      }
      else if (key=="vert_FOV")
      {
         vert_FOV=stringfunc::string_to_number(value);
      }
      else if (key=="frustum_sidelength")
      {
         frustum_sidelength=stringfunc::string_to_number(value);
      }
   } // loop over index k labeling key-value pairs

   threevector camera_posn(X,Y,Z);
   camera_posn += OBSFRUSTAGROUP_ptr->get_grid_world_origin();
   
//   cout << "Camera posn = " << camera_posn << endl;
//   cout << "Az = " << az 
//        << " El = " << el
//        << " Roll = " << roll << endl;
//   cout << "horiz FOV = " << horiz_FOV 
//        << " vert FOV = " << vert_FOV << endl;
//   cout << "Frustum sidelength = " << frustum_sidelength << endl;

   photogroup_ptr->destroy_all_photos();

// All dynamically instantiated cameras have been destroyed by
// previous call.  But STL vector of camera pointers within
// all Movies inside OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr() still
// needs to be cleared:

   OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr()->clear_all_camera_ptrs();

   OBSFRUSTAGROUP_ptr->destroy_all_OBSFRUSTA();

   photograph* photo_ptr=photogroup_ptr->generate_blank_photograph(
      horiz_FOV,vert_FOV,az,el,roll,camera_posn,frustum_sidelength);
   OBSFRUSTUM* virtual_OBSFRUSTUM_ptr=
      OBSFRUSTAGROUP_ptr->generate_virtual_OBSFRUSTUM(photo_ptr);

   int Nimages=1;
   AnimationController_ptr->set_nframes(Nimages);
   ModeController_ptr->setState(ModeController::MANIPULATE_FUSED_DATA);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::shoot_simulated_photo()
{
   cout << "inside LadarServer::shoot_simulated_photo()" << endl;
   
   set_alpha_percentage(0);
   ModeController_ptr->setState(ModeController::GENERATE_AVI_MOVIE);

   return capture_viewer_screen();
/*
   OBSFRUSTUM* virtual_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
      get_virtual_OBSFRUSTUM_ptr();
   if (virtual_OBSFRUSTUM_ptr==NULL)
   {
      return;
   }
   int virtual_OBSFRUSTUM_ID=virtual_OBSFRUSTUM_ptr->get_ID();
   OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(virtual_OBSFRUSTUM_ID);

   OBSFRUSTAGROUP_ptr->set_FOV_excess_fill_factor(1.0);
   OBSFRUSTAGROUP_ptr->flyto_camera_location(virtual_OBSFRUSTUM_ID);
   set_alpha_percentage(0);
   cout << "At end of LadarServer::shoot_simulated_photo()" << endl;
*/

}

// ---------------------------------------------------------------------
void LadarServer::set_alpha_percentage()
{
//   cout << "inside LadarServer::set_alpha_percentage()" << endl;

   double alpha=1;
   for (int k=0; k<KeyValue.size(); k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="alphaValue") alpha=0.01*stringfunc::string_to_number(value);
   }
   cout << "alpha = " << alpha << endl;
   set_alpha_percentage(alpha);
}

void LadarServer::set_alpha_percentage(double alpha)
{
   vector<Movie*> Movie_ptrs=OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();

   for (int i=0; i<int(Movie_ptrs.size()); i++)
   {
      Movie* Movie_ptr=Movie_ptrs[i];
//      cout << "i = " << i << " Movie_ptr = " << Movie_ptr << endl;
      if (Movie_ptr != NULL)
      {
         Movie_ptr->set_alpha(alpha);
      }
   } // loop over Movies within *Movie_ptrs_ptr
}



// ==========================================================================
// Plane manipulation member functions
// ==========================================================================

// Member function insert_world_plane()

QByteArray LadarServer::insert_world_plane()
{
   cout << "inside LadarServer::insert_world_plane()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_POLYLINE);

   PolyLinePickHandler_ptr->set_disable_input_flag(false);
   ROIPickHandler_ptr->set_disable_input_flag(true);

   while (!PolyLinesGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in LadarServer" << endl;
      WindowManager_ptr->process();
   }
   PolyLinesGroup_ptr->set_Geometricals_updated_flag(false);
   PolyLinesGroup_ptr->set_selected_Graphical_ID(
      PolyLinesGroup_ptr->get_most_recently_added_ID());

   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->
      get_ID_labeled_PolyLine_ptr(
         PolyLinesGroup_ptr->get_most_recently_added_ID());

   polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
   if (polyline_ptr->get_n_vertices() < 3)
   {
      string error_message="Need at least 3 points to define plane";
      return generate_error_JSON_response(error_message);
   }

   vector<threevector> V;
   for (int v=0; v<polyline_ptr->get_n_vertices(); v++)
   {
      V.push_back(polyline_ptr->get_vertex(v));
   }

   PolyLinesGroup_ptr->destroy_PolyLine(PolyLine_ptr);
   draw_world_plane(V);

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
// Member function draw_world_plane()

void LadarServer::draw_world_plane(const vector<threevector>& V)
{
   cout << "inside LadarServer::draw_world_plane()" << endl;

   threevector V_COM(Zero_vector);
   for (int v=0; v<V.size(); v++)
   {
      V_COM += V[v];
   }
   V_COM /= V.size();

   plane* plane_ptr=NULL;
   if (V.size()==3)
   {
      plane_ptr=new plane(V[0],V[1],V[2]);
   }
   else
   {
      plane_ptr=new plane(V);
   }
//   cout << "*plane_ptr = " << *plane_ptr << endl;

// Convert input threevetors from world to planar coordinates:

   twovector AB_COM(0,0);
   vector<twovector> AB;
   for (int v=0; v<V.size(); v++)
   {
      threevector Vplanar=plane_ptr->coords_wrt_plane(V[v]);
      AB.push_back(twovector(Vplanar));
      AB_COM += AB.back();
   }
   AB_COM /= AB.size();

   vector<double> radius;
   for (int v=0; v<V.size(); v++)
   {
      radius.push_back((AB[v]-AB_COM).magnitude());
   }

   int n_output_bins=20;
   prob_distribution r_probs(radius,n_output_bins);
   double r=r_probs.find_x_corresponding_to_pcum(0.75);

   int n_vertices=6;
   double dtheta=2*PI/n_vertices;
   vector<threevector> V_new;
   for (int v=0; v<n_vertices; v++)
   {
      double theta=v*dtheta;
      V_new.push_back(
         V_COM+r*cos(theta)*plane_ptr->get_ahat()
         +r*sin(theta)*plane_ptr->get_bhat());
   }
   delete plane_ptr;

// Generate translucent polygon in 3D world coordinates corresponding to 
// plane:

   colorfunc::Color polygon_color=colorfunc::pink;
   double alpha=0.33;
   bool draw_border_flag=true;
   PolygonsGroup_ptr->generate_translucent_Polygon(
      polygon_color,V_new,alpha,draw_border_flag);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::export_planes()
{
   cout << "inside LadarServer::export_planes()" << endl;
   PolyLinesGroup* PolyLinesGroup_ptr=PolygonsGroup_ptr->
      get_PolyLinesGroup_ptr();
   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr << endl;
   string output_filename="planes_3D.dat";
   cout << "output_filename = " << output_filename << endl;
   if (PolyLinesGroup_ptr != NULL)
   {
      PolyLinesGroup_ptr->save_info_to_file(output_filename);
      cout << "Plane information written to "+output_filename << endl;

   }
   return generate_JSON_response_to_geometrical_export(output_filename);
}

// ---------------------------------------------------------------------
QByteArray LadarServer::import_planes()
{
   cout << "inside LadarServer::import_planes()" << endl;
   string input_filename="planes_3D.dat";
   PolyLinesGroup* PolyLinesGroup_ptr=PolygonsGroup_ptr->
      get_PolyLinesGroup_ptr();

   PolyLinesGroup_ptr->destroy_all_PolyLines();
   PolygonsGroup_ptr->destroy_all_Polygons();

   PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(input_filename);
   cout << "*PolyLinesGroup_ptr = " << *PolyLinesGroup_ptr << endl;

   vector<vector<threevector> > V_all;
   vector<threevector> V;
   
   for (int p=0; p<PolyLinesGroup_ptr->get_n_Graphicals(); p++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(p);
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();      
      V.clear();
      for (int v=0; v<polyline_ptr->get_n_vertices(); v++)
      {
         V.push_back(polyline_ptr->get_vertex(v));
      }
      V_all.push_back(V);
   } // loop over index p labeling imported polylines

   PolyLinesGroup_ptr->destroy_all_PolyLines();

   for (int v=0; v<V_all.size(); v++)
   {
      draw_world_plane(V_all[v]);
   }
 
   return generate_JSON_response_to_polyline_event(input_filename);
}
