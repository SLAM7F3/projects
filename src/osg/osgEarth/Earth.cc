// ==========================================================================
// Earth class member function definitions
// ==========================================================================
// Last updated on 5/13/10; 11/29/10; 2/22/13
// ==========================================================================

#include <iterator>
#include <list>
#include <string>
#include <vector>
#include <osg/Quat>
#include <osgDB/ReadFile>

#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgEarth/Earth.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "astro_geo/geofuncs.h"
#include "geometry/homography.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "math/ltthreevector.h"
#include "math/mathfuncs.h"
#include "osg/osgfuncs.h"
#include "passes/Pass.h"
#include "geometry/plane.h"
#include "osg/osg3D/PointCloud.h"
#include "geometry/polyline.h"
#include "osg/osgGIS/postgis_database.h"
#include "geometry/projective.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "osg/Transformer.h"

#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::iterator;
using std::map;
using std::ostream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Earth::allocate_member_objects()
{
   Ellipsoid_model_ptr=new Ellipsoid_model();

   countries_group_ptr=new CylindersGroup(
      pass_ptr,Clock_ptr,Ellipsoid_model_ptr);
   borders_group_ptr=new PolyLinesGroup(3,pass_ptr);
   cities_group_ptr=new CylindersGroup(
      pass_ptr,Clock_ptr,Ellipsoid_model_ptr);
   minor_cities_group_ptr=new CylindersGroup(
      pass_ptr,Clock_ptr,Ellipsoid_model_ptr);
   TextureSectorsGroup_ptr=new TextureSectorsGroup(pass_ptr);
   drawable_group_refptr=new osg::Group();

   LatLongGrid_ptr=new LatLongGrid(pass_ptr,3);
   LatLongGrid_ptr->set_flat_grid_flag(false);
}		       

void Earth::initialize_member_objects()
{
   Graphical_name="Earth";
   PostGIS_database_ptr=NULL;
   refresh_longlat_lines_flag=false;
   display_long_lat_lines=true;
   display_borders=true;
   display_cities=true;

   EarthManipulator_ptr=NULL;
   log_eye_alt=8;
   log_eye_center_dist=NEGATIVEINFINITY;
   init_border_width=1.0;
}

Earth::Earth(Pass* PI_ptr,Clock* clock_ptr):
   Geometrical(3,0)
{	
   pass_ptr=PI_ptr;
   Clock_ptr=clock_ptr;
   allocate_member_objects();
   initialize_member_objects();
}		       

Earth::Earth(Pass* PI_ptr,Clock* clock_ptr,postgis_database* pgdb_ptr):
   Geometrical(3,0)
{	
   pass_ptr=PI_ptr;
   Clock_ptr=clock_ptr;
   allocate_member_objects();
   initialize_member_objects();
   PostGIS_database_ptr=pgdb_ptr;

   if (PostGIS_database_ptr != NULL)
      PostGIS_database_ptr->set_Ellipsoid_model_ptr(Ellipsoid_model_ptr);
}		       

Earth::~Earth()
{
   delete Ellipsoid_model_ptr;
   delete borders_group_ptr;
   delete cities_group_ptr;
   delete minor_cities_group_ptr;
   delete countries_group_ptr;
   delete TextureSectorsGroup_ptr;
   delete LatLongGrid_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Earth& p)
{
   outstream << "inside Earth::operator<<" << endl;
   
   outstream << static_cast<const Geometrical&>(p) << endl;
   return(outstream);
}

// ==========================================================================
// MatrixTransform member functions
// ==========================================================================

// Member function undo_datagraph_translation returns a translation
// vector which explicitly counters tbe translation embedded in a
// datagraph's topmost MatrixTransform:

