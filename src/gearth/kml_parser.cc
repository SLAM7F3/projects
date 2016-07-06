// ==========================================================================
// Kml_Parser class member function definitions
// ==========================================================================
// Last modified on 3/25/08; 4/28/11; 7/5/11; 4/5/14
// ==========================================================================

#include <vector>
#include "ogrsf_frmts.h"

#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "gearth/kml_parser.h"
#include "geometry/polygon.h"
#include "osg/osgGIS/postgis_database.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "track/track.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void kml_parser::allocate_member_objects()
{
}		       

void kml_parser::initialize_member_objects()
{
   PostGIS_database_ptr=NULL;
   filled_polygon_area_flag=true;
   polyline_width=10;
}

kml_parser::kml_parser(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

kml_parser::kml_parser(postgis_database* pgdb_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PostGIS_database_ptr=pgdb_ptr;
}

// Copy constructor:

kml_parser::kml_parser(const kml_parser& k)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(k);
}

kml_parser::~kml_parser()
{
}

// ---------------------------------------------------------------------
void kml_parser::docopy(const kml_parser& k)
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const kml_parser& k)
{
   outstream << endl;
   return(outstream);
}

// ==========================================================================
// Polyline parsing member functions
// ==========================================================================

// Member function parse_polyline_kml reads in a KML file assumed to
// contain flight path information manually entered via Google Earth.
// This method extracts the flight path information and stores it
// within member polyline object curr_polyline.

polyline& kml_parser::parse_polyline_kml(string path_filename)
{
   filefunc::ReadInfile(path_filename);

   bool parse_text_flag=false;
   vector<string> polyline_text_lines;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      if (parse_text_flag)
      {
         polyline_text_lines.push_back(filefunc::text_line[i]);
//         cout << filefunc::text_line[i] << endl;
      }
      
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      for (unsigned int s=0; s<substrings.size(); s++)
      {
         string curr_substring=substrings[s];
         if (curr_substring=="<coordinates>")
         {
//             cout << "<coordinates> found!" << endl;
            parse_text_flag=true;
         }
         else if (curr_substring=="</coordinates>")
         {
//             cout << "</coordinates> found!" << endl;
            parse_text_flag=false;
         }
      } // loop over index s labeling substrings within curr text line
   } // loop over index i labeling text lines within input kml file

   vector<string> polyline_substrings;
   vector<threevector> vertices;
   for (unsigned int p=0; p<polyline_text_lines.size(); p++)
   {
//       cout << polyline_text_lines[p] << endl;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         polyline_text_lines[p]);
      for (unsigned int s=0; s<substrings.size()-1; s++)
      {
         string curr_substring=substrings[s];
//         cout << "p = " << p 
//              << " s = " << s << " substring = " << substrings[s] << endl;
         vector<string> sub_substrings=
            stringfunc::decompose_string_into_substrings(curr_substring,",");
//         for (unsigned int l=0; l<sub_substrings.size(); l++)
//         {
//            cout << "l = " << l << " sub_substring = " << sub_substrings[l]
//                 << endl;
//         }
         threevector curr_vertex(
            stringfunc::string_to_number(sub_substrings[0]),
            stringfunc::string_to_number(sub_substrings[1]),
            stringfunc::string_to_number(sub_substrings[2]));
         vertices.push_back(curr_vertex);
//         cout << "curr_vertex = " << vertices.back() << endl;

      } // loop over index s labeling substrings
   } // loop over index p labeling polyline text lines

   curr_polyline=polyline(vertices);
//   cout << "curr_polyline = " << curr_polyline << endl;
   return curr_polyline;
}

// ==========================================================================
// KML file generation member functions
// ==========================================================================

void kml_parser::generate_empty_kml_file(string output_kml_filename)
{
//   cout << "inside kml_parser::generate_empty_kml_file()" << endl;
   ofstream outstream;

   if (!filefunc::openfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_empty_kml_file()" << endl;
      cout << "Cannot open output empty kml file" << endl;
      return;
   }

   outstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   outstream << "<kml xmlns=\"http://earth.google.com/kml/2.2\">" << endl;
   outstream << "<Document>" << endl;
   outstream << "</Document>" << endl;
   outstream << "</kml>" << endl;
}

// ---------------------------------------------------------------------
// Member function generate_polyline_kml_file takes in RGBA and width
// parameters for a polyline as well an STL vector containing
// longitude, latitude, altitude vertices.  This method writes
// polyline output to the specified KML file.

