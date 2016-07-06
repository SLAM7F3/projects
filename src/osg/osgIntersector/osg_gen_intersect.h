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
#ifndef OSG_GEN_INTERSECT_H
#define OSG_GEN_INTERSECT_H

#include "osg/osgIntersector/intersector.h"

#include <osg/Drawable>
#include <osg/Geometry>


using namespace osg;

namespace osgIsect
{


template<class Gen> struct Intersect_trait
{
};



struct GeomHit
{
//    GeomHit(const GeomHit &hit){};
    GeomHit& operator=(const GeomHit & hit)
	{
	_ratio=hit._ratio;
        _index=hit._index;
	_nbPt=hit._nbPt;
	_ipt=hit._ipt;
        _normal=hit._normal;
        _r1=hit._r1;
        _v1=hit._v1;
        _r2=hit._r2;
        _v2=hit._v2;
        _r3=hit._r3;
        _v3=hit._v3;

	return *this;
	}
    GeomHit(float ratio,unsigned int index,const Vec3& ipt,const osg::Vec3& normal, float r1, const osg::Vec3* v1, float r2, const osg::Vec3* v2, float r3, const osg::Vec3* v3):
    	_ratio(ratio),
        _index(index),
	_nbPt(3),
	_ipt(ipt),
        _normal(normal),
        _r1(r1),
        _v1(v1),
        _r2(r2),
        _v2(v2),
        _r3(r3),
        _v3(v3)
	 {}
    GeomHit(float ratio,unsigned int index,const Vec3& ipt,float r1, const osg::Vec3* v1, float r2, const osg::Vec3* v2):
    	_ratio(ratio),
        _index(index),
	_nbPt(2),
	_ipt(ipt),
        _r1(r1),
        _v1(v1),
        _r2(r2),
        _v2(v2),
	_r3(0.0),
	_v3(NULL)
	{}
    GeomHit(float ratio,unsigned int index,const Vec3& ipt,float r1, const osg::Vec3* v1):
    	_ratio(ratio),
        _index(index),
	_nbPt(1),
	_ipt(ipt),
        _r1(r1),
        _v1(v1),
	_r2(0.0),
	_v2(NULL),
	_r3(0.0),
	_v3(NULL)

	{}


