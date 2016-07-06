// ==========================================================================
// PHOTOSERVER class file
// ==========================================================================
// Last updated on 10/31/13; 11/3/13; 11/25/13; 6/7/14; 12/1/15
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "math/fourvector.h"
#include "astro_geo/geopoint.h"
#include "graphs/graphdbfuncs.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "graphs/jsonfuncs.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "numrec/nrfuncs.h"
#include "Qt/web/PhotoServer.h"
#include "geometry/polyline.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "general/stringfuncs.h"
#include "video/videosdatabasefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void PhotoServer::allocate_member_objects()
{
   data_ids_map_ptr=new DATA_IDS_MAP;
}		       

void PhotoServer::initialize_member_objects()
{
   bbox_ptr=NULL;
   Grid_ptr=NULL;

   ladar_point_cloud_suppressed_flag=false;
   JSON_filename="";
   region_height=region_width=-1;

   AnimationController_ptr=NULL;
   curr_graph_hierarchy_ptr=NULL;
   ModeController_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   Operations_ptr=NULL;
   PhotoToursGroup_ptr=NULL;
   PhotoTour_PolyLinesGroup_ptr=NULL;
   postgis_database_ptr=NULL;
   SignPostsGroup_ptr=NULL;
   SUBFRUSTAGROUP_ptr=NULL;
}

PhotoServer::PhotoServer(
   string host_IP_address,qint16 port, QObject* parent) :
   MessageServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
PhotoServer::~PhotoServer()
{
   delete data_ids_map_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray PhotoServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside PhotoServer:get() method" << endl;

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
//   for (unsigned int s=0; s<URL_substrings.size(); s++)
//   {
//      cout << "s = " << s
//           << " URL_substrings[s] = " << URL_substrings[s] << endl;
//   }

   string response_msg;

   if (URL_path=="/Set_Selected_Photo/")
   {
      set_selected_image_ID(response_msg);
   }
   else if (URL_path=="/Set_Region/")
   {
      set_regions(response_msg);
   }
   else if (URL_path=="/Delete_Region/")
   {
      delete_region(response_msg);
   }

// Thin client communications:

   else if (URL_path=="/Update_Messenger_Topic_Name/")
   {
      update_messenger_topic_name(response_msg);
   }
   else if (URL_path=="/Set_Cloud_Point_Size/")
   {
      set_cloud_point_size(response_msg);
   }
   else if (URL_path=="/Toggle_Grid_Visibility/")
   {
      toggle_grid(response_msg);
   }
   else if (URL_path=="/Set_Photo_Opacity/")
   {
      set_photo_opacity(response_msg);
   }
   else if (URL_path=="/Set_Photo_Zoom/")
   {
      set_photo_zoom(response_msg);
   }
   else if (URL_path=="/Reset_To_Default_View/")
   {
      reset_to_default_view();
   }

// Image annotation handling:
 
   else if (URL_path=="/Update_Annotations/")
   {
      return update_annotations();
   }
   else if (URL_path=="/Retrieve_Annotations/")
   {
      return retrieve_annotations();
   }
   else if (URL_path=="/Select_Annotated_Images/")
   {
      select_annotated_images();
   }
   else if (URL_path=="/Backproject_Annotations/")
   {
      backproject_annotations();
   }
   else if (URL_path=="/Clear_Backprojections/")
   {
      clear_backprojections();
   }

   else if (URL_path=="/Set_Points/")
   {
      set_points(response_msg);
   }
  else if (URL_path=="/Insert_3D_Annotation/")
   {
      insert_3D_annotation(response_msg);
   }

// Virtual tour event handling:

   else if (URL_path=="/Tour_Rewind/")
   {
      rewind_tourpath();
   }
   else if (URL_path=="/Tour_Forward/")
   {
      step_tour_forward();
   }
   else if (URL_path=="/Tour_Play/")
   {
      play_tour();
   }
   else if (URL_path=="/Tour_Pause/")
   {
      pause_tour();
   }
   else if (URL_path=="/Tour_Backward/")
   {
      step_tour_backward();
   }
   else if (URL_path=="/Clear_Tour_Path/")
   {
      clear_tourpath();
   }

// Dynamic JSON string generation via database query:

   else if (URL_path=="/Get_Node_Geoposns/")
   {
      get_node_geoposns(response_msg);
      return generate_JSON_response_to_parameters_request(response_msg);
   }
   else if (URL_path=="/Get_Node_Metadata/")
   {
      get_node_metadata(response_msg);
      return generate_JSON_response_to_parameters_request(response_msg);
   }
   else if (URL_path=="/Get_Node_Neighbors/")
   {
      get_node_neighbors(response_msg);
      return generate_JSON_response_to_parameters_request(response_msg);
   }
   else if (URL_path=="/Edit_Image_Caption/")
   {
      return edit_image_caption();
   }
   else if (URL_path=="/Get_Photo_By_ID/")
   {
      return get_photo_by_hierarchy_and_datum_IDs();
   }
   else if (URL_path=="/Get_Photo_By_URL/")
   {
      return get_photo_by_URL();
   }
   else if (URL_path=="/Get_Photo_By_Hierarchy_Level_Node/")
   {
      return get_photo_by_hierarchy_level_node();
   }
   else if (URL_path=="/Get_Sibling_Thumbnails/")
   {
      return get_sibling_thumbnails();
   }
   else if (URL_path=="/Get_Children_Thumbnails/")
   {
      return get_children_thumbnails();
   }
   else if (URL_path=="/Get_Parent_Thumbnail/")
   {
      return get_parent_thumbnail();
   }

// Camera query handling:

   else if (URL_path=="/Get_Camera_Metadata/")
   {
      return get_camera_metadata();
   }

// Graph query handling:

   else if (URL_path=="/Update_Hierarchy_Dropdown/")
   {
      return generate_all_graph_hierarchy_IDs_JSON_response();
   }
   else if (URL_path=="/Update_Selector_Metadata/")
   {
      return update_hierarchy_level_dropdowns();
   }
   else if (URL_path=="/Get_Graph_Hierarchy/")
   {
      return get_graph_hierarchy();
   }
   else if (URL_path=="/Get_Graph/")
   {
      return get_graph();
   }
   else if (URL_path=="/Redraw_Graph_Geometries/")
   {

// FAKE FAKE:  Sun Aug 11, 2013 at 11:19 am
// comment out next line for BIGLAPTOP running purposes

//      redraw_graph_geometries();
   }

   else if (URL_path=="/Get_Zeroth_Node_Given_Level/")
   {
      return get_zeroth_node_given_level();
   }


   else if (URL_path=="/Get_Connected_Graph_Components/")
   {
      return get_connected_graph_components();
   }

// Path finding query handling:

   else if (URL_path=="/Find_Best_Path/")
   {
      return find_best_path();
   }

// Image time query handling:

   else if (URL_path=="/Get_Median_Image_Time/")
   {
      return median_image_time();
   }
   else if (URL_path=="/Get_Selected_Image_Time/")
   {
      return selected_image_time();
   }
   else if (URL_path=="/Get_Extremal_Image_Times/")
   {
      return extremal_image_times();
   }
   else if (URL_path=="/Get_Image_Times/")
   {
      return get_image_times();
   }
   else if (URL_path=="/Get_Nearby_Image_Times/")
   {
      return get_nearby_image_times();
   }
   else if (URL_path=="/Select_Temporal_Neighbor/")
   {
      return select_temporal_neighbor();
   }

// Image geocoordinates query handling:

   else if (URL_path=="/Get_Image_Geocoordinates/")
   {
      return get_image_geocoordinates();
   }

// Image attribute query handling:

   else if (URL_path=="/Get_Image_Attributes/")
   {
      return get_image_attributes();
   }
   else if (URL_path=="/Select_Image_Attributes/")
   {
      select_image_attributes();
   }
   else if (URL_path=="/Clear_Color_Mapper/")
   {
      clear_color_mapper();
   }

// Image coloring query handling:

   else if (URL_path=="/Generate_Color_Histogram/")
   {
      return generate_color_histogram();
   }
   else if (URL_path=="/Restore_Original_Colors/")
   {
      return restore_original_colors();
   }
   else if (URL_path=="/Select_Dominant_Colors/")
   {
      select_dominant_colors();
   }

// Image human face query handling:

   else if (URL_path=="/Select_Human_Faces/")
   {
      select_human_faces();
   }
   else if (URL_path=="/Circle_Detected_Faces/")
   {
      return display_face_circles();
   }
   else if (URL_path=="/Get_N_Detected_Faces/")
   {
      return get_n_detected_faces();
   }

// Video keyframes query handling:

   else if (URL_path=="/Display_Video_Keyframes/")
   {
      display_video_keyframes();
   }


// SIFT matches query handling:

   else if (URL_path=="/Get_SIFT_Matches/")
   {
      return get_SIFT_matches();
   }

// 3D viewer screen capture event handling:

   else if (URL_path=="/Capture_Viewer_Screen/")
   {
      ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
         WindowManager_ptr);
      osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
         =ViewerManager_ptr->get_MyViewerEventHandler_ptr();

      ofstream outstream;
      string test_filename="/tmp/junk.dat";
      filefunc::openfile(test_filename,outstream);
      outstream << "sysfunc::get_loginname() = " << endl;
      outstream << sysfunc::get_loginname() << endl;

      string subdir="/home/"+sysfunc::get_loginname();      
      subdir += "/Desktop/movies_and_screen_shots/";
//      cout << "subdir = " << subdir << endl;
      string output_filename="viewer_screenshot_"
         +stringfunc::number_to_string(screenshot_counter++)+".png";
      string full_filename=subdir+output_filename;
//      cout << "full_filename = " << full_filename << endl;

      outstream << "full_filename = " << full_filename << endl;
      filefunc::closefile(test_filename,outstream);

      MyViewerEventHandler_ptr->setWriteImageFileName(full_filename);
      MyViewerEventHandler_ptr->setWriteImageOnNextFrame(true);           

      response_msg="Saved 3D viewer window into "+
         output_filename+" in movies_and_screen_shots folder on Desktop";
      return generate_JSON_response_to_parameters_request(response_msg);
   }

// Movie recording buttons event handling:

   else if (URL_path=="/Start_Recording_Movie/")
   {
      Start_Recording_Movie();
      return doc.toByteArray();
   }
   else if (URL_path=="/Stop_Recording_Movie/")
   {
      Stop_Recording_Movie();
      
      ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
         WindowManager_ptr);
      osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
         =ViewerManager_ptr->get_MyViewerEventHandler_ptr();
      while (MyViewerEventHandler_ptr->get_recording_flag())
      {
         WindowManager_ptr->process();
      }
      return generate_JSON_response_to_movie_request(
         MyViewerEventHandler_ptr->get_flv_movie_path());
   }
   else if (URL_substrings[0]=="/Retrieve" &&
            URL_substrings[1]=="_Screen" &&
            URL_substrings[2]=="_Capture")
   {
      string frame_time_str=URL_substrings[3].substr(1,5);
//      cout << "frame_time_str = " << frame_time_str << endl;
      double frame_time=stringfunc::string_to_number(frame_time_str);
//      cout << "Frame time = " << frame_time << endl;
      return generate_JSON_response_to_movie_frame_request(
         retrieve_movie_frame_file(frame_time));
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray PhotoServer::post(const QUrl& url, const QByteArray& postData,
                             QHttpResponseHeader& responseHeader)
{
//   cout << "inside PhotoServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   PhotoServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;
   if (URL_path=="/Set_Tour_Path/")
   {
      string error_msg;
      set_tour_path(postData,error_msg);
      ModeController_ptr->setState(ModeController::RUN_MOVIE);
      Grid_ptr->set_mask(0);

// Jump to starting photo position and then suppress ladar point cloud:

      PhotoTour* PhotoTour_ptr=PhotoToursGroup_ptr->get_PhotoTour_ptr(0);
      int starting_OBSFRUSTUM_ID=PhotoTour_ptr->get_starting_OBSFRUSTUM_ID();
      OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(
         starting_OBSFRUSTUM_ID,0);
      PhotoTour_ptr->set_prev_OBSFRUSTUM_ID(starting_OBSFRUSTUM_ID);
      suppress_ladar_point_cloud();

      return generate_JSON_response_to_tourpath_entry();
   }

   return doc.toByteArray();
}

// ==========================================================================
// JSON response member functions
// ==========================================================================

// ==========================================================================
// Photo selection member functions
// ==========================================================================

// Member function set_selected_image_ID()

bool PhotoServer::set_selected_image_ID(string& response_msg)
{
   cout << "inside PhotoServer::set_selected_image_ID()" << endl;

   unsigned int n_args=KeyValue.size();

   bool empty_fields_entry_flag=false;
   int image_ID=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      if (KeyValue[k].first=="PhotoID")
      {
         image_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   if (empty_fields_entry_flag)
   {
      response_msg="Insufficient number of parameters entered";
//      cout << "response_msg = " << response_msg << endl;
      return false;
   }

   if (image_ID < 0)
   {
      response_msg="Invalid Photo ID entered";
      return false;
   }
   int OBSFRUSTUM_ID=OBSFRUSTAGROUP_ptr->
      get_selected_photo_OBSFRUSTUM_ID(image_ID);
   cout << "Selected photo ID = " << image_ID << endl;
   cout << "Selected OBSFRUSTUM ID = " << OBSFRUSTUM_ID << endl;

   if (OBSFRUSTUM_ID < 0)
   {
      response_msg="No 3D frustum corresponds to entered Photo ID";
      return true;
   }

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=dynamic_cast<OBSFRUSTUM*>(
      OBSFRUSTAGROUP_ptr->get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID));
   selected_OBSFRUSTUM_ptr->set_selected_color(colorfunc::red);
   OBSFRUSTAGROUP_ptr->reset_colors();
   OBSFRUSTAGROUP_ptr->set_cross_fading_flag(true);
   OBSFRUSTAGROUP_ptr->set_display_Pyramids_flag(false);
   OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(OBSFRUSTUM_ID);

// Jump to starting photo position and then suppress ladar point cloud:

   if (!ladar_point_cloud_suppressed_flag)
   {
      OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(OBSFRUSTUM_ID,0);
      suppress_ladar_point_cloud();
   }
   
   Movie* Movie_ptr=selected_OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr != NULL)
   {
      Movie_ptr->set_alpha(0.0);
   }

   response_msg="Selected photo ID is OK";
   return true;
}

// ==========================================================================
// Photo annotations member functions
// ==========================================================================

// Member function update_annotations()

QByteArray PhotoServer::update_annotations()
{
//   cout << "inside PhotoServer::update_annotations()" << endl;

   unsigned int n_args=KeyValue.size();
   vector<int> Uindex,Vindex,Labelindex;
   vector<double> U,V;
   vector<string> label;
   
   int hierarchy_ID,graph_level,node_ID;
   string topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;

      if (key=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(value);
         continue;
      }
      else if (key=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(value);
         continue;
      }
      else if (key=="NodeID")
      {
         node_ID=stringfunc::string_to_number(value);
         continue;
      }
      else if (key=="TopicName")
      {
         topic=value; 
         continue;
      }

      string alpha_string;
      double number;
      stringfunc::decompose_alpha_numer_string(key,alpha_string,number);

//      cout << "k = " << k << " key = " << key 
//           << " alpha_string = " << alpha_string
//           << " number = " << number << endl;
//      cout << " value = " << value << endl;

      if (alpha_string=="U")
      {
         Uindex.push_back(int(number));
         U.push_back(stringfunc::string_to_number(value));
      }
      else if (alpha_string=="V")
      {
         Vindex.push_back(int(number));
         V.push_back(stringfunc::string_to_number(value));
      }
      else if (alpha_string=="Label")
      {
         Labelindex.push_back(int(number));
         label.push_back(value);
      }

   } // loop over index k labeling KeyValue key possibilities

   templatefunc::Quicksort(Uindex,U);
   templatefunc::Quicksort(Vindex,V);
   templatefunc::Quicksort(Labelindex,label);

   cout << "Hierarchy_ID = " << hierarchy_ID
        << " graph_level = " << graph_level
        << " node_ID = " << node_ID << endl;
   cout << "topic = " << topic << endl;
   
   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,hierarchy_ID,campaign_ID,mission_ID);

   int datum_ID=graphdbfunc::
      retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
         postgis_database_ptr,hierarchy_ID,node_ID);
   int image_ID=imagesdatabasefunc::get_image_ID(
      postgis_database_ptr,datum_ID);

//   cout << "campaign_ID = " << campaign_ID
//        << " mission_ID = " << mission_ID 
//        << " image_ID = " << image_ID
//        << " datum_ID = " << datum_ID
//        << endl;
//   for (unsigned int i=0; i<U.size(); i++)
//   {
//      cout << "i = " << i << " U = " << U[i]
//           << " V = " << V[i] << " label = " << label[i] << endl;
//   }

   imagesdatabasefunc::insert_image_annotations(
      postgis_database_ptr,
      campaign_ID,mission_ID,image_ID,datum_ID,U,V,label);

   int n_annotations=U.size();
   
   string json_string="{'";
   json_string += stringfunc::number_to_string(n_annotations);
   json_string += " annotations inserted into database'}";
//   cout << "json_string = " << json_string << endl;

   vector<int> node_IDs=
      graphdbfunc::retrieve_node_IDs_for_particular_graph_hierarchy_and_datum(
         postgis_database_ptr,hierarchy_ID,datum_ID);
   issue_update_annotations_message(n_annotations,hierarchy_ID,node_IDs,topic);

   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function retrieve_annotations()

QByteArray PhotoServer::retrieve_annotations()
{
//   cout << "inside PhotoServer::retrieve_annotations()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "node_ID = " << node_ID << endl;
//   cout << "topic = " << topic << endl;

   vector<twovector> UV;
   vector<string> label;
   imagesdatabasefunc::retrieve_image_annotations_from_database(
      postgis_database_ptr,hierarchy_ID,node_ID,UV,label);

   string json_string = "{  \n";
   json_string += " 'AnnotationPoints': [ ";

   for (unsigned int n=0; n<UV.size(); n++)
   {
      json_string += 
         " ["+stringfunc::number_to_string(UV[n].get(0))
         +","+stringfunc::number_to_string(UV[n].get(1))
         +",'"+label[n]+"' ]";
      if (n < UV.size()-1) json_string += ",";
   } // loop over index n labeling annotations
   
   json_string += " ] \n }";
   json_string += "\n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_annotated_images()

void PhotoServer::select_annotated_images()
{
//   cout << "inside PhotoServer::select_annotated_images()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic,client_name="";
   vector<int> n_annotations;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="n_annots1")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_annotations.push_back(1);
      }
      else if (KeyValue[k].first=="n_annots2")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_annotations.push_back(2);
      }
      else if (KeyValue[k].first=="n_annots3")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_annotations.push_back(3);
      }
      else if (KeyValue[k].first=="n_annots4")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_annotations.push_back(4);
      }
      else if (KeyValue[k].first=="n_annots5")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_annotations.push_back(5);
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << "hierarchy_ID = " << hierarchy_ID << endl;
   cout << "topic = " << topic << endl;

   string command="SET_CONDITION_COLOR_MAPPER";
   string key="JSON_string";

   colorfunc::RGB unselected_attribute_RGB(0.5 , 0.5 , 0.5);
   colorfunc::RGB selected_attribute_RGB(1.0 , 0.0, 0.0);

   string value=jsonfunc::generate_multi_object_selection_JSON_string(
      n_annotations,"annotations",
      selected_attribute_RGB,unselected_attribute_RGB);
