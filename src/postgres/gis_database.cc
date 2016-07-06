// =========================================================================
// Gis_Database class member function definitions 
// =========================================================================
// Last modified on 12/4/10; 10/20/11; 1/11/12; 4/5/14
// =========================================================================

#include <algorithm>
#include <iostream>
#include "ogrsf_frmts.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "general/outputfuncs.h"
#include "postgres/gis_database.h"
#include "general/stringfuncs.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void gis_database::allocate_member_objects()
{
   input_SRS_ptr=new OGRSpatialReference();
   output_SRS_ptr=new OGRSpatialReference();
   polyline_vertices_map_ptr=new POLYLINE_VERTICES_MAP;
}

void gis_database::initialize_member_objects()
{
   category_column_flag=false;
   TableName="";
   earth_flag=false;
   flat_grid_flag=false;
   specified_UTM_zonenumber=-1;
//   altitude=17;	// approx height of NYC roads near Rockefeller Center
   altitude=100;	// meters

   curr_polyline_vertices_ptr=NULL;
   Ellipsoid_model_ptr=NULL;
   poLayer_ptr=NULL;
   poCT_ptr=NULL;
   po_inverseCT_ptr=NULL;
   poFeature_ptr=NULL;
   poGeometry_ptr=NULL;
}		 

// ---------------------------------------------------------------------
void gis_database::open_OGR_connection()
{
//   cout << "inside gis_database::open_OGR_connection()" << endl;
   postgis_connection_status_flag=false;

   OGRRegisterAll();
   if (get_connection_status_flag())
   {

// On 1/26/07, Matt Harrington discovered within the source code
// gdal-1.4.0/ogr/ogrsf_frmts/pg/ogrpgdatasource.cpp that chanting PGB
// rather than PG forces PostGIS output to be transmitted/received in
// binary rather than ascii format.  This leads to a noticeable
// database access speedup:

//      string connection_string="PGB: host="+get_hostname()+
//         " user="+get_username()+" dbname="+get_databasename();
      string connection_string="PGB: host="+get_hostname()+
         " user="+get_username()+" dbname="+get_databasename()
         +" password="+get_password();

// On 10/19/11, we discovered to our horror that the following line
// failed when it had a second FALSE argument (indicating read-only
// access) for a newly created Tstorm postgis-enabled database.  This
// previous FALSE line works fine for opening all pre-Oct-2011
// databases...

// As of Jan 2012, we believe that it's always best to pass TRUE as
// the second argument to the following Open command in order to
// enable read-write access to the postgis database:

    poDS_ptr = OGRSFDriverRegistrar::Open(
       connection_string.c_str(), TRUE );
//         connection_string.c_str(), FALSE );
      if (poDS_ptr == NULL)
      {
         cout << "Cannot connect to PostGIS database" << endl;
         cout << "connection_string = " << connection_string << endl;
      }
      else
      {
         postgis_connection_status_flag=true;
         n_layers=poDS_ptr->GetLayerCount();
//         cout << "n_layers = " << n_layers << endl;
      }
   } // get_connection_status_flag() conditional
}		 

// ---------------------------------------------------------------------
gis_database::gis_database(string host,string dbname,string user):
   database(host,dbname,user)
{
//   cout << "inside gis_database constructor()" << endl;
   allocate_member_objects();
   initialize_member_objects();

   open_OGR_connection();
}

// ---------------------------------------------------------------------
// Copy constructor:

gis_database::gis_database(const gis_database& d)
{
   docopy(d);
}

gis_database::~gis_database()
{
//   cout << "inside gis_database destructor" << endl;

   OGRDataSource::DestroyDataSource( poDS_ptr );
   delete input_SRS_ptr;
   delete output_SRS_ptr;
   delete_polyline_vertices_map();
}

void gis_database::delete_polyline_vertices_map()
{
   for (POLYLINE_VERTICES_MAP::iterator itr=polyline_vertices_map_ptr->
           begin(); itr != polyline_vertices_map_ptr->end(); ++itr)
   {
      delete itr->second;
   }
   delete polyline_vertices_map_ptr;
}

