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
#ifndef OSG_LINESEG_INTERSECTVISITOR
#define OSG_LINESEG_INTERSECTVISITOR 1

#include "osg/osgIntersector/intersector.h"
#include <osg/LineSegment>
#include "osg/osgIntersector/osg_gen_intersectvisitor.h"



namespace osgIsect
{

using namespace osg;


//typedef GenIntersectVisitor<LineSegment>::HitList HitList;

class OSGINTERSECT LineSegIntersectVisitor : public GenIntersectVisitor<LineSegment>
{
public:
    LineSegIntersectVisitor()
    {
    }
    virtual ~LineSegIntersectVisitor() {}

    HitList& getIntersections(osg::Node *scene,const LineSegment* lineseg)
    {
        _lineSegment = new osg::LineSegment(*lineseg);

        //std::cout<<"near "<<nr<<std::endl;
        //std::cout<<"far "<<fr<<std::endl;

        addGen(_lineSegment.get());

        scene->accept(*this);
        return getHitList(_lineSegment.get());
    }


private:
    osg::ref_ptr<osg::LineSegment> _lineSegment;
    //friend osgIntersectVisitor;
};






}

#endif
