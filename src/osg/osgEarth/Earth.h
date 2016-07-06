// ==========================================================================
// Header file for Earth class
// ==========================================================================
// Last updated on 5/17/09; 11/29/10; 2/22/13
// ==========================================================================

#ifndef EARTH_H
#define EARTH_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <osg/Group>
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "astro_geo/Ellipsoid_model.h"
#include "math/fourvector.h"
#include "osg/osgGeometry/Geometrical.h"
#include "astro_geo/geopoint.h"
#include "osg/osgGrid/LatLongGrid.h"
#include "math/ltthreevector.h"
#include "datastructures/Mynode.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgEarth/TextureSectorsGroup.h"
#include "datastructures/Triple.h"
#include "math/twovector.h"
#include "osg/ViewFrustum.h"

class Clock;
class DataGraph;
class LatLongGrid;
class Pass;
class PointCloud;
class postgis_database;

class Earth : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Earth(Pass* PI_ptr,Clock* clock_ptr);
   Earth(Pass* PI_ptr,Clock* clock_ptr,postgis_database* pgdb_ptr);

   virtual ~Earth();
   friend std::ostream& operator<< (std::ostream& outstream,const Earth& p);

// Set & get member functions:

   void set_refresh_longlat_lines_flag(bool flag);
   bool get_refresh_longlat_lines_flag() const;

   void set_EarthManipulator_ptr(osgGA::EarthManipulator* EM_ptr);
   ViewFrustum* get_ViewFrustum_ptr();
   const ViewFrustum* get_ViewFrustum_ptr() const;
   double get_log_eye_center_dist() const;
   
   bool get_display_long_lat_lines() const;
   bool get_display_borders() const;
   bool get_display_cities() const;

   Ellipsoid_model* get_Ellipsoid_model_ptr();
   const Ellipsoid_model* get_Ellipsoid_model_ptr() const;
   Clock* get_Clock_ptr();
   const Clock* get_Clock_ptr() const;
   LatLongGrid* get_LatLongGrid_ptr();

   osg::Group* get_drawable_group_ptr();
   const threevector& get_camera_ECI_posn() const;
   const threevector& get_camera_Zhat() const;
   double get_camera_longitude() const;
   double get_camera_latitude() const;
   double get_camera_altitude() const;

   PolyLinesGroup* get_borders_group_ptr();
   CylindersGroup* get_cities_group_ptr();
   CylindersGroup* get_minor_cities_group_ptr();
   CylindersGroup* get_countries_group_ptr();
   TextureSectorsGroup* get_TextureSectorsGroup_ptr();

   void set_init_border_width(double w);

// MatrixTransform member functions:
   
   threevector undo_datagraph_translation(DataGraph* datagraph_ptr);
   threevector datagraph_origin_long_lat_alt(
      DataGraph* datagraph_ptr,const threevector& world_midpoint,
      double altitude_offset=0);
   osg::MatrixTransform* generate_earthsurface_MatrixTransform(
      const threevector& origin_long_lat_alt);
   osg::MatrixTransform* generate_UTM_to_latlong_grid_rot_MatrixTransform(
      double theta);

// Camera posn & orientation member functions:

   void compute_camera_posn_and_Zhat_in_ECI_coords();
   double compute_camera_to_screen_center_distance();

// Drawing member functions:

   void toggle_long_lat_lines();
   void toggle_borders_display();
   void toggle_cities_display();
   void set_longlat_lines_mask(unsigned int flag);
   void set_countries_mask(unsigned int flag);
   void set_borders_mask(unsigned int flag);
   unsigned int get_borders_mask() const;
   void set_cities_mask(unsigned int flag);
   void set_minor_cities_mask(unsigned int flag);
   osg::Group* generate_drawable_group();

// Cartesian to spherical coordinate system conversion member functions:

   void long_lat_corners_to_plane(
      std::vector<twovector>& long_lat_corner);
   void UTM_corners_to_plane(
      int ZoneNumber,bool northern_hemisphere_flag,
      std::vector<threevector>& UTM_corner);

