// Note added on 10/7/09: In LOSServer::compute_visibility_flowfield()
// we need to check if no ground targets have been set.  If so, alert
// should appear within thin client...

// Note added on 9/18/09: Jennifer Drexler noticed that we are not
// performing any lat/long bbox checks on ground point targets as we
// are for ROIs.  Need to add these checks in later...

// ==========================================================================
// LOSSERVER class file
// ==========================================================================
// Last updated on 1/14/12; 1/25/12; 1/30/12
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "Qt/web/LOSServer.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "osg/osgModels/LOSMODEL.h"
#include "templates/mytemplates.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgModels/OBSFRUSTUM.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "geometry/polyline.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

using std::ofstream;

// ---------------------------------------------------------------------
void LOSServer::allocate_member_objects()
{
}		       

void LOSServer::initialize_member_objects()
{
   map_selected_flag=false;
   northern_hemisphere_flag=true;
   reset_AnimationController_start_stop_times_flag=true;
   Flight_PolyLinesGroup_ptr=NULL;
   Aircraft_MODELSGROUP_ptr=NULL;
   curr_MODEL_ptr=NULL;
   threatmap_Movie_ptr=NULL;
   TilesGroup_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   GroundTarget_SignPostsGroup_ptr=NULL;
   CompassHUD_ptr=NULL;
   ImageNumberHUD_ptr=NULL;
   latlong_bbox_ptr=NULL;

   meters_per_ft=0.3048;
   meters_per_km=1000;
   meters_per_nmi=1852;

   skymap_longitude_lo=skymap_latitude_lo=0;
   skymap_longitude_hi=skymap_latitude_hi=0;

// Set 3D map defaults to Afghanistan/Pakistan:

/*
   longitude_lo=40;
   longitude_hi=54;
   latitude_lo=0;
   latitude_hi=20;
   specified_UTM_zonenumber=38;
   northern_hemisphere_flag=true;
   map_countries_name="HornOfAfrica";
*/

   longitude_lo=60;
   longitude_hi=78;
   latitude_lo=24;
   latitude_hi=42;
   specified_UTM_zonenumber=42;
   northern_hemisphere_flag=true;
   map_countries_name="Afghanistan";

//   longitude_lo=120;
//   longitude_hi=132;
//   latitude_lo=33;
//   latitude_hi=45;
//   specified_UTM_zonenumber=52;
//   northern_hemisphere_flag=true;
//   map_countries_name="Korea";

//   longitude_lo=37;
//   longitude_hi=50;
//   latitude_lo=28;
//   latitude_hi=40;
//   specified_UTM_zonenumber=38;
//   northern_hemisphere_flag=true;
//   map_countries_name="Iraq";

/*
   longitude_lo=-109;
   longitude_hi=-93;
   latitude_lo=25;
   latitude_hi=42;
   specified_UTM_zonenumber=13;
   northern_hemisphere_flag=true;
   map_countries_name="NewMexicoTexas";
*/

/*
   longitude_lo=-125;
   longitude_hi=-109;
   latitude_lo=30;
   latitude_hi=42;
   specified_UTM_zonenumber=12;
   northern_hemisphere_flag=true;
   map_countries_name="CalifAriz";
*/

   screenshot_counter=1;
}

LOSServer::LOSServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
LOSServer::~LOSServer()
{
   delete latlong_bbox_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray LOSServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
   cout << "inside LOSServer:get() #1" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );
   string URL_path;
   return get(doc,response,url,URL_path,responseHeader);
}

// ---------------------------------------------------------------------
QByteArray LOSServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside LOSServer:get() #2 method" << endl;

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
   for (int s=0; s<URL_substrings.size(); s++)
   {
      cout << "s = " << s
           << " URL_substrings[s] = " << URL_substrings[s] << endl;
   }

   string response_msg;

   if (URL_path=="/Set_Map/")
   {
      set_map();
   }

// Controls button event handling:

   else if (URL_path=="/Set_Ground_ROI/")
   {
      return set_ground_ROI_parameters(response_msg);
   }
   else if (URL_path=="/Set_Ground_Targets/")
   {
      set_ground_target_parameters(response_msg);
   }
   else if (URL_path=="/Toggle_Groundpath_Entry/")
   {
      toggle_groundpath_entry();
   }
   else if (URL_path=="/Set_Aircraft_Controls/")
   {
      set_aircraft_parameters(response_msg);
      return generate_JSON_response_to_parameters_request(response_msg);
   }
   else if (URL_path=="/Set_Sensor_Controls/")
   {
      set_sensor_parameters(response_msg);
      return generate_JSON_response_to_parameters_request(response_msg);
   }
   else if (URL_path=="/Compute_Visibility_FlowField/")
   {
      return compute_visibility_flowfield();
   }
   else if (URL_path=="/Set_Threat/")
   {
      return compute_threat_map();
   }
   else if (URL_path=="/Set_Threat_Opacity/")
   {
      change_threat_map_alpha();
   }
   else if (URL_path=="/Clear_Threat/")
   {
      clear_threat_map();
   }
   else if (URL_path=="/Set_Circular_Path/")
   {
      return set_circular_path();
   }
   else if (URL_path=="/Set_Automated_Path/")
   {
      return set_automated_path();
   }
   else if (URL_path=="/Clear_Flightpath/")
   {
      clear_flightpath();
   }
   else if (URL_path=="/Set_Clock_Controls/")
   {
      return parse_clock_parameters(response_msg);
   }
   else if (URL_path=="/Set_Raytracing_Controls/")
   {
      set_raytracing_controls();

// FAKE FAKE:  Sat Jan 14, 2012 at 1:31 pm
// Temporarily comment out set_raytracing_controls() call in favor of
// compute_ROI_visibility() for PYXIS purposes only...

//      compute_ROI_visibility();
   }
   else if (URL_path=="/Get_Target_Occlusion_Fractions/")
   {
      return get_target_occlusion_fractions();
   }
   else if (URL_path=="/Export_Avg_Occlusion_Files/")
   {
      return export_avg_occlusion_files();
   }
   else if (URL_path=="/Cancel_Computation/")
   {
      cancel_calculation();
   }

// PYXIS server member functions:

   else if (URL_path=="/Compute_ROI_visiblity/")
   {
      compute_ROI_visibility();
   }

// Video button response:
    
   else if (URL_path=="/STEP_FORWARD_MOVIE/")
   {
      display_movie_frame(1);
   }
   else if (URL_path=="/STEP_BACKWARD_MOVIE/")
   {
      display_movie_frame(-1);
   }
   else if (URL_path=="/PLAY_MOVIE/")
   {
      play_movie();
   }
   else if (URL_path=="/PAUSE_MOVIE/")
   {
      pause_movie();
      broadcast_curr_thickclient_framenumber();
   }
   else if (URL_path=="/RESTART_MOVIE/")
   {
//      cout << "Restart movie button pressed" << endl;
      reset_clock_to_starting_time();
      display_ImageNumberHUD(true);
   }

// 3D viewer screen capture event handling:

   else if (URL_path=="/Capture_Viewer_Screen/")
   {
      return capture_viewer_screen();

/*      
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
      cout << "subdir = " << subdir << endl;
      string output_filename="viewer_screenshot_"
         +stringfunc::number_to_string(screenshot_counter++)+".png";
      string full_filename=subdir+output_filename;
      cout << "full_filename = " << full_filename << endl;

      outstream << "full_filename = " << full_filename << endl;
      filefunc::closefile(test_filename,outstream);

      MyViewerEventHandler_ptr->setWriteImageFileName(full_filename);
      MyViewerEventHandler_ptr->setWriteImageOnNextFrame(true);           

      response_msg="Saved 3D viewer window into "+
         output_filename+" in movies_and_screen_shots folder on Desktop";
      return generate_JSON_response_to_parameters_request(response_msg);
*/
   }

