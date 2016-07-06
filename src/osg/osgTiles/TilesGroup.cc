// ==========================================================================
// TilesGroup class member function definitions
// ==========================================================================
// Last updated on 10/1/11; 10/2/11; 10/14/11
// ==========================================================================

#include "osg/osgGraphicals/AnimationController.h"
#include "image/compositefuncs.h"
#include "geometry/geometry_funcs.h"
#include "astro_geo/geopoint.h"
#include "geometry/polygon.h"
#include "image/raster_parser.h"
#include "video/texture_rectangle.h"
#include "osg/osgTiles/TilesGroup.h"

#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void TilesGroup::allocate_member_objects()
{
   RasterParser_ptr=new raster_parser;
}

void TilesGroup::initialize_member_objects()
{
   new_latlong_points_inside_polygon_flag=true;

   ladar_height_data_flag=false;
   prev_m=prev_n=-1;
   prev_lon=prev_lat=NEGATIVEINFINITY;

   geotif_Ptiles_subdir="xxx";

   avg_LOS_png_files_ready_flag=false;
   DTED_ztwoDarray_ptr=NULL;
   reduced_DTED_ztwoDarray_ptr=NULL;
}

TilesGroup::TilesGroup()
{
   allocate_member_objects();
   initialize_member_objects();
}