void gis_database::purge_polyline_vertices_map()
{
   delete_polyline_vertices_map();
   polyline_vertices_map_ptr=new POLYLINE_VERTICES_MAP;
}

// ---------------------------------------------------------------------
void gis_database::docopy(const gis_database& d)
{
}

// Overload = operator:

gis_database& gis_database::operator= (const gis_database& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const gis_database& d)
{
   outstream << endl;
   outstream << "GISpoint_tablenames.size() = "
             << d.GISpoint_tablenames.size() << endl;
   return(outstream);
}

// =========================================================================
// Database parsing member functions
// =========================================================================

void gis_database::read_table(string TableName)
{
//   cout << "inside gis_database::read_table, TableName = "
//        << TableName << endl;
   
   if (postgis_connection_status_flag)
   {
      this->TableName=TableName;
      poLayer_ptr = poDS_ptr->GetLayerByName(TableName.c_str());
      poLayer_ptr->ResetReading();
   }

//   cout << "poLayer_ptr = " << poLayer_ptr << endl;
}

// ---------------------------------------------------------------------
// Member functions setup_coordinate_transformation 

void gis_database::setup_coordinate_transformation(
   string input_wellknown_GCS_name,
   int input_StatePlane_number,
   int output_UTM_zonenumber,bool output_northern_hemisphere_flag)
{
   if (postgis_connection_status_flag)
   {
      input_SRS_ptr->SetWellKnownGeogCS( input_wellknown_GCS_name.c_str() );
      if (input_StatePlane_number > 0)
      {
         input_SRS_ptr->SetStatePlane( input_StatePlane_number, TRUE );
      }

      output_SRS_ptr->SetWellKnownGeogCS( "WGS84" );
      if (output_UTM_zonenumber > 0)
      {
         output_SRS_ptr->SetUTM( 
            output_UTM_zonenumber, output_northern_hemisphere_flag );
      }
      poCT_ptr = OGRCreateCoordinateTransformation(
         input_SRS_ptr,output_SRS_ptr);

      if (poCT_ptr == NULL)
      {
         cout << "Transformation from "+
            input_wellknown_GCS_name+" to WGS84 is invalid!" << endl;
         exit(-1);
      } 
      else
      {
         po_inverseCT_ptr = OGRCreateCoordinateTransformation(
            output_SRS_ptr,input_SRS_ptr);
      }
   } // connection_status_flag conditional
}

// ---------------------------------------------------------------------
// Member function pushback_gis_bbox first pushes the input bbox
// coordinates onto member STL vectors bbox and bbox_stateplane.  If
// the inverse Coordinate transformation which maps from UTM to State
// Plane coordinates is non-null, this method transforms the bbox' UTM
// coordinates into State Plane values.  The latter coordinates may
// subsequently be used to set spatial filter bounding rectangles for
// various layers when they are read in from the PostGis database.

void gis_database::pushback_gis_bbox(
   double xmin,double xmax,double ymin,double ymax)
{
//   cout << "inside gis_database::pushback_gis_bbox()" << endl;
//   cout << "xmin = " << xmin << " xmax = " << xmax
//        << " ymin = " << ymin << " ymax = " << ymax << endl;
   
   bounding_box curr_bbox(xmin,xmax,ymin,ymax);
//   cout << "curr_bbox = " << curr_bbox << endl;
   bbox.push_back(curr_bbox);

   if (po_inverseCT_ptr != NULL) 
   {
      po_inverseCT_ptr->Transform(1,curr_bbox.get_xmin_ptr(),
                                  curr_bbox.get_ymin_ptr());
      po_inverseCT_ptr->Transform(1,curr_bbox.get_xmax_ptr(),
                                  curr_bbox.get_ymax_ptr());
   }
   bbox_stateplane.push_back(curr_bbox);
}

void gis_database::popback_gis_bbox()
{
   bbox.pop_back();
   bbox_stateplane.pop_back();
}

