// ==========================================================================
// SAMs_group class member function definitions
// ==========================================================================
// Last updated on 3/23/08; 3/24/08; 3/25/08; 6/27/09; 4/5/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "gearth/kml_parser.h"
#include "messenger/Messenger.h"
#include "geometry/polyline.h"
#include "robots/SAMs_group.h"
#include "general/stringfuncs.h"

#include "templates/mytemplates.h"
#include "osg/osgfuncs.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void SAMs_group::allocate_member_objects()
{
}

void SAMs_group::initialize_member_objects()
{
   closest_SAM_ptr=NULL;
   messenger_ptr=NULL;

   SAM_output_KML_filename.push_back("SAM_sites.kml");
   SAM_output_KML_filename.push_back("country_borders.kml");
   SAM_output_KML_filename.push_back("closest_SAM_site.kml");
   SAM_output_KML_filename.push_back("flightpath.kml");
}

SAMs_group::SAMs_group()
{
   allocate_member_objects();
   initialize_member_objects();
}

SAMs_group::SAMs_group(Messenger* m_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   set_messenger_ptr(m_ptr);
}

SAMs_group::~SAMs_group()
{
   for (int i=0; i<get_n_SAMs(); i++)
   {
      delete SAM_ptrs[i];
   }
}

// ==========================================================================
// Set & get member functions
// ==========================================================================


// ==========================================================================
// SAM generation and propagation member functions
// ==========================================================================

SAM* SAMs_group::generate_new_SAM(int ID)
{
   cout << "=====================================================" << endl;
   cout << "inside SAMs_group::generate_new_SAM, ID = " << ID << endl;

   if (ID==-1) ID=get_n_SAMs();

   SAM* curr_SAM_ptr=new SAM(ID);
   SAM_ptrs.push_back(curr_SAM_ptr);

   return curr_SAM_ptr;
}

SAM* SAMs_group::generate_new_SAM(
   SAM::SAMType type,string country,
   double site_longitude,double site_latitude,double site_altitude,int ID)
{
//   cout << "=====================================================" << endl;
//   cout << "inside SAMs_group::generate_new_SAM, ID = " << ID << endl;

   if (ID==-1) ID=get_n_SAMs();

   SAM* curr_SAM_ptr=new SAM(ID);
   SAM_ptrs.push_back(curr_SAM_ptr);

   curr_SAM_ptr->set_SAM_type(type);
   curr_SAM_ptr->set_country_owner(country);
   curr_SAM_ptr->set_site_location(geopoint(
      site_longitude,site_latitude,site_altitude));
   curr_SAM_ptr->generate_threat_region();

   return curr_SAM_ptr;
}

// ---------------------------------------------------------------------
// Member function general_all_SAMs is a special purpose method which
// parses an ascii text file containing SAM information.  The file is
// assumed to containg 4 columns corresponding to SAM type, site
// longitude, site latitude and site altitude.

