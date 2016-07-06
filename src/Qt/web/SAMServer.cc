// ==========================================================================
// SAMSERVER class file
// ==========================================================================
// Last updated on 3/23/08; 3/24/08; 3/25/08; 4/28/08; 4/30/08
// ==========================================================================

#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "robots/SAMs_group.h"
#include "Qt/web/SAMServer.h"
#include "Qt/web/SKSDataServerInterfacer.h"
#include "general/stringfuncs.h"
#include "templates/mytemplates.h"
#include "geometry/polyline.h"
#include "Qt/web/WebClient.h"

#include "general/outputfuncs.h"

#include "gearth/kml_parser.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void SAMServer::allocate_member_objects()
{
}		       

void SAMServer::initialize_member_objects()
{
   SKSDataServer_URL=HTMLServer_URL="";
   SKS_interface_ptr=NULL;
}

SAMServer::SAMServer(SAMs_group* SG_ptr,string host_IP_address,
                     qint16 port, QObject* parent) :
   WebServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();

   SAMs_group_ptr=SG_ptr;
}

// ---------------------------------------------------------------------
SAMServer::~SAMServer()
{
   delete SKS_interface_ptr;
   server_ptr->close();
}

// ==========================================================================
// SKS DataServer communication member functions
// ==========================================================================

void SAMServer::establish_SKSDataServer_connection(string URL)
{
   SKSDataServer_URL=URL;
//   SKSDataServer_IP="155.34.125.216";		// touchy dataserver
//   SKSDataServer_IP="155.34.135.168";		// dsherrill dataserver

   if (SKSDataServer_URL.size()==0)
   {
      cout << "!!!!!!!!!!!!!!!!!!" << endl;
      cout << "Error in SAMServer::establish_SKSDataServer_connection()" 
           << endl;
      cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;
      cout << "!!!!!!!!!!!!!!!!!!" << endl;
   }
//   cout << "SKSDataServer_URL = " << SKSDataServer_URL << endl;
//   int port=8080;
//   SKS_interface_ptr=new SKSDataServerInterfacer(SKSDataServer_IP,port);
   SKS_interface_ptr=new SKSDataServerInterfacer(SKSDataServer_URL);
}		       

// ---------------------------------------------------------------------
// Member function query_SKS_DataServer takes in a query string and
// wraps it into an http GET command.  It then instantiates a
// WebClient object on the stack and sets its GET request.  After
// waiting for the SKS DataServer to return its http output with a
// terminal sentinel, this method strips out just the XML body of the
// SKS DataServer's message.  It finally loads the XML message into
// the SKS_interface's DOM.

string SAMServer::query_SKS_DataServer(string curr_query)
{
//   cout << "inside SAMServer::query_SKS_DataServer()" << endl;
//   cout << "query = " << curr_query << endl;

   string GET_command="GET "+curr_query+" HTTP/1.0\r\n\r\n";
   
   string SKSDataServer_IP=stringfunc::prefix(
      SKSDataServer_URL,":");
   WebClient curr_DataClient(SKSDataServer_IP,8080);
   curr_DataClient.set_GET_command(GET_command);

// On 3/18/08, Ross Anderson taught us that the main Qt event loop
// needs to be explicitly told to continue processing while we're
// waiting for the asynchronous WebClient GET request to be handled by
// the SKS DataServer.  Here we force the main Qt loop to continue
// processing until a tag indicating XML message completion has been
// received by the WebClient from the SKS DataServer:

   string XML_tag="</response>";
   while (!curr_DataClient.returned_output_contains_substring(XML_tag))
   {
      qApp->processEvents();
   }

   string XML_content=stringfunc::XML_content_between_tags(
      curr_DataClient.get_returned_output(),"response");
//   cout << "XML_content = " << XML_content << endl;
   SKS_interface_ptr->load_returned_XML_into_DOM(XML_content);
//   cout << "At end of SAMServer::query_SKS_DataServer()" << endl;
   return XML_content;
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QString SAMServer::get( const QUrl& url )
{
   cout << "inside SAMServer:get()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );
   string URL_path;
   return get(doc,response,url,URL_path);
}

// ---------------------------------------------------------------------
QString SAMServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path)
{
   cout << "inside SAMServer:get()" << endl;
   doc.appendChild( response );

   URL_path=url.path().toStdString();
   cout << "URL path = " << URL_path << endl;
    
// Display key/value items attached to URL:

   vector<pair<string,string> > KeyValue;

   typedef QPair<QString, QString> Pair;
   QList<Pair> items = url.queryItems();
   foreach( Pair item, items ) 
      {
         string key=item.first.toStdString();
         string value=item.second.toStdString();
         cout << "key = " << key << " value = " << value << endl;

         pair<string,string> P(key,value);
         KeyValue.push_back(P);
      }

   if (URL_path=="/SAM_systems/")
   {
      process_SAM_system_queries(KeyValue,doc,response);
   }
   else if (URL_path=="/Countries/")
   {
      process_country_queries(KeyValue,doc,response);
   }
   else if (URL_path=="/SAM_specifications/")
   {
      process_SAM_specs_queries(KeyValue,doc,response);
   }
   else if (URL_path=="/SAM_sites/")
   {
      process_SAM_site_queries(KeyValue,doc,response);
   }

   return doc.toString();
}

