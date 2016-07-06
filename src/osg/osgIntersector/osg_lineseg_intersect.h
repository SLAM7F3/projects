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
#ifndef OSG_LINESEG_INTERSECT_H
#define OSG_LINESEG_INTERSECT_H

#include "osg/osgIntersector/intersector.h"
#include <osg/Drawable>
#include <osg/Geometry>

#include <osg/LineSegment>
#include "osg/osgIntersector/osg_gen_intersect.h"

using namespace osg;

namespace osgIsect
{

//typedef GenIndexFunctor<GenGeomIntersect<Gen> > SpecIndexFunctor;

class OSGINTERSECT LineSegIntersect :public GenGeomIntersect<LineSegment>
{
public:


    Vec3    _s;
    Vec3    _d;
    float   _length;


    void set(const LineSegment& gen,osg::Array* vertices,float ratio=FLT_MAX)
    {
    	GenGeomIntersect<LineSegment>::set(gen,vertices,ratio);
        _s = _gen->start();
        _d = _gen->end()-_gen->start();
        _length = _d.length();
        _d /= _length;

    }

    //intersection with a point
    inline void operator()(const Vec3& , bool )
    {
    	DBG_WAR("Intersect between LineSeg and Point is not defined.");
	return;
    }


    //intersection with a segment
    inline void operator()(const Vec3& ,const Vec3& , bool)
    {
    	DBG_WAR("Intersect between LineSeg and Segment is not defined.");
	return;

    }

    //   bool intersect(const Vec3& v1,const Vec3& v2,const Vec3& v3,float& r)
    inline void operator()(const Vec3& v1,const Vec3& v2,const Vec3& v3, bool treatVertexDataAsTemporary)
    {
//	DBG_MSG(IGS_VDB,IGS_VLOW,"operator(3 vertices, temporary="<<treatVertexDataAsTemporary<<")");
        ++_index;

	float 	dist;
	Vec3	in;
	if (!triangleVsLineSegment(v1,v2,v3,_s,_d,_length,dist,in))
		return;

        osg::Vec3 normal = (v2-v1)^(v2-v3);
        normal.normalize();

        float r = dist/_length;


	if (treatVertexDataAsTemporary)
        {
	    _thl.push_back(GeomHit(r,_index-1,in,normal,0.0,NULL,0.0,NULL,0.0,NULL));
        }
        else
        {
            _thl.push_back(GeomHit(r,_index-1,in,normal,0.0,&v1,0.0,&v2,0.0,&v3));
        }
        _hit = true;

    }

    inline void operator()(unsigned int i1,unsigned int i2,unsigned int i3)
    {
	//DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indices)");
	//IGS_PTR(IGS_VDB,IGS_VLOW,vertices_);
	this->operator()((*vertices_)[i1],(*vertices_)[i2],(*vertices_)[i3],false);
    }

    inline void operator()(unsigned int i1)
    {
	//DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indice: 1 int)");
	//IGS_PTR(IGS_VDB,IGS_VLOW,vertices_);
	this->operator()((*vertices_)[i1],false);
    }

    inline void operator()(unsigned int i1,unsigned int i2)
    {
	//DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indice: 2 int)");
	//IGS_PTR(IGS_VDB,IGS_VLOW,vertices_);
	this->operator()((*vertices_)[i1],(*vertices_)[i2],false);
    }


};



}
#endif

