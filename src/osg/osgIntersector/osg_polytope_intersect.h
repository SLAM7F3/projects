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
#ifndef OSG_POLYTOPE_INTERSECT_H
#define OSG_POLYTOPE_INTERSECT_H

#include "osg/osgIntersector/intersector.h"
#include <osg/Drawable>
#include <osg/Geometry>

#include "osg/osgIntersector/osg_refpolytopeisector.h"
#include "osg/osgIntersector/osg_gen_intersect.h"

using namespace osg;


namespace osgIsect
{

   class OSGINTERSECT PolytopeIntersect : public GenGeomIntersect<RefPolytopeIsector>
   {
      public:




         void set(const RefPolytopeIsector& gen,osg::Array* vertices,float ratio=FLT_MAX)
         {
            GenGeomIntersect<RefPolytopeIsector>::set(gen,vertices,ratio);

         }

         //intersection with a point
         inline void operator()(const Vec3& v1, bool treatVertexDataAsTemporary)
         {
            ++_index;

            if (!_gen->contains(v1))
               return;

            if (treatVertexDataAsTemporary)
            {
               _thl.push_back(GeomHit((v1-_gen->getEye()).length(),_index-1,v1,1.0,NULL));
            }
            else
            {
               _thl.push_back(GeomHit((v1-_gen->getEye()).length(),_index-1,v1,1.0,&v1));
            }
            _hit = true;



            return;

         }

         inline bool getPlaneSegIsect(const Plane& plane,const Vec3 &seg1,const Vec3 &seg2,const Vec3 &eye,Vec3 &isec,Vec3 &in)
         {
            Vec3	pn=plane.getNormal();
            Vec3	pp=-pn*plane.asVec4().w();
            pp+=pn*0.001;	//to force the colliding point to be in the polytope...
            Vec3	u = seg2 - seg1;
            Vec3	w = seg1 - pp;

            float     D = pn*u;
            float     N = -pn*w;

            if (fabs(D) < 0.0000001) {          // segment is parallel to plane
               if (N == 0)                     // segment lies in plane
               {
                  if ((seg1-eye).length() < (seg2-eye).length() )
                  {
                     //take the nearest point
                     isec=seg1;
                     in=seg2;
                  }
                  else
                  {
                     isec=seg2;
                     in=seg1;
                  }
                  return true;
               }
               else
                  return false;                   // no intersection
            }
            // they are not parallel
            // compute intersect param
            float sI = N / D;
            if (sI < 0.0 || sI > 1.0)
               return false;                       // no intersection
            isec = seg1 + u* sI;                 // compute segment intersect point

            //find the pt that is IN the polytope
            if ((seg1-pp) * pn>= 0.0)
               in=seg1;
            else
               in=seg2;
            return true;
         }


         bool isectSegPolytope(const Vec3& v1,const Vec3& v2,Vec3 &in)
         {
            std::vector<Vec3> vertices;
            vertices.push_back(v1);
            vertices.push_back(v2);

            //polytope/line intersection points
            Vec3 		isect[3]={v1,v2};
            unsigned int 	isect_i=0;

            if (_gen->nbInside(vertices)!=2)
               //if 0 or 1 vertex is in the polytope
            {
               //check if line intersect with any plan of polytope
               RefPolytope::PlaneList planes=_gen->getPlaneList();
               for (RefPolytope::PlaneList::iterator plane=planes.begin();
                    plane!=planes.end() && isect_i<2;
                    plane++)
               {
                  Vec3	pisec,pin;
                  if (getPlaneSegIsect(*plane,v1,v2,_gen->getEye(),pisec,pin))
                  {
				//if yes, check if it is in the polytope
                     if (_gen->isInside(pisec))
                     {
                        //if yes, consider it as a valid intersection point
                        isect[isect_i]=pisec;
                        isect[++isect_i]=pin;
                     }
                  }
               }
               if (isect_i==0)
                  return false;
            }

            //contact point is the nearest
            if ((isect[0]-_gen->getEye()).length()<(isect[1]-_gen->getEye()).length())
               in=isect[0];
            else
               in=isect[1];
            return true;
         }
         //intersection with a segment
         inline void operator()(const Vec3& v1,const Vec3& v2, bool treatVertexDataAsTemporary)
         {
            ++_index;

            Vec3 in;
            if (!isectSegPolytope(v1,v2,in))
               return;

            if (treatVertexDataAsTemporary)
            {
               _thl.push_back(GeomHit((in-_gen->getEye()).length(),_index-1,in,1.0,NULL,1.0,NULL));
            }
            else
            {
               _thl.push_back(GeomHit((in-_gen->getEye()).length(),_index-1,in,1.0,&v1,1.0,&v2));
            }
            _hit = true;



            return;

         }

