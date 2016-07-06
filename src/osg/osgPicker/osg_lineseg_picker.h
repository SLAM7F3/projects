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
#ifndef OSG_LINESEG_PICKER_H
#define OSG_LINESEG_PICKER_H

#include "osg/osgPicker/picker.h"

#include <osg/LineSegment>
#include "osg/osgIntersector/osg_lineseg_intersect.h"

namespace osgIsect
{
template<> struct Intersect_trait<LineSegment>
{
	typedef LineSegIntersect 	SpecIntersect;
};
}
#include "osg/osgIntersector/osg_lineseg_intersectvisitor.h"
namespace osgIsect{
template<> struct IntersectVisitor_trait<LineSegment>
{
	typedef LineSegIntersectVisitor 	SpecIntersectVisitor;
};
}
#include "osg/osgPicker/osg_gen_picker.h"

#include <osgUtil/SceneView>


namespace osgIsect
{

class OSGINTERSECT LineSegPicker : public GenPicker<LineSegment>
{
public:
	LineSegPicker();
	bool LineSegPicker::pick(float rx,float ry,const osg::Matrix& viewMat,const osg::Matrix& projMat,osg::Node* node,GenIntersectVisitor<LineSegment>::HitList& hits,float rsx,float rsy);


protected:
	///here, we create the object that will intersect the world
	///must be implemented in the specialized class (for LineSeg, polytope, ...)
	virtual LineSegment*	create_pick_obj(const osg::Matrix& viewMat,const osg::Matrix& projMat,float rx,float ry,float rsx,float rsy);
};

}
#endif
