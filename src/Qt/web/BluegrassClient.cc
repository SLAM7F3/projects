// ==========================================================================
// BLUEGRASSCLIENT class file
// ==========================================================================
// Last updated on 10/3/08; 1/21/09; 1/22/09; 3/30/10
// ==========================================================================

#include <iostream>
#include <list>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "Qt/web/BluegrassClient.h"
#include "astro_geo/Clock.h"
#include "Qt/web/DOMParser.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "passes/PassInfo.h"
#include "geometry/polygon.h"
#include "geometry/polyline.h"
#include "general/stringfuncs.h"
#include "track/track.h"
#include "track/tracks_group.h"
#include "track/tracklist.h"

#include "astro_geo/geopoint.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::list;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void BluegrassClient::allocate_member_objects()
{
   http_ptr=new QHttp(this);
}		       

void BluegrassClient::initialize_member_objects()
{
   returned_output="";
   EarthRegionsGroup_ptr=NULL;
}

BluegrassClient::BluegrassClient(
   string BluegrassServer_URL,QObject* parent) :
   PickHandlerCallbacks()
{
   allocate_member_objects();
   initialize_member_objects();

   string BluegrassServer_IP=stringfunc::get_hostname_from_URL(
      BluegrassServer_URL);
   int BluegrassServer_portnumber=stringfunc::get_portnumber_from_URL(
      BluegrassServer_URL);
   http_ptr->setHost(QString(BluegrassServer_IP.c_str()),
                     BluegrassServer_portnumber);
//   connect( http_ptr, SIGNAL(requestFinished(int id,bool error) ), 
   connect( http_ptr, SIGNAL(readyRead(const QHttpResponseHeader&) ), 
            this, SLOT(httpResponseAvailable(const QHttpResponseHeader&)) );
}

// ---------------------------------------------------------------------
BluegrassClient::~BluegrassClient()
{
}

// ==========================================================================
// Bluegrass specific query member functions
// ==========================================================================

// Member function query_BluegrassServer imports a query string into
// http_ptr->get().

void BluegrassClient::query_BluegrassServer(string curr_query)
{
//   cout << "inside BluegrassClient::query_BluegrassServer()" << endl;
//   cout << "query = " << curr_query << endl;
   
   returned_output="";
   QString requestPath=QString(curr_query.c_str());
//   cout << "requestPath = " << requestPath.toStdString() << endl;
   http_ptr->get( requestPath );
}

// ---------------------------------------------------------------------
// After waiting for the BluegrassServer to return its http output
// with a terminal sentinel, member function
// get_BluegrassServer_response() strips out just the XML body of the
// BluegrassServer's message.  It finally loads the XML message into
// the SKS_interface's DOM.

string BluegrassClient::get_BluegrassServer_response()
{
//   cout << "inside BluegrassClient::get_BluegrassServer_response()" << endl;

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the WebServer.  Here we force the main Qt loop to continue
// processing until a tag indicating XML message completion has been
// received by the WebClient from the WebServer:

   string response_tag1="</response>";
   string response_tag2="<response/>";
   int counter=0;
   while (! (returned_output_contains_substring(response_tag1) ||
             returned_output_contains_substring(response_tag2)) )
   {
      int n_iters=25;
      for (int n=0; n<n_iters; n++)
      {
         qApp->processEvents();
      }
      if (counter%5000==0)
      {
         cout << counter/5000 << " " << flush;
      }
      counter++;
   }
   cout << endl;

   string XML_content=stringfunc::XML_content_between_tags(
      get_returned_output(),"response");
//   cout << "XML_content = " << XML_content << endl;
//   cout << "XML_content.size() = " << XML_content.size() << endl;
   return XML_content;
}

// ---------------------------------------------------------------------
void BluegrassClient::httpResponseAvailable( 
   const QHttpResponseHeader& response_header)
{
//   cout << "inside BluegrassClient::httpResponseAvailable()" << endl;

   QByteArray output=http_ptr->readAll();
   returned_output += string(output.data());
//   cout << "returned_output = " << returned_output << endl;
}