void SAMs_group::generate_all_SAMs(string SAM_list_filename)
{
   if (!filefunc::ReadInfile(SAM_list_filename))
   {
      cout << "Could not read in SAMs from input file!" << endl;
      return;
   }

/*
   for (int i=0; i<int(filefunc::text_line.size()); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);

      string site_type=substrings[0];
      SAM::SAMType curr_type=SAM::SA2;
      if (site_type=="SA-2")
      {
         curr_type=SAM::SA2;
      }
      else if (site_type=="SA-3")
      {
         curr_type=SAM::SA3;
      }
      else if (site_type=="SA-4")
      {
         curr_type=SAM::SA4;
      }
      else if (site_type=="SA-5")
      {
         curr_type=SAM::SA5;
      }
      else if (site_type=="SA-6")
      {
         curr_type=SAM::SA6;
      }
      else if (site_type=="SA-7")
      {
         curr_type=SAM::SA7;
      }
      else if (site_type=="SA-8")
      {
         curr_type=SAM::SA8;
      }
      else if (site_type=="SA-9")
      {
         curr_type=SAM::SA9;
      }
      else if (site_type=="SA-10")
      {
         curr_type=SAM::SA10;
      }
      else if (site_type=="SA-11")
      {
         curr_type=SAM::SA11;
      }
      else if (site_type=="SA-12")
      {
         curr_type=SAM::SA12;
      }
      else if (site_type=="SA-13")
      {
         curr_type=SAM::SA13;
      }
      else if (site_type=="SA-14")
      {
         curr_type=SAM::SA14;
      }
      else if (site_type=="SA-15")
      {
         curr_type=SAM::SA15;
      }
      else if (site_type=="SA-16")
      {
         curr_type=SAM::SA16;
      }
      else if (site_type=="SA-18")
      {
         curr_type=SAM::SA18;
      }
      else if (site_type=="FT-2000")
      {
         curr_type=SAM::FT2000;
      }
      else if (site_type=="FM-90")
      {
         curr_type=SAM::FM90;
      }
      else if (site_type=="HQ-2")
      {
         curr_type=SAM::HQ2;
      }
      else if (site_type=="Pegasus")
      {
         curr_type=SAM::Pegasus;
      }
      
      string country_name=substrings[1];

      int index=2;
      if (!stringfunc::is_number(substrings[index]))
      {
         country_name += " "+substrings[2];
         index++;
      }
//      cout << "country name = " << country_name << endl;

//      double site_longitude=stringfunc::string_to_number(substrings[index++]);
//      double site_latitude=stringfunc::string_to_number(substrings[index++]);
//      double site_altitude=stringfunc::string_to_number(substrings[index++]);

//      SAM* curr_SAM_ptr=generate_new_SAM(
//         curr_type,country_name,site_longitude,site_latitude,site_altitude);

//      if (curr_type==SAM::SA10 && country_name=="Russia")
//      {
//         cout << "New SAM = " << *curr_SAM_ptr << endl;
//      }
      
   } // loop over index i labeling lines within SAM_list_filename
*/

}

// ---------------------------------------------------------------------
// Member function loops over the SAMs within input STL vector
// input_SAM_ptrs.  It pushes their STL vectors of longitudes,
// latitude and latitude threevectors onto input double STL vector V.

void SAMs_group::collate_individual_region_lla_vertices( 
   int s,vector<SAM*>& input_SAM_ptrs,vector<vector<threevector> >& V)
{
//   cout << "inside SAMs_group::collate_individual_region_lla_vertices()"
//        << endl;
   SAM* curr_SAM_ptr=input_SAM_ptrs[s];
   V.push_back(curr_SAM_ptr->get_threat_region_long_lat_alt_vertices());
//      cout << "s = " << s << " V.size() = " << V.size() << endl;
//      cout << "V.back() = " << endl;
//      templatefunc::printVector(V.back());
}

// ---------------------------------------------------------------------
void SAMs_group::collate_threat_regions_lla_vertices( 
   vector<SAM*>& input_SAM_ptrs,vector<vector<threevector> >& V)
{
//   cout << "inside SAMs_group::collate_threat_regions_lla_vertices()"
//        << endl;
   for (int s=0; s<int(input_SAM_ptrs.size()); s++)
   {
      collate_individual_region_lla_vertices(s,input_SAM_ptrs,V);
   }
}

// ==========================================================================
// SAM querying member functions
// ==========================================================================

SAM::SAMType SAMs_group::get_type(string SAM_name) const
{
   if (SAM_name=="SA-2")
   {
      return SAM::SA2;
   }
   else if (SAM_name=="SA-3")
   {
      return SAM::SA3;
   }
   else if (SAM_name=="SA-4")
   {
      return SAM::SA4;
   }
   else if (SAM_name=="SA-5")
   {
      return SAM::SA5;
   }
   else if (SAM_name=="SA-6")
   {
      return SAM::SA6;
   }
   else if (SAM_name=="SA-7")
   {
      return SAM::SA7;
   }
   else if (SAM_name=="SA-8")
   {
      return SAM::SA8;
   }
   else if (SAM_name=="SA-9")
   {
      return SAM::SA9;
   }
   else if (SAM_name=="SA-10")
   {
      return SAM::SA10;
   }
   else if (SAM_name=="SA-11")
   {
      return SAM::SA11;
   }
   else if (SAM_name=="SA-12")
   {
      return SAM::SA12;
   }
   else if (SAM_name=="SA-13")
   {
      return SAM::SA13;
   }
   else if (SAM_name=="SA-14")
   {
      return SAM::SA14;
   }
   else if (SAM_name=="SA-15")
   {
      return SAM::SA15;
   }
   else if (SAM_name=="SA-16")
   {
      return SAM::SA16;
   }
   else if (SAM_name=="SA-18")
   {
      return SAM::SA18;
   }
   else if (SAM_name=="FT-2000")
   {
      return SAM::FT2000;
   }
   else if (SAM_name=="FM-90")
   {
      return SAM::FM90;
   }
   else if (SAM_name=="HQ-2")
   {
      return SAM::HQ2;
   }
   else if (SAM_name=="Pegasus")
   {
      return SAM::Pegasus;
   }
   else
   {
      return SAM::other;
   }
}

