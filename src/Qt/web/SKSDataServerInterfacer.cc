// ==========================================================================
// SKSDATASERVERINTERFACER class file
// ==========================================================================
// Last updated on 3/23/08; 4/28/08; 4/29/08; 6/26/08
// ==========================================================================

#include <iostream>
#include <QtXml/QtXml>
#include "SKSDataServerInterfacer.h"
#include "general/stringfuncs.h"

#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void SKSDataServerInterfacer::allocate_member_objects()
{
}		       

void SKSDataServerInterfacer::initialize_member_objects()
{
   DataServer_URL="";
}

SKSDataServerInterfacer::SKSDataServerInterfacer()
{
}

SKSDataServerInterfacer::SKSDataServerInterfacer(string URL)
{
   DataServer_URL=URL;
}

bool SKSDataServerInterfacer::load_returned_XML_into_DOM(string XML_string)
{
//   cout << "inside SKSDataServerInterfacer::load_returned_XML_into_DOM()"
//        << endl;
//   cout << "XML_string = " << XML_string << endl;
   return parser.read_XML_string_into_DOM(XML_string);
}

// ==========================================================================
// Bluegrass query member functions
// ==========================================================================

string SKSDataServerInterfacer::initial_Bluegrass_GET_request()
{
   string GET_request=
      "http://"+DataServer_URL+"/SKSDataServer/getObservations?";
   return GET_request;
}

/*
string SKSDataServerInterfacer::form_vehicle_tracks_query()
{
   double min_longitude=-101.97;
   double min_latitude=33.48;
   double max_longitude=-101.91;
   double max_latitude=33.53;
   double t_start=1190908800;
   double t_stop=1190912400;
   return form_vehicle_tracks_query(min_longitude,min_latitude,
                                    max_longitude,max_latitude,
                                    t_start,t_stop);
}
*/

string SKSDataServerInterfacer::form_vehicle_tracks_query(
   double min_longitude,double min_latitude,
   double max_longitude,double max_latitude,
   long long t_start,long long t_stop)
{
   string GET_request=initial_Bluegrass_GET_request();
   GET_request += "mode=all";
   GET_request += "&bbox="+stringfunc::number_to_string(min_longitude)
      +","+stringfunc::number_to_string(min_latitude)
      +","+stringfunc::number_to_string(max_longitude)
      +","+stringfunc::number_to_string(max_latitude);
   GET_request += "&t0="+stringfunc::number_to_string(t_start)
      +"&t1="+stringfunc::number_to_string(t_stop);
   GET_request += "&types=vehicle";
   GET_request += "&labelfield=id_bg";
   GET_request += "&";
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

// ==========================================================================
// SAM query member functions
// ==========================================================================

string SKSDataServerInterfacer::initial_SAM_GET_request()
{
   string GET_request=
      "http://"+DataServer_URL+"/SKSDataServer/query?";
   return GET_request;
}

// Member function 

string SKSDataServerInterfacer::form_all_SAM_attributes_query()
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_particular_SAM_attributes_query(
   string SAM_name)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
   GET_request += "&filter=sam/name='"+SAM_name+"'";
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_filtered_SAM_range_query(
   double range)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
   GET_request += "&filter=sam/range.max.km::float>="
      +stringfunc::number_to_string(range);
//    cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_filtered_SAM_altitude_query(
   double altitude)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
   GET_request += "&filter=sam/altitude.max.km::float>="
      +stringfunc::number_to_string(altitude);
