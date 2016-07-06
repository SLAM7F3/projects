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

#ifndef OSG_REFPOLYTOPEISECTOR_H
#define OSG_REFPOLYTOPEISECTOR_H

#include "osg/osgIntersector/intersector.h"
#include "osg/osgIntersector/osg_refpolytope.h"

using namespace osg;


namespace osgIsect
{

   class OSGINTERSECT RefPolytopeIsector : public RefPolytope
   {
      public:
         RefPolytopeIsector() : RefPolytope(){initCache();};
         RefPolytopeIsector(const Polytope &cv) : RefPolytope(cv){initCache();};
         RefPolytopeIsector(const PlaneList &pl) : RefPolytope(pl){initCache();}
         RefPolytopeIsector(const RefPolytopeIsector &polyt) : RefPolytope(polyt)
         {
            _eye=polyt._eye;
            _fwd=polyt._fwd;
            initCache();
         };

         ~RefPolytopeIsector() 
         {
            delete _cachePlanNorm;
            delete _cachePlanPoint;
         }
	
         RefPolytopeIsector operator=(const RefPolytopeIsector& polyt)
         {
            RefPolytope::operator=(polyt);
            _eye=polyt._eye;
            _fwd=polyt._fwd;
            updateCache();
            return polyt;
         }

         void set(const PlaneList &pl){RefPolytope::set(pl);updateCacheSize();}
         void add(const osg::Plane &pl){RefPolytope::add(pl);updateCacheSize();}

         ///create the cache arrays (for nothing)
         ///it is just to avoid a test when using cache... ( if _cache != NULL delete _cache), as it is initialized only once
         void initCache()
         {
            _cachePlanNorm=new Vec3[1];
            _cachePlanPoint=new Vec3[1];
            updateCacheSize();
            updateCacheValues();
         }

         ///allocate new arrays for cache
         void updateCacheSize()
         {
            delete _cachePlanNorm;
            delete _cachePlanPoint;
            _cachePlanNorm=new Vec3[_planeList.size()];
            _cachePlanPoint=new Vec3[_planeList.size()];
         }

         ///update cache values
         void updateCacheValues()
         {
            unsigned int i=0;
            for(PlaneList::const_iterator pitr=_planeList.begin();
                pitr!=_planeList.end();
                ++pitr,i++)
            {
               _cachePlanNorm[i]=pitr->getNormal();
               _cachePlanPoint[i]=-_cachePlanNorm[i]*pitr->asVec4().w();
            }
         }

         void updateCache(){updateCacheSize();updateCacheValues();}


         ///May be used to test if the polytope is valid
         inline bool	valid()
         {
            if (_planeList.size()==0)
               return false;
            return true;
         }

         ///return true if the vertex is inside
         inline bool isInside(const Vec3& v1)const
         {
            unsigned int planNb=_planeList.size();
            unsigned int above=0;
            for (unsigned int i=0;i<planNb;i++)
               if ((v1-_cachePlanPoint[i]) * _cachePlanNorm[i] >= 0.0)
                  above++;
               else
                  break;
            if (above>=planNb)
               return true;
            return false;

         }


         ///return true if at less one vertex is inside
         inline bool isInside(const std::vector<Vec3>& vertices)const
         {
            for(std::vector<Vec3>::const_iterator vitr=vertices.begin();
                vitr != vertices.end();
                ++vitr)
            {
               if (isInside(*vitr))
                  return true;
            }
            return false;

         }


         ///return the number of vertices inside
         inline unsigned long nbInside(const std::vector<Vec3>& vertices)const
         {
            unsigned int planNb=_planeList.size();
            unsigned long in=0;
            for(std::vector<Vec3>::const_iterator vitr=vertices.begin();
                vitr != vertices.end();
                ++vitr)
            {
               unsigned int above=0;
               for (unsigned int i=0;i<planNb;i++)
                  if ((*vitr-_cachePlanPoint[i]) * _cachePlanNorm[i] >= 0.0)
                     above++;
                  else
                     break;
               if (above>=planNb)
                  in++;
            }
            return in;

         }

         /** post multiply a polytope by matrix.*/
         inline void mult(const RefPolytopeIsector& polyt,const Matrix& mat)
         {
            *this=polyt;
            transform(mat);
         }
         /** pre multiply a polytope by matrix.*/
         inline void mult(const Matrix& ,const RefPolytopeIsector&)
         {
            DBG_ERR("pre valid() not implemented yet");
         }

         /** return true if polytope intersects BoundingBox. */
         inline bool intersect(const BoundingBox& bb)
         {
            return contains(bb);
         }

         /** return true if segment intersects BoundingSphere. */
         inline bool intersect(const BoundingSphere& bs)
         {
            return contains(bs);
         }




         inline void setEye(Vec3 eye){_eye=eye;}
         inline const Vec3& getEye()const{return _eye;}
         inline void setForward(Vec3 fwd){_fwd=fwd;}
         inline const Vec3& getForward()const{return _fwd;}

         inline void transform (const osg::Matrix &matrix)
         {
            //DBG_MSG(IGS_TRC,IGS_VLOW,"Polytope transform");
            Polytope::transform(matrix);
            _eye=_eye*matrix;
            _fwd=_fwd*matrix;
            updateCacheValues();

         }

         inline void transformProvidingInverse (const osg::Matrix &matrix)
         {
            DBG_MSG("Polytope transform Inverse NOT TESTED");

            Polytope::transformProvidingInverse(matrix);
            _eye=matrix*_eye;
            _fwd=matrix*_fwd;
            updateCacheValues();
         }
      protected:
         ///the eye coordinates
         Vec3	_eye;
         ///the forward point
         Vec3	_fwd;


         ///the cached plan normals
         Vec3*	_cachePlanNorm;
         ///the cached plan points
         Vec3*	_cachePlanPoint;

   };


}
#endif