// ---------------------------------------------------------------------
// Member function get_matching_SAMs takes in a string specifying a
// particular SAM type.  It scans through the entire stored SAM list
// and culls out those which match the queried type.  This method
// returns an STL vector containing pointers to SAMs matching the
// input type.

vector<SAM*> SAMs_group::get_matching_SAMs(string SAM_name)
{
//   cout << "inside SAMs_group::get_matching_SAMs()" << endl;
   
   vector<string> SAM_names;
   SAM_names.push_back(SAM_name);
   return get_matching_SAMs(SAM_names);
}

vector<SAM*> SAMs_group::get_matching_SAMs(const vector<string>& SAM_names)
{
//   cout << "inside SAMs_group::get_matching_SAMs()" << endl;
   
   vector<SAM*> matching_SAMs_ptrs;

   for (int n=0; n<int(SAM_names.size()); n++)
   {
      SAM::SAMType queried_SAM_type=get_type(SAM_names[n]);
      for (int s=0; s<get_n_SAMs(); s++)
      {
         SAM* curr_SAM_ptr=get_SAM_ptr(s);
         if (curr_SAM_ptr->get_SAM_type()==queried_SAM_type)
         {
            matching_SAMs_ptrs.push_back(curr_SAM_ptr);
         }
      } // loop over index s labeling individual SAMs
   } // loop over index n labeling SAM names

   return matching_SAMs_ptrs;
}

// ---------------------------------------------------------------------
SAM* SAMs_group::get_representative_SAM(string SAM_name)
{
//   cout << "inside SAMs_group::get_representative_SAM()" << endl;
   vector<SAM*> matching_SAMs_ptrs=get_matching_SAMs(SAM_name);

   if (matching_SAMs_ptrs.size()==0)
   {
      return NULL;
   }
   else
   {
      return matching_SAMs_ptrs.back();
   }
}

// ==========================================================================
// Country & site query member functions
// ==========================================================================

// Member function countries_owning_particular_SAM takes in a string
// specifying a particular SAM type.  It returns an STL vector filled
// with the names of countries which own the queries SAM type.  If the
// input messenger pointer is non-null, this method also sends a
// message containing a single "countries" key paired with a value
// string with country names separated by commas.

vector<string> SAMs_group::countries_owning_particular_SAMs(string SAM_name)
{
//   cout << "inside SAMs_group::countries_owning_particular_SAMs()" << endl;

   vector<string> owner_country_names;

   if (SAM_name.size()==0) return owner_country_names;

   vector<SAM*> matching_SAMs_ptrs=get_matching_SAMs(SAM_name);
   for (int s=0; s<int(matching_SAMs_ptrs.size()); s++)
   {
      SAM* curr_SAM_ptr=matching_SAMs_ptrs[s];
      string curr_country_owner=curr_SAM_ptr->get_country_owner();
      
// Perform brute-force check whether curr_country_owner already exists
// within owner_country_names.  If not, append it to end of STL vector:

      bool country_in_list_flag=false;
      for (int c=0; c<int(owner_country_names.size()); c++)
      {
         if (curr_country_owner==owner_country_names[c]) 
         {
            country_in_list_flag=true;
         }
      } // loop over index c labeling countries owning queried SAM
      
      if (!country_in_list_flag)
      {
         owner_country_names.push_back(curr_country_owner);
      }
   } // loop over index s labeling matching_SAMs_ptrs
   
   cout << "Countries with SAM = " << SAM_name << endl;
   templatefunc::printVector(owner_country_names);
   
   if (messenger_ptr != NULL)
   {
      string command="RESPONSE";
      string key="countries";
      string value;
      for (int v=0; v<int(owner_country_names.size()); v++)
      {
         value += owner_country_names[v]+",";
      }
      messenger_ptr->sendTextMessage(command,key,value);
   }

   return owner_country_names;
}

// ---------------------------------------------------------------------
// Member function SAMs_in_particular_country performs a brute force
// search for SAMs whose country owner field matches the input country
// name.  This method will soon be replaced by some call to the SKS
// DataServer.

