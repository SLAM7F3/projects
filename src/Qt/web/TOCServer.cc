// ==========================================================================
// TOCSERVER class file
// ==========================================================================
// Last updated on 9/13/10; 9/18/10; 9/20/10; 12/10/10
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "Qt/web/TOCServer.h"
#include "color/colorfuncs.h"
#include "postgres/databasefuncs.h"
#include "astro_geo/geopoint.h"
#include "geometry/homography.h"
#include "track/mover_funcs.h"
#include "templates/mytemplates.h"

#include "video/G99VideoDisplay.h"
#include "image/raster_parser.h"
#include "image/TwoDarray.h"
#include "video/videofuncs.h"


using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void TOCServer::allocate_member_objects()
{
}		       

void TOCServer::initialize_member_objects()
{
   clock.current_local_time_and_UTC();
   viewer_messenger_ptr=NULL;
   metadata_messenger_ptr=NULL;
   gis_database_ptr=NULL;
}

TOCServer::TOCServer(
   string host_IP_address,qint16 port, QObject* parent) :
   BasicServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
TOCServer::~TOCServer()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function get_sensor_type() contains hard-wired sensor ID
// information from the Sensors table in the TOC database.  Given an
// input sensor_ID, this method returns its general type.  We wrote
// this little utility member function in order to distinguish betweeb GPS
// and imagery data products.

TOCServer::SensorType TOCServer::get_sensor_type(
   int sensor_ID) const
{
//   cout << "inside TOCServer::get_sensor_type()" << endl;

/*
Sensors table in TOC database as of 9/11/10:

"1";"Microsoft webcam"
"2";"Logitech webcam"
"3";"D7 pancam"
"4";"Hockeypuck GPS device"
"5";"MIDG GPS/INS"
"9";"Canon powershot"
"10";"GPS camera"
"11";"Flip videocam"
"12";"Quad GPS/IMU"
"13";"Axis camera"
"14";"Garmin GPS"
"15";"Droid GPS"
"16";"Droid camera"
"17";"Pan-tilt camera"
"18";"FLIR"
*/

   if (sensor_ID==1 || sensor_ID==2 || sensor_ID==3 || 
       sensor_ID==9 || sensor_ID==11 || sensor_ID==13 ||
       sensor_ID==16 || sensor_ID==17 || sensor_ID==18)
   {
      return TOCServer::IMAGE;
   }
   else if (sensor_ID==4 || sensor_ID==5 || sensor_ID==12 || sensor_ID==14 ||
	    sensor_ID==15)
   {
      return TOCServer::GPS;
   }
   else if (sensor_ID==10)
   {
      return TOCServer::GPS_AND_IMAGE;
   }

}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray TOCServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside TOCServer:get() method" << endl;

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

   if (URL_path=="/Geoalign_Image/")
   {
      double min_lon,max_lon,min_lat,max_lat;
      string output_image_path=geoalign_image(min_lon,max_lon,min_lat,max_lat);
      return generate_JSON_response_to_image_geoalignment(
         output_image_path,min_lon,max_lon,min_lat,max_lat);
   }

   else if (URL_path=="/Update_Fieldtest_Mission_Platform_Selector_Dropdowns/")
   {
      return update_fieldtest_mission_platform_selector_dropdowns();
   }
   else if (URL_path=="/Update_GPS_Sensor_Dropdown/")
   {
      return update_sensor_dropdown(TOCServer::GPS);
   }
   else if (URL_path=="/Update_Image_Sensor_Dropdown/")
   {
      return update_sensor_dropdown(TOCServer::IMAGE);
   }
   else if (URL_path=="/Pick_Mission/" || URL_path=="/Pick_Fieldtest/")
   {
      pick_mission();
   }
   else if (URL_path=="/Display_GPS_track/")
   {
      return display_GPS_track();
   }

   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray TOCServer::post(const QUrl& url, const QByteArray& postData,
                             QHttpResponseHeader& responseHeader)
{
//   cout << "inside TOCServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   TOCServer::get(doc,response,url,URL_path,responseHeader);

   cout << "URL_path = " << URL_path << endl;

   return doc.toByteArray();
}

// ==========================================================================
// Image geoalignment member functions
// ==========================================================================