// ---------------------------------------------------------------------
bool gis_database::parse_table_contents(string geom_name,string where_clause)
{
//   cout << "inside gis_database::parse_table_contents()" << endl;
//   cout << "geom_name = " << geom_name << endl;

   if (!initialize_table_parsing(geom_name,where_clause)) return false;
    
   row_ID=0;
   while( (poFeature_ptr = poLayer_ptr->GetNextFeature()) != NULL)
   {
//         cout << "poFeature_ptr = " << poFeature_ptr << endl;
         
      string banner="Row ID = "+stringfunc::number_to_string(row_ID);
      if (row_ID%10000==0) outputfunc::write_banner(banner);

//         OGRFeatureDefn* poFDefn_ptr = poLayer_ptr->GetLayerDefn();
//         for( int iField = 0; iField < poFDefn_ptr->GetFieldCount(); 
//              iField++ )
//         {
//            OGRFieldDefn* FieldDefn_ptr = poFDefn_ptr->GetFieldDefn( iField );
//            string field_name(FieldDefn_ptr->GetNameRef());
//            cout << "i = " << iField << " field_name = " << field_name 
//                 << endl;
//            if ( FieldDefn_ptr->GetType() == OFTInteger )
//            {
//               cout << poFeature_ptr->GetFieldAsInteger( iField ) << endl;
//            }
//            else if ( FieldDefn_ptr->GetType() == OFTReal )
//            {
//               cout << poFeature_ptr->GetFieldAsDouble(iField) << endl;
//            }
//            else
//            {
//               cout << poFeature_ptr->GetFieldAsString(iField) << endl;
//            }
//         } // loop over iField index labeling feature fields

      poGeometry_ptr = poFeature_ptr->GetGeometryRef();

/*
         cout << "poGeometry_ptr = " << poGeometry_ptr << endl;
         cout << "geom dim = " << poGeometry_ptr->getDimension() << endl;
         cout << "coord dim = " << poGeometry_ptr->getCoordinateDimension() 
              << endl;
         cout << "Geometry name = " 
              << poGeometry_ptr->getGeometryName() << endl;
         cout << "Geometry name = " 
              << string(poGeometry_ptr->getGeometryName()) << endl;
*/

      if (poGeometry_ptr != NULL)
      {
         if (poGeometry_ptr->getGeometryType()== wkbMultiLineString)
         {
            parse_multilinestring_geometry();
         }
         else if (poGeometry_ptr->getGeometryType()==wkbLineString)
         {
            parse_linestring_geometry(poGeometry_ptr);
         } 
         else if (poGeometry_ptr->getGeometryType()== wkbMultiPoint)
         {
            parse_multipoint_geometry();
         }
         else if (poGeometry_ptr->getGeometryType()== wkbPoint)
         {
            if (earth_flag)
            {
               double longitude,latitude;
               threevector curr_point,r_hat;
               parse_point_geometry(
                  poGeometry_ptr,longitude,latitude,curr_point,r_hat);
            }
         }
         else if (poGeometry_ptr->getGeometryType()==wkbMultiPolygon)
         {
//               cout << "earth_flag = " << earth_flag << endl;
            if (earth_flag)
            {
//                  output_country_border_polygons();
               twovector centroid;
               parse_worldline_geometry(centroid);
            }
            else
            {
               parse_multipolygon_geometry();
            }
         } 
      } // poGeometry_ptr != NULL conditional

      OGRFeature::DestroyFeature( poFeature_ptr );
      row_ID++;

   } // while loop
//      cout  << "Finished parsing table contents()" << endl;

// Reset earth_flag back to false before exiting this member
// function:

   earth_flag=false;

   return true;
}

// ---------------------------------------------------------------------
// Member function initialize_table_parsing()