// Movie recording buttons event handling:

   else if (URL_path=="/Start_Recording_Movie/")
   {
      Start_Recording_Movie();
//      AnimationController_ptr->set_AVI_movie_generation_flag(
//         get_movie_recording_flag());
      return doc.toByteArray();
   }
   else if (URL_path=="/Stop_Recording_Movie/")
   {
      Stop_Recording_Movie();
//      AnimationController_ptr->set_AVI_movie_generation_flag(
//         get_movie_recording_flag());
      broadcast_curr_thickclient_framenumber();

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

QByteArray LOSServer::post(const QUrl& url, const QByteArray& postData,
                           QHttpResponseHeader& responseHeader)
{
   cout << "inside LOSServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   LOSServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;
   if (URL_path=="/Set_General_Flightpath/")
   {
      TilesGroup_ptr->set_avg_LOS_png_files_ready_flag(false);
      string error_msg;
      double flightpath_length=set_general_flightpath(postData,error_msg);

//      cout << "flightpath_length = " << flightpath_length << endl;

//      while (!TilesGroup_ptr->get_avg_LOS_png_files_ready_flag())
//      {
//         WindowManager_ptr->process();
//      }
      return generate_JSON_response_to_flightpath_request(
         flightpath_length,error_msg);
   }
   else if (URL_path=="/Start_New_Analysis/")
   {
      QString full_URL=url.toString()+"?"+QString(postData);
//      cout << "full_URL = " << full_URL.toStdString() << endl;
      extract_KeyValue_pairs(QUrl(full_URL));
      start_automatic_analysis();
   }
   
   return doc.toByteArray();
}

// ==========================================================================
// LOS movie functions
// ==========================================================================

// Member function display_ImageNumberHUD() turns on/off the movie
// number and world_time flags.

void LOSServer::display_ImageNumberHUD(bool flag)
{
   Aircraft_MODELSGROUP_ptr->set_display_ImageNumberHUD(true);
}

// ---------------------------------------------------------------------
// Member function display_movie_frame() 

void LOSServer::display_movie_frame(int frame_step)
{
//   cout << "inside LOSServer::display_movie_frame()" << endl;
//   cout << "frame_step = " << frame_step << endl;

   display_next_frame(frame_step);
   unmask_MODELs_and_rays();
//   broadcast_curr_thickclient_framenumber();
}

// ---------------------------------------------------------------------
// Member function unmask_MODELs_and_rays() 

void LOSServer::unmask_MODELs_and_rays()
{
//   cout << "inside LOSServer::unmask_MODELs_and_rays()" << endl;

   display_ImageNumberHUD(true);
   unhide_aircraft_MODEL_and_OBSFRUSTA();
}

void LOSServer::hide_aircraft_MODEL_and_OBSFRUSTA()
{
   set_aircraft_MODEL_and_OBSFRUSTA_masks(0);
}

void LOSServer::unhide_aircraft_MODEL_and_OBSFRUSTA()
{
   display_ImageNumberHUD(true);
   set_aircraft_MODEL_and_OBSFRUSTA_masks(1);
}

void LOSServer::set_aircraft_MODEL_and_OBSFRUSTA_masks(int mask_value)
{
//   cout << "inside LOSServer::set_aircraft_MODEL_and_OBSFRUSTA_masks()" 
//        << endl;
//   cout << "mask_value = " << mask_value << endl;
//   cout << "Aircraft_MODELSGROUP_ptr->get_n_OSGsubPATs() = "
//        << Aircraft_MODELSGROUP_ptr->get_n_OSGsubPATs() << endl;

   for (int i=0; i<Aircraft_MODELSGROUP_ptr->get_n_OSGsubPATs(); i++)
   {
      Aircraft_MODELSGROUP_ptr->set_OSGsubPAT_nodemask(i,mask_value);
   }

// When time-averaged raytracing results are displayed,
// LineSegmentsGroup holding individual ground target rays is masked.
// So we unmask this OSG group when time dependence is restored:

   Aircraft_MODELSGROUP_ptr->get_LineSegmentsGroup_ptr()->
      set_OSGgroup_nodemask(mask_value);
}

// ---------------------------------------------------------------------
// Member function play_movie() sets the AnimationController's clock
// running.

void LOSServer::play_movie()
{
//   cout << "inside LOSServer::play_movie()" << endl;
//   cout << "Playing movie" << endl;
   BasicServer::play_movie();
   unmask_MODELs_and_rays();
}

// ---------------------------------------------------------------------
// Member function broadcast_curr_thickclient_framenumber() issues an
// ActiveMQ message with the current framenumber in order to keep the
// thick and thin clients synchronized.

void LOSServer::broadcast_curr_thickclient_framenumber() const
{
   cout << "inside LOSServer::broadcast_curr_thickclient_framenumber()" 
        << endl;

   Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
      broadcast_current_framenumber(
         AnimationController_ptr->get_curr_framenumber());
}

// ---------------------------------------------------------------------
// Member function reset_AnimationController_start_stop_times()

void LOSServer::reset_AnimationController_start_stop_times(
   double flightpath_length)
{
   cout << "inside LOSServer::reset_AnimationController_start_stop_times()"
        << endl;
//   cout << "AnimationController_ptr = " << AnimationController_ptr << endl;

   double flight_time_in_secs=
      flightpath_length/Aircraft_MODELSGROUP_ptr->get_LiMIT_speed();   // secs

// Reset stop time = start time + flight time in secs.  Then recompute
// number of animation frames:

   Operations_ptr->set_master_world_stop_time(
      Operations_ptr->get_master_world_start_time()+flight_time_in_secs);

//   cout << "world start time = "
//        << Operations_ptr->get_master_world_start_time() << endl;
//   cout << "world stop time = "
//        << Operations_ptr->get_master_world_stop_time() << endl;
   
   AnimationController_ptr->set_world_time_params(
      Operations_ptr->get_master_world_start_time(),
      Operations_ptr->get_master_world_stop_time(),
      Operations_ptr->get_delta_master_world_time_step_per_master_frame());
   cout << "n_frames = " << AnimationController_ptr->get_nframes() << endl;
//   outputfunc::enter_continue_char();
}

// ==========================================================================
// JSON response member functions
// ==========================================================================

// Member function generate_JSON_response_to_ROI_entry() returns a
// JSON string to Michael Yee's thin client which contains a
// LineString written in GEOJSON format corresponding to the ROI bbox.
 
QByteArray LOSServer::generate_JSON_response_to_ROI_entry(
   bool valid_ROI_flag,string response_msg,
   double lower_left_longitude,double lower_left_latitude,
   double upper_right_longitude,double upper_right_latitude)
{
   cout << "Inside LOSServer::generate_JSON_response_to_ROI_entry()" << endl;
   string json_string = "{ \"type\": \"LineString\", \n";

   if (!valid_ROI_flag)
   {
      json_string += "  \"message\": \"";
      json_string += response_msg;
      json_string += "\" \n";
   }
   else
   {
      json_string += " \"coordinates\": [ ";   
      json_string += " ["+stringfunc::number_to_string(lower_left_longitude)
         +","+stringfunc::number_to_string(lower_left_latitude)+" ],";
      json_string += " ["+stringfunc::number_to_string(upper_right_longitude)
         +","+stringfunc::number_to_string(lower_left_latitude)+" ],";
      json_string += " ["+stringfunc::number_to_string(upper_right_longitude)
         +","+stringfunc::number_to_string(upper_right_latitude)+" ],";
      json_string += " ["+stringfunc::number_to_string(lower_left_longitude)
         +","+stringfunc::number_to_string(upper_right_latitude)+" ],";
      json_string += " ["+stringfunc::number_to_string(lower_left_longitude)
         +","+stringfunc::number_to_string(lower_left_latitude)+" ]";
      json_string += " ] \n";
   }
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function
// generate_JSON_response_to_circular_flightpath_entry() returns a
// JSON string to Michael Yee's thin client which contains a
// LineString written in GEOJSON form corresponding to the circular
// flight path.

QByteArray LOSServer::generate_JSON_response_to_circular_flightpath_entry(
   double center_longitude,double center_latitude,double orbit_radius,
   double score)
{
   cout << "Inside LOSServer::generate_JSON_response_to_circular_flightpath()"
        << endl;
   
   PolyLinesGroup* Path_PolyLinesGroup_ptr=Aircraft_MODELSGROUP_ptr->
      get_Path_PolyLinesGroup_ptr();

   double flightpath_length=0;
   vector<geopoint> G;
   if (Path_PolyLinesGroup_ptr->get_n_Graphicals() > 0)
   {
      PolyLine* circular_PolyLine_ptr=Path_PolyLinesGroup_ptr->
         get_PolyLine_ptr(0);
      polyline* polyline_ptr=circular_PolyLine_ptr->get_or_set_polyline_ptr();
      flightpath_length=polyline_ptr->compute_total_length();

      vector<threevector> V;
      int n_vertices=polyline_ptr->get_n_vertices();
      for (int n=0; n<n_vertices; n++)
      {
         V.push_back(polyline_ptr->get_vertex(n));
      }
      V.push_back(polyline_ptr->get_vertex(0));

      for (int n=0; n<V.size(); n++)
      {
         G.push_back(
            geopoint(northern_hemisphere_flag,specified_UTM_zonenumber,
                     V[n].get(0),V[n].get(1)));
      }
   } // n_PolyLines > 0 conditional
   
   string json_string = "{ \"type\": \"LineString\", \n";

   bool include_final_comma_flag=true;
   json_string += generate_JSON_flight_distance_and_time_string(
      flightpath_length,include_final_comma_flag);

//   json_string += " \"score\": \""+stringfunc::number_to_string(score)
//      +"\" , \n";

   json_string += " \"coordinates\": [ ";

   if (nearly_equal(center_longitude,0) &&
       nearly_equal(center_latitude,0) &&
       nearly_equal(orbit_radius,0))
   {
   }
   else
   {
      for (int n=0; n<G.size(); n++)
      {
         json_string += 
            " ["+stringfunc::number_to_string(G[n].get_longitude())
            +","+stringfunc::number_to_string(G[n].get_latitude())+" ]";
         if (n < G.size()-1) json_string += ",";
      } // loop over index n labeling geopoints within STL vector G
   }
   
   json_string += " ] \n }";
   json_string += "\n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_flightpath_request()
// returns a list of averaged LOS PNG filenames along with their
// lower-left and upper right geocoordinates in the form of a JSON
// string.  After parsing this JSON string, Michael Yee's OpenMaps
// thin client can (hopefully) display the averaged LOS PNG files
// within a browser.

QByteArray LOSServer::generate_JSON_response_to_flightpath_request(
   double flightpath_length,string error_msg)
{
//   cout << "Inside LOSServer::generate_JSON_response_to_flightpath_request()"
//        << endl;

   string json_string = "{ \n";

   json_string += "  \"message\": \"";
   json_string += error_msg;
   json_string += "\" , \n";

   bool include_final_comma_flag=false;
   json_string += generate_JSON_flight_distance_and_time_string(
      flightpath_length,include_final_comma_flag);
   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;

   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_flight_distance_and_time_string()
// returns the flight path distance in kilometers and total flight
// time in hours, minutes and seconds.

string LOSServer::generate_JSON_flight_distance_and_time_string(
   double flightpath_length,bool include_final_comma_flag)

{
   cout << "inside LOSServer::generate_JSON_flight_distance_and_time_string()"
        << endl;
   string json_string = " \"pathlength\": \""+stringfunc::number_to_string(
      0.001*flightpath_length)+"\" , \n";

   cout << "reset_AnimationController_start_stop_times_flag = "
        << reset_AnimationController_start_stop_times_flag << endl;
   if (reset_AnimationController_start_stop_times_flag)
      reset_AnimationController_start_stop_times(flightpath_length);

   double flight_time_in_secs=
      flightpath_length/Aircraft_MODELSGROUP_ptr->get_LiMIT_speed();   // secs
 
   double frac_day=timefunc::hms_to_frac_day(0,0,flight_time_in_secs);
   int hours,minutes;
   double seconds;
   timefunc::frac_day_to_hms(frac_day,hours,minutes,seconds);
//   cout << "hours = " << hours << " minutes = " << minutes
//        << " seconds = " << seconds << endl;

   json_string += " \"n_frames\": \""+stringfunc::number_to_string(
      AnimationController_ptr->get_nframes())+"\" , \n";
   json_string += " \"flighttime_hours\": \""+stringfunc::number_to_string(
      hours)+"\" , \n";
   json_string += " \"flighttime_minutes\": \""+stringfunc::number_to_string(
      minutes)+"\" , \n";
   json_string += " \"flighttime_seconds\": \""+stringfunc::number_to_string(
      seconds);
   if (include_final_comma_flag)
   {
      json_string += "\" , \n";
   }
   else
   {
      json_string += "\" \n";
   }

   cout << "json_string = " << json_string << endl;
   return json_string;
}

// ==========================================================================
// Parameter setting member functions
// ==========================================================================

// Member function set_bbox_corners() parses key-value pairs for the
// lower left and upper right corners of lat-lon (or MGRS) bounding
// boxes.

void LOSServer::set_bbox_corners(
   int k,double& lower_left_longitude,double& lower_left_latitude,
   double& upper_right_longitude,double& upper_right_latitude)
{
//   cout << "KeyValue[k].first = " << KeyValue[k].first << endl;
//   cout << "KeyValue[k].second = " << KeyValue[k].second << endl;

   if (KeyValue[k].first=="LowerLeftLongitude")
   {
      lower_left_longitude=stringfunc::string_to_number(KeyValue[k].second);
   }
   else if (KeyValue[k].first=="LowerLeftLatitude")
   {
      lower_left_latitude=stringfunc::string_to_number(KeyValue[k].second);
   }
   else if (KeyValue[k].first=="UpperRightLongitude")
   {
      upper_right_longitude=stringfunc::string_to_number(KeyValue[k].second);
   }
   else if (KeyValue[k].first=="UpperRightLatitude")
   {
      upper_right_latitude=stringfunc::string_to_number(KeyValue[k].second);
   }
   else if (KeyValue[k].first=="LowerLeftMGRS")
   {
      string lowerleftMGRS_str=KeyValue[k].second;
      geofunc::MGRS_to_long_lat(lowerleftMGRS_str,lower_left_longitude,
                                lower_left_latitude);
   }
   else if (KeyValue[k].first=="UpperRightMGRS")
   {
      string upperrightMGRS_str=KeyValue[k].second;
      geofunc::MGRS_to_long_lat(upperrightMGRS_str,upper_right_longitude,
                                upper_right_latitude);
   }
}

// ---------------------------------------------------------------------
// Member function set_map() parses and stores 3D map parameters
// selected by a user within the LOST thin client.

void LOSServer::set_map()
{
   cout << "inside LOSServer::set_map()" << endl;

   int n_args=KeyValue.size();

   for (int k=0; k<n_args; k++)
   {
      cout << "k = " << k << endl;
      cout << " KeyValue[k].first = " << KeyValue[k].first 
           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      
      if (KeyValue[k].first=="mapName")
      {
         map_countries_name=KeyValue[k].second;
      }
      if (KeyValue[k].first=="longitude_lo")
      {
         longitude_lo=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="longitude_hi")
      {
         longitude_hi=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="latitude_lo")
      {
         latitude_lo=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="latitude_hi")
      {
         latitude_hi=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="UTM_zonenumber")
      {
         specified_UTM_zonenumber=
            stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="northern_hemisphere_value")
      {
         northern_hemisphere_flag=stringfunc::string_to_boolean(
            KeyValue[k].second);
      }

   } // loop over index k labeling KeyValue key possibilities

   map_selected_flag=true;
   cout << "map_countries_name = " << map_countries_name << endl;
   cout << "lon_lo = " << longitude_lo << " lon_hi = " << longitude_hi
        << endl;
   cout << "lat_lo = " << latitude_lo << " lat_hi = " << latitude_hi
        << endl;
   cout << "UTM zone = " << specified_UTM_zonenumber
        << " northern hemi flag = " << northern_hemisphere_flag << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function set_ground_ROI_parameters() takes in geocoordinates
// for the lower left and upper right corners of a bounding box on the
// ground which restricts where the sensor's raytracing region of
// interest.

QByteArray LOSServer::set_ground_ROI_parameters(string& response_msg)
{
   cout << "inside LOSServer::set_ground_ROI_parameters()" << endl;

   int n_args=KeyValue.size();
   bool valid_ROI_flag=true;
   double lower_left_longitude=0;
   double lower_left_latitude=0;
   double upper_right_longitude=0;
   double upper_right_latitude=0;

   cout << "n_args = " << n_args << endl;
   int n_entered_values=0;
   for (int k=0; k<n_args; k++)
   {

// Check whether every input value is empty.  If so, purge any
// existing ROI:

      cout << "KeyValue[k].first = " << KeyValue[k].first
           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      cout << "KeyValue[k].first.size() = "
           << KeyValue[k].first.size() << endl;
      cout << "KeyValue[k].second.size() = "
           << KeyValue[k].second.size() << endl;

      if (KeyValue[k].first.size() > 0 && KeyValue[k].second.size() > 0) 
      {
         n_entered_values++;
      }
      
      set_bbox_corners(k,lower_left_longitude,lower_left_latitude,
                       upper_right_longitude,upper_right_latitude);
   } // loop over index k

// If CLEAR ROI button was pressed, then n_values=0:

//   cout << "n_entered_values = " << n_entered_values << endl;
   if (n_entered_values > 0 && n_entered_values < 4)
   {
      response_msg="Insufficient number of parameters entered";
      valid_ROI_flag=false;
   }

//   cout << "valid_ROI_flag = " << valid_ROI_flag << endl;
//   cout << "lower_left_longitude = " << lower_left_longitude
//        << " lower_left_latitude = " << lower_left_latitude
//        << " upper_right_longitude = " << upper_right_longitude
//        << " upper_right_latitude = " << upper_right_latitude << endl;

   if (valid_ROI_flag && n_entered_values > 0)
   {
      if (lower_left_longitude < latlong_bbox_ptr->get_xmin())
      {
         response_msg="Lower left longitude cannot be less than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_min_long());
         valid_ROI_flag=false;
      }
      else if (lower_left_longitude > latlong_bbox_ptr->get_xmax())
      {
         response_msg="Lower left longitude cannot be greater than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_max_long());
         valid_ROI_flag=false;
      }

      if (lower_left_latitude < latlong_bbox_ptr->get_ymin())
      {
         response_msg="Lower left latitude cannot be less than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_min_lat());
         valid_ROI_flag=false;
      }
      else if (lower_left_latitude > latlong_bbox_ptr->get_ymax())
      {
         response_msg="Lower left latitude cannot be greater than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_max_lat());
         valid_ROI_flag=false;
      }

      if (upper_right_longitude < latlong_bbox_ptr->get_xmin())
      {
         response_msg="Upper right longitude cannot be less than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_min_long());
         valid_ROI_flag=false;
      }
      else if (upper_right_longitude > latlong_bbox_ptr->get_xmax())
      {
         response_msg="Upper right longitude cannot be greater than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_max_long());
         valid_ROI_flag=false;
      }

      if (upper_right_latitude < latlong_bbox_ptr->get_ymin())
      {
         response_msg="Upper right latitude cannot be less than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_min_lat());
         valid_ROI_flag=false;
      }
      else if (upper_right_latitude > latlong_bbox_ptr->get_ymax())
      {
         response_msg="Upper right latitude cannot be greater than "
            +stringfunc::number_to_string(TilesGroup_ptr->get_max_lat());
         valid_ROI_flag=false;
      }
   }
   
//   cout << "response_msg = " << response_msg << endl;
//   cout << "valid_ROI_flag = " << valid_ROI_flag << endl;

   if ((lower_left_longitude > upper_right_longitude) ||
       lower_left_latitude > upper_right_latitude)
   {
      cout << "Error in LOSServer::set_ground_ROI_parameters()" << endl;
      cout << "lower_left_longitude = " << lower_left_longitude
           << " lower_left_latitude = " << lower_left_latitude
           << " upper_right_longitude = " << upper_right_longitude
           << " upper_right_latitude = " << upper_right_latitude << endl;
   }

   Aircraft_MODELSGROUP_ptr->destroy_ground_bbox();

   if (valid_ROI_flag && n_entered_values > 0)
   {
      Aircraft_MODELSGROUP_ptr->set_ground_bbox(
         lower_left_longitude,lower_left_latitude,
         upper_right_longitude,upper_right_latitude);
   }

   return generate_JSON_response_to_ROI_entry(
      valid_ROI_flag,response_msg,
      lower_left_longitude,lower_left_latitude,
      upper_right_longitude,upper_right_latitude);
}

// ---------------------------------------------------------------------
// Member function set_ground_target_parameters() computes altitude
// values for Ground target SignPosts using DTED height map data.

bool LOSServer::set_ground_target_parameters(string& response_msg)
{
   cout << "inside LOSServer::set_ground_target_parameters()" << endl;

   int n_args=KeyValue.size();
//   cout << "n_args = " << n_args << endl;

/*
   QString tmp_qstring=tmp_url.fromPercentEncoding(postData);
   string post_data=tmp_qstring.toStdString();
//   cout << "post_data = " << post_data << endl;
//   cout << "post_data.size() = " << post_data.size() << endl;

//      QUrl decoded_url=tmp_url.fromEncoded(postData);
//      string post_data2_str=decoded_url.toString().toStdString();
//      cout << "post_data2_str = " << post_data2_str << endl;
*/

   vector<int> longitude_index,latitude_index,label_index;
   vector<double> longitudes,latitudes;
   vector<string> labels;
   longitudes.reserve(1000);
   latitudes.reserve(1000);
   labels.reserve(1000);

   bool valid_targets_flag=true;
   bool longitude_entry_found_flag=false;
   bool latitude_entry_found_flag=false;
   bool mgrs_entry_found_flag=false;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         KeyValue[k].first,"123456789");

//      cout << "substrings.size() = " << substrings.size() << endl;
//      for (int s=0; s<int(substrings.size()); s++)
//      {
//         cout << "s = " << s << " substrings[s] = " << substrings[s] 
//              << endl;
//      }
  
// Recall that key-value pairs are serialized by thin client in some
// random way.  So we cannot assume that the ordering of inputs to the
// Server corresponds to the order in which the user entered
// information into the thin client.  "Latitude", "Longitude" and
// "Label" keywords therefore have integer indices appended to
// indicate ordering.

// Numerical index generally corresponds to substring of KeyValue[k].first 
// appearing after "Latitude", "Longitude" or "Label":
  
//      cout << "KeyValue[k].first.size() = " << KeyValue[k].first.size()
//           << " substrings[0].size() = " << substrings[0].size() << endl;
  
      string index_str=KeyValue[k].first.substr(
         substrings[0].size(),KeyValue[k].first.size()-substrings[0].size());
//      cout << "index_str = " << index_str << endl;
      int index=stringfunc::string_to_number(index_str);
//      cout << "index = " << index << endl;

      if (substrings[0]=="Longitude")
      {
         longitude_index.push_back(index);
         double curr_longitude=stringfunc::string_to_number(
            KeyValue[k].second);
         longitudes.push_back(curr_longitude);
         cout << "longitude = " << longitudes.back() << endl;
         longitude_entry_found_flag=true;
      }
      else if (substrings[0]=="Latitude")
      {
         latitude_index.push_back(index);
         double curr_latitude=stringfunc::string_to_number(
            KeyValue[k].second);
         latitudes.push_back(curr_latitude);
         cout << "latitude = " << latitudes.back() << endl;
         latitude_entry_found_flag=true;
      }
      else if (substrings[0]=="MGRS")
      {
         string MGRS_str=KeyValue[k].second;
         double curr_longitude,curr_latitude;
         geofunc::MGRS_to_long_lat(MGRS_str,curr_longitude,curr_latitude);
         cout << "MGRS_str = " << MGRS_str << endl;
         cout << "long = " << curr_longitude 
              << " lat = " << curr_latitude << endl;
         longitude_index.push_back(index);
         longitudes.push_back(curr_longitude);
         latitude_index.push_back(index);
         latitudes.push_back(curr_latitude);
         mgrs_entry_found_flag=true;
      }
      else if (substrings[0]=="Label")
      {
         label_index.push_back(index);
         labels.push_back(KeyValue[k].second);
         cout << "label = " << labels.back() << endl;
      }
   } // loop over index k labeling KeyValue key possibilities

// Explicitly check whether a signal corresponding to Clear Targets
// was transmitted when the Coordinate System is in either LongLat or
// MGRS modes:

   if ((longitude_entry_found_flag && latitude_entry_found_flag) ||
       mgrs_entry_found_flag)
   {
   }
   else if (n_args==2 && KeyValue[0].first=="MGRS1" &&
            KeyValue[0].second.size()==0)
   {
      cout << "Clear Targets signal detected on MGRS mode" << endl;
   }
   else if (n_args==2 && KeyValue[1].first=="MGRS1" &&
            KeyValue[1].second.size()==0)
   {
      cout << "Clear Targets signal detected on MGRS mode" << endl;
   }
   else if (n_args==3 && 
            KeyValue[0].first=="Longitude1" && KeyValue[1].first=="Latitude1" 
            && KeyValue[0].second.size()==0 && KeyValue[1].second.size()==0)
   {
      cout << "Clear Targets signal detected on LongLat mode" << endl;
   }
   else if (n_args==3 && 
            KeyValue[1].first=="Longitude1" && KeyValue[0].first=="Latitude1" 
            && KeyValue[0].second.size()==0 && KeyValue[1].second.size()==0)
   {
      cout << "Clear Targets signal detected on LongLat mode" << endl;
   }
   else if (n_args==3 && 
            KeyValue[0].first=="Longitude1" && KeyValue[2].first=="Latitude1" 
            && KeyValue[2].second.size()==0 && KeyValue[0].second.size()==0)
   {
      cout << "Clear Targets signal detected on LongLat mode" << endl;
   }
   else if (n_args==3 && 
            KeyValue[2].first=="Longitude1" && KeyValue[0].first=="Latitude1" 
            && KeyValue[2].second.size()==0 && KeyValue[0].second.size()==0)
   {
      cout << "Clear Targets signal detected on LongLat mode" << endl;
   }
   else if (n_args==3 && 
            KeyValue[1].first=="Longitude1" && KeyValue[2].first=="Latitude1" 
            && KeyValue[1].second.size()==0 && KeyValue[2].second.size()==0)
   {
      cout << "Clear Targets signal detected on LongLat mode" << endl;
   }
   else if (n_args==3 && 
            KeyValue[2].first=="Longitude1" && KeyValue[1].first=="Latitude1" 
            && KeyValue[2].second.size()==0 && KeyValue[1].second.size()==0)
   {
      cout << "Clear Targets signal detected on LongLat mode" << endl;
   }
   else
   {
      response_msg="Insufficient number of ground target parameters entered";
      return false;
   }

// Sort longitudes, latitudes and labels by their indices:

   templatefunc::Quicksort(longitude_index,longitudes);
   templatefunc::Quicksort(latitude_index,latitudes);
   templatefunc::Quicksort(label_index,labels);
   
// Generate new SignPost for each valid ground target
// longitude/latitude pair:

   GroundTarget_SignPostsGroup_ptr->destroy_all_SignPosts();
   GroundTarget_SignPostsGroup_ptr->set_common_geometrical_size(20);

//   cout << "GroundTarget_SignPostsGroup_ptr->get_common_geometrical_size()="
//        << GroundTarget_SignPostsGroup_ptr->get_common_geometrical_size()
//        << endl;
   
// As of 6/2/09, Michael Yee's webform sends a final KeyValue ground
// target pair with no value entry.  So do NOT instantiate a SignPost
// corresponding to the very last dummy lon-lat-label entry:

   cout << "longitudes.size() = " << longitudes.size() << endl;
   for (int s=0; s<longitudes.size()-1; s++)
   {
      double altitude=1000;	// meters

// Try to derive altitude from DTED map given ground target's
// longitude, latitude coords.  As DTED maps can have missing-data
// holes, we sample a distribution of Z values in the neighborhood of
// a SignPost.  Take the maximal height value as a reasonable estimate
// for the SignPosts's altitude:

      if (TilesGroup_ptr != NULL)
      {
         int n_extra_degs=0;

         geopoint curr_geopoint(
            longitudes[s],latitudes[s],0,specified_UTM_zonenumber);
         double easting=curr_geopoint.get_UTM_easting();
         double northing=curr_geopoint.get_UTM_northing();

         vector<twovector> target_posns;
         target_posns.push_back(twovector(easting,northing));

         double raytrace_cellsize=30;	// meters
         twoDarray* DTED_ztwoDarray_ptr=NULL;
         if (TilesGroup_ptr->get_ladar_height_data_flag())
         {
//             raytrace_cellsize=0.3;	// meters
            DTED_ztwoDarray_ptr=TilesGroup_ptr->
               load_ladar_height_data_into_ztwoDarray();
         }
         else
         {
            DTED_ztwoDarray_ptr=TilesGroup_ptr->load_all_DTED_tiles(
               n_extra_degs,raytrace_cellsize,target_posns);
         }

         unsigned int px,py;
         if (DTED_ztwoDarray_ptr->point_to_pixel(easting,northing,px,py))
         {
            bool altitude_calculated_flag=false;
            for (int qlimit=3; qlimit<200 && !altitude_calculated_flag; 
                 qlimit++)
            {
               vector<double> Z_values;
               for (int qx=-qlimit; qx<=qlimit; qx++)
               {
                  for (int qy=-qlimit; qy<=qlimit; qy++)
                  {
                     if (DTED_ztwoDarray_ptr->pixel_inside_working_region(
                        px+qx,py+qy))
                     {
                        double curr_z=DTED_ztwoDarray_ptr->get(px+qx,py+qy);
                        if (curr_z > 0.5*NEGATIVEINFINITY)
                        {
                           Z_values.push_back(curr_z);
                        }
                     }
                  } // loop over qy index
               } // loop over qx index
               if (Z_values.size() > 2)
               {

// On 9/16/09, we empirically found that geotiff DTED values sometimes
// erroneously equal 0.  So we sort the Z_values and take the maximal
// value in order to minimize the impact of these bad zeros:

                  std::sort(Z_values.begin(),Z_values.end());
                  altitude=Z_values.back();
                  altitude_calculated_flag=true;
               }
            } // loop of qlimit index 
         } // point_to_pixel conditional
      } // TilesGroup_ptr != NULL conditional
      
      SignPost* SignPost_ptr=
         GroundTarget_SignPostsGroup_ptr->generate_new_SignPost_on_earth(
            longitudes[s],latitudes[s],altitude,specified_UTM_zonenumber,-1);

// Limit label to its first 20 characters:

      string curr_label=labels[s].substr(0,20);
      SignPost_ptr->set_label(curr_label);

      cout << "s = " << s 
           << " Longitude = " << longitudes[s]
           << " Latitude = " << latitudes[s] 
           << " altitude = " << altitude 
           << " label = " << labels[s] << endl;
      
   } // loop over index s labeling ground targets

   GroundTarget_SignPostsGroup_ptr->set_colors(
      colorfunc::white,colorfunc::darkpurple);
//      colorfunc::darkpurple,colorfunc::white);

   return true;
}

// ---------------------------------------------------------------------
// Member function set_aircraft_parameters()

bool LOSServer::set_aircraft_parameters(string& response_msg)
{
//   cout << "inside LOSServer::set_aircraft_parameters()" << endl;

   int n_args=KeyValue.size();

   bool empty_fields_entry_flag=false;
   double altitude,speed;
   for (int k=0; k<n_args; k++)
   {

      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      if (KeyValue[k].first=="AircraftAltitude")
      {
         altitude=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="AircraftSpeed")
      {
         speed=stringfunc::string_to_number(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities

   if (empty_fields_entry_flag)
   {
      response_msg="Insufficient number of parameters entered";
      cout << response_msg << endl;
      return false;
   }

   if (altitude < 0 || speed < 0)
   {
      response_msg="Invalid negative aircraft parameters entered";
      return false;
   }

// After having read in numerical values for altitude and speed, next
// parse their units and convert values to MKS system:

   string altitude_units,speed_units;
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="AltitudeUnits")
      {
         altitude_units=KeyValue[k].second;
      }
      else if (KeyValue[k].first=="SpeedUnits")
      {
         speed_units=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities

   if (altitude_units=="ft")
   {
      altitude *= meters_per_ft ;
   }
   else if (altitude_units=="nmi")
   {
      altitude *= meters_per_nmi ;   
   }

   if (speed_units=="ft_per_sec")
   {
      speed *= meters_per_ft ;
   }
   else if (speed_units=="knots")
   {
      speed *= 0.5144 ;   // 1 knot = 0.5144 m/sec
   }
//   cout << "altitude = " << altitude << " meters " 
//        << " speed = " << speed << " m/s" << endl;

// Perform some sanity checks on input aircraft parameters:

   const double max_aircraft_altitude=50000;	// meters
   if (altitude > max_aircraft_altitude)
   {
      response_msg="Aircraft altitude not allowed to exceed 50 km";
      return false;
   }
   
   const double max_aircraft_speed=340;		// meters
   if (speed > max_aircraft_speed)
   {
      response_msg="Aircraft speed not allowed to exceed speed of sound = 340 meters/sec";
      return false;
   }

// Issue warning if aircraft's altitude is so low that it could
// potentially crash into a mountain peak:

   response_msg="";
   double ground_zmax=PointCloudsGroup_ptr->get_max_value(2);
   if (ground_zmax > altitude)
   {
//      cout << "altitude = " << altitude 
//           << " ground_zmax = " << ground_zmax << endl;
      response_msg=
         "WARNING: Aircraft altitude lies below highest mountain peak height (";
      response_msg += stringfunc::number_to_string(ground_zmax/meters_per_ft,0)
         +" feet) !";
   }

//   cout << "Aircraft_MODELSGROUP_ptr = " << Aircraft_MODELSGROUP_ptr
//        << endl;
   Aircraft_MODELSGROUP_ptr->set_aircraft_altitude(altitude);
   Aircraft_MODELSGROUP_ptr->set_LiMIT_speed(speed);

// Reset altitude of UAV's flight PolyLine if it exists:

   PolyLinesGroup* Path_PolyLinesGroup_ptr=Aircraft_MODELSGROUP_ptr->
      get_Path_PolyLinesGroup_ptr();
   if (Path_PolyLinesGroup_ptr != NULL)
   {
      PolyLine* Flightpath_PolyLine_ptr=Path_PolyLinesGroup_ptr->
         get_PolyLine_ptr(0);
      if (Flightpath_PolyLine_ptr != NULL)
      {
         Path_PolyLinesGroup_ptr->reset_PolyLine_altitudes(
            Flightpath_PolyLine_ptr);
      }

// Reset aircraft MODEL's track:

      MODEL* LiMIT_MODEL_ptr=Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);
      if (LiMIT_MODEL_ptr != NULL)
      {
         int initial_framenumber=AnimationController_ptr->
            get_first_framenumber();
         Aircraft_MODELSGROUP_ptr->
            update_UAV_track(initial_framenumber,Flightpath_PolyLine_ptr,
                             LiMIT_MODEL_ptr);
      } // LiMIT_MODEL_ptr != NULL conditional
   } // Path_PolyLinesGroup_ptr != NULL conditional

//   cout << "At end of LOSServer::set_aircraft_params()" << endl;

   if (response_msg.size()==0) response_msg="Aircraft parameters updated";
   return true;
}

// ---------------------------------------------------------------------
// Member function set_sensor_parameters() parses the sensor parameter
// section of the input HTML form.  It checks whether the entered
// vertical FOV and depression angle values are mathematically consistent.

bool LOSServer::set_sensor_parameters(string& response_msg)
{
//   cout << "inside LOSServer::set_sensor_parameters() #1" << endl;

   int n_args=KeyValue.size();

   bool empty_fields_entry_flag=false;
   double min_ground_sensor_range,max_ground_sensor_range,horiz_FOV;
   string lobe_pattern;

   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].second.size()==0) empty_fields_entry_flag=true;

      if (KeyValue[k].first=="MinRange")
      {
         min_ground_sensor_range=
            stringfunc::string_to_number(KeyValue[k].second); 
      }
      else if (KeyValue[k].first=="MaxRange")
      {
         max_ground_sensor_range=
            stringfunc::string_to_number(KeyValue[k].second);  
      }
      else if (KeyValue[k].first=="HorizontalFOV")
      {
         horiz_FOV=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="Lobes")
      {
         lobe_pattern=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities

   if (empty_fields_entry_flag)
   {
      response_msg="Insufficient number of parameters entered";
//      cout << "response_msg = " << response_msg << endl;
      return false;
   }

   if (min_ground_sensor_range < 0 || max_ground_sensor_range < 0 || 
       horiz_FOV < 0 
      )
   {
      response_msg="Invalid negative sensor parameters entered";
      return false;
   }

// After having read in numerical values for min and max ranges, next
// parse their units and convert values to meters:
   
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="MinRangeUnits")
      {
         string min_range_units=KeyValue[k].second;
         if (min_range_units=="kms")
         {
            min_ground_sensor_range *= meters_per_km; 
         }
         else if (min_range_units=="nmi")
         {
            min_ground_sensor_range *= meters_per_nmi;	
         }
      }
      else if (KeyValue[k].first=="MaxRangeUnits")
      {
         string max_range_units=KeyValue[k].second;
         if (max_range_units=="kms")
         {
            max_ground_sensor_range *= meters_per_km; 
         }
         else if (max_range_units=="nmi")
         {
            max_ground_sensor_range *= meters_per_nmi;	
         }
      }
   } // loop over index k labeling KeyValue key possibilities

   if (min_ground_sensor_range > 200*1000)
   {
      response_msg="Minimum ground sensor range must be less than 200 km";
      return false;
   }

   if (max_ground_sensor_range > 400*1000)
   {
      response_msg="Maximum ground sensor range must be less than than 400 km";
      return false;
   }

   if (min_ground_sensor_range > max_ground_sensor_range)
   {
      response_msg="Minimum ground sensor range must be less than maximum ground sensor range";
      return false;
   }

   if (horiz_FOV > 170)
   {
      response_msg="Azimuthal field-of-regard not allowed to exceed 170 degs";
      return false;
   }
   else if (horiz_FOV < 5)
   {
      response_msg="Azimuthal field-of-regard must exceed 5 degs";
      return false;
   }

   set_sensor_parameters(
      min_ground_sensor_range,max_ground_sensor_range,horiz_FOV,lobe_pattern);

   response_msg="Sensor parameters are mathematically OK.";
   return true;
}

// ---------------------------------------------------------------------
void LOSServer::set_sensor_parameters(
   double min_ground_sensor_range,double max_ground_sensor_range,
   double horiz_FOV,string lobe_pattern)
{
   cout << "inside LOSServer::set_sensor_parameters() #2" << endl;

   double roll_sgn=-1; // right sided
   if (lobe_pattern=="Left side") roll_sgn=1;

   Aircraft_MODELSGROUP_ptr->compute_OBSFRUSTUM_parameters(
      min_ground_sensor_range,max_ground_sensor_range,
      horiz_FOV,roll_sgn);

   if (lobe_pattern=="Double sided") 
   {
      cout << "Double sided LiMIT lobe pattern set" << endl;
      Aircraft_MODELSGROUP_ptr->set_double_LiMIT_lobe_pattern_flag(true);
   }
   else
   {
      Aircraft_MODELSGROUP_ptr->set_double_LiMIT_lobe_pattern_flag(false);
   }

   clear_raytracing_results();
   Aircraft_MODELSGROUP_ptr->destroy_all_MODELS();
   Aircraft_MODELSGROUP_ptr->set_generate_Predator_model_on_next_cycle_flag(
      true);
}

// ==========================================================================
// Sky map member functions
// ==========================================================================

// Member function compute_threat_map()

QByteArray LOSServer::compute_threat_map()
{
   cout << "****************************************************" << endl;
   cout << "inside LOSServer::compute_threat_map()" << endl;

   clear_raytracing_results();

   double lower_left_longitude=0;
   double lower_left_latitude=0;
   double upper_right_longitude=0;
   double upper_right_latitude=0;
   double SAM_range=0;
   string range_units;
   
   int n_args=KeyValue.size();
   cout << "n_args = " << n_args << endl;

   for (int k=0; k<n_args; k++)
   {
      cout << "KeyValue[k].first = " << KeyValue[k].first << endl;
      cout << "KeyValue[k].second = " << KeyValue[k].second << endl;

      set_bbox_corners(k,lower_left_longitude,lower_left_latitude,
                       upper_right_longitude,upper_right_latitude);

      if (KeyValue[k].first=="ThreatDistance")
      {
         SAM_range=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="ThreatUnits")
      {
         range_units=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities

   if (range_units=="ft")
   {
      SAM_range *= meters_per_ft ;
   }
   else if (range_units=="nmi")
   {
      SAM_range *= meters_per_nmi ;   
   }

   cout << "lower_left_long = " << lower_left_longitude
        << " lower_left_latitude = " << lower_left_latitude << endl;
   cout << "upper_right_long = " << upper_right_longitude
        << " upper_right_lat = " << upper_right_latitude << endl;
   cout << "range_units = " << range_units << endl;
   cout << "SAM_range in meters = " << SAM_range << endl;

   threatmap_Movie_ptr=Aircraft_MODELSGROUP_ptr->visualize_SAM_threatmap(
      SAM_range,lower_left_longitude,upper_right_longitude,
      lower_left_latitude,upper_right_latitude);
   cout << "threatmap_Movie_ptr = " << threatmap_Movie_ptr << endl;

   if (PointCloudsGroup_ptr != NULL)
   {
      int curr_index=PointCloudsGroup_ptr->get_curr_colorbar_index();
      if (curr_index != 2)
      {
         PointCloudsGroup_ptr->set_curr_colorbar_index(2);
         PointCloudsGroup_ptr->update_ColorbarHUD();
      }
   }
}

// ---------------------------------------------------------------------
// Member function alter_threat_map_alpha()

void LOSServer::change_threat_map_alpha()
{
   cout << "inside LOSServer::change_threat_map_alpha()" << endl;

   if (threatmap_Movie_ptr == NULL) return;

   double alpha=1.0;

   int n_args=KeyValue.size();
//   cout << "n_args = " << n_args << endl;
   for (int k=0; k<n_args; k++)
   {
      cout << "KeyValue[k].first = " << KeyValue[k].first << endl;
      cout << "KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="ThreatMapOpacity")
      {
         alpha=0.01*stringfunc::string_to_number(KeyValue[k].second);
      }
   } // loop over index k labeling KeyValue key possibilities

   cout << "threat_map_alpha = " << alpha << endl;
   threatmap_Movie_ptr->set_alpha(alpha);
}

// ---------------------------------------------------------------------
// Member function clear_threat_map()

void LOSServer::clear_threat_map()
{
//   cout << "inside LOSServer::clear_threat_map()" << endl;

   Aircraft_MODELSGROUP_ptr->clear_SAM_threatmap();
   string progress_type="threat map";
   Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
      broadcast_clear_progress(progress_type);

   if (PointCloudsGroup_ptr != NULL)
   {
      cout << "curr_colorbar_index = "
           << PointCloudsGroup_ptr->get_curr_colorbar_index() << endl;
      cout << "prev_colorbar_index = "
           << PointCloudsGroup_ptr->get_prev_colorbar_index() << endl;
      if (PointCloudsGroup_ptr->get_curr_colorbar_index() == 2)
      {
         PointCloudsGroup_ptr->set_curr_colorbar_index(
            PointCloudsGroup_ptr->get_prev_colorbar_index());
      }
      PointCloudsGroup_ptr->update_ColorbarHUD();
   }
}

// ---------------------------------------------------------------------
// Member function compute_visibility_flowfield()

QByteArray LOSServer::compute_visibility_flowfield()
{
//   cout << "****************************************************" << endl;
   cout << "inside LOSServer::compute_visibility_flowfield()" << endl;

   int n_args=KeyValue.size();
//   cout << "n_args = " << n_args << endl;

   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k << endl;
//      cout << "KeyValue[k].first = " << KeyValue[k].first << endl;
//      cout << "KeyValue[k].second = " << KeyValue[k].second << endl;
      
      set_bbox_corners(
         k,skymap_longitude_lo,skymap_latitude_lo,
         skymap_longitude_hi,skymap_latitude_hi);
   }
//   cout << "skymap_longitude_lo = " << skymap_longitude_lo << endl;
//   cout << "skymap_longitude_hi = " << skymap_longitude_hi << endl;
//   cout << "skymap_latitude_lo = " << skymap_latitude_lo << endl;
//   cout << "skymap_latitude_hi = " << skymap_latitude_hi << endl;

   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k << endl;
//      cout << "KeyValue[k].first = " << KeyValue[k].first << endl;
//      cout << "KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="ComputeFlowField" &&
          KeyValue[k].second=="True")
      {
         string error_message=check_aircraft_altitude_wrt_ground(
            skymap_longitude_lo,skymap_latitude_lo,
            skymap_longitude_hi,skymap_latitude_hi);
         if (error_message.size() > 0)
         {
            return generate_error_JSON_response(error_message);
         }

// Need to check if no ground targets have been set.  If so, alert
// should appear within thin client...

         if (Aircraft_MODELSGROUP_ptr->generate_target_visibility_skymaps(
            skymap_longitude_lo,skymap_latitude_lo,
            skymap_longitude_hi,skymap_latitude_hi))
         {

            return generate_JSON_response_to_flowfield_computation();
         }
         else
         {
            return QByteArray("");
         }
      }
      else if (KeyValue[k].first=="ToggleFlowField")
      {
         int nodemask_value=1;
//         if (KeyValue[k].second=="false" ||
//             KeyValue[k].second=="False") nodemask_value=0;
         if (KeyValue[k].second=="False") nodemask_value=0;
         Aircraft_MODELSGROUP_ptr->get_ArrowsGroup_ptr()->
            set_OSGgroup_nodemask(nodemask_value);
         return QByteArray("");
      }
      else if (KeyValue[k].first=="ClearFlowField" &&
               KeyValue[k].second=="True")
      {
         Aircraft_MODELSGROUP_ptr->clear_visibility_skymaps();
         return QByteArray("");
      }
   } // loop over index k labeling KeyValue key possibilities
}

// ---------------------------------------------------------------------
// Member function check_aircraft_altitude_wrt_ground() takes in the
// lower left and upper right geocoordinates for the flowfield skymap.
// It retrieves ground altitudes over a lattice within the skymap
// region.  If the aircraft's altitude is less than the ground's at
// any lattice point, this method returns an error message.  

string LOSServer::check_aircraft_altitude_wrt_ground(
   double skymap_longitude_lo,double skymap_latitude_lo,
   double skymap_longitude_hi,double skymap_latitude_hi)
{
   cout << "inside LOSServer::check_aircraft_altitude_wrt_ground()" << endl;

// Check aircraft altitude within requested skymap region:

   geopoint lower_left_corner(skymap_longitude_lo,skymap_latitude_lo);
   geopoint upper_right_corner(skymap_longitude_hi,skymap_latitude_hi);
   double xlo=lower_left_corner.get_UTM_easting();
   double xhi=upper_right_corner.get_UTM_easting();
   double ylo=lower_left_corner.get_UTM_northing();
   double yhi=upper_right_corner.get_UTM_northing();
//   cout << "xlo = " << xlo << " xhi = " << xhi << endl;
//   cout << "ylo = " << ylo << " yhi = " << yhi << endl;

   int n_xbins=15;
   int n_ybins=15;
   double dx=(xhi-xlo)/(n_xbins-1);
   double dy=(yhi-ylo)/(n_ybins-1);
//   cout << "dx = " << dx << " dy = " << dy << endl;

   PointCloud* PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);

   double ground_zmax=NEGATIVEINFINITY;
   for (int px=0; px<n_xbins; px++)
   {
      double x=xlo+px*dx;
      for (int py=0; py<n_ybins; py++)
      {
         double y=ylo+py*dy;
         double curr_z;
         if (PointCloud_ptr->find_Z_given_XY(x,y,curr_z))
         {
//            cout << "x = " << x << " y = " << y << " z = " << curr_z << endl;
            ground_zmax=basic_math::max(ground_zmax,curr_z);
         }
      } // loop over py index
   } // loop over px index
   cout << "ground_zmax = " << ground_zmax << endl;

   string return_message="";
   if (ground_zmax > Aircraft_MODELSGROUP_ptr->get_aircraft_altitude())
   {
      return_message=
         "Cannot compute skymap! Aircraft altitude is less than maximum ground altitude ("+stringfunc::number_to_string(ground_zmax/meters_per_ft,0)+
         " ft above sea level).";
   }
   cout << "return_message = " << return_message << endl;

   return return_message;
}

