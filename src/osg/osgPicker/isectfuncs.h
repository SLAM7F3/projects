// ==========================================================================
// Header file for ISECTFUNCS namespace
// ==========================================================================
// Last modified on 6/28/05
// ==========================================================================

#ifndef ISECTFUNCS_H
#define ISECTFUNCS_H

#include <osg/Vec3>
#include "osg/osgIntersector/osg_gen_intersectvisitor.h"
#include "osg/osgIntersector/osg_refpolytopeisector.h"

namespace isectfunc
{
   class osg::Geode;
   class osg::Geometry;
   class osg::Group;
   
   extern osg::Group* picker_shape;
   extern osg::Group* picked_shape;

   void clearPickedShape();
   void clearPickerShape();
   double dist_point_to_segment(osg::Vec3& p, osg::Vec3& s0,osg::Vec3& s1);
   void show_picked_isect(HitBase& hit,osg::Geode* pickedGeode);
   void show_picked_geom(
      HitBase& hit,osg::Geometry* pickedGeometry,osg::Vec3& world_coords);
   void show_picked_point(
      HitBase& hit,osg::Geometry* pickedGeometry,osg::Vec3& world_coords);
   void show_picked_edge(HitBase& hit,osg::Geometry* pickedGeometry);
   void show_picked_triangle(HitBase& hit,osg::Geometry* pickedGeometry);
   void createPickerShape(RefPolytopeIsector* polytope);
   void visualize_picking();
   
// ==========================================================================
// Inlined methods:
// ==========================================================================

} // isectfunc namespace

#endif // isectfuncs.h



