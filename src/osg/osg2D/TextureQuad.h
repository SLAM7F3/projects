// ==========================================================================
// Header file for TextureQuad class
// ==========================================================================
// Last updated on 5/29/07
// ==========================================================================

#ifndef TEXTUREQUAD_H
#define TEXTUREQUAD_H

#include <string>
#include <vector>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include "astro_geo/geopoint.h"
#include "osg/osgGeometry/Geometrical.h"
#include "math/twovector.h"

class TextureQuad : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   TextureQuad(int ID);
   virtual ~TextureQuad();
   friend std::ostream& operator<< (
      std::ostream& outstream,const TextureQuad& T);

// Set & get member functions:

// Texturing methods:

   osg::Geode* generate_drawable_geode(
      std::string image_filename,const std::vector<twovector>& UV_corners);

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const TextureQuad& T);

   osg::Geometry* construct_quad_geometry(
      const std::vector<twovector>& UV_corners);
   void set_quad_texture_coords(osg::Geometry* quad_geometry_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // TextureQuad.h



