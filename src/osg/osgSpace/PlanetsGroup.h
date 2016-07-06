// ==========================================================================
// Header file for PLANETSGROUP class
// ==========================================================================
// Last modified on 2/14/07; 3/12/07; 10/13/07
// ==========================================================================

#ifndef PLANETSGROUP_H
#define PLANETSGROUP_H

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgSceneGraph/DataGraphsGroup.h"
#include "osg/osgSpace/Planet.h"

class Clock;
class EarthGrid;
class threevector;

class PlanetsGroup : public DataGraphsGroup
{

  public:

// Initialization, constructor and destructor functions:

   PlanetsGroup(Pass* PI_ptr,Clock* clock_ptr,threevector* GO_ptr);
   virtual ~PlanetsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const PlanetsGroup& P);

// Set & get methods:

   Planet* get_Planet_ptr(int n) const;
   Planet* get_ID_labeled_Planet_ptr(int ID) const;
   Planet* get_sun_ptr() const;
   Planet* get_moon_ptr() const;
   DataGraph* get_EarthGraph_ptr();
   osg::MatrixTransform* get_EarthSpinTransform_ptr();
   const osg::MatrixTransform* get_EarthSpinTransform_ptr() const;
   osg::MatrixTransform* get_PlanetSpinTransform_ptr();
   const osg::MatrixTransform* get_PlanetSpinTransform_ptr() const;

// Planet creation methods:

   DataGraph* generate_EarthGraph();
   osg::Group* generate_solarsystem(EarthGrid* grid_ptr);
   osg::Geode* generate_starfield(EarthGrid* grid_ptr);
   osg::Geometry* generate_starfield_geometry(
      int n_stars,double star_size,EarthGrid* grid_ptr);
   osg::MatrixTransform* compute_earthrotation();
   Planet* generate_new_Planet(Planet::PlanetType planet_type);
   void update_display();

// Planet manipulation methods:

   void set_const_scale_and_posn(int ID,double scale,const threevector& V);
   void toggle_ambient_sunlight();

  protected:

  private:

   double computer_graphics_sun_scalefactor,
      computer_graphics_star_scalefactor;
   double eff_sun_dist_from_earth,eff_sun_scale;
   double eff_star_dist_from_earth,eff_star_scale;
   std::string earth_filename;
   DataGraph* EarthGraph_ptr;
   Planet *sun_ptr,*moon_ptr;

   osg::ref_ptr<osg::MatrixTransform> PlanetSpinTransform_refptr;
   osg::ref_ptr<osg::MatrixTransform> EarthSpinTransform_refptr;
   osg::ref_ptr<osg::Group> solarsystem_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PlanetsGroup& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Planet* PlanetsGroup::get_Planet_ptr(int n) const
{
   return dynamic_cast<Planet*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Planet* PlanetsGroup::get_ID_labeled_Planet_ptr(int ID) const
{
   return dynamic_cast<Planet*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline Planet* PlanetsGroup::get_sun_ptr() const
{
   return sun_ptr;
}

// --------------------------------------------------------------------------
inline Planet* PlanetsGroup::get_moon_ptr() const
{
   return moon_ptr;
}

// --------------------------------------------------------------------------
inline DataGraph* PlanetsGroup::get_EarthGraph_ptr()
{
   return EarthGraph_ptr;
}

// --------------------------------------------------------------------------
inline osg::MatrixTransform* PlanetsGroup::get_EarthSpinTransform_ptr()
{
   return EarthSpinTransform_refptr.get();
}

inline const osg::MatrixTransform* PlanetsGroup::get_EarthSpinTransform_ptr() 
   const
{
   return EarthSpinTransform_refptr.get();
}

// --------------------------------------------------------------------------
inline osg::MatrixTransform* PlanetsGroup::get_PlanetSpinTransform_ptr()
{
   return PlanetSpinTransform_refptr.get();
}

inline const osg::MatrixTransform* PlanetsGroup::get_PlanetSpinTransform_ptr()
   const
{
   return PlanetSpinTransform_refptr.get();
}

#endif // PlanetsGroup.h