// ---------------------------------------------------------------------
// Member function post() takes in header url as well as main body
// postData extracted via WebServer::readSocket().  This method
// decodes the post data and converts it to an STL string.  It then
// extracts and simplifies XML content of interest within the post
// data.

QString SAMServer::post( const QUrl& url, const QByteArray& postData )
{
//   cout << "inside SAMServer::post()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );

   string URL_path;
   SAMServer::get(doc,response,url,URL_path);

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
      process_flight_path_queries(XML_content,doc,response);
   }
    
   return doc.toString();
}

// ==========================================================================
// SAM specific query member functions
// ==========================================================================

// Member function process_SAM_system_queries takes in a key-value pair
// and generates XML plus KML output. It queries SAM type.

void SAMServer::process_SAM_system_queries(
   const vector<pair<string,string> >& KeyValue,
   QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::process_SAM_system_queries()" << endl;
 
/*
   double range=100;	// kms
   int IOC=1975;
   
   string query1=sks_interface.form_all_SAM_attributes_query();
   string query2=sks_interface.form_particular_SAM_attributes_query("SA-4");
   string query3=sks_interface.form_filtered_SAM_range_query(range);
   string query4=sks_interface.form_filtered_SAM_IOC_query(IOC);
   string query5=sks_interface.form_filtered_SAM_range_and_IOC_query(
      range,IOC);
   string query6=sks_interface.form_countries_owning_SAM_query("SA-10");
   string query7=sks_interface.form_SAM_sites_query("SA-4");
*/

   if (KeyValue[0].first=="sam_type" && KeyValue[0].second != "Unspecified")
   {
      string SAM_name=KeyValue[0].second;
//      cout << "SAM_name = " << SAM_name << endl;
      string query=SKS_interface_ptr->
         form_particular_SAM_attributes_query(SAM_name);
      cout << "query = " << query << endl;
      generate_XML_and_KML_query_output(query,doc,response);
   }  // key = sam_type conditional
}

// ---------------------------------------------------------------------
// Member function process_country_queries takes in a key-value pair
// and generates XML plus KML output.  It generates a list of SAMs
// residing within the queried country.