bool gis_database::initialize_table_parsing(
   string geom_name,string where_clause)
{
//   cout << "inside gis_database::initialize_table_parsing()" << endl;
//   cout << "geom_name = " << geom_name << endl;
//   cout << "where_clause = " << where_clause << endl;

   if (!postgis_connection_status_flag) return false;
   
   string banner="Parsing PostGIS table contents ";
   outputfunc::write_big_banner(banner);
//      cout << "earth_flag = " << earth_flag << endl;

   this->geom_name=geom_name;

//      cout << "bbox_stateplane.back() = " << endl;
//      cout << bbox_stateplane.back() << endl;
   poLayer_ptr->SetSpatialFilterRect(
      bbox_stateplane.back().get_xmin(),
      bbox_stateplane.back().get_ymin(),
      bbox_stateplane.back().get_xmax(),
      bbox_stateplane.back().get_ymax());

//      cout << "where_clause = " << where_clause << endl;
   if (where_clause.size() > 0)
   {
      poLayer_ptr->SetAttributeFilter(where_clause.c_str());
   }

   return true;
}

// ---------------------------------------------------------------------
// Member function parse_multilinestring_geometry

void gis_database::parse_multilinestring_geometry()
{
//   cout << "inside gis_database::parse_multilinestring_geometry()" << endl;
   OGRMultiLineString* poMultiLineString=dynamic_cast<OGRMultiLineString*>(
      poGeometry_ptr);

   for (int n_polyline=0; n_polyline < poMultiLineString->getNumGeometries(); 
        n_polyline++)
   {
      parse_linestring_geometry(
         poMultiLineString->getGeometryRef(n_polyline));
   } // loop over index n_polyline labeling LineStrings
}

// ---------------------------------------------------------------------
// Member function parse_linestring_geometry

void gis_database::parse_linestring_geometry(OGRGeometry* OGRGeometry_ptr)
{
//   cout << "inside gis_database::parse_linestring_geom" << endl;
   
   OGRLineString* poLineString_ptr=dynamic_cast<OGRLineString*>(
      OGRGeometry_ptr);

   curr_polyline_vertices_ptr=generate_new_polyline_vertices();

   for (int p=0; p<poLineString_ptr->getNumPoints(); p++)
   {
      double x=poLineString_ptr->getX(p);
      double y=poLineString_ptr->getY(p);
//      cout << "p = " << p << " x_init = " << x << " y_init = " << y ;

      if (poCT_ptr != NULL) poCT_ptr->Transform(1,&x,&y);
//      cout << "poCT_ptr = " << poCT_ptr << endl;

// Don't bother processing vertices if they lie outside specified
// boundingbox:

      if (x > get_gis_bbox().get_xmin() && x < get_gis_bbox().get_xmax() && 
          y > get_gis_bbox().get_ymin() && y < get_gis_bbox().get_ymax())
      {
         curr_polyline_vertices_ptr->push_back(threevector(x,y,altitude));
//         cout << "x_final = " << x << " y_final = " << y << endl;

// Note added on 1/24/07: Need to use following line to convert
// north_american_boundaries shapefile entries from long/lat coords to
// XYZ earth-centered coords in order to see higher-fidelity borders
// for North and Central America:

//         curr_polyline_vertices_ptr->push_back(
//            Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(x,y,altitude));
      }
   } // loop over p index labeling vertices for a particular polyline
}

// ---------------------------------------------------------------------
// Member function parse_multipolygon_geometry

void gis_database::parse_multipolygon_geometry()
{
//   cout << "inside gis_database::parse_multipolygon_geometry()" << endl;
   
   OGRMultiPolygon* poMultiPolygon=dynamic_cast<OGRMultiPolygon*>(
      poGeometry_ptr);

   for (int n_polygon=0; n_polygon < poMultiPolygon->getNumGeometries(); 
        n_polygon++)
   {
      parse_polygon_geometry(poMultiPolygon->getGeometryRef(n_polygon));
   }
}

// ---------------------------------------------------------------------
// Member function parse_polygon_geometry