    float		_ratio; //distance from eye
    unsigned int        _index;
    unsigned int 	_nbPt;	//number of points
    osg::Vec3	_ipt;	//intersection point (the exact one if exist, the best if many (average, center of collision...))
    osg::Vec3     _normal;//may not be valid (for geometries having 1 or 2 points)
    float               _r1;
    const osg::Vec3*    _v1;
    float               _r2;
    const osg::Vec3*    _v2;
    float               _r3;
    const osg::Vec3*    _v3;

};




template<class Gen> class GenGeomIntersect
{
public:
    GenGeomIntersect()
    {
    	vertices_=NULL;
    }

    GenGeomIntersect(const Gen& gen,float ratio=FLT_MAX)
    {
        set(gen,NULL,ratio);
    }

    osg::ref_ptr<Gen> _gen;

    int _index;
    float _ratio;
    bool _hit;

    osg::Vec3Array* vertices_;

	bool triangleVsLineSegment(const Vec3& v1,const Vec3& v2,const Vec3& v3,const Vec3& s,const Vec3 &d,const float &l,float &dist,Vec3& in)
	{
		if (v1==v2 || v2==v3 || v1==v3) return false;
	
		Vec3 v12 = v2-v1;
		Vec3 n12 = v12^d;
		float ds12 = (s-v1)*n12;
		float d312 = (v3-v1)*n12;
		if (d312>=0.0f)
		{
			if (ds12<0.0f) return false;
			if (ds12>d312) return false;
		}
		else                     // d312 < 0
		{
			if (ds12>0.0f) return false;
			if (ds12<d312) return false;
		}
	
		Vec3 v23 = v3-v2;
		Vec3 n23 = v23^d;
		float ds23 = (s-v2)*n23;
		float d123 = (v1-v2)*n23;
		if (d123>=0.0f)
		{
			if (ds23<0.0f) return false;
			if (ds23>d123) return false;
		}
		else                     // d123 < 0
		{
			if (ds23>0.0f) return false;
			if (ds23<d123) return false;
		}
	
		Vec3 v31 = v1-v3;
		Vec3 n31 = v31^d;
		float ds31 = (s-v3)*n31;
		float d231 = (v2-v3)*n31;
		if (d231>=0.0f)
		{
			if (ds31<0.0f) return false;
			if (ds31>d231) return false ;
		}
		else                     // d231 < 0
		{
			if (ds31>0.0f) return false;
			if (ds31<d231) return false;
		}
	
	
		float r3;
		if (ds12==0.0f) r3=0.0f;
		else if (d312!=0.0f) r3 = ds12/d312;
		else return false; // the triangle and the line must be parallel intersection.
	
		float r1;
		if (ds23==0.0f) r1=0.0f;
		else if (d123!=0.0f) r1 = ds23/d123;
		else return false; // the triangle and the line must be parallel intersection.
	
		float r2;
		if (ds31==0.0f) r2=0.0f;
		else if (d231!=0.0f) r2 = ds31/d231;
		else return false; // the triangle and the line must be parallel intersection.
	
		float total_r = (r1+r2+r3);
		if (total_r!=1.0f)
		{
		if (total_r==0.0f) return false; // the triangle and the line must be parallel intersection.
		float inv_total_r = 1.0f/total_r;
		r1 *= inv_total_r;
		r2 *= inv_total_r;
		r3 *= inv_total_r;
		}
	
		in = v1*r1+v2*r2+v3*r3;
	
		dist = (in-s)*d;
	
		if (dist<0.0f) return false;
		if (dist>l) return false;
		return true;
	}	
    
    //too slow? typedef std::vector<float,GeomHit> GeomHitList;
    typedef std::vector<GeomHit> GeomHitList;

    GeomHitList _thl;


    void set(const Gen& gen,osg::Array* vertices,float ratio=FLT_MAX)
    {
    	vertices_=(osg::Vec3Array*)vertices;
        _gen=new Gen(gen);
        _hit=false;
        _index = 0;
        _ratio = ratio;

    }

    //point, LineStart,LineVector
    inline float dist_pt_line(const Vec3& pt,const Vec3& ls,const Vec3& lv)
    {
    	Vec3	v=lv;
	Vec3	w=pt-ls;

	double	c1=w*v;
	double	c2=v*v;
	double	b=c1/c2;
	Vec3	pb=ls + v*b;
	return pt*pb;
    }

    //point, LineStart,LineEnd
    inline float dist_pt_seg(const Vec3& pt,const Vec3& ls,const Vec3& le)
    {
    	Vec3	v=le-ls;
	Vec3	w=pt-ls;

	double	c1=w*v;
	if (c1<=0.0)
		return pt*ls;
	double	c2=v*v;
	if (c2<=0.0)
		return pt*le;

	double	b=c1/c2;
	Vec3	pb=ls + v*b;
	return pt*pb;

    }
/*
    //intersection with a point
    inline void operator()(const Vec3& v1, bool treatVertexDataAsTemporary)
    {
    	DBG_WAR(IGS_VDB,IGS_VLOW,"Intersect between Generic and Point is not defined.");
    }

    //intersection with a segment
    inline void operator()(const Vec3& v1,const Vec3& v2, bool treatVertexDataAsTemporary)
    {
    	DBG_WAR(IGS_VDB,IGS_VLOW,"Intersect between Generic and Segment is not defined.");
    }

    //intersection with a triangle (polygons are splited into triangles)
    inline void operator()(const Vec3& v1,const Vec3& v2,const Vec3& v3, bool treatVertexDataAsTemporary)
    {
    	DBG_WAR(IGS_VDB,IGS_VLOW,"Intersect between Generic and triangle is not defined.");
    }


    inline void operator()(unsigned int i1)
    {
	DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indice: 1 int)");
	DBG_PTR(IGS_VDB,IGS_VLOW,vertices_);
	this->operator()((*vertices_)[i1],false);
    }

    inline void operator()(unsigned int i1,unsigned int i2)
    {
	DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indice: 2 int)");
	DBG_PTR(IGS_VDB,IGS_VLOW,vertices_);
	this->operator()((*vertices_)[i1],(*vertices_)[i2],false);
    }

    inline void operator()(unsigned int i1,unsigned int i2,unsigned int i3)
    {
	DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indices)");
	DBG_PTR(IGS_VDB,IGS_VLOW,vertices_);
	this->operator()((*vertices_)[i1],(*vertices_)[i2],(*vertices_)[i3],false);
    }
*/

};



}
#endif
