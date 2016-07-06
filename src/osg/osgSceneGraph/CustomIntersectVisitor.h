// ==========================================================================
// Header file for CustomIntersectVisitor class
// ==========================================================================
// Last updated on 11/13/06
// ==========================================================================

#ifndef OSGUTIL_CUSTOMINTERSECTVISITOR
#define OSGUTIL_CUSTOMINTERSECTVISITOR 1

#include <osg/Geode>
#include <osg/NodeVisitor>
#include <osg/LineSegment>
#include <osg/LOD>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/PagedLOD>
#include <osg/Transform>

#include <osgUtil/Export>
#include <osgUtil/IntersectVisitor>

#include <map>
#include <set>
#include <vector>

#include "osg/osgSceneGraph/MyNodeVisitor.h"

namespace osgUtil {


/** Basic visitor for ray based collisions of a scene.*/

   class OSGUTIL_EXPORT CustomIntersectVisitor : public MyNodeVisitor
   {
      public:

         CustomIntersectVisitor();
         virtual ~CustomIntersectVisitor();

         void reset();
        
         /** Add a line segment to use for intersection testing during
          * scene traversal.  Note, a maximum of 32 line segments can
          * be added to a IntersectVistor, adding more than this will
          * result in warning being emitted to the console and the
          * excess segments being ignored.*/
         void addLineSegment(osg::LineSegment* seg);

         typedef std::vector<Hit> HitList;
         typedef std::map<const osg::LineSegment*,HitList > 
         LineSegmentHitListMap;

         HitList& getHitList(const osg::LineSegment* seg) { 
            return _segHitList[seg]; }

         int getNumHits(const osg::LineSegment* seg) { 
            return _segHitList[seg].size(); }
        
         LineSegmentHitListMap& getSegHitList() { return _segHitList; }

         bool hits();

         enum LODSelectionMode
         {
            USE_HIGHEST_LEVEL_OF_DETAIL,
            USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION
         };
        
         void setLODSelectionMode(LODSelectionMode mode) { 
            _lodSelectionMode = mode; }
         LODSelectionMode getLODSelectionMode() const { 
            return _lodSelectionMode; }

         /** Set the eye point in local coordinates.  This is a
          * pseudo-EyePoint for billboarding and LOD purposes.  It is
          * copied from the Start point of the most-recently-added
          * segment of the intersection ray set
          * (IntersectState::_segList). */
         void setEyePoint(const osg::Vec3& eye) { _pseudoEyePoint = eye; }

         virtual osg::Vec3 getEyePoint() const;


         /** Get the distance from a point to the eye point, distance
          * value in local coordinate system.  This is calculated
          * using the pseudo-EyePoint (above) when doing LOD
          * calculcations. */
         virtual float getDistanceToEyePoint(
            const osg::Vec3& pos, bool withLODScale) const;

// Traversal member functions:

         virtual void apply(osg::Node&);
         virtual void apply(osg::Geode& node);
         virtual void apply(osg::Billboard& node);
         virtual void apply(osg::Group& node);
         virtual void apply(osg::Transform& node);
         virtual void apply(osg::MatrixTransform& node);
         virtual void apply(osg::Switch& node);
         virtual void apply(osg::LOD& node);
         virtual void apply(osg::PagedLOD& node);

      protected:

         class IntersectState : public osg::Referenced
         {
            public:

               IntersectState();

               osg::ref_ptr<osg::RefMatrix> _view_matrix;
               osg::ref_ptr<osg::RefMatrix> _view_inverse;
               osg::ref_ptr<osg::RefMatrix> _model_matrix;
               osg::ref_ptr<osg::RefMatrix> _model_inverse;

               typedef std::pair<osg::ref_ptr<osg::LineSegment>,
                  osg::ref_ptr<osg::LineSegment> >   LineSegmentPair;
               typedef std::vector< LineSegmentPair >                                              LineSegmentList;
               LineSegmentList _segList;

               typedef unsigned int LineSegmentMask;
               typedef std::vector<LineSegmentMask> LineSegmentMaskStack;
               LineSegmentMaskStack _segmentMaskStack;

               bool isCulled(const osg::BoundingSphere& bs,
                             LineSegmentMask& segMaskOut);
               bool isCulled(const osg::BoundingBox& bb,
                             LineSegmentMask& segMaskOut);

               void addLineSegment(osg::LineSegment* seg);

            protected:

               ~IntersectState();

         };

         bool intersect(osg::Drawable& gset);

         void pushMatrix(osg::RefMatrix* matrix, 
                         osg::Transform::ReferenceFrame rf);
         void popMatrix();

         bool enterNode(osg::Node& node);
         void leaveNode();

         typedef std::vector<osg::ref_ptr<IntersectState> > 
         IntersectStateStack;
        
         IntersectStateStack         _intersectStateStack;

         LineSegmentHitListMap       _segHitList;
        
         LODSelectionMode            _lodSelectionMode;
         osg::Vec3                   _pseudoEyePoint;
   };


}

#endif