void kml_parser::generate_polyline_kml_file(
   double r, double g, double b, double a,double polyline_width,
   const vector<threevector>& vertices,string output_kml_filename)
{
   ofstream outstream;
   
   if (!filefunc::openfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_polyline_kml_file()" << endl;
      cout << "Cannot open output polyline kml file" << endl;
      return;
   }

// Convert input r,g,b,a into AABBGGRR hexadecimal format:

   string hex_color_string=colorfunc::RGBA_to_AABBGGRR_hex(r,g,b,a);
   outstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   outstream << "<kml xmlns=\"http://earth.google.com/kml/2.2\">" << endl;
   outstream << "<Document>" << endl;

   export_polyline_kml(r,g,b,a,polyline_width,vertices,outstream);

   outstream << "</Document>" << endl << endl;
   outstream << "</kml>" << endl << endl;

   filefunc::closefile(output_kml_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function export_polyline_kml() takes in RGBA and width
// parameters for a polyline as well an STL vector containing
// longitude, latitude, altitude vertices.  This method writes
// polyline output to the specified KML file.

void kml_parser::export_polyline_kml(
   double r,double g,double b,double a,double polyline_width,
   const vector<threevector>& vertices,ofstream& outstream)
{
   cout << "inside kml_parser::export_polyline_kml()" << endl;

// Convert input r,g,b,a into AABBGGRR hexadecimal format:

   string hex_color_string=colorfunc::RGBA_to_AABBGGRR_hex(r,g,b,a);

   outstream << "<Placemark>" << endl;

   outstream << "<Style id=\"FlightPath\">" << endl;
   outstream << "<LineStyle>" << endl;
   outstream << "<color> " << endl;
   outstream << hex_color_string << endl;
   outstream << "</color>" << endl;
   outstream << "<width>" << endl;
   outstream << polyline_width << endl;
   outstream << "</width>" << endl;
   outstream << "</LineStyle>" << endl;
   outstream << "</Style>" << endl << endl;

   outstream << "<styleURL>#FlightPath</styleURL>" << endl;

   outstream << "<LineString>" << endl;
   outstream << "<tessellate>1</tessellate>" << endl;
   outstream << "<coordinates>" << endl;

   outstream.precision(12);
   for (unsigned int v=0; v<vertices.size(); v++)
   {
      outstream << vertices[v].get(0) << ","
                << vertices[v].get(1) << ",0" 
//                << vertices[v].get(2) << endl;
                << endl;
   } // loop over index v labeling individual polygon vertices

   outstream << "</coordinates>" << endl;
   outstream << "</LineString>" << endl;
   outstream << "</Placemark>" << endl << endl;
}

// ---------------------------------------------------------------------
// Member function generate_track_kml_file() takes in *track_ptr which
// is assumed to be filled with lon-lat-alt as a function of time
// measured in elapsed secs since midnight 1 Jan 1970.  It writes this
// time-dependent track to the specified output KML file along with a
// time-independent polyline for the track.

void kml_parser::generate_track_kml_file(
   double r, double g, double b, double a,double polyline_width,
   track* track_ptr,string output_kml_filename)
{
   ofstream outstream;
   
   if (!filefunc::openfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_track_kml_file()" << endl;
      cout << "Cannot open output polyline kml file" << endl;
      return;
   }

// Convert input r,g,b,a into AABBGGRR hexadecimal format:

   string hex_color_string=colorfunc::RGBA_to_AABBGGRR_hex(r,g,b,a);
   outstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   outstream << "<kml xmlns=\"http://earth.google.com/kml/2.2\"" << endl;
   outstream << " xmlns:gx=\"http://www.google.com/kml/ext/2.2\">" << endl;
   outstream << "<Folder>" << endl;


   outstream << "<Placemark>" << endl;
   outstream << "<gx:Track>" << endl;
   outstream.precision(12);

   threevector curr_lonlatalt;
   vector<threevector> lonlatalt;
   vector<string> timestamp;
   Clock clock;
   for (unsigned int i=0; i<track_ptr->size(); i++)
   {
      double elapsed_secs=track_ptr->get_time(i);

      if (!track_ptr->get_interpolated_posn(elapsed_secs,curr_lonlatalt))
         continue;

      clock.convert_elapsed_secs_to_date(elapsed_secs);

      string day_hour_separator_char="T";
      string time_separator_char=":";
      bool display_UTC_flag=true;
      int n_secs_digits=1;
      string curr_timestamp=clock.YYYY_MM_DD_H_M_S(
         day_hour_separator_char,time_separator_char,
         display_UTC_flag,n_secs_digits);
      curr_timestamp += "Z";
      timestamp.push_back(curr_timestamp);
      lonlatalt.push_back(curr_lonlatalt);

//      cout << timestamp.back() << "  "
//           << lonlatalt.back().get(0) << "  "
//           << lonlatalt.back().get(1) << "  "
//           << lonlatalt.back().get(2) 
//           << endl;
      
   } // loop over index i labeling track points

   for (unsigned int i=0; i<timestamp.size(); i++)
   {
      outstream << "<when>"+timestamp[i]+"</when>" << endl;
   }
   for (unsigned int i=0; i<lonlatalt.size(); i++)
   {
      string curr_lla=stringfunc::number_to_string(lonlatalt[i].get(0))+" "
         +stringfunc::number_to_string(lonlatalt[i].get(1))+" "
         +stringfunc::number_to_string(lonlatalt[i].get(2));
      outstream << "<gx:coord>"+curr_lla+"</gx:coord>" << endl;
   }
   
   outstream << "</gx:Track>" << endl << endl;
   outstream << "</Placemark>" << endl << endl;

// Write time-independent polyline to output KML file in addition to
// time-dependent track:

   export_polyline_kml(r,g,b,a,polyline_width,lonlatalt,outstream);

   outstream << "</Folder>" << endl << endl;
   outstream << "</kml>" << endl << endl;

   filefunc::closefile(output_kml_filename,outstream);
}

// ---------------------------------------------------------------------
void kml_parser::generate_polygon_kml_file(
   string poly_header_template_filename,string poly_footer_template_filename,
   const vector<threevector>& V,string output_kml_filename)
{
   ofstream outstream;
   
   if (!filefunc::openfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_polygon_kml_file()" << endl;
      cout << "Cannot open output polygon kml file" << endl;
      return;
   }

   bool strip_comments_flag=false;
   filefunc::ReadInfile(poly_header_template_filename,strip_comments_flag);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      outstream << filefunc::text_line[i] << endl;
   }
   
   outstream.precision(12);
   for (unsigned int v=0; v<V.size(); v++)
   {
      outstream << V[v].get(0) << ","
                << V[v].get(1) << ",0" 
                << endl;
//                << V[v].get(2) << endl;
   }
   

   filefunc::ReadInfile(poly_footer_template_filename,strip_comments_flag);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      outstream << filefunc::text_line[i] << endl;
   }

   filefunc::closefile(output_kml_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function generate_multi_polygons_kml_file

void kml_parser::generate_multi_polygons_kml_file(
   double r, double g, double b, double a,
   string polys_header_filename,string polys_footer_filename,
   const vector<vector<threevector> >& V,string output_kml_filename)
{
   ofstream outstream;
   
   if (!filefunc::openfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_polygon_kml_file()" << endl;
      cout << "Cannot open output polygon kml file" << endl;
      return;
   }

// Copy overall KML polygons file header:

   bool strip_comments_flag=false;
   filefunc::ReadInfile(polys_header_filename,strip_comments_flag);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      outstream << filefunc::text_line[i] << endl;
   }
   
// Convert input r,g,b,a into AABBGGRR hexadecimal format:

   string hex_color_string=colorfunc::RGBA_to_AABBGGRR_hex(r,g,b,a);

// Loop over individual polygons.  Copy separate header and footer KML
// information for each one:

   for (unsigned int polynumber=0; polynumber<V.size(); polynumber++)
   {
      outstream << "<Placemark>" << endl;

      outstream << "<Style id=\"Country\">" << endl;
      outstream << "<LineStyle>" << endl;
      outstream << "<color> " << endl;
      outstream << hex_color_string << endl;
      outstream << "</color>" << endl;
      outstream << "<width>" << endl;
      outstream << polyline_width << endl;
      outstream << "</width>" << endl;
      outstream << "</LineStyle>" << endl;

      outstream << "<PolyStyle>" << endl;
      if (filled_polygon_area_flag)
      {
         outstream << "<color> " << endl;
         outstream << hex_color_string << endl;
         outstream << "</color>" << endl;
      }
      else
      {
         outstream << "<fill>0</fill>" << endl;
      }
      outstream << "</PolyStyle>" << endl;
      outstream << "</Style>" << endl << endl;

      outstream << "<styleURL>#Country</styleURL>" << endl;

      outstream << "<Polygon>" << endl;
      outstream << "<tessellate>1</tessellate>" << endl;
      outstream << "<outerBoundaryIs>" << endl;
      outstream << "<LinearRing>" << endl;
      outstream << "<coordinates>" << endl;

      outstream.precision(12);
      vector<threevector> curr_vertices=V[polynumber];
      for (unsigned int v=0; v<curr_vertices.size(); v++)
      {
         outstream << curr_vertices[v].get(0) << ","
                   << curr_vertices[v].get(1) << ",0" 
//                << curr_vertices[v].get(2) << endl;
                   << endl;
      } // loop over index v labeling individual polygon vertices

      outstream << "</coordinates>" << endl;
      outstream << "</LinearRing>" << endl;
      outstream << "</outerBoundaryIs>" << endl;
      outstream << "</Polygon>" << endl;

      outstream << "</Placemark>" << endl << endl;

   } // loop over polynumber index labeling individual polygons

   filefunc::ReadInfile(polys_footer_filename,strip_comments_flag);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      outstream << filefunc::text_line[i] << endl;
   }

   filefunc::closefile(output_kml_filename,outstream);
}