// ---------------------------------------------------------------------
QByteArray LOSServer::generate_JSON_response_to_flowfield_computation()
{
//   cout << "Inside LOSServer::generate_JSON_response_to_flowfield_computation()" << endl;
   string json_string = "{ \"type\": \"VectorField\", \n";

   ArrowsGroup* ArrowsGroup_ptr=Aircraft_MODELSGROUP_ptr->
      get_ArrowsGroup_ptr();
   int n_arrows=ArrowsGroup_ptr->get_n_Graphicals();

   vector<double> arrow_base_longitude,arrow_base_latitude,
      arrow_tip_longitude,arrow_tip_latitude;
   for (int a=0; a<n_arrows; a++)
   {
      Arrow* Arrow_ptr=ArrowsGroup_ptr->get_Arrow_ptr(a);

      threevector V_base=Arrow_ptr->get_V_base();      
      threevector V_tip=Arrow_ptr->get_V_tip();
      double magnitude=(V_tip-V_base).magnitude();
      if (magnitude < 1) continue;
      
      geopoint base_geopoint(
         northern_hemisphere_flag,specified_UTM_zonenumber,
         V_base.get(0),V_base.get(1));
      arrow_base_longitude.push_back(base_geopoint.get_longitude());
      arrow_base_latitude.push_back(base_geopoint.get_latitude());

      geopoint tip_geopoint(
         northern_hemisphere_flag,specified_UTM_zonenumber,
         V_tip.get(0),V_tip.get(1));
      arrow_tip_longitude.push_back(tip_geopoint.get_longitude());
      arrow_tip_latitude.push_back(tip_geopoint.get_latitude());
   }
   
   json_string += " \"arrow_base_lons_lats\": [ ";   
   for (int a=0; a<arrow_base_longitude.size(); a++)
   {
      json_string += " ["+stringfunc::number_to_string(
         arrow_base_longitude[a],10)
         +","+stringfunc::number_to_string(arrow_base_latitude[a],10)+"]";
      if (a<arrow_base_longitude.size()-1)
      {
         json_string += ",";
      }
   }
   json_string += " ], \n";

   json_string += " \"arrow_tip_lons_lats\": [ ";   
   for (int a=0; a<arrow_tip_longitude.size(); a++)
   {
      json_string += " ["+stringfunc::number_to_string(
         arrow_tip_longitude[a],10)
         +","+stringfunc::number_to_string(arrow_tip_latitude[a],10)+"]";
      if (a<arrow_tip_longitude.size()-1)
      {
         json_string += ",";
      }
   }
   json_string += " ] \n";

   json_string += "} \n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Flight path member functions
// ==========================================================================

// Member function set_circular_path()

QByteArray LOSServer::set_circular_path()
{
//   cout << "inside LOSServer::set_circular_path()" << endl;

   int n_args=KeyValue.size();

   bool longitude_entry_found_flag=false;
   bool latitude_entry_found_flag=false;
   bool mgrs_entry_found_flag=false;
   bool radius_entry_found_flag=false;
   bool empty_fields_entry_flag=true;
   double center_longitude,center_latitude,orbit_radius;
   string radius_units,flightpath_dir;
   for (int k=0; k<n_args; k++)
   {

// Check whether every numeric input value is empty.  If so, purge any
// existing flight path:

      if (KeyValue[k].first != "RadiusUnits" && 
          KeyValue[k].second.size() > 0) empty_fields_entry_flag=false;

      if (KeyValue[k].first=="CenterLongitude")
      {
         if (KeyValue[k].second.size() > 0) longitude_entry_found_flag=true;
         center_longitude=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="CenterLatitude")
      {
         if (KeyValue[k].second.size() > 0) latitude_entry_found_flag=true;
         center_latitude=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="CenterMGRS")
      {
         if (KeyValue[k].second.size() > 0) mgrs_entry_found_flag=true;
         string centerMGRS_str=KeyValue[k].second;
         geofunc::MGRS_to_long_lat(centerMGRS_str,center_longitude,
                                   center_latitude);
      }
      else if (KeyValue[k].first=="OrbitRadius")
      {
         if (KeyValue[k].second.size() > 0) radius_entry_found_flag=true;
         orbit_radius=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="RadiusUnits")
      {
         radius_units=KeyValue[k].second;
      }
      else if (KeyValue[k].first=="FlightPathDir")
      {
         flightpath_dir=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "longitude_entry_found_flag = " << longitude_entry_found_flag
//        << endl;
//   cout << "latitude_entry_found_flag = " << latitude_entry_found_flag
//        << endl;
//   cout << "mgrs_entry_found_flag = " << mgrs_entry_found_flag
//        << endl;
//   cout << "radius_entry_found_flag = " << radius_entry_found_flag
//        << endl;
   cout << "flightpath_dir = " << flightpath_dir << endl;

   if ((longitude_entry_found_flag && latitude_entry_found_flag 
        && radius_entry_found_flag) || 
       (mgrs_entry_found_flag && radius_entry_found_flag) ||
       empty_fields_entry_flag)
   {
   }
   else
   {
      cout << "Insufficient number of entries found" << endl;
      return false;
   }

   cout << "empty_fields_entry_flag = " << empty_fields_entry_flag << endl;
   cout << "center_longitude = " << center_longitude
        << " center_latitude = " << center_latitude << endl;
//   outputfunc::enter_continue_char();

   if (radius_units=="kms")
   {
      orbit_radius *= meters_per_km;
   }
   else if (radius_units=="nmi")
   {
      orbit_radius *= meters_per_nmi;
   }
   cout << "orbit_radius/meter = " << orbit_radius << endl;

   int flightpath_sgn=-1;
   if (flightpath_dir=="Counterclockwise") flightpath_sgn=1;
   cout << "flightpath_sgn = " << flightpath_sgn << endl;

   clear_flightpath();

   string error_message="";
   if (!empty_fields_entry_flag)
   {
      PolyLine* circular_PolyLine_ptr=
         Aircraft_MODELSGROUP_ptr->generate_circular_PolyLine_Path(
            center_longitude,center_latitude,orbit_radius,flightpath_sgn);
      error_message=check_flightpath_wrt_ground(
         circular_PolyLine_ptr->get_polyline_ptr());
   }

   if (error_message.size() > 0)
   {
      clear_flightpath();
      return generate_error_JSON_response(error_message);
   }
   else
   {
      return generate_JSON_response_to_circular_flightpath_entry(
         center_longitude,center_latitude,orbit_radius);
   }
}

// ---------------------------------------------------------------------
// Member function set_general_flightpath() parses post data coming
// from Michael Yee's OpenMaps thin client which transmits a
// LINESTRING containing longitude,latitude waypoint coordinates.
// This method uses the input data to generate a new PolyLine in
// *FlightPolyLinesGroup_ptr.  It returns the flight path length in
// meters.

double LOSServer::set_general_flightpath(
   const QByteArray& postData,string& error_msg)
{
   cout << "inside LOSServer::set_general_flightpath()" << endl;

   QUrl tmp_url;
   QString tmp_qstring=tmp_url.fromPercentEncoding(postData);
   string post_data=tmp_qstring.toStdString();
   cout << "post_data = " << post_data << endl;
   cout << "post_data.size() = " << post_data.size() << endl;

//      QUrl decoded_url=tmp_url.fromEncoded(postData);
//      string post_data2_str=decoded_url.toString().toStdString();
//      cout << "post_data2_str = " << post_data2_str << endl;

//      QString post_qstring(postData);
//      QUrl url3(post_qstring);
//      string url3_str=url3.toString().toStdString();
//      cout << "post data 3 = " << url3_str << endl;

//      string XML_content=stringfunc::XML_content_between_tags(
//         post_data,"polyline");
//      cout << "Initial XML content = " << XML_content << endl;
//      XML_content=stringfunc::find_and_replace_char(XML_content,"+"," ");
//      cout << "Simplified XML content = " << XML_content << endl;
//      process_flight_path_queries(XML_content,doc,response);

//post_data = geometry=LINESTRING(22.171630859375+44.89990234375,23.797607421875+44.888916015625,24.874267578125+46.3720703125,23.709716796875+47.00927734375,22.8857421875+46.70166015625)

// FAKE FAKE:  Mon Jul 23, 2012 at 4:18 pm

// Flight paths over northern Iraq supplied by Andrew Silberfarb in
// July 2012:

// path1

/*
   post_data = "geometry=LINESTRING(45.025+34.983,45.015+34.979,45.001+34.973,44.989+34.969,44.979+34.965,44.969+34.961,44.957+34.957,44.947+34.954,44.937+34.950,44.925+34.945,44.914+34.940,44.903+34.937,44.893+34.933,44.882+34.929,44.872+34.925,44.861+34.922,44.850+34.917,44.838+34.912,44.827+34.908,44.817+34.905,44.806+34.901,44.797+34.897,44.786+34.893,44.772+34.888,44.761+34.884,44.751+34.880,44.740+34.876,44.729+34.872,44.717+34.867,44.707+34.864,44.697+34.860,44.686+34.856,44.677+34.852,44.666+34.849,44.652+34.843,44.640+34.839,44.630+34.835,44.620+34.831,44.610+34.828,44.598+34.823,44.588+34.820,44.574+34.814,44.563+34.810,44.552+34.806,44.542+34.802,44.531+34.798,44.519+34.794,44.509+34.790,44.495+34.785,44.484+34.780,44.473+34.777,44.463+34.773,44.452+34.769,44.443+34.765,44.432+34.761,44.418+34.756,44.407+34.752,44.397+34.748,44.386+34.744,44.376+34.741,44.364+34.736,44.353+34.732,44.339+34.727,44.328+34.723,44.318+34.719,44.307+34.715,44.296+34.711,44.284+34.706,44.273+34.703,44.263+34.699,44.253+34.695,44.240+34.690,44.229+34.686,44.215+34.681,44.204+34.677,44.197+34.675)";
*/


/*
// path 2

post_data = "geometry=LINESTRING(44.171+34.701,44.182+34.705,44.195+34.711,44.207+34.716,44.217+34.720,44.227+34.723,44.238+34.728,44.249+34.732,44.259+34.736,44.269+34.740,44.280+34.744,44.292+34.749,44.303+34.753,44.318+34.759,44.329+34.764,44.340+34.768,44.350+34.772,44.360+34.776,44.372+34.780,44.382+34.784,44.392+34.788,44.403+34.793,44.415+34.797,44.425+34.801,44.439+34.807,44.450+34.811,44.460+34.815,44.470+34.819,44.482+34.823,44.493+34.828,44.504+34.832,44.519+34.838,44.530+34.843,44.540+34.847,44.551+34.850,44.562+34.855,44.574+34.860,44.584+34.863,44.596+34.868,44.607+34.873,44.617+34.876,44.627+34.880,44.639+34.885,44.650+34.889,44.660+34.893,44.674+34.899,44.685+34.903,44.696+34.907,44.706+34.911,44.717+34.915,44.728+34.919,44.739+34.923,44.750+34.929,44.762+34.933,44.772+34.938,44.782+34.942,44.792+34.945,44.803+34.949,44.814+34.954,44.827+34.959,44.839+34.963,44.849+34.968,44.859+34.972,44.871+34.976,44.882+34.979,44.892+34.983,44.902+34.988,44.914+34.992,44.924+34.997,44.934+35.001,44.946+35.005,44.957+35.009,44.967+35.013,44.977+35.017,44.989+35.022,44.999+35.026,45.010+35.030,45.021+35.034,45.027+35.037)";
*/


/*
// path 3

post_data = "geometry=LINESTRING(45.198+35.203,45.196+35.214,45.193+35.225,45.190+35.240,45.188+35.251,45.186+35.261,45.184+35.272,45.182+35.282,45.179+35.296,45.177+35.307,45.175+35.318,45.173+35.328,45.171+35.338,45.168+35.351,45.166+35.361,45.164+35.372,45.162+35.383,45.160+35.393,45.157+35.407,45.155+35.418,45.153+35.428,45.151+35.439,45.145+35.448,45.143+35.462,45.141+35.472,45.139+35.483,45.137+35.493,45.135+35.504,45.132+35.518,45.130+35.529,45.128+35.539,45.125+35.553,45.123+35.564,45.121+35.574,45.119+35.589,45.117+35.599,45.115+35.610,45.112+35.624,45.110+35.635,45.108+35.645,45.106+35.659,45.104+35.670,45.102+35.681,45.099+35.695,45.097+35.706,45.095+35.716,45.093+35.729,45.091+35.741,45.089+35.751,45.087+35.761,45.085+35.772,45.083+35.784,45.081+35.795,45.079+35.805,45.077+35.818,45.075+35.829,45.074+35.839,45.072+35.850,45.069+35.864,45.067+35.875,45.065+35.889,45.063+35.899)";
*/


/*
// path 4

post_data = "geometry=LINESTRING(45.137+35.882,45.140+35.872,45.142+35.861,45.144+35.850,45.146+35.840,45.148+35.826,45.150+35.815,45.152+35.804,45.155+35.790,45.157+35.779,45.159+35.769,45.161+35.755,45.163+35.744,45.165+35.734,45.168+35.719,45.170+35.709,45.171+35.698,45.174+35.684,45.176+35.673,45.178+35.663,45.180+35.648,45.182+35.638,45.184+35.627,45.187+35.613,45.189+35.602,45.190+35.592,45.193+35.578,45.195+35.567,45.197+35.557,45.199+35.542,45.201+35.532,45.203+35.521,45.206+35.507,45.207+35.496,45.209+35.486,45.212+35.471,45.214+35.461,45.215+35.450,45.218+35.436,45.220+35.425,45.222+35.415,45.224+35.401,45.226+35.390,45.228+35.379,45.229+35.369,45.231+35.360,45.233+35.346,45.235+35.336,45.237+35.326,45.239+35.314,45.241+35.304,45.243+35.289,45.245+35.279,45.247+35.268,45.248+35.258,45.250+35.249,45.252+35.235,45.254+35.224,45.255+35.214,45.257+35.203,45.258+35.195)";

*/

   string linestring=stringfunc::substring_between_substrings(
      post_data,"(",")");
   linestring=stringfunc::find_and_replace_char(linestring,"(","");
   linestring=stringfunc::find_and_replace_char(linestring,")","");
//   cout << "linestring = " << linestring << endl;
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      linestring,",");

   bool valid_flightpath_flag=true;
   vector<threevector> V;
//   cout << "substrings.size() = " << substrings.size() << endl;

   for (int i=0; i<int(substrings.size()); i++)
   {
//      cout << substrings[i] << endl;
      vector<double> longlat=stringfunc::string_to_numbers(
         substrings[i],"+");
//      cout << "longlat[0] = " << longlat[0] << " longlat[1] = " << longlat[1]
//           << endl;

      if (latlong_bbox_ptr->point_inside(longlat[0],longlat[1]))
      {

// FAKE FAKE Mon Jul 23, 2012: For Andrew Silberfarb's July 2012 demo,
// we translate the input flight path by 2 degrees northward:

         const double delta_lat=0;
//         const double delta_lat=2;

         geopoint curr_flightpath_geopoint(
            longlat[0],longlat[1]+delta_lat,0,specified_UTM_zonenumber);
//      cout << "flight path geopoint = " << curr_flightpath_geopoint << endl;
         V.push_back(curr_flightpath_geopoint.get_UTM_posn());
         flightpath_geopoints.push_back(curr_flightpath_geopoint);
      }
      else
      {
         valid_flightpath_flag=false;
      }
   } // loop over index i labeling client input substrings

   if (!valid_flightpath_flag)
   {
      error_msg="Ignoring waypoints lying outside bounding box defined by ";
      error_msg += 
         stringfunc::number_to_string(latlong_bbox_ptr->get_ymin())
         +" degs < latitude < "
         +stringfunc::number_to_string(latlong_bbox_ptr->get_ymax())
         +" degs and "
         +stringfunc::number_to_string(latlong_bbox_ptr->get_xmin())
         +" degs < longitude < "
         +stringfunc::number_to_string(latlong_bbox_ptr->get_xmax())
         +" degs.  ";
   }

// We cannot form a genuine flightpath unless V.size() >= 2!

//   cout << "V.size() = " << V.size() << endl;
//   if (V.size() <= 1 && substrings.size() > 0)
   if (V.size() <= 1)
   {
      if (substrings.size() > 0)
      {
         error_msg += "Need at least 2 valid waypoints to form flight path.";
      }
      return 0;
   }

   return generate_flightpath_from_waypoints(V,error_msg);
}

// ---------------------------------------------------------------------
// Member function generate_flightpath_from_waypoints() takes in a set
// of waypoints.  It instantiates a PolyLine corresponding to the
// flight path and returns its length.

double LOSServer::generate_flightpath_from_waypoints(
   const vector<threevector>& V,string& error_msg)
{
   cout << "inside LOSServer::generate_flightpath_from_waypoints()" << endl;

   clear_flightpath();

   PolyLine* curr_PolyLine_ptr=
      Flight_PolyLinesGroup_ptr->generate_new_PolyLine(V.back(),V);
   Flight_PolyLinesGroup_ptr->reset_PolyLine_altitudes(curr_PolyLine_ptr);

//   cout << "*curr_PolyLine_ptr = " << *curr_PolyLine_ptr << endl;
//   cout << "Flight_PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << Flight_PolyLinesGroup_ptr->get_n_Graphicals() << endl;

   curr_PolyLine_ptr->add_vertex_points(
      V,curr_PolyLine_ptr->get_selected_color());
   curr_PolyLine_ptr->set_entry_finished_flag(true);

   polyline* polyline_ptr=curr_PolyLine_ptr->get_or_set_polyline_ptr();

   double flightpath_length=0;
   error_msg=check_flightpath_wrt_ground(polyline_ptr);
   if (error_msg.size()==0)
   {
      flightpath_length=polyline_ptr->compute_total_length();
   }
   else
   {
      clear_flightpath();
   }
   
   return flightpath_length;
}

// ---------------------------------------------------------------------
// Member function check_flightpath_wrt_ground() retrieves ground
// altitudes at multiple points along the XY locations specified by
// input flight path *flightpath_ptr.  If the aircraft's altitude is
// less than the ground's at any sample point, this method returns an
// error message.

string LOSServer::check_flightpath_wrt_ground(polyline* flightpath_ptr)
{
//   cout << "inside LOSServer::check_flightpath_wrt_ground()" << endl;

   PointCloud* PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);

   double ground_zmax=NEGATIVEINFINITY;
   int nbins=100;
   for (int i=0; i<nbins; i++)
   {
      double curr_frac=double(i)/double(nbins-1);
      threevector curr_waypoint=flightpath_ptr->edge_point(curr_frac);

      double ground_z;
      if (PointCloud_ptr->find_Z_given_XY(
         curr_waypoint.get(0),curr_waypoint.get(1),ground_z))
      {
//         cout << "x = " << curr_waypoint.get(0) 
//              << " y = " << curr_waypoint.get(1) 
//              << " ground_z = " << ground_z << endl;
         ground_zmax=basic_math::max(ground_zmax,ground_z);
      }
   } // loop over index i
//   cout << "ground_zmax = " << ground_zmax << endl;

   string return_message="";
   if (ground_zmax > Aircraft_MODELSGROUP_ptr->get_aircraft_altitude())
   {
      return_message="Aircraft altitude (";
      return_message += stringfunc::number_to_string(
         Aircraft_MODELSGROUP_ptr->get_aircraft_altitude()/meters_per_ft,0)+
         " ft above sea level) is less than maximum ground altitude (";
      
       return_message += stringfunc::number_to_string(
          ground_zmax/meters_per_ft,0)+
         " ft above sea level) along flight path!";
   }
//   cout << "At end of check_flightpath_wrt_ground, return_message = " 
//        << return_message << endl;

   return return_message;
}

// ---------------------------------------------------------------------
// Member function clear_flightpath()

void LOSServer::clear_flightpath()
{
//   cout << "inside LOSServer::clear_flightpath()" << endl;

   PolyLinesGroup* Path_PolyLinesGroup_ptr=Aircraft_MODELSGROUP_ptr->
      get_Path_PolyLinesGroup_ptr();
//   cout << "Path_PolyLinesGroup_ptr = " << Path_PolyLinesGroup_ptr << endl;
     
   if (Path_PolyLinesGroup_ptr != NULL)
   {
      Aircraft_MODELSGROUP_ptr->purge_UAV_MODELS_and_tracks();
   } 
   AnimationController_ptr->set_curr_framenumber(0);
}

// ---------------------------------------------------------------------
// Member function parse_clock_parameters()

QByteArray LOSServer::parse_clock_parameters(string& response_msg)
{
//   cout << "inside LOSServer::parse_clock_parameters()" << endl;

   int n_args=KeyValue.size();

   bool timestep_entry_found_flag=false;
   string clock_units;
   double increment,start_time_in_secs_since_Jan_1970=-1;
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="ClockUnits")
      {
         clock_units=KeyValue[k].second;
      }
      else if (KeyValue[k].first=="ClockIncrement")
      {
         if (KeyValue[k].second.size() > 0) timestep_entry_found_flag=true;
         increment=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (KeyValue[k].first=="StartTime")
      {
         start_time_in_secs_since_Jan_1970=
            stringfunc::string_to_number(KeyValue[k].second);
         if (start_time_in_secs_since_Jan_1970 > 0)
         {
            double duration=Operations_ptr->get_master_world_stop_time()-
               Operations_ptr->get_master_world_start_time();
            Operations_ptr->set_master_world_start_time(
               start_time_in_secs_since_Jan_1970);
            Operations_ptr->set_master_world_stop_time(
               start_time_in_secs_since_Jan_1970+duration);
         }
      }
   } // loop over index k labeling KeyValue key possibilities

   if (!timestep_entry_found_flag)
   {
      response_msg="Insufficient number of parameters entered";
      cout << response_msg << endl;
      return false;
   }

//   cout << "clock_units = " << clock_units << endl;
   if (clock_units=="hrs")
   {
      increment *= 3600;
   }
   else if (clock_units=="mins")
   {
      increment *= 60;
   }

   return set_clock_parameters(increment,response_msg);
}