void gis_database::parse_polygon_geometry(OGRGeometry* OGRGeometry_ptr)
{
//   cout << "inside gis_database::parse_polygon_geometry()" << endl;

   OGRPolygon* poPolygon_ptr=dynamic_cast<OGRPolygon*>(OGRGeometry_ptr);
   OGRLineString* poRing_ptr=dynamic_cast<OGRLineString*>(
      poPolygon_ptr->getExteriorRing());
//      OGRLinearRing* poRing_ptr=poPolygon_ptr->getExteriorRing();

   curr_polyline_vertices_ptr=generate_new_polyline_vertices();

   for (int p=0; p<poRing_ptr->getNumPoints(); p++)
   {
      double x=poRing_ptr->getX(p);
      double y=poRing_ptr->getY(p);
         cout << "p = " << p << " x = " << x << " y = " << y ;
      if (poCT_ptr != NULL) poCT_ptr->Transform(1,&x,&y);
//      cout << " xnew = " << x << " ynew = " << y << endl;

// Don't bother to process vertices if they lie outside specified
// boundingbox:

      if (x > get_gis_bbox().get_xmin() && x < get_gis_bbox().get_xmax() && 
          y > get_gis_bbox().get_ymin() && y < get_gis_bbox().get_ymax())
      {
         curr_polyline_vertices_ptr->push_back(threevector(x,y,altitude));
      } // (x,y) lies within specified bbox conditional
   } // loop over p index labeling vertices for a particular polygon
}

// ---------------------------------------------------------------------
// Member function parse_OGRPolygon_vertices() loops over all vertices
// within input *poPolygon_ptr and extracts their longitude,latitude
// values and altitudes.  It converts the angular geocoordinates into
// Cartesian coordinates and fills member STL vector
// curr_polyline_vertices with the output.

void gis_database::parse_OGRPolygon_vertices(OGRPolygon* poPolygon_ptr)
{
//   cout << "inside gis_database::parse_OGRPolygon_vertices()" << endl;

   OGRLineString* poRing_ptr=dynamic_cast<OGRLineString*>(
      poPolygon_ptr->getExteriorRing());

   vector<double> point_altitude=geofunc::raise_polyline_above_ellipsoid(
      poRing_ptr,Ellipsoid_model_ptr);
   int n_points=point_altitude.size();

// Recompute polyline vertices' locations in geocentric XYZ space
// using their improved altitude estimates:

   curr_polyline_vertices_ptr=generate_new_polyline_vertices();

   for (int p=0; p<n_points; p++)
   {
      double longitude=poRing_ptr->getX(p);
      double latitude=poRing_ptr->getY(p);

// Crop any polylines at the extermal values set by the
// longitude-latitude bounding box:

      longitude=basic_math::max(longitude,get_gis_bbox().get_xmin());
      longitude=basic_math::min(longitude,get_gis_bbox().get_xmax());

      latitude=basic_math::max(latitude,get_gis_bbox().get_ymin());
      latitude=basic_math::min(latitude,get_gis_bbox().get_ymax());
         
      threevector curr_point;
      if (flat_grid_flag)
      {
         if (specified_UTM_zonenumber < 0)
         {
            cout << "Error in gis_database::parse_OGRPolygon_vertices()"
                 << endl;
            cout << "flat_grid_flag=true" << endl;
            cout << "specified_UTM_zonenumber = "
                 << specified_UTM_zonenumber << endl;
            exit(-1);
         }

//         double border_altitude=50*1000;
         double border_altitude=9 * 1000;
         curr_point=geopoint(
            longitude,latitude,border_altitude,specified_UTM_zonenumber).
            get_UTM_posn();
      }
      else
      {
         curr_point=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
            longitude,latitude,point_altitude[p]);
      } // flat_grid_flag conditional

      curr_polyline_vertices_ptr->push_back(curr_point);
   } // loop over p index labeling vertices for a particular polygon
}

// ---------------------------------------------------------------------
// Member function parse_worldline_geometry() returns an STL
// vector of sorted (in ascending order) areas corresponding to
// separate connected worldline components.

