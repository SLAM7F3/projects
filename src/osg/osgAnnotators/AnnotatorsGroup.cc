// ==========================================================================
// ANNOTATORSGROUP class member function definitions
// ==========================================================================
// Last modified on 1/29/11; 7/9/11; 11/14/11
// ==========================================================================

#include <iostream>
#include "osg/osgAnnotators/Annotator.h"
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osgTiles/ray_tracer.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void AnnotatorsGroup::allocate_member_objects()
{
}		       

void AnnotatorsGroup::initialize_member_objects()
{
//   cout << "inside AnnotatorsGroup::init_member_objs()" << endl;

   EarthRegionsGroup_ptr=NULL;
   PointFinder_ptr=NULL;
   ray_tracer_ptr=NULL;
}		       

AnnotatorsGroup::AnnotatorsGroup(const int p_ndims,Pass* PI_ptr)
{	
//   cout << "inside AnnotatorsGroup constructor, this = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
}		       

AnnotatorsGroup::~AnnotatorsGroup()
{
}

void AnnotatorsGroup::set_raytracer_ptr(ray_tracer* rt_ptr)
{
//   cout << "inside AnnotatorsGroup::set_raytracer_ptr()" << endl;
//   cout << "this = " << this << endl;
   ray_tracer_ptr=rt_ptr;
//   cout << "ray_tracer_ptr = " << ray_tracer_ptr << endl;
}

ray_tracer* AnnotatorsGroup::get_raytracer_ptr()
{
   return ray_tracer_ptr;
}

const ray_tracer* AnnotatorsGroup::get_raytracer_ptr() const
{
   return ray_tracer_ptr;
}

// ==========================================================================

// Member function convert_UTM_to_LLA_coords takes in an annotator
// whose XYZ coordinates are assumed to be in UTM space.  If an
// EarthRegionsGroup has been defined, this method converts the
// annotator's coordinates to (longitude,latitude,altitude).  If the
// coordinate conversion fails, this boolean method returns false.

bool AnnotatorsGroup::convert_UTM_to_LLA_coords(
   const threevector& UTM_coords,Annotator* annotator_ptr)
{
   bool successful_conversion_flag=false;
   if (EarthRegionsGroup_ptr != NULL)
   {
      EarthRegion* curr_region_ptr=EarthRegionsGroup_ptr->
         get_curr_region_ptr();
      if (curr_region_ptr != NULL && curr_region_ptr->
          retrieve_lat_long_alt(UTM_coords,annotator_ptr->get_geopoint_ptr()))
      {
         successful_conversion_flag=true;
      } // retrieve_lat_long_alt conditional
   } // EarthRegionsGroup_ptr != NULL conditional
   return successful_conversion_flag;
}