vector<SAM*> SAMs_group::SAMs_in_particular_country(string country_name)
{
//   cout << "inside SAMs_group::SAMs_in_particular_country()" << endl;

   vector<SAM*> matching_SAMs_ptrs;
   for (int s=0; s<get_n_SAMs(); s++)
   {
      SAM* curr_SAM_ptr=get_SAM_ptr(s);
      if (curr_SAM_ptr->get_country_owner()==country_name)
      {
         matching_SAMs_ptrs.push_back(curr_SAM_ptr);
      }
   } // loop over index s labeling individual SAMs
   return matching_SAMs_ptrs;
}

// ---------------------------------------------------------------------
vector<SAM*> SAMs_group::representative_SAMs_in_particular_country(
   string country_name)
{
//   cout << "inside SAMs_group::representative_SAMs_in_particular_country()" << endl;

   vector<SAM*> representative_SAMs_ptrs;
   for (int s=0; s<get_n_SAMs(); s++)
   {
      SAM* curr_SAM_ptr=get_SAM_ptr(s);
      if (curr_SAM_ptr->get_country_owner()==country_name)
      {

// Perform brute-force check whethehr curr_SAM_ptr's SAM type already
// exists within representative_SAMs_ptrs.  If not, push it back onto
// the STL vector

         bool SAM_in_list_flag=false;
         for (int r=0; r<int(representative_SAMs_ptrs.size()); r++)
         {
            if (curr_SAM_ptr->get_SAM_type()==
                representative_SAMs_ptrs[r]->get_SAM_type())
            {
               SAM_in_list_flag=true;
            }
         } // loop over index r labeling representative SAMs
         if (!SAM_in_list_flag) representative_SAMs_ptrs.push_back(
            curr_SAM_ptr);
      }
   } // loop over index s labeling individual SAMs
   return representative_SAMs_ptrs;
}

// ---------------------------------------------------------------------
// Member function queried_SAM_sites takes in the name for some SAM of
// interest.  It recovers the particular SAM objects corresponding to
// this name and fills STL vectors with their longitude, latitude and
// max range information.  If the input messenger_ptr is non-null,
// this method broadcasts a "site_locations" key along with a value
// string composed of longitude,latitude,radius triples.  This
// information should be enough for Ross' thick QT client to display
// SAM sites on his viewer's map.

void SAMs_group::queried_SAM_sites(const vector<SAM*>& particular_SAMs_ptrs)
{
//   if (SAM_name.size()==0) return;

//   vector<SAM*> particular_SAMs_ptrs=get_matching_SAMs(SAM_name);

   vector<double> longitudes,latitudes,ranges;
   for (int s=0; s<int(particular_SAMs_ptrs.size()); s++)
   {
      SAM* curr_SAM_ptr=particular_SAMs_ptrs[s];
      geopoint curr_posn=curr_SAM_ptr->get_site_location();
      longitudes.push_back(curr_posn.get_longitude());
      latitudes.push_back(curr_posn.get_latitude());
      ranges.push_back(curr_SAM_ptr->get_max_range());
   } // loop over index s labeling particular SAMs

   if (messenger_ptr != NULL)
   {
      string command="RESPONSE";
      string key="site_locations";
      string value;
      for (int v=0; v<int(longitudes.size()); v++)
      {
         value += stringfunc::number_to_string(longitudes[v])+","
            +stringfunc::number_to_string(latitudes[v])+","
            +stringfunc::number_to_string(ranges[v])+",";
      }
      messenger_ptr->sendTextMessage(command,key,value);
   }
}

// ---------------------------------------------------------------------
// Member function retrieve_SAM_IDs takes in a set of SAM pointers.
// It returns their IDs within an STL vector.  We wrote this method
// for XML output purposes.

vector<int> SAMs_group::retrieve_SAM_IDs(
   const vector<SAM*>& particular_SAMs_ptrs)
{
   vector<int> SAM_IDs;

   for (int s=0; s<int(particular_SAMs_ptrs.size()); s++)
   {
      SAM* curr_SAM_ptr=particular_SAMs_ptrs[s];
      SAM_IDs.push_back(curr_SAM_ptr->get_ID());
   } // loop over index s labeling particular SAMs
   
   return SAM_IDs;
}

// ==========================================================================
// KML file output member functions
// ==========================================================================