// Member function geoalign_image() extracts tiepoint pair coordinates
// from a world map and corresponding image.  It solves for the
// inverse homography which maps the image onto the map.  This method
// then computes and returns the lon-lat geocoordinates for the
// image's lower left and upper right corners.  

string TOCServer::geoalign_image(
   double& min_lon,double& max_lon,double& min_lat,double& max_lat)
{
   cout << "inside TOCServer::geoalign_image()" << endl;

   int n_args=KeyValue.size();
   string imagePath;
   vector<int> X_index,Y_index,label_index;
   vector<double> X,Y;
   vector<string> label;

   cout.precision(12);
   for (int k=0; k<n_args; k++)
   {
      string key=KeyValue[k].first;
      string value=KeyValue[k].second;
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;
      
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         key,"123456789");
//      cout << "substrings.size() = " << substrings.size() << endl;

      string index_str=KeyValue[k].first.substr(
         substrings[0].size(),key.size()-substrings[0].size());
      cout << "index_str = " << index_str << endl;

      int index=-1;
      if (index_str.size() > 0)
      {
         index=stringfunc::string_to_number(index_str);
         cout << "index = " << index << endl;
      }

      if (key=="imagePath")
      {
         imagePath=value;
      }
      else if (substrings[0]=="X")
      {
         X_index.push_back(index);
         X.push_back(stringfunc::string_to_number(value));
      }
      else if (substrings[0]=="Y")
      {
         Y_index.push_back(index);
         Y.push_back(stringfunc::string_to_number(value));
      }
      else if (substrings[0]=="Label")
      {
         label_index.push_back(index);
         label.push_back(value);
      }
   } // loop over index k labeling key-value pairs

// Sort X,Y and labels by their indices:

   templatefunc::Quicksort(X_index,X);
   templatefunc::Quicksort(Y_index,Y);
   templatefunc::Quicksort(label_index,label);

   vector<twovector> XY,UV;
   for (int i=0; i<X_index.size()/2; i++)
   {
      cout << "i = " << i << " X_index = " << X_index[i] << " X = " << X[i]
           << " Y_index = " << Y_index[i] << " Y = " << Y[i] << endl;
      XY.push_back(twovector(X[i],Y[i]));
      UV.push_back(twovector(X[i+X.size()/2],Y[i+Y.size()/2]));
      cout << "i = " << i 
           << " XY = " << XY.back() 
           << " UV = " << UV.back() 
           << " label = " << label[i]
           << endl;
   }

   if (AnimationController_ptr==NULL)
   {
      cout << "Error in TOCServer::geoalign_image()" << endl;
      cout << "AC_ptr = NULL" << endl;
      exit(-1);
   }
   
// We assume imagePath has a form like

// 	http://localhost:8080/geoalign/data/2010-07-22/HAFB_24bit.jpg

// Replace http://localhost:8080/ with /usr/local/apache-tomcat/webapps/
   
   cout << "imagePath = " << imagePath << endl;
   string separator="8080";
   string input_imagePath=stringfunc::erase_chars_before_first_substring(
      imagePath,separator);
   input_imagePath=input_imagePath.substr(5,input_imagePath.length()-5);
   string prefix="/usr/local/apache-tomcat/webapps/";
   input_imagePath=prefix+input_imagePath;
   cout << "input_imagePath = " << input_imagePath << endl;

   G99VideoDisplay video(input_imagePath,AnimationController_ptr);

   double Umin=video.get_minU();
   double Umax=video.get_maxU();
   double Vmin=video.get_minV();
   double Vmax=video.get_maxV();

   double image_width=video.getWidth();
   double image_height=video.getHeight();
   cout << "image_width = " << image_width
        << " image_height = " << image_height << endl;

   geopoint lower_left_image_corner(XY[0].get(0),XY[0].get(1));
   bool northern_hemisphere_flag=lower_left_image_corner.
      get_northern_hemisphere_flag();
   int UTM_zonenumber=lower_left_image_corner.get_UTM_zonenumber();

   cout << "northern_hemi flag = " << northern_hemisphere_flag << endl;
   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;

   homography H;
   twovector lower_left_XY,lower_right_XY,upper_left_XY,upper_right_XY;
   videofunc::compute_image_corner_world_coords(
      XY,UV,Umax,H,lower_left_XY,lower_right_XY,upper_right_XY,upper_left_XY);

