// ==========================================================================
// TextureSector class member function definitions
// ==========================================================================
// Last updated on 1/30/10; 2/3/10; 8/5/13
// ==========================================================================

#include <iostream>
#include <iterator>
#include <osgDB/ReadFile>
#include <osg/TexMat>
#include <osg/TextureRectangle>
#include "math/constant_vectors.h"
#include "osg/osgEarth/Earth.h"
#include "astro_geo/Ellipsoid_model.h"
#include "astro_geo/geopoint.h"
#include "osg/osgEarth/TextureSector.h"
#include "math/twovector.h"


using std::cout;
using std::endl;
using std::iterator;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TextureSector::allocate_member_objects()
{
}		       

void TextureSector::initialize_member_objects()
{
   Graphical_name="TextureSector";
   center_altitude=0;
   alpha_blending_value=1.0;
   Ellipsoid_model_ptr=NULL;
   lower_left_corner_ptr=NULL;
   upper_right_corner_ptr=NULL;
   texture_rectangle_ptr=NULL;

   longitude_lo=longitude_hi=latitude_lo=latitude_hi=NEGATIVEINFINITY;
   easting_lo=easting_hi=northing_lo=northing_hi=NEGATIVEINFINITY;
}

TextureSector::TextureSector(int id,Earth* Earth_ptr):
   Geometrical(3,id)
{	
   allocate_member_objects();
   initialize_member_objects();

   if (Earth_ptr != NULL)
   {
      Ellipsoid_model_ptr=Earth_ptr->get_Ellipsoid_model_ptr();
   }
   
}		       

