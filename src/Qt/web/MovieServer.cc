// ==========================================================================
// MOVIESERVER class file
// ==========================================================================
// Last updated on 12/25/11; 1/1/12; 1/17/12
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "postgres/databasefuncs.h"
#include "image/imagefuncs.h"
#include "track/mover_funcs.h"
#include "Qt/web/MovieServer.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "video/photodbfuncs.h"
#include "geometry/polyline.h"
#include "video/texture_rectangle.h"
#include "track/tracks_group.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void MovieServer::allocate_member_objects()
{
}		       

void MovieServer::initialize_member_objects()
{
   movie_subdir_selected_flag=false;
   selected_fieldtest_ID=selected_mission_ID=selected_platform_ID=
      selected_sensor_ID=-1;
   gis_database_ptr=NULL;
   viewer_messenger_ptr=NULL;
   CM_ptr=NULL;
   MoviesGroup_ptr=NULL;
   texture_rectangle_ptr=NULL;
   TOCHUD_ptr=NULL;
   FeaturesGroup_ptr=NULL;
   FeaturePickHandler_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   PolyLinePickHandler_ptr=NULL;

   starting_image_subdir="/data/tech_challenge/field_tests";
}

MovieServer::MovieServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
//   cout << "inside MovieServer constructor, port = " << port << endl;
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
MovieServer::~MovieServer()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray MovieServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside MovieServer:get() method" << endl;

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

   if (URL_path=="/SET_MOVIE_FRAMES_SUBDIR/")
   {
      set_movie_frames_subdir();
   }
   else if (URL_path=="/RELATE_FRAMES_TO_WORLD_TIME/")
   {
      relate_frames_to_local_world_time();
   }
   else if (URL_path=="/RESET_MOVIE_PARAMS/")
   {
      reset_movie_params();
   }
   else if (URL_path=="/GOTO_MOVIE_START/")
   {
      goto_movie_start();
   }
   else if (URL_path=="/REVERSE_MOVIE/")
   {
      reverse_movie();
   }
   else if (URL_path=="/DECREMENT_MOVIE_FRAME/")
   {
      decrement_movie_frame();
   }
   else if (URL_path=="/PAUSE_MOVIE/")
   {
      pause_movie();
   }
   else if (URL_path=="/INCREMENT_MOVIE_FRAME/")
   {
      increment_movie_frame();
   }
   else if (URL_path=="/PLAY_MOVIE/")
   {
      play_movie();
   }
   else if (URL_path=="/GOTO_MOVIE_END/")
   {
      goto_movie_end();
   }
   else if (URL_path=="/INCREASE_FRAME_RATE/")
   {
      increase_frame_rate();
   }
   else if (URL_path=="/DECREASE_FRAME_RATE/")
   {
      decrease_frame_rate();
   }
   else if (URL_path=="/SET_FRAME_PERCENTAGE/")
   {
      reset_movie_params();
   }
   else if (URL_path=="/GET_NFRAMES_AND_CURR_FRAMENUMBER/")
   {
      return generate_JSON_response_to_frames_query();
   }
   else if (URL_path=="/SELECT_MOVIE_FRAMES_SUBDIR/")
   {
      return select_movie_frames_subdir_via_GUI();
   }
   else if (URL_path=="/SELECT_IMAGE_STILLS_SUBDIR/")
   {
      return select_image_stills_subdir_via_GUI();
   }

   else if (URL_path=="/Pick_Mission/")
   {
      pick_mission();
      bool images_retrieved_flag=retrieve_movie_frames();
//      cout << "images_retrieved_flag = " << images_retrieved_flag << endl;
      return generate_JSON_response_to_picked_mission(images_retrieved_flag);
   }
   else if (URL_path=="/Calibrate_frames_to_local_world_time/")
   {
      int n_calibrated_photos=calibrate_frames_to_local_world_time();
      string json_string=
         generate_JSON_response_to_frame_calibration(n_calibrated_photos);

      string progress_type="calibrate_frames";   
      viewer_messenger_ptr->broadcast_finished_progress(progress_type);

      cout << "Final json_string = " << json_string << endl;
      return QByteArray(json_string.c_str());

   }
   else if (URL_path=="/Extract_frame_geometries/")
   {
      int n_calibrated_photos=extract_frame_geometries();
      string json_string=generate_JSON_response_to_frame_calibration(
         n_calibrated_photos);

      string progress_type="frame_geometries";   
      viewer_messenger_ptr->broadcast_finished_progress(progress_type);

      cout << "Final json_string = " << json_string << endl;
      return QByteArray(json_string.c_str());
   }
   else if (URL_path=="/FLAG_FRAMES/")
   {
      return flag_frames();
   }
   else if (URL_path=="/GET_IMPORTANCE_INTERVALS/")
   {
      return generate_JSON_response_to_importance_intervals();
   }
   else if (URL_path=="/GET_MULTIMISSION_IMPORTANCE_INTERVALS/")
   {
      return generate_JSON_for_multimission_importance_intervals();
   }
   else if (URL_path=="/ANNOTATE_CURRENT_FRAME/")
   {
      annotate_current_frame();
   }
   else if (URL_path=="/Capture_Viewer_Screen/")
   {
      return capture_viewer_screen();
   }
   else if (URL_path=="/Start_Recording_Movie/")
   {
      Start_Recording_Movie();
   }
   else if (URL_path=="/Stop_Recording_Movie/")
   {
      Stop_Recording_Movie();
      return generate_AVI_movie();
   }
   else if (URL_path=="/INSERT_IMAGE_FEATURE/")
   {
      return insert_image_feature();
   }
   else if (URL_path=="/SELECT_IMAGE_FEATURE/")
   {
      int feature_ID=set_selected_Feature_ID();
      double max_blink_period=3;      // secs
      FeaturesGroup_ptr->blink_Geometrical(feature_ID,max_blink_period);
      return generate_JSON_response_to_feature_event();
   }
   else if (URL_path=="/UNSELECT_IMAGE_FEATURE/")
   {
      FeaturesGroup_ptr->set_selected_Graphical_ID(-1);
   }
   else if (URL_path=="/UPDATE_IMAGE_FEATURE/")
   {
      return update_image_feature();
   }
   else if (URL_path=="/START_DRAG_IMAGE_FEATURE/")
   {
      drag_image_feature();
   }
   else if (URL_path=="/STOP_DRAG_IMAGE_FEATURE/")
   {
      return stop_drag_image_feature();
   }
   else if (URL_path=="/DELETE_IMAGE_FEATURE/")
   {
      return delete_image_feature();
   }
   else if (URL_path=="/PURGE_IMAGE_FEATURES/")
   {
      return purge_image_features();
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
   
   else if (URL_path=="/INSERT_IMAGE_POLYLINE/")
   {
      return insert_image_polyline();
   }
   else if (URL_path=="/SELECT_IMAGE_POLYLINE/")
   {
      int polyline_ID=set_selected_PolyLine_ID();
      double max_blink_period=3;      // secs
      PolyLinesGroup_ptr->blink_Geometrical(polyline_ID,max_blink_period);
      return generate_JSON_response_to_polyline_event();
   }
   else if (URL_path=="/UNSELECT_IMAGE_POLYLINE/")
   {
      PolyLinesGroup_ptr->set_selected_Graphical_ID(-1);
      unselect_all_PolyLine_vertices();
      return generate_JSON_response_to_polyline_event();
   }
   else if (URL_path=="/UPDATE_IMAGE_POLYLINE/")
   {
      return update_image_polyline();
   }
   else if (URL_path=="/START_DRAG_IMAGE_POLYLINE/")
   {
      drag_image_polyline();
   }
   else if (URL_path=="/STOP_DRAG_IMAGE_POLYLINE/")
   {
      return stop_drag_image_polyline();
   }
   else if (URL_path=="/START_SCALE_IMAGE_POLYLINE/")
   {
      scale_image_polyline();
   }
   else if (URL_path=="/STOP_SCALE_IMAGE_POLYLINE/")
   {
      return stop_scale_image_polyline();
   }
   else if (URL_path=="/START_ROTATE_IMAGE_POLYLINE/")
   {
      rotate_image_polyline();
   }
   else if (URL_path=="/STOP_ROTATE_IMAGE_POLYLINE/")
   {
      return stop_rotate_image_polyline();
   }
   else if (URL_path=="/DOUBLE_IMAGE_POLYLINE_VERTICES/")
   {
      return double_image_polyline_vertices();
   }
   else if (URL_path=="/DELETE_IMAGE_POLYLINE/")
   {
      return delete_image_polyline();
   }
   else if (URL_path=="/PURGE_IMAGE_POLYLINES/")
   {
      return purge_image_polylines();
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

   else if (URL_path=="/SELECT_IMAGE_VERTEX/")
   {
      set_selected_Vertex_ID();
   }
   else if (URL_path=="/UNSELECT_IMAGE_VERTEX/")
   {
      unselect_image_vertex();
   }
   else if (URL_path=="/UPDATE_IMAGE_VERTEX/")
   {
      return update_image_vertex();
   }
   else if (URL_path=="/START_DRAG_IMAGE_VERTEX/")
   {
      drag_image_vertex();
   }
   else if (URL_path=="/STOP_DRAG_IMAGE_VERTEX/")
   {
      return stop_drag_image_vertex();
   }
   else if (URL_path=="/DELETE_IMAGE_VERTEX/")
   {
      return delete_image_vertex();
   }
   else if (URL_path=="/INCREASE_POLYLINE_VERTEX_SIZE/")
   {
      increase_polyline_vertex_size();
   }
   else if (URL_path=="/DECREASE_POLYLINE_VERTEX_SIZE/")
   {
      decrease_polyline_vertex_size();
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray MovieServer::post(const QUrl& url, const QByteArray& postData,
                             QHttpResponseHeader& responseHeader)
{
   cout << "inside MovieServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   MovieServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;

/*
   if (URL_path=="/Set_Tour_Path/")
   {
      string error_msg;
      set_tour_path(postData,error_msg);
      ModeController_ptr->setState(ModeController::RUN_MOVIE);
      Grid_ptr->set_mask(0);

// Jump to starting photo position and then suppress movie point cloud:

      PhotoTour* PhotoTour_ptr=PhotoToursGroup_ptr->get_PhotoTour_ptr(0);
      int starting_OBSFRUSTUM_ID=PhotoTour_ptr->get_starting_OBSFRUSTUM_ID();
      OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(
         starting_OBSFRUSTUM_ID,0);
      PhotoTour_ptr->set_prev_OBSFRUSTUM_ID(starting_OBSFRUSTUM_ID);
      suppress_movie_point_cloud();

      return generate_JSON_response_to_tourpath_entry();
   }
*/

   return doc.toByteArray();
}

// ==========================================================================
// Movie selection member functions
// ==========================================================================

// Member function select_image_stills_subdir_via_GUI()

QByteArray MovieServer::select_image_stills_subdir_via_GUI()
{
   cout << "inside MovieServer::select_image_stills_subdir_via_GUI()" 
        << endl;
  
// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

   QWidget* window_ptr=new QWidget;
   window_ptr->move(835,0);
//   cout << "window_ptr->x() = " << window_ptr->x() << endl;
//   cout << "window_ptr->y() = " << window_ptr->y() << endl;
   window_ptr->setWindowTitle("Movie frame folder picker");

   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Open Image Stills Sequence", starting_image_subdir.c_str(), 
   "Image Files (*.png *.PNG *.jpg *.JPG *.bmp *.tif)");
   string image_filename=fileName.toStdString();
   cout << "Selected image filename = " << image_filename << endl;
   string movie_frames_subdir=filefunc::getdirname(image_filename);
   cout << "movie_frames_subdir = " << movie_frames_subdir << endl;

   AnimationController_ptr->store_unordered_image_filenames(
      movie_frames_subdir);
   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
   cout << "number_of_images = " << number_of_images << endl;

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' name:

   first_frame_filename=AnimationController_ptr->
      get_next_ordered_image_filename();
   cout << "first_frame_filename = " << first_frame_filename << endl;

   AnimationController_ptr->set_nframes(number_of_images);

   regenerate_Movie_to_display();

   return generate_JSON_response_to_stills_selection(number_of_images);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_stills_selection()

QByteArray MovieServer::generate_JSON_response_to_stills_selection(int Nimages)
{
//   cout << "Inside MovieServer::generate_JSON_response_to_stills_selection()"
//        << endl;
   
   string curr_image_filename=AnimationController_ptr->
      get_ordered_image_filename(0);
//   cout << "curr_image_filename = " << curr_image_filename << endl;

   int npx,npy;
   imagefunc::get_image_width_height(curr_image_filename,npx,npy);

   string json_string = "{  \n";
   json_string += " \"NImages\": "+
      stringfunc::number_to_string(Nimages)+", \n";
   json_string += " \"CurrImageFilename\": ";
   json_string += " \""+curr_image_filename+"\", \n";
   json_string += " \"Npx\": "+
      stringfunc::number_to_string(npx)+", \n";
   json_string += " \"Npy\": "+
      stringfunc::number_to_string(npy)+" \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function select_movie_frames_via_GUI() pops open a Qt file
// dialog box querying the user to enter the folder in which a
// sequence of PNG or JPG images reside.  It then scans through all
// these files and determines the min and max frame numbers.  The
// starting frame number and total number of images is stored within
// *AnimationController_ptr.

QByteArray MovieServer::select_movie_frames_subdir_via_GUI()
{
//   cout << "inside MovieServer::select_movie_frames_subdir_via_GUI()" 
//        << endl;
  
// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

   QWidget* window_ptr=new QWidget;
   window_ptr->move(835,0);
//   cout << "window_ptr->x() = " << window_ptr->x() << endl;
//   cout << "window_ptr->y() = " << window_ptr->y() << endl;
   window_ptr->setWindowTitle("Movie frame folder picker");


   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Open Movie Frame Sequence", starting_image_subdir.c_str(), 
   "Image Files (*.png *.PNG *.jpg *.JPG *.bmp *.tif)");
   string image_filename=fileName.toStdString();
//   cout << "Selected image filename = " << image_filename << endl;
   string movie_frames_subdir=filefunc::getdirname(image_filename);
//   cout << "movie_frames_subdir = " << movie_frames_subdir << endl;

   AnimationController_ptr->store_ordered_image_filenames(movie_frames_subdir);
   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
//   cout << "number_of_images = " << number_of_images << endl;

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' name:

   int min_photo_number=-1;
   int max_photo_number=-1;
   first_frame_filename=videofunc::find_min_max_photo_numbers(
      movie_frames_subdir,min_photo_number,max_photo_number);

   int Nimages=max_photo_number-min_photo_number+1;
   if (number_of_images > Nimages)
   {
      Nimages=number_of_images;
      min_photo_number=0;
      max_photo_number=Nimages-1;
   }
//   cout << "min_photo_number = " << min_photo_number
//        << " max_photo_number = " << max_photo_number
//        << " Nimages = " << Nimages << endl;

   first_frame_filename=AnimationController_ptr->
      get_next_ordered_image_filename();
//   cout << "first_frame_filename = " << first_frame_filename << endl;

   AnimationController_ptr->set_nframes(Nimages);
   AnimationController_ptr->set_frame_counter_offset(min_photo_number);

   correlate_frame_numbers_and_world_times();
   regenerate_Movie_to_display();

   return generate_JSON_response_to_movie_selection(
      min_photo_number,max_photo_number,Nimages);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_movie_selection()

QByteArray MovieServer::generate_JSON_response_to_movie_selection(
   int min_photo_number, int max_photo_number, int Nimages)
{
//   cout << "Inside MovieServer::generate_JSON_response_to_movie_selection()"
//        << endl;
   
   string json_string = "{  \n";
   json_string += " \"MinImageNumber\": "+
      stringfunc::number_to_string(min_photo_number)+", \n";
   json_string += " \"MaxImageNumber\": "+
      stringfunc::number_to_string(max_photo_number)+", \n";
   json_string += " \"NImages\": "+
      stringfunc::number_to_string(Nimages)+" \n";
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
void MovieServer::set_movie_frames_subdir()
{
//   cout << "inside MovieServer::set_movie_frames_subdir()" << endl;
   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "key = " << key
//           << " value = " << value << endl;

      if (key=="ImagePathCopy" && value.size() > 0)
      {
         string movie_pathname=value;
//         cout << "movie_pathname = " << movie_pathname << endl;
         movie_frames_subdir=filefunc::getdirname(movie_pathname);
//         cout << "Movie frames subdir = " << movie_frames_subdir << endl;
         movie_subdir_selected_flag=true;
      }
   } // loop over index k labeling KeyValue key possibilities
}

// ---------------------------------------------------------------------
// Member function relate_frames_to_local_world_time()

void MovieServer::relate_frames_to_local_world_time()
{
//   cout << "inside MovieServer::relate_frames_to_local_world_time()" << endl;
   int n_args=KeyValue.size();

   int true_start_framenumber,true_stop_framenumber;
   string startDate,stopDate,startTime,stopTime;
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "key = " << key
//           << " value = " << value << endl;

      if (key=="StartFrameNumber" && value.size() > 0)
      {
         true_start_framenumber=stringfunc::string_to_number(value);
      }
      else if (key=="StopFrameNumber" && value.size() > 0)
      {
         true_stop_framenumber=stringfunc::string_to_number(value);
      }
      else if (key=="StartDate" && value.size() > 0)
      {
         startDate=value;
      }
      else if (key=="StopDate" && value.size() > 0)
      {
         stopDate=value;
      }
      else if (key=="StartTime" && value.size() > 0)
      {
         startTime=value;
      }
      else if (key=="StopTime" && value.size() > 0)
      {
         stopTime=value;
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "true_start_framenumber = " << true_start_framenumber
//        << " true_stop_framenumber = " << true_stop_framenumber << endl;
//   cout << "startDate = " << startDate 
//        << " stopDate = " << stopDate << endl;
//   cout << "startTime = " << startTime
//        << " stopTime = " << stopTime << endl;

   vector<string> startDate_substrings=
      stringfunc::decompose_string_into_substrings(startDate,"-");
   vector<string> stopDate_substrings=
      stringfunc::decompose_string_into_substrings(stopDate,"-");
   vector<string> startTime_substrings=
      stringfunc::decompose_string_into_substrings(startTime,":");
   vector<string> stopTime_substrings=
      stringfunc::decompose_string_into_substrings(stopTime,":");
   int year_start=stringfunc::string_to_number(startDate_substrings[0]);
   int year_stop=stringfunc::string_to_number(stopDate_substrings[0]);
   int month_start=stringfunc::string_to_number(startDate_substrings[1]);
   int month_stop=stringfunc::string_to_number(stopDate_substrings[1]);
   int day_start=stringfunc::string_to_number(startDate_substrings[2]);
   int day_stop=stringfunc::string_to_number(stopDate_substrings[2]);

//   cout << "year_start = " << year_start << " month_start = " << month_start
//        << " day_start = " << day_start << endl;
//   cout << "year_stop = " << year_stop << " month_stop = " << month_stop
//        << " day_stop = " << day_stop << endl;

   int hour_start=stringfunc::string_to_number(startTime_substrings[0]);
   int hour_stop=stringfunc::string_to_number(stopTime_substrings[0]);
   int min_start=stringfunc::string_to_number(startTime_substrings[1]);
   int min_stop=stringfunc::string_to_number(stopTime_substrings[1]);

// User may not necessarily have entered any secs entry.  If not, set
// secs to default zero value:

   double sec_start=0;
   double sec_stop=0;
   if (startTime_substrings.size() > 2)
   {
      sec_start=stringfunc::string_to_number(startTime_substrings[2]);
   }

   if (stopTime_substrings.size() > 2)
   {
      sec_stop=stringfunc::string_to_number(stopTime_substrings[2]);
   }
   
//   cout << "hour_start = "<< hour_start << " min_start = " << min_start
//        << " sec_start = " << sec_start << endl << endl;
//   cout << "hour_stop = "<< hour_stop << " min_stop = " << min_stop
//        << " sec_stop = " << sec_stop << endl;

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->set_local_time(
      year_start,month_start,day_start,hour_start,min_start,sec_start);
   double start_secs_since_ref_date=clock_ptr->
      secs_elapsed_since_reference_date();
   clock_ptr->set_local_time(
      year_stop,month_stop,day_stop,hour_stop,min_stop,sec_stop);
   double stop_secs_since_ref_date=clock_ptr->
      secs_elapsed_since_reference_date();
   
//   cout.precision(13);
//   cout << "start_secs_since_ref_date = " << start_secs_since_ref_date << endl;
//   cout << "stop_secs_since_ref_date = " << stop_secs_since_ref_date << endl;
   
   double timeoverframes_ratio=
      (stop_secs_since_ref_date-start_secs_since_ref_date)/
      (true_stop_framenumber-true_start_framenumber);

   int true_first_framenumber=
      AnimationController_ptr->get_first_framenumber()+
      AnimationController_ptr->get_frame_counter_offset();
   int true_last_framenumber=AnimationController_ptr->get_last_framenumber()+
      AnimationController_ptr->get_frame_counter_offset();
   double first_secs_since_ref_date=start_secs_since_ref_date+
      timeoverframes_ratio*(true_first_framenumber-true_start_framenumber);
   double last_secs_since_ref_date=start_secs_since_ref_date+
      timeoverframes_ratio*(true_last_framenumber-true_start_framenumber);

//   cout << "true_first_framenumber = " << true_first_framenumber << endl;
//   cout << "true_last_framenumber = " << true_last_framenumber << endl;

   AnimationController_ptr->specify_extremal_frame_world_times(
      AnimationController_ptr->get_first_framenumber(),
      AnimationController_ptr->get_last_framenumber(),
      first_secs_since_ref_date,last_secs_since_ref_date);
   Operations_ptr->get_ImageNumberHUD_ptr()->
      set_display_movie_world_time_flag(true);
}

// ==========================================================================
// Movie playback manipulation member functions:
// ==========================================================================

void MovieServer::goto_movie_start()
{
//   cout << "inside MovieServer::goto_movie_start()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->set_curr_framenumber(AnimationController_ptr->
      get_first_framenumber());
   AnimationController_ptr->setState(AnimationController::PAUSE);
}

void MovieServer::reverse_movie()
{
//   cout << "inside MovieServer::reverse_movie()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->setState(AnimationController::REVERSE);
}

void MovieServer::decrement_movie_frame()
{
//   cout << "inside MovieServer::decrement_movie_frame()" << endl;

   if (AnimationController_ptr==NULL) return;

// To avoid race condition, we explicitly copy DECREMENT_FRAME lines
// from AnimationController::update() rather than set AnimationController
// state to DECREMENT_FRAME:

   AnimationController_ptr->decrement_frame_counter();
   AnimationController_ptr->update_all_Graphicals_animation();
   AnimationController_ptr->setState(AnimationController::PAUSE);

   broadcast_curr_image_URL();
}

void MovieServer::pause_movie()
{
//   cout << "inside MovieServer::pause_movie()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->setState(AnimationController::PAUSE);
}

void MovieServer::increment_movie_frame()
{
   cout << "inside MovieServer::increment_movie_frame()" << endl;

   if (AnimationController_ptr==NULL) return;

// To avoid race condition, we explicitly copy INCREMENT_FRAME lines
// from AnimationController::update() rather than set AnimationController
// state to INCREMENT_FRAME:

   AnimationController_ptr->increment_frame_counter();
   AnimationController_ptr->update_all_Graphicals_animation();
   AnimationController_ptr->setState(AnimationController::PAUSE);

   broadcast_curr_image_URL();
}

void MovieServer::play_movie()
{
//   cout << "inside MovieServer::play_movie()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->setState(AnimationController::PLAY);
}

void MovieServer::goto_movie_end()
{
//   cout << "inside MovieServer::goto_movie_end()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->set_curr_framenumber(AnimationController_ptr->
      get_last_framenumber());
//   cout << "AC_ptr->get_last_framenumber() = "
//        << AnimationController_ptr->get_last_framenumber() << endl;
   AnimationController_ptr->setState(AnimationController::PAUSE);
}

void MovieServer::broadcast_curr_image_URL()
{
   cout << "inside MovieServer::broadcast_curr_image_URL()" << endl;

   if (AnimationController_ptr==NULL) return;

   int n_frames=AnimationController_ptr->get_nframes();
//   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
//   string image_URL=AnimationController_ptr->
//      get_ordered_image_filename(curr_framenumber);
   string image_URL=AnimationController_ptr->get_curr_image_filename();

   int npx,npy;
   imagefunc::get_image_width_height(image_URL,npx,npy);

   videofunc::broadcast_current_image_URL(
      viewer_messenger_ptr,n_frames,image_URL,npx,npy);
}

void MovieServer::increase_frame_rate()
{
//   cout << "inside MovieServer::increase_frame_rate()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->decrement_delay();
}

void MovieServer::decrease_frame_rate()
{
//   cout << "inside MovieServer::increase_frame_rate()" << endl;

   if (AnimationController_ptr==NULL) return;
   AnimationController_ptr->increment_delay();
}

void MovieServer::reset_movie_params()
{
//   cout << "inside MovieServer::reset_movie_params()" << endl;
   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "key = " << key
//           << " value = " << value << endl;
      if (key=="CurrFrameNumber" && value.size() > 0)
      {
         int true_frame_number=stringfunc::string_to_number(value);
         int curr_framenumber=true_frame_number-AnimationController_ptr->
            get_frame_counter_offset();

// Force actual framenumber to remain within valid interval
// [first_framenumber,last_framenumber] : 

         curr_framenumber=basic_math::max(curr_framenumber,
	         AnimationController_ptr->get_first_framenumber());
         curr_framenumber=basic_math::min(curr_framenumber,
	         AnimationController_ptr->get_last_framenumber());
         AnimationController_ptr->set_curr_framenumber(curr_framenumber);
         AnimationController_ptr->setState(AnimationController::PAUSE);
      }
      else if (key=="FrameSkip" && value.size() > 0)
      {
         int frame_skip=stringfunc::string_to_number(value);
         AnimationController_ptr->set_frame_skip(frame_skip);
         AnimationController_ptr->setState(AnimationController::PAUSE);
      }
      else if (key=="FramePercentage" && value.size() > 0)
      {
         double frame_percentage=0.01*stringfunc::string_to_number(value);
//         cout << "frame_percentage = " << frame_percentage << endl;
         int delta_nframes=(frame_percentage*
         AnimationController_ptr->get_nframes())-1;
         delta_nframes=basic_math::max(0,delta_nframes);
         delta_nframes=basic_math::min(
            AnimationController_ptr->get_nframes()-1,delta_nframes);
//         cout << "delta_nframes = " << delta_nframes << endl;
//         cout << "AC_ptr->frame_counter_offset = "
//              << AnimationController_ptr->get_frame_counter_offset() << endl;
//         cout << "AC_ptr->first_framenumber = "
//              << AnimationController_ptr->get_first_framenumber() << endl;
         AnimationController_ptr->set_curr_framenumber(
            AnimationController_ptr->get_first_framenumber()+delta_nframes);
//         cout << "curr_framenumber = "
//              << AnimationController_ptr->get_curr_framenumber() << endl;
         
         AnimationController_ptr->setState(AnimationController::JUMP_TO_FRAME);
      }
   } // loop over index k labeling KeyValue key possibilities
}

// ==========================================================================
// TOC mission member functions
// ==========================================================================

// Member function pick_mission()

void MovieServer::pick_mission()
{
   cout << "inside MovieServer::pick_mission()" << endl;
//   cout << "n_keys = " << n_keys << endl;
   cout << "TOCHUD_ptr = " << TOCHUD_ptr << endl;
   cout << "this = " << this << endl;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

//      cout << "k = " << k 
//           << " Key = " << key
//           << " Value = " << value << endl;

      if (key=="FieldtestDropdownID" || key=="FieldtestSelectorID" ||
	      key=="FieldtestPickerID" || key=="FieldtestCalibratorID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
      else if (key=="MissionDropdownID" || key=="MissionSelectorID" ||
	       key=="MissionPickerID" || key=="MissionCalibratorID")
      {
         selected_mission_ID=stringfunc::string_to_number(value);
         if (TOCHUD_ptr != NULL)
         {
            string mission_label="Mission #"+stringfunc::number_to_string(
               selected_mission_ID);
            cout << "mission_label = " << mission_label << endl;
            TOCHUD_ptr->set_string0(mission_label);
         }
      }
      else if (key=="PlatformDropdownID" || key=="PlatformSelectorID" ||
	       key=="PlatformPickerID" || key=="PlatformCalibratorID")
      {
         selected_platform_ID=stringfunc::string_to_number(value);
         cout << "selected_Platform_ID = " << selected_platform_ID 
              << endl;

         if (TOCHUD_ptr != NULL && gis_database_ptr != NULL)
         {
            vector<string> platform_label;
            vector<int> platform_ID;
            mover_func::retrieve_platform_metadata_from_database(
               gis_database_ptr,platform_label,platform_ID);
            for (int j=0; j<platform_ID.size(); j++)
            {
               if (platform_ID[j]==selected_platform_ID)
               {
                  TOCHUD_ptr->set_string1(platform_label[j]);
                  cout << "platform label = " << platform_label[j] << endl;
               }
            } // loop over index j
         }
      }
      else if (key=="SensorDropdownID" || key=="SensorSelectorID" ||
	       key=="SensorPickerID" || key=="SensorCalibratorID")
      {
         selected_sensor_ID=stringfunc::string_to_number(value);
      }
   }

   cout << "selected_fieldtest_ID = " << selected_fieldtest_ID << endl;
   cout << "selected_mission_ID = " << selected_mission_ID << endl;
   cout << "selected_platform_ID = " << selected_platform_ID << endl;
   cout << "selected_sensor_ID = " << selected_sensor_ID << endl;
}

// ---------------------------------------------------------------------
// Member function retrieve_sensor_label() takes in the ID for some
// sensor.  It performs a brute force search over all sensor labels
// for the counterpart to the input sensor ID.

string MovieServer::retrieve_sensor_label(int curr_sensor_ID)
{
//   cout << "inside MovieServer::retrieve_sensor_label()" << endl;

   if (sensor_labels.size()==0)
   {
      mover_func::retrieve_sensor_metadata_from_database(
         gis_database_ptr,sensor_labels,sensor_IDs);
   }

   string selected_sensor_label;
   for (int i=0; i<sensor_IDs.size(); i++)
   {
      if (sensor_IDs[i]==selected_sensor_ID)
      {
         selected_sensor_label=sensor_labels[i];
      }
   }

   return selected_sensor_label;
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_picked_mission()

QByteArray MovieServer::generate_JSON_response_to_picked_mission(
   bool images_retrieved_flag)
{
   cout << "inside MovieServer::generate_JSON_response_to_picked_mission()"
        << endl;

   string json_string = "{ \"FieldtestID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_fieldtest_ID)
      +"\" , \n";
   json_string += "\"MissionID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_mission_ID)
      +"\" , \n";
   json_string += "\"PlatformID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_platform_ID)
      +"\" , \n";
   json_string += "\"SensorID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_sensor_ID)
      +"\" , \n";

   string selected_fieldtest_date=mover_func::get_fieldtest_date(
      selected_fieldtest_ID,gis_database_ptr);
   json_string += " \"FieldtestDate\": ";   
   json_string += " \""+selected_fieldtest_date+"\" , \n";

   string selected_sensor_label=retrieve_sensor_label(selected_sensor_ID);
   json_string += " \"SensorLabel\": ";   
   json_string += " \""+selected_sensor_label+"\" , \n";

   json_string += "\"ImagesRetrievedFlag\": ";
   if (images_retrieved_flag)
   {
      json_string += "1  \n";
   }
   else
   {
      json_string += "0  \n";
   }
   json_string += "} \n";

   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function retrieve_movie_frames() first recovers the
// name of the subdirectory in which a sequcne of PNG or JPG images
// for a particular mission and sensor reside.  It then scans through all
// these files and determines the min and max frame numbers.  The
// starting frame number and total number of images is stored within
// *AnimationController_ptr.  Finally, this method attempts to
// correlate frame numbers and local world times based upon database
// records.

bool MovieServer::retrieve_movie_frames()
{
//   cout << "inside MovieServer::retrieve_movie_frames()" << endl;
  
   string movie_frames_subdir=photodbfunc::retrieve_photo_subdir_from_database(
      gis_database_ptr,selected_mission_ID,selected_sensor_ID);
//   cout << "movie_frames_subdir = " << movie_frames_subdir << endl;
   
   if (movie_frames_subdir.size()==0)
   {
      return false;
   }

   deduce_movie_params_from_frame_files(movie_frames_subdir);

   correlate_frame_numbers_and_world_times();
   regenerate_Movie_to_display();
   return true;
}

// ---------------------------------------------------------------------
// Member function deduce_movie_params_from_frame_files() assumes that
// a set of temporally ordered video frames reside within input
// movie_frames_subdir.  It first loads these frames into the STL map
// of *AnimationController_ptr.  This method next extracts the minimum
// and maximum image numbers for the ordered sequence.  It broadcasts
// this extremal image information.  Finally, this method resets the
// AnimationController's frame offset to equal the minimum image number.

void MovieServer::deduce_movie_params_from_frame_files(
   string movie_frames_subdir)
{
//   cout << "inside MovieServer::deduce_movie_params_from_frame_files()"
//        << endl;

   AnimationController_ptr->set_curr_framenumber(0);
   AnimationController_ptr->store_ordered_image_filenames(movie_frames_subdir);

   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
   cout << "number_of_images = " << number_of_images << endl;

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' names:

   int min_photo_number=-1;
   int max_photo_number=-1;
   videofunc::find_min_max_photo_numbers(
      movie_frames_subdir,min_photo_number,max_photo_number);

   int Nimages=max_photo_number-min_photo_number+1;
   if (number_of_images > Nimages)
   {
      Nimages=number_of_images;
      min_photo_number=0;
      max_photo_number=Nimages-1;
   }
   cout << "min_photo_number = " << min_photo_number
        << " max_photo_number = " << max_photo_number
        << " Nimages = " << Nimages << endl;

   first_frame_filename=AnimationController_ptr->
      get_next_ordered_image_filename();
   cout << "first_frame_filename = " << first_frame_filename << endl;

   videofunc::broadcast_video_params(
      viewer_messenger_ptr,min_photo_number,max_photo_number,Nimages);

   AnimationController_ptr->set_nframes(Nimages);
   AnimationController_ptr->set_frame_counter_offset(min_photo_number);
}

// ---------------------------------------------------------------------
// Member function regenerate_Movie_to_display() destroys any existing
// texture rectangle and regenerates it using the
// first_frame_filename.  It also destroys any existing movie and
// regenerates a new movie from the texture rectangle.  This method
// returns a pointer to the new Movie object.

Movie* MovieServer::regenerate_Movie_to_display()
{
   cout << "inside MovieServer::regenerate_Movie_to_display()" << endl;

// Check if MoviesGroup_ptr has been set yet.  If not, return...

   if (MoviesGroup_ptr==NULL) return NULL;
   
   delete texture_rectangle_ptr;

   cout << "first_frame_filename = " << first_frame_filename << endl;
   texture_rectangle_ptr=
      MoviesGroup_ptr->generate_new_texture_rectangle(first_frame_filename);

   int min_photo_number=0;
//   int min_photo_number=AnimationController_ptr->get_frame_counter_offset();
   int max_photo_number=
      min_photo_number+AnimationController_ptr->get_nframes()-1;
//   cout << " min_photo = " << min_photo_number
//        << " max_photo = " << max_photo_number << endl;
   
   texture_rectangle_ptr->set_first_frame_to_display(min_photo_number);
   texture_rectangle_ptr->set_last_frame_to_display(max_photo_number);

   MoviesGroup_ptr->destroy_all_Movies();
   Movie* movie_ptr=MoviesGroup_ptr->
      generate_new_Movie(texture_rectangle_ptr);

   cout << "at end of MovieServer::regnerate_Movie_to_display()" << endl;

   return movie_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_frames_query()

QByteArray MovieServer::generate_JSON_response_to_frames_query()
{
//   cout << "inside MovieServer::generate_JSON_response_to_frames_query()"
//        << endl;

   int n_frames=AnimationController_ptr->get_nframes();
   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
//   string image_filename=AnimationController_ptr->
//      get_ordered_image_filename(curr_framenumber);

   string json_string = "{ \"Nframes\": ";
   json_string += stringfunc::number_to_string(n_frames)+", \n";
   json_string += "\"CurrFrameNumber\": ";
   json_string += stringfunc::number_to_string(curr_framenumber)+", \n";
   json_string += "\"TrueFrameNumber\": ";
   json_string += stringfunc::number_to_string(AnimationController_ptr->
   get_true_framenumber())+", \n";
   
//   json_string += " \"CurrImageFilename\": ";
//   json_string += " \""+image_filename+"\" \n";
//   json_string += "} \n";

//   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Video frame/time calibration member functions
// ==========================================================================

// Member function calibrate_frames_to_local_world_time() takes in a
// starting and stopping set of frame numbers as well as local time
// stamps.  It linearly interpolates/extrapolates these user
// specified data to assign a timestamp to all frames within the
// current video clip.  This method updates the timestamp column
// within the photos table of the TOC database.

int MovieServer::calibrate_frames_to_local_world_time()
{
   cout << "inside MovieServer::calibrate_frames_to_local_world_time()" 
	<< endl;
   int n_args=KeyValue.size();

   string progress_type="calibrate_frames";
   viewer_messenger_ptr->broadcast_clear_progress(progress_type);

   int true_start_framenumber,true_stop_framenumber;
   string startDate,stopDate,startTime,stopTime;
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
      cout << "key = " << key
           << " value = " << value << endl;

      if (key=="StartFrameNumber" && value.size() > 0)
      {
         true_start_framenumber=stringfunc::string_to_number(value);
      }
      else if (key=="StopFrameNumber" && value.size() > 0)
      {
         true_stop_framenumber=stringfunc::string_to_number(value);
      }
      else if (key=="StartDate" && value.size() > 0)
      {
         startDate=value;
      }
      else if (key=="StopDate" && value.size() > 0)
      {
         stopDate=value;
      }
      else if (key=="StartTime" && value.size() > 0)
      {
         startTime=value;
      }
      else if (key=="StopTime" && value.size() > 0)
      {
         stopTime=value;
      }
   } // loop over index k labeling KeyValue key possibilities

   double dataupload_progress=0.05;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   cout << "true_start_framenumber = " << true_start_framenumber
        << " true_stop_framenumber = " << true_stop_framenumber << endl;
   cout << "startDate = " << startDate 
        << " stopDate = " << stopDate << endl;
   cout << "startTime = " << startTime
        << " stopTime = " << stopTime << endl;

   vector<string> startDate_substrings=
      stringfunc::decompose_string_into_substrings(startDate,"-");
   vector<string> stopDate_substrings=
      stringfunc::decompose_string_into_substrings(stopDate,"-");
   vector<string> startTime_substrings=
      stringfunc::decompose_string_into_substrings(startTime,":");
   vector<string> stopTime_substrings=
      stringfunc::decompose_string_into_substrings(stopTime,":");
   int year_start=stringfunc::string_to_number(startDate_substrings[0]);
   int year_stop=stringfunc::string_to_number(stopDate_substrings[0]);
   int month_start=stringfunc::string_to_number(startDate_substrings[1]);
   int month_stop=stringfunc::string_to_number(stopDate_substrings[1]);
   int day_start=stringfunc::string_to_number(startDate_substrings[2]);
   int day_stop=stringfunc::string_to_number(stopDate_substrings[2]);

   cout << "year_start = " << year_start << " month_start = " << month_start
        << " day_start = " << day_start << endl;
   cout << "year_stop = " << year_stop << " month_stop = " << month_stop
        << " day_stop = " << day_stop << endl;

   int hour_start=stringfunc::string_to_number(startTime_substrings[0]);
   int hour_stop=stringfunc::string_to_number(stopTime_substrings[0]);
   int min_start=stringfunc::string_to_number(startTime_substrings[1]);
   int min_stop=stringfunc::string_to_number(stopTime_substrings[1]);

// User may not necessarily have entered any secs entry.  If not, set
// secs to default zero value:

   double sec_start=0;
   double sec_stop=0;
   if (startTime_substrings.size() > 2)
   {
      sec_start=stringfunc::string_to_number(startTime_substrings[2]);
   }

   if (stopTime_substrings.size() > 2)
   {
      sec_stop=stringfunc::string_to_number(stopTime_substrings[2]);
   }

   dataupload_progress=0.25;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);
   
   cout << "hour_start = "<< hour_start << " min_start = " << min_start
        << " sec_start = " << sec_start << endl << endl;
   cout << "hour_stop = "<< hour_stop << " min_stop = " << min_stop
        << " sec_stop = " << sec_stop << endl;

   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   clock_ptr->set_local_time(
      year_start,month_start,day_start,hour_start,min_start,sec_start);
   double start_secs_since_ref_date=clock_ptr->
      secs_elapsed_since_reference_date();
   clock_ptr->set_local_time(
      year_stop,month_stop,day_stop,hour_stop,min_stop,sec_stop);
   double stop_secs_since_ref_date=clock_ptr->
      secs_elapsed_since_reference_date();
   
   cout.precision(13);
   cout << "start_secs_since_ref_date = " << start_secs_since_ref_date << endl;
   cout << "stop_secs_since_ref_date = " << stop_secs_since_ref_date << endl;
   cout << "true_start_framenumber = " << true_start_framenumber
        << " true_stop_framenumber = " << true_stop_framenumber << endl;

   double timeoverframes_ratio=
      (stop_secs_since_ref_date-start_secs_since_ref_date)/
      (true_stop_framenumber-true_start_framenumber);
   cout << "timeoverframes_ratio = " << timeoverframes_ratio << endl;

   cout << "Selected_mission_ID = " << selected_mission_ID
        << " selected_sensor_ID = " << selected_sensor_ID << endl;

   dataupload_progress=0.5;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   vector<int> photo_IDs,photo_framenumbers;
   vector<string> photo_filenames;
   photodbfunc::retrieve_photo_metadata_from_database(
      gis_database_ptr,selected_mission_ID,selected_sensor_ID,
      photo_IDs,photo_filenames,photo_framenumbers);
   
   cout << "photo_IDs.size() = " << photo_IDs.size()
        << " photo_framenumbers.size() = " << photo_framenumbers.size()
        << " photo_filenames.size() = " << photo_filenames.size() << endl;

   vector<string> photo_timestamps;
   for (int i=0; i<photo_IDs.size(); i++)
   {
      int curr_photo_ID=photo_IDs[i];
      int curr_framenumber=photo_framenumbers[i];
      double curr_secs_since_ref_date=start_secs_since_ref_date+
         timeoverframes_ratio*(curr_framenumber-true_start_framenumber);
      clock_ptr->convert_elapsed_secs_to_date(curr_secs_since_ref_date);

      string day_hour_separator_char=" ";
      string time_separator_char=":";
      bool display_UTC_flag=true;
      int n_secs_digits=2;
      photo_timestamps.push_back(clock_ptr->
	      YYYY_MM_DD_H_M_S(day_hour_separator_char,
	      time_separator_char,display_UTC_flag,n_secs_digits));

      cout << "photo ID = " << curr_photo_ID
           << " framenumber = " << curr_framenumber
           << " timestamp = " << photo_timestamps.back() << endl;
   }

   dataupload_progress=0.75;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);
   
   photodbfunc::update_photo_timestamps_in_database(
      gis_database_ptr,selected_mission_ID,selected_sensor_ID,
      photo_IDs,photo_timestamps);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   return photo_timestamps.size();
}

// ---------------------------------------------------------------------
// Member function extract_frame_geometries()

int MovieServer::extract_frame_geometries()
{
   cout << "inside MovieServer::extract_frame_geometries()" << endl;
   int n_args=KeyValue.size();

   string progress_type="frame_geometries";
   viewer_messenger_ptr->broadcast_clear_progress(progress_type);

   double dataupload_progress=0.05;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   cout << "selected_fieldtest_ID = " 
        << selected_fieldtest_ID << endl;

   tracks_group* tracks_group_ptr=
      mover_func::retrieve_all_tracks_in_TOC_database(
         gis_database_ptr,selected_fieldtest_ID);

   dataupload_progress=0.50;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   cout << "Selected_mission_ID = "
        << selected_mission_ID << endl;

   int n_photos_calibrated=photodbfunc::fuse_photo_and_gps_metadata(
      gis_database_ptr,selected_mission_ID,tracks_group_ptr);

   dataupload_progress=0.90;
   viewer_messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

   return n_photos_calibrated;
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_frame_calibration()

string MovieServer::generate_JSON_response_to_frame_calibration(
   int n_calibrated_photos)
{
   cout << "inside MovieServer::generate_JSON_response_to_frame_calibration()"
        << endl;

   string json_string = "{ \"FieldtestID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_fieldtest_ID)
      +"\" , \n";
   json_string += "\"MissionID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_mission_ID)
      +"\" , \n";
   json_string += "\"PlatformID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_platform_ID)
      +"\" , \n";
   json_string += "\"SensorID\": ";
   json_string += "\""+stringfunc::number_to_string(selected_sensor_ID)
      +"\" , \n";
   json_string += "\"Nphotos\": ";
   json_string += stringfunc::number_to_string(n_calibrated_photos)+"} \n";

   cout << "Final json_string = " << json_string << endl;
   return json_string;
}

// ---------------------------------------------------------------------
void MovieServer::correlate_frame_numbers_and_world_times()
{
//   cout << "inside MovieServer::correlate_frame_numbers_and_world_times()"
//        << endl;

// Don't attempt to read photo metadata from TOC database if mission
// or sensor have not yet been selected!
   
   if (selected_mission_ID==-1 || selected_sensor_ID==-1) return;

   vector<int> photo_IDs,photo_framenumbers;
   vector<string> photo_filenames,photo_timestamps;

   photodbfunc::retrieve_photo_metadata_from_database(
      gis_database_ptr,selected_mission_ID,selected_sensor_ID,
      photo_IDs,photo_filenames,photo_framenumbers,photo_timestamps);

// If no image timestamps have been previously stored within photos
// table of TOC database, don't attempt to perform any frame_number to
// world_time correlation:

   if (photo_timestamps.size()==0) return;

   int n_photos=photo_IDs.size();
//   cout << "n_photos = " << n_photos << endl;
   
   if (photo_timestamps[0].size()==0 ||
   photo_timestamps[n_photos-1].size()==0) return;
   
   for (int i=0; i<n_photos; i++)
   {
      if (i < 2 || i > n_photos-3)
      {
//         cout << "i = " << i << " ID = " << photo_IDs[i]
//              << " filename = " << filefunc::getbasename(photo_filenames[i])
//              << " frame# = " << photo_framenumbers[i]
//              << " time = " << photo_timestamps[i] << endl;
      }
   }

   int start_framenumber=photo_framenumbers[0];
   int stop_framenumber=photo_framenumbers[n_photos-1];

//   cout << "start_framenumber = " << start_framenumber << endl;
//   cout << "stop_framenumber = " << stop_framenumber << endl;

//   cout << "photo_timestamps[0] = " << photo_timestamps[0] << endl;
//   cout << "photo_timestamps[n_photos-1] = " 
//        << photo_timestamps[n_photos-1] << endl;

   bool UTC_flag=true;
   Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();
   double start_elapsed_secs=clock_ptr->timestamp_string_to_elapsed_secs(
      photo_timestamps[0],UTC_flag);
   double stop_elapsed_secs=clock_ptr->timestamp_string_to_elapsed_secs(
      photo_timestamps[n_photos-1],UTC_flag);

//   cout.precision(12);
//   cout << "start_elapsed_secs = " << start_elapsed_secs
//        << " stop_elapsed_secs = " << stop_elapsed_secs << endl;

   AnimationController_ptr->specify_extremal_frame_world_times(
      AnimationController_ptr->get_first_framenumber(),
      AnimationController_ptr->get_last_framenumber(),
      start_elapsed_secs,stop_elapsed_secs);

   Operations_ptr->get_ImageNumberHUD_ptr()->
      set_display_movie_world_time_flag(true);
}

// ==========================================================================
// Video frame triage member functions
// ==========================================================================

// Member function flag_frames() parses starting and stopping video
// frame numbers entered by a user along with an importance value.  It
// updates the corresponding entries within the photos table of the
// TOC database with the input importance information.

QByteArray MovieServer::flag_frames()
{
   cout << "inside MovieServer::flag_frames()" << endl;

   int starting_flagged_frame,stopping_flagged_frame,flagged_frame_importance;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

//      cout << "k = " << k 
//           << " Key = " << key
 //          << " Value = " << value << endl;

      if (key=="annoStartFrame")
      {
         starting_flagged_frame=stringfunc::string_to_number(value);
      }
      else if (key=="annoEndFrame")
      {
         stopping_flagged_frame=stringfunc::string_to_number(value);
      }
      else if (key=="annoImportance")
      {
         flagged_frame_importance=stringfunc::string_to_number(value);
      }

   } // loop over index k labeling keys

   cout << "starting_flagged_frame = " << starting_flagged_frame << endl;
   cout << "stopping_flagged_frame = " << stopping_flagged_frame << endl;
   cout << "flagged_frame_importance = " << flagged_frame_importance << endl;

   for (int f=starting_flagged_frame; f<= stopping_flagged_frame; f++)
   {
      string curr_image_filename=AnimationController_ptr->
         get_ordered_image_filename(f);
//      cout << "f = " << f 
//           << " curr_image_filename = " << curr_image_filename << endl;
      photodbfunc::update_photo_importance_in_database(
         gis_database_ptr,curr_image_filename,flagged_frame_importance);
   }

   return generate_JSON_response_to_flagged_frames(
      starting_flagged_frame,stopping_flagged_frame,flagged_frame_importance);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_flagged_frames()

QByteArray MovieServer::generate_JSON_response_to_flagged_frames(
   int starting_flagged_frame,int stopping_flagged_frame,
   int flagged_frame_importance)
{
//   cout << "inside MovieServer::generate_JSON_response_to_flagged_frames()"
//        << endl;

   string message="Importance set to ";
   if (flagged_frame_importance==1)
   {
      message += "LOW ";
   }
   else if (flagged_frame_importance==2)
   {
      message += "MEDIUM ";
   }
   if (flagged_frame_importance==3)
   {
      message += "HIGH ";
   }
   message += " for frames "+stringfunc::number_to_string(
      starting_flagged_frame)+" thru "+stringfunc::number_to_string(
         stopping_flagged_frame);
   
   string json_string = "{ \"message\": \" ";
   json_string += message +" \" \n";
   json_string += "} \n";

/*
   string json_string = "{ \"Starting_flagged_frame\": ";
   json_string += stringfunc::number_to_string(starting_flagged_frame)
      +", \n";
   json_string += "\"Stopping_flagged_frame\": ";
   json_string += stringfunc::number_to_string(stopping_flagged_frame)
      +", \n";
   json_string += "\"Importance\": ";
   if (flagged_frame_importance==1)
   {
      json_string += "'LO'";
   }
   else if (flagged_frame_importance==2)
   {
      json_string += "'MED'";
   }
   if (flagged_frame_importance==1)
   {
      json_string += "'HI'";
   }
   json_string += " \n";
   json_string += "} \n";
*/

//   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_importance_intervals()
// generates and returns a JSON string containing startFrame,
// stopFrame and importance triples which cover all frames within the
// current video clip.  This triple information will be displayed as
// a SIMILE timeline by Diane Staheli.

QByteArray MovieServer::generate_JSON_response_to_importance_intervals()
{
//   cout << "inside MovieServer::generate_JSON_response_to_importance_intervals()"
//        << endl;

//   cout << "selected_mission_ID = " << selected_mission_ID << endl;

   string json_string = "[ \n";   
   json_string += generate_JSON_for_single_mission_importance_intervals(
      selected_mission_ID);
   json_string += "] \n";
   
//   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_for_single_mission_importance_intervals()
// generates and returns a JSON string containing startFrame,
// stopFrame and importance triples which cover all frames within the
// current video clip.  This triple information will be displayed as
// a SIMILE timeline by Diane Staheli.

string MovieServer::generate_JSON_for_single_mission_importance_intervals(
   int mission_ID,int n_indent_spaces)
{
//   cout << "inside MovieServer::generate_JSON_for_single_mission_importance_intervals()"
//        << endl;
//   cout << "mission_ID = " << mission_ID << endl;

   vector<fourvector> photo_importance_intervals=
      photodbfunc::compute_photo_importance_intervals(
         gis_database_ptr,mission_ID);

   string white_space;
   for (int n=0; n<n_indent_spaces; n++)
   {
      white_space += " ";
   }

   string json_string;   
//   cout << "photo_importance_intervals.size() = "
//        << photo_importance_intervals.size() << endl;
   for (int i=0; i<photo_importance_intervals.size(); i++)
   {
      fourvector ID_start_stop_importance=photo_importance_intervals[i];
      int starting_photo_ID=ID_start_stop_importance.get(0);
      double start_frame=ID_start_stop_importance.get(1);
      double stop_frame=ID_start_stop_importance.get(2);
      int stopping_photo_ID=starting_photo_ID+stop_frame-start_frame;
      int importance=ID_start_stop_importance.get(3);

//      cout << "i = " << i << " importance = " << importance << endl;

      int photo_counter,imp;
      string caption;
      string starting_photo_timestamp,stopping_photo_timestamp;
      string starting_photo_filename,stopping_photo_filename;

      photodbfunc::retrieve_particular_photo_metadata_from_database(
         gis_database_ptr,starting_photo_ID,caption,starting_photo_timestamp,
         starting_photo_filename,photo_counter,imp);
      photodbfunc::retrieve_particular_photo_metadata_from_database(
         gis_database_ptr,stopping_photo_ID,caption,stopping_photo_timestamp,
         stopping_photo_filename,photo_counter,imp);

      Clock* clock_ptr=AnimationController_ptr->get_clock_ptr();      
   
      bool UTC_flag=false;
      double start_time=-1;
      double stop_time=-1;
      if (starting_photo_timestamp.size() > 0 &&
          starting_photo_timestamp != "NULL")
      {
//         cout << "start_timestamp = " << starting_photo_timestamp << endl;
         start_time=clock_ptr->timestamp_string_to_elapsed_secs(
            starting_photo_timestamp,UTC_flag);

      }
      if (stopping_photo_timestamp.size() > 0 &&
          stopping_photo_timestamp != "NULL")
      {
//         cout << "stop_timestamp = " << stopping_photo_timestamp << endl;
         stop_time=clock_ptr->timestamp_string_to_elapsed_secs(
            stopping_photo_timestamp,UTC_flag);
      }

// If valid timestamps for the starting and stopping frames enclosing
// the current importance interval do not exist within the photos
// table of the TOC database, we arbitrarily assign these times as 2pm
// and 3pm on the field test date:

      if (start_time < 0 || stop_time < 0 || stop_time < start_time)
      {
         double fieldtest_date=databasefunc::get_fieldtest_time(
            gis_database_ptr,selected_fieldtest_ID);
//         cout << "fieldtest_date = " << fieldtest_date << endl;
         start_time=fieldtest_date+14*3600;
         stop_time=fieldtest_date+15*3600;
         clock_ptr->convert_elapsed_secs_to_date(start_time);
//         cout << "Fake start timestamp = "
//              << clock_ptr->YYYY_MM_DD_H_M_S() << endl;
         clock_ptr->convert_elapsed_secs_to_date(stop_time);
//         cout << "Fake stop timestamp = "
//              << clock_ptr->YYYY_MM_DD_H_M_S() << endl;
      }

      json_string += white_space+"   { \n";
      json_string += white_space+"      'startFrame': '"+
         stringfunc::number_to_string(start_frame)+"', \n";
      json_string += white_space+"      'startTime': '"+
         stringfunc::number_to_string(start_time)+"', \n";
      json_string += white_space+"      'stopFrame': '"+
         stringfunc::number_to_string(stop_frame)+"', \n";
      json_string += white_space+"      'stopTime': '"+
         stringfunc::number_to_string(stop_time)+"', \n";
      json_string += white_space+"      'importance': '"+
         stringfunc::number_to_string(importance)+"' \n";
      json_string += white_space+"   }";
      if (i < photo_importance_intervals.size()-1) json_string += ",";
      json_string += "\n";
   }  // loop over index i labeling importance intervals
   
   return json_string;
}

// ---------------------------------------------------------------------
// Member function generate_JSON_for_multimission_importance_intervals()
// generates and returns a JSON string containing startFrame,
// stopFrame and importance triples which cover all frames within all
// missions for the currently selected fieldtest.  This triple
// information will be displayed as a SIMILE timeline by Diane Staheli.

QByteArray MovieServer::generate_JSON_for_multimission_importance_intervals()
{
   cout << "inside MovieServer::generate_JSON_for_multimission_importance_intervals()"
        << endl;
   cout << "selected_fieldtest_ID = " << selected_fieldtest_ID << endl;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

//      cout << "k = " << k 
//           << " Key = " << key
 //          << " Value = " << value << endl;

      if (key=="FieldtestID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
   }

   vector<int> fieldtest_ID,mission_ID,platform_ID;
   vector<string> fieldtest_label,mission_label,platform_label;
   mover_func::retrieve_fieldtest_mission_platform_metadata_from_database(
      gis_database_ptr,fieldtest_label,fieldtest_ID,
      mission_label,mission_ID,platform_label,platform_ID);

// Need to count number of field tests which match
// selected_fieldtest_ID for final JSON comma placing purposes:

   int n_matching_fieldtests=0;
   for (int i=0; i<fieldtest_ID.size(); i++)
   {
      if (fieldtest_ID[i] == selected_fieldtest_ID) n_matching_fieldtests++;
   }
   cout << "n_matching_fieldtests = " << n_matching_fieldtests << endl;
      
   int fieldtest_counter=0;
   string json_string = "{ \n";
   json_string += "  'missions': [ \n";
   for (int i=0; i<fieldtest_ID.size(); i++)
   {
      if (fieldtest_ID[i] != selected_fieldtest_ID) continue;

      json_string += "    {\n";
      json_string += "       'fieldtest': '"+fieldtest_label[i]+"', \n";
      json_string += "       'mission': '"+mission_label[i]+"', \n";
      json_string += "       'platform': '"+platform_label[i]+"', \n";

      json_string += "       'annotations': [ \n";
      int n_indent_spaces=11;
      json_string += generate_JSON_for_single_mission_importance_intervals(
         mission_ID[i],n_indent_spaces);
      json_string += "       ]\n";
      json_string += "    }";
      if (fieldtest_counter < n_matching_fieldtests-1)
      {
         json_string += ",";
      }
      json_string += "\n";

      fieldtest_counter++;
   } // loop over index i labeling correlated fieldtest, mission, platform 
     //	labels

   json_string += "  ] \n";
   json_string += "} \n";
   
   cout << "Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function annotate_current_frame()

void MovieServer::annotate_current_frame()
{
   cout << "inside MovieServer::annotate_current_frame()" << endl;

   string curr_image_filename=AnimationController_ptr->
      get_next_ordered_image_filename();
   cout << " curr_image_filename = " << curr_image_filename << endl;

   int image_ID=photodbfunc::get_photo_ID(
      gis_database_ptr,curr_image_filename);
   cout << "image_ID = " << image_ID << endl;
   twovector photo_dims=photodbfunc::get_photo_dims(gis_database_ptr,image_ID);
   int npx=photo_dims.get(0);
   int npy=photo_dims.get(1);

   videofunc::broadcast_selected_image(
      viewer_messenger_ptr,image_ID,curr_image_filename,npx,npy);
}

// ==========================================================================
// Feature manipulation member functions
// ==========================================================================

QByteArray MovieServer::insert_image_feature()
{
   cout << "inside Movie::insert_image_feature()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_FEATURE);

   FeaturePickHandler_ptr->set_enable_pick_flag(true);

   while (!FeaturesGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in MovieServer" << endl;
      WindowManager_ptr->process();
   }
   FeaturesGroup_ptr->set_Geometricals_updated_flag(false);

   ModeController_ptr->setState(ModeController::VIEW_DATA);
   FeaturePickHandler_ptr->set_enable_pick_flag(false);

   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray MovieServer::update_image_feature()
{
   cout << "inside MovieServer::update_image_feature()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_FEATURE);

   int existing_feature_ID,new_feature_ID,n_feature_coords=0;
   double X,Y;
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
   if (n_feature_coords < 2)
   {
   }
   else
   {
      threevector posn(X,Y);   
      Feature_ptr->set_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),
         FeaturesGroup_ptr->get_passnumber(),posn);
   }

   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
void MovieServer::drag_image_feature()
{
   cout << "inside MovieServer::drag_image_feature()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_FEATURE);
   FeaturePickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray MovieServer::stop_drag_image_feature()
{
   cout << "inside MovieServer::stop_drag_image_feature()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   FeaturePickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
int MovieServer::set_selected_Feature_ID() const
{
   cout << "inside MovieServer::set_selected_Feature_ID()" << endl;

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
QByteArray MovieServer::delete_image_feature()
{
//   cout << "inside MovieServer::delete_image_feature()" << endl;

   set_selected_Feature_ID();
   FeaturesGroup_ptr->destroy_feature();
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
QByteArray MovieServer::purge_image_features()
{
   cout << "inside MovieServer::purge_image_feature()" << endl;

   FeaturesGroup_ptr->destroy_all_Features();
   return generate_JSON_response_to_feature_event();
}

// ---------------------------------------------------------------------
void MovieServer::decrease_feature_size()
{
   cout << "inside MovieServer::decrease_feature_size()" << endl;
   FeaturesGroup_ptr->change_size(0.5);
}

void MovieServer::increase_feature_size()
{
   cout << "inside MovieServer::increase_feature_size()" << endl;
   FeaturesGroup_ptr->change_size(2.0);
}

// ---------------------------------------------------------------------
QByteArray MovieServer::export_features()
{
   cout << "inside MovieServer::export_features()" << endl;

   string image_filename=AnimationController_ptr->get_curr_image_filename();
   string basename=filefunc::getbasename(image_filename);
   string prefix=stringfunc::prefix(basename);
   string output_filename="features_2D_"+prefix+".txt";
   cout << "output_filename = " << output_filename << endl;

   FeaturesGroup_ptr->save_feature_info_to_file(output_filename);
   cout << "Feature information written to "+output_filename << endl;
   return generate_JSON_response_to_geometrical_export(output_filename);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_geometrical_export()

QByteArray MovieServer::generate_JSON_response_to_geometrical_export(
   string output_filename)
{
   cout << "Inside MovieServer::generate_JSON_response_to_geometrical_export()"
        << endl;
   
   string json_string = "{  \n";
   json_string += " \"Export_filename\": \""+output_filename+"\"";
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
QByteArray MovieServer::import_features()
{
   cout << "inside MovieServer::import_features()" << endl;

   string image_filename=AnimationController_ptr->get_curr_image_filename();
   string basename=filefunc::getbasename(image_filename);
   string prefix=stringfunc::prefix(basename);
   string input_filename="features_2D_"+prefix+".txt";
   cout << "input_filename = " << input_filename << endl;

   if (!filefunc::fileexist(input_filename))
   {
      bool successful_import_flag=false;
      return generate_JSON_response_to_feature_event(
         input_filename,successful_import_flag);
   }
   else
   {
      FeaturesGroup_ptr->read_feature_info_from_file(input_filename);
      return generate_JSON_response_to_feature_event(input_filename);
   }
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_feature_event()

QByteArray MovieServer::generate_JSON_response_to_feature_event()
{
   string input_filename="";
   return generate_JSON_response_to_feature_event(input_filename);
}

QByteArray MovieServer::generate_JSON_response_to_feature_event(
   string input_filename,bool successful_import_flag)
{
   cout << "Inside MovieServer::generate_JSON_response_to_feature_event()"
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
         +stringfunc::number_to_string(Feature_posn.get(1))+"]";
      if (n < n_Features-1) json_string += ",";
   } // loop over index n labeling Features
   
   json_string += " ], \n ";
   json_string += " \"Import_filename\": \""+input_filename+"\", \n";

   cout << "successful_import_flag = " << successful_import_flag << endl;
   json_string += " \"Import_success\": "+stringfunc::number_to_string(
      successful_import_flag)+", \n";

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

QByteArray MovieServer::insert_image_polyline()
{
//   cout << "inside MovieServer::insert_image_polyline()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::INSERT_POLYLINE);

   while (!PolyLinesGroup_ptr->get_Geometricals_updated_flag())
   {
//      cout << "Waiting for signposts to be updated in MovieServer" << endl;
      WindowManager_ptr->process();
   }
   PolyLinesGroup_ptr->set_Geometricals_updated_flag(false);
   PolyLinesGroup_ptr->set_selected_Graphical_ID(
      PolyLinesGroup_ptr->get_most_recently_added_ID());

// Label PolyLine with its ID or length:

   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->
      get_ID_labeled_PolyLine_ptr(
         PolyLinesGroup_ptr->get_most_recently_added_ID());

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
QByteArray MovieServer::update_image_polyline()
{
   cout << "inside MovieServer::update_image_polyline()" << endl;
//   cout << "ModeController_ptr = " << ModeController_ptr << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);

   int existing_PolyLine_ID,new_PolyLine_ID;
   double X,Y;
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
      PolyLine_ptr->generate_PolyLine_ID_label();
   }
 
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
int MovieServer::set_selected_PolyLine_ID() const
{
   cout << "inside MovieServer::set_selected_PolyLine_ID()" << endl;

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
void MovieServer::drag_image_polyline()
{
//   cout << "inside MovieServer::drag_image_polyline()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);
   PolyLinePickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray MovieServer::stop_drag_image_polyline()
{
//   cout << "inside MovieServer::stop_drag_image_polyline()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   PolyLinePickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   return regenerate_image_PolyLine();
}

// ---------------------------------------------------------------------
void MovieServer::scale_image_polyline()
{
   cout << "inside MovieServer::scale_image_polyline()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);
   PolyLinePickHandler_ptr->set_enable_drag_flag(true);
   PolyLinePickHandler_ptr->set_scaling_mode(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray MovieServer::stop_scale_image_polyline()
{
   cout << "inside MovieServer::stop_scale_image_polyline()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   PolyLinePickHandler_ptr->set_enable_drag_flag(false);
   PolyLinePickHandler_ptr->set_scaling_mode(false);
   CM_ptr->set_enable_drag_flag(false);
   return regenerate_image_PolyLine();
}

// ---------------------------------------------------------------------
void MovieServer::rotate_image_polyline()
{
   cout << "inside MovieServer::rotate_image_polyline()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);
   PolyLinePickHandler_ptr->set_enable_drag_flag(true);
   PolyLinePickHandler_ptr->set_rotation_mode(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray MovieServer::stop_rotate_image_polyline()
{
   cout << "inside MovieServer::stop_rotate_image_polyline()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   PolyLinePickHandler_ptr->set_enable_drag_flag(false);
   PolyLinePickHandler_ptr->set_rotation_mode(false);
   CM_ptr->set_enable_drag_flag(false);
   return generate_JSON_response_to_polyline_event();
//   return regenerate_image_PolyLine();
}

// ---------------------------------------------------------------------
// Member function regenerate_image_PolyLine() regenerates a 2D
// PolyLine where each vertex is translated by the difference between
// the PolyLine's current position and its zeroth vertex position.

QByteArray MovieServer::regenerate_image_PolyLine()
{
   cout << "inside MovieServer::regenerate_image_polyline()" << endl;

   int selected_PolyLine_ID=PolyLinesGroup_ptr->get_selected_Graphical_ID();
   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(
      selected_PolyLine_ID);
   if (PolyLine_ptr != NULL)
   {
      threevector PolyLine_posn;
      PolyLine_ptr->get_UVW_coords(
         PolyLinesGroup_ptr->get_curr_t(),
         PolyLinesGroup_ptr->get_passnumber(),PolyLine_posn);

      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      cout << "*polyline_ptr = " << *polyline_ptr << endl;

      vector<threevector> vertices;
      int n_vertices=polyline_ptr->get_n_vertices();
//   cout << "n_vertices = " << n_vertices << endl;
      for (int v=0; v<n_vertices; v++)
      {
         vertices.push_back(PolyLine_posn+
            polyline_ptr->get_vertex(v)-polyline_ptr->get_vertex(0));
      }

      PolyLine_ptr=PolyLinesGroup_ptr->regenerate_PolyLine(
         vertices,PolyLine_ptr,PolyLine_ptr->get_permanent_color(),
         PolyLine_ptr->get_selected_color());
      PolyLinesGroup_ptr->set_selected_Graphical_ID(selected_PolyLine_ID);
   }

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray MovieServer::double_image_polyline_vertices()
{
//   cout << "inside MovieServer::double_image_polyline_vertices()" << endl;
   int selected_PolyLine_ID=set_selected_PolyLine_ID();
   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_ID_labeled_PolyLine_ptr(
      selected_PolyLine_ID);
   if (PolyLine_ptr != NULL)
   {
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
//      cout << "*polyline_ptr = " << *polyline_ptr << endl;
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
//      PolyLine_ptr->generate_PolyLine_length_label();

   } // PolyLine_ptr != NULL conditional

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray MovieServer::delete_image_polyline()
{
//   cout << "inside MovieServer::delete_image_polyline()" << endl;

   set_selected_PolyLine_ID();
   PolyLinesGroup_ptr->destroy_PolyLine();
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
QByteArray MovieServer::purge_image_polylines()
{
//   cout << "inside MovieServer::purge_image_polylines()" << endl;

   PolyLinesGroup_ptr->destroy_all_PolyLines();
   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
void MovieServer::decrease_polyline_size()
{
//   cout << "inside MovieServer::decrease_polyline_size()" << endl;
   PolyLinesGroup_ptr->change_size(0.5);
}

void MovieServer::increase_polyline_size()
{
//   cout << "inside MovieServer::increase_polyline_size()" << endl;
   PolyLinesGroup_ptr->change_size(2.0);
}

// ---------------------------------------------------------------------
QByteArray MovieServer::export_polylines()
{
   cout << "inside MovieServer::export_polylines()" << endl;
   string output_filename=PolyLinesGroup_ptr->save_info_to_file();
   cout << "Polyline information written to "+output_filename << endl;
   return generate_JSON_response_to_geometrical_export(output_filename);
}

// ---------------------------------------------------------------------
QByteArray MovieServer::import_polylines()
{
   cout << "inside MovieServer::import_polylines()" << endl;
   string input_filename=
      PolyLinesGroup_ptr->reconstruct_polylines_from_file_info();
   cout << "Polyline info imported from " << input_filename << endl;
   return generate_JSON_response_to_polyline_event(input_filename);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_polyline_event() returns
// a JSON string containing all PolyLine IDs, the selected PolyLine's ID

QByteArray MovieServer::generate_JSON_response_to_polyline_event()
{
   string input_filename="";
   return generate_JSON_response_to_polyline_event(input_filename);
}

QByteArray MovieServer::generate_JSON_response_to_polyline_event(
   string input_filename)
{
   cout << "Inside MovieServer::generate_JSON_response_to_polyline_event()"
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
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
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
            +stringfunc::number_to_string(curr_vertex.get(1))+"]";
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

osgGeometry::PointsGroup* MovieServer::get_selected_PolyLine_vertices() const
{
//   cout << "inside MovieServer::get_selected_PolyLine_vertices()" << endl;

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
void MovieServer::get_selected_PolyLine_and_vertex_IDs(
   int& selected_PolyLine_ID,int& selected_vertex_ID) const
{
//   cout << "inside MovieServer::get_selected_PolyLine_and_vertex_IDs()" 
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
void MovieServer::unselect_all_PolyLine_vertices() const
{
//   cout << "inside MovieServer::unselect_all_PolyLine_vertices()" << endl;

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
int MovieServer::set_selected_Vertex_ID() const
{
//   cout << "inside MovieServer::set_selected_Vertex_ID()" << endl;

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
int MovieServer::unselect_image_vertex() const
{
//   cout << "inside MovieServer::unselect_image_vertex()" << endl;

   osgGeometry::PointsGroup* PointsGroup_ptr=get_selected_PolyLine_vertices();
   PointsGroup_ptr->set_selected_Graphical_ID(-1);
}

// ---------------------------------------------------------------------
QByteArray MovieServer::update_image_vertex()
{
//   cout << "inside MovieServer::update_image_vertex()" << endl;
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
   }
   
//   cout << "Selected PolyLine_ID = " << selected_PolyLine_ID << endl;
//   cout << "Selected Vertex_ID = " << selected_vertex_ID << endl;
   osgGeometry::PointsGroup* PointsGroup_ptr=get_selected_PolyLine_vertices();
   
   osgGeometry::Point* Point_ptr=PointsGroup_ptr->get_ID_labeled_Point_ptr(
      selected_vertex_ID);

   if (n_vertex_coords==2)
   {
      threevector new_vertex_posn(X,Y);   
      regenerate_PolyLine(
         selected_PolyLine_ID,selected_vertex_ID,new_vertex_posn);
   }

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
// Member function regenerate_PolyLine() takes in a new location for
// some vertex within some PolyLine.

void MovieServer::regenerate_PolyLine(
   int selected_PolyLine_ID,int selected_vertex_ID,
   const threevector& new_vertex_posn,bool delete_vertex_flag)
{
//   cout << "inside MovieServer::regenerate_PolyLine()" << endl;
   
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

void MovieServer::regenerate_PolyLine_wo_vertex(
   int selected_PolyLine_ID,int selected_vertex_ID)
{
   threevector new_vertex_posn(Zero_vector);
   bool delete_vertex_flag=true;
   regenerate_PolyLine(selected_PolyLine_ID,selected_vertex_ID,
		       new_vertex_posn,delete_vertex_flag);
}

// ---------------------------------------------------------------------
void MovieServer::drag_image_vertex()
{
//   cout << "inside MovieServer::drag_image_vertex()" << endl;
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE_VERTEX);
   PolyLinePickHandler_ptr->set_enable_drag_flag(true);
   CM_ptr->set_enable_drag_flag(true);
}

QByteArray MovieServer::stop_drag_image_vertex()
{
//   cout << "inside MovieServer::stop_drag_image_vertex()" << endl;
   ModeController_ptr->setState(ModeController::VIEW_DATA);
   PolyLinePickHandler_ptr->set_enable_drag_flag(false);
   CM_ptr->set_enable_drag_flag(false);
   return move_image_vertex(0);
}

// ---------------------------------------------------------------------
QByteArray MovieServer::move_image_vertex(double dz)
{
//   cout << "inside MovieServer::move_image_vertex(), dz = " << dz << endl;

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
QByteArray MovieServer::delete_image_vertex()
{
//   cout << "inside MovieServer::delete_image_vertex()" << endl;

   int selected_PolyLine_ID,selected_vertex_ID;
   get_selected_PolyLine_and_vertex_IDs(
      selected_PolyLine_ID,selected_vertex_ID);
   regenerate_PolyLine_wo_vertex(
      selected_PolyLine_ID,selected_vertex_ID);

   return generate_JSON_response_to_polyline_event();
}

// ---------------------------------------------------------------------
void MovieServer::increase_polyline_vertex_size()
{
   change_polyline_vertex_size(2);
}

void MovieServer::decrease_polyline_vertex_size()
{
   change_polyline_vertex_size(0.5);
}

void MovieServer::change_polyline_vertex_size(double factor)
{
//   cout << "inside MovieServer::change_polyline_vertex_size()" << endl;
//   cout << "factor = " << factor << endl;

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
