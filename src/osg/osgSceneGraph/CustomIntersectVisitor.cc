// ==========================================================================
// CustomIntersectVisitor class
// ==========================================================================
// Last updated on 11/13/06
// ==========================================================================

#include <osgUtil/IntersectVisitor>

#include <osg/Billboard>
#include <osg/CameraNode>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Projection>
#include <osg/TriangleFunctor>
#include <osg/io_utils>

#include <float.h>
#include <algorithm>
#include <map>
#include <iostream>

#include "osg/osgSceneGraph/CustomIntersectVisitor.h"

using namespace osg;
using namespace osgUtil;
using std::cout;
using std::endl;

CustomIntersectVisitor::IntersectState::IntersectState()
{
   _segmentMaskStack.push_back(0xffffffff);
}


CustomIntersectVisitor::IntersectState::~IntersectState()
{
}


bool CustomIntersectVisitor::IntersectState::isCulled(const BoundingSphere& bs,LineSegmentMask& segMaskOut)
{
   bool hit = false;
   LineSegmentMask mask = 0x00000001;
   segMaskOut = 0x00000000;
   LineSegmentMask segMaskIn = _segmentMaskStack.back();
   for(IntersectState::LineSegmentList::iterator sitr=_segList.begin();
       sitr!=_segList.end();
       ++sitr)
   {
      if ((segMaskIn & mask) && (sitr->second)->intersect(bs))
      {
         segMaskOut = segMaskOut| mask;
         hit = true;
      }
      mask = mask << 1;
   }
   return !hit;
}

bool CustomIntersectVisitor::IntersectState::isCulled(const BoundingBox& bb,LineSegmentMask& segMaskOut)
{
   bool hit = false;
   LineSegmentMask mask = 0x00000001;
   segMaskOut = 0x00000000;
   LineSegmentMask segMaskIn = _segmentMaskStack.back();
   for(IntersectState::LineSegmentList::iterator sitr=_segList.begin();
       sitr!=_segList.end();
       ++sitr)
   {
      if ((segMaskIn & mask) && (sitr->second)->intersect(bb))
      {
         segMaskOut = segMaskOut| mask;
         hit = true;
      }
      mask = mask << 1;
   }
   return !hit;
}

void CustomIntersectVisitor::IntersectState::addLineSegment(osg::LineSegment* seg)
{
   // create a new segment transformed to local coordintes.
   LineSegment* ns = new LineSegment;

   if (_model_inverse.valid()) 
   {
      if (_view_inverse.valid())
      {
         osg::Matrix matrix = (*(_view_inverse)) * (*(_model_inverse));
         ns->mult(*seg,matrix);
      }
      else
      { 
         ns->mult(*seg,*(_model_inverse));
      }
   }
   else if (_view_inverse.valid())
   {
      ns->mult(*seg,*(_view_inverse));
   }
   else
   {
      *ns = *seg;
   }
    
   _segList.push_back(LineSegmentPair(seg,ns));
}


CustomIntersectVisitor::CustomIntersectVisitor()
{

   // overide the default node visitor mode.
   setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    
   // Initialize eyepoint to 0,0,0
   setEyePoint(Vec3(0.0f,0.0f,0.0f));

   setLODSelectionMode(USE_HIGHEST_LEVEL_OF_DETAIL); // original CustomIntersectVisitor behavior
   //setLODSelectionMode(USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION);

   reset();
}


CustomIntersectVisitor::~CustomIntersectVisitor()
{
}


void CustomIntersectVisitor::reset()
{
   _intersectStateStack.clear();

   // create a empty IntersectState on the the intersectStateStack.
   _intersectStateStack.push_back(new IntersectState);

   _segHitList.clear();

}