// ---------------------------------------------------------------------
string& BluegrassClient::get_returned_output()
{
   return returned_output;
}

// ---------------------------------------------------------------------
bool BluegrassClient::returned_output_contains_substring(string substring)
{
//   cout << "inside BluegrassClient::returned_output_contains_substring()" << endl;
   int posn=stringfunc::first_substring_location(returned_output,substring);
//   cout << "posn = " << posn << endl;
   return (posn >= 0);
}

// ==========================================================================
// Dataserver track retrieval member functions
// ==========================================================================

string BluegrassClient::form_mover_tracks_query(PassInfo* passinfo_ptr)
{
//   cout << "inside BluegrassClient::form_mover_tracks_query()" << endl;
   string banner="Forming mover tracks query:";
   outputfunc::write_banner(banner);

   double longitude_min=passinfo_ptr->get_longitude_lo();
   double longitude_max=passinfo_ptr->get_longitude_hi();
   double latitude_min=passinfo_ptr->get_latitude_lo();
   double latitude_max=passinfo_ptr->get_latitude_hi();
   double elapsed_secs_min=
      passinfo_ptr->get_elapsed_secs_since_epoch_lo();
   double elapsed_secs_max=
      passinfo_ptr->get_elapsed_secs_since_epoch_hi();

   string query="/vehicle_tracks/?";
   query += "min_longitude="+stringfunc::number_to_string(longitude_min);
   query += "&min_latitude="+stringfunc::number_to_string(latitude_min);
   query += "&max_longitude="+stringfunc::number_to_string(longitude_max);
   query += "&max_latitude="+stringfunc::number_to_string(latitude_max);
   query += "&t_start="+stringfunc::number_to_string(elapsed_secs_min);
   query += "&t_stop="+stringfunc::number_to_string(elapsed_secs_max);

//      cout.precision(12);
//      cout << "longitude_min = " << longitude_min << endl;
//      cout << "longitude_max = " << longitude_max << endl;
//      cout << "latitude_min = " << latitude_min << endl;
//      cout << "latitude_max = " << latitude_max << endl;
//      cout << "elapsed secs min = " << elapsed_secs_min << endl;
//      cout << "elapsed secs max = " << elapsed_secs_max << endl;

//   cout << "query = " << query << endl;
   return query;
}

// ---------------------------------------------------------------------
// Member function retrieve_mover_tracks takes in the XML output from
// some prior SKS DataServer query within input string SKS_response.
// It extracts the returned track information from the parsed XML into
// the output tracks_group object.  This method returns the number of
// tracks satisfying the SKS query.

int BluegrassClient::retrieve_mover_tracks(
   string SKS_response,double secs_offset,double tracks_altitude,
   tracks_group* mover_tracks_ptr)
{
   threevector old_origin_offset=Zero_vector;
   double mover_posns_rescaling_factor=1.0;
   threevector new_origin_offset=Zero_vector;
   return retrieve_mover_tracks(
      SKS_response,EarthRegionsGroup_ptr->get_specified_UTM_zonenumber(),
      old_origin_offset,mover_posns_rescaling_factor,new_origin_offset,
      secs_offset,tracks_altitude,mover_tracks_ptr);
}

// This overloaded version of retrieve_mover_tracks() was generalized
// in Jan 2009 to allow for Bluegrass truth tracks to be superposed on
// Baghdad for UAV path planning with dynamic ground movers.  It takes
// in old and new origin offsets (corresponding to Lubbock and
// Baghdad) as well as a scale factor by which mover positions are
// stretched relative to the new origin.  

