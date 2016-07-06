/* -*-c++-*- osgIntersect - Copyright (C) 2005 IGEOSS (www.igeoss.com)
 *                                             frederic.marmond@igeoss.com
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#ifndef OSG_POLYTOPE_PICKER_H
#define OSG_POLYTOPE_PICKER_H

#include "osg/osgPicker/picker.h"

#include <osg/Polytope>
#include "osg/osgIntersector/osg_polytope_intersect.h"

namespace osgIsect{
   template<> struct Intersect_trait<RefPolytopeIsector>
   {
         typedef PolytopeIntersect 	SpecIntersect;
   };
}

#include "osg/osgIntersector/osg_polytope_intersectvisitor.h"
namespace osgIsect{
   template<> struct IntersectVisitor_trait<RefPolytopeIsector>
   {
         typedef PolytopeIntersectVisitor 	SpecIntersectVisitor;
   };
}

#include "osg/osgPicker/osg_gen_picker.h"

#include <osgUtil/SceneView>

namespace osgIsect
{


   class OSGINTERSECT PolytopePicker : public GenPicker<RefPolytopeIsector>
   {
      public:
         PolytopePicker();
         ///normalized screen coords and pixel size
         bool PolytopePicker::pick(
            float rx,float ry,const osg::Matrix& viewMat,
            const osg::Matrix& projMat,osg::Node* node,
            GenIntersectVisitor<RefPolytopeIsector>::HitList& hits,
            float rsx,float rsy);


      protected:
         ///here, we create the object that will intersect the world
         virtual RefPolytopeIsector*  create_pick_obj(
            const osg::Matrix& viewMat,const osg::Matrix& projMat,
            float rx,float ry,float rsx,float rsy);
   };

}
#endif