// PostGIS database retrieval methods:

   void retrieve_borders_from_PostGIS_database(
      std::string borders_tablename,
      double longitude_min=-181,double longitude_max=181,
      double latitude_min=-80,double latitude_max=80);
   void retrieve_borders_from_PostGIS_database(
      std::string borders_tablename,
      double longitude_min,double longitude_max,
      double latitude_min,double latitude_max,
      colorfunc::Color country_color);
   void retrieve_borders_from_PostGIS_database(
      std::string borders_tablename,
      double longitude_min,double longitude_max,
      double latitude_min,double latitude_max,
      osg::Vec4& border_color,colorfunc::Color& country_name_color);
   void set_altitude_dependent_border_width();
   void retrieve_cities_from_PostGIS_database(
      double longitude_min=-181,double longitude_max=181,
      double latitude_min=-87,double latitude_max=87);
   void retrieve_cities_from_PostGIS_database(
      double longitude_min,double longitude_max,
      double latitude_min,double latitude_max,
      colorfunc::Color city_color);
   void retrieve_US_cities_from_PostGIS_database(
      double longitude_min,double longitude_max,
      double latitude_min,double latitude_max,
      colorfunc::Color city_color,int population_threshold=50000);
   bool long_lat_for_specified_geosite(
      std::string geosite_name,geopoint& site_geopoint);

  protected:

  private:

   bool refresh_longlat_lines_flag;
   bool display_long_lat_lines,display_borders,display_cities;
   
// Camera posn member vars:

   double log_eye_alt,log_eye_center_dist;
   double camera_longitude,camera_latitude,camera_altitude;
   double init_border_width;

   LatLongGrid* LatLongGrid_ptr;
   Ellipsoid_model* Ellipsoid_model_ptr;
   osgGA::EarthManipulator* EarthManipulator_ptr;

   postgis_database* PostGIS_database_ptr;
   Pass* pass_ptr;
   Clock* Clock_ptr;

   PolyLinesGroup* borders_group_ptr;
   CylindersGroup *cities_group_ptr,*countries_group_ptr;
   CylindersGroup *minor_cities_group_ptr;
   TextureSectorsGroup* TextureSectorsGroup_ptr;

   osg::ref_ptr<osg::Group> drawable_group_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Earth& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Earth::set_refresh_longlat_lines_flag(bool flag)
{
   refresh_longlat_lines_flag=flag;
}

inline bool Earth::get_refresh_longlat_lines_flag() const
{
   return refresh_longlat_lines_flag;
}

inline void Earth::set_EarthManipulator_ptr(osgGA::EarthManipulator* EM_ptr)
{
   EarthManipulator_ptr=EM_ptr;
   LatLongGrid_ptr->set_CM_3D_ptr(EarthManipulator_ptr);
}

inline double Earth::get_log_eye_center_dist() const
{
   return log_eye_center_dist;
}

inline ViewFrustum* Earth::get_ViewFrustum_ptr() 
{
   return EarthManipulator_ptr->get_ViewFrustum_ptr();
}

inline const ViewFrustum* Earth::get_ViewFrustum_ptr() const
{
   return EarthManipulator_ptr->get_ViewFrustum_ptr();
}

inline double Earth::get_camera_longitude() const
{
   return camera_longitude;
}

inline double Earth::get_camera_latitude() const
{
   return camera_latitude;
}

inline double Earth::get_camera_altitude() const
{
   return camera_altitude;
}

inline Ellipsoid_model* Earth::get_Ellipsoid_model_ptr()
{
   return Ellipsoid_model_ptr;
}

inline Clock* Earth::get_Clock_ptr()
{
   return Clock_ptr;
}

inline const Clock* Earth::get_Clock_ptr() const
{
   return Clock_ptr;
}

inline const Ellipsoid_model* Earth::get_Ellipsoid_model_ptr() const
{
   return Ellipsoid_model_ptr;
}

inline LatLongGrid* Earth::get_LatLongGrid_ptr()
{
   return LatLongGrid_ptr;
}

inline osg::Group* Earth::get_drawable_group_ptr()
{
   return drawable_group_refptr.get();
}

inline const threevector& Earth::get_camera_ECI_posn() const
{
   return get_ViewFrustum_ptr()->get_camera_posn();
}

inline const threevector& Earth::get_camera_Zhat() const
{
   return get_ViewFrustum_ptr()->get_camera_Zhat();
}

inline bool Earth::get_display_long_lat_lines() const
{
   return display_long_lat_lines;
}

inline bool Earth::get_display_borders() const
{
   return display_borders;
}

inline bool Earth::get_display_cities() const
{
   return display_cities;
}

inline PolyLinesGroup* Earth::get_borders_group_ptr()
{
   return borders_group_ptr;
}

inline CylindersGroup* Earth::get_cities_group_ptr()
{
   return cities_group_ptr;
}

inline CylindersGroup* Earth::get_minor_cities_group_ptr()
{
   return minor_cities_group_ptr;
}

inline CylindersGroup* Earth::get_countries_group_ptr()
{
   return countries_group_ptr;
}

inline TextureSectorsGroup* Earth::get_TextureSectorsGroup_ptr()
{
   return TextureSectorsGroup_ptr;
}

inline void Earth::set_init_border_width(double w)
{
   init_border_width=w;
}


#endif // Earth.h



