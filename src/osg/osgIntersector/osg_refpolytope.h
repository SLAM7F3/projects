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
#ifndef OSG_REFPOLYTOPE_H
#define OSG_REFPOLYTOPE_H

#include "osg/osgIntersector/intersector.h"
#include <osg/Polytope>
#include <osg/Referenced>

using namespace osg;


namespace osgIsect
{

class OSGINTERSECT RefPolytope : public Referenced,public Polytope
{
public:
	RefPolytope() : Referenced(),Polytope(){};
	RefPolytope(const Polytope &cv) : Referenced(),Polytope(cv){};
	RefPolytope(const RefPolytope &cv) : Referenced(cv),Polytope(cv){};
	RefPolytope(const PlaneList &pl) : Referenced(),Polytope(pl){};


protected:
};


}
#endif