TilesGroup::~TilesGroup()
{
   delete RasterParser_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const TilesGroup& tg)
{
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void TilesGroup::set_geotif_subdir(std::string subdir)
{
//   cout << "inside TilesGroup::set_geotif_subdir()" << endl;
   geotif_subdir=subdir;
   geotif_Ztiles_subdir=geotif_subdir+"Ztiles/";
   geotif_Ptiles_subdir=geotif_subdir+"Ptiles/";
}

void TilesGroup::set_geotif_Ztiles_subdir(std::string subdir)
{
//   cout << "inside TilesGroup::set_geotif_Ztiles_subdir()" << endl;
   geotif_Ztiles_subdir=subdir;
}

// ==========================================================================
// Line-of-sight tiling member functions
// ==========================================================================

// Member function individual_latlong_tiles_intercepting_ground_bbox()
// reconstructs the corner geopoints for the input ground bounding
// box.  If the bbox's corners do not lie within the wedge defined by
// input vertex0 and angular interval [theta_min,theta_max], this
// method returns an empty STL vector.  Otherwise, it searches over a
// longitude-latitude grid defined by [min_long,max_long] &
// [min_lat,max_lat] and returns those cells within the output STL
// vector intercepting the ground bbox.

vector<threevector>& 
TilesGroup::individual_latlong_tiles_intercepting_ground_bbox(
   const geopoint& vertex0,double theta_min,double theta_max,
   bounding_box* ground_bbox_ptr)
{
//   cout << "inside TilesGroup::latlong_tiles_intercepting_ground_bbox()" 
//        << endl;

//   cout << "min_long = " << min_long << " max_long = " << max_long << endl;
//   cout << "min_lat = " << min_lat << " max_lat = " << max_lat << endl;
//   cout << "theta_min = " << theta_min*180/PI
//        << " theta_max = " << theta_max*180/PI << endl;

// First save current longitude,latitude points inside polygon into
// STL vector member prev_latlong_points_inside_polygon:

   prev_latlong_points_inside_polygon.clear();
   for (unsigned int i=0; i<latlong_points_inside_polygon.size(); i++)
   {
      prev_latlong_points_inside_polygon.push_back(
         latlong_points_inside_polygon[i]);
   }

   double xmin=ground_bbox_ptr->get_xmin();
   double xmax=ground_bbox_ptr->get_xmax();
   double ymin=ground_bbox_ptr->get_ymin();
   double ymax=ground_bbox_ptr->get_ymax();
   double xnew_min=xmin-0.1*(xmax-xmin);
   double xnew_max=xmax-0.1*(xmax-xmin);
   double ynew_min=ymin-0.1*(ymax-ymin);
   double ynew_max=ymax-0.1*(ymax-ymin);
   xmin=xnew_min;
   xmax=xnew_max;
   ymin=ynew_min;
   ymax=ynew_max;
//   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//   cout << "ymin = " << ymin << " ymax = " << ymax << endl;

   geopoint lower_left_corner(
      northern_hemisphere_flag,specified_UTM_zonenumber,xmin,ymin);
   geopoint lower_right_corner(
      northern_hemisphere_flag,specified_UTM_zonenumber,xmax,ymin);
   geopoint upper_right_corner(
      northern_hemisphere_flag,specified_UTM_zonenumber,xmax,ymax);
   geopoint upper_left_corner(
      northern_hemisphere_flag,specified_UTM_zonenumber,xmin,ymax);
//   cout << "lower_left_corner = " << lower_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;

   latlong_points_inside_polygon.clear();

// Check whether any corner's angle relative to vertex0 location lies
// within the angular range [theta_min,theta_max].  If not, the ground
// bbox is not intercepted by the OBSFRUSTUM.  And no tiles should be
// returned within latlong_points_inside_polygon:

   vector<threevector> corner_posn;
   corner_posn.push_back(lower_left_corner.get_UTM_posn());
   corner_posn.push_back(lower_right_corner.get_UTM_posn());
   corner_posn.push_back(upper_right_corner.get_UTM_posn());
   corner_posn.push_back(upper_left_corner.get_UTM_posn());
   bool bbox_inside_OBSFRUSTUM_flag=false;
   for (unsigned int c=0; c<corner_posn.size(); c++)
   {
      threevector r_hat=(corner_posn[c]-vertex0.get_UTM_posn()).unitvector();
      double curr_theta=atan2(r_hat.get(1),r_hat.get(0));
      curr_theta=basic_math::phase_to_canonical_interval(
         curr_theta,theta_min-PI,theta_min+PI);
//      cout << "curr_theta = " << curr_theta*180/PI << endl;
      if (curr_theta > theta_min && curr_theta < theta_max)
         bbox_inside_OBSFRUSTUM_flag=true;
   }
//   cout << "bbox_inside_OBSFRUSTUM_flag = " 
//        << bbox_inside_OBSFRUSTUM_flag << endl;
   if (!bbox_inside_OBSFRUSTUM_flag) return latlong_points_inside_polygon;

   for (int curr_long=min_long; curr_long <=max_long; curr_long++)
   {
      for (int curr_lat=min_lat; curr_lat <= max_lat; curr_lat++)
      {
         bool include_long_lat_coords_into_list_flag=true;
         if (curr_long+1 < lower_left_corner.get_longitude())
            include_long_lat_coords_into_list_flag=false;
         if (curr_long-1 > upper_right_corner.get_longitude())
            include_long_lat_coords_into_list_flag=false;
         if (curr_lat+1 < lower_left_corner.get_latitude())
            include_long_lat_coords_into_list_flag=false;
         if (curr_lat-1 > upper_right_corner.get_latitude())
            include_long_lat_coords_into_list_flag=false;

         if (include_long_lat_coords_into_list_flag) 
         {
//            cout << "curr_long = " << curr_long 
//                 << " curr_lat = " << curr_lat << " lies in bbox" << endl;
            latlong_points_inside_polygon.push_back(
               threevector(curr_long,curr_lat));
         }
      } // loop over curr_lat index
   } // loop over curr_long index

/*
// Perform brute force search over entries in
// latlong_points_inside_polygon to see whether any are different from
// prev_latlong_points_inside_polygon:

   new_latlong_points_inside_polygon_flag=false;
   if (latlong_points_inside_polygon.size() >
       prev_latlong_points_inside_polygon.size())
   {
      new_latlong_points_inside_polygon_flag=true;
   }
   else
   {
      for (int i=0; i<int(latlong_points_inside_polygon.size()) &&
              !new_latlong_points_inside_polygon_flag; i++)
      {
         threevector curr_longlat(latlong_points_inside_polygon[i]);

         bool curr_longlat_found_in_previous_list_flag=false;
         for (int j=0; j<int(prev_latlong_points_inside_polygon.size()) &&
                 !curr_longlat_found_in_previous_list_flag; j++)
         {
            if (curr_longlat.nearly_equal(
               prev_latlong_points_inside_polygon[j]))
            {
               curr_longlat_found_in_previous_list_flag=true;
            }
         } // loop over index j labeling prev long,lat pnts inside polygon
         if (!curr_longlat_found_in_previous_list_flag) 
            new_latlong_points_inside_polygon_flag=true;
      } // loop over index i labeling current long,lat points inside polygon
   }
//   cout << "new_latlong_points_inside_polygon_flag = "
//        << new_latlong_points_inside_polygon_flag << endl;
*/

   if (latlong_points_inside_polygon.size()==0)
   {
      cout << "Warning in TilesGroup::individual_latlong_tiles_intercepting_ground_bbox()" << endl;
      cout << "latlong_points_inside_polygon.size()==0 !!" << endl;
   }

//   cout << "latlong_points_inside_polygon.size() = "
//        << latlong_points_inside_polygon.size() << endl;

//   cout << "latlong_points_inside_polygon = " << endl;
//   templatefunc::printVector(latlong_points_inside_polygon);
   
   return latlong_points_inside_polygon;
}

// ----------------------------------------------------------------
// Member function individual_latlong_tiles_intercepting_polygon() 

vector<threevector>& 
TilesGroup::individual_latlong_tiles_intercepting_polygon(
   const geopoint& vertex1,const geopoint& vertex2,const geopoint& vertex3,
   const threevector& apex,double max_range)
{
//   cout << "inside TilesGroup::latlong_tiles_intercepting_polygon()" << endl;
//   cout << "max_range = " << max_range << endl;
//   cout << "apex = " << apex << endl;

   vector<threevector> vertices;
   double alt=vertex1.get_altitude();
   vertices.push_back(
      threevector(vertex1.get_longitude(),vertex1.get_latitude(),alt));
   vertices.push_back(
      threevector(vertex2.get_longitude(),vertex2.get_latitude(),alt));
   vertices.push_back(
      threevector(vertex3.get_longitude(),vertex3.get_latitude(),alt));
//   templatefunc::printVector(vertices);

// First save current longitude,latitude points inside polygon into
// STL vector member prev_latlong_points_inside_polygon:

   prev_latlong_points_inside_polygon.clear();
   for (unsigned int i=0; i<latlong_points_inside_polygon.size(); i++)
   {
      prev_latlong_points_inside_polygon.push_back(
         latlong_points_inside_polygon[i]);
   }

   latlong_points_inside_polygon.clear();

//   cout << "min_long = " << min_long << " max_long = " << max_long << endl;
//   cout << "min_lat = " << min_lat << " max_lat = " << max_lat << endl;
   for (int curr_long=min_long; curr_long <=max_long; curr_long++)
   {
      for (int curr_lat=min_lat; curr_lat <= max_lat; curr_lat++)
      {
         vector<threevector> longlat_cell_corner;
         longlat_cell_corner.push_back(threevector(curr_long,curr_lat,alt));
         longlat_cell_corner.push_back(threevector(curr_long+1,curr_lat,alt));
         longlat_cell_corner.push_back(
            threevector(curr_long+1,curr_lat+1,alt));
         longlat_cell_corner.push_back(
            threevector(curr_long,curr_lat+1,alt));

         bool include_long_lat_coords_into_list_flag=false;
         for (int c=0; c<4 && !include_long_lat_coords_into_list_flag; c++)
         {
            threevector curr_corner=longlat_cell_corner[c];
            threevector next_corner=longlat_cell_corner[modulo(c+1,4)];

// We need to subdivide each latlong cell edge into a set of points
// and test whether any of them lie inside the polygon.  If so, we
// must include the latlong cell into the list of cells which the
// polygon intersects:

//            int n_steps=10;
            int n_steps=50;
//            int n_steps=200;
            for (int n=0; n<=n_steps && 
                    !include_long_lat_coords_into_list_flag; n++)
            {
               double frac=double(n)/n_steps;
               threevector curr_point=(1-frac)*curr_corner+frac*next_corner;
               geopoint curr_geopoint(
                  curr_point.get(0),curr_point.get(1),
                  curr_point.get(2),specified_UTM_zonenumber);

// Test whether curr_point lies within max_range of apex.  If so,
// check whether it also lies inside the polygon defined by the input
// vertices.  If so, include the current long-lat coordinates into the
// list:

               double sqrd_range=(curr_geopoint.get_UTM_posn()-apex).
                  sqrd_magnitude();
//               cout << "curr_geopoint = " << curr_geopoint
//                    << " apex = " << apex << endl;

//               double ratio=sqrt(sqrd_range/sqr(max_range));
//               cout << "range = " << sqrt(sqrd_range)
//                    << " max_range = " << max_range
//                    << " ratio = " << ratio << endl;

               if (sqrd_range < sqr(max_range))
               {
                  if (geometry_func::Point_in_Triangle(
                     curr_point,vertices[0],vertices[1],vertices[2]))
                  {
                     include_long_lat_coords_into_list_flag=true;
                  }
               } // range < max_range conditional
            } // loop over index n labeling longlat_cell_edge subdivisions
         } // loop over index c labeling longlat_cell_corners

// On 5/19/09 & 1/27/10, we realized that some OBSFRUSTUM footprints
// may be so small that they lie completely inside some 1 deg x 1 deg
// long-lat cell.  To cover this case, we ensure that the long-lat
// cells containing the 3 input vertices are added to
// latlong_points_inside_polygon:

         if (basic_math::mytruncate(vertex1.get_longitude())==curr_long &&
             basic_math::mytruncate(vertex1.get_latitude())==curr_lat)
         {
            include_long_lat_coords_into_list_flag=true;
         }
                  
         if (basic_math::mytruncate(vertex2.get_longitude())==curr_long &&
             basic_math::mytruncate(vertex2.get_latitude())==curr_lat)
         {
            include_long_lat_coords_into_list_flag=true;
         }

         if (basic_math::mytruncate(vertex3.get_longitude())==curr_long &&
             basic_math::mytruncate(vertex3.get_latitude())==curr_lat)
         {
            include_long_lat_coords_into_list_flag=true;
         }
               
//         cout << " vertex0 = " << vertices[0]
//              << " vertex1 = " << vertices[1]
//              << " vertex2 = " << vertices[2] << endl;

         if (include_long_lat_coords_into_list_flag) 
         {
//            cout << "curr_long = " << curr_long 
//                 << " curr_lat = " << curr_lat << " lies in triangle" << endl;
            latlong_points_inside_polygon.push_back(
               threevector(curr_long,curr_lat));
         }
      } // loop over curr_lat index
   } // loop over curr_long index

//   cout << "latlong_points_inside_polygon.size() = "
//        << latlong_points_inside_polygon.size() << endl;
//   cout << "latlong_points_inside_polygon = " << endl;
//   templatefunc::printVector(latlong_points_inside_polygon);
//   outputfunc::enter_continue_char();

   return latlong_points_inside_polygon;
}

// ----------------------------------------------------------------
// Member function latlong_bbox_corners_intercepting_polygon() takes
// in the STL vector of (longitude,latitude) geocoordinate labels for
// tiles intercepting some polygon (e.g. Viewfrustum z-plane triangle
// or geotiff rectangle).  It returns the lower left and upper right
// corner geopoints for the bounding box which encloses all the input
// tiles.

void TilesGroup::latlong_bbox_corners_intercepting_polygon(
   const vector<threevector>& latlong_points_inside_polygon,
   geopoint& lower_left_corner,geopoint& upper_right_corner)
{
//   cout << "inside TilesGroup::latlong_bbox_corners_intercepting_polygon()"
//        << endl;

   int min_long=1000; // degs
   int max_long=-1000;
   int min_lat=1000;
   int max_lat=-1000;
   
   for (unsigned int i=0; i<latlong_points_inside_polygon.size(); i++)
   {
      int curr_x(latlong_points_inside_polygon[i].get(0));
      int curr_y(latlong_points_inside_polygon[i].get(1));
//      cout << "curr_x = " << curr_x << " curr_y = " << curr_y << endl;
      min_long=basic_math::min(min_long,curr_x);
      max_long=basic_math::max(max_long,curr_x);
      min_lat=basic_math::min(min_lat,curr_y);
      max_lat=basic_math::max(max_lat,curr_y);
   }

   if (min_long > 180 || min_lat > 180 || max_long < -180 || max_lat < -180)
   {
      cout << 
         "Error in TilesGroup::latlong_bbox_corners_intercepting_polygon()"
           << endl;
      cout << "min_long = " << min_long << " max_long = " << max_long
           << endl;
      cout << "min_lat = " << min_lat << " max_lat = " << max_lat << endl;
      exit(-1);
   }
   
   lower_left_corner=geopoint(
      double(min_long),double(min_lat),0.0,specified_UTM_zonenumber);
   upper_right_corner=geopoint(
      double(max_long+1),double(max_lat+1),0.0,specified_UTM_zonenumber);
}

// ----------------------------------------------------------------
// Method generate_subtile_twoDarray() takes in spacing and bbox
// corner information for a tile covering some specified polygon.  It
// generates and returns twoDarray *ztwoDarray_ptr which can store
// DTED height values for this subtile.

twoDarray* TilesGroup::generate_subtile_twoDarray(
   double delta_x,double delta_y,
   geopoint& lower_left_corner,geopoint& upper_right_corner)
{
//   cout << "inside TilesGroup::generate_subtile_twoDarray()" << endl;
//   cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;
//   cout << "lower_left_corner = " << lower_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;
         
   int mdim=(upper_right_corner.get_UTM_easting()-
             lower_left_corner.get_UTM_easting())/delta_x+2;
   int ndim=(upper_right_corner.get_UTM_northing()-
             lower_left_corner.get_UTM_northing())/delta_y+2;
//   cout << "mdim = " << mdim << " ndim " << ndim << endl;
         
   twoDarray* ztwoDarray_ptr=new twoDarray(mdim,ndim);
   ztwoDarray_ptr->init_coord_system(
      lower_left_corner.get_UTM_easting()-delta_x,
      upper_right_corner.get_UTM_easting()+delta_x,
      lower_left_corner.get_UTM_northing()-delta_y,
      upper_right_corner.get_UTM_northing()+delta_y);
   return ztwoDarray_ptr;
}

twoDarray* TilesGroup::generate_subtile_twoDarray(
   double delta_x,double delta_y,
   double x_lo,double x_hi,double y_lo,double y_hi)
{
//   cout << "inside TilesGroup::generate_subtile_twoDarray()" << endl;
//   cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;
//   cout << "x_lo = " << x_lo << " x_hi = " << x_hi << endl;
//   cout << "y_lo = " << y_lo << " y_hi = " << y_hi << endl;
         
   int mdim=(x_hi-x_lo)/delta_x+2;
   int ndim=(y_hi-y_lo)/delta_y+2;
//   cout << "mdim = " << mdim << " ndim " << ndim << endl;
         
   twoDarray* ztwoDarray_ptr=new twoDarray(mdim,ndim);
   ztwoDarray_ptr->init_coord_system(
      x_lo-delta_x,x_hi+delta_x,
      y_lo-delta_y,y_hi+delta_y);
   return ztwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function read_geotif_subtiles_overlapping_polygon() is a
// highly specialized subroutine which we wrote in Mar 2009 in order
// to read in geotif files containing Afghanistan/Pakistan DTED height
// information.  Each geotif file is assumed to be named like
// e063n37.tif.  This method first retrieves all tiles corresponding to
// the input longitude,latitude coordinate pairs within
// latlong_points_inside_polygon.  It then adds their z values into
// input *ztwoDarray_ptr created by method
// generate_subtile_twoDarray() which is large enough to accomodate
// all the input subtiles' height data.

bool TilesGroup::read_geotif_subtiles_overlapping_polygon(
   const vector<threevector>& latlong_points_inside_polygon,
   twoDarray* ztwoDarray_ptr)
{
//   cout << "inside TilesGroup::read_geotif_subtiles_overlapping_polygon()"
//        << endl;

   bool some_data_imported_flag=false;

   ztwoDarray_ptr->initialize_values(NEGATIVEINFINITY);
   for (unsigned int i=0; i<latlong_points_inside_polygon.size(); i++)
   {
      twovector curr_geocoords(latlong_points_inside_polygon[i]);
      int curr_longitude=basic_math::round(curr_geocoords.get(0));
      int curr_latitude=basic_math::round(curr_geocoords.get(1));
//      cout << "curr_longitude = " << curr_longitude << endl;

      string geotif_filename;
      if (curr_longitude < 0)
      {
         geotif_filename=geotif_Ztiles_subdir+
            "w"+stringfunc::integer_to_string(fabs(curr_longitude),3)+
            "n"+stringfunc::integer_to_string(curr_latitude,2)+".tif";
      }
      else
      {
         geotif_filename=geotif_Ztiles_subdir+
            "e"+stringfunc::integer_to_string(curr_longitude,3)+
            "n"+stringfunc::integer_to_string(curr_latitude,2)+".tif";
      }
      cout << "Input geotif subtile filename = " << geotif_filename << endl;

      bool curr_data_OK_flag=
         read_geotif_subtile_height_data(geotif_filename,ztwoDarray_ptr);
      if (curr_data_OK_flag) some_data_imported_flag=true;
   } // loop over index i labeling latlong geotif subtiles
//   outputfunc::enter_continue_char();

   return some_data_imported_flag;
}

// ----------------------------------------------------------------
// Boolean member function read_geotif_subtile_height_data() returns
// false if it cannot read in height data from the requested geotif
// file.  Otherwise, this method imports data from the input geotif
// file.  It then adds the height information into input
// *ztwoDarray_ptr.

bool TilesGroup::read_geotif_subtile_height_data(
   string geotif_filename,twoDarray* ztwoDarray_ptr)
{
//   cout << "inside TilesGroup::read_geotif_subtile_height_data()" << endl;
//   cout << "geotif_filename = " << geotif_filename << endl;
//   cout << "specified UTM zonenumber = "
//        << get_specified_UTM_zonenumber() << endl;

   bool geotif_successfully_opened=
      RasterParser_ptr->open_image_file(geotif_filename);
   if (!geotif_successfully_opened)
   {
      cout << "Error in TilesGroup::read_geotif_subtile_height_data()!"
           << endl;
      cout << "Could not read in " << geotif_filename << endl;
//      outputfunc::enter_continue_char();
      return false;
   }

   const int channel_ID=0;
   RasterParser_ptr->fetch_raster_band(channel_ID);
   twoDarray* subtile_ztwoDarray_ptr=RasterParser_ptr->get_ztwoDarray_ptr();

   RasterParser_ptr->read_raster_data(subtile_ztwoDarray_ptr);
   RasterParser_ptr->close_image_file();

// Loop over subtile's entries and copy them into *ztwoDarray_ptr:

   unsigned int qx,qy;
   double curr_x,curr_y;
   for (unsigned int px=0; px<subtile_ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<subtile_ztwoDarray_ptr->get_ndim(); py++)
      {
         if (subtile_ztwoDarray_ptr->pixel_to_point(px,py,curr_x,curr_y))
         {
            if (ztwoDarray_ptr->point_to_pixel(curr_x,curr_y,qx,qy))
            {

// To avoid overwriting valid DTED data with NULL information, reset
// ztwoDarray pixel's value to the maximum of its current and new
// values:

               double curr_z=subtile_ztwoDarray_ptr->get(px,py);
//               cout << "qx = " << qx << " qy = " << qy 
//                    << " curr_x = " << curr_x << " curr_y = " << curr_y
//                    << " curr_z = " << curr_z << endl;
               ztwoDarray_ptr->put(
                  qx,qy,basic_math::max(curr_z,ztwoDarray_ptr->get(qx,qy)));
            }
         }
      } // loop over py index
   } // loop over px index

   int n_pixels=0;
   int n_bad_pixels=0;
   for (unsigned int qx=0; qx<ztwoDarray_ptr->get_mdim(); qx++)
   {
      for (unsigned int qy=0; qy<ztwoDarray_ptr->get_ndim(); qy++)
      {
         double curr_z=ztwoDarray_ptr->get(qx,qy);
         if (curr_z < 0.1) n_bad_pixels++;
         n_pixels++;
      }
   }

   cout << "n_bad_pixels = " << n_bad_pixels
        << " n_total_pixels = " << n_pixels
        << " bad frac = " << double(n_bad_pixels)/double(n_pixels)
        << endl;

   return true;
}

// ----------------------------------------------------------------
// Member function
// load_DTED_subtiles_overlapping_polygon_into_ztwoDarray() is a
// high-level subroutine which outputs DTED height data into a
// dynamically instantiated twoDarray.  It takes in the
// longitude/latitude bounds of a large DTED map (e.g. for all of
// Afghanistan) as well as the map's specified UTM zonenumber.  It
// also takes in the geocoordinates for a polygon which covers some
// part of this map (e.g. the triangle corresponds to the projection
// of an OBSFRUSTUM onto the ground plane).  This method instantiates
// twoDarray *ztwoDarray_ptr with pixel spacing set by input
// parameters delta_x and delta_y.  It fills *ztwoDarray_ptr using the
// DTED geotif files stored in subdirectory geotif_subdir and returns
// its pointer.

twoDarray*& 
TilesGroup::load_DTED_subtiles_overlapping_polygon_into_ztwoDarray(
   const vector<threevector>& latlong_points_inside_polygon,
   double delta_x,double delta_y)
{
//   cout << "inside TilesGroup::load_DTED_subtiles_overlapping_polygon_into_ztwoDarray()" << endl;
//   cout << "latlong_points_inside_polygon = " << endl;
//   templatefunc::printVector(latlong_points_inside_polygon);

   geopoint lower_left_corner,upper_right_corner;
   latlong_bbox_corners_intercepting_polygon(
      latlong_points_inside_polygon,lower_left_corner,upper_right_corner);
//   cout << "lower_left_corner = " << lower_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;

   if (new_latlong_points_inside_polygon_flag)
   {
//      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//      cout << "Reloading z tiles" << endl;
//      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;  

      delete DTED_ztwoDarray_ptr;
      DTED_ztwoDarray_ptr=generate_subtile_twoDarray(
         delta_x,delta_y,lower_left_corner,upper_right_corner);

      read_geotif_subtiles_overlapping_polygon(
         latlong_points_inside_polygon,DTED_ztwoDarray_ptr);
   }
   
   return DTED_ztwoDarray_ptr;
}

// ---------------------------------------------------------------------
int TilesGroup::get_terrain_reduction_scale_factor() 
{
   terrain_reduction_scale_factor=11;
   if (ladar_height_data_flag)
   {
//      terrain_reduction_scale_factor=1;
      terrain_reduction_scale_factor=3;
   }
   return terrain_reduction_scale_factor;
}

// ---------------------------------------------------------------------
// Member function generate_reduced_DTED_ztwoDarray() subsamples the
// contents of member *DTED_ztwoDarray_ptr and places the results into
// member *reduced_DTED_ztwoDarray_ptr.  This method also destroys any
// previously instantiated *reduced_DTED_ztwoDarray_ptr before
// generating a new one.

twoDarray* TilesGroup::generate_reduced_DTED_ztwoDarray()
{
//   cout << "inside TilesGroup::generate_reduced_DTED_ztwoDarray()" << endl;

   int mdim=DTED_ztwoDarray_ptr->get_mdim();
   int ndim=DTED_ztwoDarray_ptr->get_ndim();
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

//   cout << "Instantiating reduced DTED ztwoDarray:" << endl;

   delete reduced_DTED_ztwoDarray_ptr;
   reduced_DTED_ztwoDarray_ptr=new twoDarray(
      mdim/get_terrain_reduction_scale_factor(),
      ndim/get_terrain_reduction_scale_factor());
   
   int extremal_sentinel=2;
   compositefunc::extremal_subsample_twoDarray(
      DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,extremal_sentinel);

   return reduced_DTED_ztwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function update_avg_LOS_tiles() takes in a set of tiles
// whose lower left corner geocoordinates correspond to integer
// longitude,latitude pairs.  Looping over each tile, this method
// reads in number-of-views normalization and average LOS occlusion
// information from input geotif files if they exist into twoDarrays
// *nviews_ztwoDarray_ptr and *avgLOS_ptwoDarray_ptr.  It then updates
// each pixel's value within these twoDarrays based upon corresponding
// entries within *DTED_ptwoDarray_ptr.  The updated contents of
// *nviews_ztwoDarray_ptr and *avgLOS_ptwoDarray_ptr are written back
// to the geotif files for each tile.

void TilesGroup::update_avg_LOS_tiles(
   double delta_x,double delta_y,
   vector<threevector>& latlong_points_inside_polygon,
   twoDarray* DTED_ptwoDarray_ptr,
   AnimationController* AnimationController_ptr)
{
//    cout << "inside TilesGroup::update_avg_LOS_tiles()" << endl;

   const double nviews_min=-1;
   const double nviews_max=20;
   const double p_min=-1;
   const double p_max=1.01;

   for (unsigned int i=0; i<latlong_points_inside_polygon.size(); i++)
   {
//      cout << "i = " << i
//           << " latlong_points_inside_polygon.size() = "
//           << latlong_points_inside_polygon.size() << endl;

      twovector curr_geocoords(latlong_points_inside_polygon[i]);
      int curr_longitude=basic_math::round(curr_geocoords.get(0));
      int curr_latitude=basic_math::round(curr_geocoords.get(1));

      string nviews_geotif_filename,avg_LOS_filename_prefix;
      if (curr_longitude < 0)
      {
         nviews_geotif_filename=geotif_Ptiles_subdir+
            "nview_w"+stringfunc::number_to_string(fabs(curr_longitude))+
            "n"+stringfunc::number_to_string(curr_latitude)+".tif";
         avg_LOS_filename_prefix=
            "avg_LOS_w"+stringfunc::number_to_string(fabs(curr_longitude))+
            "n"+stringfunc::number_to_string(curr_latitude);
      }
      else
      {
         nviews_geotif_filename=geotif_Ptiles_subdir+
            "nview_e"+stringfunc::number_to_string(curr_longitude)+
            "n"+stringfunc::number_to_string(curr_latitude)+".tif";
         avg_LOS_filename_prefix=
            "avg_LOS_e"+stringfunc::number_to_string(curr_longitude)+
            "n"+stringfunc::number_to_string(curr_latitude);
      }
      string avg_LOS_geotif_filename=
         geotif_Ptiles_subdir+avg_LOS_filename_prefix+".tif";

//      cout << "nviews_geotif_filename = " 
//           << nviews_geotif_filename << endl;
//      cout << "avg_LOS_geotif_filename = "
//           << avg_LOS_geotif_filename << endl;

// Recall that lines of longitude/latitude do NOT correspond to lines
// of northing/easting.  Since we raytrace in UTM coordinates but
// store results in LongLat coords, we need to add a small fudge
// factor so that LOS tiles overlap to form complete mosaics:

      const double delta_angle=0.05;
//      const double delta_lat=0.03;

      geopoint lower_left_corner(
         double(curr_longitude-delta_angle),double(curr_latitude-delta_angle),
         0.0,specified_UTM_zonenumber);
      geopoint upper_right_corner(
         double(curr_longitude+1+delta_angle),
         double(curr_latitude+1+delta_angle),0.0,
         specified_UTM_zonenumber);

//      cout << "lower_left_corner = " << lower_left_corner << endl;
//      cout << "upper_right_corner = " << upper_right_corner << endl;

      raster_parser RasterParser;
      twoDarray* nviews_ztwoDarray_ptr=NULL;
      twoDarray* avgLOS_ptwoDarray_ptr=NULL;

// Check whether a geotif for the current latlong tile holding number
// of views information for each pixel already exists.  If so, fill
// *nviews_ztwoDarray_ptr with the file's contents.  Otherwise,
// instantiate *nviews_ztwoDarray_ptr and initialize its contents to
// zero:

      if (filefunc::fileexist(nviews_geotif_filename))
      {
//         cout << "nviews_geotif_filename = " 
//              << nviews_geotif_filename << " already exists" << endl;
         bool predelete_ztwoDarray_ptr_flag=false;
         RasterParser.open_image_file(nviews_geotif_filename,
                                      predelete_ztwoDarray_ptr_flag);
         int channel_ID=0;
         RasterParser.fetch_raster_band(channel_ID);
         nviews_ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
         RasterParser.read_raster_data(nviews_ztwoDarray_ptr);
         RasterParser.convert_GUInts_to_doubles(
            nviews_min,nviews_max,nviews_ztwoDarray_ptr);
         RasterParser.close_image_file();
      }
      else
      {
         nviews_ztwoDarray_ptr=generate_subtile_twoDarray(
            delta_x,delta_y,lower_left_corner,upper_right_corner);
         nviews_ztwoDarray_ptr->clear_values();
      }

// Check whether a geotif for the current latlong tile average LOS
// occlusion information for each pixel already exists.  If so, fill
// *avgLOS_ptwoDarray_ptr with the file's contents.  Otherwise,
// instantiate *avgLOS_ptwoDarray_ptr and initialize its contents to
// zero:

      if (filefunc::fileexist(avg_LOS_geotif_filename))
      {
//         cout << "avg_LOS_geotif_filename = " 
//              << avg_LOS_geotif_filename << " already exists" << endl;
         bool predelete_ztwoDarray_ptr_flag=false;
         RasterParser.open_image_file(
            avg_LOS_geotif_filename,predelete_ztwoDarray_ptr_flag);
         int channel_ID=0;
         RasterParser.fetch_raster_band(channel_ID);
         avgLOS_ptwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
         RasterParser.read_raster_data(avgLOS_ptwoDarray_ptr);
         RasterParser.convert_GUInts_to_doubles(
            p_min,p_max,avgLOS_ptwoDarray_ptr);
         RasterParser.close_image_file();
      }
      else
      {
         avgLOS_ptwoDarray_ptr=generate_subtile_twoDarray(
            delta_x,delta_y,lower_left_corner,upper_right_corner);
         avgLOS_ptwoDarray_ptr->initialize_values(-1);
      }

// Loop over each pixel in *nviews_ztwoDarray_ptr and increment its
// value if the corresponding entry within *DTED_ptwoDarray_ptr >= 0
// (which indicates the position lies inside the visible OBSFRUSTUM).
// *nviews_ztwoDarray_ptr holds denominator normalization data.  Use
// it to compute weighting fraction f for the current LOS probability.
// Append the weighted probability to the running average LOS
// probability stored in *avgLOS_ptwoDarray_ptr.
            
      unsigned int qx,qy;
      double curr_x,curr_y;
      for (unsigned int px=0; px<nviews_ztwoDarray_ptr->get_mdim(); px++)
      {
         for (unsigned int py=0; py<nviews_ztwoDarray_ptr->get_ndim(); py++)
         {
            nviews_ztwoDarray_ptr->pixel_to_point(px,py,curr_x,curr_y);
                  
            if (DTED_ptwoDarray_ptr->point_to_pixel(curr_x,curr_y,qx,qy))
            {
               double curr_p=DTED_ptwoDarray_ptr->get(qx,qy);
               if (curr_p >= 0)
               {
                  nviews_ztwoDarray_ptr->put(
                     px,py,nviews_ztwoDarray_ptr->get(px,py)+1);
                  double f=1.0/nviews_ztwoDarray_ptr->get(px,py);          

                  double prev_avgLOS_value=avgLOS_ptwoDarray_ptr->get(px,py);
                  if (prev_avgLOS_value < 0) f=1;
                        
                  avgLOS_ptwoDarray_ptr->put(
                     px,py,f*curr_p+(1-f)*prev_avgLOS_value);
               }
            }
         } // loop over py index
      } // loop over px index
            
// Write out updated versions of nviews and average LOS occlusion
// geotif files:

      bool output_floats_flag=false;
//      cout << "lower_left_corner = " << lower_left_corner << endl;
      RasterParser.write_raster_data(
         output_floats_flag,nviews_geotif_filename,
         lower_left_corner.get_UTM_zonenumber(),
         lower_left_corner.get_northern_hemisphere_flag(),
         nviews_ztwoDarray_ptr,nviews_min,nviews_max);

      RasterParser.write_raster_data(
         output_floats_flag,avg_LOS_geotif_filename,
         lower_left_corner.get_UTM_zonenumber(),
         lower_left_corner.get_northern_hemisphere_flag(),
         avgLOS_ptwoDarray_ptr,p_min,p_max);

// Recall that nviews_ztwoDarray_ptr and avgLOS_ptwoDarray_ptr both
// point to ztwoDarrays dynamically instantiated within
// RasterParser::open_image_file().  In order to avoid segmentation
// fault, we do NOT pre-delete ztwoDarray_ptr within this method.  So
// we must delete both pointers here.  To avoid a
// RasterParser::ztwoDarray_ptr from becoming a dangling pointer
// (which will lead to a segmentation fault in RasterParser's
// destructor which explicitly deletes ztwoDarray_ptr), we must reset
// RasterParser::ztwoDarray_ptr to NULL:

      delete nviews_ztwoDarray_ptr;
      delete avgLOS_ptwoDarray_ptr;
      nviews_ztwoDarray_ptr=NULL;
      avgLOS_ptwoDarray_ptr=NULL;

      RasterParser.null_ztwoDarray_ptr();

// In order to generate PNG tiles which can be displayed in Michael
// Yee's Openmaps thin client, we need to convert from UTM to
// longitude-latitude coordinates.  In particular, the rows and
// columns of the output twoDarray must correspond to lines of
// constant latitude and longitude.  As of 3/31/09, use GDALWARP to
// convert from UTM to WGS84 coordiantes:

      string avg_LOS_geotif_long_lat_filename=
         geotif_Ptiles_subdir+avg_LOS_filename_prefix+"_longlat.tif";
//      cout << "avg_LOS_geotif_long_lat_filename = "
//           << avg_LOS_geotif_long_lat_filename << endl;
      string unix_command_str=
         "gdalwarp -t_srs WGS84 "+avg_LOS_geotif_filename+" "
         +avg_LOS_geotif_long_lat_filename;
//      cout << "unix_command_str = " << unix_command_str << endl;
      sysfunc::unix_command(unix_command_str);
      

// On 4/13/10, we discovered to our horror that GDALWARP (at least on
// touchy2) does not always successfully convert
// avg_LOS_geotif_filename into avg_LOS_geotif_long_lat_filename.  So
// we must test for the existence of the latter file.  If it is not
// generated, ignore the rest of this loop's contents...

      int sleep_counter=0;
      const int max_sleep_counter=10;
      if (!filefunc::fileexist(avg_LOS_geotif_long_lat_filename) &&
          sleep_counter < max_sleep_counter)
      {
         cout << "sleep_counter = " << sleep_counter << endl;
         sleep_counter++;
         sleep(1);
//         continue;
      }

      if (sleep_counter==max_sleep_counter)
      {
         cout << "Trouble in TilesGroup::update_avg_LOS_tiles()!" << endl;
         cout << "Gdalwarp unable to generate " 
              << avg_LOS_geotif_long_lat_filename << endl;
         continue;
      }
      
      raster_parser RasterParser_final;
      RasterParser_final.open_image_file(avg_LOS_geotif_filename);
      int channel_ID=0;
      RasterParser_final.fetch_raster_band(channel_ID);

      twoDarray* avgLOS_final_ptwoDarray_ptr=
         RasterParser_final.get_ztwoDarray_ptr();
      RasterParser_final.read_raster_data(avgLOS_final_ptwoDarray_ptr);
      RasterParser_final.convert_GUInts_to_doubles(
         p_min,p_max,avgLOS_final_ptwoDarray_ptr);

      lower_left_corner=geopoint(
         avgLOS_final_ptwoDarray_ptr->get_xlo(),
         avgLOS_final_ptwoDarray_ptr->get_ylo());
//      cout << "lower_left_corner = " << lower_left_corner << endl;
      upper_right_corner=geopoint(
         avgLOS_final_ptwoDarray_ptr->get_xhi(),
         avgLOS_final_ptwoDarray_ptr->get_yhi());
//      cout << "upper_right_corner = " << upper_right_corner << endl;

// Write out PNG version of average LOS occlusion files:

      string avg_LOS_png_filename=
         webapps_outputs_subdir+avg_LOS_filename_prefix+".png";
//      cout << "avg_LOS_png_filename = " << avg_LOS_png_filename << endl;

      string movie_filename="twodarray";
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         movie_filename,AnimationController_ptr);
      texture_rectangle_ptr->initialize_twoDarray_image(
         avgLOS_final_ptwoDarray_ptr);
      const double output_alpha=0.4;
      texture_rectangle_ptr->convert_greyscale_image_to_hue_colored(
         output_alpha);
      texture_rectangle_ptr->write_curr_frame(avg_LOS_png_filename);
      delete texture_rectangle_ptr;

      RasterParser_final.close_image_file();

      avg_LOS_png_filenames.push_back(avg_LOS_png_filename);
      png_lower_left_corners.push_back(lower_left_corner);
      png_upper_right_corners.push_back(upper_right_corner);

   } // loop over index i labeling latlong geotif subtiles

   avg_LOS_png_files_ready_flag=true;
}

// ----------------------------------------------------------------
// Member function purge_tile_files() purges the contents of LOS
// geotiff files left over from the previous running of the
// line-of-sight analysis main program.

void TilesGroup::purge_tile_files()
{
//   cout << "inside TilesGroup::purge_tile_files()" << endl;
//   cout << "geotif_Ptiles_subdir = " << geotif_Ptiles_subdir << endl;
   if (geotif_Ptiles_subdir=="xxx")
   {
      cout << "Error in TilesGroup::purge_tile_files()!" << endl;
      cout << "geotif_Ptiles_subdir not properly set within main program!"
           << endl;
      exit(-1);
   }
   filefunc::purge_files_in_subdir(geotif_Ptiles_subdir);
//   cout << "get_webapps_outputs_subdir() = "
//        << get_webapps_outputs_subdir() << endl;
   filefunc::purge_files_with_suffix_in_subdir(
      get_webapps_outputs_subdir(),"png");
//   outputfunc::enter_continue_char();
}

// ----------------------------------------------------------------
// Member function export_avg_ground_bbox_LOS() first fills an STL
// vector with averaged LOS tile filenames corresponding to the input
// longitude and latitude ranges.  It then calls
// TilesGroup::convert_avg_geotifs_from_greyscale_to_color().

string TilesGroup::export_avg_ground_bbox_LOS(
   int min_lon,int max_lon,int min_lat,int max_lat)
{
//   cout << "inside TilesGroup::export_avg_ground_bbox_LOS()" << endl;
   
   string avg_LOS_filename_prefix;
   vector<string> avg_LOS_tif_filenames;
   for (int lon=min_lon; lon <= max_lon; lon++)
   {
      for (int lat=min_lat; lat <= max_lat; lat++)
      {
         if (lon < 0)
         {
            avg_LOS_filename_prefix=
               "avg_LOS_w"+stringfunc::number_to_string(fabs(lon))+
               "n"+stringfunc::number_to_string(lat);
         }
         else
         {
            avg_LOS_filename_prefix=
               "avg_LOS_e"+stringfunc::number_to_string(lon)+
               "n"+stringfunc::number_to_string(lat);
         }
         string avg_LOS_tif_filename=geotif_Ptiles_subdir+
            avg_LOS_filename_prefix+".tif";
         if (filefunc::fileexist(avg_LOS_tif_filename))
         {
            avg_LOS_tif_filenames.push_back(avg_LOS_tif_filename);
         }
      } // loop over lat index
   } // loop over lon index

   return convert_avg_geotifs_from_greyscale_to_color(avg_LOS_tif_filenames);
}

// ----------------------------------------------------------------
// Member function convert_avg_geotifs_from_greyscale_to_color()
// takes in an STL vector of averaged LOS tile filenames.  It converts
// the visibility geotif files' contents from greyscale to hues
// ranging from red to green.  This method then transforms the full
// raster image's geocoordinates from UTM to lat-lon.  Finally, it
// returns the full pathname for the output, recolored and transformed
// raster image.

string TilesGroup::convert_avg_geotifs_from_greyscale_to_color(
   const vector<string>& avg_LOS_tif_filenames)
{
//   cout << "inside TilesGroup::convert_avg_geotifs_from_greyscale_to_color()" 
//        << endl;
   const double p_min=-1;
   const double p_max=1.01;

   vector<string> colored_avg_LOS_tif_filenames;
   for (unsigned int i=0; i<avg_LOS_tif_filenames.size(); i++)
   {
      raster_parser RasterParser_final;
      RasterParser_final.open_image_file(avg_LOS_tif_filenames[i]);
      int channel_ID=0;
      RasterParser_final.fetch_raster_band(channel_ID);

      twoDarray* avgLOS_final_ptwoDarray_ptr=
         RasterParser_final.get_ztwoDarray_ptr();

      double xlo=avgLOS_final_ptwoDarray_ptr->get_xlo();
      double xhi=avgLOS_final_ptwoDarray_ptr->get_xhi();
      double dx=avgLOS_final_ptwoDarray_ptr->get_deltax();

      double ylo=avgLOS_final_ptwoDarray_ptr->get_ylo();
      double yhi=avgLOS_final_ptwoDarray_ptr->get_yhi();
      double dy=avgLOS_final_ptwoDarray_ptr->get_deltay();

      RasterParser_final.read_raster_data(avgLOS_final_ptwoDarray_ptr);
      RasterParser_final.convert_GUInts_to_doubles(
         p_min,p_max,avgLOS_final_ptwoDarray_ptr);

      string movie_filename="twodarray";
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         movie_filename,NULL);
      texture_rectangle_ptr->initialize_twoDarray_image(
         avgLOS_final_ptwoDarray_ptr);
      const double output_alpha=1.0;
      texture_rectangle_ptr->convert_greyscale_image_to_hue_colored(
         output_alpha);

      RGBA_array rgba_array=texture_rectangle_ptr->get_RGBA_twoDarrays();
      twoDarray* RtwoDarray_ptr=rgba_array.first;
      twoDarray* GtwoDarray_ptr=rgba_array.second;
      twoDarray* BtwoDarray_ptr=rgba_array.third;
      twoDarray* AtwoDarray_ptr=rgba_array.fourth;

      RtwoDarray_ptr->set_xlo(xlo);
      RtwoDarray_ptr->set_xhi(xhi);
      RtwoDarray_ptr->set_deltax(dx);
      RtwoDarray_ptr->set_ylo(ylo);
      RtwoDarray_ptr->set_yhi(yhi);
      RtwoDarray_ptr->set_deltay(dy);

      GtwoDarray_ptr->set_xlo(xlo);
      GtwoDarray_ptr->set_xhi(xhi);
      GtwoDarray_ptr->set_deltax(dx);
      GtwoDarray_ptr->set_ylo(ylo);
      GtwoDarray_ptr->set_yhi(yhi);
      GtwoDarray_ptr->set_deltay(dy);

      BtwoDarray_ptr->set_xlo(xlo);
      BtwoDarray_ptr->set_xhi(xhi);
      BtwoDarray_ptr->set_deltax(dx);
      BtwoDarray_ptr->set_ylo(ylo);
      BtwoDarray_ptr->set_yhi(yhi);
      BtwoDarray_ptr->set_deltay(dy);

      AtwoDarray_ptr->set_xlo(xlo);
      AtwoDarray_ptr->set_xhi(xhi);
      AtwoDarray_ptr->set_deltax(dx);
      AtwoDarray_ptr->set_ylo(ylo);
      AtwoDarray_ptr->set_yhi(yhi);
      AtwoDarray_ptr->set_deltay(dy);

      string dirname=filefunc::getdirname(avg_LOS_tif_filenames[i]);
      string basename=filefunc::getbasename(avg_LOS_tif_filenames[i]);
      string colored_tif_filename=dirname+"colored_"+basename;
      colored_avg_LOS_tif_filenames.push_back(colored_tif_filename);

      RasterParser_final.write_colored_raster_data(
         colored_avg_LOS_tif_filenames.back(),
         specified_UTM_zonenumber,northern_hemisphere_flag,
         RtwoDarray_ptr,GtwoDarray_ptr,BtwoDarray_ptr,AtwoDarray_ptr);
      RasterParser_final.close_image_file();

      delete texture_rectangle_ptr;
   } // loop over index i labeling avg_LOS_tif_filenames

// Apply GDALWARP command to all of the geotif files in order to
// convert from UTM to lat-lon geocoords:

   string colored_avg_LOS_lonlat_tif_filename=
      geotif_Ptiles_subdir+"avg_colored_LOS_bbox_lonlat.tif";
//   cout << "colored_avg_LOS_lonlat_tif_filename = "
//        << colored_avg_LOS_lonlat_tif_filename << endl;

   string unix_command_str="gdalwarp -t_srs WGS84 ";
   for (unsigned int i=0; i<avg_LOS_tif_filenames.size(); i++)
   {
      unix_command_str += colored_avg_LOS_tif_filenames[i]+" ";
   }
   unix_command_str += colored_avg_LOS_lonlat_tif_filename;
//   cout << "unix_command_str = " << unix_command_str << endl;
   sysfunc::unix_command(unix_command_str);

   bool colored_avg_tif_exists_flag=false;
   while (!colored_avg_tif_exists_flag)
   {
      cout << "Waiting for colored_avg_tif = " 
           << colored_avg_LOS_lonlat_tif_filename << endl;
      colored_avg_tif_exists_flag=filefunc::fileexist(
         colored_avg_LOS_lonlat_tif_filename);
   }

// Delete colored average LOS geotif tiles in UTM coordinates (to
// avoid having extra files sitting in geotif_Ptiles_subdir with avg_LOS
// in their names):

   for (unsigned int i=0; i<colored_avg_LOS_tif_filenames.size(); i++)
   {
      string curr_filename=colored_avg_LOS_tif_filenames[i];
      unix_command_str="/bin/rm "+curr_filename;
      sysfunc::unix_command(unix_command_str);
   }

   return colored_avg_LOS_lonlat_tif_filename;
}