TextureSector::~TextureSector()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const TextureSector& T)
{
//   outstream << "inside TextureSector::operator<<" << endl;
   outstream << "longitude_lo = " << T.longitude_lo 
             << " longitude_hi = " << T.longitude_hi << endl;
   outstream << "latitude_lo = " << T.latitude_lo
             << " latitude_hi = " << T.latitude_hi << endl;
   outstream << "center_altitude = " << T.center_altitude << endl;
   if (T.lower_left_corner_ptr != NULL)
   {
      outstream << "lower_left_geocorner = " << *T.lower_left_corner_ptr 
                << endl;
   }
   outstream << "alpha_blending_value = " << T.alpha_blending_value << endl;
   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

void TextureSector::set_sector_texture_coords(
   osg::Geometry* sector_geometry_ptr)
{
   osg::Vec2Array* texcoords = new osg::Vec2Array(4);

   double lo=0.0;
   double hi=1-lo;

   (*texcoords)[0].set(lo,lo); // texture coord for vertex 0
   (*texcoords)[1].set(hi,lo); // texture coord for vertex 1 
   (*texcoords)[2].set(hi,hi); // texture coord for vertex 2
   (*texcoords)[3].set(lo,hi); // texture coord for vertex 3

   sector_geometry_ptr->setTexCoordArray(0,texcoords);
}

// ---------------------------------------------------------------------
// Member function set_average_posn takes in a time and pass number.
// It translates the TextureSector so that its graphical's location
// coincides with the average corner position.

void TextureSector::set_average_posn(double curr_t,int pass_number)
{
//   cout << "inside TextureSector::set_average_posn(), t = " << curr_t
//        <<  " pass_number = " << pass_number << endl;

//   cout << "corners_avg_location = "
//        << corners_average_location << endl;

   set_UVW_coords(curr_t,pass_number,corners_average_location);

//   threevector UVW;
//   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;
}

// ---------------------------------------------------------------------
void TextureSector::pop_off_Movie_ptr(Movie* m_ptr)
{
//   cout << "inside TextureSector::pop_off_Movie_ptr()" << endl;
   for (std::vector<Movie*>::iterator iter=Movie_ptrs.begin();
        iter != Movie_ptrs.end(); iter++)
   {
      if (*iter==m_ptr)
      {
         Movie_ptrs.erase(iter);
         break;
      }
   }
}

// ==========================================================================
// Texturing methods
// ==========================================================================

void TextureSector::construct_easting_northing_bbox(
   bool northern_hemisphere_flag,int UTM_zonenumber,
   double east_lo,double east_hi,double north_lo,double north_hi,
   double texture_altitude)
{
//   cout << "inside TextureSector::construct_easting_northing_bbox()" << endl;
//   cout << "northern_flag = " << northern_hemisphere_flag
//        << " UTM_zonenumber = " << UTM_zonenumber << endl;

   easting_lo=east_lo;
   easting_hi=east_hi;
   northing_lo=north_lo;
   northing_hi=north_hi;
   center_altitude=texture_altitude;

//   cout << "east_lo = " << easting_lo
//        << " east_hi = " << easting_hi << endl;
//   cout << "north_lo = " << northing_lo
//        << " north_hi = " << northing_hi << endl;

   lower_left_corner_ptr=new geopoint(
      northern_hemisphere_flag,UTM_zonenumber,
      easting_lo,northing_lo,center_altitude);
   upper_right_corner_ptr=new geopoint(
      northern_hemisphere_flag,UTM_zonenumber,
      easting_hi,northing_hi,center_altitude);

   longitude_lo=lower_left_corner_ptr->get_longitude();
   longitude_hi=upper_right_corner_ptr->get_longitude();
   latitude_lo=lower_left_corner_ptr->get_latitude();
   latitude_hi=upper_right_corner_ptr->get_latitude();

//   cout << "lon_lo = " << longitude_lo << endl;
//   cout << "lon_hi = " << longitude_hi << endl;
//   cout << "lat_lo = " << latitude_lo << endl;
//   cout << "lat_hi = " << latitude_hi << endl;
   
//   cout << "easting_lo = " << easting_lo
//        << " easting_hi = " << easting_hi << endl;
//   cout << "northing_lo = " << northing_lo
//        << " northing_hi = " << northing_hi << endl;
}

void TextureSector::construct_long_lat_bbox(
   double long_lo,double long_hi,double lat_lo,double lat_hi,
   double texture_altitude)
{
   cout << "inside TextureSector::construct_long_lat_bbox()" << endl;
   
   longitude_lo=long_lo;
   longitude_hi=long_hi;
   latitude_lo=lat_lo;
   latitude_hi=lat_hi;
   center_altitude=texture_altitude;
   lower_left_corner_ptr=new geopoint(
      longitude_lo,latitude_lo,center_altitude);
   upper_right_corner_ptr=new geopoint(
      longitude_hi,latitude_hi,center_altitude);
}

// ---------------------------------------------------------------------
osg::Geode* TextureSector::generate_drawable_geode(string image_filename)
{
//   cout << "inside TextureSector::generate_drawable_geode()" << endl;
//   cout << "image_filename = " << image_filename << endl;
   
   osg::Geode* geode_ptr=new osg::Geode();
   osg::Geometry* geometry_ptr=construct_surface_sector_geometry();
   set_sector_texture_coords(geometry_ptr);
   geode_ptr->addDrawable(geometry_ptr);
   
   osg::TextureRectangle* texture_ptr = new osg::TextureRectangle;
   osg::TexMat* texmat = new osg::TexMat;
   texmat->setScaleByTextureRectangleSize(true);

   osg::StateSet* stateset_ptr = geode_ptr->getOrCreateStateSet();
   stateset_ptr->setTextureAttributeAndModes(
      0, texture_ptr, osg::StateAttribute::ON);
   stateset_ptr->setTextureAttributeAndModes(
      0, texmat, osg::StateAttribute::ON);

// Enable alpha blending:

   stateset_ptr->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   stateset_ptr->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

   osg::Image* image_ptr = osgDB::readImageFile(image_filename);
   if (!image_ptr)
   {
      cout << "Couldn't find texture image" << endl;
      return NULL;
   }
   texture_ptr->setImage(image_ptr);

   return geode_ptr;
}

// ---------------------------------------------------------------------
osg::Geode* TextureSector::generate_drawable_geode(Movie* Movie_ptr)
{
//   cout << "inside TextureSector::generate_drawable_geode()" << endl;
   osg::Geode* geode_ptr=new osg::Geode();
   osg::Geometry* geometry_ptr=construct_surface_sector_geometry();
   set_sector_texture_coords(geometry_ptr);
   geode_ptr->addDrawable(geometry_ptr);

   osg::TextureRectangle* texture_ptr=Movie_ptr->get_texture_rectangle_ptr()->
      get_TextureRectangle_ptr();
   
//   osg::TextureRectangle* texture_ptr = new osg::TextureRectangle;

   osg::TexMat* texmat = new osg::TexMat;
   texmat->setScaleByTextureRectangleSize(true);

   osg::StateSet* stateset_ptr = geode_ptr->getOrCreateStateSet();
   stateset_ptr->setTextureAttributeAndModes(
      0, texture_ptr, osg::StateAttribute::ON);
   stateset_ptr->setTextureAttributeAndModes(
      0, texmat, osg::StateAttribute::ON);

// Enable alpha blending:

   stateset_ptr->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
   stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   stateset_ptr->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

/*
   osg::Image* image_ptr = osgDB::readImageFile(image_filename);
   if (!image_ptr)
   {
      cout << "Couldn't find texture image" << endl;
      return NULL;
   }
   texture_ptr->setImage(image_ptr);
*/

   return geode_ptr;
}

// ---------------------------------------------------------------------
osg::Geometry* TextureSector::construct_surface_sector_geometry()
{
//   cout << "inside TextureSector::construct_surface_sector_geom()" << endl;

   osg::Geometry* sector_geometry_ptr = new osg::Geometry();
                            
// Initially compute corner locations with zero altitude.  Center of
// image will then be covered by ellipsoid's surface:

   vector<threevector> corner;

   compute_corner_XYZs(center_altitude,corner);

// Raise corner locations away from ellipsoid's surface by a distance
// needed to guarantee center's location lies above ellipsoid:

   if (Ellipsoid_model_ptr != NULL)
   {
      double corner_altitude=-center_altitude+5;
      compute_corner_XYZs(corner_altitude,corner);
   }

// Store relative positions of the TextureSector's corners relative to
// their average location to avoid floating point problems:

   osg::Vec3Array* relative_cornerVertices=new osg::Vec3Array;
   for (int i=0; i<int(corner.size()); i++)
   {
      threevector curr_relative_corner=corner[i]-corners_average_location;
      relative_cornerVertices->push_back(
         osg::Vec3(curr_relative_corner.get(0),
                   curr_relative_corner.get(1),
                   curr_relative_corner.get(2)));
   }
   sector_geometry_ptr->setVertexArray(relative_cornerVertices);

   osg::Vec4Array* colors = new osg::Vec4Array;
   colors->push_back(osg::Vec4(1,1,1,alpha_blending_value));

   sector_geometry_ptr->setColorArray(colors);
   sector_geometry_ptr->setColorBinding(osg::Geometry::BIND_OVERALL);

   osg::DrawElementsUInt* sector=new osg::DrawElementsUInt(
      osg::PrimitiveSet::QUADS,0);
   sector->push_back(0);
   sector->push_back(1);
   sector->push_back(2);
   sector->push_back(3);
   sector_geometry_ptr->addPrimitiveSet(sector);

   return sector_geometry_ptr;
}

// ---------------------------------------------------------------------
// Member function compute_corner_XYZs takes in the longitude/latitude
// OR easting/northign bounds for the corner locations of the texture
// image.  It also takes in an altitude for these corners above the
// ellipsoid's surface.  It computes the XYZ locations of the corners
// in geocentric coordinates as well as the altitude of the texture
// image's center.  This method also computes the average of the
// corners' locations.

void TextureSector::compute_corner_XYZs(
   double corner_altitude,vector<threevector>& corner)
{
   cout << "inside TextureSector::compute_corner_XYZs()" << endl;
//   cout << "easting_lo = " << easting_lo 
//        << " longitude_lo = " << longitude_lo << endl;

// FAKE FAKE:  Tues Dec 10, 2013 at 6:13 pm

   double delta_easting=0;
   double delta_northing=0;

//   double delta_easting=-7;
//   double delta_northing=5;

//   double delta_easting=100;
//   double delta_northing=100;

//   cout << "Enter delta_easting:" << endl;
//   cin >> delta_easting;
//   cout << "Enter delta_northing:" << endl;
//   cin >> delta_northing;

   easting_lo += delta_easting;
   easting_hi += delta_easting;
   northing_lo += delta_northing;
   northing_hi += delta_northing;

   corner.clear();
   if (Ellipsoid_model_ptr != NULL)
   {
      corner.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         longitude_lo,latitude_lo,corner_altitude));
      corner.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         longitude_hi,latitude_lo,corner_altitude));
      corner.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         longitude_hi,latitude_hi,corner_altitude));
      corner.push_back(Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         longitude_lo,latitude_hi,corner_altitude));

      threevector center=0.25*(corner[0]+corner[1]+corner[2]+corner[3]);

      double center_longitude,center_latitude;
      Ellipsoid_model_ptr->ConvertXYZToLongLatAlt(
         center,center_longitude,center_latitude,center_altitude);
   }
   else
   {
      if (easting_lo > 0.5*NEGATIVEINFINITY)
      {
         corner.push_back(threevector(
            easting_lo,northing_lo,center_altitude));
         corner.push_back(threevector(
            easting_hi,northing_lo,center_altitude));
         corner.push_back(threevector(
            easting_hi,northing_hi,center_altitude));
         corner.push_back(threevector(
            easting_lo,northing_hi,center_altitude));
         cout << "easting_lo = " << easting_lo
              << " easting_hi = " << easting_hi << endl;
         cout << "northing_lo = " << northing_lo
              << " northing_hi = " << northing_hi << endl;
      }
      else
      {
         geopoint curr_geopoint(longitude_lo,latitude_lo);
         corner.push_back(threevector(
            curr_geopoint.get_UTM_easting(),
            curr_geopoint.get_UTM_northing(),center_altitude));
      
         curr_geopoint=geopoint(longitude_hi,latitude_lo);
         corner.push_back(threevector(
            curr_geopoint.get_UTM_easting(),
            curr_geopoint.get_UTM_northing(),center_altitude));

         curr_geopoint=geopoint(longitude_hi,latitude_hi);
         corner.push_back(threevector(
            curr_geopoint.get_UTM_easting(),
            curr_geopoint.get_UTM_northing(),center_altitude));

         curr_geopoint=geopoint(longitude_lo,latitude_hi);
         corner.push_back(threevector(
            curr_geopoint.get_UTM_easting(),
            curr_geopoint.get_UTM_northing(),center_altitude));
      }

   } // Ellipsoid_model_center conditional;

   compute_corners_avg_location(corner);
}