         typedef std::pair<float,Vec3> FloatVec3;
         inline static bool dist_ordering(const FloatVec3& v1,const FloatVec3& v2) { return v1.first < v2.first ;}


         //   bool intersect(const Vec3& v1,const Vec3& v2,const Vec3& v3,float& r)
         inline void operator()(const Vec3& v1,const Vec3& v2,const Vec3& v3, bool treatVertexDataAsTemporary)
         {
            ++_index;

            std::vector<FloatVec3> isect;

            Vec3	eye=_gen->getEye();
            Vec3 	in;
            if (isectSegPolytope(v1,v2,in))
               isect.push_back(FloatVec3((in-eye).length(),in));
            if (isectSegPolytope(v1,v3,in))
               isect.push_back(FloatVec3((in-eye).length(),in));
            if (isectSegPolytope(v2,v3,in))
               isect.push_back(FloatVec3((in-eye).length(),in));
            if (!isect.size())
            {
               //here, 2 cases:
               //- all points are inside
               std::vector<Vec3> vertices;
               vertices.push_back(v1);
               vertices.push_back(v2);
               vertices.push_back(v3);

               if (_gen->nbInside(vertices)==3)
               {
                  isect.push_back(FloatVec3((v1-eye).length(),v1));
                  isect.push_back(FloatVec3((v2-eye).length(),v2));
                  isect.push_back(FloatVec3((v3-eye).length(),v3));
               }
               else
                  //or, all points are around
               {
                  //intersect triangle with ray eye+forward
                  //if hit, then the triangle polytope section is all in the triangle.
                  //else, the triangle doesn't hit.
                  float 	dist;
                  Vec3	in;
                  Vec3	fwd=_gen->getForward()-eye;
                  float 	fwdl=fwd.length();
                  fwd/=fwdl;
                  if (!triangleVsLineSegment(v1,v2,v3,eye,fwd,fwdl,dist,in))
                     return;
                  isect.push_back(FloatVec3(dist,in));
               }
            }

            if (isect.size())
               std::sort(isect.begin(), isect.end(), dist_ordering);
            in=(isect.begin())->second;
            float dist=(isect.begin())->first;


            Vec3 normal = (v2-v1)^(v3-v1);
            normal.normalize();

            if (treatVertexDataAsTemporary)
            {
               _thl.push_back(GeomHit(GeomHit(dist,_index-1,in,normal,1.0,NULL,1.0,NULL,1.0,NULL)));
            }
            else
            {
               _thl.push_back(GeomHit(dist,_index-1,in,normal,1.0,&v1,1.0,&v2,1.0,&v3));
            }
            _hit = true;



            return;
         }

         inline void operator()(unsigned int i1,unsigned int i2,unsigned int i3)
         {
            //DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indices)");
            //DBG_PTR(IGS_VDB,IGS_VLOW,vertices_);
            this->operator()((*vertices_)[i1],(*vertices_)[i2],(*vertices_)[i3],false);
         }

         inline void operator()(unsigned int i1)
         {
            //DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indice: 1 int)");
            //DBG_PTR(IGS_VDB,IGS_VLOW,vertices_);
            this->operator()((*vertices_)[i1],false);
         }

         inline void operator()(unsigned int i1,unsigned int i2)
         {
            //DBG_MSG(IGS_VDB,IGS_VLOW,"operator(indice: 2 int)");
            //DBG_PTR(IGS_VDB,IGS_VLOW,vertices_);
            this->operator()((*vertices_)[i1],(*vertices_)[i2],false);
         }


   };



}
#endif