threevector Earth::undo_datagraph_translation(DataGraph* datagraph_ptr)
{
//   cout << "inside Earth::undo_datagraph_translation" << endl;
   
   osg::Matrix top_matrix=datagraph_ptr->get_top_Matrix();
   osg::Vec3 UTM_trans(top_matrix(3,0),top_matrix(3,1),top_matrix(3,2));
//   cout << "UTM_trans = " << threevector(UTM_trans) << endl;

   double xmax=datagraph_ptr->get_xyz_bbox().xMax();
   double xmin=datagraph_ptr->get_xyz_bbox().xMin();
   double ymax=datagraph_ptr->get_xyz_bbox().yMax();
   double ymin=datagraph_ptr->get_xyz_bbox().yMin();

   threevector world_midpoint(UTM_trans.x()+0.5*(xmax-xmin),
                              UTM_trans.y()+0.5*(ymax-ymin),0);
//   cout << "world_midpoint = " << world_midpoint << endl;
//   cout << "zmin = " << zmin << endl;

   return world_midpoint;
}

// ---------------------------------------------------------------------
// Member function datagraph_origin_long_lat_alt returns the
// longitude, latitude and altitude coordinates corresponding to the
// datagraph's midpoint.

threevector Earth::datagraph_origin_long_lat_alt(
   DataGraph* datagraph_ptr,const threevector& world_midpoint,
   double altitude_offset)
{
//   cout << "inside Earth::datagraph_origin_long_lat_alt" << endl;
   
   double latitude,longitude;
   latlongfunc::UTMtoLL(
      datagraph_ptr->get_pass_ptr()->get_UTM_zonenumber(),
      datagraph_ptr->get_pass_ptr()->get_northern_hemisphere_flag(),
      world_midpoint.get(1),world_midpoint.get(0),latitude,longitude);
//   cout << "datagraph_ptr->get_pass_ptr() = " 
//        << datagraph_ptr->get_pass_ptr() << endl;
//   cout << "pass_ptr->get_UTM_zonenumber() = "
//       << pass_ptr->get_UTM_zonenumber() << endl;
//   cout << "northern hemisphere = " 
//        << pass_ptr->get_northern_hemisphere_flag() << endl;

// Perform vertical translation so that point corresponding to z_min
// is mapped to altitude_offset on blue marble:

   double zmin=datagraph_ptr->get_xyz_bbox().zMin();
   threevector origin_long_lat_alt(longitude,latitude,-zmin+altitude_offset);
//   cout << "origin_long_lat_alt = " << origin_long_lat_alt << endl;

   return origin_long_lat_alt;
}

// ---------------------------------------------------------------------
// Member function generate_earthsurface_MatrixTransform generates a
// MatrixTransform which manipulates a datagraph so that it lies on
// surface of earth.  The datagraph's origin is translated to the blue
// marble's origin_long_lat and origin_altitude.  Rotate its XYZ axes
// so that they locally align with the east, north and radial earth
// directions:

osg::MatrixTransform* Earth::generate_earthsurface_MatrixTransform(
   const threevector& origin_long_lat_alt)
{
//   cout << "inside Earth::generate_earthsurface_MT()" << endl;
   threevector longlatalt_trans=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
      origin_long_lat_alt.get(0),origin_long_lat_alt.get(1),
      origin_long_lat_alt.get(2));
   genmatrix* R_ptr=Ellipsoid_model_ptr->east_north_radial_to_XYZ_rotation(
      origin_long_lat_alt.get(1),origin_long_lat_alt.get(0));
   osg::MatrixTransform* SurfaceTransform_ptr=
      osgfunc::generate_rot_and_trans(*R_ptr,longlatalt_trans);

   SurfaceTransform_ptr->setName("SurfaceTransform");
   return SurfaceTransform_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_UTM_to_latlong_grid_rot_MatrixTransform
// returns a Z-rotation which optimally aligns a UTM subgrid with
// underlying lines of longitude & latitude on the blue marble.
// Recall that UTM lines of constant easting [northing] do NOT
// generally correspond to lines of constant longitude [latitude].
// Instead, the latter are generally curved arcs within a 2D Cartesian
// UTM grid since they converge on the 3D ellipsoid.  But sufficiently
// small UTM subgrid areas can be brought into good alignment with
// lines of longitude & latitude via a small rotation about local
// r-hat...