int BluegrassClient::retrieve_mover_tracks(
   string SKS_response,int specified_UTM_zonenumber,
   const threevector& old_origin_offset,double mover_posns_rescaling_factor,
   const threevector& new_origin_offset,
   double secs_offset,double tracks_altitude,tracks_group* mover_tracks_ptr)
{
//   cout << "inside BluegrassClient::retrieve_mover_tracks()" << endl;
//   cout << "secs_offset = " << secs_offset << endl;

//   cout << "SKS_response = " << SKS_response << endl;
   TrackList* TrackList_ptr=
      generate_tracks_from_parsed_XML(SKS_response);
   if (TrackList_ptr==NULL) return 0;

// Generate truth vehicle tracks:

   cout << "Generating truth vehicle tracks:" << endl;

//   1 m/s = 2.23693 miles/hour
   const double meters_per_second_to_miles_per_hour=2.236936;

   bool northern_hemisphere_flag;
   double easting,northing;
   Clock clock;
   TrackItems ti = TrackList_ptr->getTrack();
   for (TrackItemsIterator i = ti.begin(); i != ti.end(); ++i ) 
   {
      track* curr_track_ptr=mover_tracks_ptr->generate_new_track();
      curr_track_ptr->set_entityID( (*i)->get_entityID() );
      curr_track_ptr->set_entityType( (*i)->get_entityType() );
      curr_track_ptr->set_label( (*i)->get_label() );

      int label_ID=-1;
      string track_label=curr_track_ptr->get_label();
      if (track_label.size() > 0)
      {
         string track_label_integer_ID_str=
            stringfunc::suffix(track_label,"V");
         if (track_label_integer_ID_str.size() > 0)
         {
            label_ID=stringfunc::string_to_integer(
               track_label_integer_ID_str);
            curr_track_ptr->set_label_ID(label_ID);
//            cout << "curr_track_ptr->get_label_ID() = " << label_ID << endl;

// Add (track_label_ID,track_ID) pair to *mover_tracks_ptr's
// track_label_map member:

            (*mover_tracks_ptr->get_track_labels_map_ptr())
               [curr_track_ptr->get_label_ID()]=curr_track_ptr->get_ID();
         }
      } // track_label.size() > 0 conditional

      for (std::list<Observation *>::iterator oi = 
              (*i)->observationList.begin(); 
           oi != (*i)->observationList.end(); ++oi ) 
      {
         double elapsed_secs_since_ref_epoch=(*oi)->get_time();
         double curr_t=elapsed_secs_since_ref_epoch+secs_offset;
//         clock.convert_elapsed_secs_to_date(curr_t);

         bool specified_northern_hemisphere_flag=true;
         double longitude=(*oi)->get_longitude();
         double latitude=(*oi)->get_latitude();
         latlongfunc::LL_to_northing_easting(
            longitude,latitude,
            specified_northern_hemisphere_flag,specified_UTM_zonenumber,
            easting,northing);
         threevector curr_posn(easting,northing,tracks_altitude);

         double speed=(*oi)->get_speed();
         double azimuth=(*oi)->get_azimuth();
//         cout << "speed = " << speed
//              << " azimuth (degs) = " << azimuth << endl;

         double theta=(90-azimuth)*PI/180;
         double Vx=speed*cos(theta);
         double Vy=speed*sin(theta);
//         cout << "Vx = " << Vx << " Vy = " << Vy << endl;
         threevector curr_velocity(Vx,Vy);

// Add delta Easting and Northing offsets to track positions:
// (e.g. Convert Bluegrass truth track UTM values into reasonable
// coordinate offsets for Baghdad in order to simulate ground movers
// in Baghdad)

         curr_posn -= old_origin_offset;
         curr_posn *= mover_posns_rescaling_factor;
         curr_posn += new_origin_offset;

//         cout << "curr_t = " << curr_t
//              << " curr_posn = " << curr_posn
//              << " curr_vel = " << curr_velocity 
//              << endl;
         
         curr_track_ptr->set_posn_velocity(curr_t,curr_posn,curr_velocity);

//         cout.precision(12);
//         int ID=curr_track_ptr->get_ID();
//         int label_ID=curr_track_ptr->get_label_ID();
      } // loop over Observation iterator oi
      curr_track_ptr->temporally_sort_posns_and_velocities();

//      clock.convert_elapsed_secs_to_date(
//         curr_track_ptr->get_earliest_time());
//      cout << "Earliest time = " 
//           << clock.YYYY_MM_DD_H_M_S() << endl;
//      clock.convert_elapsed_secs_to_date(
//         curr_track_ptr->get_latest_time());
//      cout << "Latest time = " 
//           << clock.YYYY_MM_DD_H_M_S() << endl;

   } // loop over TrackItems iterator i
   
   cout << "Number of mover tracks = " 
        << mover_tracks_ptr->get_n_tracks() << endl;
   return mover_tracks_ptr->get_n_tracks();
}