// Member function generate_empty_KML_files() flushes out the contents
// of the SAM_sites, country_borders and single_SAM_site KML files.
// This method should be called at the start of the SAM demo.

void SAMs_group::generate_empty_KML_files()
{
//   cout << "inside SAMs_group::generate_empty_KML_files()" << endl;

   kml_parser empty_kml_parser;
   for (int f=0; f<int(SAM_output_KML_filename.size()); f++)
   {
      empty_kml_parser.generate_empty_kml_file(SAM_output_KML_filename[f]);
   }                                                       
}

// ---------------------------------------------------------------------
// Member function generate_owner_countries_KML_file() generates a KML
// file with colored borders for the owner countries within the input
// STL vector.

void SAMs_group::generate_owner_country_KML_file(string country_name)
{
   vector<string> country_owner_names;
   country_owner_names.push_back(country_name);
   generate_owner_countries_KML_file(country_owner_names);
}

// This overloaded version of generate_owner_countires_KML_file takes
// in an STL vector of country owner names.  If it is empty, this
// method generates a KML file containing zero country borders which
// can still be automatically displayed in Google Earth.

void SAMs_group::generate_owner_countries_KML_file(
   vector<string> country_owner_names)
{
   kml_parser country_kml_parser;

   vector<vector<threevector> > V;
   for (int c=0; c<int(country_owner_names.size()); c++)
   {
      string simplified_country_name=
         geofunc::generate_simplified_country_name(country_owner_names[c]);
      country_kml_parser.extract_country_border_vertices(
         simplified_country_name,V);
   } // loop over index c labeling countries

   double r_border=1.0;
   double g_border=0.0;
   double b_border=1.0;
   double a_border=1.0;

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/uavs/kml_templates/";
   string polys_header_filename=subdir+"polys_header.kml";
   string polys_footer_filename=subdir+"polys_footer.kml";

   country_kml_parser.generate_country_borders_kml_file(
      V,r_border,g_border,b_border,a_border,
      polys_header_filename,polys_footer_filename,
      SAM_output_KML_filename[1]);
}

// ---------------------------------------------------------------------
// Member function generate_SAM_sites_KML_file generates a KML file
// containing sites corresponding to a queried SAM type.  The circles
// appearing within GoogleEarth are colored according to the input
// parameter.

void SAMs_group::generate_SAM_sites_KML_file(
   string query_SAM_name,colorfunc::Color curr_color,
   string output_KML_filename)
{
   if (query_SAM_name.size()==0) return;
   vector<SAM*> particular_SAMs_ptrs=get_matching_SAMs(query_SAM_name);
   generate_SAM_sites_KML_file(particular_SAMs_ptrs,curr_color,
                               output_KML_filename);
}

void SAMs_group::generate_SAM_sites_KML_file(
   vector<SAM*>& matching_SAMs_ptrs,colorfunc::Color curr_color,
   string output_KML_filename)
{
   vector<vector<threevector> > SAM_vertices;
   collate_threat_regions_lla_vertices(matching_SAMs_ptrs,SAM_vertices);

   vector<string> SAM_names,dynamic_URLs;
   for (int s=0; s<int(matching_SAMs_ptrs.size()); s++)
   {
      SAM* curr_SAM_ptr=matching_SAMs_ptrs[s];
      SAM_names.push_back(curr_SAM_ptr->get_name());

//      string curr_URL="http://touchy/rco/sam.php?site=101&system=SA-13&country=Russia&lat=53.822196 degs&lon=48.732296 degs&range=5.0 kms&max_speed=550.0 m/s&max_alt=3.0 kms&weight=3.0 kgs&ioc=1975";
//      dynamic_URLs.push_back(curr_URL);      

      dynamic_URLs.push_back(curr_SAM_ptr->generate_dynamic_URL());
   }

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/uavs/kml_templates/";
   string polys_header_filename=subdir+"polys_header.kml";
   string polys_footer_filename=subdir+"polys_footer.kml";

   const double alpha=0.7;
   osg::Vec4 RGBA=colorfunc::get_OSG_color(curr_color,alpha);
//   cout << "RGBA = " << endl;
//   osgfunc::print_Vec4(RGBA);

   if (output_KML_filename.size()==0)
   {
      output_KML_filename=SAM_output_KML_filename[0];
   }

   kml_parser threat_region_kml_parser;
   threat_region_kml_parser.generate_multi_polygons_kml_file(
      RGBA.r(),RGBA.g(),RGBA.b(),RGBA.a(),
      polys_header_filename,polys_footer_filename,
      SAM_vertices,SAM_names,dynamic_URLs,output_KML_filename);
}