osg::MatrixTransform* Earth::generate_UTM_to_latlong_grid_rot_MatrixTransform(
   double theta)
{
//   cout << "inside Earth::generate_UTM_to_latlong_grid_rot_MT()" << endl;
   rotation R(0,0,-theta);
   osg::MatrixTransform* UTM_to_LL_transform_ptr=osgfunc::generate_rot(R);
   UTM_to_LL_transform_ptr->setName("UTM_to_LatLong_Transform");
   return UTM_to_LL_transform_ptr;
}

// ==========================================================================
// Camera posn & orientation member functions:
// ==========================================================================

void Earth::compute_camera_posn_and_Zhat_in_ECI_coords()
{
//   cout << "inside Earth::compute_camera_posn_and_Zhat_in_ECI_coords()"
//        << endl;
   
   get_ViewFrustum_ptr()->retrieve_camera_posn_and_orientation();

   Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
      get_camera_ECI_posn(),*Clock_ptr,
      camera_longitude,camera_latitude,camera_altitude);

   log_eye_alt=log10(camera_altitude);
}

// ---------------------------------------------------------------------
// Member function compute_camera_to_screen_center_distance() first
// retrieves the ECI coordinates for the ellipsoid point located at
// the center of the screen.  If the ellipsoid does not currently
// cover the screen center, the previous screen_center_intercept is
// used.  It then computes the distance between the screen center
// point and the camera and stores the result within member variable
// log_eye_center_dist.

