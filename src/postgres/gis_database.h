// ==========================================================================
// Header file for gis_database class 
// ==========================================================================
// Last modified on 5/16/10; 5/30/10; 11/30/10
// ==========================================================================

#ifndef GIS_DATABASE_H
#define GIS_DATABASE_H

#include <vector>
#include "geometry/bounding_box.h"
#include "postgres/database.h"
#include "astro_geo/Ellipsoid_model.h"
#include "math/threevector.h"

class OGRDataSource;
class OGRLayer;
class OGRSpatialReference;
class OGRCoordinateTransformation;
class OGRFeature;
class OGRGeometry;
class OGRPolygon;

class gis_database: public database
{

  public:

   gis_database(std::string host,std::string dbname,std::string user);
   gis_database(const gis_database& d);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~gis_database();
   gis_database& operator= (const gis_database& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const gis_database& d);

// Set and get member functions:

   void set_category_column_flag(bool flag);
   bool get_category_column_flag() const;
   void set_TableName(std::string TableName);
   std::string get_TableName() const;
   void set_earth_flag(bool flag);
   void set_flat_grid_flag(bool flag);
   void set_specified_UTM_zonenumber(int zonenumber);
   void set_altitude(double z);
   void pushback_gis_bbox(double xmin,double xmax,double ymin,double ymax);
   void popback_gis_bbox();
   bounding_box& get_gis_bbox();
   const bounding_box& get_gis_bbox() const;
   std::vector< std::string >* get_geometry_names_ptr();
   void set_Ellipsoid_model_ptr(Ellipsoid_model* EM_ptr);

   void pushback_GISpoint_tablename(std::string TableName);
   std::vector<std::string>& get_GISpoint_tablenames();
   const std::vector<std::string>& get_GISpoint_tablenames() const;

   void pushback_GISlines_tablename(std::string TableName);
   std::vector<std::string>& get_GISlines_tablenames();
   const std::vector<std::string>& get_GISlines_tablenames() const;

   void pushback_GISpolys_tablename(std::string TableName);
   std::vector<std::string>& get_GISpolys_tablenames();
   const std::vector<std::string>& get_GISpolys_tablenames() const;

// GIS database parsing member functions:

   void read_table(std::string TableName);
   void setup_coordinate_transformation(
      std::string input_wellknown_GCS_name,
      int input_StatePlane_number=-1,
      int output_UTM_zonenumber=-1,bool output_northern_hemisphere_flag=true);

   bool parse_table_contents(
      std::string geom_name="name",std::string where_clause="");   
   void parse_multilinestring_geometry();
   void parse_linestring_geometry(OGRGeometry* OGRGeometry_ptr);
   void parse_multipolygon_geometry();
   void parse_polygon_geometry(OGRGeometry* OGRGeometry_ptr);

   void parse_OGRPolygon_vertices(OGRPolygon* poPolygon_ptr);
   std::vector<double> parse_worldline_geometry(twovector& centroid);

   void parse_multipoint_geometry();
   bool parse_point_geometry(
      OGRGeometry* OGRGeometry_ptr,double& longitude,double& latitude,
      threevector& curr_point,threevector& r_hat);

// GIS info retrieval member functions:

   bool long_lat_for_specified_geosite(
      std::string TableName,std::string geosite_name,twovector& LongLat);
   void output_country_border_polygons();

  protected:

   bool category_column_flag,postgis_connection_status_flag;
   bool earth_flag,flat_grid_flag;
   std::string TableName;
   int n_layers,row_ID,geom_name_column;
   int specified_UTM_zonenumber;
   double altitude;
   std::vector<bounding_box> bbox;
   std::vector<bounding_box> bbox_stateplane;
   std::string geom_name;
   std::vector<std::string> geometry_names;
   std::vector<threevector>* curr_polyline_vertices_ptr;

   typedef std::map<int,std::vector<threevector>* > POLYLINE_VERTICES_MAP;
   POLYLINE_VERTICES_MAP* polyline_vertices_map_ptr;

   Ellipsoid_model* Ellipsoid_model_ptr;
   std::vector<std::string> GISpoint_tablenames,GISlines_tablenames,
      GISpolys_tablenames;

   OGRDataSource* poDS_ptr;
   OGRLayer* poLayer_ptr;
   OGRSpatialReference *input_SRS_ptr,*output_SRS_ptr;
   OGRCoordinateTransformation *poCT_ptr,*po_inverseCT_ptr;
   OGRFeature* poFeature_ptr;
   OGRGeometry* poGeometry_ptr;

   void open_OGR_connection();
   bool initialize_table_parsing(
      std::string geom_name,std::string where_clause);
   std::vector<threevector>* generate_new_polyline_vertices();
   void delete_polyline_vertices_map();
   void purge_polyline_vertices_map();

  private: 

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const gis_database& d);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void gis_database::set_category_column_flag(bool flag)
{
   category_column_flag=flag;
}

inline bool gis_database::get_category_column_flag() const
{
   return category_column_flag;
}

inline void gis_database::set_TableName(std::string TableName)
{
   this->TableName=TableName;
}

inline std::string gis_database::get_TableName() const
{
   return TableName;
}

inline void gis_database::set_earth_flag(bool flag)
{
   earth_flag=flag;
}

inline void gis_database::set_flat_grid_flag(bool flag)
{
   flat_grid_flag=flag;
}

inline void gis_database::set_specified_UTM_zonenumber(int zonenumber)
{
   specified_UTM_zonenumber=zonenumber;
}

inline void gis_database::set_altitude(double z)
{
   altitude=z;
}

inline bounding_box& gis_database::get_gis_bbox()
{
   return bbox.back();
}

inline const bounding_box& gis_database::get_gis_bbox() const
{
   return bbox.back();
}

inline std::vector< std::string >* gis_database::get_geometry_names_ptr()
{
   return &geometry_names;
}

inline void gis_database::set_Ellipsoid_model_ptr(Ellipsoid_model* EM_ptr)
{
   Ellipsoid_model_ptr=EM_ptr;
}

inline void gis_database::pushback_GISpoint_tablename(
   std::string TableName)
{
   GISpoint_tablenames.push_back(TableName);
}

inline std::vector<std::string>& gis_database::get_GISpoint_tablenames()
{
   return GISpoint_tablenames;
}

inline const std::vector<std::string>& 
gis_database::get_GISpoint_tablenames() const
{
   return GISpoint_tablenames;
}


inline void gis_database::pushback_GISlines_tablename(
   std::string TableName)
{
   GISlines_tablenames.push_back(TableName);
}

inline std::vector<std::string>& gis_database::get_GISlines_tablenames()
{
   return GISlines_tablenames;
}

inline const std::vector<std::string>& 
gis_database::get_GISlines_tablenames() const
{
   return GISlines_tablenames;
}


inline void gis_database::pushback_GISpolys_tablename(
   std::string TableName)
{
   GISpolys_tablenames.push_back(TableName);
}

inline std::vector<std::string>& gis_database::get_GISpolys_tablenames()
{
   return GISpolys_tablenames;
}

inline const std::vector<std::string>& 
gis_database::get_GISpolys_tablenames() const
{
   return GISpolys_tablenames;
}



#endif  // gis_database.h