void SAMServer::process_country_queries(
   const vector<pair<string,string> >& KeyValue,
   QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::process_country_queries()" << endl;

   if (KeyValue[0].first=="country" && KeyValue[0].second != "Unspecified")
   {
      string country_name=KeyValue[0].second;
//      cout << "country_name = " << country_name << endl;

      vector<SAM*> matching_SAMs_ptrs=SAMs_group_ptr->
         SAMs_in_particular_country(country_name);

      vector<SAM*> representative_SAMs_ptrs=SAMs_group_ptr->
         representative_SAMs_in_particular_country(country_name);
      output_SAM_list(representative_SAMs_ptrs,doc,response);  

      vector<string> country_owner_names;
      country_owner_names.push_back(country_name);
      output_country_list(country_owner_names,doc,response);  

      output_site_list(matching_SAMs_ptrs,doc,response);

// Generate KML file illustrating queried country's border:

      SAMs_group_ptr->generate_owner_country_KML_file(country_name);

// Generate KML file illustrating all SAM sites within queried country:

      SAMs_group_ptr->generate_SAM_sites_KML_file(
         matching_SAMs_ptrs,colorfunc::red);

   } // key==country conditional

}

// ---------------------------------------------------------------------
// Member function process_SAM_specs_queries takes in a key-value pair
// and generates XML plus KML output.  It determines the SAMS
// satisfying maximum range, maximum altitude and IOC constraints.

void SAMServer::process_SAM_specs_queries(
   const vector<pair<string,string> >& KeyValue,
   QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::process_SAM_specs_queries()" << endl;

   if ( (KeyValue[0].first=="range" && KeyValue[0].second != "Unspecified") ||
       (KeyValue[1].first=="altitude" && KeyValue[1].second != "Unspecified") 
        ||
       (KeyValue[2].first=="ioc" && KeyValue[2].second != "Unspecified") )
   {
      double max_range=0;
      double max_altitude=0;
      int IOC=0;
      if (KeyValue[0].second != "Unspecified")
      {
         max_range=stringfunc::string_to_number(KeyValue[0].second);
      }
      if (KeyValue[1].second != "Unspecified")
      {
         max_altitude=stringfunc::string_to_number(KeyValue[1].second);
      }
      if (KeyValue[2].second != "Unspecified")
      {
         IOC=stringfunc::string_to_integer(KeyValue[2].second);
      }
      
      cout << "max range = " << max_range << endl;
      cout << "max altitude = " << max_altitude << endl;
      cout << "IOC = " << IOC << endl;
      string query=SKS_interface_ptr->
         form_filtered_SAM_range_altitude_IOC_query(
            max_range,max_altitude,IOC);
      generate_XML_and_KML_query_output(query,doc,response);
   } // range, altitude & IOC query conditional
}

// ---------------------------------------------------------------------
// Member function process_SAM_site_queries takes in the ID for some
// specific SAM site.  It generates a URL containing SAM parameters
// appropriate for the selected site which can be parsed by the PHP
// scripting language.  PHP dynamically generates html pages by
// filling variables within a template.  On 3/14/08, Dave Ceddia
// installed PHP onto touchy so that it is automaticallly invoked by
// Apache when the URL returned by this method is submitted to a
// browser.

