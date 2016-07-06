// ==========================================================================
// Header file for kml_parser class
// ==========================================================================
// Last modified on 3/25/08; 4/28/11; 7/5/11
// ==========================================================================

#ifndef KML_PARSER_H
#define KML_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "astro_geo/geopoint.h"
#include "geometry/polyline.h"
#include "math/threevector.h"

class OGRFeature;
class OGRGeometry;
class postgis_database;
class track;

class kml_parser
{

  public:

// Initialization, constructor and destructor functions:

   kml_parser();
   kml_parser(postgis_database* pgdb_ptr);
   kml_parser(const kml_parser& k);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~kml_parser();
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const kml_parser& k);

// Set and get member functions:

   void set_filled_polygon_area_flag(bool flag);
   void set_input_filename(std::string filename);
   std::string get_input_filename() const;
   void set_output_filename(std::string filename);
   std::string get_output_filename() const;
   polyline& get_curr_polyline();
   const polyline& get_curr_polyline() const;

// Polyline parsing member functions:

   polyline& parse_polyline_kml(std::string path_filename);

// KML file generation member functions:

   void generate_empty_kml_file(
      std::string output_kml_filename);
   void generate_polyline_kml_file(
      double r, double g, double b, double a,double polyline_width,
      const std::vector<threevector>& vertices,
      std::string output_kml_filename);
   void export_polyline_kml(
      double r,double g,double b,double a,double polyline_width,
      const std::vector<threevector>& vertices,std::ofstream& outstream);

   void generate_track_kml_file(
      double r, double g, double b, double a,double polyline_width,
      track* track_ptr,std::string output_kml_filename);

   void generate_polygon_kml_file(
      std::string poly_header_template_filename,
      std::string poly_footer_template_filename,
      const std::vector<threevector>& V,std::string output_kml_filename);
   void generate_multi_polygons_kml_file(
      double r, double g, double b, double a,
      std::string polys_header_filename,std::string polys_footer_filename,
      const std::vector<std::vector<threevector> >& V,
      std::string output_kml_filename);
   void generate_multi_polygons_kml_file(
      double r, double g, double b, double a,
      std::string polys_header_filename,std::string polys_footer_filename,
      const std::vector<std::vector<threevector> >& V,
      const std::vector<std::string>& poly_names,
      const std::vector<std::string>& dynamic_URLs,
      std::string output_kml_filename);

   void generate_gridlines_kml_file(
      unsigned int n_ubins,unsigned int n_vbins,
      const std::vector<geopoint>& start_vertices,
      const std::vector<geopoint>& stop_vertices,
      const std::vector<std::string> pushpin_name,
      const std::vector<std::string> pushpin_label,
      const std::vector<double> pushpin_lon,
      const std::vector<double> pushpin_lat,
      std::string output_kml_filename);

// Country border member functions

   void extract_country_border_vertices(
      std::string country_name,std::vector<std::vector<threevector> >& V);
   void generate_country_borders_kml_file(
      const std::vector<std::vector<threevector> >& V,
      double r, double g, double b, double a,
      std::string polys_header_filename,std::string polys_footer_filename,
      std::string output_kml_filename);
   void retrieve_country_borders_from_PostGIS_database(
      OGRFeature* poFeature_ptr,int geom_name_column=-1);

  private: 

   bool filled_polygon_area_flag;
   std::string input_filename,output_filename;
   polyline curr_polyline;
   double polyline_width;
   colorfunc::RGBA curr_rgba;
   postgis_database* PostGIS_database_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const kml_parser& k);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void kml_parser::set_filled_polygon_area_flag(bool flag)
{
   filled_polygon_area_flag=flag;
}

inline void kml_parser::set_input_filename(std::string filename)
{
   input_filename=filename;
}

inline std::string kml_parser::get_input_filename() const
{
   return input_filename;
}

inline void kml_parser::set_output_filename(std::string filename)
{
   output_filename=filename;
}

inline std::string kml_parser::get_output_filename() const
{
   return output_filename;
}

inline polyline& kml_parser::get_curr_polyline()
{
   return curr_polyline;
}

inline const polyline& kml_parser::get_curr_polyline() const
{
   return curr_polyline;
}

#endif  // kml_parser.h



