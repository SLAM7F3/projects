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
#ifndef OSG_POLYTOPE_INTERSECTVISITOR
#define OSG_POLYTOPE_INTERSECTVISITOR 1

#include "osg/osgIntersector/intersector.h"
#include "osg/osgIntersector/osg_gen_intersectvisitor.h"



namespace osgIsect
{

using namespace osg;



class OSGINTERSECT PolytopeIntersectVisitor : public GenIntersectVisitor<RefPolytopeIsector>
{
public:
    PolytopeIntersectVisitor()
    {
    }
    virtual ~PolytopeIntersectVisitor() {}

    HitList& getIntersections(osg::Node *scene,const RefPolytopeIsector* polytope)
    {
        _polytope = new RefPolytopeIsector(*polytope);


        addGen(_polytope.get());

        scene->accept(*this);
        return getHitList(_polytope.get());
    }


private:
    osg::ref_ptr<RefPolytopeIsector> _polytope;
    //friend osgIntersectVisitor;
};





}

#endif