// ---------------------------------------------------------------------
// Member function generate_single_SAM_site_KML_file generates a KML
// file containing a particular SAM site.

void SAMs_group::generate_single_SAM_site_KML_file(
   int SAM_ID,colorfunc::Color curr_color)
{
   vector<SAM*> matching_SAMs_ptrs;
   matching_SAMs_ptrs.push_back(get_SAM_ptr(SAM_ID));

   string KML_filename=SAM_output_KML_filename[2];
   generate_SAM_sites_KML_file(
      matching_SAMs_ptrs,curr_color,KML_filename);
}

/*
void SAMs_group::generate_single_SAM_site_KML_file(
   int SAM_ID,colorfunc::Color curr_color)
{
   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/uavs/kml_templates/";
   string polys_header_filename=subdir+"polys_header.kml";
   string polys_footer_filename=subdir+"polys_footer.kml";

   vector<SAM*> particular_SAMs_ptrs;
   particular_SAMs_ptrs.push_back(get_SAM_ptr(SAM_ID));

   vector<vector<threevector> > SAM_vertices;
   collate_threat_regions_lla_vertices(
      particular_SAMs_ptrs,SAM_vertices);

   kml_parser threat_region_kml_parser;

   const double alpha=0.7;
   osg::Vec4 RGBA=colorfunc::get_OSG_color(curr_color,alpha);
//   cout << "RGBA = " << endl;
//   osgfunc::print_Vec4(RGBA);
   
   threat_region_kml_parser.generate_multi_polygons_kml_file(
      RGBA.r(),RGBA.g(),RGBA.b(),RGBA.a(),
      polys_header_filename,polys_footer_filename,
      SAM_vertices,SAM_output_KML_filename[2]);
}
*/

// ==========================================================================
// Flight path evaluation member functions
// ==========================================================================

// Member function construct_flight_waypoints takes in polyline flight
// path containing longitude,latitude,altitude triples.  This method
// fills and returns STL vector flight_waypoints with geopoints
// constructed from the input polyline vertices.

vector<geopoint>& SAMs_group::construct_flight_waypoints(
   const polyline& flight_path)
{
//   cout << "inside SAMs_group::construct_flight_waypoints()" << endl;

   flight_waypoints.clear();
   for (unsigned int n=0; n<flight_path.get_n_vertices(); n++)
   {
      threevector lla=flight_path.get_vertex(n);
//      cout << "n = " << n << " lla = " << lla << endl;
      flight_waypoints.push_back(geopoint(lla.get(0),lla.get(1),lla.get(2)));
//      cout << "curr waypoint = " << flight_waypoints.back() << endl;
   } // loop over index n labeling flight path vertices
   return flight_waypoints;
}

// ---------------------------------------------------------------------
// Member function compute_flight_path_distance returns the total
// distance measured in meters for the input flight path assuming a
// spherical earth model.

double SAMs_group::compute_flight_path_distance(const polyline& flight_path)
{
//   cout << "inside SAMs_group::compute_flight_path_distance()" << endl;

   construct_flight_waypoints(flight_path);

   double total_distance=0;
   for (int n=0; n<int(flight_waypoints.size()-1); n++)
   {
      total_distance += 1000*geofunc::distance_between_groundpoints(
         flight_waypoints[n],flight_waypoints[n+1]);
   }

   cout << "Total flight path distance = " << total_distance/1000
        << " kilometers" << endl;

   if (messenger_ptr != NULL)
   {
      string command="RESPONSE";
      string key="flight_path_distance";
      string value=stringfunc::number_to_string(total_distance);
      messenger_ptr->sendTextMessage(command,key,value);
   }

   return total_distance;
}

// ---------------------------------------------------------------------
// Member function find_closest_SAM_site assumes that
// compute_flight_path_distance() has been previously called.  It
// takes in the flight path distance measured in meters and
// subdivideds the flight path into 10 km segments.  For each segment,
// this method performs a brute force search for the closest SAM site.
// It stores the closest SAM's pointer within member closest_SAM_ptr
// and returns the linear distance to the closest SAM in kilometers.