//   cout << "lower_left_XY = " << lower_left_XY << endl;
//   cout << "lower_right_XY = " << lower_right_XY << endl;
//   cout << "upper_right_XY = " << upper_right_XY << endl;
//   cout << "upper_left_XY = " << upper_left_XY << endl;

   int new_xdim,new_ydim;
   double min_easting,max_easting,min_northing,max_northing;
   videofunc::compute_extremal_easting_northing(
      lower_left_XY,lower_right_XY,upper_right_XY,upper_left_XY,
      image_width,image_height,new_xdim,new_ydim,
      min_easting,max_easting,min_northing,max_northing);

   geopoint lower_left_bbox_corner(northern_hemisphere_flag,UTM_zonenumber,
	   min_easting,min_northing);
   geopoint upper_right_bbox_corner(northern_hemisphere_flag,UTM_zonenumber,
	   max_easting,max_northing);
   min_lon=lower_left_bbox_corner.get_longitude();
   min_lat=lower_left_bbox_corner.get_latitude();
   max_lon=upper_right_bbox_corner.get_longitude();
   max_lat=upper_right_bbox_corner.get_latitude();

   return export_geoaligned_image(
      new_xdim,new_ydim,min_easting,max_easting,min_northing,max_northing,
      input_imagePath,H,video);
}

// ---------------------------------------------------------------------
// Member function export_geoaligned_image() returns the full path to
// the exported geoaligned image.

string TOCServer::export_geoaligned_image(
   double new_xdim,double new_ydim,double min_easting,double max_easting,
   double min_northing,double max_northing,string input_imagePath,
   homography& H,const G99VideoDisplay& video)
{
   cout << "inside TOCServer::export_geoaligned_image()" << endl;

   int n_output_channels=4;
   texture_rectangle* new_texture_rectangle_ptr=new texture_rectangle(
      new_xdim,new_ydim,1,n_output_channels,AnimationController_ptr);

   twoDarray* RtwoDarray_ptr=new twoDarray(new_xdim,new_ydim);
   RtwoDarray_ptr->init_coord_system(
      min_easting,max_easting,min_northing,max_northing);
   cout << "*RtwoDarray_ptr = " << *RtwoDarray_ptr << endl;
   twoDarray* GtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);
   twoDarray* BtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);
   twoDarray* AtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);

   new_texture_rectangle_ptr->initialize_RGB_twoDarray_image(RtwoDarray_ptr);

   for (int px=0; px<RtwoDarray_ptr->get_mdim(); px++)
   {
//      cout << "RtwoDarray px = " << px << " mdim = " << new_xdim << endl;
      for (int py=0; py<RtwoDarray_ptr->get_ndim(); py++)
      { 
         double curr_x,curr_y;
         RtwoDarray_ptr->pixel_to_point(px,py,curr_x,curr_y);
         twovector XY(curr_x,curr_y);
         twovector UV=H.project_world_plane_to_image_plane(XY);
         double U=UV.get(0);
         double V=UV.get(1);
         
         int R,G,B;
         R=G=B=-1;
         video.get_texture_rectangle_ptr()->get_RGB_values(U,V,R,G,B);

// If no data exists at a pixel location within the output image, set
// its alpha value to zero:

         int A=255;
         if (R==-1)
         {
            R=G=B=0;
            A=0;
         }

// As of Sep 18, 2010, we experiment with setting alpha values for genuine
// black pixels (e.g. produced by AutopanoPro) to zero:

         if (R < 1 && G < 1 && B < 1)
         {
            A=0;
         }

         RtwoDarray_ptr->put(px,py,R);
         GtwoDarray_ptr->put(px,py,G);
         BtwoDarray_ptr->put(px,py,B);
         AtwoDarray_ptr->put(px,py,A);

         if (new_texture_rectangle_ptr->getNchannels()==3)
         {
            new_texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         }
         else if (new_texture_rectangle_ptr->getNchannels()==4)
         {
            new_texture_rectangle_ptr->set_pixel_RGBA_values(
               px,py,new_texture_rectangle_ptr->get_m_image_ptr(),R,G,B,A);
         }

      } // loop over py index
   } // loop over px index