float CustomIntersectVisitor::getDistanceToEyePoint(const Vec3& pos, bool /*withLODScale*/) const
{
   if (_lodSelectionMode==USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION)
   {
      // osg::notify(osg::NOTICE)<<"CustomIntersectVisitor::getDistanceToEyePoint)"<<(pos-getEyePoint()).length()<<std::endl;
      // LODScale is not available to CustomIntersectVisitor, so we ignore the withLODScale argument
      //if (withLODScale) return (pos-getEyePoint()).length()*getLODScale();
      //else return (pos-getEyePoint()).length();
      return (pos-getEyePoint()).length();
   }
   else
   {
      return 0.0f;
   }
}


bool CustomIntersectVisitor::hits()
{
   for(LineSegmentHitListMap::iterator itr = _segHitList.begin();
       itr != _segHitList.end();
       ++itr)
   {
      if (!(itr->second.empty())) return true;
   }
   return false;
}

osg::Vec3 CustomIntersectVisitor::getEyePoint() const
{
   const IntersectState* cis = _intersectStateStack.empty() ? 0 : _intersectStateStack.back().get();
   if (cis && (cis->_model_inverse.valid() || cis->_view_inverse.valid()))
   {
        
      osg::Vec3 eyePoint = _pseudoEyePoint;
      if (cis->_view_inverse.valid()) eyePoint = eyePoint * (*(cis->_view_inverse));
      if (cis->_model_inverse.valid()) eyePoint = eyePoint * (*(cis->_model_inverse));
        
      //osg::notify(osg::NOTICE)<<"CustomIntersectVisitor::getEyePoint()"<<eyePoint<<std::endl;

      return eyePoint;
   }
   else
   {
      return _pseudoEyePoint;
   }
}

void CustomIntersectVisitor::addLineSegment(LineSegment* seg)
{
   if (!seg) return;
    
   if (!seg->valid())
   {
      notify(WARN)<<"Warning: invalid line segment passed to CustomIntersectVisitor::addLineSegment(..)"<<std::endl;
      notify(WARN)<<"         "<<seg->start()<<" "<<seg->end()<<" segment ignored.."<< std::endl;
      return;
   }
    
   IntersectState* cis = _intersectStateStack.back().get();
    
   if (cis->_segList.size()>=32)
   {
      notify(WARN)<<"Warning: excessive number of line segmenets passed to CustomIntersectVisitor::addLineSegment(..), maximum permitted is 32 line segments."<<std::endl;
      notify(WARN)<<"         "<<seg->start()<<" "<<seg->end()<<" segment ignored.."<< std::endl;
      return;
   }

   setEyePoint(seg->start()); // set start of line segment to be pseudo EyePoint for billboarding and LOD purposes

   // first check to see if segment has already been added.
   for(IntersectState::LineSegmentList::iterator itr = cis->_segList.begin();
       itr != cis->_segList.end();
       ++itr)
   {
      if (itr->first == seg) return;
   }

   cis->addLineSegment(seg);

}


void CustomIntersectVisitor::pushMatrix(RefMatrix* matrix, osg::Transform::ReferenceFrame rf)
{
   IntersectState* nis = new IntersectState;

   IntersectState* cis = _intersectStateStack.back().get();

   if (rf == osg::Transform::RELATIVE_RF)
   {
      // share the original view matrix
      nis->_view_matrix = cis->_view_matrix;
      nis->_view_inverse = cis->_view_inverse;
        
      // set up new model matrix
      nis->_model_matrix = matrix;
      if (cis->_model_matrix.valid())
      {
         nis->_model_matrix->postMult(*(cis->_model_matrix));
      }

      RefMatrix* inverse_world = new RefMatrix;
      inverse_world->invert(*(nis->_model_matrix));
      nis->_model_inverse = inverse_world;
   }
   else
   {
      // set a new view matrix
      nis->_view_matrix = matrix;

      RefMatrix* inverse_world = new RefMatrix;
      inverse_world->invert(*(nis->_view_matrix));
      nis->_view_inverse = inverse_world;

      // model matrix now blank.
      nis->_model_matrix = 0;
      nis->_model_inverse = 0;
   }


   IntersectState::LineSegmentMask segMaskIn = cis->_segmentMaskStack.back();
   IntersectState::LineSegmentMask mask = 0x00000001;
   for(IntersectState::LineSegmentList::iterator sitr=cis->_segList.begin();
       sitr!=cis->_segList.end();
       ++sitr)
   {
      if ((segMaskIn & mask))
      {
         nis->addLineSegment(sitr->first.get());
      }
      mask = mask << 1;
   }

   _intersectStateStack.push_back(nis);
}