double SAMs_group::find_closest_SAM_site(
   polyline& flight_path,double flight_path_distance_in_meters)
{
//   cout << "inside SAMs_group::find_closest_SAM_site()" << endl;

   const double waypoint_separation=10000;	// meters
   unsigned int n_waypoints=static_cast<unsigned int>(
      flight_path_distance_in_meters/waypoint_separation+1);
//   cout << "n_waypoints = " << n_waypoints << endl;

   vector<threevector> V;
   flight_path.compute_regularly_spaced_edge_points(n_waypoints,V);
   polyline resampled_flight_path(V);
   construct_flight_waypoints(resampled_flight_path);

// Break up flight path into 10 km segments.  Compute each segment's
// distance to all SAM sites.  Save ID and distance to closest SAM:

   int closest_SAM_ID=-1;
   double min_distance=POSITIVEINFINITY;
   for (int n=0; n<int(flight_waypoints.size()); n++)
   {
      for (int s=0; s<get_n_SAMs(); s++)
      {
         SAM* curr_SAM_ptr=get_SAM_ptr(s);
         geopoint SAM_geopoint=curr_SAM_ptr->get_site_location();
         double curr_distance = geofunc::distance_between_groundpoints(
            flight_waypoints[n],SAM_geopoint);
         if (curr_distance < min_distance)
         {
            min_distance=curr_distance;
            closest_SAM_ID=curr_SAM_ptr->get_ID();
         }
      } // loop over index s labeling SAM sites
   } // loop over index n labeling flight waypoints

   closest_SAM_ptr=get_SAM_ptr(closest_SAM_ID);

   cout << "Closest SAM ID = " << closest_SAM_ID << endl;
   cout << "SAM name = " << closest_SAM_ptr->get_name() << endl;
   cout << "Min distance to SAM site in kms = " << min_distance << endl;
   cout << "Closest SAM geopoint = " << closest_SAM_ptr->get_site_location()
        << endl;

   if (messenger_ptr != NULL)
   {
      string command="RESPONSE";
      string key="closest_SAM";
      string value=stringfunc::number_to_string(min_distance)+",";
      value += stringfunc::number_to_string(closest_SAM_ID);
      messenger_ptr->sendTextMessage(command,key,value);
   }

   return min_distance;
}

// ---------------------------------------------------------------------
// Member function generate_flightpath_KML_file() takes in a polyline
// containing longitude,latitude,altitude vertices.  This method
// outputs a KML file containing a single cyan polyline representing
// the flight path.

void SAMs_group::generate_flightpath_KML_file(const polyline& flight_path)
{
   vector<threevector> vertices;
   for (unsigned int v=0; v<flight_path.get_n_vertices(); v++)
   {
      vertices.push_back(flight_path.get_vertex(v));
   } 

   kml_parser flightpath_parser;
   double r=0;
   double g=1;
   double b=1;
   double a=1;
   double width=5;
   flightpath_parser.generate_polyline_kml_file(
      r,g,b,a,width,vertices,SAM_output_KML_filename[3]);
}

// ---------------------------------------------------------------------
// Member function analyze_flight_path is a high-level method which
// takes in a polyline flight path and computes its length in
// kilometers.  It also performs a brute force search over all SAMs
// and determines which one lies closest to the flight path.  This
// method returns the impact parameter distance in kilometers as well
// as the ID for the closest SAM site to the flight path.

void SAMs_group::analyze_flight_path(
   polyline& flight_path,double& flight_path_distance_in_kms,
   double& min_distance_to_SAM_in_kms,int& closest_SAM_ID)
{
//   cout << "inside SAMs_group::analyze_flight_path()" << endl;
//   cout << "flight_path = " << flight_path << endl;

   generate_flightpath_KML_file(flight_path);

   double flight_path_distance_in_meters=
      compute_flight_path_distance(flight_path);
   flight_path_distance_in_kms=0.001*flight_path_distance_in_meters;
   min_distance_to_SAM_in_kms=find_closest_SAM_site(
      flight_path,flight_path_distance_in_meters);

   closest_SAM_ID=get_closest_SAM_ptr()->get_ID();
   colorfunc::Color danger_color=colorfunc::yellow;
   generate_single_SAM_site_KML_file(closest_SAM_ID,danger_color);

   cout << "flight path distance in kms = "
        << flight_path_distance_in_kms << endl;
   cout << "min dist to SAM in kms = " << min_distance_to_SAM_in_kms << endl;
   cout << "closest_SAM_ID = " << closest_SAM_ID << endl;
}