// ---------------------------------------------------------------------
// Member function set_clock_parameters()

QByteArray LOSServer::set_clock_parameters(
   double increment,string& response_msg)
{
   cout << "inside LOSServer::set_clock_parameters()" << endl;

   MODEL* LiMIT_MODEL_ptr=Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);
   PolyLine* Flight_PolyLine_ptr=static_cast<PolyLine*>(
      Flight_PolyLinesGroup_ptr->get_most_recently_added_Graphical_ptr());

// Recompute flight time based upon latest best estimate for flight
// pathlength divided by flight speed:

   if (Flight_PolyLine_ptr != NULL)
   {
      polyline* flight_polyline_ptr=
         Flight_PolyLine_ptr->get_or_set_polyline_ptr();
//      cout << "n polyline vertices = " << flight_polyline_ptr->get_n_vertices()
//           << endl;
      double flightpath_length=flight_polyline_ptr->get_total_length();
//      cout << "total length = " << flightpath_length << endl;
      double flight_time_in_secs=
         flightpath_length/Aircraft_MODELSGROUP_ptr->get_LiMIT_speed();// secs
//      cout << "flight time in secs = " << flight_time_in_secs << endl;
      Operations_ptr->set_master_world_stop_time(
         Operations_ptr->get_master_world_start_time()+
         flight_time_in_secs);
   }
   
