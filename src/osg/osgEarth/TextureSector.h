// As of Jan 2010, we believe that this TextureSector class needs
// MAJOR reworking.  In particular, it should take in a Movie object
// and use the Movie's GEODE rather than instantiating its own.  The
// geode's geometry can be specialized within this class to fit
// somewhere on the earth.  But the basic computer graphics should not
// replicate that already coded up within our Movie/G99VD classes...


// ==========================================================================
// Header file for Textureector class
// ==========================================================================
// Last updated on 1/30/10; 2/3/10; 8/5/13
// ==========================================================================

#ifndef TEXTURESECTOR_H
#define TEXTURESECTOR_H

#include <string>
#include <vector>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include "astro_geo/geopoint.h"
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osg2D/Movie.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"
#include "math/twovector.h"

class Earth;
class Ellipsoid_model;
class MoviesGroup;

class TextureSector : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   TextureSector(int ID,Earth* Earth_ptr=NULL);
   virtual ~TextureSector();
   friend std::ostream& operator<< (
      std::ostream& outstream,const TextureSector& T);

// Set & get member functions:

   void set_longitude_lo(double l);
   void set_longitude_hi(double l);
   void set_latitude_lo(double l);
   void set_latitude_hi(double l);
   void set_center_altitude(double a);

   double get_longitude_lo() const;
   double get_longitude_hi() const;
   double get_latitude_lo() const;
   double get_latitude_hi() const;
   double get_center_altitude() const;

   void set_alpha_blending_value(double alpha);
   void set_average_posn(double curr_t,int pass_number);

   geopoint* get_lower_left_corner_ptr();
   const geopoint* get_lower_left_corner_ptr() const;

   void push_back_video_corner_vertices(
      const std::vector<threevector>& vertices);
   const threevector& get_bottom_right_video_corner() const;
   const threevector& get_bottom_left_video_corner() const;
   const threevector& get_top_left_video_corner() const;
   const threevector& get_top_right_video_corner() const;

   void set_texture_rectangle_ptr(texture_rectangle* tr_ptr);
   texture_rectangle* get_texture_rectangle_ptr();
   const texture_rectangle* get_texture_rectangle_ptr() const;

   void push_back_Movie_ptr(Movie* m_ptr);
   void pop_off_Movie_ptr(Movie* m_ptr);
   std::vector<Movie*> get_Movie_ptrs();
   const std::vector<Movie*> get_Movie_ptrs() const;

// Texturing methods:

   void construct_easting_northing_bbox(
      bool northern_hemisphere_flag,int UTM_zonenumber,
      double east_lo,double east_hi,double north_lo,double north_hi,
      double texture_altitude=0);
   void construct_long_lat_bbox(
      double long_lo,double long_hi,double lat_lo,double lat_hi,
      double texture_altitude=0);
   osg::Geode* generate_drawable_geode(std::string image_filename);
   osg::Geode* generate_drawable_geode(Movie* Movie_ptr);
   void compute_corner_XYZs(
      double corner_altitude,std::vector<threevector>& corner);
   void compute_corners_avg_location(const std::vector<threevector>& corner);

// Video chip member functions

   void initialize_video_chip(Movie* Movie_ptr);
//   void update_movie_chips(
//      std::vector< std::vector<threevector> >& multi_BL_BR_TR_TL_posns );

// New members added in 2010 which we at least understand...

   void reposition_Movie(Movie* Movie_ptr);

  private:

   double longitude_lo,longitude_hi,latitude_lo,latitude_hi,center_altitude;
   double easting_lo,easting_hi,northing_lo,northing_hi;
   double alpha_blending_value;
   threevector corners_average_location;
   Ellipsoid_model* Ellipsoid_model_ptr;
   geopoint *lower_left_corner_ptr,*upper_right_corner_ptr;

   std::vector<threevector> video_corner_vertices;
   texture_rectangle* texture_rectangle_ptr;
   std::vector<Movie*> Movie_ptrs;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const TextureSector& T);

   osg::Geometry* construct_surface_sector_geometry();
   void set_sector_texture_coords(osg::Geometry* sector_geometry_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void TextureSector::set_longitude_lo(double l)
{
   longitude_lo=l;
}

inline void TextureSector::set_longitude_hi(double l)
{
   longitude_hi=l;
}

inline void TextureSector::set_latitude_lo(double l)
{
   latitude_lo=l;
}

inline void TextureSector::set_latitude_hi(double l)
{
   latitude_hi=l;
}

inline void TextureSector::set_center_altitude(double a)
{
   center_altitude=a;
}

inline double TextureSector::get_longitude_lo() const
{
   return longitude_lo;
}

inline double TextureSector::get_longitude_hi() const
{
   return longitude_hi;
}

inline double TextureSector::get_latitude_lo() const
{
   return latitude_lo;
}

inline double TextureSector::get_latitude_hi() const
{
   return latitude_hi;
}

inline double TextureSector::get_center_altitude() const
{
   return center_altitude;
}

inline void TextureSector::set_alpha_blending_value(double alpha)
{
   alpha_blending_value=alpha;
}

inline geopoint* TextureSector::get_lower_left_corner_ptr()
{
   return lower_left_corner_ptr;
}

inline const geopoint* TextureSector::get_lower_left_corner_ptr() const
{
   return lower_left_corner_ptr;
}

inline void TextureSector::push_back_video_corner_vertices(
   const std::vector<threevector>& vertices)
{
   for (int v=0; v<int(vertices.size()); v++)
   {
      video_corner_vertices.push_back(vertices[v]);
   }
}

inline const threevector& TextureSector::get_bottom_right_video_corner() const
{
   return video_corner_vertices[1];
}

inline const threevector& TextureSector::get_bottom_left_video_corner() const
{
   return video_corner_vertices[0];
}

inline const threevector& TextureSector::get_top_left_video_corner() const
{
   return video_corner_vertices[3];
}

inline const threevector& TextureSector::get_top_right_video_corner() const
{
   return video_corner_vertices[2];
}

inline void TextureSector::set_texture_rectangle_ptr(
   texture_rectangle* tr_ptr)
{
   texture_rectangle_ptr=tr_ptr;
}

inline texture_rectangle* TextureSector::get_texture_rectangle_ptr()
{
   return texture_rectangle_ptr;
}

inline const texture_rectangle* TextureSector::get_texture_rectangle_ptr() const
{
   return texture_rectangle_ptr;
}

inline void TextureSector::push_back_Movie_ptr(Movie* m_ptr)
{
   Movie_ptrs.push_back(m_ptr);
}

inline std::vector<Movie*> TextureSector::get_Movie_ptrs()
{
   return Movie_ptrs;
}

inline const std::vector<Movie*> TextureSector::get_Movie_ptrs() const
{
   return Movie_ptrs;
}

#endif // TextureSector.h



