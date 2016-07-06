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
#include "osg/osgPicker/osg_lineseg_picker.h"
#include "osg/osgPicker/osg_lineseg_pickvisitor.h"

#include <osg/Projection>

//a lot is token from osgProducer/Viewer

using namespace osgIsect;


LineSegPicker::LineSegPicker()
{
   _iv=NULL;
}

bool LineSegPicker::pick(float rx,float ry,const osg::Matrix& viewMat,const osg::Matrix& projMat,osg::Node* node,GenIntersectVisitor<LineSegment>::HitList& hits,float rsx,float rsy)
{
   _iv=new LineSegPickVisitor();
   bool rc=pick_gen(rx,ry,viewMat,projMat,node,hits,rsx,rsy);
   delete _iv;
   _iv=NULL;
   return rc;
}


LineSegment*	LineSegPicker::create_pick_obj(const osg::Matrix& viewMat,const osg::Matrix& projMat,float rx,float ry,float,float)
{
   osg::Matrixd inverseMVPW;
   inverseMVPW.invert(viewMat * projMat);
   osg::Vec3 near_point = osg::Vec3(rx,ry,-1.0f)*inverseMVPW;
   osg::Vec3 far_point = osg::Vec3(rx,ry,1.0f)*inverseMVPW;
   _iv->setxy(rx,ry);
   return new LineSegment(near_point,far_point);
}