vector<double> gis_database::parse_worldline_geometry(twovector& centroid)
{
//   cout << "inside gis_database::parse_worldline_geometry()" << endl;

   vector<double> worldline_areas;

   double max_area=NEGATIVEINFINITY;

   if (Ellipsoid_model_ptr==NULL)
   {
      cout << 
         "Ellipsoid_model_ptr=NULL in gis_database::parse_worldline_geometry()" << endl;
      return worldline_areas;
   }

   OGRMultiPolygon* poMultiPolygon=dynamic_cast<OGRMultiPolygon*>(
      poGeometry_ptr);

   OGRPoint* centroid_ptr=new OGRPoint();
   for (int n_polygon=0; n_polygon < poMultiPolygon->getNumGeometries(); 
        n_polygon++)
   {
      OGRPolygon* poPolygon_ptr=dynamic_cast<OGRPolygon*>(
         poMultiPolygon->getGeometryRef(n_polygon));
      poPolygon_ptr->Centroid(centroid_ptr);

      double curr_area=poPolygon_ptr->get_Area();
      worldline_areas.push_back(curr_area);
      
      if (curr_area > max_area)
      {
         max_area=curr_area;
         centroid=twovector(centroid_ptr->getX(),centroid_ptr->getY());
      }
      parse_OGRPolygon_vertices(poPolygon_ptr);
   } // loop over index n_polygon labeling Polygons
   delete centroid_ptr;

   std::sort(worldline_areas.begin(),worldline_areas.end());
//   for (int w=0; w<worldline_areas.size(); w++)
//   {
//      cout << "w = " << w
//           << " worldline_area = " << worldline_areas[w] << endl;
//   }

   return worldline_areas;
}

// ---------------------------------------------------------------------
// Member function parse_multipoint_geometry

void gis_database::parse_multipoint_geometry()
{
   OGRMultiPoint* poMultiPoint=dynamic_cast<OGRMultiPoint*>(poGeometry_ptr);
   for (int n_point=0; n_point < poMultiPoint->getNumGeometries(); n_point++)
   {
      double longitude,latitude;
      threevector curr_point,r_hat;
      parse_point_geometry(
         poMultiPoint->getGeometryRef(n_point),longitude,latitude,
         curr_point,r_hat);
   } // loop over index n_point labeling Points
}

// ---------------------------------------------------------------------
// Member function parse_point_geometry()

bool gis_database::parse_point_geometry(
   OGRGeometry* OGRGeometry_ptr,double& longitude,double& latitude,
   threevector& curr_point,threevector& r_hat)
{
//   cout << "inside postgis_database::parse_point_geometry()" << endl;

   OGRPoint* poPoint_ptr=dynamic_cast<OGRPoint*>(OGRGeometry_ptr);

   longitude=poPoint_ptr->getX();
   latitude=poPoint_ptr->getY();
//      cout << "p = " << p << " x = " << x << " y = " << y ;
   if (poCT_ptr != NULL) poCT_ptr->Transform(1,&longitude,&latitude);
//   cout << " longitude = " << x << " latitude = " << y << endl;

// Don't bother processing vertices if they lie outside specified
// boundingbox:

   bool point_inside_bbox_flag=false;
   if (longitude > get_gis_bbox().get_xmin() && 
       longitude < get_gis_bbox().get_xmax() && 
       latitude > get_gis_bbox().get_ymin() && 
       latitude < get_gis_bbox().get_ymax())
   {
      const osg::Vec3f Z_hat(0,0,1);
      if (flat_grid_flag)
      {
         if (specified_UTM_zonenumber < 0)
         {
            cout << "Error in gis_database::parse_point_geometry()!"
                 << endl;
            cout << "flat_grid_flag=true" << endl;
            cout << "specified_UTM_zonenumber = "
                 << specified_UTM_zonenumber << endl;
            exit(-1);
         }

         double point_altitude=2*1000;	// meters
         curr_point=geopoint(
            longitude,latitude,point_altitude,specified_UTM_zonenumber).
            get_UTM_posn();
         r_hat=z_hat;
      }
      else
      {
         curr_point=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
            longitude,latitude,altitude);
         Ellipsoid_model_ptr->compute_east_north_radial_dirs(
            latitude,longitude);
         r_hat=Ellipsoid_model_ptr->get_radial_hat();
      } // flat_grid_flag conditional
      point_inside_bbox_flag=true;
   } // point inside bbox conditional

   return point_inside_bbox_flag;
}

