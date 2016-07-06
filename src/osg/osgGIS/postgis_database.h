// ==========================================================================
// Header file for postgis_database class 
// ==========================================================================
// Last modified on 5/13/10; 5/14/10; 11/30/10
// ==========================================================================

#ifndef POSTGIS_DATABASE_H
#define POSTGIS_DATABASE_H

class OGRPoint;

#include "postgres/gis_database.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"

class postgis_database: public gis_database
{

  public:

   postgis_database(std::string host,std::string dbname,std::string user);
   postgis_database(const postgis_database& d);
   ~postgis_database();
   postgis_database& operator= (const postgis_database& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const postgis_database& d);

// Set and get member functions:

   void set_country_name_color(colorfunc::Color c);
   void set_city_color(colorfunc::Color c);
   void set_PolyLine_text_size(double size);
   std::vector< std::string >* get_geometry_names_ptr();
   void set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
   void set_CountriesGroup_ptr(CylindersGroup* CG_ptr);
   void set_CitiesGroup_ptr(CylindersGroup* CG_ptr);

// Database parsing member functions:

   bool parse_table_contents(
      std::string geom_name="name",std::string where_clause="");   
   void parse_multilinestring_geometry();
   void parse_linestring_geometry(OGRGeometry* OGRGeometry_ptr);
   void parse_polygon_geometry(OGRGeometry* OGRGeometry_ptr);

   void parse_worldline_geometry();
   void parse_worldpoint_geometry(
      OGRGeometry* OGRGeometry_ptr,CylindersGroup* CylindersGroup_ptr,
      colorfunc::Color cylinder_color,
      double text_displacement,double text_size,
      bool text_screen_axis_alignment_flag=true);

  private: 

   double PolyLine_text_size;
   colorfunc::Color country_name_color,city_color;
   CylindersGroup *CountriesGroup_ptr,*CitiesGroup_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const postgis_database& d);

   void set_country_name(OGRPoint* centroid_ptr,double max_area);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void postgis_database::set_country_name_color(colorfunc::Color c)
{
   country_name_color=c;
}

inline void postgis_database::set_city_color(colorfunc::Color c)
{
   city_color=c;
}

inline void postgis_database::set_PolyLine_text_size(double size)
{
   PolyLine_text_size=size;
}

inline std::vector< std::string >* postgis_database::get_geometry_names_ptr()
{
   return &geometry_names;
}

inline void postgis_database::set_CountriesGroup_ptr(CylindersGroup* CG_ptr)
{
   CountriesGroup_ptr=CG_ptr;
}

inline void postgis_database::set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr)
{
   PolyLinesGroup_ptr=PLG_ptr;
}

inline void postgis_database::set_CitiesGroup_ptr(CylindersGroup* CG_ptr)
{
   CitiesGroup_ptr=CG_ptr;
}

#endif  // postgis_database.h