void CustomIntersectVisitor::popMatrix()
{
   if (!_intersectStateStack.empty())
   {
      _intersectStateStack.pop_back();
   }
}


bool CustomIntersectVisitor::enterNode(Node& node)
{
   const BoundingSphere& bs = node.getBound();
   if (bs.valid() && node.isCullingActive())
   {
      IntersectState* cis = _intersectStateStack.back().get();
      IntersectState::LineSegmentMask sm=0xffffffff;
      if (cis->isCulled(bs,sm)) return false;
      cis->_segmentMaskStack.push_back(sm);
      return true;
   }
   else
   {
      IntersectState* cis = _intersectStateStack.back().get();
      if (!cis->_segmentMaskStack.empty()) 
         cis->_segmentMaskStack.push_back(cis->_segmentMaskStack.back());
      else
         cis->_segmentMaskStack.push_back(0xffffffff);
      return true;
   }
}


void CustomIntersectVisitor::leaveNode()
{
   IntersectState* cis = _intersectStateStack.back().get();
   cis->_segmentMaskStack.pop_back();
}



struct TriangleHit
{
      TriangleHit(unsigned int index, const osg::Vec3& normal, float r1, const osg::Vec3* v1, float r2, const osg::Vec3* v2, float r3, const osg::Vec3* v3):
         _index(index),
         _normal(normal),
         _r1(r1),
         _v1(v1),        
         _r2(r2),
         _v2(v2),        
         _r3(r3),
         _v3(v3) {}

      unsigned int        _index;
      const osg::Vec3     _normal;
      float               _r1;
      const osg::Vec3*    _v1;        
      float               _r2;
      const osg::Vec3*    _v2;        
      float               _r3;
      const osg::Vec3*    _v3;        
};


struct TriangleIntersect
{
      osg::ref_ptr<LineSegment> _seg;

      Vec3    _s;
      Vec3    _d;
      float   _length;

      int _index;
      float _ratio;
      bool _hit;
    
    

      typedef std::multimap<float,TriangleHit> TriangleHitList;
    
      TriangleHitList _thl;

      TriangleIntersect()
      {
      }

      TriangleIntersect(const LineSegment& seg,float ratio=FLT_MAX)
      {
         set(seg,ratio);
      }
    
      void set(const LineSegment& seg,float ratio=FLT_MAX)
      {
         _seg=new LineSegment(seg);
         _hit=false;
         _index = 0;
         _ratio = ratio;

         _s = _seg->start();
         _d = _seg->end()-_seg->start();
         _length = _d.length();
         _d /= _length;
      }