void SAMServer::process_SAM_site_queries(
   const vector<pair<string,string> >& KeyValue,
   QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::process_SAM_site_queries()" << endl;

/*
   cout << "KeyValue.size = " << KeyValue.size() << endl;
   string key,value;
   for (int s=0; s<KeyValue.size(); s++)
   {
      key=KeyValue[s].first;
      value=KeyValue[s].second;
      cout << "s = " << s << " key = " << key 
           << " value = " << value << endl;
   }
*/

   string key=KeyValue[0].first;
   string value=KeyValue[0].second;
   if (key=="siteID")
   {
      string site_ID=value;
//      cout << "site_ID = " << site_ID << endl;

      int queried_ID=stringfunc::string_to_number(site_ID);
      cout << "queried ID = " << queried_ID << endl;
      SAM* curr_SAM_ptr=SAMs_group_ptr->get_SAM_ptr(queried_ID);
      if (curr_SAM_ptr != NULL)
      {
         cout << "Queried SAM = " << *curr_SAM_ptr << endl;
         string dynamic_URL=curr_SAM_ptr->generate_dynamic_URL();
         QUrl dynamic_QURL(QString(dynamic_URL.c_str()));
         cout << "dynamic_URL = " << dynamic_URL << endl;

// Output dynamic URL to XML output:
         
         QDomElement URL_list_element=doc.createElement("URL_list");  
         response.appendChild(URL_list_element);

         QDomElement URL_element=doc.createElement("URL");
         URL_list_element.appendChild(URL_element);

         string encoded_URL=dynamic_QURL.toEncoded().constData();
         URL_element.appendChild(doc.createTextNode(
            QString( encoded_URL.c_str() )));

// Execute http call to Michael Yee's launcher web applet to
// dynamically generate SAM web page.  Explicitly change ampersands
// within secondary dynamic_URL to %26 in order to avoid their being
// parsed within primary launch_URL:

         QString launch_URL=
            QString("/LaunchService/launch?url=%1").
            arg(QString(dynamic_QURL.toEncoded()).replace("&","%26"));
         cout << "launch_URL = "
              << launch_URL.toStdString() << endl;
         
         if (HTMLServer_URL.size()==0)
         {
            cout << "Error in SAMServer::process_SAM_site_queries()" << endl;
            cout << "HTMLServer_URL = " << HTMLServer_URL << endl;
            return;
         }
         
         QString HTMLServer_hostname(
            (stringfunc::get_hostname_from_URL(HTMLServer_URL)).c_str());
         qint16 HTMLServer_portnumber=stringfunc::get_portnumber_from_URL(
            HTMLServer_URL);
         http_client.setHost(HTMLServer_hostname,HTMLServer_portnumber);
//         http_client.setHost(QString(HTMLServer_IP.c_str()),8080);    
         http_client.get(launch_URL);
      }
      else
      {
         cout << "No SAM corresponding to queried ID exists!" << endl;
      }
   } // key == Entity_ID conditional
}

// ---------------------------------------------------------------------
// Member function process_flight_path_queries takes in an XML string
// which is assumed to have the following form:

/*
<polyline>
<point Longitude="120.838659" Latitude="23.166451765" Altitude="0" />
<point Longitude="119.125" Latitude="23.685" Altitude="0" />
<point Longitude="117.128" Latitude="24.791" Altitude="0" />
<point Longitude="116.839" Latitude="26.003" Altitude="0" />
<point Longitude="118.342" Latitude="26.355" Altitude="0" />
</polyline>
*/

// After parsing the longitudes, latitudes and altitudes for each
// point, this method reconstructs the flight path's polyline. It
// first generates a KML file for display within Google Earth.  

// This member function next computes the path's total length as well
// as SAM site closest to the path.  This method returns these flight
// path analysis parameters within the output XML document.