//    cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_filtered_SAM_range_altitude_IOC_query(
   double range,double altitude,int IOC)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
   GET_request += "&filter=sam/range.max.km::float>=";
   GET_request += stringfunc::number_to_string(range);
   GET_request += ",sam/altitude.max.km::float>="
      +stringfunc::number_to_string(altitude);
   GET_request += ",sam/ioc.year::int>="+stringfunc::number_to_string(IOC);
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_filtered_SAM_IOC_query(int IOC)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
   GET_request += "&filter=sam/ioc.year::int>="
      +stringfunc::number_to_string(IOC);
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_filtered_SAM_range_and_IOC_query(
   double range,int IOC)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=*/*";
   GET_request += "&filter=sam/range.max.km::float>=";
   GET_request += stringfunc::number_to_string(range);
   GET_request += ",sam/ioc.year::int>="
      +stringfunc::number_to_string(IOC);
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_countries_owning_SAM_query(
   string SAM_name)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=sam/{country|country:owns(this)}/name";
   GET_request += "&filter=sam/name='"+SAM_name+"'";
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

string SKSDataServerInterfacer::form_SAM_sites_query(string SAM_name)
{
   string GET_request=initial_SAM_GET_request();
   GET_request += "type=sam";
   GET_request += "&attrib=sam/range.max.km,sam/site,sam/name";
   GET_request += "&filter=sam/name=";
   GET_request += "'"+SAM_name+"'";
//   cout << "GET_request = " << GET_request << endl;
   return GET_request;
}

// ==========================================================================
// Response parsing member functions
// ==========================================================================

// Member function extract_named_values_from_attributes takes in the
// name_field for some attribute.  If an attribute with that name
// actually exists within the XML returned by the SKS DataServer, this
// boolean method returns true as well as the values corresponding to
// the name in the output STL vector of strings.  Trailing white
// spaces are removed from the named values.

// As of 6:30 pm on Friday, Mar 21, we strongly suspect that this next
// member function is too sensitive to white spaces appearing within
// SKS DataServer entries.  Delsey needs to reduce this white space
// sensitivity on her end.  But we should also try to combat it within
// DomParser...

bool SKSDataServerInterfacer::extract_named_values_from_attributes(
   string name_field,vector<string>& named_values)
{
//   cout << "inside SKSDataServerInterfacer::extract_named_value_from_attributes()" << endl;

   named_values.clear();
   vector<QDomElement> attribute_elements=parser.find_elements("attribute");

   for (int e=0; e<attribute_elements.size(); e++)
   {
      QDomElement curr_element=attribute_elements[e];
      if (parser.element_has_specified_attribute_key_value_pair(
         curr_element,"name",name_field))
      {
         string curr_value;
         if (parser.extract_attribute_value_from_element(
            curr_element,"value",curr_value))
         {
            named_values.push_back(stringfunc::remove_trailing_whitespace(
               curr_value));
         }
         else
         {
            named_values.push_back("Unknown");
         }
      }
      else
      {
//         cout << "Element does NOT have specified attribute key value pair"
//              << endl;
//         cout << "name_field = " << name_field << endl;
//         named_values.push_back("Unknown");
      }
   } // loop over index e labeling input QDomElements
   return (named_values.size() > 0);
}

// ---------------------------------------------------------------------
// Member function extract_named_geometries_from_coverages takes in
// the name for some SKS Database Coverage entry.  If any geometries
// are associated with this Coverage, this boolean method returns true
// as well as the geometry strings within the output STL vector.

bool SKSDataServerInterfacer::extract_named_geometries_from_coverages(
   string name_field,vector<string>& geometries)
{
//   cout << "inside SKSDataServerInterfacer::extract_named_geometries_from_SKSDataServer_coverages()" << endl;
   
   bool geometry_found_flag=false;
   vector<QDomElement> coverage_elements=parser.find_elements("coverage");
   for (int e=0; e<coverage_elements.size(); e++)
   {
      QDomElement curr_element=coverage_elements[e];
      if (parser.element_has_specified_attribute_key_value_pair(
         curr_element,"name",name_field))
      {
         string curr_geometry;
         if (parser.extract_attribute_value_from_element(
            curr_element,"geometry",curr_geometry))
         {
            geometries.push_back(curr_geometry);
            geometry_found_flag=true;
         }
      }
   } // loop over index e labeling input QDomElements
   return geometry_found_flag;
}

// ---------------------------------------------------------------------
// Member function extract_points takes in an STL vector of SKS
// DataServer geometry strings.  If the geometries correspond to SKS
// DataServer Point objects, this method extracts the points'
// longitude and latitude coordinates and returns them within the
// output STL vector.

vector<threevector> SKSDataServerInterfacer::extract_points(
   const vector<string>& geometries)
{
   vector<threevector> lla;
   
   for (int g=0; g<geometries.size(); g++)
   {
      string curr_geometry=geometries[g];
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         curr_geometry);

//      templatefunc::printVector(substrings);
      
      if (substrings[0]=="SRID=4326;POINT")
      {
         string longitude_str=substrings[1].substr(
            1,substrings[1].length()-1);
         string latitude_str=substrings[2].substr(
            0,substrings[2].length()-1);
//         cout << "long_str = " << longitude_str << endl;
//         cout << " lat_str = " << latitude_str << endl;

         threevector curr_lla(stringfunc::string_to_number(
            longitude_str),stringfunc::string_to_number(latitude_str));
         lla.push_back(curr_lla);
      }
   } // loop over index g labeling geometry lines

//   templatefunc::printVector(lla);
   return lla;
}