      //   bool intersect(const Vec3& v1,const Vec3& v2,const Vec3& v3,float& r)
      inline void operator () (const Vec3& v1,const Vec3& v2,const Vec3& v3, bool treatVertexDataAsTemporary)
      {
         ++_index;

         if (v1==v2 || v2==v3 || v1==v3) return;

         Vec3 v12 = v2-v1;
         Vec3 n12 = v12^_d;
         float ds12 = (_s-v1)*n12;
         float d312 = (v3-v1)*n12;
         if (d312>=0.0f)
         {
            if (ds12<0.0f) return;
            if (ds12>d312) return;
         }
         else                     // d312 < 0
         {
            if (ds12>0.0f) return;
            if (ds12<d312) return;
         }

         Vec3 v23 = v3-v2;
         Vec3 n23 = v23^_d;
         float ds23 = (_s-v2)*n23;
         float d123 = (v1-v2)*n23;
         if (d123>=0.0f)
         {
            if (ds23<0.0f) return;
            if (ds23>d123) return;
         }
         else                     // d123 < 0
         {
            if (ds23>0.0f) return;
            if (ds23<d123) return;
         }

         Vec3 v31 = v1-v3;
         Vec3 n31 = v31^_d;
         float ds31 = (_s-v3)*n31;
         float d231 = (v2-v3)*n31;
         if (d231>=0.0f)
         {
            if (ds31<0.0f) return;
            if (ds31>d231) return;
         }
         else                     // d231 < 0
         {
            if (ds31>0.0f) return;
            if (ds31<d231) return;
         }
        

         float r3;
         if (ds12==0.0f) r3=0.0f;
         else if (d312!=0.0f) r3 = ds12/d312;
         else return; // the triangle and the line must be parallel intersection.
        
         float r1;
         if (ds23==0.0f) r1=0.0f;
         else if (d123!=0.0f) r1 = ds23/d123;
         else return; // the triangle and the line must be parallel intersection.
        
         float r2;
         if (ds31==0.0f) r2=0.0f;
         else if (d231!=0.0f) r2 = ds31/d231;
         else return; // the triangle and the line must be parallel intersection.

         float total_r = (r1+r2+r3);
         if (total_r!=1.0f)
         {
            if (total_r==0.0f) return; // the triangle and the line must be parallel intersection.
            float inv_total_r = 1.0f/total_r;
            r1 *= inv_total_r;
            r2 *= inv_total_r;
            r3 *= inv_total_r;
         }
        
         Vec3 in = v1*r1+v2*r2+v3*r3;
         if (!in.valid())
         {
            osg::notify(WARN)<<"Warning:: Picked up error in TriangleIntersect"<<std::endl;
            osg::notify(WARN)<<"   ("<<v1<<",\t"<<v2<<",\t"<<v3<<")"<<std::endl;
            osg::notify(WARN)<<"   ("<<r1<<",\t"<<r2<<",\t"<<r3<<")"<<std::endl;
            return;
         }

         float d = (in-_s)*_d;

         if (d<0.0f) return;
         if (d>_length) return;

         osg::Vec3 normal = v12^v23;
         normal.normalize();

         float r = d/_length;

        
         if (treatVertexDataAsTemporary)
         {
            _thl.insert(std::pair<const float,TriangleHit>(r,TriangleHit(_index-1,normal,r1,0,r2,0,r3,0)));
         }
         else
         {
            _thl.insert(std::pair<const float,TriangleHit>(r,TriangleHit(_index-1,normal,r1,&v1,r2,&v2,r3,&v3)));
         }
         _hit = true;

      }

};

bool CustomIntersectVisitor::intersect(Drawable& drawable)
{
   bool hitFlag = false;

   IntersectState* cis = _intersectStateStack.back().get();

   const BoundingBox& bb = drawable.getBound();

   for(IntersectState::LineSegmentList::iterator sitr=cis->_segList.begin();
       sitr!=cis->_segList.end();
       ++sitr)
   {
      if (sitr->second->intersect(bb))
      {

         TriangleFunctor<TriangleIntersect> ti;
         ti.set(*sitr->second);
         drawable.accept(ti);
         if (ti._hit)
         {
            
            osg::Geometry* geometry = drawable.asGeometry();
                

            for(TriangleIntersect::TriangleHitList::iterator thitr=ti._thl.begin();
                thitr!=ti._thl.end();
                ++thitr)
            {
                
               Hit hit;
               hit._nodePath = _nodePath;
               hit._matrix = cis->_model_matrix;
               hit._inverse = cis->_model_inverse;
               hit._drawable = &drawable;
               if (_nodePath.empty()) hit._geode = NULL;
               else hit._geode = dynamic_cast<Geode*>(_nodePath.back());

               TriangleHit& triHit = thitr->second;
                    
               hit._ratio = thitr->first;
               hit._primitiveIndex = triHit._index;
               hit._originalLineSegment = sitr->first;
               hit._localLineSegment = sitr->second;

               hit._intersectPoint = sitr->second->start()*(1.0f-hit._ratio)+
                  sitr->second->end()*hit._ratio;

               hit._intersectNormal = triHit._normal;
                    
               if (geometry)
               {
                  osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                  if (vertices)
                  {
                     osg::Vec3* first = &(vertices->front());
                     if (triHit._v1) hit._vecIndexList.push_back(triHit._v1-first);
                     if (triHit._v2) hit._vecIndexList.push_back(triHit._v2-first);
                     if (triHit._v3) hit._vecIndexList.push_back(triHit._v3-first);
                  }
               }
                    

               _segHitList[sitr->first.get()].push_back(hit);

               std::sort(_segHitList[sitr->first.get()].begin(),_segHitList[sitr->first.get()].end());

               hitFlag = true;

            }
         }
      }
   }
   return hitFlag;
}