// ---------------------------------------------------------------------
// Member function compute_corner_avg_location() computes the average
// of the input corners' locations.

void TextureSector::compute_corners_avg_location(
   const vector<threevector>& corner)
{
   cout << "inside TextureSector::compute_corner_avg_location()" << endl;
   
// Compute average of corner locations:

   corners_average_location=Zero_vector;
   for (int c=0; c<int(corner.size()); c++)
   {
      cout << "c = " << c
           << " corner[c] = " << corner[c] << endl;
      corners_average_location += corner[c];
   }
   corners_average_location /= corner.size();

//   cout << "corners_average_location = " << corners_average_location << endl;
}

// ==========================================================================
// Video chip member functions
// ==========================================================================

// Member function initialize_video_chip sets the input video's corner
// texture fractions to their default extremal values.  It also
// initializes the video chip corners' world space locations based
// upon member STL vectors video_corner_vertices.

void TextureSector::initialize_video_chip(Movie* Movie_ptr)
{
//   cout << "inside TextureSector::initialize_video_chip()" << endl;

   if (texture_rectangle_ptr==NULL)
   {
      cout << "Error in TextureSector::initialize_video_chip()!" << endl;
      cout << "texture_rectangle_ptr = NULL!" << endl;
      return;
   }

   twovector lower_left_texture_fracs(
      Movie_ptr->get_minU(),Movie_ptr->get_minV());
   twovector lower_right_texture_fracs(
      Movie_ptr->get_maxU(),Movie_ptr->get_minV());
   twovector upper_right_texture_fracs(
      Movie_ptr->get_maxU(),Movie_ptr->get_maxV());
   twovector upper_left_texture_fracs(
      Movie_ptr->get_minU(),Movie_ptr->get_maxV());
   
   threevector video_bottom_left=video_corner_vertices[0];
   threevector video_bottom_right=video_corner_vertices[1];
   threevector video_top_right=video_corner_vertices[2];
   threevector video_top_left=video_corner_vertices[3];

//   cout << "video_bottom_left = " << video_bottom_left << endl;
//   cout << "video_bottom_right = " << video_bottom_right << endl;
//   cout << "video_top_right = " << video_top_right << endl;
//   cout << "video_top_left = " << video_top_left << endl;
 
   Movie_ptr->initialize_3D_chip(lower_left_texture_fracs,
                                 lower_right_texture_fracs,
                                 upper_right_texture_fracs,
                                 upper_left_texture_fracs,
                                 video_top_right,video_top_left,
                                 video_bottom_left,video_bottom_right);

   Movie_ptr->compute_3D_chip(lower_left_texture_fracs,
                              lower_right_texture_fracs,
                              upper_right_texture_fracs,
                              upper_left_texture_fracs);
}

