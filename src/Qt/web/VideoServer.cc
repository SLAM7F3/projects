// ==========================================================================
// VIDEOSERVER class file
// ==========================================================================
// Last updated on 5/12/09; 5/14/09; 5/15/09; 5/22/09
// ==========================================================================

#include <iostream>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include <QFile>

#include "math/basic_math.h"
#include "Qt/web/DOMParser.h"
#include "astro_geo/geopoint.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "Qt/web/VideoServer.h"

#include "general/filefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void VideoServer::allocate_member_objects()
{
}		       

void VideoServer::initialize_member_objects()
{
   n_horiz_output_pixels=-1;
//   n_horiz_output_pixels=250;
//   n_horiz_output_pixels=500;
//   n_horiz_output_pixels=750;
//   n_horiz_output_pixels=800;

   Movie_ptr=NULL;
   tracks_group_ptr=NULL;
   CylindersGroup_ptr=NULL;
   EarthRegionsGroup_ptr=NULL;

   photo_longitude=photo_latitude=photo_altitude=-1;
   photo_horizontal_uncertainty=photo_vertical_uncertainty=photo_time=-1;
}

VideoServer::VideoServer(
   string host_IP_address,qint16 port, QObject* parent) :
   WebServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
VideoServer::~VideoServer()
{
   server_ptr->close();
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

// Member function VideoServer::get() is significantly different from
// WebServer::get().  It is designed specifically to return PNG output
// to a web client which requests a video chip via a
// longitude-latitude bounding box.  Because the returned image
// represents binary rather than text data, we bind a new QDataStream
// to the output socket and do not send any output to the QTextStream
// which is opened within WebServer::get().  [Once this method is
// finished writing output to QDataStream, it sets WebServer's
// write_text_content_to_socket_flag to false so that no additional
// information is written to the output socket within
// WebServer::readSocket().] Secondly, we need to return the contents
// of QByteArrays (basically char*) rather than QStrings.  With lots
// of help from Dave Ceddia, we also learned the painful and hard way
// that we must send both the ResponseHeader and PNG data payload to
// QDatastream via its writeRawData member function rather than
// Operator<<.  The latter introduces spurious bytes between the
// header and payload which causes the browser to malfunction.

QByteArray VideoServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
   cout << "inside VideoServer:get()" << endl;

   QByteArray empty;

   URL_path=url.path().toStdString();
   cout << "URL path = " << URL_path << endl;
   extract_KeyValue_pairs(url);
   
   if (Movie_ptr==NULL)
   {
      cout << "Error in VideoServer::get()!" << endl;
      cout << "Movie_ptr=NULL!" << endl;
      return empty;
   }

   if ( (URL_path=="/video_chip_bbox/" &&
         extract_chip_corresponding_to_bbox() ) ||
        (URL_path=="/video_chip_posn_and_radius/" &&
         extract_chip_corresponding_to_center_and_radius() ) ||
        ( (URL_path=="/video_chip_on_track/" &&
           extract_chip_centered_on_track()) ) )
   {
//      cout << "PNG_data_array.size() = " << PNG_data_array.size() << endl;
      responseHeader.setContentType("image/png");
      responseHeader.setContentLength(PNG_data_array.size());
//      cout << "responseHeader.toString().toStdString() = "
//           << responseHeader.toString().toStdString() << endl;
//      cout << "responseHeader.isValid() = "
//           << responseHeader.isValid() << endl;

      QTcpSocket* socket_ptr = qobject_cast<QTcpSocket *>( sender() );

      QDataStream os( socket_ptr );
      QByteArray ResponseHeaderByteArray=
         responseHeader.toString().toLocal8Bit();
      int n_header_bytes=os.writeRawData(
         ResponseHeaderByteArray,ResponseHeaderByteArray.size());
      int n_data_bytes=os.writeRawData(
         PNG_data_array,PNG_data_array.size());
         
      write_text_content_to_socket_flag=false;
   }

   return empty;
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QByteArray VideoServer::post( 
   const QUrl& url, const QByteArray& postData,
   QHttpResponseHeader& responseHeader)
{
   cout << "inside VideoServer::post()" << endl;

   VideoServer::get(url,responseHeader);

   if (URL_path=="/flightpath/")
   {
      QUrl tmp_url;
      QString tmp_qstring=tmp_url.fromPercentEncoding(postData);
      string post_data=tmp_qstring.toStdString();
//      cout << "post_data = " << post_data << endl;

/*
      QUrl decoded_url=tmp_url.fromEncoded(postData);
      string post_data2_str=decoded_url.toString().toStdString();
      cout << "post_data2_str = " << post_data2_str << endl;

      QString post_qstring(postData);
      QUrl url3(post_qstring);
      string url3_str=url3.toString().toStdString();
      cout << "post data 3 = " << url3_str << endl;
*/

      string XML_content=stringfunc::XML_content_between_tags(
         post_data,"polyline");
//      cout << "Initial XML content = " << XML_content << endl;
      XML_content=stringfunc::find_and_replace_char(XML_content,"+"," ");
//      cout << "Simplified XML content = " << XML_content << endl;
   }
   else if (URL_path=="/annotate")	// for iPhone pictures
   {
      read_input_photo(postData);
   } // URL_path conditional

   QByteArray empty_array;
   return empty_array;
}

// ==========================================================================
// Video specific query member functions
// ==========================================================================

// Member function extract_chip_corresponding_to_bbox takes in an STL
// vector of string key-value pairs whose size is assumed to equal 4.
// It loops over these string pairs and pulls out min & max longitude
// & latitude values.  This method then extracts the current video
// subframe corresponding to the lower left and upper right bbox
// corners.  It returns the video subframe's byte content within
// member QBytearray PNG_data_array.  If any of the input parameters
// are nonsensical, this boolean method returns false.

bool VideoServer::extract_chip_corresponding_to_bbox()
{
//   cout << "inside VideoServer::extract_chip_corresponding_to_bbox()" << endl;

   int n_args=4;
   if (KeyValue.size() < n_args || KeyValue.size() > n_args+2) return false;
   n_args=KeyValue.size();

   double min_longitude,max_longitude,min_latitude,max_latitude;
   double time_stamp=0;

   int n_acceptable_keys=0;
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="MinimumLongitude")
      {
         min_longitude=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="MaximumLongitude")
      {
         max_longitude=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="MinimumLatitude")
      {
         min_latitude=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="MaximumLatitude")
      {
         max_latitude=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="NHorizPixels")
      {
         n_horiz_output_pixels=
            stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="TimeStamp")
      {
         time_stamp=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }

   } // loop over index k labeling KeyValue key possibilities

