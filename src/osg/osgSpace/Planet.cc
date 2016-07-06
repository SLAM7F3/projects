// ==========================================================================
// Planet class member function definitions
// ==========================================================================
// Last updated on 1/3/07; 1/4/07; 1/29/07; 7/29/07
// ==========================================================================

#include <string>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Light>
#include <osg/LightSource>

#include <osg/Material>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include "osg/osgSpace/Planet.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Planet::allocate_member_objects()
{
   geode_refptr = new osg::Geode();
   lightModel_refptr = new osg::LightModel;
}		       

void Planet::initialize_member_objects()
{
   Graphical_name="Planet";
   maximum_ambient_sunlight_flag=false;
//   maximum_ambient_sunlight_flag=true;
   earth_radius=6378137.0;  // meters  (true size)
   DataNode_refptr=geode_refptr.get();
}

Planet::Planet(int ID,PlanetType& curr_planet_type,LeafNodeVisitor* LNV_ptr,
               TreeVisitor* TV_ptr):
   DataGraph(3,ID,LNV_ptr,TV_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
   planet_type=curr_planet_type;
}		       

Planet::~Planet()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Planet& p)
{
   outstream << "inside Planet::operator<<" << endl;
   
   threevector UVW;
   p.get_UVW_coords(0,0,UVW);
   cout << "UVW = " << UVW << endl;
//   outstream << static_cast<const Annotator&>(l) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing a textured unit sphere centered at the origin.

osg::Geode* Planet::generate_drawable_geode()
{

   osg::StateSet* StateSet_ptr=geode_refptr->getOrCreateStateSet();
   geode_refptr->addDrawable(generate_drawable(StateSet_ptr));
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
osg::ShapeDrawable* Planet::generate_drawable(osg::StateSet* StateSet_ptr)
{
   const double radius=1;
   osg::Sphere* unitSphere = new osg::Sphere( osg::Vec3(0,0,0), radius);
   osg::ShapeDrawable* unitSphereDrawable = new osg::ShapeDrawable(
      unitSphere);

   string texture_filename="";
   switch (planet_type)
   {
      case Sun:
         texture_filename="SolarSystem/sun_medres.png";
         break;
      case Earth:
         texture_filename="Images/globe.jpg";
         break;
      case Moon:
         texture_filename="SolarSystem/MoonMap2_2500x1250.jpg";
//         texture_filename="SolarSystem/moon_bright.jpg";
//         texture_filename="SolarSystem/moon256128.jpg";
         break;
      case Star:
         texture_filename="";
         break;
      case Unknown:
         texture_filename="";
         break;
   }
   if (texture_filename != "") texture_surface(texture_filename,StateSet_ptr);

// Make Sun a light emitter:

   if (planet_type==Sun)
   {
      osg::Material* material = new osg::Material;
      material->setEmission( 
         osg::Material::FRONT_AND_BACK, osg::Vec4(1,1,0,0) );
      StateSet_ptr->setAttributeAndModes( 
         material, osg::StateAttribute::ON );
   }
   else if (planet_type==Moon)
   {
      osg::Material* material_ptr = new osg::Material;

//      material_ptr->setColorMode(osg::Material::SPECULAR);
//      material_ptr->setSpecular(
//         osg::Material::FRONT,osg::Vec4(1,1,1,1));

      material_ptr->setAmbient(
         osg::Material::FRONT_AND_BACK,osg::Vec4(1,1,1,1));

      StateSet_ptr->setAttributeAndModes( 
         material_ptr, osg::StateAttribute::ON );

//      dirtyDisplay();      
      for (unsigned int n=0; n<geode_refptr->getNumDrawables(); n++)
      {
         geode_refptr->getDrawable(n)->dirtyDisplayList();
      }

   }
   
   return unitSphereDrawable;
}

// ---------------------------------------------------------------------
void Planet::texture_surface(
   string texture_filename,osg::StateSet* StateSet_ptr)
{
   osg::Image* Surface_image = osgDB::readImageFile(texture_filename);
   osg::Texture2D* SurfaceTexture_ptr = new osg::Texture2D(Surface_image);
   SurfaceTexture_ptr->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
   SurfaceTexture_ptr->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
   StateSet_ptr->setTextureAttributeAndModes( 
      0, SurfaceTexture_ptr, osg::StateAttribute::ON );
}

// ==========================================================================
// Planet manipulation methods
// ==========================================================================

// Member function set_scale_and_posn takes in a time and pass number,
// scale and position.  The scaling factor (relative to 1 meter) and
// position information (measured in meters) are stored for later
// callback retrieval.

void Planet::set_scale_and_posn(
   double curr_t,int pass_number,double scale,const threevector& V)
{
   set_scale(curr_t,pass_number,threevector(scale,scale,scale));
   set_UVW_coords(curr_t,pass_number,V);

//   threevector UVW;
//   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;
}

// ==========================================================================
// Sun light illumination methods
// ==========================================================================

osg::Group* Planet::createSunLightGroup(const threevector& sun_posn)
{
   osg::LightSource* sunLightSource = new osg::LightSource;
   osg::Light* sunLight = sunLightSource->getLight();

   sunLight->setPosition( osg::Vec4( sun_posn.get(0),sun_posn.get(1),
                                     sun_posn.get(2),1.0));

//   sunLight->setAmbient( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
//   sunLight->setAmbient( osg::Vec4( 0.85f, 0.85f, 0.85f, 1.0f ) );
//   sunLight->setDiffuse( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
//   sunLight->setDiffuse( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//   sunLight->setSpecular( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

   sunLightSource->setLight( sunLight );
   sunLightSource->setLocalStateSetModes( osg::StateAttribute::ON );
   sunLightSource->getOrCreateStateSet()->setMode(
      GL_LIGHTING, osg::StateAttribute::ON);
   sunLightSource->getOrCreateStateSet()->setAttribute(
      lightModel_refptr.get());

   set_ambient_sunlight_intensity();

   return sunLightSource;
}

// ---------------------------------------------------------------------
// Member function set_ambient_sunlight_intensity sets the
// LightModel's ambient intensity level to either a small or large
// value depending upon whether day/night vs permanent daytime
// illumination conditions are to be simulated.

void Planet::set_ambient_sunlight_intensity()
{
   const osg::Vec4 daynight_ambient_intensity(0.1,0.1,0.1,1); // isds3d laptop
//   const osg::Vec4 daynight_ambient_intensity(0.4,0.4,0.4,1.0); // Fusion1 tower
   const osg::Vec4 permanent_day_ambient_intensity(1,1,1,1);

   if (maximum_ambient_sunlight_flag)
   {
      lightModel_refptr->setAmbientIntensity(permanent_day_ambient_intensity);
   }
   else
   {
      lightModel_refptr->setAmbientIntensity(daynight_ambient_intensity);
   }
}

// ---------------------------------------------------------------------
void Planet::toggle_ambient_sunlight_intensity()
{
   maximum_ambient_sunlight_flag=!maximum_ambient_sunlight_flag;
   set_ambient_sunlight_intensity();
}

// ---------------------------------------------------------------------
// Member function generate_atmosphere_billboard

osg::Billboard* Planet::generate_atmosphere_billboard(
   double planet_radius,const osg::Vec4& color)
{
   atmosphere_billboard_refptr = new osg::Billboard();
   atmosphere_billboard_refptr->setMode(osg::Billboard::POINT_ROT_EYE);
   double square_size=1.2*planet_radius;

   atmosphere_billboard_refptr->addDrawable(
      createSquare(osg::Vec3(-square_size,0,-square_size),
                   osg::Vec3(2*square_size,0,0),
                   osg::Vec3(0,0,2*square_size),
                   createBillboardImage( 
                      color,
//                      colorfunc::get_OSG_color(colorfunc::yellow),
//                      colorfunc::get_OSG_color(colorfunc::cyan),
//                      osg::Vec4( 1.0, 1.0, 0, 1.0), 
                     static_cast<unsigned int>(0.5*square_size), 1.0) ),
      osg::Vec3(0.0f,0.0f,0.0f));
   return atmosphere_billboard_refptr.get();
}

// ---------------------------------------------------------------------
// Member function createSquare was lifted from the osgPlanets
// example.  It appears to generate a rectangle around a planet for
// billboard emplacement.

/** create quad at specified position. */

osg::Drawable* Planet::createSquare(
   const osg::Vec3& corner,const osg::Vec3& width,
   const osg::Vec3& height, osg::Image* image)
{
   // set up the Geometry.
   osg::Geometry* geom = new osg::Geometry;

   osg::Vec3Array* coords = new osg::Vec3Array(4);
   (*coords)[0] = corner;
   (*coords)[1] = corner+width;
   (*coords)[2] = corner+width+height;
   (*coords)[3] = corner+height;
   geom->setVertexArray(coords);

   osg::Vec3Array* norms = new osg::Vec3Array(1);
   (*norms)[0] = width^height;
   (*norms)[0].normalize();
    
   geom->setNormalArray(norms);
   geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

   osg::Vec2Array* tcoords = new osg::Vec2Array(4);
   (*tcoords)[0].set(0.0f,0.0f);
   (*tcoords)[1].set(1.0f,0.0f);
   (*tcoords)[2].set(1.0f,1.0f);
   (*tcoords)[3].set(0.0f,1.0f);
   geom->setTexCoordArray(0,tcoords);
    
   osg::Vec4Array* colours = new osg::Vec4Array(1);
   (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);
   geom->setColorArray(colours);
   geom->setColorBinding(osg::Geometry::BIND_OVERALL);
   geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    
   if (image)
   {
      osg::StateSet* stateset = new osg::StateSet;
      osg::Texture2D* texture = new osg::Texture2D;
      texture->setImage(image);
      stateset->setTextureAttributeAndModes(
         0,texture,osg::StateAttribute::ON);
      stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
      stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
      stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      geom->setStateSet(stateset);
   }
    
   return geom;
}

// ---------------------------------------------------------------------
// Member function createBillboardImage was lifted from the osgPlanets
// example.  It appears to generate an attenuated atmosphere
// surrounding a planet.

osg::Image* Planet::createBillboardImage(
   const osg::Vec4& centerColour, unsigned int size, float power)
{
   osg::Vec4 backgroundColour = centerColour;
   backgroundColour[3] = 0.0f;
    
   osg::Image* image = new osg::Image;
   image->allocateImage(size,size,1,GL_RGBA,GL_UNSIGNED_BYTE);
     
   float mid = (float(size)-1)*0.5f;
   float div = 2.0f/float(size);
   for(unsigned int r=0;r<size;++r)
   {
      unsigned char* ptr = image->data(0,r,0);
      for(unsigned int c=0;c<size;++c)
      {
         float dx = (float(c) - mid)*div;
         float dy = (float(r) - mid)*div;
         float r = powf(1.0f-sqrtf(dx*dx+dy*dy),power);
         if (r<0.0f) r=0.0f;
         osg::Vec4 color = centerColour*r+backgroundColour*(1.0f-r);
         // color.set(1.0f,1.0f,1.0f,0.5f);
         *ptr++ = (unsigned char)((color[0])*255.0f);
         *ptr++ = (unsigned char)((color[1])*255.0f);
         *ptr++ = (unsigned char)((color[2])*255.0f);
         *ptr++ = (unsigned char)((color[3])*255.0f);
      }
   }
   return image;
}