//   cout << "Time step = " << increment << " secs" << endl;
//   cout << "world start time = " 
//        << Operations_ptr->get_master_world_start_time() << endl;
//   cout << "world stop time = " 
//        << Operations_ptr->get_master_world_stop_time() << endl;
//   cout << "world stop - start time = "
//        << Operations_ptr->get_master_world_stop_time()-
//      Operations_ptr->get_master_world_start_time() << endl;


   Operations_ptr->set_delta_master_world_time_step_per_master_frame(
      increment); // secs

   if (reset_AnimationController_start_stop_times_flag)
   {
      AnimationController_ptr->set_world_time_params(
         Operations_ptr->get_master_world_start_time(),
         Operations_ptr->get_master_world_stop_time(),
         Operations_ptr->get_delta_master_world_time_step_per_master_frame());
//   cout << "n_frames = " << AnimationController_ptr->get_nframes()
//        << endl;
   }
   
   if (LiMIT_MODEL_ptr != NULL && Flight_PolyLine_ptr != NULL)
   {
      track* track_ptr=Aircraft_MODELSGROUP_ptr->update_UAV_track(
         AnimationController_ptr->get_first_framenumber(),
         Flight_PolyLine_ptr,LiMIT_MODEL_ptr);

// Broadcast LiMIT flight path sample points to LOST thin client:

      bool compute_posns_with_distinct_dirs_flag=false;
      Aircraft_MODELSGROUP_ptr->broadcast_add_track_to_GoogleEarth_channel(
         track_ptr,compute_posns_with_distinct_dirs_flag);

// Reset constant scale for LiMIT model for all new time values:

      threevector scale;
      LiMIT_MODEL_ptr->get_scale(
         Aircraft_MODELSGROUP_ptr->get_initial_t(),
         Aircraft_MODELSGROUP_ptr->get_passnumber(),scale);
      Aircraft_MODELSGROUP_ptr->set_constant_scale(
         LiMIT_MODEL_ptr,scale.get(0));
   }

   reset_clock_to_starting_time();
   clear_raytracing_results();