// ---------------------------------------------------------------------
// Member function generate_new_polyline_vertices()

vector<threevector>* gis_database::generate_new_polyline_vertices()
{
//   cout << "inside gis_database::generate_new_polyline_vertices_to_map()" << endl;

   vector<threevector>* polyline_vertices_ptr=new vector<threevector>;
   int polyline_ID=polyline_vertices_map_ptr->size();
   (*polyline_vertices_map_ptr)[polyline_ID]=polyline_vertices_ptr;
   return polyline_vertices_ptr;
}

// =========================================================================
// GIS info retrieval member functions
// =========================================================================

// Member function long_lat_for_specified_geosite takes in some
// PostGIS table along with an input string for geosite of interest.
// It executes a Postgres call to select the longitude and latitude
// coordinates corresponding to the input string.  If found, this
// boolean method returns true and the geographical location within
// output twovector LongLat.  Otherwise, this method returns false.

bool gis_database::long_lat_for_specified_geosite(
   string TableName,string geosite_name,twovector& LongLat)
{
   string select_command = 
      "SELECT x(the_geom) as Longitude,y(the_geom) as Latitude from ";
   select_command += TableName;
   select_command += " where lower(name)=lower('"+geosite_name+"')";
//   cout << "select_command = " << select_command << endl;

   vector<string> commands;
   commands.push_back(select_command);

   set_SQL_commands(commands);
   execute_SQL_commands();

   Genarray<string>* field_array_ptr=get_field_array_ptr();
//   cout << "*field_array_ptr = " << *field_array_ptr << endl;

   if (field_array_ptr->get_mdim() >= 1)
   {
      LongLat=twovector(
         stringfunc::string_to_number(field_array_ptr->get(0,0)),
         stringfunc::string_to_number(field_array_ptr->get(0,1)));
//      cout << "LongLat = " << LongLat << endl;
      return true;
   }
   else
   {
//      cout << "Could not find long/lat for input location" << endl;
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function output_country_border_polygons is a one-time
// special-purpose variant
// gis_database::parse_worldline_geometry().  It was written in
// order to extract to text files the longitude,latitude vertices for
// all the countries of the world.  We wrote this hack in order to
// generate KML shape files for countries' borders.

void gis_database::output_country_border_polygons()
{
//   cout << "inside gis_database::output_country_border_polygons()" 
//        << endl;

   OGRMultiPolygon* poMultiPolygon=dynamic_cast<OGRMultiPolygon*>(
      poGeometry_ptr);

   string country_name=
      poFeature_ptr->GetFieldAsString(geom_name.c_str());
//   cout << "country = " << country_name << endl;

// Hack : For reasons we don't understand, the very first country name
// is not read out from the PostGIS world borders table.  So we
// hardwire it here:

   if (country_name.size()==0) country_name="Russia";

   string simplified_country_name=
      geofunc::generate_simplified_country_name(country_name);

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/OSG/countries/";
   subdir += simplified_country_name+"/";
   filefunc::dircreate(subdir);

   for (int n_polygon=0; n_polygon < poMultiPolygon->getNumGeometries(); 
        n_polygon++)
   {
      string curr_output_filename=subdir+simplified_country_name+"_"+
         stringfunc::integer_to_string(n_polygon,3)+".dat";
      ofstream outstream;
      filefunc::openfile(curr_output_filename,outstream);

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

         outstream << longitude << " , "
                  << latitude << " , " 
                  << altitude << endl;

      } // loop over p index labeling vertices for a particular polygon

      outstream << endl;
      filefunc::closefile(curr_output_filename,outstream);

   } // loop over index n_polygon labeling Polygons
}