double Earth::compute_camera_to_screen_center_distance()
{
   threevector screen_center_intercept;
   if (EarthManipulator_ptr->
       compute_screen_center_intercept(screen_center_intercept))
   {
      log_eye_center_dist=
         EarthManipulator_ptr->compute_camera_to_screen_center_distance(
            screen_center_intercept);
   }
   
   return log_eye_center_dist;
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

void Earth::toggle_long_lat_lines()
{
   display_long_lat_lines = !display_long_lat_lines;
   set_longlat_lines_mask(static_cast<unsigned int>(
      display_long_lat_lines));
}

void Earth::set_longlat_lines_mask(unsigned int flag)
{
   LatLongGrid_ptr->get_LongLinesGroup_ptr()->get_OSGgroup_ptr()->
      setNodeMask(flag);
   LatLongGrid_ptr->get_LatLinesGroup_ptr()->get_OSGgroup_ptr()->
      setNodeMask(flag);
   refresh_longlat_lines_flag=static_cast<bool>(flag);
}  

void Earth::set_countries_mask(unsigned int flag)
{
   countries_group_ptr->get_OSGgroup_ptr()->setNodeMask(flag);
}  

void Earth::toggle_borders_display()
{
   display_borders = !display_borders;
   set_borders_mask(static_cast<unsigned int>(display_borders));
}

void Earth::set_borders_mask(unsigned int flag)
{
   borders_group_ptr->get_OSGgroup_ptr()->setNodeMask(flag);
}  

unsigned int Earth::get_borders_mask() const
{
   return borders_group_ptr->get_OSGgroup_ptr()->getNodeMask();
}  

void Earth::toggle_cities_display()
{
   display_cities = ! display_cities;
   set_cities_mask(static_cast<unsigned int>(display_cities));
   set_minor_cities_mask(static_cast<unsigned int>(display_cities));
}

void Earth::set_cities_mask(unsigned int flag)
{
   cities_group_ptr->get_OSGgroup_ptr()->setNodeMask(flag);
}  

void Earth::set_minor_cities_mask(unsigned int flag)
{
   minor_cities_group_ptr->get_OSGgroup_ptr()->setNodeMask(flag);
}  

// ---------------------------------------------------------------------
osg::Group* Earth::generate_drawable_group()
{
   drawable_group_refptr->addChild(
      LatLongGrid_ptr->get_LongLinesGroup_ptr()->get_OSGgroup_ptr());
   drawable_group_refptr->addChild(
      LatLongGrid_ptr->get_LatLinesGroup_ptr()->get_OSGgroup_ptr());
   drawable_group_refptr->addChild(countries_group_ptr->get_OSGgroup_ptr());
   drawable_group_refptr->addChild(borders_group_ptr->get_OSGgroup_ptr());
   drawable_group_refptr->addChild(cities_group_ptr->get_OSGgroup_ptr());
   drawable_group_refptr->addChild(minor_cities_group_ptr->get_OSGgroup_ptr());
   return drawable_group_refptr.get();
}
     
// ==========================================================================
// Cartesian to spherical coordinate system conversion member functions
// ==========================================================================

// Member function long_lat_corners_to_plane

void Earth::long_lat_corners_to_plane(vector<twovector>& long_lat_corner)
{
   cout.precision(10);

   twovector long_lat_avg(0,0);
   threevector XYZ_avg(0,0,0);

   vector<threevector> XYZ;
   for (unsigned int i=0; i<long_lat_corner.size(); i++)
   {
      long_lat_avg += long_lat_corner[i];

      XYZ.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         long_lat_corner[i].get(0),long_lat_corner[i].get(1),0));
      XYZ_avg += XYZ.back();
      
   }
   long_lat_avg /= long_lat_corner.size();
   long_lat_corner.push_back(long_lat_avg);

   XYZ_avg /= XYZ.size();
   XYZ.push_back(XYZ_avg);

   plane P(XYZ);

   cout << "plane origin = " << P.get_origin() << endl;
   cout << "ahat = " << P.get_ahat() << endl;
   cout << "bhat = " << P.get_bhat() << endl;
   cout << "nhat = " << P.get_nhat() << endl;

   vector<twovector> AB;
   for (unsigned int i=0; i<XYZ.size(); i++)
   {
      threevector ABN=P.coords_wrt_plane(XYZ[i]);
      AB.push_back(twovector(ABN.get(0),ABN.get(1)));
      cout << "i = " << i 
           << " long = " << long_lat_corner[i].get(0)
           << " lat = " << long_lat_corner[i].get(1)
           << " X = " << XYZ[i].get(0) 
           << " Y = " << XYZ[i].get(1) 
           << " Z = " << XYZ[i].get(2) 
           << " A = " << ABN.get(0) 
           << " B = " << ABN.get(1)
           << " N = " << ABN.get(2) << endl;
   }

   homography H;
   H.parse_homography_inputs(long_lat_corner,AB);
   H.compute_homography_matrix();
//   double RMS_residual=
      H.check_homography_matrix(long_lat_corner,AB);
}

// ---------------------------------------------------------------------
// Member function UTM_corners_to_plane

void Earth::UTM_corners_to_plane(
   int ZoneNumber,bool northern_hemisphere_flag,
   vector<threevector>& UTM_corner)
{
//   cout << "inside Earth::UTM_corners_to_plane()" << endl;
   
   cout.precision(10);
   vector<threevector> UTM,XYZ;
   for (unsigned int i=0; i<UTM_corner.size(); i++)
   {
      double alt_1=0.0;
      UTM.push_back(threevector(
         UTM_corner[i].get(0),UTM_corner[i].get(1),alt_1));

      double curr_long,curr_lat;
      latlongfunc::UTMtoLL(
         ZoneNumber,northern_hemisphere_flag,
         UTM_corner[i].get(1),UTM_corner[i].get(0),curr_lat,curr_long);

      XYZ.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         curr_long,curr_lat,alt_1));

      cout << "E = " << UTM.back().get(0)
           << " N = " << UTM.back().get(1)
           << " A = " << UTM.back().get(2) << endl;
      cout << "long = " << curr_long 
           << " lat = " << curr_lat
           << " alt_1 = " << alt_1 << endl;
      cout << "X = " << XYZ.back().get(0)
           << " Y = " << XYZ.back().get(1)
           << " Z = " << XYZ.back().get(2) << endl;

      double alt_2=100.0;
      UTM.push_back(threevector(
         UTM_corner[i].get(0),UTM_corner[i].get(1),alt_2));
      XYZ.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         curr_long,curr_lat,alt_2));

      cout << "E = " << UTM.back().get(0)
           << " N = " << UTM.back().get(1)
           << " A = " << UTM.back().get(2) << endl;
      cout << "long = " << curr_long 
           << " lat = " << curr_lat
           << " alt_2 = " << alt_2 << endl;
      cout << "X = " << XYZ.back().get(0)
           << " Y = " << XYZ.back().get(1)
           << " Z = " << XYZ.back().get(2) << endl;
   }

   projective P(UTM.size());
   P.parse_projective_inputs(UTM,XYZ);
   P.compute_projective_matrix();