//   response_msg="Clock time step updated";
   response_msg="";
   return generate_JSON_response_to_clock_parameters_request(response_msg);
}

// ==========================================================================
// Raytracing member functions
// ==========================================================================

// Member function set_raytracing_controls()

bool LOSServer::set_raytracing_controls()
{
   cout << "inside LOSServer::set_raytracing_controls()" << endl;

   int n_args=KeyValue.size();
//   cout << "n_args = " << n_args << endl;

   bool start_raytracing_flag=false;
   bool display_cumulative_results_flag=false;
   bool clear_raytracing_results_flag=false;

   MODEL* MODEL_ptr=Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      
      if (KeyValue[k].first=="StartRaytracing")
      {
         string truefalse_label=KeyValue[k].second;
         if (truefalse_label=="True")
         {
            start_raytracing_flag=true;
         }

         MODEL_ptr->set_raytrace_occluded_ground_regions_flag(true);
         string progress_type="raytracing";
         Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
            broadcast_progress(0.02,progress_type);
      }
      else if (KeyValue[k].first=="CumulativeResults")
      {
         string truefalse_label=KeyValue[k].second;
         if (truefalse_label=="True")
         {
            display_cumulative_results_flag=true;
         }
      }
      else if (KeyValue[k].first=="TraceOnlyTargets")
      {
         string truefalse_label=KeyValue[k].second;
         if (truefalse_label=="True")
         {
            double increment=60;	// secs
            if (TilesGroup_ptr->get_ladar_height_data_flag())
            {
               increment=5;	// secs
            }

            string response_msg;
            set_clock_parameters(increment,response_msg);

// Recall that set_clock_params() calls clear_raytracing_results()
// which resets raytrace_ROI_flag to true.  So we need to set
// raytrace_ROI_flag to false afterwards:

            MODEL_ptr->set_raytrace_ROI_flag(false);
         }
      }
      else if (KeyValue[k].first=="ClearRayTracingResults")
      {
         string truefalse_label=KeyValue[k].second;
         if (truefalse_label=="True")
         {
            clear_raytracing_results_flag=true;
         }
      }
   } // loop over index k labeling KeyValue key possibilities

   cout << "start_raytracing_flag = " << start_raytracing_flag << endl;
   cout << "display_cumulative_results_flag = " 
        << display_cumulative_results_flag << endl;
   cout << "clear_raytracing_results_flag = " 
        << clear_raytracing_results_flag << endl;

   if (start_raytracing_flag)
   {
      reset_clock_to_starting_time();
      play_movie();
   }
   else if (display_cumulative_results_flag)
   {
      AnimationController_ptr->setState(AnimationController::PAUSE);
      hide_aircraft_MODEL_and_OBSFRUSTA();
      Aircraft_MODELSGROUP_ptr->display_average_LOS_results();
      display_ImageNumberHUD(false);
   }
   else if (clear_raytracing_results_flag)
   {
      clear_raytracing_results();
      display_ImageNumberHUD(true);
   }
}

