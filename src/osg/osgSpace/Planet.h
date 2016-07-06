// ==========================================================================
// Header file for Planet class
// ==========================================================================
// Last updated on 12/28/06; 1/4/07; 1/29/07; 2/14/07
// ==========================================================================

#ifndef PLANET_H
#define PLANET_H

#include <iostream>
#include <string>
#include <osg/LightModel>
#include <osg/ShapeDrawable>
#include "osg/osgSceneGraph/DataGraph.h"

// class osg::Billboard;
// class osg::Drawable;
// class osg::Geode;
// class osg::Image;
// class osg::StateSet;

class LeafNodeVisitor;
class threevector;
class TreeVisitor;

class Planet : public DataGraph
{

  public:
    
   enum PlanetType 
   {
      Sun=0,Earth,Moon,Star,Unknown
   };

// Initialization, constructor and destructor functions:

   Planet(int ID,PlanetType& curr_planet_type,LeafNodeVisitor* LNV_ptr,
          TreeVisitor* TV_ptr);
   virtual ~Planet();
   friend std::ostream& operator<< (std::ostream& outstream,const Planet& p);

// Set & get methods:

   double get_earth_radius() const;
   osg::Billboard* get_atmosphere_ptr();
   const osg::Billboard* get_atmosphere_ptr() const;

// Drawing methods:

   osg::Geode* generate_drawable_geode();
   osg::ShapeDrawable* generate_drawable(osg::StateSet* StateSet_ptr);
   void texture_surface(
      std::string texture_filename,osg::StateSet* StateSet_ptr);
   void set_scale_and_posn(
      double curr_t,int pass_number,double scale,const threevector& V);

// Sun light illumination methods:

   osg::Group* createSunLightGroup(const threevector& sun_posn);
   osg::Billboard* generate_atmosphere_billboard(
      double planet_radius,const osg::Vec4& color);
   osg::Drawable* createSquare(
      const osg::Vec3& corner,const osg::Vec3& width,
      const osg::Vec3& height, osg::Image* image=NULL);
   void set_ambient_sunlight_intensity();
   void toggle_ambient_sunlight_intensity();
   osg::Image* createBillboardImage(
      const osg::Vec4& centerColour, unsigned int size, float power);

  protected:

  private:

   bool maximum_ambient_sunlight_flag;
   PlanetType planet_type;
   double earth_radius;
   osg::ref_ptr<osg::Billboard> atmosphere_billboard_refptr;
   osg::ref_ptr<osg::Geode> geode_refptr;
   osg::ref_ptr<osg::LightModel> lightModel_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Planet& p);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double Planet::get_earth_radius() const
{
   return earth_radius;
}

inline osg::Billboard* Planet::get_atmosphere_ptr()
{
   return atmosphere_billboard_refptr.get();
}

inline const osg::Billboard* Planet::get_atmosphere_ptr() const
{
   return atmosphere_billboard_refptr.get();
}

#endif // Planet.h