void SAMServer::process_flight_path_queries(
   string XML_string,QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::process_flight_path_queries()" << endl;
   
   DOMParser parser;
   parser.read_XML_string_into_DOM(XML_string);

   vector<threevector> vertices;
   vector<QDomElement> point_elements=parser.find_elements("point");

   for (int e=0; e<point_elements.size(); e++)
   {
      QDomElement curr_element=point_elements[e];

      threevector curr_vertex;
      string curr_long_str,curr_lat_str,curr_alt_str;

      if (parser.extract_attribute_value_from_element(
         curr_element,"Longitude",curr_long_str));
      {
         double curr_longitude=
            stringfunc::string_to_number(
               stringfunc::remove_trailing_whitespace(curr_long_str));
         curr_vertex.put(0,curr_longitude);
      }

      if (parser.extract_attribute_value_from_element(
         curr_element,"Latitude",curr_lat_str));
      {
         double curr_latitude=
            stringfunc::string_to_number(
               stringfunc::remove_trailing_whitespace(curr_lat_str));
         curr_vertex.put(1,curr_latitude);
      }

      if (parser.extract_attribute_value_from_element(
         curr_element,"Altitude",curr_alt_str));
      {
         double curr_altitude=
            stringfunc::string_to_number(
               stringfunc::remove_trailing_whitespace(curr_alt_str));
         curr_vertex.put(2,curr_altitude);
      }
      vertices.push_back(curr_vertex);
   } // loop over index e labeling point elements

   polyline flight_path(vertices);
//   cout << "flight_path = " << flight_path << endl;

   int closest_SAM_ID;
   double flight_path_distance_in_kms,min_distance_to_SAM_in_kms;
   SAMs_group_ptr->analyze_flight_path(
      flight_path,flight_path_distance_in_kms,
      min_distance_to_SAM_in_kms,closest_SAM_ID);
   
// Write flight path analysis results to output XML document:

/*
   QDomElement FlightPath_element = doc.createElement( "FlightPath" );
   response.appendChild(FlightPath_element);
   FlightPath_element.appendChild( doc.createTextNode( 
      QString(XML_string.c_str())));
*/

   QDomElement PathLength_element=doc.createElement("PathLength");
   response.appendChild(PathLength_element);
   PathLength_element.setAttribute(
      "LengthInKilometers",QString(
         stringfunc::number_to_string(flight_path_distance_in_kms).c_str()));

   QDomElement ClosestApproach_element=doc.createElement(
      "ClosestApproachDistance");
   response.appendChild(ClosestApproach_element);
   ClosestApproach_element.setAttribute(
      "DistanceInKilometers",QString(
         stringfunc::number_to_string(min_distance_to_SAM_in_kms).c_str()));

   QDomElement ClosestSAM_element=doc.createElement("Closest_SAM");
   response.appendChild(ClosestSAM_element);
   ClosestSAM_element.setAttribute(
      "ID",QString(stringfunc::number_to_string(closest_SAM_ID).c_str()));
}

// ---------------------------------------------------------------------
// Member function collate_country_owners takes in an STL vector
// containing SAM names.  It loops over each SAM and performs an SKS
// DataServer query for its country owners.  This method performs a
// brute-force distillation and returns an entire set of country
// owners with no repeating members.  This method should someday be
// simplified by refinements to be made by Delsey and Michael within
// their SKS DataServer meta-language.

vector<string> SAMServer::collate_country_owners(string SAM_name)
{
   vector<string> SAM_names;
   SAM_names.push_back(SAM_name);
   return collate_country_owners(SAM_names);
}

vector<string> SAMServer::collate_country_owners(
   const vector<string>& SAM_names)
{
   vector<string> country_owner_names;

   for (int n=0; n<SAM_names.size(); n++)
   {
      string query=SKS_interface_ptr->form_countries_owning_SAM_query(
         SAM_names[n]);
      query_SKS_DataServer(query);

      vector<string> curr_SAM_country_owner_names;
      SKS_interface_ptr->extract_named_values_from_attributes(
         "name",curr_SAM_country_owner_names);

// Append current names of SAM country owners onto country_owner_names
// if they don't already reside within the output STL vector:

      for (int c=0; c<curr_SAM_country_owner_names.size(); c++)
      {
         string curr_SAM_country_owner=curr_SAM_country_owner_names[c];
         bool name_already_in_list=false;
         for (int i=0; i<country_owner_names.size(); i++)
         {
            if (curr_SAM_country_owner==country_owner_names[i])
            {
               name_already_in_list=true;
            }
         }

         if (!name_already_in_list)
         {
            country_owner_names.push_back(curr_SAM_country_owner);
         }
      } // loop over index c labeling curr SAM country owners

   } // loop over index n labeling SAM names

   cout << "Country owner names returned from SKS DataServer = " << endl;
   templatefunc::printVector(country_owner_names);

   return country_owner_names;
}

// ---------------------------------------------------------------------
// Member function retrieve_SAM_sites