//   double residual=
      P.check_projective_matrix(UTM,XYZ);
}

// ==========================================================================
// Postgres database retrieval methods:
// ==========================================================================

void Earth::retrieve_borders_from_PostGIS_database(
   string borders_tablename,
   double longitude_min,double longitude_max,
   double latitude_min,double latitude_max)
{   

//   const osg::Vec4 border_color(0.95686 , 0.55294 , 0.93725 , 1.0);
//   const osg::Vec4 border_color(0.4784,0.2764,0.4686,1.0);
   osg::Vec4 border_color(0.621960 , 0.359411, 0.609215, 1.0);
   colorfunc::Color country_name_color=colorfunc::pink;
   retrieve_borders_from_PostGIS_database(
      borders_tablename,longitude_min,longitude_max,
      latitude_min,latitude_max,border_color,country_name_color);
}

void Earth::retrieve_borders_from_PostGIS_database(
   string borders_tablename,
   double longitude_min,double longitude_max,
   double latitude_min,double latitude_max,
   colorfunc::Color country_color)
{   
   osg::Vec4 border_color=colorfunc::get_OSG_color(country_color);
   retrieve_borders_from_PostGIS_database(
      borders_tablename,longitude_min,longitude_max,
      latitude_min,latitude_max,border_color,country_color);
}

void Earth::retrieve_borders_from_PostGIS_database(
   string borders_tablename,
   double longitude_min,double longitude_max,
   double latitude_min,double latitude_max,
   osg::Vec4& border_color,colorfunc::Color& country_name_color)
{   
//   cout << "inside Earth::retrieve_borders_from_PostGIS_database()" << endl;
//   cout << "lon_min = " << longitude_min << " lon_max = " << longitude_max
//        << endl;
//   cout << "lat_min = " << latitude_min << " lat_max = " << latitude_max
//        << endl;

   if (PostGIS_database_ptr == NULL || 
       !(PostGIS_database_ptr->get_connection_status_flag())) return;

   PostGIS_database_ptr->read_table(borders_tablename);
   PostGIS_database_ptr->set_earth_flag(true);

   PostGIS_database_ptr->pushback_gis_bbox(
      longitude_min,longitude_max,latitude_min,latitude_max);
   PostGIS_database_ptr->set_altitude(1000);	// meters

   PostGIS_database_ptr->set_CountriesGroup_ptr(countries_group_ptr);
   PostGIS_database_ptr->set_PolyLinesGroup_ptr(borders_group_ptr);
   

   PostGIS_database_ptr->set_country_name_color(country_name_color);

   string geom_name;
   if (borders_tablename=="country_borders")
   {
      geom_name="cntry_name";
   }
   else if (borders_tablename=="state_borders")
   {
      geom_name="state";
   }
   
   PostGIS_database_ptr->parse_table_contents(geom_name);
   PostGIS_database_ptr->popback_gis_bbox();
   
// Bunch together multiple geometries within GeometricalsGroup's single
// geode.  Then attach that single geode to GeometricalGroup's single
// PAT.  Finally, add GeometricalGroup's single PAT to PolyLinesGroup's
// OSGgroup:

   const threevector reference_vertex(0,0,0);
   borders_group_ptr->attach_bunched_geometries_to_OSGsubPAT(
      reference_vertex,0);
   borders_group_ptr->set_uniform_color(border_color);

   set_altitude_dependent_border_width();
   borders_group_ptr->update_display();

   countries_group_ptr->reset_colors();
   countries_group_ptr->attach_bunched_geometries_to_OSGsubPAT(
      reference_vertex,0);
}