//   cout.precision(12);
//   cout << "min_longitude = " << min_longitude 
//        << " min_latitude =  " << min_latitude << endl;
//   cout << "max_longitude = " << max_longitude 
//        << " max_latitude =  " << max_latitude << endl;
//   cout << "NHorizPixels = " << n_horiz_output_pixels << endl;
//   cout << "time_stamp= " << time_stamp << endl;
   
   if (n_acceptable_keys < n_args) return false;

   if (min_longitude < -180 || min_longitude > 180 ||
       max_longitude < -180 || max_longitude > 180 ||
       min_latitude < -180 || min_latitude > 180 ||
       max_latitude < -180 || max_latitude > 180) return false;

//      string output_subdir="./subframes";
//      string output_filename="videochip";
//      Movie_ptr->export_current_subframe(
//         min_longitude,max_longitude,min_latitude,max_latitude,
//         output_subdir,output_filename);

   bool draw_central_bbox_flag=false;
   if (!Movie_ptr->export_current_subframe(
      min_longitude,max_longitude,min_latitude,max_latitude,
      draw_central_bbox_flag,n_horiz_output_pixels))
   {
      return false;
   }

   PNG_data_array=QByteArray(
      Movie_ptr->get_texture_rectangle_ptr()->
      get_output_image_string().c_str(),
      Movie_ptr->get_texture_rectangle_ptr()->
      get_output_image_string().size());

   return true;
}

// ---------------------------------------------------------------------
// Member function extract_chip_corresponding_to_center_and_radius
// extracts a center point's longitude and latitude coordinates along
// with a radius parameter measured in meters from the input STL
// vector of string pairs.  (The input vector's size must equal 3.)
// After converting to UTM coordinates, this method computes a
// longitude-latitude bbox from the input arguments.  It then calls
// Movie member functions which write to file output a video chip
// whose number of horizontal pixels is fixed at a large enough value
// to be comfortably viewed within a web browser window.  The output
// PNG file is written to the output_subdir specified within this
// method.  If any of the input parameters are nonsensical, this
// boolean method returns false.

