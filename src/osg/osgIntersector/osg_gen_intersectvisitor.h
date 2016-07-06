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
#ifndef OSG_GEN_INTERSECTVISITOR
#define OSG_GEN_INTERSECTVISITOR 1

#include "osg/osgIntersector/intersector.h"


#include <osg/NodeVisitor>
#include <osg/LineSegment>
#include <osg/Geode>
#include <osg/Matrix>
#include <osg/Drawable>

#include <osgUtil/Export>

#include <map>
#include <set>
#include <vector>


namespace osgIsect
{

   template<class Gen> struct IntersectVisitor_trait
   {
   };

   using namespace osg;

   class HitBase
   {
      public:
         HitBase(){};
         HitBase(const HitBase& hit)
         {
            // copy data across.
            _ratio = hit._ratio;
            _nodePath = hit._nodePath;
            _geode = hit._geode;
            _drawable = hit._drawable;
            _matrix = hit._matrix;
            _inverse = hit._inverse;

            _vecIndexList = hit._vecIndexList;
            _primitiveIndex = hit._primitiveIndex;
            _intersectPoint = hit._intersectPoint;
            _intersectNormal = hit._intersectNormal;
         }

         ~HitBase(){};

         HitBase& operator = (const HitBase& hit)
         {
            if (&hit==this) return *this;

            _matrix = hit._matrix;
            _inverse = hit._inverse;

            // copy data across.
            _ratio = hit._ratio;
            _nodePath = hit._nodePath;
            _geode = hit._geode;
            _drawable = hit._drawable;

            _vecIndexList = hit._vecIndexList;
            _primitiveIndex = hit._primitiveIndex;
            _intersectPoint = hit._intersectPoint;
            _intersectNormal = hit._intersectNormal;

            return *this;
         }


         typedef std::vector<int> VecIndexList;
//	typedef GenIndexFunctor<GenGeomIntersect<Gen> > SpecIndexFunctor;

         ///returns the (or one between many if many) intersection point
         const osg::Vec3& getLocalIntersectPoint() const { 
            return _intersectPoint; }
         const osg::Vec3& getLocalIntersectNormal() const { 
            return _intersectNormal; }

         ///returns the (or one between many if many) intersection point
         const osg::Vec3 getWorldIntersectPoint() const { 
            if (_matrix.valid()) return _intersectPoint*(*_matrix); 
            else return _intersectPoint; }
         const osg::Vec3 getWorldIntersectNormal() const
         {
            if (_inverse.valid())
            {
               Vec3 norm = Matrix::transform3x3(*_inverse,_intersectNormal);
               norm.normalize();
               return norm;
            }
            else return _intersectNormal;
         }


         float                           _ratio;
         osg::NodePath                   _nodePath;
         osg::ref_ptr<osg::Geode>        _geode;
         osg::ref_ptr<osg::Drawable>     _drawable;
         osg::ref_ptr<osg::RefMatrix>    _matrix;
         osg::ref_ptr<osg::RefMatrix>    _inverse;

         VecIndexList                    _vecIndexList;
         int                             _primitiveIndex;
         osg::Vec3                       _intersectPoint;
         osg::Vec3                       _intersectNormal;
   };

///FMD: try to template the osgHit
/// => Generic Hit, or GenHit
///it may be templated by LineSegment or Polytope or whatever you want (?!?)
///it will be called 'Gen'
   template <class Gen>
      class GenHit: public HitBase
      {
         public:
            GenHit():HitBase(){};

            GenHit(const GenHit& hit)
               :HitBase(hit),
                _originalGen(hit._originalGen),
                _localGen(hit._localGen){};
            GenHit& operator = (const GenHit& hit)
            {
               HitBase::operator=(hit);
               _originalGen=hit._originalGen;
               _localGen=hit._localGen;
               return *this;
            };

            bool operator < (const GenHit& hit) const
            {
               if (_originalGen<hit._originalGen) return true;
               if (_originalGen>hit._originalGen) return false;
               return _ratio<hit._ratio;
            }

            osg::ref_ptr<Gen>		_originalGen;
            osg::ref_ptr<Gen>		_localGen;
      };


/** Basic visitor for generic collisions of a scene.*/
   template <class Gen>
      class GenIntersectVisitor : public osg::NodeVisitor
      {
         public:





            GenIntersectVisitor();
            virtual ~GenIntersectVisitor();

            void reset();

            /** Add a Gen to use for intersection testing during scene traversal.*/
            void addGen(Gen* gen);

            typedef GenHit<Gen> Hit;
            typedef std::vector<Hit> HitList;
            typedef std::map<Gen*,HitList > HitListMap;

            //void sortHitList(Gen* gen){std::sort(_genHitList[gen].begin(),_genHitList[gen].end());}
            HitList& getHitList(Gen* gen) { return _genHitList[gen]; }
            int getNumHits(Gen* gen) { return _genHitList[gen].size(); }


            bool hits();

            virtual void apply(osg::Node&);
            virtual void apply(osg::Geode& node);
            virtual void apply(osg::Billboard& node);

            virtual void apply(osg::Group& node);
            virtual void apply(osg::Transform& node);
            virtual void apply(osg::Switch& node);
            virtual void apply(osg::LOD& node);

         protected:

            class GenIntersectState : public osg::Referenced
            {
               public:

                  GenIntersectState();

                  osg::ref_ptr<osg::RefMatrix> _matrix;
                  osg::ref_ptr<osg::RefMatrix> _inverse;

                  typedef std::pair<osg::ref_ptr<Gen>,osg::ref_ptr<Gen> > GenPair;
                  typedef std::vector< GenPair > GenList;
                  GenList _genList;

                  typedef unsigned int GenMask;
                  typedef std::vector<GenMask> GenMaskStack;
                  GenMaskStack _genMaskStack;

                  bool isCulled(const osg::BoundingSphere& bs,GenMask& genMaskOut);
                  bool isCulled(const osg::BoundingBox& bb,GenMask& genMaskOut);

                  void addPair(Gen* first,Gen* second)
                  {
                     _genList.push_back(GenPair(first,second));
                  }

               protected:

                  ~GenIntersectState();

            };

            bool intersect(osg::Drawable& gset);

            void pushMatrix(const osg::Matrix& matrix);
            void popMatrix();

            bool enterNode(osg::Node& node);
            void leaveNode();

            typedef std::vector<osg::ref_ptr<GenIntersectState> > GenIntersectStateStack;

            GenIntersectStateStack         _intersectStateStack;

            osg::NodePath           _nodePath;
            HitListMap       	_genHitList;
      };


}

#include "osg/osgIntersector/osg_gen_intersectvisitor.hxx"
#endif