//   cout << "json_string = " << value << endl;

   issue_message(command,key,value,topic,client_name);
}

// ---------------------------------------------------------------------
// Member function backproject_annotations()

void PhotoServer::backproject_annotations()
{
//   cout << "inside PhotoServer::backproject_annotations()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "node_ID = " << node_ID << endl;
//   cout << "topic = " << topic << endl;

   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,hierarchy_ID,campaign_ID,mission_ID);
//   cout << "campaignID = " << campaign_ID << " missionID = " << mission_ID
//        << endl;

   int datum_ID=graphdbfunc::
      retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
         postgis_database_ptr,hierarchy_ID,node_ID);
   int image_ID=imagesdatabasefunc::get_image_ID(
      postgis_database_ptr,datum_ID);
//   cout << "datum_ID = " << datum_ID 
//        << " image_ID = " << image_ID << endl;
   
   threevector camera_posn,az_el_roll,f_u0_v0;
   bool sensor_metadata_flag=
      imagesdatabasefunc::retrieve_particular_sensor_metadata_from_database(
         postgis_database_ptr,campaign_ID,mission_ID,image_ID,
         camera_posn,az_el_roll,f_u0_v0);

   if (!sensor_metadata_flag)
   {
      // return some error message to thin client in form of JSON string
   }
   
//   cout << "camera_posn = " << camera_posn << endl;
//   cout << "az_el_roll = " << az_el_roll << endl;
//   cout << "f_u0_v0 = " << f_u0_v0 << endl;
//   cout << "sensor_metadata_flag = " << sensor_metadata_flag << endl;
   
   double f=f_u0_v0.get(0);
   double u0=f_u0_v0.get(1);
   double v0=f_u0_v0.get(2);
   double az=az_el_roll.get(0)*PI/180;		// radians
   double el=az_el_roll.get(1)*PI/180;		// radians
   double roll=az_el_roll.get(2)*PI/180;	// radians
   
   camera* camera_ptr=new camera(f,f,u0,v0);
   camera_ptr->set_world_posn(camera_posn);
   camera_ptr->set_Rcamera(az,el,roll);
   camera_ptr->construct_projection_matrix();
//   cout << "*camera_ptr = " << *camera_ptr << endl;

   vector<twovector> UV;
   vector<string> label;
   imagesdatabasefunc::retrieve_image_annotations_from_database(
      postgis_database_ptr,hierarchy_ID,node_ID,UV,label);

   for (unsigned int i=0; i<UV.size(); i++)
   {
      threevector r_hat=camera_ptr->pixel_ray_direction(UV[i]);
//      cout << "i = " << i 
//           << " UV = " << UV[i] 
//           << " r_hat = " << r_hat << endl;
      issue_curr_backprojection_message(
         hierarchy_ID,node_ID,topic,client_name,camera_posn,r_hat,label[i]);
   }
   delete camera_ptr;
}

// ---------------------------------------------------------------------
// Member function clear_backprojections()

void PhotoServer::clear_backprojections()
{
   cout << "inside PhotoServer::clear_backprojections()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "topic = " << topic << endl;

}

// ---------------------------------------------------------------------
// Member function set_points()