// ==========================================================================
// Ladar height data member functions
// ==========================================================================

string TilesGroup::get_ladar_geotif_filename()
{
//   cout << "inside TilesGroup::get_ladar_geotif_filename()" << endl;

/*
// FAKE FAKE:  Monday, Nov 15, 2010 at 8:21 am
//  Hardwire one particular ALIRT data set for alg testing purposes

   string geotif_subdir="/data/DTED/Afghanistan/geotif/";
   string ladar_geotif_filename=geotif_subdir+
      "Ztiles/20101025-130724-1064nm_fused.tif";
*/

// FAKE FAKE:  Sat, Feb 5, 2011 at 3:25 pm
// Hardwire flight facility ALIRT data set for D7 raytracing purposes

   string geotif_subdir=
 "/media/66368D22368CF3F9/ALIRT/HAFB_minimap/geotifs/geotifs_0.5m/bbox_tiles/";
   string ladar_geotif_filename=geotif_subdir+"flightfacility_ladarmap.tif";

/*
// FAKE FAKE:  Fri, Jun 3, 2011 at 7 am
// Hardwire one FOB Blessing tile for pano raytracing purposes

   string geotif_subdir=
      "/data/DTED/FOB_Blessing/geotif/Ztiles/";
   string ladar_geotif_filename=geotif_subdir+"FOB_filtered_ztiles.tif";
*/

//   string ladar_geotif_filename="";
//   cout << "ladar_geotif_filename = " << ladar_geotif_filename << endl;

   return ladar_geotif_filename;
}