// ---------------------------------------------------------------------
TrackList* BluegrassClient::generate_tracks_from_parsed_XML(string XML_input)
{
//   cout << "inside BluegrassClient::generate_tracks_from_parsed_XML()"
//        << endl;

   QDomDocument doc;
   doc.setContent( QByteArray(XML_input.c_str()) );
    
   QDomElement root = doc.documentElement();
//   cout << "root.tagName() = " << root.tagName().toStdString() << endl;
   if ( root.tagName() != "response" ) return NULL;

   DOMParser parser;
   parser.read_XML_string_into_DOM(XML_input);

   string tagname="track";
   vector<QDomElement> track_elements=parser.find_elements(tagname);
//   cout << "track_elements.size() = " << track_elements.size() << endl;

   TrackList* tracklist_ptr=new TrackList;
   for (int t=0; t<track_elements.size(); t++)
   {
      string entityID,entityType,label;
      parser.extract_attribute_value_from_element(
         track_elements[t],"entityId",entityID);
      parser.extract_attribute_value_from_element(
         track_elements[t],"entityType",entityType);
      parser.extract_attribute_value_from_element(
         track_elements[t],"label",label);
//      cout << "t = " << t << " label = " << label << endl;

      vector<string> time_values,longitude_values,latitude_values,
         speed_values,azimuth_values;
      parser.extract_attribute_values_from_children_elements(
         track_elements[t],"observation","time",time_values);
      parser.extract_attribute_values_from_children_elements(
         track_elements[t],"observation","lon",longitude_values);
      parser.extract_attribute_values_from_children_elements(
         track_elements[t],"observation","lat",latitude_values);
      parser.extract_attribute_values_from_children_elements(
         track_elements[t],"observation","speed",speed_values);
      parser.extract_attribute_values_from_children_elements(
         track_elements[t],"observation","azimuth",azimuth_values);

// Save observation data for current track into STL list of
// dynamically generated Observation objects:

      list<Observation*> obs_list;
      for (int i=0; i<time_values.size(); i++)
      {
         Observation* obs_ptr=new Observation;
         long long curr_time=stringfunc::string_to_integer(
            time_values[i]);
         double curr_longitude=stringfunc::string_to_number(
            longitude_values[i]);
         double curr_latitude=stringfunc::string_to_number(
            latitude_values[i]);
         double curr_speed=stringfunc::string_to_number(
            speed_values[i]);
         double curr_azimuth=stringfunc::string_to_number(
            azimuth_values[i]);
         obs_ptr->set_time(curr_time);
         obs_ptr->set_longitude(curr_longitude);
         obs_ptr->set_latitude(curr_latitude);
         obs_ptr->set_speed(curr_speed);
         obs_ptr->set_azimuth(curr_azimuth);
         obs_list.push_back(obs_ptr);
      }

      TrackItem* track_item_ptr=new TrackItem(
         entityID,entityType,label,obs_list);
      tracklist_ptr->addItem(track_item_ptr);
   } // loop over index t labeling tracks
   
   return tracklist_ptr;
}

// ---------------------------------------------------------------------
// Member function eliminate_repeated_tracks deletes from input
// ROI_tracks_group_ptr all tracks which already exist within input
// STL vector ROI_tracks_group_ptrs.  This method performs a brute
// force search over elements of the former for matches with elements
// of the latter.