bool PhotoServer::set_points(string& response_msg)
{
//   cout << "inside PhotoServer::set_points()" << endl;

   if (WindowManager_ptr==NULL) 
   {
      cout << "Error in PhotoServer::set_points()" << endl;
      cout << "window_mgr_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   unsigned int n_args=KeyValue.size();
   double horiz_FOV;
   for (unsigned int k=0; k<n_args; k++)
   {
      cout << "k = " << k 
           << " KeyValue[k].first = " << KeyValue[k].first 
           << " KeyValue[k].second = " << KeyValue[k].second
           << endl;
      if (KeyValue[k].first=="Zoom")
      {
         horiz_FOV=stringfunc::string_to_number(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities

   return true;
}

// ---------------------------------------------------------------------
// Member function insert_3D_annotation()

bool PhotoServer::insert_3D_annotation(string& response_msg)
{
   cout << "inside PhotoServer::insert_3D_annotation()" << endl;

   unsigned int n_args=KeyValue.size();

   if (SignPostsGroup_ptr==NULL) 
   {
      cout << "Error in PhotoServer::insert_3D_annotation()" << endl;
      cout << "SignPostsGroup_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   bool empty_fields_entry_flag=false;
   string AnnotationLabel;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      if (KeyValue[k].first=="AnnotationLabel")
      {
         AnnotationLabel=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities

   ModeController_ptr->setState(ModeController::INSERT_ANNOTATION);

   response_msg="Selected photo ID is OK";
   return true;
}

/*
// ---------------------------------------------------------------------
// Member function move_3D_annotation()

bool PhotoServer::move_3D_annotation(string& response_msg)
{
   cout << "inside PhotoServer::move_3D_annotation()" << endl;

   unsigned int n_args=KeyValue.size();

   if (SignPostsGroup_ptr==NULL) 
   {
      cout << "Error in PhotoServer::move_3D_annotation()" << endl;
      cout << "SignPostsGroup_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   bool empty_fields_entry_flag=false;
   double X,Y,Z;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      if (KeyValue[k].first=="AnnotationX")
      {
         X=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="AnnotationY")
      {
         Y=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="AnnotationZ")
      {
         Z=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities

   ModeController_ptr->setState(ModeController::INSERT_ANNOTATION);

   response_msg="Selected photo ID is OK";
   return true;
}
*/

// ---------------------------------------------------------------------
// Member function set_regions() extracts a 2D bounding box selected
// within the current image plane.  It also searches for a physical
// bounding box height and width specified in meters.

bool PhotoServer::set_regions(string& response_msg)
{
   cout << "inside PhotoServer::set_regions()" << endl;

   if (bbox_ptr==NULL)
   {
      cout << "Error in PhotoServer::set_regions()" << endl;
      cout << "bbox_ptr = NULL!" << endl;
      return false;
   }

   unsigned int n_args=KeyValue.size();
   bool empty_fields_entry_flag=false;
   string AnnotationLabel;
   int Region_ID=-1;
   double u_hi,u_lo,v_hi,v_lo;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      cout << "k = " << k 
           << " KeyValue[k].first = " << KeyValue[k].first 
           << " KeyValue[k].second = " << KeyValue[k].second
           << endl;

      if (KeyValue[k].first=="id")
      {
         Region_ID=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="bottom")
      {
         v_lo=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="top")
      {
         v_hi=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="left")
      {
         u_lo=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="right")
      {
         u_hi=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="height")
      {
         region_height=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="width")
      {
         region_width=stringfunc::string_to_number(KeyValue[k].second);
      }

   } // loop over index k labeling KeyValue key possibilities

   cout << "Region_ID = " << Region_ID << endl;
   cout << "u_lo = " << u_lo << " u_hi = " << u_hi << endl;
   cout << "v_lo = " << v_lo << " v_hi = " << v_hi << endl;
   cout << "height = " << region_height 
        << " width = " << region_width << endl;
   *bbox_ptr=bounding_box(u_lo,u_hi,v_lo,v_hi);

   cout << "dU/dV = " << (u_hi-u_lo)/(v_hi-v_lo) << endl;

//   region_height=0.241;	// Average head height = 24.1 cm ?
   region_height=8*2.54/100.0;	// Karl says his box height is 8" in size
   bbox_ptr->set_physical_deltaX(region_width);
   bbox_ptr->set_physical_deltaY(region_height);

//   cout << "*bbox_ptr = " << *bbox_ptr << endl;
   SUBFRUSTAGROUP_ptr->set_new_subfrustum_ID(Region_ID);

// Erase all OBSFRUSTA other than selected one:

   int selected_OBSFRUSTUM_ID=OBSFRUSTAGROUP_ptr->
      get_selected_Graphical_ID();
//      get_most_recently_selected_ID();
   if (selected_OBSFRUSTUM_ID >= 0)
   {
      OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
      OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(selected_OBSFRUSTUM_ID);
   }

   return true;
}

// ==========================================================================
// 3D viewer controls member functions
// ==========================================================================

// Member function set_cloud_point_size()

bool PhotoServer::set_cloud_point_size(string& response_msg)
{
//   cout << "inside PhotoServer::set_cloud_point_size()" << endl;

   if (PointCloudsGroup_ptr==NULL) 
   {
      cout << "Error in PhotoServer::set_cloud_point_size()" << endl;
      cout << "PointCloudsGroup_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   unsigned int n_args=KeyValue.size();
   bool empty_fields_entry_flag=false;
   int point_size=2;
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      if (KeyValue[k].first=="Size")
      {
         point_size=stringfunc::string_to_integer(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "point_size = " << point_size << endl;
   PointCloudsGroup_ptr->set_pt_size(point_size);

   return true;
}

// ---------------------------------------------------------------------
// Member function toggle_grid()

bool PhotoServer::toggle_grid(string& response_msg)
{
//   cout << "inside PhotoServer::toggle_grid()" << endl;

   if (Grid_ptr==NULL) 
   {
      cout << "Error in PhotoServer::toggle_grid()" << endl;
      cout << "Grid_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   unsigned int n_args=KeyValue.size();
   bool empty_fields_entry_flag=false;
   bool show_grid_flag=true;
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      if (KeyValue[k].first=="ShowGrid")
      {
         show_grid_flag=stringfunc::string_to_boolean(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "show_grid_flag = " << show_grid_flag << endl;
   if (show_grid_flag)
   {
      Grid_ptr->set_mask(1);
   }
   else
   {
      Grid_ptr->set_mask(0);
   }
   
   return true;
}

// ---------------------------------------------------------------------
// Member function set_photo_opacity()

bool PhotoServer::set_photo_opacity(string& response_msg)
{
//   cout << "inside PhotoServer::set_photo_opacity()" << endl;

   if (OBSFRUSTAGROUP_ptr==NULL) 
   {
      cout << "Error in PhotoServer::set_photo_opacity()" << endl;
      cout << "OBSFRUSTAGROUP_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   unsigned int n_args=KeyValue.size();
   bool empty_fields_entry_flag=false;
   double alpha=1;
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      if (KeyValue[k].first=="Opacity")
      {
         alpha=0.01*stringfunc::string_to_number(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "alpha = " << alpha << endl;

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
      get_selected_OBSFRUSTUM_ptr();
   if (selected_OBSFRUSTUM_ptr==NULL) return false;

   Movie* Movie_ptr=selected_OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr==NULL) return false;

   Movie_ptr->set_alpha(alpha);
//   cout << "Movie alpha = " << Movie_ptr->get_alpha() << endl;
   
   return true;
}

// ---------------------------------------------------------------------
// Member function set_photo_zoom()

bool PhotoServer::set_photo_zoom(string& response_msg)
{
//   cout << "inside PhotoServer::set_photo_zoom()" << endl;

   if (WindowManager_ptr==NULL) 
   {
      cout << "Error in PhotoServer::set_photo_zoom()" << endl;
      cout << "window_mgr_ptr=NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

   unsigned int n_args=KeyValue.size();
   double horiz_FOV;
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      if (KeyValue[k].first=="Zoom")
      {
         horiz_FOV=stringfunc::string_to_number(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "horiz_FOV = " << horiz_FOV << endl;

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
      get_selected_OBSFRUSTUM_ptr();
   if (selected_OBSFRUSTUM_ptr==NULL) return false;

   Movie* Movie_ptr=selected_OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr==NULL) return false;

   double curr_horiz_FOV=WindowManager_ptr->get_lens_horizontal_FOV();
   double angular_scale_factor=horiz_FOV/curr_horiz_FOV;
   WindowManager_ptr->rescale_viewer_FOV(angular_scale_factor);

   return true;
}

// ---------------------------------------------------------------------
// Member function reset_to_default_view()

bool PhotoServer::reset_to_default_view()
{
   cout << "inside PhotoServer::reset_to_default_view()" << endl;

   CM_3D_ptr->reset_view_to_home();
   return true;
}

// ==========================================================================
// PhotoTour member functions
// ==========================================================================

// Member function set_tour_path() parses post data coming from
// Jennifer Drexler's thin client which transmits a LINESTRING
// containing longitude,latitude waypoint coordinates.  This method
// uses the input data to generate a new polyline.  It returns the
// tour path length in meters.

void PhotoServer::set_tour_path(const QByteArray& postData,string& error_msg)
{
   cout << "inside PhotoServer::set_tour_path()" << endl;

   QUrl tmp_url;
   QString tmp_qstring=tmp_url.fromPercentEncoding(postData);
   string post_data=tmp_qstring.toStdString();
//   cout << "post_data = " << post_data << endl;
//   cout << "post_data.size() = " << post_data.size() << endl;

   string linestring=stringfunc::substring_between_substrings(
      post_data,"(",")");
   linestring=stringfunc::find_and_replace_char(linestring,"(","");
   linestring=stringfunc::find_and_replace_char(linestring,")","");
//   cout << "linestring = " << linestring << endl;
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      linestring,",");

   vector<threevector> V;
//   cout << "substrings.size() = " << substrings.size() << endl;

   for (unsigned int i=0; i<substrings.size(); i++)
   {
//      cout << substrings[i] << endl;
      vector<double> longlat=stringfunc::string_to_numbers(substrings[i],"+");
//      cout << "longlat[0] = " << longlat[0] << " longlat[1] = " << longlat[1]
//           << endl;

      geopoint curr_tourpath_geopoint(
         longlat[0],longlat[1],0,specified_UTM_zonenumber);
//      cout << "tour path geopoint = " << curr_tourpath_geopoint << endl;
      V.push_back(curr_tourpath_geopoint.get_UTM_posn());
   } // loop over index i labeling client input substrings

   clear_tourpath();

// We cannot form a genuine flightpath unless V.size() >= 2!

   if (V.size() <= 1)
   {
      if (substrings.size() > 0)
      {
         error_msg += "Need at least 2 valid waypoints to form flight path.";
      }
   }

//   cout << "V.size() = " << V.size() << endl;
//   for (unsigned int i=0; i<V.size(); i++)
//   {
//      cout << "i = " << i << " V[i] = " << V[i] << endl;
//   }

   PolyLine* curr_PolyLine_ptr=
      PhotoTour_PolyLinesGroup_ptr->generate_new_PolyLine(V.back(),V);

// Hide PhotoTour PolyLine:

   PhotoTour_PolyLinesGroup_ptr->erase_Graphical(
      curr_PolyLine_ptr->get_ID());

//   cout << "*curr_PolyLine_ptr = " << *curr_PolyLine_ptr << endl;
//   cout << "PhotoTour_PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << PhotoTour_PolyLinesGroup_ptr->get_n_Graphicals() << endl;

//   curr_PolyLine_ptr->add_vertex_points(
//      V,curr_PolyLine_ptr->get_selected_color());
//   curr_PolyLine_ptr->set_entry_finished_flag(true);

   polyline* polyline_ptr=curr_PolyLine_ptr->get_or_set_polyline_ptr();
//   cout << "*polyline_ptr = " << *polyline_ptr << endl;

   PhotoToursGroup_ptr->destroy_all_PhotoTours();
   PhotoTour* PhotoTour_ptr=PhotoToursGroup_ptr->generate_new_PhotoTour();
   PhotoTour_ptr->construct_camera_path(
      polyline_ptr,PhotoToursGroup_ptr->get_camera_posns_kdtree_ptr());
}

// ---------------------------------------------------------------------
// Member function rewind_tourpath()

void PhotoServer::rewind_tourpath()
{
   cout << "inside PhotoServer::rewind_tourpath()" << endl;
   AnimationController_ptr->set_curr_framenumber(0);
   OBSFRUSTAGROUP_ptr->set_prev_OBSFRUSTUM_framenumber(-1);
}

// ---------------------------------------------------------------------
// Member function play_tour()

void PhotoServer::play_tour()
{
   cout << "inside PhotoServer::play_tour()" << endl;
   AnimationController_ptr->setState( AnimationController::PLAY );
}

// ---------------------------------------------------------------------
// Member function pause_tour()

void PhotoServer::pause_tour()
{
   cout << "inside PhotoServer::pause_tour()" << endl;
   AnimationController_ptr->setState( AnimationController::PAUSE );
}

// ---------------------------------------------------------------------
// Member function step_tour_forward()

void PhotoServer::step_tour_forward()
{
   cout << "inside PhotoServer::step_tour_forward()" << endl;
   display_next_frame(1);
}

// ---------------------------------------------------------------------
// Member function step_tour_backward()

void PhotoServer::step_tour_backward()
{
   cout << "inside PhotoServer::step_tour_forward()" << endl;
   display_next_frame(-1);
}

// ---------------------------------------------------------------------
// Member function clear_tourpath()

void PhotoServer::clear_tourpath()
{
   cout << "inside PhotoServer::clear_tourpath()" << endl;
     
   pause_tour();
   if (PhotoTour_PolyLinesGroup_ptr != NULL)
   {
      PhotoTour_PolyLinesGroup_ptr->destroy_all_PolyLines();
   } 
   AnimationController_ptr->set_curr_framenumber(0);
   PhotoToursGroup_ptr->destroy_all_PhotoTours();
   Grid_ptr->set_mask(1);
   reset_to_default_view();
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_tourpath_entry() returns
// a JSON string to Jennifer Drexler's thin client which contains a
// LineString written in GEOJSON form corresponding to the actual tour
// through reconstructed cameras.

QByteArray PhotoServer::generate_JSON_response_to_tourpath_entry()
{
   cout << "Inside PhotoServer::generate_JSON_response_to_tourpath_entry()"
        << endl;

   if (PhotoToursGroup_ptr==NULL)
   {
      cout << "Error in PhotoServer::generate_JSON_response_to_tourpath_entry()" << endl;
      cout << "PhotoToursGroup_ptr = NULL!" << endl;
      exit(-1);
   }
   
   PhotoTour* PhotoTour_ptr=static_cast<PhotoTour*>(
      PhotoToursGroup_ptr->get_most_recently_added_Graphical_ptr());

   vector<int> TourPhotoIDs=PhotoTour_ptr->get_tour_photo_IDs();
   vector<threevector> TourPosns=PhotoTour_ptr->get_tour_posns();
   double tourpath_length=PhotoTour_ptr->get_tour_length();

   vector<geopoint> G;
   for (unsigned int n=0; n<TourPosns.size(); n++)
   {
      G.push_back(geopoint(northern_hemisphere_flag,specified_UTM_zonenumber,
                           TourPosns[n].get(0),TourPosns[n].get(1)));
   }
   
   string json_string = "{ \"type\": \"LineString\", \n";
   bool include_final_comma_flag=true;
   json_string += generate_JSON_tourpath_distance(
      tourpath_length,include_final_comma_flag);

   json_string += " \"tour_image_IDs\": [ ";
   for (unsigned int i=0; i<TourPhotoIDs.size(); i++)
   {
      int curr_image_ID=TourPhotoIDs[i];
      json_string += stringfunc::number_to_string(curr_image_ID);
      if (i < TourPhotoIDs.size()-1) json_string += ",";
   } // loop over index i labeling tour photo IDs
   json_string += " ], \n";
   json_string += " \"coordinates\": [ ";
   for (unsigned int n=0; n<G.size(); n++)
   {
      json_string += 
         " ["+stringfunc::number_to_string(G[n].get_longitude())
         +","+stringfunc::number_to_string(G[n].get_latitude())+" ]";
      if (n < G.size()-1) json_string += ",";
   } // loop over index n labeling geopoints within STL vector G
   
   json_string += " ] \n }";
   json_string += "\n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_tourpath_distance() returns the
// tour path distance in meters.

string PhotoServer::generate_JSON_tourpath_distance(
   double tourpath_length,bool include_final_comma_flag)
{
//   cout << "inside PhotoServer::generate_JSON_flight_distance()" << endl;
   string json_string = " \"pathlength\": \""+stringfunc::number_to_string(
      tourpath_length);

   if (include_final_comma_flag)
   {
      json_string += "\" , \n";
   }
   else
   {
      json_string += "\" \n";
   }
   return json_string;
}

// ---------------------------------------------------------------------
// Member function suppress_ladar_point_cloud() checks if more than
// one point cloud exists.  If so, we assume that the ladar point
// cloud has ID > 0.  This method erases any PointCloud with ID > 0.

void PhotoServer::suppress_ladar_point_cloud()
{
   if (PointCloudsGroup_ptr->get_n_Graphicals() > 1)
   {
      PointCloudsGroup_ptr->erase_all_Graphicals();
      PointCloudsGroup_ptr->unerase_Graphical(0);
   }
   ladar_point_cloud_suppressed_flag=true;
}

// ---------------------------------------------------------------------
// Member function delete_region()

bool PhotoServer::delete_region(string& response_msg)
{
   cout << "inside PhotoServer::delete_region()" << endl;

   unsigned int n_args=KeyValue.size();
   bool empty_fields_entry_flag=false;
   int SubRegion_ID=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      cout << "k = " << k 
           << " KeyValue[k].first = " << KeyValue[k].first 
           << " KeyValue[k].second = " << KeyValue[k].second
           << endl;

      if (KeyValue[k].first=="id")
      {
         SubRegion_ID=stringfunc::string_to_number(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << "SubRegion_ID = " << SubRegion_ID << endl;

   OBSFRUSTUM* SUBFRUSTUM_ptr=SUBFRUSTAGROUP_ptr->
      get_ID_labeled_OBSFRUSTUM_ptr(SubRegion_ID);
   cout << "SUBFRUSTUM_ptr = " << SUBFRUSTUM_ptr << endl;
   
   if (SUBFRUSTUM_ptr != NULL) 
   {
      SUBFRUSTAGROUP_ptr->destroy_OBSFRUSTUM(SUBFRUSTUM_ptr);
      SUBFRUSTAGROUP_ptr->null_SUBFRUSTUM_ptr();
   }
   
   SUBFRUSTAGROUP_ptr->set_Graphical_counter(
      SUBFRUSTAGROUP_ptr->get_Graphical_counter()+1);

   MODELSGROUP* human_MODELSGROUP_ptr=SUBFRUSTAGROUP_ptr->
      get_target_MODELSGROUP_ptr();
   if (human_MODELSGROUP_ptr != NULL)
      human_MODELSGROUP_ptr->destroy_all_MODELS();

   return true;
}

// ==========================================================================
// Dynamic JSON string generation via database query member functions
// ==========================================================================

bool PhotoServer::get_node_geoposns(std::string& response_msg)
{
/*
   vector<int> image_ID,photo_importance;
   vector<int> npx,npy,thumbnail_npx,thumbnail_npy;
   vector<string> image_filenames,image_timestamps,photo_URL,thumbnail_URL;
   vector<double> zposn,azimuth,elevation,roll,focal_param,
      longitude,latitude;
   
   photodbfunc::retrieve_photo_metadata_from_database(
      postgis_database_ptr,bundler_IO_subdir,
      image_ID,photo_importance,npx,npy,thumbnail_npx,thumbnail_npy,
      image_filenames,image_timestamps,photo_URL,thumbnail_URL,
      zposn,azimuth,elevation,roll,focal_param,longitude,latitude);

   response_msg=photodbfunc::generate_geolocation_JSON_string(
      image_ID,longitude,latitude);
*/

   return true;
}

bool PhotoServer::get_node_metadata(std::string& response_msg)
{
/*
   vector<int> image_ID,photo_importance;
   vector<int> npx,npy,thumbnail_npx,thumbnail_npy;
   vector<string> image_filenames,image_timestamps,photo_URL,thumbnail_URL;
   vector<double> zposn,azimuth,elevation,roll,focal_param,
      longitude,latitude;
   
   photodbfunc::retrieve_photo_metadata_from_database(
      postgis_database_ptr,bundler_IO_subdir,
      image_ID,photo_importance,npx,npy,thumbnail_npx,thumbnail_npy,
      image_filenames,image_timestamps,photo_URL,thumbnail_URL,
      zposn,azimuth,elevation,roll,focal_param,longitude,latitude);

   response_msg=photodbfunc::generate_metadata_JSON_string(
      get_requested_node_ID(),image_ID,npx,npy,
      image_filenames,image_timestamps,
      longitude,latitude,zposn,azimuth,elevation,roll);
*/

   return true;
}

bool PhotoServer::get_node_neighbors(std::string& response_msg)
{
   vector<int> neighbor_IDs=jsonfunc::retrieve_node_neighbors_from_database(
      postgis_database_ptr,get_requested_node_ID());

   response_msg=jsonfunc::generate_neighbor_JSON_string(
      get_requested_node_ID(),neighbor_IDs);
   return true;
}

int PhotoServer::get_requested_node_ID()
{
   unsigned int n_args=KeyValue.size();
   int requested_node_ID=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      if (KeyValue[k].first=="ID")
      {
         requested_node_ID=stringfunc::string_to_integer(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "requested_node_ID = " << requested_node_ID << endl;
   return requested_node_ID;
}

// ---------------------------------------------------------------------
// Member function get_photo_by_hierarchy_and_datum_IDs() returns a
// JSON string containing all metadata extracted from the images table
// of the IMAGERY database for a particular image specified by its
// integer datum ID.

QByteArray PhotoServer::get_photo_by_hierarchy_and_datum_IDs()
{
   cout << "inside PhotoServer::get_photo_by_hierarchy_and_datum_IDs() #1" 
        << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,datum_ID,graph_level;
   hierarchy_ID=datum_ID=graph_level=-1;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         datum_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }

   } // loop over index k labeling KeyValue key possibilities

   cout << "HierarchyID = " << hierarchy_ID
        << " datum_ID = " << datum_ID
        << " GraphLevel = " << graph_level << endl;
   cout << "client_name = " << client_name << endl;
   cout << "topic = " << topic << endl;

   if (datum_ID < 0)
   {
      int zeroth_node_ID=graphdbfunc::get_zeroth_node_given_level(
         postgis_database_ptr,hierarchy_ID,0);
//      cout << "zeroth_node_ID = " << zeroth_node_ID << endl;

      datum_ID=graphdbfunc::
         retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
            postgis_database_ptr,hierarchy_ID,zeroth_node_ID);
//      cout << "datum_ID = " << datum_ID << endl;
   }
   return get_photo_by_hierarchy_and_datum_IDs(
      hierarchy_ID,graph_level,datum_ID,topic,client_name);
}

// ---------------------------------------------------------------------
QByteArray PhotoServer::get_photo_by_hierarchy_and_datum_IDs(
   int hierarchy_ID,int graph_level,int datum_ID,
   string topic,string client_name)
{
//   cout << "inside PhotoServer::get_photo_by_hierarchy_and_datum_IDs() #2" 
//        << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID 
//        << " graph_level = " << graph_level 
//        << " datum_ID = " << datum_ID << endl;
//   cout << "topic = " << topic << " client_name = " << client_name << endl;

// Reset Graph Viewer coloring to default scheme based upon node
// clusterings rather than by image attributes whenever a new
// Hierarchy is selected in any client:

   clear_color_mapper(topic,client_name);

   int image_ID,npx,npy,thumbnail_npx,thumbnail_npy,importance;
   double image_epoch;
   string image_timestamp,image_URL,thumbnail_URL;

   bool valid_image_ID_flag=
      imagesdatabasefunc::retrieve_particular_image_metadata_from_database(
         postgis_database_ptr,datum_ID,image_ID,
         importance,image_timestamp,image_epoch,
         image_URL,npx,npy,
         thumbnail_URL,thumbnail_npx,thumbnail_npy);

   if (!valid_image_ID_flag)
   {
      string response_msg="No image corresponding to input ID found in images table of IMAGERY database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }
   else
   {
      int node_ID;
      graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum(
         postgis_database_ptr,hierarchy_ID,graph_level,datum_ID,node_ID);

//      cout << "hierarchy_ID = " << hierarchy_ID
//           << "graph_level = " << graph_level << endl;
//      cout << "Output node_ID = " << node_ID << endl;

      if (hierarchy_ID < 0 || node_ID < 0)
      {
         cout << "Error in PhotoServer::get_photo_by_hierarchy_and_datum_IDs() #2"
              << endl;
         cout << "hierarchy_ID = " << hierarchy_ID
              << " node_ID = " << node_ID << endl;
         exit(-1);
      }
      
      string image_caption;
      graphdbfunc::retrieve_node_label_from_database(
         postgis_database_ptr,hierarchy_ID,node_ID,image_caption);

      if (topic.size() > 0)
      {
         issue_update_curr_node_message(
            hierarchy_ID,graph_level,node_ID,datum_ID,topic,client_name);
      }
      
      return generate_JSON_response_to_requested_photo(
         hierarchy_ID,graph_level,node_ID,datum_ID,
         image_caption,image_timestamp,image_URL,npx,npy,
         thumbnail_URL,thumbnail_npx,thumbnail_npy,importance);
   }
}

// ---------------------------------------------------------------------
// Member function get_photo_by_URL() returns a JSON string
// containing all metadata extracted from the photos table of the TOC
// database for a particular photo specified by its URL string.

QByteArray PhotoServer::get_photo_by_URL()
{
//   cout << "inside PhotoServer::get_photo_by_URL()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,graph_level;
   hierarchy_ID=graph_level=-1;
   string URL,topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="URL")
      {
         URL=stringfunc::get_reduced_URL(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "URL = " << URL << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "graph_level = " << graph_level << endl;

   if (hierarchy_ID < 0 || graph_level < 0)
   {
      cout << "Error in Photoserver::get_photo_by_URL!()" << endl;
      exit(-1);
   }

   if (URL.size()==0)
   {
      string response_msg="Invalid URL entered";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   int datum_ID=imagesdatabasefunc::get_datum_ID(
      postgis_database_ptr,hierarchy_ID,URL);
//   cout << "Retrieved datum_ID = " << datum_ID << endl;
   if (datum_ID==-1) 
   {
      string response_msg="No image corresponding to input URL found in images table of IMAGERY database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   return get_photo_by_hierarchy_and_datum_IDs(
      hierarchy_ID,graph_level,datum_ID,topic);
}

// ---------------------------------------------------------------------
// Member function get_photo_by_hierarchy_level_node() generates a JSON
// string containing all metadata extracted from the images table of
// the IMAGERY database for a particular image specified by its graph
// hierarchy ID, graph level and node ID.

QByteArray PhotoServer::get_photo_by_hierarchy_level_node()
{
//   cout << "inside PhotoServer::get_photo_by_hierarchy_level_node()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,graph_level,node_ID;
   string topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " graph_level = " << graph_level
//        << " node_ID = " << node_ID << endl;

   int datum_ID=
      graphdbfunc::retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
         postgis_database_ptr,hierarchy_ID,node_ID);
//   cout << "Output datum_ID = " << datum_ID << endl;

   return get_photo_by_hierarchy_and_datum_IDs(
      hierarchy_ID,graph_level,datum_ID,topic);
}

// ---------------------------------------------------------------------
// Member function get_neighbor_thumbnails() takes in a database ID for
// some image within the images table of the IMAGERY database.  It
// retrieves the database IDs, URLs and thumbnail URLs for all
// neighbors of the input image.  This method returns a JSON string
// containing an array of neighbor metadata for the input image.

QByteArray PhotoServer::get_neighbor_thumbnails()
{
//   cout << "inside PhotoServer::get_neighbor_thumbnails()" << endl;

   if(curr_graph_hierarchy_ptr == NULL)
   {
      string response_msg=
         "Graph hierarchy has not yet been instantiated!";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,datum_ID,graph_level;
   hierarchy_ID=datum_ID=graph_level=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         datum_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "graph_level = " << graph_level << endl;
//   cout << "datum_ID = " << datum_ID << endl;

   if (datum_ID==-1)
   {
      string response_msg=
         "No photo corresponding to input ID found in images table of IMAGERY database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   int node_ID=-1;
   graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum(
      postgis_database_ptr,hierarchy_ID,graph_level,datum_ID,node_ID);
//   cout << "node_ID = " << node_ID << endl;

   if (node_ID==-1)
   {
      cout << "Error in PhotoServer::get_neighbor_thumbnails()" << endl;
      cout << " node_ID = " << node_ID << endl;

      string response_msg=
         "No node ID found corresponding to datum ID";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   graph* graph_ptr=curr_graph_hierarchy_ptr->get_graph_ptr(graph_level);
   node* node_ptr = graph_ptr->get_node_ptr(node_ID);

   vector<int> neighbor_node_IDs=node_ptr->get_sorted_neighbor_node_IDs();
   vector<int> neighbor_data_IDs;
   for(int i = 0; i < neighbor_node_IDs.size(); i++)
   {
      neighbor_data_IDs.push_back(
         graphdbfunc::retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
            postgis_database_ptr,hierarchy_ID,neighbor_node_IDs[i]));
   }
   
// Make sure all thumbnail IDs within STL vector actually correspond
// to valid photos within the database:

   bool valid_thumbnail_IDs_flag=true;
   for (unsigned int i=0; i<neighbor_data_IDs.size(); i++)
   {
      if (!imagesdatabasefunc::image_in_database(
             postgis_database_ptr, neighbor_data_IDs[i])) 
         valid_thumbnail_IDs_flag=false;
   }

   if (!valid_thumbnail_IDs_flag)
   {
      string response_msg=
         "Not all thumbnails corresponding to carousel IDs found in photos table of database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   vector<string> carousel_URLs,carousel_thumbnail_URLs;
   for (unsigned int i=0; i<neighbor_data_IDs.size(); i++)
   {
      int datum_ID=neighbor_data_IDs[i];

      bool thumbnail_flag=false;
      string photo_URL=imagesdatabasefunc::get_image_URL(
         postgis_database_ptr,datum_ID,thumbnail_flag);
      carousel_URLs.push_back(photo_URL);
      
      thumbnail_flag=true;
      string thumbnail_URL=imagesdatabasefunc::get_image_URL(
         postgis_database_ptr,datum_ID,thumbnail_flag);
      carousel_thumbnail_URLs.push_back(thumbnail_URL);
   }

   return generate_JSON_response_to_requested_thumbnails(
      neighbor_data_IDs,carousel_URLs,carousel_thumbnail_URLs);
}

// ---------------------------------------------------------------------
// Member function get_sibling_thumbnails() takes in a database ID for
// some image within the images table of the IMAGERY database.  It
// retrieves the database IDs, URLs and thumbnail URLs for all
// siblings of the input image.  This method returns a JSON string
// containing an array of sibling metadata for the input image.

QByteArray PhotoServer::get_sibling_thumbnails()
{
   cout << "inside PhotoServer::get_sibling_thumbnails()" << endl;

// FAKE FAKE:  Tues Dec 1, 2015 at 8:40 am

   return get_neighbor_thumbnails();

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,datum_ID,graph_level;
   hierarchy_ID=datum_ID=graph_level=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         datum_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "graph_level = " << graph_level << endl;
//   cout << "datum_ID = " << datum_ID << endl;

   if (datum_ID==-1)
   {
      string response_msg=
         "No photo corresponding to input ID found in images table of IMAGERY database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   vector<int> sibling_data_IDs=
      graphdbfunc::retrieve_sibling_data_IDs_from_database(
         postgis_database_ptr,hierarchy_ID,graph_level,datum_ID);

// Make sure all thumbnail IDs within STL vector actually correspond
// to valid photos within the database:

   bool valid_thumbnail_IDs_flag=true;
   for (unsigned int i=0; i<sibling_data_IDs.size(); i++)
   {
//      cout << "i = " << i << " sibling_data_ID = " << sibling_data_IDs[i]
//           << endl;
      if (!imagesdatabasefunc::image_in_database(postgis_database_ptr,
                                                 sibling_data_IDs[i])) valid_thumbnail_IDs_flag=false;
   }

   if (!valid_thumbnail_IDs_flag)
   {
      string response_msg=
         "Not all thumbnails corresponding to carousel IDs found in photos table of database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   vector<string> carousel_URLs,carousel_thumbnail_URLs;
   for (unsigned int i=0; i<sibling_data_IDs.size(); i++)
   {
      int datum_ID=sibling_data_IDs[i];

      bool thumbnail_flag=false;
      string photo_URL=imagesdatabasefunc::get_image_URL(
         postgis_database_ptr,datum_ID,thumbnail_flag);
      carousel_URLs.push_back(photo_URL);
      
      thumbnail_flag=true;
      string thumbnail_URL=imagesdatabasefunc::get_image_URL(
         postgis_database_ptr,datum_ID,thumbnail_flag);
      carousel_thumbnail_URLs.push_back(thumbnail_URL);
   }

   return generate_JSON_response_to_requested_thumbnails(
     sibling_data_IDs,carousel_URLs,carousel_thumbnail_URLs);
}

// ---------------------------------------------------------------------
// Member function get_children_thumbnails() takes in a database ID
// for some photo within the photos table of the TOC database.  It
// retrieves the database IDs, URLs and thumbnail URLs for all
// children of the input photo.  This method returns a JSON string
// containing an array of sibling metadata for the input photo.

QByteArray PhotoServer::get_children_thumbnails()
{
//   cout << "inside PhotoServer::get_children_thumbnails()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,image_ID,graph_level;
   hierarchy_ID=image_ID=graph_level=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         image_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "graph_level = " << graph_level << endl;
//   cout << "image_ID = " << image_ID << endl;

   if (image_ID==-1)
   {
      string response_msg=
         "No photo corresponding to input ID found in photos table of database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }
   
   vector<int> children_data_IDs=
      graphdbfunc::retrieve_children_data_IDs_from_database(
         postgis_database_ptr,hierarchy_ID,graph_level,image_ID);
//   cout << "children_data_IDs.size() = " << children_data_IDs.size() << endl;

// Make sure all thumbnail IDs within STL vector actually correspond
// to valid photos within the database:

   bool valid_thumbnail_IDs_flag=true;
   for (unsigned int i=0; i<children_data_IDs.size(); i++)
   {
//      cout << "i = " << i << " children_data_ID = " << children_data_IDs[i]
//           << endl;
      if (!imagesdatabasefunc::image_in_database(postgis_database_ptr,
          children_data_IDs[i])) valid_thumbnail_IDs_flag=false;
   }

   if (!valid_thumbnail_IDs_flag)
   {
      string response_msg=
         "Not all thumbnails corresponding to carousel IDs found in photos table of database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   vector<string> carousel_URLs,carousel_thumbnail_URLs;
   for (unsigned int i=0; i<children_data_IDs.size(); i++)
   {
      int datum_ID=children_data_IDs[i];

      bool thumbnail_flag=false;
      string photo_URL=imagesdatabasefunc::get_image_URL(
         postgis_database_ptr,datum_ID,thumbnail_flag);
      carousel_URLs.push_back(photo_URL);
      
      thumbnail_flag=true;
      string thumbnail_URL=imagesdatabasefunc::get_image_URL(
         postgis_database_ptr,datum_ID,thumbnail_flag);
      carousel_thumbnail_URLs.push_back(thumbnail_URL);
   }
   return generate_JSON_response_to_requested_thumbnails(
     children_data_IDs,carousel_URLs,carousel_thumbnail_URLs);
}

// ---------------------------------------------------------------------
// Member function get_parent_thumbnail() takes in a database ID
// for some photo within the photos table of the TOC database.  It
// retrieves the database IDs, URLs and thumbnail URL for the unique
// parent of the input photo.  This method returns a JSON string
// containing an array of parental metadata for the input photo.

QByteArray PhotoServer::get_parent_thumbnail()
{
//   cout << "inside PhotoServer::get_parent_thumbnail()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,datum_ID,graph_level;
   hierarchy_ID=datum_ID=graph_level=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PhotoID")
      {
         datum_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "graph_level = " << graph_level << endl;
//   cout << "datum_ID = " << datum_ID << endl;

   if (datum_ID==-1)
   {
      string response_msg=
         "No photo corresponding to input ID found in photos table of database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }
   
   int parent_datum_ID=
      graphdbfunc::retrieve_parent_datum_ID_from_database(
         postgis_database_ptr,hierarchy_ID,graph_level,datum_ID);

// Make sure parent thumbnail ID actually corresponds to valid photo
// within the database:

   if (!imagesdatabasefunc::image_in_database(
      postgis_database_ptr,parent_datum_ID))
   {
      string response_msg=
         "Parent thumbnail not found in photos table of database";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   vector<int> parent_data_IDs;
   parent_data_IDs.push_back(parent_datum_ID);
   
   vector<string> carousel_URLs,carousel_thumbnail_URLs;

   datum_ID=parent_datum_ID;
   bool thumbnail_flag=false;
   string photo_URL=imagesdatabasefunc::get_image_URL(
      postgis_database_ptr,datum_ID,thumbnail_flag);
   carousel_URLs.push_back(photo_URL);
      
   thumbnail_flag=true;
   string thumbnail_URL=imagesdatabasefunc::get_image_URL(
      postgis_database_ptr,datum_ID,thumbnail_flag);
   carousel_thumbnail_URLs.push_back(thumbnail_URL);

   return generate_JSON_response_to_requested_thumbnails(
     parent_data_IDs,carousel_URLs,carousel_thumbnail_URLs);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_requested_photo()

QByteArray PhotoServer::generate_JSON_response_to_requested_photo(
   int hierarchy_ID,int graph_level,int node_ID,int datum_ID,
   string caption,string photo_timestamp,
   string photo_URL,int npx,int npy,
   string thumbnail_URL,int thumbnail_npx,int thumbnail_npy,
   int importance)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_requested_photo()"
//        << endl;
//   cout << "graph_level = " << graph_level << endl;
//   cout << "datum_ID = " << datum_ID << endl;
   
   string json_string = "{  \n";
   json_string += " 'hierarchy_ID': '"+stringfunc::number_to_string(
      hierarchy_ID)+"' , \n";
   json_string += " 'graph_level': '"+stringfunc::number_to_string(graph_level)
      +"' , \n";
   json_string += " 'node_ID': '"+stringfunc::number_to_string(node_ID)
      +"' , \n";
   json_string += " 'datum_ID': '"+stringfunc::number_to_string(datum_ID)
      +"' , \n";
   json_string += " 'image_caption': '"+caption+"' , \n";

   string image_date="";
   string image_time="";
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      photo_timestamp);
   if (substrings.size()==2)
   {
      image_date=substrings[0];
      image_time=substrings[1]+" UTC";
   }
   json_string += " 'Date': '"+image_date+"' , \n";
   json_string += " 'Time': '"+image_time+"', \n";

   json_string += " 'URL': '"+get_tomcat_URL_prefix()+photo_URL+"' , \n";
   json_string += " 'Npx': '"+stringfunc::number_to_string(npx)+"' , \n";
   json_string += " 'Npy': '"+stringfunc::number_to_string(npy)+"' , \n";
   json_string += " 'Thumbnail_URL': '"+get_tomcat_URL_prefix()+thumbnail_URL
      +"' , \n";
   json_string += " 'Thumbnail_Npx': '"
      +stringfunc::number_to_string(thumbnail_npx)+"' , \n";
   json_string += " 'Thumbnail_Npy': '"
      +stringfunc::number_to_string(thumbnail_npy)+"' , \n";
   json_string += " 'importance': '"
      +stringfunc::number_to_string(importance)+"' , \n";

   int curr_n_siblings=graphdbfunc::get_n_siblings_from_database(
      postgis_database_ptr,hierarchy_ID,graph_level,datum_ID);
//   cout << "curr_n_siblings = " << curr_n_siblings << endl;
   json_string += " 'n_siblings': '"
      +stringfunc::number_to_string(curr_n_siblings)+"', \n";

   int curr_n_children=graphdbfunc::get_n_children_from_database(
      postgis_database_ptr,hierarchy_ID,graph_level,datum_ID);
   json_string += " 'n_children': '"
      +stringfunc::number_to_string(curr_n_children)+"' \n";

   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_requested_thumbnails()

QByteArray PhotoServer::generate_JSON_response_to_requested_thumbnails(
   vector<int>& image_IDs,vector<string>& photo_URLs,
   vector<string>& thumbnail_URLs)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_requested_thumbnails()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " requestedPhotos: [ \n";
   for (unsigned int n=0; n<image_IDs.size(); n++)
   {
      json_string += "     { \n";
      json_string += "       photo_id: "+
         stringfunc::number_to_string(image_IDs[n])+", \n";
      json_string += "       URL: '"
         +get_tomcat_URL_prefix()+photo_URLs[n]+"', \n";
      json_string += "       thumbnail_URL: '"
         +get_tomcat_URL_prefix()+thumbnail_URLs[n]+"'\n";
      json_string += "     }";
      if (n < image_IDs.size()-1) json_string += ",";      
      json_string += " \n";
   }
   json_string += "   ] \n ";   
   json_string += "} \n";
   
//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_zeroth_node_ID()

QByteArray PhotoServer::generate_JSON_response_to_zeroth_node_ID(
   int zeroth_node_ID)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_zeroth_node_ID()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " zerothNodeID: "
      +stringfunc::number_to_string(zeroth_node_ID)+" \n";
   json_string += "} \n";
   
//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_invalid_entry()

QByteArray PhotoServer::generate_JSON_response_to_invalid_entry(
   string response_msg)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_invalid_entry()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " 'response_msg': '"+response_msg+"'  \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Camera query handling member functions
// ==========================================================================

// Member function get_camera_metadata()

QByteArray PhotoServer::get_camera_metadata()
{
   cout << "inside PhotoServer::get_camera_metadata()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << "hierarchy_ID = " << hierarchy_ID << endl;

   QByteArray JSON_response=
      generate_JSON_response_to_requested_camera_metadata(hierarchy_ID);
//   cout << "JSON_response = " << JSON_response.data() << endl;
   return JSON_response;
}

// ---------------------------------------------------------------------
// Member function
// generate_JSON_response_to_requested_camera_metadata() retrieves
// camera intrinsic and extrinsic parameters from either a postgres
// database table or package files.  It returns camera parameters
// within a JSON string for all sensors corresponding to the requested
// hierarchy_ID.

QByteArray PhotoServer::generate_JSON_response_to_requested_camera_metadata(
   int hierarchy_ID)
{
   cout << "Inside PhotoServer::generate_JSON_response_to_requested_camera_metadata()"
        << endl;
   cout << "hierarchy_ID = " << hierarchy_ID << endl;

// First determine if input hierarchy_ID exists within imagery database:

   bool hierarchy_ID_in_database_flag=false;
   vector<int> hierarchy_IDs;
   vector<string> hierarchy_descriptions;
   if (graphdbfunc::retrieve_hierarchy_IDs_from_database(
      postgis_database_ptr,hierarchy_IDs,hierarchy_descriptions))
   {
      for (unsigned int h=0; h<hierarchy_IDs.size(); h++)
      {
         if (hierarchy_ID==hierarchy_IDs[h])
         {
            hierarchy_ID_in_database_flag=true;
            break;
         }
      }
   }

   cout << "hierarchy_ID_in_database_flag = " 
        << hierarchy_ID_in_database_flag << endl;

   vector<int> image_ID;
   vector<double> FOV_u,FOV_v,U0,V0,az,el,roll;
   vector<double> camera_longitude,camera_latitude,camera_altitude,
      camera_easting,camera_northing,frustum_sidelength;
   vector<string> URL;

   if (hierarchy_ID_in_database_flag)
   {
      int campaign_ID,mission_ID;
      if (imagesdatabasefunc::get_campaign_mission_IDs(
         postgis_database_ptr,hierarchy_ID,campaign_ID,mission_ID))
      {
         cout << "campaign_ID = " << campaign_ID
              << " mission_ID = " << mission_ID << endl;
         if (imagesdatabasefunc::threeD_sensor_metadata_in_database(
            postgis_database_ptr,campaign_ID,mission_ID))
         {
            imagesdatabasefunc::retrieve_sensor_metadata_from_database(
               postgis_database_ptr,campaign_ID,mission_ID,
               image_ID,URL,FOV_u,FOV_v,U0,V0,az,el,roll,camera_longitude,
               camera_latitude,camera_altitude,frustum_sidelength);
         }
         else
         {
            imagesdatabasefunc::retrieve_sensor_metadata_from_database(
               postgis_database_ptr,campaign_ID,mission_ID,
               image_ID,URL,U0,V0,camera_longitude,camera_latitude);
         }
         cout << "image_ID.size() = " << image_ID.size() << endl;
         cout << "URL.size() = " << URL.size() << endl;
      }
   }
   else
   {
      string bundler_subdir="/data_second_disk/bundler/GEO/20120105_1402/";
      string packages_subdir=bundler_subdir+"packages/";
      vector<string> package_filenames=
         filefunc::files_in_subdir_matching_substring(packages_subdir,"photo");
      unsigned int n_cameras=package_filenames.size();
      cout << "n_cameras = " << n_cameras << endl;
      string URL_prefix="/data/ImageEngine/GEO/20120105_1402/";
      
      for (unsigned int c=0; c<n_cameras; c++)
      {
         vector<vector<string> > all_substrings=
            filefunc::ReadInSubstrings(package_filenames[c]);
         string curr_image_filename=all_substrings[0].back();
         URL.push_back(URL_prefix+filefunc::getbasename(curr_image_filename));
         image_ID.push_back(
            stringfunc::string_to_number(all_substrings[1].back()));
         double f=stringfunc::string_to_number(all_substrings[2].back());
         U0.push_back(stringfunc::string_to_number(all_substrings[4].back()));
         V0.push_back(stringfunc::string_to_number(all_substrings[5].back()));
         double aspect_ratio=U0.back()/V0.back();

         double curr_FOV_u,curr_FOV_v;
         camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
            f,aspect_ratio,curr_FOV_u,curr_FOV_v);
         FOV_u.push_back(curr_FOV_u*180/PI);
         FOV_v.push_back(curr_FOV_v*180/PI);
         az.push_back(stringfunc::string_to_number(all_substrings[6].back()));
         el.push_back(stringfunc::string_to_number(all_substrings[7].back()));
         roll.push_back(stringfunc::string_to_number(
            all_substrings[8].back()));
         camera_longitude.push_back(
            stringfunc::string_to_number(all_substrings[9].back()));
         camera_latitude.push_back(
            stringfunc::string_to_number(all_substrings[10].back()));
         camera_easting.push_back(
            stringfunc::string_to_number(all_substrings[11].back()));
         camera_northing.push_back(
            stringfunc::string_to_number(all_substrings[12].back()));

// FAKE FAKE: Hardwire camera altitude offset so that rays intersect
// at Z=0 within Brendan Edwards thin client WebGL viewer:

//         double Z_fudge=0;		// meters
         double Z_fudge=138.197;	// meters
         
         camera_altitude.push_back(stringfunc::string_to_number(
            all_substrings[13].back())-Z_fudge);

         frustum_sidelength.push_back(stringfunc::string_to_number(
            all_substrings[14].back()));
      } // loop over index c labeling cameras
   }
   
   return generate_JSON_response_to_requested_camera_metadata(
      hierarchy_ID,image_ID,URL,FOV_u,FOV_v,U0,V0,az,el,roll,
      camera_longitude,camera_latitude,camera_altitude,frustum_sidelength);
}

// ---------------------------------------------------------------------
QByteArray PhotoServer::generate_JSON_response_to_requested_camera_metadata(
   int hierarchy_ID,
   const vector<int>& image_ID,const vector<string>& URL,
   const vector<double>& FOV_u,const vector<double>& FOV_v,
   const vector<double>& U0,const vector<double>& V0,
   const vector<double>& az,const vector<double>& el,
   const vector<double>& roll,const vector<double>& camera_lon,
   const vector<double>& camera_lat,const vector<double>& camera_alt,
   const vector<double>& frustum_sidelength)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_requested_camera_metadata() #2"
//        << endl;

   string json_string = "{  \n";
   json_string += "   graphHierarchy: { \n";
   json_string += "      ID: "+stringfunc::number_to_string(
      hierarchy_ID)+", \n";
   json_string += "      cameras: [ \n";

   bool threeD_flag=false;
   if (FOV_u.size() > 0) threeD_flag=true;

   unsigned int n_cameras=image_ID.size();
//   cout << "n_cameras = " << n_cameras << endl;
   for (unsigned int c=0; c<n_cameras; c++)
   {
      json_string += "         { \n";
      json_string += "         ID: "+stringfunc::number_to_string(
         image_ID[c])+" , \n";
      json_string += "         URL: \""+URL[c]+"\" , \n";
      if (threeD_flag)
      {
         json_string += "         FOV_u: "+stringfunc::number_to_string(
            FOV_u[c])+" , \n";
         json_string += "         FOV_v: "+stringfunc::number_to_string(
            FOV_v[c])+" , \n";
      }
      json_string += "         U0: "+stringfunc::number_to_string(
         U0[c])+" , \n";
      json_string += "         V0: "+stringfunc::number_to_string(
         V0[c])+" , \n";
      if (threeD_flag)
      {
         json_string += "         az: "+stringfunc::number_to_string(
            az[c])+" , \n";
         json_string += "         el: "+stringfunc::number_to_string(
            el[c])+" , \n";
         json_string += "         roll: "+stringfunc::number_to_string(
            roll[c])+" , \n";
      }
      json_string += "         longitude: "+stringfunc::number_to_string(
         camera_lon[c],6)+" , \n";
      json_string += "         latitude: "+stringfunc::number_to_string(
         camera_lat[c],6);
      if (threeD_flag)
      {
         json_string += " , \n";
         json_string += "         altitude: "+stringfunc::number_to_string(
            camera_alt[c])+" ,  \n";
         json_string += "         frustum_sidelength: "+
            stringfunc::number_to_string(frustum_sidelength[c])+"  \n";
      }
      else
      {
         json_string += " \n";
      }
      
      json_string += "         } ";
      if (c < n_cameras-1) json_string += ", ";
      json_string += "\n";
   } // loop over index c labeling cameras
   
   json_string += "      ] \n";
   
   json_string += "   } \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Graph query handling member functions
// ==========================================================================

// Member function get_graph_hierarchy()

QByteArray PhotoServer::get_graph_hierarchy()
{
//   cout << "inside PhotoServer::get_graph_hierarchy()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="ID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;

   string description;
   unsigned int n_graphs,n_levels,n_connected_components;
   vector<int> graph_ID,graph_level,parent_graph_ID,n_nodes,n_links;

   if (hierarchy_ID < 0)  // return names of all graph hierarchies at startup
   {
      return generate_all_graph_hierarchy_IDs_JSON_response();
   }
   else if (hierarchy_ID >= 1000) // graph info does NOT come from database
   {
      curr_graph_hierarchy_ptr=
         graphdbfunc::reconstruct_graph_hierarchy_from_JSON_file(
            JSON_filename,hierarchy_ID,
            n_graphs,n_levels,n_connected_components,
            description,graph_ID,graph_level,parent_graph_ID,
            n_nodes,n_links);
   }
   else 
   {
      curr_graph_hierarchy_ptr=
         graphdbfunc::reconstruct_graph_hierarchy_from_database(
            postgis_database_ptr,hierarchy_ID);
      graphdbfunc::retrieve_graph_hierarchy_metadata_from_database(
         postgis_database_ptr,hierarchy_ID,description,n_graphs,n_levels,
         n_connected_components);
//      cout << "*graph_hierarchy_ptr = " << *graph_hierarchy_ptr << endl;
      graphdbfunc::retrieve_graphs_metadata_from_database(
         postgis_database_ptr,hierarchy_ID,n_graphs,
         graph_ID,graph_level,parent_graph_ID,n_nodes,n_links);

   }

   QByteArray JSON_response=
      generate_JSON_response_to_requested_graph_hierarchy(
         hierarchy_ID,description,n_graphs,n_levels,n_connected_components,
         graph_ID,graph_level,parent_graph_ID,n_nodes,n_links);
//   cout << "JSON_response = " << JSON_response.data() << endl;
   return JSON_response;
}

// ---------------------------------------------------------------------
// Member function generate_all_graph_hierarchy_IDs_JSON_response()
// returns a JSON string containing an array of integer graph
// hierarchy IDs and string hierarchy descriptions.

QByteArray PhotoServer::generate_all_graph_hierarchy_IDs_JSON_response()
{
//   cout << "Inside PhotoServer::generate_all_graph_hierarchy_IDs_JSON_response()"
//        << endl;

   vector<int> hierarchy_IDs;
   vector<string> hierarchy_descriptions;

   graphdbfunc::retrieve_hierarchy_IDs_from_database(
      postgis_database_ptr,hierarchy_IDs,hierarchy_descriptions);
   
   unsigned int n_hierarchies=hierarchy_IDs.size();

   string json_string = "{  \n";
   json_string += "   graphHierarchy: [ \n";
   for (unsigned int h=0; h<n_hierarchies; h++)
   {
      json_string += "   { \n";
      json_string += "      id: "+
         stringfunc::number_to_string(hierarchy_IDs[h])+", \n";
      json_string += "      description: '"+
         hierarchy_descriptions[h]+"' \n";
      json_string += "   }";
      if (h < n_hierarchies-1) json_string += ", ";
      json_string += "\n";
   }
   json_string += " ] \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_requested_graph_hierarchy()

QByteArray PhotoServer::generate_JSON_response_to_requested_graph_hierarchy(
   int hierarchy_ID,string description,int n_graphs,int n_levels,
   int n_connected_components,
   const vector<int>& graph_ID,const vector<int>& graph_level,
   const vector<int>& parent_graph_ID,const vector<int>& n_nodes,
   const vector<int>& n_links)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_requested_graph_hierarchy()"
//        << endl;

   string json_string = "{  \n";
   json_string += "   graphHierarchy: { \n";
   json_string += "      ID: "+stringfunc::number_to_string(
      hierarchy_ID)+", \n";
   json_string += "      description: "+description+" , \n";
   json_string += "      numGraphs: "+stringfunc::number_to_string(n_graphs)
      +" , \n";
   json_string += "      numLevels: "+stringfunc::number_to_string(n_levels)
      +",  \n";
   json_string += "      numConnectedComponents: "
      +stringfunc::number_to_string(n_connected_components)
      +",  \n";
   json_string += "      graphs: [ \n";
   for (unsigned int g=0; g<n_graphs; g++)
   {
      json_string += "         { \n";
      json_string += "         ID: "+stringfunc::number_to_string(
         graph_ID[g])+" , \n";
      json_string += "         level: "+stringfunc::number_to_string(
         graph_level[g])+" , \n";
      json_string += "         parentGraphID: "+stringfunc::number_to_string(
         parent_graph_ID[g])+" , \n";
      json_string += "         numNodes: "+stringfunc::number_to_string(
         n_nodes[g])+" , \n";
      json_string += "         numLinks: "+stringfunc::number_to_string(
         n_links[g])+" \n";
      json_string += "         } ";
      if (g < n_graphs-1) json_string += ", ";
      json_string += "\n";
   }
   
   json_string += "      ] \n";
   
   json_string += "   } \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function get_graph()

QByteArray PhotoServer::get_graph()
{
//   cout << "inside PhotoServer::get_graph()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int graph_ID=-1;
   bool get_nodes_flag=true;
   bool get_edges_flag=true;
   bool get_annotations_flag=true;
   vector<int> incident_node_IDs;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphID")
      {
         graph_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="nodes")
      {
         get_nodes_flag=stringfunc::string_to_boolean(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="edges")
      {
         get_edges_flag=stringfunc::string_to_boolean(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="annotations")
      {
         get_annotations_flag=stringfunc::string_to_boolean(
            KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="edgesAdjacentToNodes")
      {
         string nodeIDs_str=KeyValue[k].second; 
         vector<double> values=stringfunc::string_to_numbers(nodeIDs_str,",");
         for (unsigned int n=0; n<values.size(); n++)
         {
            incident_node_IDs.push_back(int(values[n]));
         }
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "graph_ID = " << graph_ID << endl;
//   cout << "get_nodes_flag = " << get_nodes_flag << endl;
//   cout << "get_edges_flag = " << get_edges_flag << endl;
//   cout << "incident_node_IDs.size() = " << incident_node_IDs.size() << endl;

   bool graph_retrieved_flag;
   int level,parent_graph_ID,n_nodes,n_links;
   if (hierarchy_ID >= 1000) // graph info does NOT come from database
   {
      string json_string=graphdbfunc::get_JSON_string_from_JSON_file(
         JSON_filename);
      return QByteArray(json_string.c_str());
   }
   else 
   {
      graph_retrieved_flag=
         graphdbfunc::retrieve_particular_graph_metadata_from_database(
            postgis_database_ptr,hierarchy_ID,graph_ID,
            level,parent_graph_ID,n_nodes,n_links);
   }
//   cout <<  "graph_retrieved_flag = " << graph_retrieved_flag << endl;

   if (!graph_retrieved_flag)
   {
      string error_message="No graph exists corresponding to input graph_ID = "
         +stringfunc::number_to_string(graph_ID);
      return generate_error_JSON_response(error_message);
   }
   else
   {
      reconstruct_graph_hierarchy_only_if_necessary(hierarchy_ID);


      return generate_JSON_response_to_requested_graph(
         hierarchy_ID,level,get_nodes_flag,get_edges_flag,
         get_annotations_flag,incident_node_IDs);
   }
}

// ---------------------------------------------------------------------
// Member function reconstruct_graph_hierarchy_only_if_necessary()

// We NEVER want to have to unnecessarily regenerate
// *curr_graph_hierarchy_ptr.  So check if the requested graph's
// hierarchy already matches *curr_graph_hierarchy_ptr.  If so, don't
// perform expensive graph hierarchy reconstruction from database...

void PhotoServer::reconstruct_graph_hierarchy_only_if_necessary(
   int graph_hierarchy_ID)
{
//   cout << "Inside PhotoServer::reconstruct_graph_hierarchy_only_if_necessary()" << endl;

   if (curr_graph_hierarchy_ptr==NULL)
   {
      curr_graph_hierarchy_ptr=
         graphdbfunc::reconstruct_graph_hierarchy_from_database(
            postgis_database_ptr,graph_hierarchy_ID);
   }
   else if (curr_graph_hierarchy_ptr->get_ID() != graph_hierarchy_ID)
   {
      curr_graph_hierarchy_ptr=
         graphdbfunc::reconstruct_graph_hierarchy_from_database(
            postgis_database_ptr,graph_hierarchy_ID);
   }
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_requested_graph()

QByteArray PhotoServer::generate_JSON_response_to_requested_graph(
   int hierarchy_ID,int graph_level,bool get_nodes_flag,bool get_edges_flag,
   bool get_annotations_flag,const vector<int>& incident_node_IDs)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_requested_graph()"
//        << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "n_levels = " << curr_graph_hierarchy_ptr->get_n_levels() << endl;
//   cout << "n_graphs = " << curr_graph_hierarchy_ptr->get_n_graphs() << endl;
//   cout << "graph_level = " << graph_level << endl;

   graph* graph_ptr=curr_graph_hierarchy_ptr->get_graph_ptr(graph_level);
   string json_string=imagesdatabasefunc::write_graph_json_string(
      postgis_database_ptr,hierarchy_ID,graph_ptr,
      get_nodes_flag,get_edges_flag,get_annotations_flag,incident_node_IDs);

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function update_hierarchy_level_dropdowns()

QByteArray PhotoServer::update_hierarchy_level_dropdowns()
{
//   cout << "inside PhotoServer::update_hierarchy_level_dropdowns()" << endl;

   vector<int> hierarchy_IDs,graph_IDs,graph_levels;
   vector<string> hierarchy_labels;

   graphdbfunc::retrieve_hierarchies_levels_from_database(
      postgis_database_ptr,hierarchy_IDs,hierarchy_labels,
      graph_IDs,graph_levels);

// Store correlated metadata retrieved from database within STL maps:

   typedef map<int,vector<int> > ONE_ID_TO_MANY_ID_MAP;
   typedef map<int,int> ONE_ID_TO_ONE_ID_MAP;
   typedef map<int,string> ID_TO_LABEL_MAP;

   ONE_ID_TO_MANY_ID_MAP hierarchy_graph_IDs_map,hierarchy_level_map;
   ONE_ID_TO_ONE_ID_MAP graph_ID_level_map;
   ID_TO_LABEL_MAP hierarchy_label_map;
   
// First count number of independent hierarchies.  Fill STL maps
// containing graph IDs and hierarchy labels as a function of
// hierarchy ID:

   int n_hierarchies=0;
   for (unsigned int i=0; i<hierarchy_IDs.size(); i++)
   {
      int curr_hierarchy_ID=hierarchy_IDs[i];

      vector<int> curr_graph_IDs;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=hierarchy_graph_IDs_map.find(
         curr_hierarchy_ID);
      if (iter != hierarchy_graph_IDs_map.end())
      {
         curr_graph_IDs=iter->second;
      }
      else
      {
         n_hierarchies++;
      }
      
      curr_graph_IDs.push_back(graph_IDs[i]);

      hierarchy_graph_IDs_map[curr_hierarchy_ID]=curr_graph_IDs;
      hierarchy_label_map[curr_hierarchy_ID]=hierarchy_labels[i];
   } // loop over index i labeling hierarchy IDs
//   cout << "n_hierarchies = " << n_hierarchies << endl;
   
// Next fill STL map containing graph levels as a function of
// hierarchy ID:

   for (unsigned int i=0; i<hierarchy_IDs.size(); i++)
   {
      int curr_hierarchy_ID=hierarchy_IDs[i];

      vector<int> curr_levels;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=hierarchy_level_map.find(
         curr_hierarchy_ID);
      if (iter != hierarchy_level_map.end())
      {
         curr_levels=iter->second;
      }
      curr_levels.push_back(graph_levels[i]);

      hierarchy_level_map[curr_hierarchy_ID]=curr_levels;
   } // loop over index i labeling hierarchy IDs

// Write hierarchy_ID, hierarchy_label and corresponding graph IDs and
// levels to output JSON string:

   string json_string = "{ ";
   json_string += " \"Hierarchy_ID_label_graphIDs\": [ ";   
   int prev_hierarchy_ID=-1;

   int hierarchy_counter=0;
   for (unsigned int i=0; i<hierarchy_IDs.size(); i++)
   {
      int curr_hierarchy_ID=hierarchy_IDs[i];
//      cout << "curr_hierarchy_ID = " << curr_hierarchy_ID << endl;

// Do not repeat hierarchy information within output JSON string more
// than once:

      if (curr_hierarchy_ID==prev_hierarchy_ID)
      {
         continue;
      }
      else
      {
         prev_hierarchy_ID=curr_hierarchy_ID;
      }

      ID_TO_LABEL_MAP::iterator label_iter=hierarchy_label_map.find(
         curr_hierarchy_ID);
      string curr_hierarchy_label=label_iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_hierarchy_ID)+","
         +"\""+curr_hierarchy_label+"\" ," ;

      json_string += " [ ";

      vector<int> curr_graph_IDs;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=hierarchy_graph_IDs_map.find(
         curr_hierarchy_ID);
      if (iter != hierarchy_graph_IDs_map.end())
      {
         curr_graph_IDs=iter->second;
      }
      int n_graphs=curr_graph_IDs.size();

//      cout << "Hierarchy ID = " << curr_hierarchy_ID << endl;
      for (unsigned int j=0; j<n_graphs; j++)
      {
         int curr_graph_ID=curr_graph_IDs[j];
         json_string += stringfunc::number_to_string(curr_graph_ID);
         if (j < n_graphs-1)
         {
            json_string += ",";
         }
      }
      json_string += " ], [ ";

      vector<int> curr_levels;
      ONE_ID_TO_MANY_ID_MAP::iterator level_iter=hierarchy_level_map.find(
         curr_hierarchy_ID);
      if (level_iter != hierarchy_level_map.end())
      {
         curr_levels=level_iter->second;
      }
      int n_levels=curr_levels.size();

//      cout << "Hierarchy ID = " << curr_hierarchy_ID << endl;
      for (unsigned int j=0; j<n_levels; j++)
      {
         int curr_level=curr_levels[j];
         json_string += stringfunc::number_to_string(curr_level);
         if (j < n_levels-1)
         {
            json_string += ",";
         }
      }

      json_string += " ] ] ";

      if (hierarchy_counter < n_hierarchies-1)
      {
         json_string += ",";
      }
      hierarchy_counter++;
   } // loop over index i labeling hierarchy ID in chronological order
   json_string += " ] \n";

   json_string += "} \n";

//   cout << "Final update_selector_metadata json_string = " 
//        << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function get_zeroth_node_given_level() 

QByteArray PhotoServer::get_zeroth_node_given_level()
{
//   cout << "inside PhotoServer::get_zeroth_node_given_level()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int graph_level=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "graph_level = " << graph_level << endl;
   int zeroth_node_ID=graphdbfunc::get_zeroth_node_given_level(
      postgis_database_ptr,hierarchy_ID,graph_level);
//   cout << "zeroth_node_ID = " << zeroth_node_ID << endl;

   if (zeroth_node_ID < 0)
   {
      string response_msg=
         "Zeroth node for input graph level not found";
      return generate_JSON_response_to_invalid_entry(response_msg);
   }

   return generate_JSON_response_to_zeroth_node_ID(zeroth_node_ID);
}

// ---------------------------------------------------------------------
// Member function display_graph_geometries() retrieves any graph
// polygons stored within the graph_geometries table of the IMAGERY
// database.  If any are found for the specified hierarchy_ID, this
// method displays them as backdrops for the current graph.

void PhotoServer::redraw_graph_geometries()
{
   cout << "inside PhotoServer::redraw_graph_geometries()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   string topic="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities

   cout << "hierarchy_ID = " << hierarchy_ID << endl;
   cout << "topic = " << topic << endl;

// First erase any background geometries previously drawn:

   issue_clear_polygons_message(topic);
 
   vector<string> polygon_labels;
   vector<fourvector> polygon_RGBA;
   vector<vector<twovector> > polygon_vertices;

   bool polygons_retrieved_flag=graphdbfunc::
      retrieve_graph_polygons_from_database(
         postgis_database_ptr,hierarchy_ID,
         polygon_labels,polygon_RGBA,polygon_vertices);
   
   if (polygons_retrieved_flag)
   {
      for (unsigned int p=0; p<polygon_RGBA.size(); p++)
      {
         issue_update_graph_polygons_message(
            polygon_vertices[p],polygon_RGBA[p],topic);
      } // loop over index p labeling retrieved polygons
   }
}

// ==========================================================================
// Connected graph components member functions
// ==========================================================================

// Member function get_connected_graph_components() takes in a graph
// hierarchy ID.  It generates and returns a JSON string containing
// graph level, connected graph component ID and n_nodes per component 
// metadata.

QByteArray PhotoServer::get_connected_graph_components()
{
//   cout << "inside PhotoServer::get_connected_graph_components()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;

   vector<int> graph_IDs,levels,connected_component_IDs,n_nodes;
   graphdbfunc::retrieve_connected_component_info_from_database(
      postgis_database_ptr,hierarchy_ID,graph_IDs,levels,
      connected_component_IDs,n_nodes);

   return generate_JSON_response_to_connected_components(
      hierarchy_ID,graph_IDs,levels,connected_component_IDs,n_nodes);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_connected_components()

QByteArray PhotoServer::generate_JSON_response_to_connected_components(
   int hierarchy_ID,
   const vector<int>& graph_IDs,const vector<int>& levels,
   const vector<int>& connected_component_IDs,const vector<int>& n_nodes)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_connected_components()"
//        << endl;

   unsigned int n_graphs=graph_IDs.size();
//   for (unsigned int i=0; i<n_graphs; i++)
//   {
//      cout << "i = " << i
//           << " graphID = " << graph_IDs[i]
//           << " connected_component = " << connected_component_IDs[i]
//           << " n_nodes = " << n_nodes[i] << endl;
//   }

   string json_string = "{  \n";
   json_string += "  hierarchyID: "+stringfunc::number_to_string(
      hierarchy_ID)+", \n";

   json_string += "  connectedGraphComponents: [ \n";
   for (unsigned int g=0; g<graph_IDs.size(); g++)
   {
      json_string += "      { \n";
      json_string += "         graphID: "+stringfunc::number_to_string(
         graph_IDs[g])+" , \n";
      json_string += "         level: "+stringfunc::number_to_string(
         levels[g])+" , \n";
      json_string += "         componentID: "+stringfunc::number_to_string(
         connected_component_IDs[g])+" , \n";
      json_string += "         numNodes: "+stringfunc::number_to_string(
         n_nodes[g])+"  \n";
      json_string += "      }";
      if (g < n_graphs-1) json_string += ", ";
      json_string += "\n";
   }
   json_string += "      ] \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Best path query handling member functions
// ==========================================================================

// Member function find_best_path() takes in the IDs for two images
// within the photos table of the TOC database.  It returns a JSON
// string listing SIFT tiepoint pairs for the pair of input images.

QByteArray PhotoServer::find_best_path()
{
//   cout << "inside PhotoServer::find_best_path()" << endl;

   int StartNodeID=-1;
   int StopNodeID=-1;
   int HierarchyID=-1;
   string topic;
   unsigned int n_args=KeyValue.size();
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="StartNodeID")
      {
         StartNodeID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="StopNodeID")
      {
         StopNodeID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="HierarchyID")
      {
         HierarchyID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "StartNodeID = " << StartNodeID 
//        << " StopNodeID = " << StopNodeID << endl;
//   cout << "HierarchyID = " << HierarchyID << endl;

   if (StartNodeID < 0)
   {
      string error_msg="Start node ID doesn't belong to graph!";
      return generate_error_JSON_response(error_msg);
   }
   if (StopNodeID < 0)
   {
      string error_msg="Stop node ID doesn't belong to graph!";
      return generate_error_JSON_response(error_msg);
   }
   if (HierarchyID < 0)
   {
      string error_msg="Hierarchy ID doesn't belong to graph pyramid!";
      return generate_error_JSON_response(error_msg);
   }

   int graph_ID=graphdbfunc::get_graph_ID_given_node_and_hierarchy_IDs(
      postgis_database_ptr,StartNodeID,HierarchyID);
//   cout << "Graph_ID = " << graph_ID << endl;
   if (graph_ID < 0)
   {
      string error_msg="Graph ID doesn't belong to graph pyramid!";
      return generate_error_JSON_response(error_msg);
   }

   int graph_level=graphdbfunc::get_graph_level_given_ID(
      postgis_database_ptr,HierarchyID,graph_ID);
//   cout << "graph_level = " << graph_level << endl;

   reconstruct_graph_hierarchy_only_if_necessary(HierarchyID);
   
   graph* graph_ptr=curr_graph_hierarchy_ptr->get_graph_ptr(graph_level);

   bool startnode_flag=graph_ptr->node_in_graph(StartNodeID);
   bool stopnode_flag=graph_ptr->node_in_graph(StopNodeID);
   if (!startnode_flag)
   {
      string error_msg="Start node ID doesn't belong to graph!";
      return generate_error_JSON_response(error_msg);
   }
   if (!stopnode_flag)
   {
      string error_msg="Stop node ID doesn't belong to graph!";
      return generate_error_JSON_response(error_msg);
   }
   
   vector<int> path_node_IDs=
      graph_ptr->compute_Astar_path(StartNodeID,StopNodeID);

   vector<int> image_IDs;
   vector<string> image_URLs,thumbnail_URLs;
   for (unsigned int i=0; i<path_node_IDs.size(); i++)
   {
      int curr_node_ID=path_node_IDs[i];
      int image_ID;
      string image_URL,thumbnail_URL;
      imagesdatabasefunc::retrieve_image_ID_URL_given_node_ID(
         postgis_database_ptr,HierarchyID,curr_node_ID,
         image_ID,image_URL,thumbnail_URL);
      image_IDs.push_back(image_ID);
      image_URLs.push_back(image_URL);
      thumbnail_URLs.push_back(thumbnail_URL);
   }

// Broadcast ActiveMQ message containing best path node IDs so that
// Michael Yee's graph tool can display the results:

   issue_best_path_message(HierarchyID,graph_level,path_node_IDs,topic);

   return generate_JSON_response_to_requested_thumbnails(
      image_IDs,image_URLs,thumbnail_URLs);
}

/*
// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_best_path()

QByteArray PhotoServer::generate_JSON_response_to_best_path(
   const vector<int>& path_node_IDs)
{
   cout << "Inside PhotoServer::generate_JSON_response_to_best_path()"
        << endl;
   
   string json_string = "{  \n";
   json_string += " path_node_IDs: [ \n";
   for (unsigned int n=0; n<path_node_IDs.size()-1; n++)
   {
      json_string += stringfunc::number_to_string(path_node_IDs[n])+", ";
   }
   json_string += stringfunc::number_to_string(
      path_node_IDs[path_node_IDs.size()-1]);
   json_string += " ] \n ";   
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}
*/

// ==========================================================================
// SIFT matches query handling member functions
// ==========================================================================

// Member function get_SIFT_matches() takes in the IDs for two images
// within the photos table of the TOC database.  It returns a JSON
// string listing SIFT tiepoint pairs for the pair of input images.

QByteArray PhotoServer::get_SIFT_matches()
{
//   cout << "inside PhotoServer::get_SIFT_matches()" << endl;

   int campaign_ID1,mission_ID1,image_ID1;
   int campaign_ID2,mission_ID2,image_ID2;
   campaign_ID1=mission_ID1=image_ID1=
      campaign_ID2=mission_ID2=image_ID2=-1;
   
   unsigned int n_args=KeyValue.size();
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="campaign_id1")
      {
         campaign_ID1=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="mission_id1")
      {
         mission_ID1=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="image_id1")
      {
         image_ID1=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="campaign_id2")
      {
         campaign_ID2=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="mission_id2")
      {
         mission_ID2=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="image_id2")
      {
         image_ID2=stringfunc::string_to_number(KeyValue[k].second); 
      }

   } // loop over index k labeling KeyValue key possibilities

   vector<twovector> feature_matches;
   if (image_ID1 != -1 && image_ID2 != -1)
   {
//      imagesdatabasefunc::retrieve_SIFT_matches_from_database(
//        postgis_database_ptr,campaign_ID1,mission_ID1,image_ID1,
//         campaign_ID2,mission_ID2,image_ID2,feature_matches);
   }
   else
   {
      string error_message=
         "image_ID1="+stringfunc::number_to_string(image_ID1)+
         " image_ID2="+stringfunc::number_to_string(image_ID2)+
         " are not both valid IDs!";
      return generate_error_JSON_response(error_message);
   }
   
   return generate_JSON_response_to_SIFT_matches(feature_matches);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_requested_thumbnails()

QByteArray PhotoServer::generate_JSON_response_to_SIFT_matches(
   const vector<twovector>& feature_matches)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_SIFT_matches()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " sift_matches: [ \n";
   for (unsigned int n=0; n<feature_matches.size(); n++)
   {
      twovector curr_tiepoint_pair=feature_matches[n];
      json_string += "{ \n";
      json_string += "  sift_feature_ID1: "+
         stringfunc::number_to_string(curr_tiepoint_pair.get(0))+", \n";
      json_string += "  sift_feature_ID2: "+
         stringfunc::number_to_string(curr_tiepoint_pair.get(1))+" \n";
      json_string += "}";
      if (n < feature_matches.size()-1) json_string += ",";
      json_string += "\n";
   }
   json_string += " ] \n ";   
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// ActiveMQ Message handling member functions
// ==========================================================================

// Member function issue_update_curr_node_message()

void PhotoServer::issue_update_curr_node_message(
   int hierarchy_ID,int graph_level,int node_ID,int datum_ID,string topic,
   string client_name)
{
//   cout << "inside PhotoServer::issue_update_curr_node_message()" << endl;
//   cout << "topic = " << topic << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " graph_level = " << graph_level
//        << " node_ID = " << node_ID 
//        << " datum_ID " << datum_ID << endl;
//   cout << "client_name = " << client_name << endl;

   if (hierarchy_ID < 0 || graph_level < 0 || node_ID < 0) return;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
   
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   int image_ID=imagesdatabasefunc::get_image_ID(
      postgis_database_ptr,datum_ID);
//   cout << "image_ID = " << image_ID << endl;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="UPDATE_CURRENT_NODE";
 
   key="hierarchy_ID";
   value=stringfunc::number_to_string(hierarchy_ID);
   properties.push_back(property(key,value));

   key="graph_level";
   value=stringfunc::number_to_string(graph_level);
   properties.push_back(property(key,value));

   key="node_ID";
   value=stringfunc::number_to_string(node_ID);
   properties.push_back(property(key,value));

   key="datum_ID";
   value=stringfunc::number_to_string(datum_ID);
   properties.push_back(property(key,value));

   key="image_ID";
   value=stringfunc::number_to_string(image_ID);
   properties.push_back(property(key,value));

// If client_name is non_empty, temporarily reset message sender ID to
// client_name.  After sending the message, reset message sender ID to
// its original value.  We implemented this swapping in Feb 2012 for
// Timeline client messaging purposes...

   string orig_msg_sender_ID=Messenger_ptr->get_msg_sender_ID();
   if (client_name.size() > 0)
   {
      Messenger_ptr->set_msg_sender_ID(client_name);
   }
   Messenger_ptr->sendTextMessage(command,properties);

   Messenger_ptr->set_msg_sender_ID(orig_msg_sender_ID);
}

// ---------------------------------------------------------------------
// Member function issue_update_curr_caption_message()

void PhotoServer::issue_update_curr_caption_message(
   int hierarchy_ID,const vector<int>& node_IDs,string caption,string topic)
{
//   cout << "inside PhotoServer::issue_update_curr_caption_message()" << endl;

// On 4/27/11, we empirically found that ActiveMQ dies if it tries to
// transmit an empty caption string.  So if the caption is empty,
// broadcast out 'NULL' as the caption:

   if (caption.size()==0) caption="NULL";

   if (hierarchy_ID < 0 || node_IDs.size()==0) return;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="UPDATE_CURRENT_IMAGE_CAPTION";
 
   key="hierarchy_ID";
   value=stringfunc::number_to_string(hierarchy_ID);
   properties.push_back(property(key,value));

   key="caption";
   value=caption;
   properties.push_back(property(key,value));

   key="node_IDs";
   value="";
   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
      value += stringfunc::number_to_string(node_IDs[n]);
      if (n < node_IDs.size()-1) value += ", ";
   }
   properties.push_back(property(key,value));
   
   key="graph_levels";
   value="";
   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
      int graph_level=graphdbfunc::get_graph_level_given_node_ID(
         postgis_database_ptr,hierarchy_ID,node_IDs[n]);
      value += stringfunc::number_to_string(graph_level);
      if (n < node_IDs.size()-1) value += ", ";
   }
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_update_annotations_message() broadcasts the
// current number of annotations associated with the current image
// (which is associated with one or more nodes).

void PhotoServer::issue_update_annotations_message(
   int n_annotations,int hierarchy_ID,const vector<int>& node_IDs,string topic)
{
//   cout << "inside PhotoServer::issue_update_annotations_message()" << endl;

   if (hierarchy_ID < 0 || node_IDs.size()==0) return;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

//   command="UPDATE_CURRENT_IMAGE_ANNOTATIONS";
   command="UPDATE_CURRENT_IMAGE_PROPERTY";
 
   key="hierarchy_ID";
   value=stringfunc::number_to_string(hierarchy_ID);
   properties.push_back(property(key,value));

/*
   key="n_annotations";
   value=stringfunc::number_to_string(n_annotations);
   properties.push_back(property(key,value));
*/

// As of 4/19/13, Michael Yee has generalized his graph viewer so that
// it can dynamically update arbitrary node properties.  In
// particular, it modifies the number of annotations for the current
// image via the following two key-value pairs:

   key="propertyKey";
   value="annotations";
   properties.push_back(property(key,value));

   key="propertyValue";
   value=stringfunc::number_to_string(n_annotations);
   properties.push_back(property(key,value));

   key="node_IDs";
   value="";
   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
      value += stringfunc::number_to_string(node_IDs[n]);
      if (n < node_IDs.size()-1) value += ", ";
   }
   properties.push_back(property(key,value));
   
   key="graph_levels";
   value="";
   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
      int graph_level=graphdbfunc::get_graph_level_given_node_ID(
         postgis_database_ptr,hierarchy_ID,node_IDs[n]);
      value += stringfunc::number_to_string(graph_level);
      if (n < node_IDs.size()-1) value += ", ";
   }
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_curr_backprojection_message()

void PhotoServer::issue_curr_backprojection_message(
   int hierarchy_ID,int node_ID,string topic,string client_name,
   const threevector& camera_posn,const threevector& r_hat,
   std::string label)
{
//   cout << "inside PhotoServer::issue_curr_backprojection_message()" << endl;
//   cout << "topic = " << topic << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " graph_level = " << graph_level
//        << " node_ID = " << node_ID 
//        << " datum_ID " << datum_ID << endl;
//   cout << "client_name = " << client_name << endl;

   if (hierarchy_ID < 0 || node_ID < 0) return;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
   
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="UPDATE_CURRENT_BACKPROJECTION";
 
   key="hierarchy_ID";
   value=stringfunc::number_to_string(hierarchy_ID);
   properties.push_back(property(key,value));

   key="node_ID";
   value=stringfunc::number_to_string(node_ID);
   properties.push_back(property(key,value));

   key="camera_posn_x";
   value=stringfunc::number_to_string(camera_posn.get(0));
   properties.push_back(property(key,value));

   key="camera_posn_y";
   value=stringfunc::number_to_string(camera_posn.get(1));
   properties.push_back(property(key,value));

   key="camera_posn_z";
   value=stringfunc::number_to_string(camera_posn.get(2));
   properties.push_back(property(key,value));

   key="rhat_x";
   value=stringfunc::number_to_string(r_hat.get(0));
   properties.push_back(property(key,value));

   key="rhat_y";
   value=stringfunc::number_to_string(r_hat.get(1));
   properties.push_back(property(key,value));

   key="rhat_z";
   value=stringfunc::number_to_string(r_hat.get(2));
   properties.push_back(property(key,value));

   key="label";
   value=label;
   properties.push_back(property(key,value));

// If client_name is non_empty, temporarily reset message sender ID to
// client_name.  After sending the message, reset message sender ID to
// its original value.  We implemented this swapping in Feb 2012 for
// Timeline client messaging purposes...

   string orig_msg_sender_ID=Messenger_ptr->get_msg_sender_ID();
   if (client_name.size() > 0)
   {
      Messenger_ptr->set_msg_sender_ID(client_name);
   }
   Messenger_ptr->sendTextMessage(command,properties);

   Messenger_ptr->set_msg_sender_ID(orig_msg_sender_ID);
}

// ---------------------------------------------------------------------
// Member function issue_best_path_message(

void PhotoServer::issue_best_path_message(
   int hierarchy_ID,int graph_level,vector<int>& node_IDs,string topic)
{
//   cout << "inside PhotoServer::issue_best_path_message()" << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " graph_level = " << graph_level << endl;

   if (hierarchy_ID < 0 || graph_level < 0) return;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="DISPLAY_BEST_PATH";
 
   key="hierarchy_ID";
   value=stringfunc::number_to_string(hierarchy_ID);
   properties.push_back(property(key,value));

   key="graph_level";
   value=stringfunc::number_to_string(graph_level);
   properties.push_back(property(key,value));

   key="node_IDs";
   value="";
   for (unsigned int i=0; i<node_IDs.size(); i++)
   {
      value += stringfunc::number_to_string(node_IDs[i]);
      if (i < node_IDs.size()-1) value += ", ";
   }
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_clear_polygons_message()

void PhotoServer::issue_clear_polygons_message(string topic)
{
//   cout << "inside PhotoServer::issue_clear_polygons_message()" << endl;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command="CLEAR_POLYGONS";
   typedef pair<string,string> property;
   vector<property> properties;

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_update_graph_polygons_message()

void PhotoServer::issue_update_graph_polygons_message(
   const vector<twovector>& polygon_vertices,
   const fourvector& RGBA,string topic)
{
//   cout << "inside PhotoServer::issue_update_graph_polygons_message()" << endl;

   Messenger* Messenger_ptr=get_Messenger_ptr(topic);
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="DRAW_POLYGON";

   string json_string = "{  \n";
   json_string += " 'points': [ ";
   for (unsigned int n=0; n<polygon_vertices.size(); n++)
   {
      json_string += 
         " ["+stringfunc::number_to_string(polygon_vertices[n].get(0))
         +","+stringfunc::number_to_string(polygon_vertices[n].get(1))
         +" ]";
      if (n < polygon_vertices.size()-1) json_string += ",";
   } // loop over index n labeling polygon vertices
   json_string += " ], \n";
   json_string += " 'color': [ ";
   json_string += stringfunc::number_to_string(RGBA.get(0))+",";
   json_string += stringfunc::number_to_string(RGBA.get(1))+",";
   json_string += stringfunc::number_to_string(RGBA.get(2))+",";
   json_string += stringfunc::number_to_string(RGBA.get(3));
   json_string += " ] \n }";
   json_string += "\n";

   key="polygon_JSON";
   value=json_string;
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}


//   (text) CLEAR_POLYGONS

// ==========================================================================
// Image caption member functions
// ==========================================================================

// Member function edit_image_caption()

QByteArray PhotoServer::edit_image_caption()
{
   cout << "inside PhotoServer::edit_image_caption()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID,graph_level,node_ID;
   string image_caption="";
   string topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         graph_level=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="ImageCaption")
      {
         image_caption=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "image_caption = " << image_caption << endl;

//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " graph_level = " << graph_level
//        << " node_ID = " << node_ID << endl;
//   cout << "topicname = " << topic << endl;

   int datum_ID=
      graphdbfunc::retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
         postgis_database_ptr,hierarchy_ID,node_ID);
//   cout << "datum_ID = " << datum_ID << endl;

   vector<int> node_IDs=
      graphdbfunc::retrieve_node_IDs_for_particular_graph_hierarchy_and_datum(
         postgis_database_ptr,hierarchy_ID,datum_ID);

   for (unsigned int n=0; n<node_IDs.size(); n++)
   {
//      cout << "n = " << n << " node_IDs[n] = " << node_IDs[n] << endl;
      graphdbfunc::update_node_label_in_database(
         postgis_database_ptr,hierarchy_ID,node_IDs[n],image_caption);
   }

   issue_update_curr_caption_message(
      hierarchy_ID,node_IDs,image_caption,topic);

   return generate_JSON_response_to_image_caption(image_caption);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_image_caption()

QByteArray PhotoServer::generate_JSON_response_to_image_caption(
   string image_caption)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_image_caption()"
//        << endl;

   string json_string = "{  \n";
   json_string += " 'image_caption': '"+image_caption+"' \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Image time member functions
// ==========================================================================

// Member function median_image_time() retrieves all images
// corresponding to the specified Hierarchy and Graph IDs.  It returns
// their median epoch time in seconds.

QByteArray PhotoServer::median_image_time()
{
//   cout << "inside PhotoServer::median_image_time()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchyID,graphID;
   
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchyID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphID")
      {
         graphID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   int campaignID,missionID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,hierarchyID,campaignID,missionID);
//   cout << "campaignID = " << campaignID << " missionID = " << missionID
//        << endl;

   double median_epoch=imagesdatabasefunc::retrieve_median_image_time(
      postgis_database_ptr,campaignID,missionID);
   return generate_JSON_response_to_requested_image_time(median_epoch);
}

// ---------------------------------------------------------------------
// Member function selected_image_time() returns the epoch time
// corresponding to the specified Hierarchy, Graph and Image IDs.

QByteArray PhotoServer::selected_image_time()
{
//   cout << "inside PhotoServer::selected_image_time()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchyID,graphID,imageID;
   
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchyID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphID")
      {
         graphID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="ImageID")
      {
         imageID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   int campaignID,missionID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,hierarchyID,campaignID,missionID);
//   cout << "campaignID = " << campaignID << " missionID = " << missionID
//        << endl;

   double image_epoch=imagesdatabasefunc::retrieve_image_time(
      postgis_database_ptr,campaignID,missionID,imageID);
   return generate_JSON_response_to_requested_image_time(image_epoch);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_requested_image_time()

QByteArray PhotoServer::generate_JSON_response_to_requested_image_time(
   double image_epoch)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_requested_image_time()"
//        << endl;

//   cout.precision(12);
//   cout << "image_epoch = " << image_epoch << endl;

   Clock clock;
   clock.convert_elapsed_secs_to_date(image_epoch);
//   cout << "Clock time = " << clock.YYYY_MM_DD_H_M_S() << endl;

   string json_string = "{  \n";
   json_string += " 'center_time': '"+stringfunc::number_to_string(
      image_epoch)+"' \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function extremal_image_times()

QByteArray PhotoServer::extremal_image_times()
{
//   cout << "inside PhotoServer::extremal_image_times()" << endl;

   unsigned int n_args=KeyValue.size();
   int campaignID,missionID;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="CampaignID")
      {
         campaignID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="MissionID")
      {
         missionID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "campaignID = " << campaignID
//        << " missionID = " << missionID << endl;

   double starting_epoch,stopping_epoch;
   imagesdatabasefunc::retrieve_extremal_image_times(
      postgis_database_ptr,campaignID,missionID,starting_epoch,stopping_epoch);

   return generate_JSON_response_to_extremal_image_times(
      starting_epoch,stopping_epoch);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_extremal_image_times()

QByteArray PhotoServer::generate_JSON_response_to_extremal_image_times(
   double starting_epoch,double stopping_epoch)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_extremal_image_times()"
//        << endl;

   string json_string = "{  \n";
   json_string += " 'starting_epoch': '"+stringfunc::number_to_string(
      starting_epoch)+"', \n";
   json_string += " 'stopping_epoch': '"+stringfunc::number_to_string(
      stopping_epoch)+"' \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function get_image_times()

QByteArray PhotoServer::get_image_times()
{
//   cout << "inside PhotoServer::get_image_times()" << endl;

   unsigned int n_args=KeyValue.size();
   int HierarchyID,GraphID;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         HierarchyID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         GraphID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "HierarchyID = " << HierarchyID
//        << " GraphID = " << GraphID
//        << endl;

   vector<int> datum_IDs,image_IDs;
   vector<double> epoch_times;
   vector<string> thumbnail_URLs;
   imagesdatabasefunc::retrieve_image_metadata_from_database(
      postgis_database_ptr,HierarchyID,GraphID,
      datum_IDs,image_IDs,epoch_times,thumbnail_URLs);

   return generate_JSON_response_to_image_times(
      HierarchyID,GraphID,datum_IDs,image_IDs,epoch_times,thumbnail_URLs);
}

// ---------------------------------------------------------------------
// Member function get_nearby_image_times()

QByteArray PhotoServer::get_nearby_image_times()
{
   cout << "inside PhotoServer::get_nearby_image_times()" << endl;

   unsigned int n_args=KeyValue.size();
   int Hierarchy_ID,Graph_ID;
   int Year,Month,Day,Hour,Minute,Second;
   double Millisecond;
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k << " KeyValue[k].first = "
//           << KeyValue[k].first
//           << " KeyValue[k].second = "
//           << KeyValue[k].second << endl;
      
      if (KeyValue[k].first=="HierarchyID")
      {
         Hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphLevel")
      {
         Graph_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Year")
      {
         Year=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Month")
      {
         Month=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Day")
      {
         Day=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Hour")
      {
         Hour=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Minute")
      {
         Minute=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Second")
      {
         Second=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="Millisecond")
      {
         Millisecond=stringfunc::string_to_number(KeyValue[k].second); 
      }

   } // loop over index k labeling KeyValue key possibilities

   cout << "Hierarchy_ID = " << Hierarchy_ID
        << " Graph_ID = " << Graph_ID
        << " Year = " << Year
        << " Month = " << Month
        << " Day = " << Day
        << " Hour = " << Hour
        << " Minute = " << Minute
        << " Second = " << Second
        << " Millisecond = " << Millisecond
        << endl;
   double secs=Second+0.001*Millisecond;

   Clock clock;
   clock.set_UTC_time(Year,Month,Day,Hour,Minute,secs);
   double center_epoch_time=clock.secs_elapsed_since_reference_date();
   cout.precision(12);
//   cout << "center epoch time = " << center_epoch_time << endl;

   double nearby_minutes=0.5;
   double start_epoch_time=center_epoch_time-nearby_minutes*60;
   double stop_epoch_time=center_epoch_time+nearby_minutes*60;
//   cout << "start_epoch_time = " << start_epoch_time 
//        << " stop_epoch_time = " << stop_epoch_time << endl;
   
   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,Hierarchy_ID,campaign_ID,mission_ID);

   vector<int> datum_IDs,image_IDs;
   vector<double> epoch_times;
   vector<string> thumbnail_URLs;

   imagesdatabasefunc::retrieve_images_within_time_interval(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_epoch_time,stop_epoch_time,
      datum_IDs,image_IDs,epoch_times,thumbnail_URLs);
//   cout << "datum_IDs.size() = " << datum_IDs.size() << endl;
//   cout << "image_IDs.size() = " << image_IDs.size() << endl;

//   cout << "data_ids_map_ptr = " << data_ids_map_ptr << endl;
   data_ids_map_ptr->clear();
   for (unsigned int d=0; d<datum_IDs.size(); d++)
   {
      DATA_IDS_MAP::iterator iter=data_ids_map_ptr->find(datum_IDs[d]);
      if (iter==data_ids_map_ptr->end())
      {
         (*data_ids_map_ptr)[datum_IDs[d]]=true;
      }
   } // loop over index d labeling datum_IDs
   
   nearby_minutes=5;
   start_epoch_time=center_epoch_time-nearby_minutes*60;
   stop_epoch_time=center_epoch_time+nearby_minutes*60;

   vector<double> distinct_epoch_minutes;
   imagesdatabasefunc::retrieve_image_minutes_within_time_interval(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_epoch_time,stop_epoch_time,distinct_epoch_minutes);

   const double nearby_hours=5;
   start_epoch_time=center_epoch_time-nearby_hours*3600;
   stop_epoch_time=center_epoch_time+nearby_hours*3600;

   vector<double> distinct_epoch_hours;
   imagesdatabasefunc::retrieve_image_hours_within_time_interval(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_epoch_time,stop_epoch_time,distinct_epoch_hours);

   const double nearby_days=5;
   start_epoch_time=center_epoch_time-nearby_days*24*3600;
   stop_epoch_time=center_epoch_time+nearby_days*24*3600;

   vector<double> distinct_epoch_days;
   imagesdatabasefunc::retrieve_image_hours_within_time_interval(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_epoch_time,stop_epoch_time,distinct_epoch_days);

   const double nearby_months=5;
   start_epoch_time=center_epoch_time-nearby_months*30*24*3600;
   stop_epoch_time=center_epoch_time+nearby_months*30*24*3600;

   vector<double> distinct_epoch_months;
   imagesdatabasefunc::retrieve_image_hours_within_time_interval(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_epoch_time,stop_epoch_time,distinct_epoch_months);

   const double nearby_years=10;
   start_epoch_time=center_epoch_time-nearby_years*365*24*3600;
   stop_epoch_time=center_epoch_time+nearby_years*365*24*3600;

   vector<double> distinct_epoch_years;
   imagesdatabasefunc::retrieve_image_hours_within_time_interval(
      postgis_database_ptr,campaign_ID,mission_ID,
      start_epoch_time,stop_epoch_time,distinct_epoch_years);

   int curr_datum_ID,curr_image_ID;
   double curr_epoch;
   string curr_thumbnail_URL;

   for (unsigned int h=0; h<distinct_epoch_minutes.size(); h++)
   {
      if (!imagesdatabasefunc::retrieve_closest_time_image(
         postgis_database_ptr,campaign_ID,mission_ID,
         distinct_epoch_minutes[h],curr_datum_ID,curr_image_ID,
         curr_epoch,curr_thumbnail_URL)) continue;

      if (data_ids_map_ptr->find(curr_datum_ID) !=
      data_ids_map_ptr->end()) continue;
      (*data_ids_map_ptr)[curr_datum_ID]=true;

      datum_IDs.push_back(curr_datum_ID);
      image_IDs.push_back(curr_image_ID);
      epoch_times.push_back(curr_epoch);
      thumbnail_URLs.push_back(curr_thumbnail_URL);
   }

   for (unsigned int h=0; h<distinct_epoch_hours.size(); h++)
   {
      if (!imagesdatabasefunc::retrieve_closest_time_image(
         postgis_database_ptr,campaign_ID,mission_ID,
         distinct_epoch_hours[h],curr_datum_ID,curr_image_ID,
         curr_epoch,curr_thumbnail_URL)) continue;

      if (data_ids_map_ptr->find(curr_datum_ID) !=
      data_ids_map_ptr->end()) continue;
      (*data_ids_map_ptr)[curr_datum_ID]=true;

      datum_IDs.push_back(curr_datum_ID);
      image_IDs.push_back(curr_image_ID);
      epoch_times.push_back(curr_epoch);
      thumbnail_URLs.push_back(curr_thumbnail_URL);
   }

   for (unsigned int h=0; h<distinct_epoch_days.size(); h++)
   {
      if (!imagesdatabasefunc::retrieve_closest_time_image(
         postgis_database_ptr,campaign_ID,mission_ID,
         distinct_epoch_days[h],curr_datum_ID,curr_image_ID,
         curr_epoch,curr_thumbnail_URL)) continue;

      if (data_ids_map_ptr->find(curr_datum_ID) !=
      data_ids_map_ptr->end()) continue;
      (*data_ids_map_ptr)[curr_datum_ID]=true;

      datum_IDs.push_back(curr_datum_ID);
      image_IDs.push_back(curr_image_ID);
      epoch_times.push_back(curr_epoch);
      thumbnail_URLs.push_back(curr_thumbnail_URL);
   }

   for (unsigned int h=0; h<distinct_epoch_months.size(); h++)
   {
      if (!imagesdatabasefunc::retrieve_closest_time_image(
         postgis_database_ptr,campaign_ID,mission_ID,
         distinct_epoch_months[h],curr_datum_ID,curr_image_ID,
         curr_epoch,curr_thumbnail_URL)) continue;

      if (data_ids_map_ptr->find(curr_datum_ID) !=
      data_ids_map_ptr->end()) continue;
      (*data_ids_map_ptr)[curr_datum_ID]=true;

      datum_IDs.push_back(curr_datum_ID);
      image_IDs.push_back(curr_image_ID);
      epoch_times.push_back(curr_epoch);
      thumbnail_URLs.push_back(curr_thumbnail_URL);
   }

   for (unsigned int h=0; h<distinct_epoch_years.size(); h++)
   {
      if (!imagesdatabasefunc::retrieve_closest_time_image(
         postgis_database_ptr,campaign_ID,mission_ID,
         distinct_epoch_years[h],curr_datum_ID,curr_image_ID,
         curr_epoch,curr_thumbnail_URL)) continue;

      if (data_ids_map_ptr->find(curr_datum_ID) !=
      data_ids_map_ptr->end()) continue;
      (*data_ids_map_ptr)[curr_datum_ID]=true;

      datum_IDs.push_back(curr_datum_ID);
      image_IDs.push_back(curr_image_ID);
      epoch_times.push_back(curr_epoch);
      thumbnail_URLs.push_back(curr_thumbnail_URL);
   }

   return generate_JSON_response_to_image_times(
      Hierarchy_ID,Graph_ID,datum_IDs,image_IDs,epoch_times,thumbnail_URLs);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_image_times()

QByteArray PhotoServer::generate_JSON_response_to_image_times(
   int HierarchyID,int GraphID,const vector<int>& datum_IDs,
   const vector<int>& image_IDs,const vector<double>& epoch_times,
   const vector<string>& thumbnail_URLs)
{
   cout << "Inside PhotoServer::generate_JSON_response_to_image_times()"
        << endl;

   Clock clock;
   double median_epoch_time=0;
   if (epoch_times.size() > 0) median_epoch_time=
	mathfunc::median_value(epoch_times);
   clock.convert_elapsed_secs_to_date(median_epoch_time);   

   string json_string = "{  \n";
   json_string +="'dateTimeFormat': 'iso8601',\n";
   json_string +="'medianEpochTime': '"+clock.ISO_format()+"',\n";

   json_string += "'events': [ \n";

   for (unsigned int i=0; i<image_IDs.size(); i++)
   {
      double curr_epoch=epoch_times[i];
      clock.convert_elapsed_secs_to_date(curr_epoch);
      json_string += "{'start': '"+clock.ISO_format()+"',\n";
      json_string += "'title': 'Img "+stringfunc::number_to_string(
         image_IDs[i])+"', \n";
      json_string += "'HierarchyID': '"+stringfunc::number_to_string(
         HierarchyID)+"', \n";
      json_string += "'GraphID': '"+stringfunc::number_to_string(
         GraphID)+"', \n";
      json_string += "'DatumID': '"+stringfunc::number_to_string(
         datum_IDs[i])+"', \n";
      json_string += "'ImageID': '"+stringfunc::number_to_string(
         image_IDs[i])+"' \n";
//         image_IDs[i])+"', \n";

      string microthumbnail_dirname=filefunc::getdirname(thumbnail_URLs[i]);
      microthumbnail_dirname += "microthumbnails/";
      string microthumbnail_basename=filefunc::getbasename(thumbnail_URLs[i]);
      microthumbnail_basename="micro"+microthumbnail_basename;
      string microthumbnail_filename=microthumbnail_dirname+
         microthumbnail_basename;

//      json_string += "'icon': '"+get_tomcat_URL_prefix()
//         +microthumbnail_filename+"' \n";

      json_string += "}";
      if (i < image_IDs.size()-1) json_string += ",";
//      json_string += ", \n";
   } // loop over index i labeling image epoch times

   json_string += " ] \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;

   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_temporal_neighbor()

QByteArray PhotoServer::select_temporal_neighbor()
{
   cout << "inside PhotoServer::get_temporal_neighbor()" << endl;

   unsigned int n_args=KeyValue.size();
   int Hierarchy_ID,Graph_ID,Node_ID,temporal_offset;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k << " KeyValue[k].first = "
//           << KeyValue[k].first
//           << " KeyValue[k].second = "
//           << KeyValue[k].second << endl;
      
      if (KeyValue[k].first=="HierarchyID")
      {
         Hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="GraphID")
      {
         Graph_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         Node_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="temporal_offset")
      {
         temporal_offset=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="TopicName")
      {
         topic=KeyValue[k].second; 
      }
   }

   int Sensor_ID=imagesdatabasefunc::retrieve_sensor_metadata_ID(
      postgis_database_ptr,Hierarchy_ID,Graph_ID,Node_ID);

   cout << "hierarchy_ID = " << Hierarchy_ID
        << " graph_ID = " << Graph_ID 
        << " node_ID = " << Node_ID
        << " sensor_ID = " << Sensor_ID
        << " temporal_offset = " << temporal_offset
        << endl;

   int temporal_neighbor_node_ID=-1;
   double new_epoch;
   if (temporal_offset < -9999)
   {
      temporal_neighbor_node_ID=
         imagesdatabasefunc::retrieve_beginning_temporal_node_ID(
            postgis_database_ptr,Hierarchy_ID,Node_ID,new_epoch);
   }
   else if (temporal_offset==1)
   {
      temporal_neighbor_node_ID=
         imagesdatabasefunc::retrieve_next_temporal_neighbor_node_ID(
            postgis_database_ptr,Hierarchy_ID,Node_ID,Sensor_ID,new_epoch);
   }
   else if (temporal_offset==-1)
   {
      temporal_neighbor_node_ID=
         imagesdatabasefunc::retrieve_prev_temporal_neighbor_node_ID(
            postgis_database_ptr,Hierarchy_ID,Node_ID,Sensor_ID,new_epoch);
   }
   else if (temporal_offset > 9999)
   {
      temporal_neighbor_node_ID=
         imagesdatabasefunc::retrieve_ending_temporal_node_ID(
            postgis_database_ptr,Hierarchy_ID,Node_ID,new_epoch);
   }
   cout << "temporal neighbor node ID = " << temporal_neighbor_node_ID << endl;

   if (topic.size() > 0)
   {
      int datum_ID=graphdbfunc::
         retrieve_datum_ID_for_particular_graph_hierarchy_and_node(
            postgis_database_ptr,Hierarchy_ID,Node_ID);
      issue_update_curr_node_message(
         Hierarchy_ID,Graph_ID,temporal_neighbor_node_ID,datum_ID,
         topic,client_name);
   }

   return generate_JSON_response_to_temporal_neighbor(
      temporal_neighbor_node_ID,new_epoch);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_temporal_neighbor()

QByteArray PhotoServer::generate_JSON_response_to_temporal_neighbor(
   int temporal_neighbor_node_ID,double new_epoch)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_temporal_neighbor()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " 'TemporalNeighborNodeID': "+stringfunc::number_to_string(
      temporal_neighbor_node_ID)+",  \n";
   json_string += " 'center_time': '"+stringfunc::number_to_string(
      new_epoch)+"' \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Image attribute member functions
// ==========================================================================

// Member function get_image_attributes()

QByteArray PhotoServer::get_image_attributes()
{
   cout << "Inside PhotoServer::get_image_attributes()" << endl;
   unsigned int n_args=KeyValue.size();
   int Hierarchy_ID=-1;
   string client_name,topic;
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         Hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "topic = " << topic << endl;

   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,Hierarchy_ID,campaign_ID,mission_ID);

   vector<string> attribute_keys;
   vector<vector<string> > attribute_values;
   imagesdatabasefunc::retrieve_image_attribute_keys_values_from_database(
      postgis_database_ptr,campaign_ID,mission_ID,
      attribute_keys,attribute_values);

   return generate_JSON_response_to_image_attributes(
      attribute_keys,attribute_values);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_image_attributes()

QByteArray PhotoServer::generate_JSON_response_to_image_attributes(
   vector<string>& attribute_keys,vector<vector<string> >& attribute_values)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_image_attributes()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " imageAttributes: [ \n";
   for (unsigned int n=0; n<attribute_keys.size(); n++)
   {
      string curr_key=stringfunc::find_and_replace_char(
         attribute_keys[n],"_"," ");

      json_string += "     { \n";
      json_string += "       attribute_key: '"+curr_key+"', \n";
      json_string += "       attribute_values: [";
      for (unsigned int v=0; v<attribute_values[n].size(); v++)
      {
         string curr_value=(attribute_values[n])[v];
         if (stringfunc::is_number(curr_value))
         {
            json_string += (attribute_values[n])[v];
         }
         else
         {
            json_string += "'"+(attribute_values[n])[v]+"'";
         }
         if (v < attribute_values[n].size()-1)
         {
            json_string += ",";
         }
      } // loop over index v labeling attribute values for curr_key
      json_string += "] \n";
      json_string += "     }";
      if (n < attribute_keys.size()-1) json_string += ",";
      json_string += " \n";
   }
   json_string += "   ] \n ";   
   json_string += "} \n";
   
   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_image_attributes()

void PhotoServer::select_image_attributes()
{
   cout << "Inside PhotoServer::select_image_attributes()" << endl;
   unsigned int n_args=KeyValue.size();
   int Hierarchy_ID=-1;
   string client_name,topic;
   vector<twovector> selected_attribute_keys_values;
   for (unsigned int k=0; k<n_args; k++)
   {
      cout << "k = " << k 
           << " KeyValue[k].first = " << KeyValue[k].first
           << " KeyValue[k].second = " << KeyValue[k].second
           << endl;

      string currKey=KeyValue[k].first;
      
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         currKey,"_");

      if (KeyValue[k].first=="HierarchyID")
      {
         Hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (currKey=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (currKey=="Topic")
      {
         topic=KeyValue[k].second; 
      }
      if (substrings.size()==5)
      {
         int curr_attribute_key_ID=
            stringfunc::string_to_number(substrings[2]);
         int curr_attribute_value_ID=
            stringfunc::string_to_number(substrings[4]);
         cout << "curr_attribute_key_ID = " 
              << curr_attribute_key_ID 
              << " curr_attribute_value_ID = "
              << curr_attribute_value_ID << endl;
         selected_attribute_keys_values.push_back(
            twovector(curr_attribute_key_ID,curr_attribute_value_ID));
         cout << "selected attribute key value = "
              << selected_attribute_keys_values.back() << endl;
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << "Hierarchy_ID = " << Hierarchy_ID << endl;
   cout << "client_name = " << client_name << endl;
   cout << "topic = " << topic << endl;

// Consolidate selected attribute key-value pairs into an STL map:

   typedef map<int,vector<int> > KEY_VALUE_IDS_MAP;
   KEY_VALUE_IDS_MAP selected_key_value_IDs_map;

   for (unsigned int i=0; i<selected_attribute_keys_values.size(); i++)
   {
      twovector key_value_ID=selected_attribute_keys_values[i];
      int curr_key_ID=key_value_ID.get(0);
      int curr_value_ID=key_value_ID.get(1);

      KEY_VALUE_IDS_MAP::iterator iter=selected_key_value_IDs_map.find(
         curr_key_ID);
      vector<int> value_IDs;
      if (iter != selected_key_value_IDs_map.end())
      {
         value_IDs=iter->second;
      }
      value_IDs.push_back(curr_value_ID);
      selected_key_value_IDs_map[curr_key_ID]=value_IDs;
   } // loop over index i labeling selected attribute key-value ID pairs

   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,Hierarchy_ID,campaign_ID,mission_ID);

   vector<string> all_attribute_keys;
   vector<vector<string> > all_attribute_values;
   imagesdatabasefunc::retrieve_image_attribute_keys_values_from_database(
      postgis_database_ptr,campaign_ID,mission_ID,
      all_attribute_keys,all_attribute_values);
   cout << "all_attribute_keys.size() = " << all_attribute_keys.size()
        << endl;
   cout << "all_attribute_values.size() = " << all_attribute_values.size()
        << endl;
   
   colorfunc::RGB selected_attribute_RGB(1.0 , 0.0 , 0.0);
//   colorfunc::RGB unselected_attribute_RGB(0.0 , 0.0 , 0.85);
   colorfunc::RGB unselected_attribute_RGB(0.5 , 0.5 , 0.5);

   string command="SET_CONDITION_COLOR_MAPPER";
   string key="JSON_string";
   string value=jsonfunc::generate_condition_color_mapper_JSON_string(
      all_attribute_keys,all_attribute_values,selected_key_value_IDs_map,
      selected_attribute_RGB,unselected_attribute_RGB);
   cout << "json_string = " << value << endl;
   issue_message(command,key,value,topic,client_name);
}

// ---------------------------------------------------------------------
// Member function clear_color_mapper() broadcasts an
// ActiveMQ message when the user wants to view graph nodes colored
// according to their clustering and no longer by image attributes.

void PhotoServer::clear_color_mapper()
{
//   cout << "Inside PhotoServer::clear_color_mapper()" << endl;
   unsigned int n_args=KeyValue.size();
   string client_name,topic;

   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "topic = " << topic << endl;
   clear_color_mapper(topic,client_name);
}

void PhotoServer::clear_color_mapper(string topic,string client_name)
{
//   cout << "inside PhotoServer::clear_color_mapper()" << endl;
   
   string command="CLEAR_COLOR_MAPPER";
   string key,value;
   issue_message(command,key,value,topic,client_name);
}

// ==========================================================================
// Image geocoordinates member functions
// ==========================================================================

// Member function get_image_geocoordinates()

QByteArray PhotoServer::get_image_geocoordinates()
{
   cout << "inside PhotoServer::get_image_geocoordinates()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchyID=-1;
   
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchyID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

   if (hierarchyID < 0)
   {
      string error_message="HierarchyID must be specified";
      return generate_error_JSON_response(error_message);
   }

   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,hierarchyID,campaign_ID,mission_ID);
//   cout << "campaignID = " << campaign_ID << " missionID = " << mission_ID
//        << endl;

// Recover world region info given campaign_ID   

   int UTM_zonenumber;
   string northern_hemisphere_flag;
   imagesdatabasefunc::retrieve_world_region_metadata_from_database(
      postgis_database_ptr,campaign_ID,UTM_zonenumber,
      northern_hemisphere_flag);
//   cout << "UTM_zonenumber = " << UTM_zonenumber
//        << " northern flag = " << northern_hemisphere_flag << endl;

   vector<int> datum_IDs;
   vector<threevector> camera_posns;
   imagesdatabasefunc::retrieve_all_sensor_posns_from_database(
      postgis_database_ptr,campaign_ID,mission_ID,datum_IDs,camera_posns);

   return generate_JSON_response_to_image_geocoordinates(
      UTM_zonenumber,northern_hemisphere_flag,datum_IDs,camera_posns);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_image_geocoordinates()

QByteArray PhotoServer::generate_JSON_response_to_image_geocoordinates(
   int UTM_zonenumber,string northern_hemisphere_flag,
   const vector<int>& datum_IDs,const vector<threevector>& camera_posns)
{
   cout << "Inside PhotoServer::generate_JSON_response_to_image_geocoordinates()"
        << endl;

   string json_string = "{  \n";
   json_string +="   'UTMZoneNumber': "+
      stringfunc::number_to_string(UTM_zonenumber)+",\n";
   json_string +="   'NorthernHemisphereFlag': '"+
      northern_hemisphere_flag+"',\n";
   json_string += "   'CameraPositions': [ \n";

   for (unsigned int i=0; i<datum_IDs.size(); i++)
   {
      json_string += "   { \n";
      json_string += "     'DatumID': "+stringfunc::number_to_string(
         datum_IDs[i])+", \n";
      json_string += "     'Easting': "+stringfunc::number_to_string(
         camera_posns[i].get(0))+",\n";
      json_string += "     'Northing': "+stringfunc::number_to_string(
         camera_posns[i].get(1))+",\n";
      json_string += "     'Altitude': "+stringfunc::number_to_string(
         camera_posns[i].get(2))+"\n";
      json_string += "   }";
      if (i < datum_IDs.size()-1) json_string += ",";
      json_string += "\n";
   } // loop over index i labeling image epoch times

   json_string += "  ] \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Image coloring member functions
// ==========================================================================

// Member function generate_color_histogram()

QByteArray PhotoServer::generate_color_histogram()
{
//   cout << "Inside PhotoServer::generate_color_histogram()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;

   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " node_ID = " << node_ID << endl;
   if (hierarchy_ID < 0 || node_ID < 0)
   {
      string error_msg="Bad hierarchy_ID and/or node_ID input";
      return generate_error_JSON_response(error_msg);
   }
   string image_URL=imagesdatabasefunc::get_image_URL(
      postgis_database_ptr,hierarchy_ID,node_ID);
//   cout << "image_URL = " << image_URL << endl;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      image_URL,NULL);
   if (texture_rectangle_ptr->get_VideoType()==texture_rectangle::unknown)
   {
      string error_msg="Could not import image from file";
      return generate_error_JSON_response(error_msg);
   }

   bool generate_quantized_color_image_flag=true;
   string subdir="/data/ImageEngine/Quantized_Colors/";
   filefunc::dircreate(subdir);
   string basename=filefunc::getbasename(image_URL);
   basename="quantized_"+basename;
   string quantized_color_image_filename=subdir+basename;

   vector<double> color_histogram=videofunc::generate_color_histogram(
      generate_quantized_color_image_flag,texture_rectangle_ptr,
      quantized_color_image_filename);

   vector<string> color_labels;
   color_labels.push_back("Red");
   color_labels.push_back("Orange");
   color_labels.push_back("Yellow");
   color_labels.push_back("Green");
   color_labels.push_back("Blue");
   color_labels.push_back("Purple");
   color_labels.push_back("Black");
   color_labels.push_back("White");
   color_labels.push_back("Grey");
   color_labels.push_back("Brown");
   templatefunc::Quicksort_descending(color_histogram,color_labels);
   
   int npx=texture_rectangle_ptr->getWidth();
   int npy=texture_rectangle_ptr->getHeight();
   delete texture_rectangle_ptr;

   while (!filefunc::fileexist(quantized_color_image_filename))
   {
      cout << "Waiting for quantized image to be generated" << endl;
      sleep(1);
   }

   return generate_JSON_response_to_color_histogram(
      quantized_color_image_filename,npx,npy,color_histogram,color_labels);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_color_histogram()

QByteArray PhotoServer::generate_JSON_response_to_color_histogram(
   string quantized_image_filename,int npx,int npy,
   const vector<double>& color_histogram,
   const vector<string>& color_labels)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_color_histogram()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " 'QuantizedImageFilename': '"+quantized_image_filename
      +"' , \n";
   json_string += " 'Npx': '"+stringfunc::number_to_string(npx)+"' , \n";
   json_string += " 'Npy': '"+stringfunc::number_to_string(npy)+"' , \n";

   for (unsigned int c=0; c<color_histogram.size(); c++)
   {
      string key="Color_"+stringfunc::number_to_string(c);
      string color_percentage=
         stringfunc::number_to_string(100*color_histogram[c],1);
      string value="['"+color_labels[c]+"','"+color_percentage+"']";
      json_string += " '"+key+"': "+value;
      if (c < color_histogram.size()-1) json_string += ",";
      json_string += "\n";
   }
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function restore_original_colors()

QByteArray PhotoServer::restore_original_colors()
{
//   cout << "Inside PhotoServer::restore_original_colors()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;

   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "hierarchy_ID = " << hierarchy_ID
//        << " node_ID = " << node_ID << endl;
   if (hierarchy_ID < 0 || node_ID < 0)
   {
      string error_msg="Bad hierarchy_ID and/or node_ID input";
      return generate_error_JSON_response(error_msg);
   }
   string image_URL=imagesdatabasefunc::get_image_URL(
      postgis_database_ptr,hierarchy_ID,node_ID);
//   cout << "image_URL = " << image_URL << endl;

   unsigned int npx,npy;
   imagefunc::get_image_width_height(image_URL,npx,npy);

   string json_string = "{  \n";
   json_string += " 'ImageURL': '"+image_URL+"' , \n";
   json_string += " 'Npx': '"+stringfunc::number_to_string(npx)+"' , \n";
   json_string += " 'Npy': '"+stringfunc::number_to_string(npy)+"' \n";
   json_string += "} \n";

//   cout << "json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_dominant_colors()

void PhotoServer::select_dominant_colors()
{
//   cout << "Inside PhotoServer::select_dominant_colors()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   string primary_color,secondary_color,tertiary_color;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="PrimaryColor")
      {
         primary_color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="SecondaryColor")
      {
         secondary_color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="TertiaryColor")
      {
         tertiary_color=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "primary color = " << primary_color << endl;
//   cout << "secondary color = " << secondary_color << endl;
//   cout << "tertiary color = " << tertiary_color << endl;

   colorfunc::Color primaryColor=colorfunc::string_to_color(primary_color);

   colorfunc::RGB selected_attribute_RGB(1.0 , 0.0 , 0.0);
   colorfunc::RGB unselected_attribute_RGB(0.5 , 0.5 , 0.5);
//   colorfunc::RGB unselected_attribute_RGB(0.0 , 0.0 , 0.85);

   string command="SET_CONDITION_COLOR_MAPPER";
   string key="JSON_string";
   string value=jsonfunc::generate_dominant_colorings_JSON_string(
      primary_color,secondary_color,tertiary_color,
      selected_attribute_RGB,unselected_attribute_RGB);

   cout << "json_string = " << value << endl;
   issue_message(command,key,value,topic,client_name);
}

// ==========================================================================
// Image human faces member functions
// ==========================================================================

// Member function select_human_faces()

void PhotoServer::select_human_faces()
{
   cout << "Inside PhotoServer::select_human_faces()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   vector<int> n_human_faces;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
//      cout << "k = " << k 
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second
//           << endl;
      
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="n_faces1")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_human_faces.push_back(1);
      }
      else if (KeyValue[k].first=="n_faces2")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_human_faces.push_back(2);
      }
      else if (KeyValue[k].first=="n_faces3")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_human_faces.push_back(3);
      }
      else if (KeyValue[k].first=="n_faces4")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_human_faces.push_back(4);
      }
      else if (KeyValue[k].first=="n_faces5")
      {
         if (stringfunc::string_to_boolean(KeyValue[k].second))
            n_human_faces.push_back(5);
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
   cout << "hierarchy_ID = " << hierarchy_ID << endl;
   cout << "topic = " << topic << endl;

   string command="SET_CONDITION_COLOR_MAPPER";
   string key="JSON_string";

   colorfunc::RGB unselected_attribute_RGB(0.5 , 0.5 , 0.5);
   colorfunc::RGB selected_attribute_RGB(1.0 , 0.0, 0.0);
//   colorfunc::RGB selected_attribute_RGB=
//      colorfunc::get_RGB_values(colorfunc::get_color(n_human_faces));

   string value=jsonfunc::generate_multi_object_selection_JSON_string(
      n_human_faces,"human_faces",
      selected_attribute_RGB,unselected_attribute_RGB);
   cout << "json_string = " << value << endl;

   issue_message(command,key,value,topic,client_name);
}

// ---------------------------------------------------------------------
// Member function display_face_circles()

QByteArray PhotoServer::display_face_circles()
{
   cout << "Inside PhotoServer::display_face_circles()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "node_ID = " << node_ID << endl;
//   cout << "topic = " << topic << endl;

   vector<twovector> center;
   vector<double> radius;
   imagesdatabasefunc::retrieve_detected_face_circles_from_database(
      postgis_database_ptr,hierarchy_ID,node_ID,center,radius);
   int n_detected_faces=center.size();

   string image_URL=imagesdatabasefunc::get_image_URL(
      postgis_database_ptr,hierarchy_ID,node_ID);
//   cout << "image_URL = " << image_URL << endl;

   string subdir="/data/ImageEngine/Human_Faces/";
   filefunc::dircreate(subdir);
   string basename=filefunc::getbasename(image_URL);
   basename="human_faces_"+basename;
   string human_faces_image_filename=subdir+basename;

// Restrict size of human_faces_images so that purple face circles
// show up clearly:

   int max_xdim=1200;
   int max_ydim=1200;
   videofunc::downsize_image(
      image_URL,max_xdim,max_ydim,human_faces_image_filename);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      human_faces_image_filename,NULL);
   if (texture_rectangle_ptr->get_VideoType()==texture_rectangle::unknown)
   {
      string error_msg="Could not import image from file";
      return generate_error_JSON_response(error_msg);
   }

   videofunc::display_circles(
      texture_rectangle_ptr,human_faces_image_filename,center,radius);

   int npx=texture_rectangle_ptr->getWidth();
   int npy=texture_rectangle_ptr->getHeight();
   delete texture_rectangle_ptr;

   return generate_JSON_response_to_human_faces(
      n_detected_faces,human_faces_image_filename,npx,npy);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_human_faces()

QByteArray PhotoServer::generate_JSON_response_to_human_faces(
   int n_detected_faces,string human_faces_image_filename,int npx,int npy)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_human_faces()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " 'FacesImageFilename': '"+human_faces_image_filename
      +"' , \n";
   json_string += " 'Npx': '"+stringfunc::number_to_string(npx)+"' , \n";
   json_string += " 'Npy': '"+stringfunc::number_to_string(npy)+"' , \n";
   json_string += " 'Nfaces': '"+stringfunc::number_to_string(
      n_detected_faces)+"' \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function get_n_detected_faces()

QByteArray PhotoServer::get_n_detected_faces()
{
   cout << "Inside PhotoServer::get_n_detected_faces()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="NodeID")
      {
         node_ID=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
   cout << "node_ID = " << node_ID << endl;
//   cout << "topic = " << topic << endl;

   vector<twovector> center;
   vector<double> radius;
   imagesdatabasefunc::retrieve_detected_face_circles_from_database(
      postgis_database_ptr,hierarchy_ID,node_ID,center,radius);
   int n_detected_faces=center.size();
   cout << "n_detected_faces = " << n_detected_faces << endl;

   return generate_JSON_response_to_n_faces(n_detected_faces);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_human_faces()

QByteArray PhotoServer::generate_JSON_response_to_n_faces(
   int n_detected_faces)
{
//   cout << "Inside PhotoServer::generate_JSON_response_to_n_faces()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " 'Nfaces': '"+stringfunc::number_to_string(
      n_detected_faces)+"' \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Video keyframe member functions
// ==========================================================================

// Member function display_video_keyframes() assigns varying RGB
// colors to distinct video keyframe IDs.  It then generates and
// exports a keyframe_colorings JSON file which follows the
// SET_DISCRETE_COLOR_MAPPER syntax specified by Michael Yee in
// November 2013.  An ActiveMQ message containing the tomcat URL for
// the JSON file is broadcast.  The graph viewer subsequently imports
// the JSON information and recolors all nodes in the image graph
// to illustrate keyframe "temporal segments" in the sinusoidal video
// clip display.

void PhotoServer::display_video_keyframes()
{
//   cout << "Inside PhotoServer::display_video_keyframes()" << endl;

   unsigned int n_args=KeyValue.size();
   int hierarchy_ID=-1;
   int node_ID=-1;
   string topic,client_name="";
   for (unsigned int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="HierarchyID")
      {
         hierarchy_ID=stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="ClientName")
      {
         client_name=KeyValue[k].second; 
      }
      else if (KeyValue[k].first=="Topic")
      {
         topic=KeyValue[k].second; 
      }
   } // loop over index k labeling KeyValue key possibilities
//   cout << "client_name = " << client_name << endl;
//   cout << "hierarchy_ID = " << hierarchy_ID << endl;
//   cout << "topic = " << topic << endl;

   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_database_ptr,hierarchy_ID,campaign_ID,mission_ID);
//   cout << "campaign_ID = " << campaign_ID
//        << " mission_ID = " << mission_ID << endl;

   vector<int> keyframe_image_IDs;
   videosdatabasefunc::get_distinct_keyframe_IDs(
      postgis_database_ptr,campaign_ID,mission_ID,keyframe_image_IDs);
   
//   cout << "keyframe_image_IDs.size() = " 
//        << keyframe_image_IDs.size() << endl;
//   for (unsigned int k=0; k<keyframe_image_IDs.size(); k++)
//   {
//      cout << "k = " << k 
//           << " keyframe_image_ID = " << keyframe_image_IDs[k]
//           << endl;
//   }

// Assign qualitatively-different hues to successive keyframes.  Also
// choose saturations and values which don't yield keyframe node colorings
// that are too washed out or dark:

   double delta_hue=107;	// prime
   vector<colorfunc::RGB> keyframe_colors;
   for (unsigned int n=0; n<keyframe_image_IDs.size(); n++)
   {
      double h=0+n*delta_hue;
      h=basic_math::phase_to_canonical_interval(h,0,360);
      double v=0.5+0.5*nrfunc::ran1();
      double s=0.5+0.5*nrfunc::ran1();

      double r,g,b;
      colorfunc::hsv_to_RGB(h,s,v,r,g,b);
      
      colorfunc::RGB node_RGB(r,g,b);
      keyframe_colors.push_back(node_RGB);
   }
   colorfunc::RGB defaultColor_RGB(0.5,0.5,0.5);

   string command="SET_DISCRETE_COLOR_MAPPER";
   string json_string=jsonfunc::generate_discrete_color_mapper_JSON_string(
      "video_keyframes",defaultColor_RGB,keyframe_image_IDs,keyframe_colors);

//   string key="mapping_JSON";
//   cout << "json_string = " << json_string << endl;
//   issue_message(command,key,json_string,topic,client_name);

// Export json string to output text file:

   string image_URL=imagesdatabasefunc::get_image_URL(
      postgis_database_ptr,campaign_ID,mission_ID,0);
   string json_filename=filefunc::getdirname(image_URL)+
      "keyframe_colorings.json";

   ofstream json_stream;
   filefunc::openfile(json_filename,json_stream);
   json_stream << json_string << endl;
   filefunc::closefile(json_filename,json_stream);

   string key="mapping_JSON_URL";
   string tomcat_URL=get_tomcat_URL_prefix()+json_filename;
//   cout << "tomcat_URL = " << tomcat_URL << endl;
   issue_message(command,key,tomcat_URL,topic,client_name);

//   cout << "at end of PhotoServer::display_video_keyframes()" << endl;

}