// Write out geoaligned image to output imagePath which points to
// subdirectory related to input_imagePath:

// Recall PNG images support alpha channel while JPG images do NOT!

   string output_suffix="png";

   string basename=filefunc::getbasename(input_imagePath);
   string basename_prefix=stringfunc::prefix(basename);
   string dirname=filefunc::getdirname(input_imagePath);

   string output_dirname=stringfunc::erase_chars_after_first_substring(
      dirname,"data/");
   output_dirname += "geoaligned_images/";
   string output_basename="geoaligned_"+basename_prefix+"."+output_suffix;
   string output_imagePath=output_dirname+output_basename;
   cout << "output_imagePath = " << output_imagePath << endl;
   new_texture_rectangle_ptr->write_curr_frame(output_imagePath);

   string geotif_basename="geoaligned_"+basename_prefix+".tif";
   string geotif_imagePath=output_dirname+geotif_basename;
   cout << "geotif_imagePath = " << geotif_imagePath << endl;

// Generate output geotif file containing header metadata:

   raster_parser geotif_raster;

   int output_UTM_zonenumber=19;
   bool output_northern_hemisphere_flag=true;
   geotif_raster.write_colored_raster_data(
      geotif_imagePath,output_UTM_zonenumber,output_northern_hemisphere_flag,
      RtwoDarray_ptr,GtwoDarray_ptr,BtwoDarray_ptr,AtwoDarray_ptr);
   
   delete RtwoDarray_ptr;
   delete GtwoDarray_ptr;
   delete BtwoDarray_ptr;
   delete AtwoDarray_ptr;

   string tomcat_dirname=
      "http://127.0.0.1:8080/geoalign/data/geoaligned_images/";
   string tomcat_imagePath=tomcat_dirname+output_basename;

   return tomcat_imagePath;
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_image_geoalignment() returns a
// JSON string to the thin client which contains the full path for the
// orthorectified image.  The message also contains LineString written
// in GEOJSON format corresponding to the image's georegistered bbox.
 