// ---------------------------------------------------------------------
void Earth::set_altitude_dependent_border_width()
{
   double linewidth=init_border_width;

   if (log_eye_alt >= 6.5 && log_eye_alt < 7.0)
   {
      linewidth=1.5*init_border_width;
   }
   else if (log_eye_alt >= 6.0 && log_eye_alt < 6.5)
   {
      linewidth=2.5*init_border_width;
   }
   else if (log_eye_alt >= 5.5 && log_eye_alt < 6.0)
   {
      linewidth=3.5*init_border_width;
   }
   else if (log_eye_alt >= 5.0 && log_eye_alt < 5.5)
   {
      linewidth=5.0*init_border_width;
   }
   else if (log_eye_alt < 5.0)
   {
      linewidth=7.0*init_border_width;
   }
//   cout << "log_eye_alt = " << log_eye_alt 
//        << " border linewidth = " << linewidth << endl;
   borders_group_ptr->set_width(linewidth);
}

// ---------------------------------------------------------------------
void Earth::retrieve_cities_from_PostGIS_database(
   double longitude_min,double longitude_max,
   double latitude_min,double latitude_max)
{   
   colorfunc::Color city_color=colorfunc::red;
   retrieve_cities_from_PostGIS_database(
      longitude_min,longitude_max,latitude_min,latitude_max,city_color);
}

void Earth::retrieve_cities_from_PostGIS_database(
   double longitude_min,double longitude_max,
   double latitude_min,double latitude_max,
   colorfunc::Color city_color)
{   
//   cout << "inside Earth::retrieve_cities_from_PostGIS_database()" << endl;
//   cout << "long_min = " << longitude_min << " long_max = " << longitude_max
//        << endl;
//   cout << "lat_min = " << latitude_min << " lat_max = " << latitude_max
//        << endl;

   if (PostGIS_database_ptr == NULL || 
       !(PostGIS_database_ptr->get_connection_status_flag())) return;

   const string cities_table_name="cities";
   PostGIS_database_ptr->read_table(cities_table_name);
   PostGIS_database_ptr->set_city_color(city_color);
   PostGIS_database_ptr->set_earth_flag(true);
   cities_group_ptr->set_rh(10000,200);

   PostGIS_database_ptr->pushback_gis_bbox(
      longitude_min,longitude_max,latitude_min,latitude_max);
   PostGIS_database_ptr->set_CitiesGroup_ptr(cities_group_ptr);

//   PostGIS_database_ptr->set_reference_vertex(reference_vertex);
   PostGIS_database_ptr->set_altitude(200);	// meters

   PostGIS_database_ptr->parse_table_contents();
   PostGIS_database_ptr->popback_gis_bbox();

   const threevector reference_vertex(0,0,0);
   cities_group_ptr->attach_bunched_geometries_to_OSGsubPAT(
      reference_vertex,0);
}

// ---------------------------------------------------------------------
// Member function retrive_US_cities_from_PostGIS_database() is a
// specialized method which populates *minor_cities_group_ptr with
// Cylinders based upon cities whose 1990 populations exceed some
// minimal threshold.  