// ---------------------------------------------------------------------
// This overloaded version of generate_multi_polygons_kml_file takes
// in a set of polygon string names.  It generates a queryable icon at
// the center of each polygon which is labeled with a string name.
// The user can click on a link to a dynamic URL which brings up
// site-specific information.

void kml_parser::generate_multi_polygons_kml_file(
   double r, double g, double b, double a,
   string polys_header_filename,string polys_footer_filename,
   const vector<vector<threevector> >& V,
   const vector<string>& poly_names,
   const vector<string>& dynamic_URLs,
   string output_kml_filename)
{
   ofstream outstream;
   
   if (!filefunc::openfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_polygon_kml_file()" << endl;
      cout << "Cannot open output polygon kml file" << endl;
      return;
   }

// Copy overall KML polygons file header:

   bool strip_comments_flag=false;
   filefunc::ReadInfile(polys_header_filename,strip_comments_flag);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      outstream << filefunc::text_line[i] << endl;
   }
   
// Convert input r,g,b,a into AABBGGRR hexadecimal format:

   string hex_color_string=colorfunc::RGBA_to_AABBGGRR_hex(r,g,b,a);

// Loop over individual polygons.  Copy separate header and footer KML
// information for each one:

   for (unsigned int polynumber=0; polynumber<V.size(); polynumber++)
   {
      outstream << "<Placemark>" << endl;

      outstream << "<Style id=\"SAM\">" << endl;

      if (filled_polygon_area_flag)
      {
         outstream << "<IconStyle>" << endl;
         outstream << "<scale>0.6</scale>" << endl;
         outstream << "<Icon>" << endl;

         string subdir=
            "/home/cho/programs/c++/svn/projects/src/mains/uavs/kml_files/";
         outstream << subdir+"icon53Red.png" << endl;
         outstream << "</Icon>" << endl;
         outstream << "</IconStyle>" << endl;
      }
      
      outstream << "<LineStyle>" << endl;
      outstream << "<color> " << endl;
      outstream << hex_color_string << endl;
      outstream << "</color>" << endl;
      outstream << "<width>" << endl;
      outstream << polyline_width << endl;
      outstream << "</width>" << endl;
      outstream << "</LineStyle>" << endl;

      outstream << "<PolyStyle>" << endl;
      if (filled_polygon_area_flag)
      {
         outstream << "<color> " << endl;
         outstream << hex_color_string << endl;
         outstream << "</color>" << endl;
      }
      else
      {
         outstream << "<fill>0</fill>" << endl;
      }
      outstream << "</PolyStyle>" << endl;
      outstream << "</Style>" << endl;

      outstream << "<name>" << endl;
      outstream << poly_names[polynumber] << endl;
      outstream << "</name>" << endl;
      outstream << "<styleURL>#SAM</styleURL>" << endl;

      outstream << "<description>" << endl;
      outstream << "<![CDATA[" << endl;

//      outstream << "<a href=\"http://touchy/rco/sam.php?site=101&system=SA-13&country=Russia&lat=53.822196 degs&lon=48.732296 degs&range=5.0 kms&max_speed=550.0 m/s&max_alt=3.0 kms&weight=3.0 kgs&ioc=1975\">" << endl;

      outstream << "<a href=\"";
      outstream << dynamic_URLs[polynumber];
      outstream << "\">" << endl;

      outstream << "Generate dynamic web page" << endl;
      outstream << "</a>" << endl;
      outstream << "]]>" << endl;
      outstream << "</description>" << endl;

      outstream << "<MultiGeometry>" << endl;

      vector<threevector> curr_vertices=V[polynumber];
      polygon curr_poly(curr_vertices);
      threevector COM=curr_poly.compute_COM();

      outstream.precision(12);

      outstream << "<Point>" << endl;
      outstream << "<coordinates>" << endl;
      outstream << stringfunc::number_to_string(COM.get(0)) << ","
                << stringfunc::number_to_string(COM.get(1)) << ","
                << stringfunc::number_to_string(COM.get(1)) << endl;
      outstream << "</coordinates>" << endl;
      outstream << "</Point>" << endl;

      outstream << "<Polygon>" << endl;
      outstream << "<tessellate>1</tessellate>" << endl;
      outstream << "<outerBoundaryIs>" << endl;
      outstream << "<LinearRing>" << endl;
      outstream << "<coordinates>" << endl;
      
      for (unsigned int v=0; v<curr_vertices.size(); v++)
      {
         outstream << curr_vertices[v].get(0) << ","
                   << curr_vertices[v].get(1) << ",0" 
//                << curr_vertices[v].get(2) << endl;
                   << endl;
      } // loop over index v labeling individual polygon vertices
   
      outstream << "</coordinates>" << endl;
      outstream << "</LinearRing>" << endl;
      outstream << "</outerBoundaryIs>" << endl;
      outstream << "</Polygon>" << endl;

      outstream << "</MultiGeometry>" << endl;
      outstream << "</Placemark>" << endl << endl;

   } // loop over polynumber index labeling individual polygons

   filefunc::ReadInfile(polys_footer_filename,strip_comments_flag);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      outstream << filefunc::text_line[i] << endl;
   }

   filefunc::closefile(output_kml_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function generate_gridlines_kml_file() 

void kml_parser::generate_gridlines_kml_file(
   unsigned int n_ubins,unsigned int n_vbins,
   const vector<geopoint>& start_vertices,
   const vector<geopoint>& stop_vertices,
   const vector<string> pushpin_name,
   const vector<string> pushpin_label,
   const vector<double> pushpin_lon,
   const vector<double> pushpin_lat,
   string output_kml_filename)
{
//   cout << "inside kml_parser::generate_gridlines_kml_file()" << endl;
   ofstream outstream;
   
   if (!filefunc::appendfile(output_kml_filename,outstream))
   {
      cout << "Error in kml_parser::generate_gridlines_kml_file()" << endl;
      cout << "Cannot open output kml file" << endl;
      return;
   }

   outstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
   outstream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">" << endl;
   
   outstream << "<Document>" << endl;
   outstream << "<name>Grid.kmz</name>" << endl;

// Style info for pink comment bubbles:
   
   outstream << "<Style id=\"sh_pink-circle\">" << endl;
   outstream << "<IconStyle>" << endl;
   outstream << "<scale>1.3</scale>" << endl;
   outstream << "<Icon>" << endl;
   outstream << "<href>http://maps.google.com/mapfiles/kml/paddle/pink-circle.png</href>" << endl;
   outstream << "</Icon>" << endl;
   outstream << "<hotSpot x=\"32\" y=\"1\" xunits=\"pixels\" yunits=\"pixels\"/>" << endl;
   outstream << "</IconStyle>" << endl;
   outstream << "<ListStyle>" << endl;
   outstream << "<ItemIcon>" << endl;
   outstream << "<href>http://maps.google.com/mapfiles/kml/paddle/pink-circle-lv.png</href>" << endl;
   outstream << "</ItemIcon>" << endl;
   outstream << "</ListStyle>"  << endl;
   outstream << "</Style>" << endl;
   
   outstream << "<Style id=\"sn_pink-circle\">" << endl;
   outstream << "<IconStyle>" << endl;
   outstream << "<scale>1.1</scale>" << endl;
   outstream << "<Icon>" << endl;
   outstream << "<href>http://maps.google.com/mapfiles/kml/paddle/pink-circle.png</href>" << endl;
   outstream << "</Icon>" << endl;
   outstream << "<hotSpot x=\"32\" y=\"1\" xunits=\"pixels\" yunits=\"pixels\"/>" << endl;
   outstream << "</IconStyle>" << endl;
   outstream << "<ListStyle>" << endl;
   outstream << "<ItemIcon>" << endl;
   outstream << "<href>http://maps.google.com/mapfiles/kml/paddle/pink-circle-lv.png</href>" << endl;
   outstream << "</ItemIcon>" << endl;
   outstream << "</ListStyle>" << endl;
   outstream << "</Style>" << endl;

   outstream << "<StyleMap id=\"msn_pink-circle\">" << endl;
   outstream << "<Pair>" << endl;
   outstream << "<key>normal</key>" << endl;
   outstream << "<styleUrl>#sn_pink-circle</styleUrl>" << endl;
   outstream << "</Pair>" << endl;
   outstream << "<Pair>" << endl;
   outstream << "<key>highlight</key>" << endl;
   outstream << "<styleUrl>#sh_pink-circle</styleUrl>" << endl;
   outstream << "</Pair>" << endl;
   outstream << "</StyleMap>" << endl;

   int counter=0;
   unsigned int n_gridlines=start_vertices.size();
   for (unsigned int n=0; n<n_gridlines; n++)
   {
      outstream << "<Placemark>" << endl;
      outstream << "<Style id=\"GridLine\">" << endl;
      outstream << "<LineStyle>" << endl;
      outstream << "<color> " << endl;

      string red_hex_color_string="ff0000ff";
      string orange_hex_color_string="ff3388ee";
      string yellow_hex_color_string="ff00ffff";

      string hex_color_string;
      if (counter%10==0)
      {
         hex_color_string=red_hex_color_string;
      }
      else
      {
         hex_color_string=orange_hex_color_string;
      }
      outstream << hex_color_string << endl;

      outstream << "</color>" << endl;
      outstream << "<width>" << endl;

      polyline_width=2;
      if (counter%10==0)
      {
         polyline_width=4;
      }
      
      outstream << polyline_width << endl;
      outstream << "</width>" << endl;
      outstream << "</LineStyle>" << endl;
      outstream << "</Style>" << endl << endl;

      outstream << "<styleURL>#gridline</styleURL>" << endl;

      outstream << "<LineString>" << endl;
      outstream << "<tessellate>1</tessellate>" << endl;
      outstream << "<coordinates>" << endl;

      outstream.precision(12);
      outstream << start_vertices[n].get_longitude() << ","
                << start_vertices[n].get_latitude() << ",0" 
                << endl;
      outstream << stop_vertices[n].get_longitude() << ","
                << stop_vertices[n].get_latitude() << ",0" 
          << endl;

      outstream << "</coordinates>" << endl;
      outstream << "</LineString>" << endl;
      outstream << "</Placemark>" << endl << endl;

      counter++;
      if (n==n_ubins) counter=0;

   } // loop over index n labeling grid lines

// Center grid locations:

//   cout << "pushpin_name.size() = " << pushpin_name.size() << endl;
   for (unsigned int i=0; i<pushpin_name.size(); i++)
   {
      outstream << "<Placemark>" << endl;
      outstream << "<name>"+pushpin_name[i]+"</name>" << endl;
      outstream << "<description>"+pushpin_label[i]+"</description>" << endl;
      outstream << "<styleUrl>#msn_pink-circle</styleUrl>" << endl;
      outstream << "<gx:balloonVisibility>1</gx:balloonVisibility>" << endl;
      outstream << "<Point>" << endl;
      outstream << "<coordinates>"+
         stringfunc::number_to_string(pushpin_lon[i])+","+
         stringfunc::number_to_string(pushpin_lat[i])+",0</coordinates>" 
                << endl;
      outstream << "</Point>" << endl;
      outstream << "</Placemark>" << endl;
   }

   outstream << "</Document>" << endl << endl;
   outstream << "</kml>" << endl << endl;

   filefunc::closefile(output_kml_filename,outstream);
}

// ==========================================================================
// Country border member functions
// ==========================================================================

// Member function extract_country_border_vertices recovers the
// individual polygon data files corresponding to the input country
// name from its associated subdir within astro_geo/countries/.  It
// loops over each polygon's longitude-latitude-altitude vertices and
// appends them to a double STL vector. 

void kml_parser::extract_country_border_vertices(
   string country_name,vector<vector<threevector> >& V)
{
   string subdir=
      "/home/cho/programs/c++/svn/projects/src/astro_geo/countries/";
   subdir += country_name+"/";
//   cout << "subdir = " << subdir << endl;

   string tmp_filename=filefunc::generate_tmpfilename();
//   cout << "tmp_filename = " << tmp_filename << endl;

   string unixcommandstr="ls "+subdir+ " > "+tmp_filename;
   sysfunc::unix_command(unixcommandstr);
   filefunc::ReadInfile(tmp_filename);

   unixcommandstr="rm "+tmp_filename;
   sysfunc::unix_command(unixcommandstr);

   vector<string> polygon_filenames;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      polygon_filenames.push_back(subdir+filefunc::text_line[i]);
//      cout << polygon_filenames.back() << endl;
   }

// Loop over each polygon data file and extracts its longitude,
// latitude, altitude vertices.  Append this information to double STL
// vector V:

   for (unsigned int p=0; p<polygon_filenames.size(); p++)
   {
      string curr_poly_filename=polygon_filenames[p];
      filefunc::ReadInfile(curr_poly_filename);
      
      vector<threevector> poly_vertices;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         const string separator_char=",";
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i],separator_char);
         double curr_longitude=stringfunc::string_to_number(substrings[0]);
         double curr_latitude=stringfunc::string_to_number(substrings[1]);
         double curr_altitude=stringfunc::string_to_number(substrings[2]);
         threevector lla(curr_longitude,curr_latitude,curr_altitude);
         poly_vertices.push_back(lla);