/*
// ---------------------------------------------------------------------
// Member function update_movie_chips is an experimental method
// created for algorithm development purposes only...

void TextureSector::update_movie_chips(
   vector< vector<threevector> >& multi_BL_BR_TR_TL_posns )
{
//   cout << "inside TextureSector::update_movie_chips()" << endl;
   int n_movies=int(multi_BL_BR_TR_TL_posns.size());
   if (n_movies != int(Movie_ptrs.size()))
   {
      cout << "Trouble in TextureSector::update_movie_chips()!" << endl;
      cout << "multi_BL_BR_TR_TL_posns.size() = " 
           << multi_BL_BR_TR_TL_posns.size() << endl;
      cout << "Movie_ptrs.size() = " << Movie_ptrs.size() << endl;
      cout << "These two integers should be equal!" << endl;
      return;
   }
   
   for (int m=0; m<n_movies; m++)
   {
      Movie* curr_Movie_ptr=Movie_ptrs[m];
      if (!curr_Movie_ptr->get_dynamic_frame_flag()) continue;
      vector<threevector> curr_BL_BR_TR_TL=multi_BL_BR_TR_TL_posns[m];
      curr_Movie_ptr->compute_3D_chip(
         curr_BL_BR_TR_TL[0],curr_BL_BR_TR_TL[1],
         curr_BL_BR_TR_TL[2],curr_BL_BR_TR_TL[3]);
   } // loop over index m labeling 
}
*/



// ==========================================================================
// As of Feb 3, 2010, we have become hopelessly confused with our
// overly complicated TextureSector and TextureSectorsGroup classes !
// So we'll try rebuilding these classes using the much simpler
// approach of consolidating all computer graphics within Movie class
// objects...
// ==========================================================================

// Member function reposition_Movie() resets a movie's 3D coordinates
// so that it hovers above the lat-lon bounding box set within
// TextureSector::construct_long_lat_bbox().

void TextureSector::reposition_Movie(Movie* Movie_ptr)
{
//   cout << "inside TextureSector::reposition_Movie()" << endl;

   threevector bottom_left=lower_left_corner_ptr->get_UTM_posn();
   threevector bottom_right(
      upper_right_corner_ptr->get_UTM_easting(),
      lower_left_corner_ptr->get_UTM_northing(),
      upper_right_corner_ptr->get_altitude());
   threevector top_right=upper_right_corner_ptr->get_UTM_posn();
   threevector top_left(
      lower_left_corner_ptr->get_UTM_easting(),
      upper_right_corner_ptr->get_UTM_northing(),
      upper_right_corner_ptr->get_altitude());

   Movie_ptr->reset_geom_vertices(
      bottom_right,bottom_left,top_left,top_right);
}
