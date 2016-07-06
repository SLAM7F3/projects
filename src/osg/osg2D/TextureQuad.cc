// ==========================================================================
// TextureQuad class member function definitions
// ==========================================================================
// Last updated on 6/29/07
// ==========================================================================

#include <iostream>
#include <osgDB/ReadFile>
#include <osg/TexMat>
#include <osg/TextureRectangle>
#include "osg/osg2D/TextureQuad.h"
#include "math/twovector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TextureQuad::allocate_member_objects()
{
}		       

void TextureQuad::initialize_member_objects()
{
   Graphical_name="TextureQuad";
}

TextureQuad::TextureQuad(int id):
   Geometrical(3,id)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

TextureQuad::~TextureQuad()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const TextureQuad& T)
{
   outstream << "inside TextureQuad::operator<<" << endl;
   return(outstream);
}

// ==========================================================================
// Texturing methods
// ==========================================================================

osg::Geode* TextureQuad::generate_drawable_geode(
   string image_filename,const vector<twovector>& UV_corners)
{
   osg::Geode* geode_ptr=new osg::Geode();
   osg::Geometry* geometry_ptr=construct_quad_geometry(UV_corners);
   set_quad_texture_coords(geometry_ptr);
   geode_ptr->addDrawable(geometry_ptr);
   
   osg::Texture2D* texture_ptr=new osg::Texture2D;
   osg::StateSet* stateSet_ptr= geode_ptr->getOrCreateStateSet();
   stateSet_ptr->setTextureAttributeAndModes (
      0,texture_ptr,osg::StateAttribute::ON);

   osg::Image* image_ptr=osgDB::readImageFile(image_filename);
   if (!image_ptr)
   {
      cout << "Couldn't find texture image" << endl;
      return NULL;
   }
   texture_ptr->setImage(image_ptr);

   return geode_ptr;
}

// ---------------------------------------------------------------------
osg::Geometry* TextureQuad::construct_quad_geometry(
   const vector<twovector>& UV_corners)
{
   osg::Geometry* quad_geometry_ptr = new osg::Geometry();

   osg::Vec3Array* vertices_ptr=new osg::Vec3Array();
   for (int c=0; c<int(UV_corners.size()); c++)
   {
      vertices_ptr->push_back(
         osg::Vec3(UV_corners[c].get(0),0,UV_corners[c].get(1)));
   }
   quad_geometry_ptr->setVertexArray(vertices_ptr);
                  
//   osg::Vec4Array* colors = new osg::Vec4Array;
//   colors->push_back(osg::Vec4(1,1,1,1));
//   quad_geometry_ptr->setColorArray(colors);
//   quad_geometry_ptr->setColorBinding(osg::Geometry::BIND_OVERALL);

   osg::Vec3Array* normals_ptr=new osg::Vec3Array();
   normals_ptr->push_back (osg::Vec3 (0, -1, 0));
   quad_geometry_ptr->setNormalArray(normals_ptr);
   quad_geometry_ptr->setNormalBinding (osg::Geometry::BIND_OVERALL);

   quad_geometry_ptr->addPrimitiveSet(
      new osg::DrawArrays (osg::PrimitiveSet::QUADS,0,vertices_ptr->size()));

   return quad_geometry_ptr;
}

// ---------------------------------------------------------------------
void TextureQuad::set_quad_texture_coords(
   osg::Geometry* quad_geometry_ptr)
{
   osg::Vec2Array* texcoords = new osg::Vec2Array(4);

   double lo=0.0;
   double hi=1-lo;

   (*texcoords)[0].set(lo,lo); // texture coord for vertex 0
   (*texcoords)[1].set(hi,lo); // texture coord for vertex 1 
   (*texcoords)[2].set(hi,hi); // texture coord for vertex 2
   (*texcoords)[3].set(lo,hi); // texture coord for vertex 3

   quad_geometry_ptr->setTexCoordArray(0,texcoords);
}