void SAMServer::retrieve_SAM_sites(string SAM_name)
{
//   cout << "inside SAMServer::retrieve_SAM_sites()" << endl;

   string curr_query=SKS_interface_ptr->form_SAM_sites_query(SAM_name);
   string XML_content=query_SKS_DataServer(curr_query);
//   cout << "Curr SKS query = " << curr_query << endl;
//   cout << "XML response = " << XML_content << endl;

   vector<string> SAM_names,max_ranges,geometries,longitudes,latitudes;
   SKS_interface_ptr->extract_named_values_from_attributes(
      "name",SAM_names);
   SKS_interface_ptr->extract_named_values_from_attributes(
      "range.max.km",max_ranges);

   SKS_interface_ptr->extract_named_geometries_from_coverages(
      "site",geometries);
   vector<threevector> site_posns=SKS_interface_ptr->extract_points(
      geometries);

   for (int s=0; s<site_posns.size(); s++)
   {
      longitudes.push_back(stringfunc::number_to_string(
         site_posns[s].get(1)));
      latitudes.push_back(stringfunc::number_to_string(
         site_posns[s].get(0)));
   }

   cout << "SAM names:" << endl;
   templatefunc::printVector(SAM_names);
   cout << "Max ranges" << endl;
   templatefunc::printVector(max_ranges);
//   cout << "Site posns:" << endl;
//   templatefunc::printVector(site_posns);
   cout << "Longitudes" << endl;
   templatefunc::printVector(longitudes);
   cout << "Latitudes" << endl;
   templatefunc::printVector(latitudes);
}

// ==========================================================================
// XML output member functions
// ==========================================================================

// Member function generate_XML_and_KML_query_output takes in some
// query for the SKS DataServer.  It then transforms the DataServer's
// response into simplified XML output which can be more easily read
// by thick and thin clients.  It also generates KML files for
// periodic redisplay within Google Earth.

void SAMServer::generate_XML_and_KML_query_output(
   string query,QDomDocument& doc,QDomElement& response)
{
   string XML_content=query_SKS_DataServer(query);
//   cout << "XML response = " << XML_content << endl;

   vector<string> SAM_names=output_SAM_list(doc,response);

   generate_SAM_owner_country_output(SAM_names,doc,response);
   generate_SAM_sites_output(SAM_names,doc,response);
}

// ---------------------------------------------------------------------
// Member function output_SAM_list adds fixed SAM attribute
// information to the output XML document for each input SAM
// representative.

vector<string> SAMServer::output_SAM_list(
   QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::output_SAM_list()" << endl;

   vector<string> SAM_names,max_ranges,max_altitudes,warhead_weights,
      max_speeds,ioc_years;
   SKS_interface_ptr->extract_named_values_from_attributes(
      "name",SAM_names);
   SKS_interface_ptr->extract_named_values_from_attributes(
      "range.max.km",max_ranges);
   SKS_interface_ptr->extract_named_values_from_attributes(
      "ioc.year",ioc_years);
   SKS_interface_ptr->extract_named_values_from_attributes(
      "altitude.max.km",max_altitudes);
   SKS_interface_ptr->extract_named_values_from_attributes(
      "warhead.weight.kg",warhead_weights);
   SKS_interface_ptr->extract_named_values_from_attributes(
      "speed.max.mps",max_speeds);

/*
      cout << "SAM names:" << endl;
      templatefunc::printVector(SAM_names);
      cout << "Max ranges" << endl;
      templatefunc::printVector(max_ranges);
      cout << "Max altitudes" << endl;
      templatefunc::printVector(max_altitudes);
      cout << "IOC years" << endl;
      templatefunc::printVector(ioc_years);
      cout << "Warhead weights" << endl;
      templatefunc::printVector(warhead_weights);
      cout << "Max speeds" << endl;
      templatefunc::printVector(max_speeds);
*/

   QDomElement sam_list_element=doc.createElement("SAM_list");   
   response.appendChild(sam_list_element);
   int n_outputs=min(SAM_names.size(),max_ranges.size(),max_altitudes.size(),
                     warhead_weights.size(), max_speeds.size());
//   cout << "n_outputs = " << n_outputs << endl;

   for (int t=0; t<n_outputs; t++)
   {
      QDomElement sam_type_element=doc.createElement("SAM");
      sam_list_element.appendChild(sam_type_element);
      sam_type_element.appendChild(doc.createTextNode(
         QString(SAM_names[t].c_str())));

//      cout << "t = " << t << endl;
//      cout << "SAM_name = " << SAM_names[t] << endl;
//      cout << "range = " << max_ranges[t] << endl;
//      cout << "alt = " << max_altitudes[t] << endl;
//      cout << "IOC = " << ioc_years[t] << endl;
//      cout << "weight = " << warhead_weights[t] << endl;
//      cout << "max_speeds = " << max_speeds[t] << endl;
    
      check_for_unknown_values(SAM_names);
      check_for_unknown_values(max_ranges);
      check_for_unknown_values(max_altitudes);
      check_for_unknown_values(ioc_years);
      check_for_unknown_values(warhead_weights);
      check_for_unknown_values(max_speeds);

      sam_type_element.setAttribute(
         "RangeInKms",QString(max_ranges[t].c_str()));
      sam_type_element.setAttribute(
         "MaxTargetAltitudeInKms",QString(max_altitudes[t].c_str()));
      sam_type_element.setAttribute(
         "IOCYear",QString(ioc_years[t].c_str()));
      sam_type_element.setAttribute(
         "WarheadWeightInKgs",QString(warhead_weights[t].c_str()));
      sam_type_element.setAttribute(
         "MaxSpeedInMetersPerSec",QString(max_speeds[t].c_str()));

   } // loop over index t labeling SAM types

   return SAM_names;
}