void Earth::retrieve_US_cities_from_PostGIS_database(
   double longitude_min,double longitude_max,
   double latitude_min,double latitude_max,
   colorfunc::Color city_color,int population_threshold)
{   
//   cout << "inside Earth::retrieve_US_cities_from_PostGIS_database()" << endl;
//   cout << "long_min = " << longitude_min << " long_max = " << longitude_max
//        << endl;
//   cout << "lat_min = " << latitude_min << " lat_max = " << latitude_max
//        << endl;

   if (PostGIS_database_ptr == NULL || 
       !(PostGIS_database_ptr->get_connection_status_flag())) return;

   const string cities_table_name="US_cities";
   PostGIS_database_ptr->read_table(cities_table_name);
   PostGIS_database_ptr->set_city_color(city_color);
   PostGIS_database_ptr->set_earth_flag(true);
   minor_cities_group_ptr->set_rh(3000,120);

   PostGIS_database_ptr->pushback_gis_bbox(
      longitude_min,longitude_max,latitude_min,latitude_max);
   PostGIS_database_ptr->set_CitiesGroup_ptr(minor_cities_group_ptr);

//   PostGIS_database_ptr->set_reference_vertex(reference_vertex);
   PostGIS_database_ptr->set_altitude(200);	// meters

   string geom_name="name";
//   string where_clause="pop_1990 > 50000";
   string where_clause="pop_1990 > "+stringfunc::number_to_string(
      population_threshold);
   PostGIS_database_ptr->parse_table_contents(geom_name,where_clause);
   PostGIS_database_ptr->popback_gis_bbox();

   const threevector reference_vertex(0,0,0);
   minor_cities_group_ptr->attach_bunched_geometries_to_OSGsubPAT(
      reference_vertex,0);
   double geometrical_factor=1.0;
   double text_scale_factor=0.1;
   minor_cities_group_ptr->change_size(geometrical_factor,text_scale_factor);
}

// ---------------------------------------------------------------------
bool Earth::long_lat_for_specified_geosite(
   string geosite_name,geopoint& site_geopoint)
{
   twovector LongLat;
   if (geosite_name=="Boston" || geosite_name=="boston")
   {
      double longitude=-71.06;	// degs
      double latitude=42.36;	// degs
      double altitude=10000;
      site_geopoint=geopoint(longitude,latitude,altitude);
      cout << "Boston geopoint = " << site_geopoint << endl;
      return true;
   }
   else if (geosite_name=="New York" || geosite_name=="new york" ||
            geosite_name=="New York City" || geosite_name=="new york city")
   {

// NYC UTM zone 18

      double longitude=-74.01;	// degs		
      double latitude=40.71;	// degs		

//      double longitude=-73.95;	// degs		
//      double latitude=40.775;	// degs		

      double altitude=10000;
      site_geopoint=geopoint(longitude,latitude,altitude);
      cout << "New York City geopoint = " << site_geopoint << endl;
      return true;
   }
   else if (geosite_name=="Baghdad" || geosite_name=="baghdad")
   {

// Baghdad UTM zone 38

//      double longitude=44.328;	// degs		// Fused sat EO demo value
//      double latitude=33.298;	// degs		// Fused sat EO demo value

      double longitude=44.37;	// degs		// ISDS demo value
      double latitude=33.30;	// degs		// ISDS demo value

//      double longitude=44.24;	// degs		// HAFB movie value
//      double latitude=33.28;    // degs		// HAFB movie value

      double altitude=10000;
      site_geopoint=geopoint(longitude,latitude,altitude);
      cout << "Baghdad geopoint = " << site_geopoint << endl;
      return true;
   }
   else if (geosite_name=="San Clemente" || geosite_name=="san clemente")
   {
      double longitude=-118.5;	// degs
      double latitude=32.91;   	// degs
      double altitude=5000;
      site_geopoint=geopoint(longitude,latitude,altitude);
      cout << "San Clemente geopoint = " << site_geopoint << endl;
      return true;
   }
   else
   {
      const string cities_table_name="cities";   
      if (PostGIS_database_ptr->get_connection_status_flag() &&
          PostGIS_database_ptr->long_lat_for_specified_geosite(
             cities_table_name,geosite_name,LongLat))
      {
         double city_altitude=pow(10,5.41);
         site_geopoint=geopoint(LongLat.get(0),LongLat.get(1),city_altitude);
         cout << "site geopoint = " << site_geopoint << endl;
         return true;
      }
      else
      {
         return false;
      }
   } // geosite_name = Boston conditional
}