// ----------------------------------------------------------------
// Member function load_ladar_height_data_into_ztwoDarray()

twoDarray*& TilesGroup::load_ladar_height_data_into_ztwoDarray()
{
//   cout << "inside TilesGroup::load_ladar_height_data_into_ztwoDarray() #1"
//        << endl;
   return load_ladar_height_data_into_ztwoDarray(
      get_ladar_geotif_filename());
}

// ----------------------------------------------------------------
twoDarray*& TilesGroup::load_ladar_height_data_into_ztwoDarray(
   string ladar_geotif_filename)
{
//   cout << "inside TilesGroup::load_ladar_height_data_into_ztwoDarray() #2" 
//        << endl;
//   cout << "ladar_geotif_filename = " << ladar_geotif_filename << endl;

// As of 5/16/11, we no longer assume input ladar maps are
// sufficiently small to be held fully within DTED_ztwoDarray_ptr.
// But we don't want to waste time reading in a ladar geotif if it is
// already in memory:

   if (ladar_geotif_filename==prev_ladar_geotif_filename)
   {
      return DTED_ztwoDarray_ptr;
   }
   prev_ladar_geotif_filename=ladar_geotif_filename;

   bool geotif_successfully_opened=
      RasterParser_ptr->open_image_file(ladar_geotif_filename);
   if (!geotif_successfully_opened)
   {
      cout << "Error in TilesGroup::read_ladar_geotif_height_data()!"
           << endl;
      cout << "Could not read in " << ladar_geotif_filename << endl;
//      outputfunc::enter_continue_char();
      return DTED_ztwoDarray_ptr;
   }

   const int channel_ID=0;
   RasterParser_ptr->fetch_raster_band(channel_ID);
   twoDarray* subtile_ztwoDarray_ptr=RasterParser_ptr->get_ztwoDarray_ptr();
   RasterParser_ptr->read_raster_data(subtile_ztwoDarray_ptr);
   RasterParser_ptr->close_image_file();

   DTED_ztwoDarray_ptr=subtile_ztwoDarray_ptr;

   return DTED_ztwoDarray_ptr;
}