// ---------------------------------------------------------------------
// Member function check_for_unknown_values searches for -1 sentinel
// strings coming from SKS DataServer output.  If found, it replaces
// these strings with "Unknown" tags.

void SAMServer::check_for_unknown_values(vector<string>& values)
{
   for (int v=0; v<values.size(); v++)
   {
      if (values[v]=="-1")
      {
         values[v]="Unknown";
      }
   }
}

// ---------------------------------------------------------------------
void SAMServer::output_SAM_list(
   const vector<SAM*>& representative_SAM_ptrs,
   QDomDocument& doc,QDomElement& response)
{
   QDomElement sam_list_element=doc.createElement("SAM_list");   
   response.appendChild(sam_list_element);

   for (int t=0; t<representative_SAM_ptrs.size(); t++)
   {
      QDomElement sam_type_element=doc.createElement("SAM");
      sam_list_element.appendChild(sam_type_element);
      sam_type_element.appendChild(doc.createTextNode(
         QString(representative_SAM_ptrs[t]->get_name().c_str())));

      string IOC_str="Unknown";
      string max_range_str="Unknown";
      string max_tgt_alt_str="Unknown";
      string warhead_weight_str="Unknown";
      string max_speed_str="Unknown";
      
      int IOC=representative_SAM_ptrs[t]->get_IOC();
      double max_range=representative_SAM_ptrs[t]->get_max_range();
      double max_tgt_alt=representative_SAM_ptrs[t]->
         get_max_target_altitude();
      double warhead_weight=representative_SAM_ptrs[t]->get_warhead_weight();
      double max_speed=representative_SAM_ptrs[t]->get_max_speed();

      if (IOC > 0)
      {
         IOC_str=stringfunc::number_to_string(IOC);
      }
      if (max_range > 0)
      {
         max_range_str=stringfunc::number_to_string(max_range);
      }
      if (max_tgt_alt > 0)
      {
         max_tgt_alt_str=stringfunc::number_to_string(max_tgt_alt);
      }
      if (warhead_weight > 0)
      {
         warhead_weight_str=stringfunc::number_to_string(warhead_weight);
      }
      if (max_speed > 0)
      {
         max_speed_str=stringfunc::number_to_string(max_speed);
      }
      
      sam_type_element.setAttribute(
         "IOCYear",QString(IOC_str.c_str()));
      sam_type_element.setAttribute(
         "RangeInKms",QString(max_range_str.c_str()));
      sam_type_element.setAttribute(
         "MaxTargetAltitudeInKms",QString(max_tgt_alt_str.c_str()));
      sam_type_element.setAttribute(
         "WarheadWeightInKgs",QString(warhead_weight_str.c_str()));
      sam_type_element.setAttribute(
         "MaxSpeedInMetersPerSec",QString(max_speed_str.c_str()));

   } // loop over index t labeling SAM types
}