// ==========================================================================
// Traversal member functions
// ==========================================================================

void CustomIntersectVisitor::apply(Node& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(Node)" << endl;
//   cout << "classname = " << node.className() << endl;

   if (!enterNode(node)) return;
   traverse(node);
   leaveNode();

   MyNodeVisitor::apply(node);
}

void CustomIntersectVisitor::apply(Geode& geode)
{
//   cout << "Inside CustomIntersectVisitor::apply(geode)" << endl;
//   cout << "classname = " << geode.className() << endl;

   if (!enterNode(geode)) return;

   for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
   {
      intersect(*geode.getDrawable(i));
   }
   leaveNode();

   MyNodeVisitor::apply(geode);
}

void CustomIntersectVisitor::apply(Billboard& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(Billboard)" << endl;
//   cout << "classname = " << node.className() << endl;

   if (!enterNode(node)) return;

   // CustomIntersectVisitor doesn't have getEyeLocal(), can we use NodeVisitor::getEyePoint()?
   const Vec3& eye_local = getEyePoint();

   for(unsigned int i = 0; i < node.getNumDrawables(); i++ )
   {
      const Vec3& pos = node.getPosition(i);
      osg::ref_ptr<RefMatrix> billboard_matrix = new RefMatrix;
      node.computeMatrix(*billboard_matrix,eye_local,pos);

      pushMatrix(billboard_matrix.get(), osg::Transform::RELATIVE_RF);

      intersect(*node.getDrawable(i));

      popMatrix();

   }
   leaveNode();

   MyNodeVisitor::apply(node);
}

void CustomIntersectVisitor::apply(Group& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(Group)" << endl;
//   cout << "classname = " << node.className() << endl;

   if (!enterNode(node)) return;
   traverse(node);
   leaveNode();

   MyNodeVisitor::apply(node);
}

void CustomIntersectVisitor::apply(Transform& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(Transform)" << endl;
//   cout << "classname = " << node.className() << endl;

   if (!enterNode(node)) return;

   osg::ref_ptr<RefMatrix> matrix = new RefMatrix;
   node.computeLocalToWorldMatrix(*matrix,this);

   pushMatrix(matrix.get(), node.getReferenceFrame());
   traverse(node);
   popMatrix();
   leaveNode();

   MyNodeVisitor::apply(node);
}

void CustomIntersectVisitor::apply(MatrixTransform& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(MatrixTransform)" << endl;
//   cout << "classname = " << node.className() << endl;

   if (!enterNode(node)) return;

   osg::ref_ptr<RefMatrix> matrix = new RefMatrix;
   node.computeLocalToWorldMatrix(*matrix,this);

   pushMatrix(matrix.get(), node.getReferenceFrame());
   traverse(node);
   popMatrix();
   leaveNode();

   MyNodeVisitor::apply(node);
}

void CustomIntersectVisitor::apply(Switch& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(Switch)" << endl;
   apply((Group&)node);
}

void CustomIntersectVisitor::apply(LOD& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(LOD)" << endl;
//   cout << "classname = " << node.className() << endl;

   apply((Group&)node);
}

void CustomIntersectVisitor::apply(PagedLOD& node)
{
//   cout << "Inside CustomIntersectVisitor::apply(PagedLOD)" << endl;
//   cout << "classname = " << node.className() << endl;

   apply((LOD&)node);
}