bool VideoServer::extract_chip_corresponding_to_center_and_radius()
{
//   cout << "inside VideoServer::extract_chip_corresponding_to_center_and_radius()" << endl;

   int n_args=3;
   if (KeyValue.size() < n_args || KeyValue.size() > n_args+2) return false;
   n_args=KeyValue.size();
//   cout << "n_args = " << n_args << endl;

   double center_longitude=NEGATIVEINFINITY;
   double center_latitude=NEGATIVEINFINITY;
   double radius=NEGATIVEINFINITY;
   double time_stamp=0;

   int n_acceptable_keys=0;
   for (int k=0; k<n_args; k++)
   {
      if (KeyValue[k].first=="CenterLongitude")
      {
         center_longitude=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="CenterLatitude")
      {
         center_latitude=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="RadiusInMeters")
      {
         radius=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="NHorizPixels")
      {
         n_horiz_output_pixels=
            stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="TimeStamp")
      {
         time_stamp=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }

   } // loop over index k labeling KeyValue key possibilities

//   cout.precision(12);
//   cout << "center_longitude = " << center_longitude 
//        << " center_latitude =  " << center_latitude << endl;
//   cout << "radius = " << radius << endl;

   if (n_acceptable_keys < n_args) return false;

   if (center_longitude < -180 || center_longitude > 180 ||
       center_latitude < -180 || center_latitude > 180 ||
       radius < 0) return false;
   
   geopoint center_posn(center_longitude,center_latitude);
   bool draw_central_bbox_flag=false;
   return extract_chip_given_geopoint_and_radius(
      center_posn,radius,draw_central_bbox_flag);
}

// ---------------------------------------------------------------------
// Member function extract_chip_centered_on_track extracts a truth
// vehicle's label ID along with a radius parameter measured in meters
// from the input STL vector of string pairs.  (The input vector's
// size must equal 2.)  This method computes a geopoint whose
// coordinates correspond to the vehicle's instantaneous position.  It
// then calls Movie member functions which generate a stringstream for
// a PNG video chip whose diagonal radius equals the input parameter.
// If any of the input parameters are nonsensical, this boolean method
// returns false.