// ---------------------------------------------------------------------
// Member function output_country_list adds country owner information
// to output XML document:

void SAMServer::output_country_list(
   const vector<string>& country_owner_names,
   QDomDocument& doc,QDomElement& response)
{
   QDomElement countrylist_element=doc.createElement("Country_list");
   response.appendChild(countrylist_element);
   
   for (int c=0; c<country_owner_names.size(); c++)
   {
      QDomElement country_element=doc.createElement("Country");
      countrylist_element.appendChild(country_element);
      country_element.appendChild(doc.createTextNode(
         QString(country_owner_names[c].c_str())));
   } // loop over index c labeling countries
}

// ---------------------------------------------------------------------
// Member function output_site_list adds fixed SAM attributes to the
// output XML document.

void SAMServer::output_site_list(
   const vector<SAM*> matching_SAMs_ptrs,
   QDomDocument& doc,QDomElement& response)
{
//   cout << "inside SAMServer::output_site_list()" << endl;
   
   vector<int> SAM_IDs=SAMs_group_ptr->retrieve_SAM_IDs(
      matching_SAMs_ptrs);

   QDomElement sitelist_element=doc.createElement("Site_list");
   response.appendChild(sitelist_element);

   for (int s=0; s<SAM_IDs.size(); s++)
   {
      int ID=SAM_IDs[s];
      SAM* curr_SAM_ptr=SAMs_group_ptr->get_SAM_ptr(ID);
      string entity_ID=curr_SAM_ptr->get_entity_ID();

      geopoint curr_location=curr_SAM_ptr->get_site_location();
      
//      cout << "s = " << s
//           << " ID = " << ID
//           << " location = " << curr_location << endl;

      QDomElement site_element=doc.createElement("Site");
      sitelist_element.appendChild(site_element);

      site_element.setAttribute(
         "ID",QString(entity_ID.c_str()));
      site_element.setAttribute(
         "SAM",QString(curr_SAM_ptr->get_name().c_str()));
      site_element.setAttribute(
         "Longitude",QString(stringfunc::number_to_string(
            curr_location.get_longitude()).c_str()));
      site_element.setAttribute(
         "Latitude",QString(stringfunc::number_to_string(
            curr_location.get_latitude()).c_str()));
      site_element.setAttribute(
         "RangeInKms",QString( stringfunc::number_to_string(
            curr_SAM_ptr->get_max_range()).c_str() ));
      site_element.setAttribute(
         "Country",QString(curr_SAM_ptr->get_country_owner().c_str()));

   } // loop over index s labeling individual SAM sites
}

// ---------------------------------------------------------------------
// Member function generate_SAM_owner_country_output adds SAM country
// owner information to the output XML document.  It also generates a
// KML file illustrating the owner countries.

void SAMServer::generate_SAM_owner_country_output(
   const vector<string>& SAM_names,
   QDomDocument& doc,QDomElement& response)
{
   vector<string> country_owner_names=collate_country_owners(SAM_names);

//   if (country_owner_names.size() > 0)
   {
      output_country_list(country_owner_names,doc,response);

      SAMs_group_ptr->generate_owner_countries_KML_file(
         country_owner_names);
   }
}

// ---------------------------------------------------------------------
// Member function generate_SAM_sites_output adds SAM site information
// to the output XML document.  It also generates a KML file
// illustrating individual SAM sites.

void SAMServer::generate_SAM_sites_output(
   const vector<string>& SAM_names,
   QDomDocument& doc,QDomElement& response)
{
   vector<SAM*> matching_SAMs_ptrs=SAMs_group_ptr->get_matching_SAMs(
      SAM_names);
   output_site_list(matching_SAMs_ptrs,doc,response);
   SAMs_group_ptr->generate_SAM_sites_KML_file(
      matching_SAMs_ptrs,colorfunc::red);
}