void BluegrassClient::eliminate_repeated_tracks(
   tracks_group* ROI_tracks_group_ptr,
   const vector<tracks_group*>& ROI_tracks_group_ptrs)
{
//   cout << "inside BluegrassClient::eliminate_repeated_tracks()" << endl;
//   cout << "Initially, ROI_tracks_group_ptr->get_n_tracks() = "
//        << ROI_tracks_group_ptr->get_n_tracks() << endl;

   int n_existing_tracks=0;
   int n_matches=0;
   vector<track*> ptrs_for_tracks_to_delete;
//   cout << "ROI_tracks_group_ptrs.size() = "
//        << ROI_tracks_group_ptrs.size() << endl;
   for (int g=0; g<ROI_tracks_group_ptrs.size()-1; g++)
   {
      tracks_group* existing_tracks_group_ptr=ROI_tracks_group_ptrs[g];
      n_existing_tracks += existing_tracks_group_ptr->get_n_tracks();
//      cout << "n_existing_tracks = " << n_existing_tracks << endl;
         
      vector<track*> ROI_tracks_ptrs=ROI_tracks_group_ptr->
         get_all_track_ptrs();
      
      for (int t=0; t<int(ROI_tracks_ptrs.size()); t++)
      {
         track* curr_track_ptr=ROI_tracks_ptrs[t];
         if (curr_track_ptr==NULL) continue;

         vector<track*> existing_tracks_ptrs=existing_tracks_group_ptr->
            get_all_track_ptrs();
         for (int s=0; s<int(existing_tracks_ptrs.size()); s++)
         {
            track* existing_track_ptr=existing_tracks_ptrs[s];
            if (existing_track_ptr==NULL) continue;
            if (curr_track_ptr->get_label_ID()==
                existing_track_ptr->get_label_ID())
            {
               cout << "matching label ID = "
                    << curr_track_ptr->get_label_ID() << endl;
               n_matches++;
               ptrs_for_tracks_to_delete.push_back(curr_track_ptr);
               break;
            }
         } // loop over index s labeling tracks in existing ROI tracks grp
      } // loop over index t labeling tracks within latest ROI trcks grp
   } // loop over index g labeling ROI tracks groups

   for (int i=0; i<ptrs_for_tracks_to_delete.size(); i++)
   {
      track* track_to_destroy_ptr=ptrs_for_tracks_to_delete[i];
//      cout << "i = " << i 
//           << " Deleting track whose ID = "
//           << track_to_destroy_ptr->get_ID()
//           << endl;
      ROI_tracks_group_ptr->destroy_track(track_to_destroy_ptr);
   }

//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//   cout << "n_existing_tracks = " << n_existing_tracks << endl;
//   cout << "n_matches = " << n_matches << endl;
//   cout << "Finally, ROI_tracks_group_ptr->get_n_tracks() = "
//        << ROI_tracks_group_ptr->get_n_tracks() << endl;
//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;      
}

// ---------------------------------------------------------------------
// Member function generate_ROIs_from_parsed_XML