bool VideoServer::extract_chip_centered_on_track()
{
//   cout << "inside VideoServer::extract_chip_centered_on_track()" << endl;
//   cout << "KeyValue.size() = " << KeyValue.size() << endl;

   int n_args=2;
   if (KeyValue.size() < n_args || KeyValue.size() > n_args+2) return false;
   n_args=KeyValue.size();

   int track_label_ID=-1;
   double radius=NEGATIVEINFINITY;
   double time_stamp=0;

   int n_acceptable_keys=0;
   for (int k=0; k<n_args; k++)
   {
//      cout << "k = " << k
//           << " KeyValue[k].first = " << KeyValue[k].first
//           << " KeyValue[k].second = " << KeyValue[k].second << endl;
      if (KeyValue[k].first=="TrackID")
      {
         track_label_ID=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="RadiusInMeters")
      {
         radius=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="NHorizPixels")
      {
         n_horiz_output_pixels=
            stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
      else if (KeyValue[k].first=="TimeStamp")
      {
         time_stamp=stringfunc::string_to_number(KeyValue[k].second);
         n_acceptable_keys++;
      }
   } // loop over index k labeling KeyValue key possibilities

//   cout.precision(12);
//   cout << "track_label_ID = " << track_label_ID << endl;
//   cout << "radius = " << radius << endl;
//   cout << "n_acceptable_keys = " << n_acceptable_keys << endl;
   
   if (n_acceptable_keys < n_args) return false;

   if (track_label_ID < 0 || radius < 0) return false;
   if (tracks_group_ptr==NULL) return false;

   track* curr_track_ptr=tracks_group_ptr->
      get_track_given_label(track_label_ID);
   int Cylinder_ID=curr_track_ptr->get_Cylinder_ID();

   /*
   int track_ID=tracks_group_ptr->get_track_ID(track_label_ID);
//   cout << "track_ID = " << track_ID << endl;
   int Cylinder_ID=tracks_group_ptr->get_track_ptr(track_ID)->
      get_Cylinder_ID();
   */

//   cout << "Cylinder_ID = " << Cylinder_ID << endl;
   Cylinder* Cylinder_ptr=CylindersGroup_ptr->get_ID_labeled_Cylinder_ptr(
      Cylinder_ID);

   threevector cyl_posn;
   Cylinder_ptr->get_UVW_coords(
      CylindersGroup_ptr->get_curr_t(),CylindersGroup_ptr->get_passnumber(),
      cyl_posn);
//   cout << "cyl_posn = " << cyl_posn << endl;

   geopoint curr_mover_geolocation(
      EarthRegionsGroup_ptr->get_northern_hemisphere_flag(),
      EarthRegionsGroup_ptr->get_specified_UTM_zonenumber(),
      cyl_posn.get(0),cyl_posn.get(1));
//   cout << "curr_mover_geolocation = " << curr_mover_geolocation << endl;

   bool draw_central_bbox_flag=true;
   return extract_chip_given_geopoint_and_radius(
      curr_mover_geolocation,radius,draw_central_bbox_flag);
}

// ---------------------------------------------------------------------
bool VideoServer::extract_chip_given_geopoint_and_radius(
   const geopoint& center_posn,double radius,bool draw_central_bbox_flag)
{
//   cout << "inside VideoServer::extract_chip_given_geopoint_and_radius()" << endl;

   double easting=center_posn.get_UTM_easting();
   double northing=center_posn.get_UTM_northing();
   geopoint lower_left_corner(
      center_posn.get_northern_hemisphere_flag(),
      center_posn.get_UTM_zonenumber(),
      easting-radius,northing-radius);
   geopoint upper_right_corner(
      center_posn.get_northern_hemisphere_flag(),
      center_posn.get_UTM_zonenumber(),
      easting+radius,northing+radius);
   double min_longitude=lower_left_corner.get_longitude();
   double max_longitude=upper_right_corner.get_longitude();
   double min_latitude=lower_left_corner.get_latitude();
   double max_latitude=upper_right_corner.get_latitude();

//   cout << "min longitude = " << min_longitude
//        << " max longitude = " << max_longitude << endl;
//   cout << "min latitude = " << min_latitude
//        << " max latitude = " << max_latitude << endl;

   string output_subdir=
      "/data/video/2007/Lubbock/constant_hawk/AR1_002/video_chips/";
   string output_filename="curr_videochip";

   if (!Movie_ptr->export_current_subframe(
      min_longitude,max_longitude,min_latitude,max_latitude,
      draw_central_bbox_flag,n_horiz_output_pixels))
   {
      return false;
   }

   PNG_data_array=QByteArray(
      Movie_ptr->get_texture_rectangle_ptr()->
      get_output_image_string().c_str(),
      Movie_ptr->get_texture_rectangle_ptr()->
      get_output_image_string().size());

   return true;
}

// ==========================================================================
// XML output member functions
// ==========================================================================

// Member function copy_XML_query_output

QDomDocument& VideoServer::copy_XML_query_output(string query)
{
//   string XML_content=query_SKS_DataServer(query);
   string XML_content="foo";
//   cout << "XML response = " << XML_content << endl;

   DOMParser parser;
   parser.read_XML_string_into_DOM(XML_content);
   return parser.get_doc();
}

// ==========================================================================
// iPhone photo handling member functions
// ==========================================================================

// Member function read_input_photo() extracts metadata sent by the
// iPhone along with a JPG image transmitted as a QByteArray.

void VideoServer::read_input_photo(const QByteArray& postData)
{
   cout << "inside VideoServer::read_input_photo()" << endl;
   cout << "iPhone photo request received" << endl;
   cout << "postData.size() = " << postData.size() << endl;

// Loop over KeyValue pairs and extract iPhone metadata:

   for (int i=0; i<KeyValue.size(); i++)
   {
      pair<string,string> curr_pair(KeyValue[i]);
      if (curr_pair.first=="Longitude")
      {
         photo_longitude=stringfunc::string_to_number(curr_pair.second);
      }
      else if (curr_pair.first=="Latitude")
      {
         photo_latitude=stringfunc::string_to_number(curr_pair.second);
      }
      else if (curr_pair.first=="Altitude")
      {
         photo_altitude=stringfunc::string_to_number(curr_pair.second);
      }
      else if (curr_pair.first=="HorizontalUncertainty")
      {
         photo_horizontal_uncertainty=
            stringfunc::string_to_number(curr_pair.second);
      }
      else if (curr_pair.first=="VerticalUncertainty")
      {
         photo_vertical_uncertainty=
            stringfunc::string_to_number(curr_pair.second);
      }
      else if (curr_pair.first=="Time")
      {
         photo_time=stringfunc::string_to_number(curr_pair.second);
      }
   }

   cout << "photo_longitude = " << photo_longitude 
        << " photo_latitude = " << photo_latitude
        << " photo_altitude = " << photo_altitude
        << endl;
   cout << "horiz_uncertainty = " << photo_horizontal_uncertainty
        << " vert_uncertainty = " << photo_vertical_uncertainty << endl;

   QDateTime mytime;
   mytime.setTime_t(photo_time);
   qDebug() << mytime;

// On 5/14/09, Ross indicated that the image type should be
// extractable from the HTTP header's contentType.  If protected
// member variable content_type is empty, assume incoming image is a
// JPEG:

   string suffix="jpg";
   if (content_type=="image/jpg")
   {
      suffix="jpg";
   }
   else if (content_type=="image/png")
   {
      suffix="png";
   }

   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();
   texture_rectangle_ptr->read_image_from_char_buffer(
      suffix,postData.constData(),postData.size());
   texture_rectangle_ptr->set_TextureRectangle_image();
   Movie_ptr->reset_texture_coords();
   Movie_ptr->dirtyGeomDisplay();
}

    
      