//         cout << "long = " << curr_longitude
//              << " lat = " << curr_latitude
//              << " alt = " << curr_altitude << endl;
      } // loop over index i labeling polygon file lines

      V.push_back(poly_vertices);
   } // loop over index p labeling individual polgyons
}

// ---------------------------------------------------------------------
// Member function generate_country_borders_kml_file takes in a double
// STL vector V containing longitude-latitude-altitude vertices for a
// set of countries.  It generates an output KML file where the
// countries borders are heavily highlighted.

void kml_parser::generate_country_borders_kml_file(
   const vector<vector<threevector> >& V,
   double r, double g, double b, double a,
   string polys_header_filename,string polys_footer_filename,
   string output_kml_filename)
{
   filled_polygon_area_flag=false;
//   polyline_width=100;
   polyline_width=10;
   curr_rgba.first=r;
   curr_rgba.first=g;
   curr_rgba.first=b;
   curr_rgba.first=a;
   
   generate_multi_polygons_kml_file(
      r,g,b,a,
      polys_header_filename,polys_footer_filename,V,output_kml_filename);
}

// ---------------------------------------------------------------------
// Member function 

void kml_parser::retrieve_country_borders_from_PostGIS_database(
   OGRFeature* poFeature_ptr,int geom_name_column)
{
   OGRGeometry* poGeometry_ptr=poFeature_ptr->GetGeometryRef();

   OGRMultiPolygon* poMultiPolygon=dynamic_cast<OGRMultiPolygon*>(
      poGeometry_ptr);

   string country_name=
      poFeature_ptr->GetFieldAsString(geom_name_column);
//   cout << "country = " << country_name << endl;

// Hack : For reasons we don't understand, the very first country name
// is not read out from the PostGIS world borders table.  So we
// hardwire it here:

   if (country_name.size()==0) country_name="Russia";

   for (int n_polygon=0; n_polygon < poMultiPolygon->getNumGeometries(); 
        n_polygon++)
   {
      OGRPolygon* poPolygon_ptr=dynamic_cast<OGRPolygon*>(
         poMultiPolygon->getGeometryRef(n_polygon));
      OGRLineString* poRing_ptr=dynamic_cast<OGRLineString*>(
         poPolygon_ptr->getExteriorRing());

// Compute location of polyline vertices assuming each has zero
// altitude:

      int n_points=poRing_ptr->getNumPoints();
      for (int p=0; p<n_points; p++)
      {
         double longitude=poRing_ptr->getX(p);
         double latitude=poRing_ptr->getY(p);
         double altitude=0;

         cout << longitude << " , "
              << latitude << " , " 
              << altitude << endl;

      } // loop over p index labeling vertices for a particular polygon

   } // loop over index n_polygon labeling Polygons
}


