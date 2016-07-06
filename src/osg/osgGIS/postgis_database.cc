// =========================================================================
// Postgis_Database class member function definitions 
// =========================================================================
// Last modified on 5/13/10; 5/14/10; 11/30/10; 12/4/10
// =========================================================================

#include <iostream>
#include "ogrsf_frmts.h"

#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/osgGIS/GISfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/polyline.h"
#include "osg/osgGIS/postgis_database.h"
#include "general/stringfuncs.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void postgis_database::allocate_member_objects()
{
}

void postgis_database::initialize_member_objects()
{
   country_name_color=colorfunc::pink;
   city_color=colorfunc::red;
   PolyLine_text_size=5;
   CountriesGroup_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   CitiesGroup_ptr=NULL;
}		 

// ---------------------------------------------------------------------
postgis_database::postgis_database(string host,string dbname,string user):
   gis_database(host,dbname,user)
{
//   cout << "inside postgis_database constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

postgis_database::postgis_database(const postgis_database& d):
   gis_database(d)
{
   docopy(d);
}

postgis_database::~postgis_database()
{
//   cout << "inside postgis_database destructor" << endl;
}

// ---------------------------------------------------------------------
void postgis_database::docopy(const postgis_database& d)
{
}

// Overload = operator:

postgis_database& postgis_database::operator= (const postgis_database& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const postgis_database& d)
{
   outstream << endl;
   outstream << "GISpoint_tablenames.size() = "
             << d.GISpoint_tablenames.size() << endl;
   return outstream;
}

// =========================================================================
// =========================================================================

bool postgis_database::parse_table_contents(
   string geom_name,string where_clause)
{
//   cout << "inside postgis_database::parse_table_contents()" << endl;
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

//         cout << "poGeometry_ptr = " << poGeometry_ptr << endl;
//         cout << "geom dim = " << poGeometry_ptr->getDimension() << endl;
//         cout << "coord dim = " << poGeometry_ptr->getCoordinateDimension() 
//              << endl;
//         cout << "Geometry name = " 
//              << poGeometry_ptr->getGeometryName() << endl;
//         cout << "Geometry name = " 
//              << string(poGeometry_ptr->getGeometryName()) << endl;


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
               const double city_text_displacement=25*1000;	// meters
//               const double city_text_displacement=50*1000;	// meters
               const double city_text_size=20000;

               parse_worldpoint_geometry(
                  poGeometry_ptr,CitiesGroup_ptr,city_color,
                  city_text_displacement,city_text_size);
            }
         }
         else if (poGeometry_ptr->getGeometryType()==wkbMultiPolygon)
         {
            if (earth_flag)
            {
//               output_country_border_polygons();
               parse_worldline_geometry();
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
// Member function parse_multilinestring_geometry

void postgis_database::parse_multilinestring_geometry()
{
//   cout << "inside postgis_database::parse_multilinestring_geometry()" 
//        << endl;
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

void postgis_database::parse_linestring_geometry(OGRGeometry* OGRGeometry_ptr)
{
//   cout << "inside postgis_database::parse_linestring_geom" << endl;
 
   gis_database::parse_linestring_geometry(OGRGeometry_ptr);

   if (curr_polyline_vertices_ptr->size() < 2) return;
   
   const bool force_display_flag=false;
   const bool single_polyline_per_geode_flag=false;
   const int n_text_messages=1;

   PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->
      generate_new_PolyLine(
         *curr_polyline_vertices_ptr,force_display_flag,
         single_polyline_per_geode_flag,n_text_messages);
   curr_PolyLine_ptr->fill_drawable_geode(
      PolyLinesGroup_ptr->get_geode_ptr(),force_display_flag);

// Save name associated with current polyline within STL vector member
// geometry_names if a database column containing name information has
// been specified:

   if (geom_name.size() > 0 && n_text_messages >= 1)
   {
      string text_label(
         poFeature_ptr->GetFieldAsString(geom_name.c_str()));
//         cout << "text_label = " << text_label << endl;

      polyline curr_polyline(*(curr_PolyLine_ptr->construct_polyline()));
      const double edge_frac=0.5;
      threevector label_posn(curr_polyline.edge_point(edge_frac));

// If necessary, modify label directions so that they can be easily
// read from left to right:

      threevector label_dir(curr_polyline.edge_direction(edge_frac));
      if (label_dir.get(0) < 0) label_dir=-label_dir;

//      cout << "PolyLine_text_size = " << PolyLine_text_size << endl;
      curr_PolyLine_ptr->set_label(text_label,label_posn,label_dir,
                                   PolyLine_text_size);
   } // geom_name.size >= 0 conditional
}

// ---------------------------------------------------------------------
// Member function parse_polygon_geometry

void postgis_database::parse_polygon_geometry(OGRGeometry* OGRGeometry_ptr)
{
//   cout << "inside postgis_database::parse_polygon_geometry()" << endl;

   gis_database::parse_polygon_geometry(OGRGeometry_ptr);

   if (curr_polyline_vertices_ptr->size() > 0 &&
       PolyLinesGroup_ptr != NULL)
   {
      const osg::Vec4 polyline_color=
         colorfunc::get_OSG_color(colorfunc::white);

      bool force_display_flag=false;
      bool single_polyline_per_geode_flag=false;
      PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         *curr_polyline_vertices_ptr,polyline_color,
         force_display_flag,single_polyline_per_geode_flag);
      curr_PolyLine_ptr->fill_drawable_geode(
         PolyLinesGroup_ptr->get_geode_ptr(),force_display_flag);
   } // curr_polyline_vertices_ptr->size() > 0 && PolyLinesGroup_ptr != NULL
}

// ---------------------------------------------------------------------
// Member function parse_worldline_geometry()

void postgis_database::parse_worldline_geometry()
{
//   cout << "inside postgis_database::parse_worldline_geometry()" << endl;
   
   twovector centroid;
   vector<double> areas=gis_database::parse_worldline_geometry(centroid);
   double max_area=areas.back();
   if (max_area < 0.5*NEGATIVEINFINITY) return;

   if (PolyLinesGroup_ptr==NULL)
   {
      cout << 
         "PolyLines_Group_ptr=NULL in postgis_database::parse_worldline_geometry()" << endl;
      return;
   }

   const bool force_display_flag=false;
   const bool single_polyline_per_geode_flag=false;
   for (POLYLINE_VERTICES_MAP::iterator itr=polyline_vertices_map_ptr->
           begin(); itr != polyline_vertices_map_ptr->end(); ++itr)
   {
      curr_polyline_vertices_ptr=itr->second;
      if (curr_polyline_vertices_ptr->size() > 0)
      {
         threevector reference_vertex=Zero_vector;
         PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->
            generate_new_PolyLine(
               reference_vertex,*curr_polyline_vertices_ptr,
               force_display_flag,single_polyline_per_geode_flag);
         curr_PolyLine_ptr->fill_drawable_geode(
            PolyLinesGroup_ptr->get_geode_ptr(),force_display_flag);
      } // curr_polyline_vertices_ptr->size() > 0 conditional
   } // loop over itr iterator
   purge_polyline_vertices_map();

// On 11/30/10, we empirically found that imposing the following check
// on max_area (whose units we don't currently understand) eliminates
// labeling of small islands off of CA's and TX's coasts with state
// names:

   if (max_area < 1) return;
   
   OGRPoint* centroid_ptr=new OGRPoint();
   centroid_ptr->setX(centroid.get(0));
   centroid_ptr->setY(centroid.get(1));
   set_country_name(centroid_ptr,max_area);
   delete centroid_ptr;
}

// ---------------------------------------------------------------------
// Member function set_country_name()

void postgis_database::set_country_name(OGRPoint* centroid_ptr,double max_area)
{
//   cout << "inside postgis_database::set_country_name()" << endl;

// FAKE FAKE:  Tuesday, Nov 30,2010 at 7:59 am

// Major [minor] City names should only be 40 kms [20 kms] above flat
// grid and not 50 as for country names

   const double country_text_displacement=50*1000;	// meters

// Scale text size using power-law based upon square-root of its
// maximum area size:

   double sqrt_max_area=sqrt(max_area);
   double country_text_size=50000*pow(2,log10(sqrt_max_area));
   country_text_size=basic_math::max(20000.0,country_text_size);
   bool text_screen_alignment_axis_flag=false;
//   bool text_screen_alignment_axis_flag=flat_grid_flag;

   string country_name=poFeature_ptr->GetFieldAsString(geom_name.c_str());
//   cout << "country_name = " << country_name << endl;

   parse_worldpoint_geometry(
      centroid_ptr,CountriesGroup_ptr,country_name_color,
      country_text_displacement,country_text_size,
      text_screen_alignment_axis_flag);
}

// ---------------------------------------------------------------------
// Member function parse_worldpoint_geometry

void postgis_database::parse_worldpoint_geometry(
   OGRGeometry* OGRGeometry_ptr,CylindersGroup* CylindersGroup_ptr,
   colorfunc::Color cylinder_color,double text_displacement,double text_size,
   bool text_screen_axis_alignment_flag)
{
//   cout << "inside postgis_database::parse_worldpoint_geometry()" << endl;

   double longitude,latitude;
   threevector curr_point,r_hat;
   if (parse_point_geometry(
      OGRGeometry_ptr,longitude,latitude,curr_point,r_hat))
   {
      string label;
      if (geom_name.size() > 0)
      {
         label=poFeature_ptr->GetFieldAsString(geom_name.c_str());
//         cout << "label = " << label << endl;
      }
      Cylinder* curr_Cylinder_ptr=GISfunc::generate_GIS_Cylinder(
         curr_point,r_hat,CylindersGroup_ptr,cylinder_color,
         text_displacement,text_size,
         text_screen_axis_alignment_flag,label);

      if (label.size() > 0 && !text_screen_axis_alignment_flag 
          && !flat_grid_flag)
      {
         Ellipsoid_model_ptr->align_text_with_cardinal_dirs(
            longitude,latitude,curr_Cylinder_ptr->get_text_ptr(0));
      }
   } // point inside bbox conditional
}
