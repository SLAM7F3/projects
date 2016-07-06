// ==========================================================================
// Header file for Polygon class
// ==========================================================================
// Last updated on 2/21/08; 2/25/08; 6/28/12
// ==========================================================================

#ifndef OSGPOLYGON_H
#define OSGPOLYGON_H

#include <iostream>
#include <string>
#include <osg/Array>
#include "osg/osgGeometry/Geometrical.h"
#include "geometry/polygon.h"

// class osg::Drawable;
// class osg::StateSet;
class Pass;
class PolyLine;

namespace osgGeometry
{
   
// Forward declaration of PointsGroup class which sits inside
// osgGeometry namespace:

   class PointsGroup;

   class Polygon : public Geometrical
      {

        public:
    
// Initialization, constructor and destructor functions:

         Polygon(
            const int p_ndims,Pass* PI_ptr,threevector* GO_ptr,
            const threevector& reference_origin,const polygon& p,int id);
         virtual ~Polygon();
         friend std::ostream& operator<< (
            std::ostream& outstream,const Polygon& r);

// Set & get methods:

         polygon* get_relative_poly_ptr();
         const polygon* get_relative_poly_ptr() const;
         osgGeometry::PointsGroup* get_PointsGroup_ptr();
         const osgGeometry::PointsGroup* get_PointsGroup_ptr() const;
         void set_PolyLine_ptr(PolyLine* PolyLine_ptr);
         PolyLine* get_PolyLine_ptr();
         const PolyLine* get_PolyLine_ptr() const;

// Drawing methods:

         osg::Geode* generate_drawable_geode();
         void enable_alpha_blending();
         void set_relative_vertices();
   
        protected:

        private:

         polygon* relative_poly_ptr;
         PolyLine* PolyLine_ptr;
         osgGeometry::PointsGroup* PointsGroup_ptr;
         osg::ref_ptr<osg::Geometry> geom_refptr;
         osg::ref_ptr<osg::StateSet> stateset_refptr;

         void allocate_member_objects(); 
         void initialize_member_objects();
         void docopy(const Polygon& p);

         osg::Geometry* generate_drawable_geom();
      };
   

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline polygon* Polygon::get_relative_poly_ptr()
      {
         return relative_poly_ptr;
      }

   inline const polygon* Polygon::get_relative_poly_ptr() const
      {
         return relative_poly_ptr;
      }

   inline osgGeometry::PointsGroup* Polygon::get_PointsGroup_ptr()
      {
         return PointsGroup_ptr;
      }

   inline const osgGeometry::PointsGroup* Polygon::get_PointsGroup_ptr() const
      {
         return PointsGroup_ptr;
      }

   inline void Polygon::set_PolyLine_ptr(PolyLine* PolyLine_ptr)
      {
         this->PolyLine_ptr=PolyLine_ptr;
      }
   
   inline PolyLine* Polygon::get_PolyLine_ptr()
      {
         return PolyLine_ptr;
      }

   inline const PolyLine* Polygon::get_PolyLine_ptr() const
      {
         return PolyLine_ptr;
      }

} // osgGeometry namespace


#endif // OSGPolygon.h