// ----------------------------------------------------------------
twoDarray*& TilesGroup::load_ladar_height_data_into_ztwoDarray(
   double delta_x,double delta_y)
{
//   cout << "inside TilesGroup::load_ladar_height_data_into_ztwoDarray() #3" 
//        << endl;

   return load_ladar_height_data_into_ztwoDarray(
      get_ladar_geotif_filename(),delta_x,delta_y);
}

// ----------------------------------------------------------------
twoDarray*& TilesGroup::load_ladar_height_data_into_ztwoDarray(
   string ladar_geotif_filename,double delta_x,double delta_y)
{
   cout << "inside TilesGroup::load_ladar_height_data_into_ztwoDarray() #4" 
        << endl;
   cout << "ladar_geotif_filename = " << ladar_geotif_filename << endl;

// As of 2/6/11, we assume that any input ladar map is sufficiently
// small that it can be fully loaded into DTED_ztwoDarray_ptr just
// once:

   if (DTED_ztwoDarray_ptr != NULL) return DTED_ztwoDarray_ptr;

   bool geotif_successfully_opened=
      RasterParser_ptr->open_image_file(ladar_geotif_filename);
   if (!geotif_successfully_opened)
   {
      cout << "Error in TilesGroup::read_ladar_geotif_height_data()!"
           << endl;
      cout << "Could not read in " << ladar_geotif_filename << endl;
//      outputfunc::enter_continue_char();
      return DTED_ztwoDarray_ptr;
   }

   const int channel_ID=0;
   RasterParser_ptr->fetch_raster_band(channel_ID);
   twoDarray* subtile_ztwoDarray_ptr=RasterParser_ptr->get_ztwoDarray_ptr();
   RasterParser_ptr->read_raster_data(subtile_ztwoDarray_ptr);
   RasterParser_ptr->close_image_file();

   delete DTED_ztwoDarray_ptr;

   double xlo=subtile_ztwoDarray_ptr->get_xlo();
   double xhi=subtile_ztwoDarray_ptr->get_xhi();
   double ylo=subtile_ztwoDarray_ptr->get_ylo();
   double yhi=subtile_ztwoDarray_ptr->get_yhi();
   DTED_ztwoDarray_ptr=generate_subtile_twoDarray(
      delta_x,delta_y,xlo,xhi,ylo,yhi);

// Loop over rows & columns of *DTED_ztwoDarray_ptr.  Fill its pixels 
// height values read in from raster image and stored within
// *subtile_ztwoDarray_ptr:

   unsigned int qx,qy;
   double x,y;
   for (unsigned int px=0; px<DTED_ztwoDarray_ptr->get_mdim(); px++)
   {
      DTED_ztwoDarray_ptr->px_to_x(px,x);
      for (unsigned int py=0; py<DTED_ztwoDarray_ptr->get_ndim(); py++)
      {
         DTED_ztwoDarray_ptr->py_to_y(py,y);

         double curr_z=0;
         if (subtile_ztwoDarray_ptr->point_to_pixel(x,y,qx,qy))
         {
            curr_z=subtile_ztwoDarray_ptr->get(qx,qy);
         }
         DTED_ztwoDarray_ptr->put(px,py,curr_z);
         
      } // loop over py index
   } // loop over px index

   return DTED_ztwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function set_ladar_data_bbox() takes in extremal X & Y
// values for an entire set of ladar tiles (e.g. 6x5 FOB Blessing
// chips).  It also takes in the x and y extents for each tile.

void TilesGroup::set_ladar_data_bbox(
   double xmin,double xmax,double ymin,double ymax,double dx,double dy)
{
//   cout << "inside TilesGroup::set_ladar_data_bbox()" << endl;
   
   ladar_data_bbox.set_xy_bounds(xmin,xmax,ymin,ymax);
   ladar_data_bbox.set_physical_deltaX(dx);
   ladar_data_bbox.set_physical_deltaY(dy);
}

// ----------------------------------------------------------------
bool TilesGroup::get_ladar_z_given_xy(double x,double y,double& z)
{
   if (geotif_subdir=="/data/DTED/FOB_Blessing/geotif/")
   {
      return get_FOB_Blessing_ladar_z_given_xy(x,y,z);
   }

   if (DTED_ztwoDarray_ptr->point_inside_working_region(x,y))
   {
      z=DTED_ztwoDarray_ptr->fast_XY_to_Z(x,y);
//   cout << "z = " << z << endl;
      return true;
   }
   else
   {
      return false;
   }
}

// ----------------------------------------------------------------
// Member function get_FOB_Blessing_ladar_z_given_xy() first computes integer
// indices m and n for the ladar tile containing the input (x,y) pair.
// It loads the tile corresponding to (m,n) into *DTED_ztwoDarray_ptr
// if it is not already in memory.  This method then returns
// z = DTED_ztwoDarray_ptr(x,y).  We wrote this highly specialized
// method for the TOC11 red actor path network problem.

bool TilesGroup::get_FOB_Blessing_ladar_z_given_xy(double x,double y,double& z)
{
//   cout << "inside TilesGroup::get_FOB_Blessing_ladar_z_given_xy()" << endl;
   double xfrac=
      (x-ladar_data_bbox.get_xmin())/ladar_data_bbox.get_physical_deltaX();
   double yfrac=
      (y-ladar_data_bbox.get_ymin())/ladar_data_bbox.get_physical_deltaY();
   int m=basic_math::mytruncate(xfrac);
   int n=basic_math::mytruncate(yfrac);

//   if (m < 0 || n < 0)
//   {
//      cout << "inside TilesGroup::get_FOB_Blessing_ladar_z_given_xy()" << endl;
//      cout << "x = " << x << " y = " << y << endl;
//      cout << "xfrac = " << xfrac << " yfrac = " << yfrac << endl;
//      cout << "m = " << m << " n = " << n << endl;
//      cout << "ladar_data_bbox = " << ladar_data_bbox << endl;
//   }

// Check whether new FOB Blessing height tile needs to be loaded into
// memory:

   if (m != prev_m || n != prev_n)
   {

// As of 5/16/11, we hardwire subdir and filenames for FOB Blessing
// ladar tiles:

      string ladar_tile_subdir=
         "/data_second_disk/DTED/FOB_Blessing/geotif/Ztiles/";
      string ladar_tile_filename=ladar_tile_subdir+
         "filtered_tile_"+stringfunc::number_to_string(m)+"_"+
         stringfunc::number_to_string(n)+".tif";
//   cout << "x = " << x << " y = " << y 
//        << " ladar_tile_filename = " << ladar_tile_filename << endl;

      DTED_ztwoDarray_ptr=load_ladar_height_data_into_ztwoDarray(
         ladar_tile_filename);

      prev_m=m;
      prev_n=n;
   }

   if (DTED_ztwoDarray_ptr->point_inside_working_region(x,y))
   {
      z=DTED_ztwoDarray_ptr->fast_XY_to_Z(x,y);
//   cout << "z = " << z << endl;
      return true;
   }
   else
   {
      return false;
   }
}

// ==========================================================================
// SRTM height data member functions
// ==========================================================================

// Member function get_SRTM_nadir_z_given_lon_lat() first checks if
// the nine SRTM tiles nearest to the input lon,lat geocoordinates
// are already stored in memory.  If not, the 3x3 tiles are loaded.  
// This method next computes the easting,northing coordinates
// corresponding to the currently specified UTM zonenumber.  If the
// easting,northing pair overlaps the 3x3 SRTM tiles in memory, this
// boolean method returns true along with the nadir z value.  

bool TilesGroup::get_SRTM_nadir_z_given_lon_lat(
   double lon,double lat,double& z)
{
//   cout << "inside TilesGroup::get_SRTM_nadir_z_given_lon_lat()" << endl;

// Check whether new SRTM height tiles need to be loaded into memory:

   if ( fabs(lon-prev_lon) > 0.99 || fabs(lat-prev_lat) > 0.99)
   {
      double delta_x=100;	// meters
      double delta_y=100;	// meters
      bool some_data_imported_flag;
      DTED_ztwoDarray_ptr=load_nine_SRTM_tiles_into_ztwoDarray(
         lon,lat,delta_x,delta_y,some_data_imported_flag);
      prev_lon=lon;
      prev_lat=lat;
   }

   geopoint curr_point(lon,lat,0.0,specified_UTM_zonenumber);
   double easting=curr_point.get_UTM_easting();
   double northing=curr_point.get_UTM_northing();
//   cout << "easting = " << easting << " northing = " << northing << endl;
//   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;
   if (DTED_ztwoDarray_ptr->point_inside_working_region(easting,northing))
   {
      z=DTED_ztwoDarray_ptr->fast_XY_to_Z(easting,northing);
//      cout << "z = " << z << endl;
      return true;
   }
   else
   {
      return false;
   }
}

// ----------------------------------------------------------------
// Member function load_nine_SRTM_tiles_into_ztwoDarray() computes the
// extremal easting and northing extents of a 1 deg x 1 deg lat-long
// DTED tile.  It then instantiates twoDarray *DTED_ztwoDarray_ptr
// with spacing set by input delta_x,y parameters.  This method reads
// DTED data from an input geotif file into *DTED_ztwoDarray_ptr and
// returns the twoDarray.

twoDarray*& TilesGroup::load_nine_SRTM_tiles_into_ztwoDarray(
   double longitude,double latitude,double delta_x,double delta_y,
   bool& some_data_imported_flag)
{
//   cout << "inside TilesGroup::load_nine_SRTM_tiles_into_ztwoDarray()" 
//        << endl;
//   cout << "specified_UTM_zonenumber = " << specified_UTM_zonenumber << endl;
//   cout << "input longitude = " << longitude
//        << " latitude = " << latitude << endl;
   
   geopoint lower_left_corner(
      longitude-1,latitude-1,0.0,specified_UTM_zonenumber);
   geopoint lower_right_corner(
      longitude+2,latitude-1,0.0,specified_UTM_zonenumber);
   geopoint upper_right_corner(
      longitude+2,latitude+2,0.0,specified_UTM_zonenumber);
   geopoint upper_left_corner(
      longitude-1,latitude+2,0.0,specified_UTM_zonenumber);

   double easting_lo=basic_math::min(lower_left_corner.get_UTM_easting(),
                         upper_left_corner.get_UTM_easting());
   double easting_hi=basic_math::max(lower_right_corner.get_UTM_easting(),
                         upper_right_corner.get_UTM_easting());
   double northing_lo=basic_math::min(lower_left_corner.get_UTM_northing(),
                          lower_right_corner.get_UTM_northing());
   double northing_hi=basic_math::max(upper_left_corner.get_UTM_northing(),
                          upper_right_corner.get_UTM_northing());
   cout << "easting_lo=" << easting_lo
        << " easting_hi = " << easting_hi << endl;
   cout << "northing_lo = " << northing_lo
        << " northing_hi = " << northing_hi << endl;

   delete DTED_ztwoDarray_ptr;
   DTED_ztwoDarray_ptr=generate_subtile_twoDarray(
      delta_x,delta_y,easting_lo,easting_hi,northing_lo,northing_hi);

   vector<threevector> latlong_points;
   latlong_points.push_back(threevector(longitude-1,latitude-1,0));
   latlong_points.push_back(threevector(longitude,latitude-1,0));
   latlong_points.push_back(threevector(longitude+1,latitude-1,0));
   latlong_points.push_back(threevector(longitude-1,latitude,0));
   latlong_points.push_back(threevector(longitude,latitude,0));
   latlong_points.push_back(threevector(longitude+1,latitude,0));
   latlong_points.push_back(threevector(longitude-1,latitude+1,0));
   latlong_points.push_back(threevector(longitude,latitude+1,0));
   latlong_points.push_back(threevector(longitude+1,latitude+1,0));
   some_data_imported_flag=read_geotif_subtiles_overlapping_polygon(
      latlong_points,DTED_ztwoDarray_ptr);
//   outputfunc::enter_continue_char();

   return DTED_ztwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function estimate_SRTM_z_given_aerial_pt_and_ray() takes in
// some aerial point along with pointing direction r_hat.  It
// iteratively refines the estimate for Z_ground at the ground
// intercept location for the ray from the aerial point.

bool TilesGroup::estimate_SRTM_z_given_aerial_pt_and_ray(
   const geopoint& aerial_point,const threevector& r_hat,double& Z_ground)
{
//   cout << "inside TilesGroup::estimate_SRTM_z_given_aerial_pt_and_ray()"
//        << endl;

   geopoint ground_intercept;
   bool flag=estimate_SRTM_ground_intercept_given_aerial_pt_and_ray(
      aerial_point,r_hat,ground_intercept);
   Z_ground=ground_intercept.get_altitude();
//   cout << "Z_ground = " << Z_ground << endl;
   return flag;
}

// ----------------------------------------------------------------
bool TilesGroup::estimate_SRTM_ground_intercept_given_aerial_pt_and_ray(
   const geopoint& aerial_point,const threevector& r_hat,
   geopoint& ground_intercept_point)
{
//   cout << "inside TilesGroup::estimate_SRTM_ground_intercept_given_aerial_pt_and_ray()"
//        << endl;

   if (r_hat.get(2) >= 0) return false;

   double z_nadir;
   bool nadir_z_flag=get_SRTM_nadir_z_given_lon_lat(
      aerial_point.get_longitude(),aerial_point.get_latitude(),z_nadir);
   if (!nadir_z_flag) return false;

   double z_aerial=aerial_point.get_altitude();
   threevector aerial_posn=aerial_point.get_UTM_posn();

   double lambda=(z_nadir-z_aerial)/r_hat.get(2);
   threevector ground_intercept_0=aerial_posn+lambda*r_hat;
   double range_0=(ground_intercept_0-aerial_posn).magnitude();
   
   const double max_range=50*1000;	// meters
   if (range_0 > max_range) return false;

   ground_intercept_point=geopoint(
      aerial_point.get_northern_hemisphere_flag(),
      aerial_point.get_UTM_zonenumber(),
      ground_intercept_0.get(0),ground_intercept_0.get(1),
      aerial_point.get_altitude());
   double z1;
   bool z1_flag=get_SRTM_nadir_z_given_lon_lat(
      ground_intercept_point.get_longitude(),
      ground_intercept_point.get_latitude(),z1);

   if (!z1_flag) return false;

   lambda=(z1-z_aerial)/r_hat.get(2);

   threevector ground_intercept_1=aerial_posn+lambda*r_hat;
   double range_1=(ground_intercept_1-aerial_posn).magnitude();
   if (range_1 > max_range) return false;

   ground_intercept_point=geopoint(
      aerial_point.get_northern_hemisphere_flag(),
      aerial_point.get_UTM_zonenumber(),
      ground_intercept_1.get(0),ground_intercept_1.get(1),z1);
   double z2;
   bool z2_flag=get_SRTM_nadir_z_given_lon_lat(
      ground_intercept_point.get_longitude(),
      ground_intercept_point.get_latitude(),z2);

   if (!z2_flag) return false;

   lambda=(z2-z_aerial)/r_hat.get(2);
   threevector ground_intercept_2=aerial_posn+lambda*r_hat;
//   cout << "ground_intercept_2 = " << ground_intercept_2 << endl;
   ground_intercept_point=geopoint(
      aerial_point.get_northern_hemisphere_flag(),
      aerial_point.get_UTM_zonenumber(),
      ground_intercept_2.get(0),ground_intercept_2.get(1),z2);

   return true;
}

// ==========================================================================
// Skymap generation member functions
// ==========================================================================

// Member function load_all_DTED_tiles() first computes the extremal X
// and Y coordinates from the input ground target_posns.  It then
// extracts the lower left and upper right geocoordinates for a
// bounding box surrounding the ground targets by at least
// n_extra_degs degrees in longitude and latitude.
// *DTED_ztwoDarray_ptr is instantiated to hold the entire bounding
// box's worth of height data which are read in from geotif files.
// This method returns DTED_ztwoDarray_ptr.

// We wrote this member function in Sept 2009 in order to
// significantly speed up raytracing of individual ground targets for
// skymap generation purpose.  This method enables reading in terrain
// height data just once for an entire skymap computation.

twoDarray*& TilesGroup::load_all_DTED_tiles(
   int n_extra_degs,double raytrace_cellsize,
   const vector<twovector>& target_posns)
{
//   cout << "inside TilesGroup::load_all_DTED_tiles()" << endl;

   double xmin=POSITIVEINFINITY;
   double xmax=NEGATIVEINFINITY;
   double ymin=POSITIVEINFINITY;
   double ymax=NEGATIVEINFINITY;

//   cout << "target_posns.size() = " << target_posns.size() << endl;
   for (unsigned int t=0; t<target_posns.size(); t++)
   {
      xmin=basic_math::min(xmin,target_posns[t].get(0));
      xmax=basic_math::max(xmax,target_posns[t].get(0));
      ymin=basic_math::min(ymin,target_posns[t].get(1));
      ymax=basic_math::max(ymax,target_posns[t].get(1));
   }
//   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//   cout << "ymin = " << ymin << " ymax = " << ymax << endl;

   geopoint lower_left_corner(
      northern_hemisphere_flag,specified_UTM_zonenumber,xmin,ymin);
   geopoint upper_right_corner(
      northern_hemisphere_flag,specified_UTM_zonenumber,xmax,ymax);
//   cout << "lower_left_corner = " << lower_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;

   return load_all_DTED_tiles(
      n_extra_degs,raytrace_cellsize,lower_left_corner,upper_right_corner);
}

// ----------------------------------------------------------------
twoDarray*& TilesGroup::load_all_DTED_tiles(
   int n_extra_degs,double raytrace_cellsize,
   const geopoint& lower_left_corner,const geopoint& upper_right_corner)
{
//   cout << "inside TilesGroup::load_all_DTED_tiles()" << endl;

   int min_longitude=lower_left_corner.get_long_degs()-n_extra_degs;
   if (lower_left_corner.get_long_degs() < 0) min_longitude--;
   int max_longitude=upper_right_corner.get_long_degs()+n_extra_degs;
   if (upper_right_corner.get_long_degs() < 0) max_longitude--;

   int min_latitude=lower_left_corner.get_lat_degs()-n_extra_degs;
   int max_latitude=upper_right_corner.get_lat_degs()+n_extra_degs;

   geopoint quantized_lower_left_corner(
      double(min_longitude),double(min_latitude),0.0,
      specified_UTM_zonenumber);
   geopoint quantized_upper_right_corner(
      double(max_longitude+1),double(max_latitude+1),0.0,
      specified_UTM_zonenumber);

//   cout << "min_longitude = " << min_longitude
//        << " max_longitude = " << max_longitude << endl;
//   cout << "min_latitude = " << min_latitude
//        << " max_latitude = " << max_latitude << endl;

   double delta_x=raytrace_cellsize;
   double delta_y=raytrace_cellsize;
   delete DTED_ztwoDarray_ptr;
   DTED_ztwoDarray_ptr=generate_subtile_twoDarray(
      delta_x,delta_y,quantized_lower_left_corner,
      quantized_upper_right_corner);

   vector<threevector> latlong_points_inside_bbox;
   for (int longitude=min_longitude; longitude <= max_longitude; longitude++)
   {
      for (int latitude=min_latitude; latitude <= max_latitude; latitude++)
      {
         latlong_points_inside_bbox.push_back(
            threevector(longitude,latitude));
      }
   }
   
   read_geotif_subtiles_overlapping_polygon(
      latlong_points_inside_bbox,DTED_ztwoDarray_ptr);
   
//   cout << "Finished loading DTED tiles" << endl;
   return DTED_ztwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function load_single_DTED_tile_into_ztwoDarray() computes the
// extremal easting and northing extents of a 1 deg x 1 deg lat-long
// DTED tile.  It then instantiates twoDarray *DTED_ztwoDarray_ptr
// with spacing set by input delta_x,y parameters.  This method reads
// DTED data from an input geotif file into *DTED_ztwoDarray_ptr and
// returns the twoDarray.

twoDarray*& TilesGroup::load_single_DTED_tile_into_ztwoDarray(
   double longitude,double latitude,double delta_x,double delta_y,
   bool& some_data_imported_flag)
{
   cout << "inside TilesGroup::load_single_DTED_tile_into_ztwoDarray()" 
        << endl;
   cout << "specified_UTM_zonenumber = "
        << specified_UTM_zonenumber << endl;
   cout << "input longitude = " << longitude
        << " latitude = " << latitude << endl;
   
   geopoint lower_left_corner(
      longitude,latitude,0.0,specified_UTM_zonenumber);
   geopoint lower_right_corner(
      longitude+1,latitude,0.0,specified_UTM_zonenumber);
   geopoint upper_right_corner(
      longitude+1,latitude+1,0.0,specified_UTM_zonenumber);
   geopoint upper_left_corner(
      longitude,latitude+1,0.0,specified_UTM_zonenumber);

   double easting_lo=basic_math::min(lower_left_corner.get_UTM_easting(),
                         upper_left_corner.get_UTM_easting());
   double easting_hi=basic_math::max(lower_right_corner.get_UTM_easting(),
                         upper_right_corner.get_UTM_easting());
   double northing_lo=basic_math::min(lower_left_corner.get_UTM_northing(),
                          lower_right_corner.get_UTM_northing());
   double northing_hi=basic_math::max(upper_left_corner.get_UTM_northing(),
                          upper_right_corner.get_UTM_northing());
   cout << "easting_lo=" << easting_lo
        << " easting_hi = " << easting_hi << endl;
   cout << "northing_lo = " << northing_lo
        << " northing_hi = " << northing_hi << endl;

   delete DTED_ztwoDarray_ptr;
   DTED_ztwoDarray_ptr=generate_subtile_twoDarray(
      delta_x,delta_y,easting_lo,easting_hi,northing_lo,northing_hi);

   vector<threevector> latlong_points;
   latlong_points.push_back(threevector(longitude,latitude,0));

   some_data_imported_flag=read_geotif_subtiles_overlapping_polygon(
      latlong_points,DTED_ztwoDarray_ptr);
//   outputfunc::enter_continue_char();

   return DTED_ztwoDarray_ptr;
}