// ---------------------------------------------------------------------
// Member function clear_raytracing_results()

void LOSServer::clear_raytracing_results()
{
//   cout << "inside LOSServer::clear_raytracing_results()" << endl;
   Aircraft_MODELSGROUP_ptr->clear_raytracing_results();
}

// ---------------------------------------------------------------------
// Member function get_target_occlusion_fractions() takes in either
// integer ID for some ground target or an input frame number passed
// as arguments via a GET request.  In the first case, this method
// constructs a JSON string containing an output array with as many
// entries as there are time steps in the current flight path.  The
// array values equal -1 (target lies outside FOV), 0 (target is
// occluded) or 1 (target is visible).  In the second case, this
// method constructs a JSON string containing an output array with as
// many entries as there are ground targets.  The array values again
// equal -1, 0 or 1.

QByteArray LOSServer::get_target_occlusion_fractions()
{
//   cout << "inside LOSServer::get_target_occlusion_fractions()" << endl;

//   if (Aircraft_MODELSGROUP_ptr==NULL) return;
//   if (GroundTarget_SignPostsGroup_ptr==NULL) return;

   int target_ID=-1;
   int frame_number=-1;
   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      cout << "k = " << k << " KeyValue[k].first = "
           << KeyValue[k].first << endl;
      
      if (KeyValue[k].first=="TargetID")
      {
         target_ID=stringfunc::string_to_number(KeyValue[k].second);
         hide_aircraft_MODEL_and_OBSFRUSTA();
      }
      else if (KeyValue[k].first=="FrameNumber")
      {
         frame_number=stringfunc::string_to_number(KeyValue[k].second);
         unhide_aircraft_MODEL_and_OBSFRUSTA();
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout << "target_ID = " << target_ID << endl;
//   cout << "frame_number = " << frame_number << endl;

   string json_string = "{ \n";
   if (frame_number==-1)
   {
      json_string += "  \"Visibility_over_time\": [";

      SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
         get_ID_labeled_SignPost_ptr(target_ID);
   
      if (SignPost_ptr != NULL)
      {
         for (int f=Aircraft_MODELSGROUP_ptr->get_first_framenumber();
              f <= Aircraft_MODELSGROUP_ptr->get_last_framenumber(); f++)
         {
            double curr_t(f);
            int curr_visibility=
               Aircraft_MODELSGROUP_ptr->get_ground_target_visibility(
                  curr_t,target_ID);
            string curr_visibility_str=
               +" visibility: "+stringfunc::number_to_string(curr_visibility);
//            cout << curr_visibility_str << endl;
            
            json_string += stringfunc::number_to_string(curr_visibility);

            if (f < Aircraft_MODELSGROUP_ptr->get_last_framenumber())
            {
               json_string += ", ";
            }
         } // loop over index f labeling frames
      } // SignPost_ptr != NULL conditional

      MODEL* LiMIT_MODEL_ptr=Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);
      Aircraft_MODELSGROUP_ptr->get_LineSegmentsGroup_ptr()->
         destroy_all_LineSegments();

// FAKE FAKE:  Thurs Jan 14, 2010 at 10:16 am

// Experiment with NOT resetting clock to starting time in order to
// stay in synch with thin client...

//      reset_clock_to_starting_time();

      Aircraft_MODELSGROUP_ptr->set_ray_type(
         LOSMODELSGROUP::multi_air_to_single_ground);

      PointCloudsGroup_ptr->reset_cloud_coloring_to_zeroth_height_colormap();

// Recall Aircraft_MODELSGROUP_ptr has TWO OSGsubPATs.  The zeroth
// OSGsubPAT holds the LineSegmentsGroup which stores air-to-ground
// colored rays.  The first OSGsubPAT holds aircraft OBSFRUSTA.  When
// multi-air-to-single-ground rays are displayed, we want to view the
// former and hide the latter:

      Aircraft_MODELSGROUP_ptr->set_OSGsubPAT_nodemask(0,1);
      Aircraft_MODELSGROUP_ptr->get_LineSegmentsGroup_ptr()->
         set_OSGgroup_nodemask(1);
      display_ImageNumberHUD(false);

      Aircraft_MODELSGROUP_ptr->draw_colored_multi_air_to_single_ground_rays(
         target_ID,LiMIT_MODEL_ptr);
   }
   else if (target_ID==-1)
   {
      Aircraft_MODELSGROUP_ptr->set_ray_type(
         LOSMODELSGROUP::single_air_to_multi_ground);

// Purge all ray LineSegments inside *Aircraft_MODELSGROUP_ptr when
// ray type is switched from multi_air_to_single_ground back to
// single_air_to_multi_ground:

      Aircraft_MODELSGROUP_ptr->
         purge_multi_air_to_single_ground_linesegments();

      json_string += "  \"Visibility_over_target\": [";
      int n_targets=GroundTarget_SignPostsGroup_ptr->get_n_Graphicals();
      for (int t=0; t<n_targets; t++)
      {
         SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
            get_SignPost_ptr(t);
         
         double curr_t(frame_number);
         int curr_visibility=
            Aircraft_MODELSGROUP_ptr->get_ground_target_visibility(
               curr_t,SignPost_ptr->get_ID());
//         cout << "visibility = " << curr_visibility << endl;

         string curr_visibility_str=
            +" visibility: "+stringfunc::number_to_string(curr_visibility);
//         cout << curr_visibility_str << endl;
         
         json_string += stringfunc::number_to_string(curr_visibility);
         if (t < n_targets-1)
         {
            json_string += ", ";
         }
      } // loop over index t labeling ground targets
   }
   
   json_string += "] } \n";
//   cout << "json_string = " << json_string << endl;
   
   return QByteArray(json_string.c_str());   
}

// ---------------------------------------------------------------------
// Member function export_avg_occlusion_files() takes in a ground
// ROI bbox. It instantiates R, G & B twoDarrays to hold color
// information corresponding to time-averaged visibility within ROI
// ground cells.  These color twoDarrays are output via a
// raster_parser object to a geotif file which has valid bbox header
// information. 

QByteArray LOSServer::export_avg_occlusion_files()
{
   cout << "inside LOSServer::generate_avg_occlusion_files()" << endl;
   
   int n_args=KeyValue.size();

   string filename_stem;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k << endl;
//      cout << " KeyValue[k].first = " << KeyValue[k].first 
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      if (KeyValue[k].first=="filename_stem")
      {
         filename_stem=KeyValue[k].second;
      }
   }
   string basename=filefunc::getbasename(filename_stem);
   string dirname=filefunc::getdirname(filename_stem);
   cout << "basename = " << basename << endl;
   cout << "dirname = " << dirname << endl;
//   outputfunc::enter_continue_char();

   string Desktop_dirname=
      "~/Desktop/LOST_inputs_and_outputs/outputs/imagery/"+dirname;
   filefunc::dircreate(Desktop_dirname);
   cout << "Desktop_dirname = " << Desktop_dirname << endl;

   string output_geotif_filename=Desktop_dirname+basename+".tif";
   string output_nitf_filename=Desktop_dirname+basename+".nitf";
   cout << "Desktop_geotif_filename = " << output_geotif_filename << endl;
   cout << "Desktop_nitf_filename = " << output_nitf_filename << endl;

   Aircraft_MODELSGROUP_ptr->export_average_occlusion_files(
      dirname,basename,output_geotif_filename,output_nitf_filename);

   return generate_JSON_response_to_export_avg_occlusion_files(
      output_geotif_filename,output_nitf_filename);
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_export_avg_occlusion_files() 
// returns a JSON string to the thin client which contains the full
// path for a geotif containing average ROI occlusion fractions.
// The message also contains LineString written
// in GEOJSON format corresponding to the image's georegistered bbox.
 