vector<polyline*> BluegrassClient::generate_ROIs_from_parsed_XML(
   string XML_input,vector<vector<int> >& 
   label_IDs_for_slow_vehicles_passing_thru_ROIs)
{
//   cout << "inside BluegrassClient::generate_ROIs_from_parsed_XML()"
//        << endl;

   vector<polyline*> ROI_polyline_ptrs;
//   vector<vector<int> > label_IDs_for_slow_vehicles_passing_thru_ROIs;

   QDomDocument doc;
   doc.setContent( QByteArray(XML_input.c_str()) );
    
   QDomElement root = doc.documentElement();
//   cout << "root.tagName() = " << root.tagName().toStdString() << endl;
   if ( root.tagName() != "response" ) return ROI_polyline_ptrs;

   DOMParser parser;
   parser.read_XML_string_into_DOM(XML_input);

   string tagname="region";
   vector<QDomElement> region_elements=parser.find_elements(tagname);
//   cout << "region_elements.size() = " << region_elements.size() << endl;

// FAKE FAKE: Hardwire Lubbock ladar extremal values to provide sanity
// check on bbox results returned by SKS:

   const double min_easting=215963.53125;
   const double max_easting=241961.5;
   const double min_northing=3701696;
   const double max_northing=3729787.5;

   for (int r=0; r<region_elements.size(); r++)
   {
      string ewkt,label,area,entityLabels,bbox_str;
      parser.extract_attribute_value_from_element(
         region_elements[r],"ewkt",ewkt);
      parser.extract_attribute_value_from_element(
         region_elements[r],"label",label);
      parser.extract_attribute_value_from_element(
         region_elements[r],"area",area);
      parser.extract_attribute_value_from_element(
         region_elements[r],"entityLabels",entityLabels);
      parser.extract_attribute_value_from_element(
         region_elements[r],"bbox",bbox_str);
      double length_scale=sqrt(stringfunc::string_to_number(area));
//      cout << "r = " << r 
//           << " ewkt = " << ewkt 
//           << " label = " << label
//           << " area = " << area 
//           << " entityLabels = " << entityLabels
//           << " bbox = " << bbox_str 
//           << " sqrt(area) = " << length_scale
//           << endl;

      if (length_scale > 1000)
      {
         cout << "ewkt = " << ewkt << endl;
         outputfunc::enter_continue_char();
      }

// Parse POLYGON coordinates contained within EWKT string:
      
      string suffix=stringfunc::erase_chars_before_first_substring(ewkt,"(");
      string prefix=stringfunc::erase_chars_after_first_substring(suffix,")");
//      cout << "prefix = " << prefix << endl << endl;
      string poly_coords_string=prefix.substr(2,prefix.size()-3);
//      cout << "poly_coords_string = " << poly_coords_string << endl << endl;
      vector<string> ll_str_pairs=
         stringfunc::decompose_string_into_substrings(poly_coords_string,",");

      const int SKS_fudge_factor=2;
      vector<threevector> ROI_poly_vertices;
      for (int p=0; p<ll_str_pairs.size()-SKS_fudge_factor; p++)
      {
//         cout << "p = " << p
//              << " ll_str_pairs[p] = " << ll_str_pairs[p] << endl;
         vector<double> curr_ll_pair=
            stringfunc::string_to_numbers(ll_str_pairs[p]);
         cout.precision(12);
//         cout << "curr_ll_pair[0] = " << curr_ll_pair[0]
//              << " curr_ll_pair[1] = " << curr_ll_pair[1] << endl;
         geopoint curr_geopoint(curr_ll_pair[0],curr_ll_pair[1]);
         threevector curr_vertex(curr_geopoint.get_UTM_easting(),
                                 curr_geopoint.get_UTM_northing());
//         cout << curr_vertex.get(0) << "  " << curr_vertex.get(1) << endl;
         const double TINY=1.0E-6;
         int n_vertices=ROI_poly_vertices.size();
         
         if (n_vertices==0 || 
             (n_vertices > 0 && 
              (!curr_vertex.nearly_equal(ROI_poly_vertices.back(),TINY))))
         {

            double curr_x=curr_vertex.get(0);
            double curr_y=curr_vertex.get(1);
            if (curr_x > min_easting && curr_x < max_easting &&
                curr_y > min_northing && curr_y < max_northing)
            {
               
               ROI_poly_vertices.push_back(curr_vertex);
//               cout << "ROI_poly_vertices.back() = "
//                    << ROI_poly_vertices.back() << endl;
            }
         }
      } // loop over index p labeling longitude-latitude string pairs
        //  for rth autogenerated region

      vector<string> bbox_coords_str=
         stringfunc::decompose_string_into_substrings(bbox_str,",");
      vector<double> bbox_coords;
      for (int c=0; c<bbox_coords_str.size(); c++)
      {
         bbox_coords.push_back(stringfunc::string_to_number(
            bbox_coords_str[c]));
//         cout << "c = " << c 
//              << " bbox_coords_str[c] = " << bbox_coords_str[c] 
//              << bbox_coords.back() 
//              << endl;
      }
      geopoint bbox_lower_left_corner(bbox_coords[0],bbox_coords[1]);
      geopoint bbox_upper_right_corner(bbox_coords[2],bbox_coords[3]);
      double min_x=bbox_lower_left_corner.get_UTM_easting();
      double min_y=bbox_lower_left_corner.get_UTM_northing();
      double max_x=bbox_upper_right_corner.get_UTM_easting();
      double max_y=bbox_upper_right_corner.get_UTM_northing();

// Make sure bbox corners lie within ladar point cloud area!

      if (min_x < min_easting || max_x > max_easting ||
          min_y < min_northing || max_y > max_northing) continue;

// Check whether new bbox corners coincide with any previously defined
// bbox corners:

      bool new_bbox_unique_flag=true;
      for (int b=0; b<auto_nominated_bboxes.size(); b++)
      {
         if (nearly_equal(min_x,auto_nominated_bboxes[b].get_xmin()) ||
             nearly_equal(max_x,auto_nominated_bboxes[b].get_xmax()) ||
             nearly_equal(min_y,auto_nominated_bboxes[b].get_ymin()) ||
             nearly_equal(max_y,auto_nominated_bboxes[b].get_ymax()) )
         {
            new_bbox_unique_flag=false;
         }
      }
      if (!new_bbox_unique_flag)
      {
//         cout << "!!!  BBOX PREVIOUSLY DECLARED !!!" << endl;
         continue;
      }
      auto_nominated_bboxes.push_back(bounding_box(min_x,max_x,min_y,max_y));
//      cout << "auto_nominated_bboxes.size() = "
//           << auto_nominated_bboxes.size() << endl;

      vector<threevector> bbox_vertices;
      bbox_vertices.push_back(threevector(min_x,min_y));
      bbox_vertices.push_back(threevector(min_x,max_y));
      bbox_vertices.push_back(threevector(max_x,max_y));
      bbox_vertices.push_back(threevector(max_x,min_y));

// Transform bbox center to longitude,latitude coords.  Needed to give
// this info to Tim Schreiner for Google Earth ROI display & query
// purposes on 8/6/08:

//      double center_x=0.5*(min_x+max_x);
//      double center_y=0.5*(min_y+max_y);
//      bool northern_hemisphere_flag=true;
//      int UTM_zone=14;	// Lubbock, TX
//      geopoint bbox_center(northern_hemisphere_flag,UTM_zone,
//                           center_x,center_y);
//      cout << "bbox center longitude = " << bbox_center.get_longitude()
//           << " bbox center latitude = " << bbox_center.get_latitude() 
//           << endl;

      polyline* ROI_polyline_ptr=new polyline(bbox_vertices);
      ROI_polyline_ptrs.push_back(ROI_polyline_ptr);
//      cout << "ROI_polyline_ptrs.size() = "
//           << ROI_polyline_ptrs.size() << endl;

// Extract labels for slow vehicles passing through bbox from SKS
// response:

      vector<string> entity_label_substrings=
         stringfunc::decompose_string_into_substrings(entityLabels,",");
      vector<int> slow_vehicle_label_IDs;
      for (int v=0; v<entity_label_substrings.size(); v++)
      {
         string curr_vehicle_label_str=
            stringfunc::suffix(entity_label_substrings[v],"V");
         int curr_vehicle_label_ID=stringfunc::string_to_integer(
            curr_vehicle_label_str);
         slow_vehicle_label_IDs.push_back(curr_vehicle_label_ID);
      }

      label_IDs_for_slow_vehicles_passing_thru_ROIs.push_back(
         slow_vehicle_label_IDs);

   } // loop over index r labeling region elements

   return ROI_polyline_ptrs;
}

// ---------------------------------------------------------------------
void BluegrassClient::display_selected_vehicle_webpage(string vehicle_label)
{
//   cout << "inside BluegrassClient::display_selected_vehicle_webpage()"
//        << endl;
//   cout << "vehicle_label = " << vehicle_label << endl;

   string curr_query="/vehicle_label/?label="+vehicle_label;
   query_BluegrassServer(curr_query);
}
