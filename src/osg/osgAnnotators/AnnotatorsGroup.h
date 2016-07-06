// ==========================================================================
// Header file for ANNOTATORSGROUP class
// ==========================================================================
// Last modified on 1/29/11; 7/9/11; 11/14/11
// ==========================================================================

#ifndef ANNOTATORSGROUP_H
#define ANNOTATORSGROUP_H

#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGraphicals/PointFinder.h"

class Annotator;
class ray_tracer;
class threevector;

class AnnotatorsGroup 
{

  public:

// Initialization, constructor and destructor functions:

   AnnotatorsGroup(const int p_ndims,Pass* PI_ptr);
   virtual ~AnnotatorsGroup();

// Set & get methods:

   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);
   void set_PointFinder_ptr(PointFinder* PointFinder_ptr);
   void set_raytracer_ptr(ray_tracer* rt_ptr);
   ray_tracer* get_raytracer_ptr();
   const ray_tracer* get_raytracer_ptr() const;

   bool convert_UTM_to_LLA_coords(
      const threevector& UTM_coords,Annotator* annotator_ptr);

  protected:

   EarthRegionsGroup* EarthRegionsGroup_ptr;
   PointFinder* PointFinder_ptr;
   ray_tracer* ray_tracer_ptr;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const AnnotatorsGroup& A);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void AnnotatorsGroup::set_EarthRegionsGroup_ptr(
   EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;
}

inline void AnnotatorsGroup::set_PointFinder_ptr(PointFinder* PointFinder_ptr)
{
   this->PointFinder_ptr=PointFinder_ptr;
}

#endif // AnnotatorsGroup.h