QByteArray LOSServer::generate_JSON_response_to_export_avg_occlusion_files(
   string geotif_filename,string nitf_filename)
{
   cout << "Inside LOSServer::generate_JSON_response_to_export_avg_occlusion_files()" 
        << endl;

   string json_string = "{ \"GeotifFilename\": ";
   json_string += "\""+geotif_filename+"\" , \n";
   json_string += "\"NitfFilename\": ";
   json_string += "\""+nitf_filename+"\" , \n";
   string msg="Geotif & NITF files exported to outputs/imagery subfolder of LOST_inputs_and_outputs on Desktop";
   json_string += "\"message\": ";
   json_string += "\""+msg+"\" \n";
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function set_automated_path()

QByteArray LOSServer::set_automated_path()
{
//   cout << "inside LOSServer::set_automated_path()" << endl;

   double automated_path_progress=0.02;
   string progress_type="automated path";
   Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
      broadcast_progress(automated_path_progress,progress_type);

   string pathtype;
   for (int k=0; k<KeyValue.size(); k++)
   {

// Check whether every numeric input value is empty.  If so, purge any
// existing flight path:

      if (KeyValue[k].first=="pathType")
      {
         pathtype=KeyValue[k].second;
         cout << "pathtype = " << pathtype << endl;
      }
   }

   clear_flightpath();

   int n_ground_targets=GroundTarget_SignPostsGroup_ptr->get_n_Graphicals();
   double score=0;
   if (pathtype=="Circle")
   {
      double center_longitude,center_latitude,orbit_radius;
      score=Aircraft_MODELSGROUP_ptr->
         compute_optimal_clockwise_circular_flightpath(
            center_longitude,center_latitude,orbit_radius);

// Renormalize score so that it ranges from 0 to 1:

      score /= n_ground_targets;

      int flightpath_sgn=-1;	// clockwise
      cout << "center_lon = " << center_longitude
           << " center_lat = " << center_latitude
           << " radius = " << orbit_radius << endl;
      cout << "score = " << score << endl;

      PolyLine* circular_PolyLine_ptr=
         Aircraft_MODELSGROUP_ptr->generate_circular_PolyLine_Path(
            center_longitude,center_latitude,orbit_radius,
            flightpath_sgn);
      Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
         broadcast_finished_progress(progress_type);
      return generate_JSON_response_to_circular_flightpath_entry(
         center_longitude,center_latitude,orbit_radius,score);
   }
   else if (pathtype=="Ellipse")
   {
      double center_longitude,center_latitude,orbit_radius;
      double ellipse_a,ellipse_b,ellipse_phi;

      score=Aircraft_MODELSGROUP_ptr->
         compute_optimal_clockwise_ellipse_params(
            center_longitude,center_latitude,
            ellipse_a,ellipse_b,ellipse_phi);

// Renormalize score so that it ranges from 0 to 1:

      score /= n_ground_targets;

      cout << "center_lon = " << center_longitude
           << " center_lat = " << center_latitude
           << " a = " << ellipse_a 
           << " b = " << ellipse_b
           << " phi = " << ellipse_phi*180/PI
           << endl;
      cout << "score = " << score << endl;

      double pathlength=Aircraft_MODELSGROUP_ptr->
         generate_ellipse_flightpath(center_longitude,center_latitude,
         ellipse_a,ellipse_b,ellipse_phi);
      Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
         broadcast_finished_progress(progress_type);
      return generate_JSON_response_to_automated_flightpath_entry(score);
   }
   else if (pathtype=="Line")
   {
      twovector r1,r2;
      score=Aircraft_MODELSGROUP_ptr->compute_optimal_linesegment_params(
         r1,r2);
      double pathlength=Aircraft_MODELSGROUP_ptr->
         generate_linesegment_flightpath(r1,r2);

// Renormalize score so that it ranges from 0 to 1:

      score /= (sqrt(pathlength)*n_ground_targets);

      Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
         broadcast_finished_progress(progress_type);
      return generate_JSON_response_to_automated_flightpath_entry(score);
   }
}

// ---------------------------------------------------------------------
// Member function
// generate_JSON_response_to_automated_flightpath_entry() returns a
// JSON string to Michael Yee's thin client which contains a
// LineString written in GEOJSON form corresponding to the circular
// flight path.

QByteArray LOSServer::generate_JSON_response_to_automated_flightpath_entry(
   double score)
{
//   cout << "Inside LOSServer::generate_JSON_response_to_automated_flightpath()"
//        << endl;
   
   PolyLinesGroup* Path_PolyLinesGroup_ptr=Aircraft_MODELSGROUP_ptr->
      get_Path_PolyLinesGroup_ptr();

   double flightpath_length=0;
   vector<geopoint> G;
   if (Path_PolyLinesGroup_ptr->get_n_Graphicals() > 0)
   {
      PolyLine* PolyLine_ptr=Path_PolyLinesGroup_ptr->
         get_PolyLine_ptr(0);
      polyline* polyline_ptr=PolyLine_ptr->get_or_set_polyline_ptr();
      flightpath_length=polyline_ptr->compute_total_length();

      vector<threevector> V;
      int n_vertices=polyline_ptr->get_n_vertices();
      for (int n=0; n<n_vertices; n++)
      {
         V.push_back(polyline_ptr->get_vertex(n));
      }

      for (int n=0; n<V.size(); n++)
      {
         G.push_back(
            geopoint(northern_hemisphere_flag,specified_UTM_zonenumber,
                     V[n].get(0),V[n].get(1)));
      }
   } // n_PolyLines > 0 conditional
   
   string json_string = "{ \"type\": \"LineString\", \n";

   bool include_final_comma_flag=true;
   json_string += generate_JSON_flight_distance_and_time_string(
      flightpath_length,include_final_comma_flag);

//   json_string += " \"score\": \""+stringfunc::number_to_string(score)
//      +"\" , \n";
   json_string += " \"coordinates\": [ ";

   for (int n=0; n<G.size(); n++)
   {
      json_string += 
         " ["+stringfunc::number_to_string(G[n].get_longitude())
         +","+stringfunc::number_to_string(G[n].get_latitude())+" ]";
      if (n < G.size()-1) json_string += ",";
   } // loop over index n labeling geopoints within STL vector G
   
   json_string += " ] \n }";
   json_string += "\n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function cancel_calculation() 

void LOSServer::cancel_calculation()
{
   cout << "inside LOSServer::cancel_calculation()" << endl;
   
   string banner="CANCELING CALCULATION";
   outputfunc::write_big_banner(banner);

   MODEL* MODEL_ptr=Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);

// If raytracing is underway, we effectively terminate it and clear
// out any existing raytracing results:

   if (MODEL_ptr->get_raytrace_occluded_ground_regions_flag())
   {
      MODEL_ptr->set_raytrace_occluded_ground_regions_flag(false);
      clear_raytracing_results();
      display_ImageNumberHUD(true);
   }
}

// ---------------------------------------------------------------------
// As of 1/14/12, we believe that member function
// toggle_groundpath_entry() is an orphan.  It is called from
// TOC11.html.  But it doesn't appear to do anything useful...

void LOSServer::toggle_groundpath_entry()
{
   cout << "inside LOSServer::toggle_groundpath_entry()" << endl;
}

// ==========================================================================
// PYXIS server member functions
// ==========================================================================

// Member function compute_ROI_visibility() prepares and begins a LOST
// server computation of a ROI's instantaneous and time-averaged
// visibilities.  A call to this method ends in
// *Aircraft_MODELSGROUP_ptr exporting geotif and nitf files
// containing time-averaged occlusion results.

void LOSServer::compute_ROI_visibility()
{
   cout << "inside LOSServer::compute_ROI_visibility()" << endl;
   string output_file_basename="TimeAveragedVisibility";

   int n_args=KeyValue.size();
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="TimeAveragedROIVisibilityFilename")
      {
         output_file_basename=KeyValue[k].second;
      }
   } // loop over index k labeling KeyValue key possibilities

   MODEL* MODEL_ptr=Aircraft_MODELSGROUP_ptr->get_MODEL_ptr(0);
   cout << "MODEL_ptr = " << MODEL_ptr << endl;
   
   MODEL_ptr->set_raytrace_occluded_ground_regions_flag(true);
   string progress_type="raytracing";
   Aircraft_MODELSGROUP_ptr->get_viewer_Messenger_ptr()->
      broadcast_progress(0.02,progress_type);

   Aircraft_MODELSGROUP_ptr->set_PYXIS_server_flag(true);

   Aircraft_MODELSGROUP_ptr->set_PYXIS_output_subdir(
      "~/Desktop/LOST_inputs_and_outputs/outputs/imagery/");
   Aircraft_MODELSGROUP_ptr->set_PYXIS_output_basename(output_file_basename);

   reset_clock_to_starting_time();
   play_movie();
}

// ---------------------------------------------------------------------
// Member function start_automatic_analysis() parses key-value pairs
// read in via an http POST request which we assume originated from a
// machine rather than an interactive human user.  It extracts ground
// ROI, sensor model, temporal sampling and flight path parameters
// from the key-value pairs.  Once all input parameters have been
// input, ROI raytracing is initiated.

void LOSServer::start_automatic_analysis()
{
   cout << "inside LOSServer::start_automatic_analysis()" << endl;

   int n_args=KeyValue.size();

   double ROI_LowerLeftLongitude,ROI_LowerLeftLatitude;
   double ROI_UpperRightLongitude,ROI_UpperRightLatitude;
   double sensor_MinRange,sensor_MaxRange,sensor_HorizFOV;
   string lobe_pattern="Right side";
   double clock_increment;
   int n_waypoints;
   vector<fourvector> flight_waypoints;

   for (int k=0; k<n_args; k++)
   {
      cout << "k = " << k << endl;
      cout << " KeyValue[k].first = " << KeyValue[k].first 
           << " KeyValue[k].second = " << KeyValue[k].second << endl;

      string curr_key=KeyValue[k].first;
      string separator_chars=":";
      vector<string> key_substrings=
         stringfunc::decompose_string_into_substrings(
            curr_key,separator_chars);
      int n_key_substrings=key_substrings.size();

      if (curr_key=="ROI_LowerLeftLongitude")
      {
         ROI_LowerLeftLongitude=
            stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (curr_key=="ROI_LowerLeftLatitude")
      {
         ROI_LowerLeftLatitude=
            stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (curr_key=="ROI_UpperRightLongitude")
      {
         ROI_UpperRightLongitude=
            stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (curr_key=="ROI_UpperRightLatitude")
      {
         ROI_UpperRightLatitude=
            stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (curr_key=="Sensor_MinRange")
      {
         sensor_MinRange=stringfunc::string_to_number(KeyValue[k].second)*
            meters_per_km;
      }
      else if (curr_key=="Sensor_MaxRange")
      {
         sensor_MaxRange=stringfunc::string_to_number(KeyValue[k].second)*
            meters_per_km;
      }
      else if (curr_key=="Sensor_HorizontalFOV")
      {
         sensor_HorizFOV=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (curr_key=="Sensor_LobePattern")
      {
         if (KeyValue[k].second=="Left")
         {
            lobe_pattern="Left side";
         }
         else if (KeyValue[k].second=="Both")
         {
            lobe_pattern="Double sided";
         }
      }
      else if (curr_key=="ClockIncrement")
      {
         clock_increment=stringfunc::string_to_number(KeyValue[k].second);
      }
      else if (curr_key=="N_waypoints")
      {
         n_waypoints=stringfunc::string_to_number(KeyValue[k].second);
         fourvector zero_fourvector(0,0,0,0);
         for (int w=0; w<n_waypoints; w++)
         {
            flight_waypoints.push_back(zero_fourvector);
         }
      }
      else if (n_key_substrings==2)
      {
         int waypoint_ID=stringfunc::string_to_integer(key_substrings[1]);
         double value=stringfunc::string_to_number(KeyValue[k].second);
//         cout << "key_substrings[0] = "
//              << key_substrings[0]
//              << " waypoint_ID = " << waypoint_ID << endl;

         if (key_substrings[0]=="Lon")
         {
            flight_waypoints[waypoint_ID].put(0,value);
         }
         else if (key_substrings[0]=="Lat")
         {
            flight_waypoints[waypoint_ID].put(1,value);
         }
         else if (key_substrings[0]=="Alt")
         {
            flight_waypoints[waypoint_ID].put(2,value);
         }
         else if (key_substrings[0]=="Time")
         {
            flight_waypoints[waypoint_ID].put(3,value);
         }
      }

   } // loop over index k labeling extracted key-value pairs

// Set ground Region of Interest:

   Aircraft_MODELSGROUP_ptr->set_ground_bbox(
      ROI_LowerLeftLongitude,ROI_LowerLeftLatitude,
      ROI_UpperRightLongitude,ROI_UpperRightLatitude);
   cout << "ROI_bbox = "
        << *(Aircraft_MODELSGROUP_ptr->get_ground_bbox_ptr()) 
        << endl;

// Set sensor parameters:

   set_sensor_parameters(
      sensor_MinRange,sensor_MaxRange,sensor_HorizFOV,lobe_pattern);

   cout << "sensor: minrange = " << sensor_MinRange
        << " maxrange = " << sensor_MaxRange
        << " horiz FOV = " << sensor_HorizFOV 
        << " lobe_pattern = " << lobe_pattern
        << endl;

// Set clock increment:

   clock_increment *= 60;	// secs
   cout << "clock_increment = " << clock_increment 
        << " n_waypoints = " << n_waypoints << endl;

   string response_msg;
   set_clock_parameters(clock_increment,response_msg);

// Set aircraft parameters:

   tracks_group* tracks_group_ptr=new tracks_group();
   track* track_ptr=tracks_group_ptr->generate_new_track();
   for (int w=0; w<n_waypoints; w++)
   {
      fourvector curr_waypoint=flight_waypoints[w];
      double curr_longitude=curr_waypoint.get(0);
      double curr_latitude=curr_waypoint.get(1);
      double curr_altitude=curr_waypoint.get(2);
      double curr_time=curr_waypoint.get(3);
      geopoint curr_geopt(curr_longitude,curr_latitude,curr_altitude);
      track_ptr->set_XYZ_coords(curr_time,curr_geopt.get_UTM_posn());
   }

   cout << "*track_ptr = " << *track_ptr << endl;
   cout << "Track length = " << track_ptr->total_length() << endl;

   const double meters_per_sec_to_miles_per_hour=2.227;
   double avg_speed=track_ptr->avg_speed()/meters_per_sec_to_miles_per_hour;
   		// avg speed in meters/sec
   cout << "Average speed = " << avg_speed << endl;
   double median_altitude=track_ptr->median_altitude();
   cout << "Median altitude = " << median_altitude << endl;

   Aircraft_MODELSGROUP_ptr->set_aircraft_altitude(median_altitude);
   Aircraft_MODELSGROUP_ptr->set_LiMIT_speed(avg_speed);

// Generate flight PolyLine:

   string error_msg;
   double flightpath_length=
      generate_flightpath_from_waypoints(track_ptr->get_posns(),error_msg);
   cout << "flightpath_length = " << flightpath_length << endl;

   tracks_group_ptr->destroy_all_tracks();
   delete tracks_group_ptr;

   Aircraft_MODELSGROUP_ptr->set_PYXIS_server_flag(true);
   Aircraft_MODELSGROUP_ptr->set_PYXIS_output_subdir(
      "~/Desktop/LOST_inputs_and_outputs/outputs/imagery/");
   string output_file_basename="TimeAveragedVisibility";
   Aircraft_MODELSGROUP_ptr->set_PYXIS_output_basename(output_file_basename);
}