QByteArray TOCServer::generate_JSON_response_to_image_geoalignment(
   string output_image_path,double min_easting, double max_easting,
   double min_northing,double max_northing)
{
   cout << "Inside TOCServer::generate_JSON_response_to_image_alignment()" 
        << endl;
   string json_string = "{ \"GeoalignedImageFilename\": ";
   json_string += "\""+output_image_path+"\" , \n";
   json_string += "\"type\": \"LineString\", \n";

   twovector lower_left_XY(min_easting,min_northing);
   twovector upper_right_XY(max_easting,max_northing);

   json_string += " \"coordinates\": [ ";   
   json_string += " ["+stringfunc::number_to_string(lower_left_XY.get(0))
      +","+stringfunc::number_to_string(lower_left_XY.get(1))+" ],";
   json_string += " ["+stringfunc::number_to_string(upper_right_XY.get(0))
      +","+stringfunc::number_to_string(upper_right_XY.get(1))+" ]";
   json_string += " ] \n";
   json_string += "} \n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Blue force tracking member functions
// ==========================================================================

// Member function display_GPS_track()

QByteArray TOCServer::display_GPS_track()
{
   cout << "inside TOCServer::display_GPS_track()" << endl;

   bool daylight_savings_flag=true;
   int selected_fieldtest_ID=-1;
   int selected_mission_ID=-1;
   int selected_platform_ID=-1;
   int selected_sensor_ID=-1;

//   cout << "n_keys = " << n_keys << endl;
   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k
//           << " key = " << key
//           << " value = " << value << endl;

      if (key=="FieldtestSelectorID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
      else if (key=="MissionSelectorID")
      {
         selected_mission_ID=stringfunc::string_to_number(value);
      }
      else if (key=="PlatformSelectorID")
      {
         selected_platform_ID=stringfunc::string_to_number(value);
      }
      else if (key=="SensorSelectorID")
      {
         selected_sensor_ID=stringfunc::string_to_number(value);
      }
   }

   cout << "selected_fieldtest_ID = " << selected_fieldtest_ID << endl;
   cout << "selected_mission_ID = " << selected_mission_ID << endl;
   cout << "selected_platform_ID = " << selected_platform_ID << endl;
   cout << "selected_sensor_ID = " << selected_sensor_ID << endl;

   colorfunc::Color track_color=
      colorfunc::get_platform_color(selected_platform_ID);
   colorfunc::RGB track_RGB=colorfunc::get_RGB_values(track_color);
   string RRGGBB_hex_str=colorfunc::RGB_to_RRGGBB_hex(track_RGB);

   cout << "RRGGBB_hex_str = " << RRGGBB_hex_str << endl;

   vector<int> trackpoint_ID,fix_quality,n_satellites;
   vector<double> elapsed_secs,horiz_dilution,longitude,latitude,altitude;
   vector<double> roll,pitch,yaw;

   mover_func::retrieve_track_points_metadata_from_database(
      daylight_savings_flag,gis_database_ptr,
      selected_mission_ID,selected_sensor_ID,
      trackpoint_ID,elapsed_secs,fix_quality,n_satellites, horiz_dilution,
      longitude,latitude,altitude,roll,pitch,yaw);

   vector<geopoint> GPS_trackpoints;
   for (int i=0; i<longitude.size(); i++)
   {
//      cout << "i = " << i << " lon = " << longitude[i]
//           << " lat = " << latitude[i] << endl;
      GPS_trackpoints.push_back(geopoint(longitude[i],latitude[i]));
   }
   cout << "GPS_trackpoints.size() = " << GPS_trackpoints.size() << endl;

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
   json_string += "\"n_trackpoints\": ";
   json_string += stringfunc::number_to_string(GPS_trackpoints.size())
      +" , \n";
   json_string += "\"TrackColor\": \""+RRGGBB_hex_str+"\" , \n";

   json_string += "\"type\": \"LineString\", \n";
   json_string += " \"coordinates\": [ ";   

   int n_trackpoints=GPS_trackpoints.size();
   for (int i=0; i<n_trackpoints; i++)
   {
      geopoint curr_geopoint(GPS_trackpoints[i]);
      json_string += " ["+stringfunc::number_to_string(
         curr_geopoint.get_longitude())+","+stringfunc::number_to_string(
            curr_geopoint.get_latitude())+" ]";
      if (i<n_trackpoints-1)
      {
         json_string += ",";
      }
   }

   json_string += " ] \n";
   json_string += "} \n";

//   cout << "Final json_string = " << json_string << endl;
   cout << "json_string.size() = " << json_string.size() << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// Data selection member functions
// ==========================================================================

// Member function update_fieldtest_mission_platform_selector_dropdowns()

QByteArray TOCServer::update_fieldtest_mission_platform_selector_dropdowns()
{
   cout << "inside TOCServer::update_fieldtest_mission_platform_selector_dropdowns()" << endl;

   vector<string> fieldtest_label,mission_label,platform_label;
   vector<int> fieldtest_ID,mission_ID,platform_ID;

// Retrieve correlated fieldtest, mission and platform label and ID
// information which are ordered chronologically (increasing):

   mover_func::retrieve_fieldtest_mission_platform_metadata_from_database(
      gis_database_ptr,fieldtest_label,fieldtest_ID,
      mission_label,mission_ID,platform_label,platform_ID);

// Store correlated metadata retrieved from database within STL maps:

   typedef map<int,vector<int> > ONE_ID_TO_MANY_ID_MAP;
   typedef map<int,int> ONE_ID_TO_ONE_ID_MAP;
   typedef map<int,string> ID_TO_LABEL_MAP;

   ONE_ID_TO_MANY_ID_MAP fieldtest_mission_IDs_map;
   ONE_ID_TO_ONE_ID_MAP mission_platform_IDs_map;
   ID_TO_LABEL_MAP fieldtest_label_map,mission_label_map,platform_label_map;
   
   for (int i=0; i<fieldtest_ID.size(); i++)
   {
      int curr_fieldtest_ID=fieldtest_ID[i];

      vector<int> curr_mission_IDs;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=fieldtest_mission_IDs_map.find(
         curr_fieldtest_ID);
      if (iter != fieldtest_mission_IDs_map.end())
      {
         curr_mission_IDs=iter->second;
      }
      curr_mission_IDs.push_back(mission_ID[i]);

      fieldtest_mission_IDs_map[curr_fieldtest_ID]=curr_mission_IDs;
      fieldtest_label_map[curr_fieldtest_ID]=fieldtest_label[i];
   } // loop over index i labeling fieldtest IDs
   
   for (int m=0; m<mission_ID.size(); m++)
   {
      int curr_mission_ID=mission_ID[m];
      int curr_platform_ID=platform_ID[m];
      mission_platform_IDs_map[curr_mission_ID]=curr_platform_ID;
      mission_label_map[curr_mission_ID]=mission_label[m];
   }

   for (int p=0; p<platform_ID.size(); p++)
   {
      int curr_platform_ID=platform_ID[p];
      platform_label_map[curr_platform_ID]=platform_label[p];
   } // loop over index i labeling platform IDs

   string json_string = "{ ";

// Write fieldtest_ID, fieldtest_label and corresponding mission IDs
// to output JSON string:

   json_string += " \"Fieldtest_ID_label_missionIDs\": [ ";   
   int n_fieldtests=fieldtest_ID.size();
   int prev_fieldtest_ID=-1;

   for (int i=0; i<n_fieldtests; i++)
   {
      int curr_fieldtest_ID=fieldtest_ID[i];

// Do not repeat fieldtest information within output JSON string more
// than once:

      if (curr_fieldtest_ID==prev_fieldtest_ID)
      {
         continue;
      }
      else
      {
         prev_fieldtest_ID=curr_fieldtest_ID;
      }

      ID_TO_LABEL_MAP::iterator label_iter=fieldtest_label_map.find(
         curr_fieldtest_ID);
      string curr_fieldtest_label=label_iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_fieldtest_ID)+","
         +"\""+curr_fieldtest_label+"\" ," ;

      json_string += " [ ";

      vector<int> curr_mission_IDs;
      ONE_ID_TO_MANY_ID_MAP::iterator iter=fieldtest_mission_IDs_map.find(
         curr_fieldtest_ID);
      if (iter != fieldtest_mission_IDs_map.end())
      {
         curr_mission_IDs=iter->second;
      }
      int n_missions=curr_mission_IDs.size();

      cout << "Fieldtest ID = " << curr_fieldtest_ID << endl;
      for (int j=0; j<n_missions; j++)
      {
         int curr_mission_ID=curr_mission_IDs[j];
         json_string += stringfunc::number_to_string(curr_mission_ID);
         if (j < n_missions-1)
         {
            json_string += ",";
         }
      }
      json_string += " ] ] ";
      if (i < n_fieldtests-1)
      {
         json_string += ",";
      }
   } // loop over index i labeling fieldtest ID in chronological order

   json_string += " ], \n";

// Write mission_ID vs mission_label to output JSON string:

   json_string += " \"Mission_IDs_labels\": [ ";   
   int n_missions=mission_ID.size();
   for (int i=0; i<n_missions; i++)
   {
      int curr_mission_ID=mission_ID[i];
      ID_TO_LABEL_MAP::iterator label_iter=mission_label_map.find(
         curr_mission_ID);
      string curr_mission_label=label_iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_mission_ID)+",\""
         +curr_mission_label+"\" ]";
      if (i < n_missions-1)
      {
         json_string += ",";
      }
   } // loop over inde xi labeling mission ID in chronological order
   json_string += " ], \n";

// Write mission_ID vs platform_ID to output JSON string:

   json_string += " \"Mission_ID_vs_Platform_ID\": [ ";   
   int mission_counter=0;
   for (ONE_ID_TO_ONE_ID_MAP::iterator iter=mission_platform_IDs_map.begin();
        iter != mission_platform_IDs_map.end(); ++iter)
   {
      int curr_mission_ID=iter->first;
      int curr_platform_ID=iter->second;
      cout << "Mission ID = " << curr_mission_ID 
           << " Platform ID = " << curr_platform_ID << endl;

      json_string += " [ "
         +stringfunc::number_to_string(curr_mission_ID)+","
         +stringfunc::number_to_string(curr_platform_ID);
      json_string += " ] ";
      if (mission_counter < n_missions-1)
      {
         json_string += ",";
      }
      mission_counter++;
   }
   json_string += " ], \n";
      
// Write platform_ID and platform_label to output JSON string:

   json_string += " \"Platform_ID_label\": [ ";   
   int platform_counter=0;
   int n_platforms=platform_label_map.size();
   for (ID_TO_LABEL_MAP::iterator iter=platform_label_map.begin();
        iter != platform_label_map.end(); ++iter)
   {
      int curr_platform_ID=iter->first;
      string curr_platform_label=iter->second;

      json_string += " [ "
         +stringfunc::number_to_string(curr_platform_ID)+","+
         "\""+curr_platform_label+"\" ] ";

      if (platform_counter < n_platforms-1)
      {
         json_string += ",";
      }
      platform_counter++;
   }
   json_string += " ] \n";

   json_string += "} \n";

   cout << "Final update_fieldtest_mission_platform_selector_dropdowns() json_string = " 
        << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function update_sensor_dropdown()

QByteArray TOCServer::update_sensor_dropdown(
	TOCServer::SensorType basic_sensor_type)
{
   cout << "inside TOCServer::update_sensor_dropdown()" << endl;
//   cout << "basic_sensor_type = " << basic_sensor_type << endl;

   vector<string> fieldtest_label,mission_label,platform_label,sensor_label;
   vector<int> fieldtest_ID,mission_ID,platform_ID,sensor_ID;

// Scan over entire table for GPS or PHOTOS.  Retrieve correlated sets
// of STL vectors containing fieldtest, mission, platform and sensor
// IDs for all independent *MISSIONS*:

   string database_table_name;
   if (basic_sensor_type==GPS)
   {
      database_table_name="track_points";
      mover_func::
         retrieve_correlated_fieldtest_mission_platform_sensor_metadata(
            gis_database_ptr,database_table_name,
	    fieldtest_label,fieldtest_ID,mission_label,mission_ID,
	    platform_label,platform_ID,sensor_label,sensor_ID);
   }
   else if (basic_sensor_type==IMAGE)
   {
      database_table_name="photos";

      mover_func::
         retrieve_correlated_fieldtest_mission_platform_sensor_metadata(
            gis_database_ptr,database_table_name,
	    fieldtest_label,fieldtest_ID,mission_label,mission_ID,
	    platform_label,platform_ID,sensor_label,sensor_ID);
   }
   else
   {
      cout << "Error in TOCServer::update_sensor_dropdown()" << endl;
      cout << "Basic_sensor_type = " << basic_sensor_type << endl;
      exit(-1);
   }
  
/* 
   cout << "fieldtest_label.size() = " << fieldtest_label.size() << endl;
   cout << "mission_label.size() = " << mission_label.size() << endl;
   cout << "platform_label.size() = " << platform_label.size() << endl;
   cout << "sensor_label.size() = " << sensor_label.size() << endl;

   cout << "Selected fieldtest ID = " << selected_fieldtest_ID << endl;
   cout << "Selected mission ID = " << selected_mission_ID << endl;
   cout << "Selected platform ID = " << selected_platform_ID << endl;
   cout << "Selected sensor ID = " << selected_sensor_ID << endl;

   for (int f=0; f<fieldtest_label.size(); f++)
   {
      cout << "f = " << f
           << " fieldtest = " << fieldtest_label[f]
           << " mission = " << mission_label[f]
           << " platform_label = " << platform_label[f]
           << " sensor = " << sensor_label[f] << endl;
      cout << " fieldtest ID = " << fieldtest_ID[f]
           << " mission ID = " << mission_ID[f]
           << " platform ID = " << platform_ID[f]
           << " sensor ID = " << sensor_ID[f] << endl;
   }
*/

// Determine possible sensor IDs and their corresponding labels as a
// function of basic_sensor_type input:

   int n_sensors=sensor_ID.size();
   int GPS_sensor_counter=0;
   int image_sensor_counter=0;

   vector<int> sensor_IDs_list;
   vector<string> sensor_labels_list;
   for (int f=0; f<n_sensors; f++)
   {
      SensorType curr_sensor_type=get_sensor_type(sensor_ID[f]);
//      cout << "f = " << f << " curr_sensor_type = " << curr_sensor_type
//           << endl;

      bool GPS_condition=
         ((curr_sensor_type==GPS || curr_sensor_type==GPS_AND_IMAGE) 
         && basic_sensor_type==GPS);
      bool IMAGE_condition=
         ((curr_sensor_type==IMAGE || curr_sensor_type==GPS_AND_IMAGE)
         && basic_sensor_type==IMAGE);
//      cout << "GPS_condition = " << GPS_condition << endl;

      if (GPS_condition || IMAGE_condition)
      {
         int curr_sensor_ID=sensor_ID[f];

// Do not add current sensor information to output sensor list if it
// has previously been included:

         bool sensor_previously_included_flag=false;
         for (int g=0; g<sensor_IDs_list.size(); g++)
         {
            if (curr_sensor_ID==sensor_IDs_list[g])
            {
               sensor_previously_included_flag=true;
            }
         }
         if (!sensor_previously_included_flag)
         {
            sensor_IDs_list.push_back(sensor_ID[f]);
            sensor_labels_list.push_back(sensor_label[f]);
         }
      } // GPS_condition || IMAGE_condition
   }

// Alphabetically sort sensor_labels_list while making corresponding
// changes to sensor_IDs_list:

   templatefunc::Quicksort(sensor_labels_list,sensor_IDs_list);

// Construct STL vector containing sensor ID as a function of mission ID:

   vector<pair<int,int> > sensorID_for_missionID;
   for (int m=0; m<mission_ID.size(); m++)
   {
      pair<int,int> p;
      p.first=mission_ID[m];
      p.second=sensor_ID[m];
      sensorID_for_missionID.push_back(p);
   }

// Write output JSON string:

   string json_string = "{ ";
   json_string += " \"Sensor_IDs_labels\": [ ";   
   for (int g=0; g<sensor_IDs_list.size(); g++)
   {
      json_string += " [ "
         +stringfunc::number_to_string(sensor_IDs_list[g])+",\""
         +sensor_labels_list[g]+"\" ]";
      if (g < sensor_IDs_list.size()-1)
      {
         json_string += ",";
      }
   }
   json_string += " ], \n";

   json_string += " \"MissionID_vs_SensorID\": [ ";   
   for (int g=0; g<sensorID_for_missionID.size(); g++)
   {
      pair<int,int> curr_p=sensorID_for_missionID[g];
      int curr_missionID=curr_p.first;
      int curr_sensorID=curr_p.second;
      json_string += " [ "
         +stringfunc::number_to_string(curr_missionID)+","
         +stringfunc::number_to_string(curr_sensorID)+" ]";
      if (g < sensorID_for_missionID.size()-1)
      {
         json_string += ",";
      }
   }
   json_string += " ] \n";

   json_string += "} \n";



   cout << "Final json_string = " << json_string << endl;
   cout << "at end of TOCServer::update_sensor_dropdown()" << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function pick_mission()

void TOCServer::pick_mission()
{
   cout << "inside TOCServer::pick_mission()" << endl;
   cout << "n_keys = " << n_keys << endl;

   int selected_fieldtest_ID=-1;
   int selected_mission_ID=-1;
   int selected_platform_ID=-1;
   int selected_sensor_ID=-1;

   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];

      cout << "k = " << k 
           << " Key = " << key
           << " Value = " << value << endl;

      if (key=="FieldtestDropdownID" || key=="FieldtestSelectorID" ||
	      key=="Fieldtest_SelectorID" || key=="FieldtestCalibratorID")
      {
         selected_fieldtest_ID=stringfunc::string_to_number(value);
      }
      else if (key=="MissionDropdownID" || key=="MissionSelectorID" ||
	      key=="MissionCalibratorID")
      {
         selected_mission_ID=stringfunc::string_to_number(value);
      }
      else if (key=="PlatformDropdownID" || key=="PlatformSelectorID" ||
	      key=="PlatformCalibratorID")
      {
         selected_platform_ID=stringfunc::string_to_number(value);
      }
      else if (key=="SensorDropdownID" || key=="SensorSelectorID" ||
	      key=="SensorCalibratorID")
      {
         selected_sensor_ID=stringfunc::string_to_number(value);
      }
   }

   cout << "selected_fieldtest_ID = " << selected_fieldtest_ID << endl;
   cout << "selected_mission_ID = " << selected_mission_ID << endl;
   cout << "selected_platform_ID = " << selected_platform_ID << endl;
   cout << "selected_sensor_ID = " << selected_sensor_ID << endl;
}
