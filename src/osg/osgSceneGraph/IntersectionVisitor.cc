
#include "osg/osgSceneGraph/IntersectionVisitor.h"

#include <osg/PagedLOD>
#include <osg/Transform>
#include <osg/Projection>
#include <osg/CameraNode>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TriangleFunctor>

using namespace osgUtil;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LineSegmentIntersector
//


struct TriangleIntersection
{
      TriangleIntersection(unsigned int index, const osg::Vec3& normal, float r1, const osg::Vec3* v1, float r2, const osg::Vec3* v2, float r3, const osg::Vec3* v3):
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


struct TriangleIntersector
{
      osg::Vec3   _s;
      osg::Vec3   _d;
      float       _length;

      int         _index;
      float       _ratio;
      bool        _hit;

      typedef std::multimap<float,TriangleIntersection> TriangleIntersections;
    
      TriangleIntersections _intersections;

      TriangleIntersector()
      {
      }

      TriangleIntersector(const osg::Vec3d& start, osg::Vec3d& end, float ratio=FLT_MAX)
      {
         set(start,end,ratio);
      }
    
      void set(const osg::Vec3d& start, osg::Vec3d& end, float ratio=FLT_MAX)
      {
         _hit=false;
         _index = 0;
         _ratio = ratio;

         _s = start;
         _d = end - start;
         _length = _d.length();
         _d /= _length;
      }

      //   bool intersect(const Vec3& v1,const Vec3& v2,const Vec3& v3,float& r)
      inline void operator () (const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3, bool treatVertexDataAsTemporary)
      {
         ++_index;

         if (v1==v2 || v2==v3 || v1==v3) return;

         osg::Vec3 v12 = v2-v1;
         osg::Vec3 n12 = v12^_d;
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

         osg::Vec3 v23 = v3-v2;
         osg::Vec3 n23 = v23^_d;
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

         osg::Vec3 v31 = v1-v3;
         osg::Vec3 n31 = v31^_d;
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
        
         osg::Vec3 in = v1*r1+v2*r2+v3*r3;
         if (!in.valid())
         {
            osg::notify(osg::WARN)<<"Warning:: Picked up error in TriangleIntersect"<<std::endl;
            osg::notify(osg::WARN)<<"   ("<<v1<<",\t"<<v2<<",\t"<<v3<<")"<<std::endl;
            osg::notify(osg::WARN)<<"   ("<<r1<<",\t"<<r2<<",\t"<<r3<<")"<<std::endl;
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
            _intersections.insert(std::pair<const float,TriangleIntersection>(r,TriangleIntersection(_index-1,normal,r1,0,r2,0,r3,0)));
         }
         else
         {
            _intersections.insert(std::pair<const float,TriangleIntersection>(r,TriangleIntersection(_index-1,normal,r1,&v1,r2,&v2,r3,&v3)));
         }
         _hit = true;

      }

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LineSegmentIntersector
//

LineSegmentIntersector::LineSegmentIntersector(const osg::Vec3d& start, const osg::Vec3d& end):
   _parent(0),
   _start(start),
   _end(end)
{
}

LineSegmentIntersector::LineSegmentIntersector(CoordinateFrame cf, const osg::Vec3d& start, const osg::Vec3d& end):
   Intersector(cf),
   _parent(0),
   _start(start),
   _end(end)
{
}

LineSegmentIntersector::LineSegmentIntersector(CoordinateFrame cf, double x, double y):
   Intersector(cf),
   _parent(0)
{
   switch(cf)
   {
      case WINDOW : _start.set(x,y,0.0); _end.set(x,y,1.0); break;
      case PROJECTION : _start.set(x,y,1.0); _end.set(x,y,-1.0); break;
      case VIEW : _start.set(x,y,0.0); _end.set(x,y,1.0); break;
      case MODEL : _start.set(x,y,0.0); _end.set(x,y,1.0); break;
   }
}

Intersector* LineSegmentIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
   if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
   {
      osg::ref_ptr<LineSegmentIntersector> lsi = new LineSegmentIntersector(_start, _end);
      lsi->_parent = this;
      return lsi.release();
   }

   // compute the matrix that takes this Intersector from its CoordinateFrame into the local MODEL coordinate frame
   // that geometry in the scene graph will always be in.
   osg::Matrix matrix;
   switch (_coordinateFrame)
   {
      case(WINDOW): 
         if (iv.getWindowMatrix()) matrix.preMult( *iv.getWindowMatrix() );
         if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
         if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
         if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
         break;
      case(PROJECTION): 
         if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
         if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
         if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
         break;
      case(VIEW): 
         if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
         if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
         break;
      case(MODEL):
         if (iv.getModelMatrix()) matrix = *iv.getModelMatrix();
         break;
   }

   osg::Matrix inverse;
   inverse.invert(matrix);

   osg::ref_ptr<LineSegmentIntersector> lsi = new LineSegmentIntersector(_start * inverse, _end * inverse);
   lsi->_parent = this;
   return lsi.release();
}

bool LineSegmentIntersector::enter(const osg::Node& node)
{
   return !node.isCullingActive() || intersects( node.getBound() );
}

void LineSegmentIntersector::leave()
{
   // do nothing
}

void LineSegmentIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
   osg::Vec3d s(_start), e(_end);    
   if ( !intersectAndClip( s, e, drawable->getBound() ) ) return;

   // reset the clipped range as it can be too close in on the BB, and cause missing due precission issues.
   s = _start;
   e = _end;

   osg::TriangleFunctor<TriangleIntersector> ti;
   ti.set(s,e);
   drawable->accept(ti);
   if (ti._hit)
   {
      osg::Geometry* geometry = drawable->asGeometry();

      for(TriangleIntersector::TriangleIntersections::iterator thitr = ti._intersections.begin();
          thitr != ti._intersections.end();
          ++thitr)
      {

         // get ratio in s,e range
         float ratio = thitr->first;

         // remap ratio into _start, _end range
         ratio = ((s-_start).length() + ratio * (e-s).length() )/(_end-_start).length();

         TriangleIntersection& triHit = thitr->second;

         Intersection hit;
         hit.ratio = ratio;
         hit.matrix = iv.getModelMatrix();
         hit.nodePath = iv.getNodePath();
         hit.drawable = drawable;
         hit.primitiveIndex = triHit._index;

         hit.localIntersectionPoint = s*(1.0f-ratio) + e*ratio;
         hit.localIntersectionNormal = triHit._normal;

         if (geometry)
         {
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            if (vertices)
            {
               osg::Vec3* first = &(vertices->front());
               if (triHit._v1) hit.indexList.push_back(triHit._v1-first);
               if (triHit._v2) hit.indexList.push_back(triHit._v2-first);
               if (triHit._v3) hit.indexList.push_back(triHit._v3-first);
            }
         }

         insertIntersection(hit);

      }
   }
    
}

void LineSegmentIntersector::reset()
{
   Intersector::reset();
    
   _intersections.clear();
}

bool LineSegmentIntersector::intersects(const osg::BoundingSphere& bs)
{
   // if bs not valid then return true based on the assumption that an invalid sphere is yet to be defined.
   if (!bs.valid()) return true; 

   osg::Vec3d sm = _start - bs._center;
   double c = sm.length2()-bs._radius*bs._radius;
   if (c<0.0) return true;

   osg::Vec3d se = _end-_start;
   double a = se.length2();
   double b = (sm*se)*2.0;
   double d = b*b-4.0*a*c;

   if (d<0.0) return false;

   d = sqrt(d);

   double div = 1.0/(2.0*a);

   double r1 = (-b-d)*div;
   double r2 = (-b+d)*div;

   if (r1<=0.0 && r2<=0.0) return false;

   if (r1>=1.0 && r2>=1.0) return false;

    // passed all the rejection tests so line must intersect bounding sphere, return true.
   return true;
}

bool LineSegmentIntersector::intersectAndClip(osg::Vec3d& s, osg::Vec3d& e,const osg::BoundingBox& bb)
{
   // compate s and e against the xMin to xMax range of bb.
   if (s.x()<=e.x())
   {

      // trivial reject of segment wholely outside.
      if (e.x()<bb.xMin()) return false;
      if (s.x()>bb.xMax()) return false;

      if (s.x()<bb.xMin())
      {
         // clip s to xMin.
         s = s+(e-s)*(bb.xMin()-s.x())/(e.x()-s.x());
      }

      if (e.x()>bb.xMax())
      {
         // clip e to xMax.
         e = s+(e-s)*(bb.xMax()-s.x())/(e.x()-s.x());
      }
   }
   else
   {
      if (s.x()<bb.xMin()) return false;
      if (e.x()>bb.xMax()) return false;

      if (e.x()<bb.xMin())
      {
         // clip s to xMin.
         e = s+(e-s)*(bb.xMin()-s.x())/(e.x()-s.x());
      }

      if (s.x()>bb.xMax())
      {
         // clip e to xMax.
         s = s+(e-s)*(bb.xMax()-s.x())/(e.x()-s.x());
      }
   }

   // compate s and e against the yMin to yMax range of bb.
   if (s.y()<=e.y())
   {

      // trivial reject of segment wholely outside.
      if (e.y()<bb.yMin()) return false;
      if (s.y()>bb.yMax()) return false;

      if (s.y()<bb.yMin())
      {
         // clip s to yMin.
         s = s+(e-s)*(bb.yMin()-s.y())/(e.y()-s.y());
      }

      if (e.y()>bb.yMax())
      {
         // clip e to yMax.
         e = s+(e-s)*(bb.yMax()-s.y())/(e.y()-s.y());
      }
   }
   else
   {
      if (s.y()<bb.yMin()) return false;
      if (e.y()>bb.yMax()) return false;

      if (e.y()<bb.yMin())
      {
         // clip s to yMin.
         e = s+(e-s)*(bb.yMin()-s.y())/(e.y()-s.y());
      }

      if (s.y()>bb.yMax())
      {
         // clip e to yMax.
         s = s+(e-s)*(bb.yMax()-s.y())/(e.y()-s.y());
      }
   }

   // compate s and e against the zMin to zMax range of bb.
   if (s.z()<=e.z())
   {

      // trivial reject of segment wholely outside.
      if (e.z()<bb.zMin()) return false;
      if (s.z()>bb.zMax()) return false;

      if (s.z()<bb.zMin())
      {
         // clip s to zMin.
         s = s+(e-s)*(bb.zMin()-s.z())/(e.z()-s.z());
      }

      if (e.z()>bb.zMax())
      {
         // clip e to zMax.
         e = s+(e-s)*(bb.zMax()-s.z())/(e.z()-s.z());
      }
   }
   else
   {
      if (s.z()<bb.zMin()) return false;
      if (e.z()>bb.zMax()) return false;

      if (e.z()<bb.zMin())
      {
         // clip s to zMin.
         e = s+(e-s)*(bb.zMin()-s.z())/(e.z()-s.z());
      }

      if (s.z()>bb.zMax())
      {
         // clip e to zMax.
         s = s+(e-s)*(bb.zMax()-s.z())/(e.z()-s.z());
      }
   }
    
   // osg::notify(osg::NOTICE)<<"clampped segment "<<s<<" "<<e<<std::endl;
    
    // if (s==e) return false;

   return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectorGroup
//
PolytopeIntersector::PolytopeIntersector(const osg::Polytope& polytope):
   _parent(0),
   _polytope(polytope)
{
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, const osg::Polytope& polytope):
   Intersector(cf),
   _parent(0),
   _polytope(polytope)
{
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, double xMin, double yMin, double xMax, double yMax):
   Intersector(cf),
   _parent(0)
{
   double zNear = 0.0;
   switch(cf)
   {
      case WINDOW : zNear = 0.0; break;
      case PROJECTION : zNear = 1.0; break;
      case VIEW : zNear = 0.0; break;
      case MODEL : zNear = 0.0; break;
   }

   _polytope.add(osg::Plane(1.0, 0.0, 0.0, -xMin));
   _polytope.add(osg::Plane(-1.0,0.0 ,0.0, xMax));
   _polytope.add(osg::Plane(0.0, 1.0, 0.0,-yMin));
   _polytope.add(osg::Plane(0.0,-1.0,0.0, yMax));
   _polytope.add(osg::Plane(0.0,0.0,1.0, zNear));
}

Intersector* PolytopeIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
   if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
   {
      osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(_polytope);
      pi->_parent = this;
      return pi.release();
   }

   // compute the matrix that takes this Intersector from its CoordinateFrame into the local MODEL coordinate frame
   // that geometry in the scene graph will always be in.
   osg::Matrix matrix;
   switch (_coordinateFrame)
   {
      case(WINDOW): 
         if (iv.getWindowMatrix()) matrix.preMult( *iv.getWindowMatrix() );
         if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
         if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
         if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
         break;
      case(PROJECTION): 
         if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
         if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
         if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
         break;
      case(VIEW): 
         if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
         if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
         break;
      case(MODEL):
         if (iv.getModelMatrix()) matrix = *iv.getModelMatrix();
         break;
   }

   osg::Polytope transformedPolytope;
   transformedPolytope.setAndTransformProvidingInverse(_polytope, matrix);

   osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(transformedPolytope);
   pi->_parent = this;
   return pi.release();
}

bool PolytopeIntersector::enter(const osg::Node& node)
{
   return !node.isCullingActive() || _polytope.contains( node.getBound() );
}


void PolytopeIntersector::leave()
{
   // do nothing.
}


void PolytopeIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
   if ( !_polytope.contains( drawable->getBound() ) ) return;

   osg::Geometry* geometry = drawable->asGeometry();
   osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;
   if (vertices)
   {
      if (!_polytope.contains(*vertices)) return;
   }

   Intersection hit;
   hit.nodePath = iv.getNodePath();
   hit.drawable = drawable;
    
   insertIntersection(hit);
}


void PolytopeIntersector::reset()
{
   Intersector::reset();
    
   _intersections.clear();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectorGroup
//

IntersectorGroup::IntersectorGroup()
{
}

void IntersectorGroup::addIntersector(Intersector* intersector)
{
   _intersectors.push_back(intersector);
}

void IntersectorGroup::clear()
{
   _intersectors.clear();
}

Intersector* IntersectorGroup::clone(osgUtil::IntersectionVisitor& iv)
{
   IntersectorGroup* ig = new IntersectorGroup;
    
   // now copy across all intersectors that arn't disabled.
   for(Intersectors::iterator itr = _intersectors.begin();
       itr != _intersectors.end();
       ++itr)
   {
      if (!(*itr)->disabled())
      {
         ig->addIntersector( (*itr)->clone(iv) );
      }
   }

   return ig;
}

bool IntersectorGroup::enter(const osg::Node& node)
{
   if (disabled()) return false;
    
   bool foundIntersections = false;
    
   for(Intersectors::iterator itr = _intersectors.begin();
       itr != _intersectors.end();
       ++itr)
   {
      if ((*itr)->disabled()) (*itr)->incrementDisabledCount();
      else if ((*itr)->enter(node)) foundIntersections = true;
      else (*itr)->incrementDisabledCount();
   }
    
   if (!foundIntersections) 
   {
      // need to call leave to clean up the DisabledCount's.
      leave();
      return false;
   }
    
   // we have found at least one suitable intersector, so return true
   return true;
}

void IntersectorGroup::leave()
{
   for(Intersectors::iterator itr = _intersectors.begin();
       itr != _intersectors.end();
       ++itr)
   {
      if ((*itr)->disabled()) (*itr)->decrementDisabledCount();
   }
}

void IntersectorGroup::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
   if (disabled()) return;

   unsigned int numTested = 0;
   for(Intersectors::iterator itr = _intersectors.begin();
       itr != _intersectors.end();
       ++itr)
   {
      if (!(*itr)->disabled())
      {
         (*itr)->intersect(iv, drawable);
            
         ++numTested;
      }
   }
    
   // osg::notify(osg::NOTICE)<<"Number testing "<<numTested<<std::endl;

}

void IntersectorGroup::reset()
{
   Intersector::reset();
    
   for(Intersectors::iterator itr = _intersectors.begin();
       itr != _intersectors.end();
       ++itr)
   {
      (*itr)->reset();
   }
}

bool IntersectorGroup::containsIntersections()
{
   for(Intersectors::iterator itr = _intersectors.begin();
       itr != _intersectors.end();
       ++itr)
   {
      if ((*itr)->containsIntersections()) return true;
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectionVisitor
//

IntersectionVisitor::IntersectionVisitor(Intersector* intersector, ReadCallback* readCallback)
{
   // overide the default node visitor mode.
   setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    
   setIntersector(intersector);
    
   setReadCallback(readCallback);
}

void IntersectionVisitor::setIntersector(Intersector* intersector)
{
   // keep refernce around just in case intersector is already in the _intersectorStack, otherwsie the clear could delete it.
   osg::ref_ptr<Intersector> temp = intersector;

   _intersectorStack.clear();

   if (intersector) _intersectorStack.push_back(intersector);
}

void IntersectionVisitor::reset()
{
   if (!_intersectorStack.empty())
   {
      osg::ref_ptr<Intersector> intersector = _intersectorStack.front();
      intersector->reset();
        
      _intersectorStack.clear();
      _intersectorStack.push_back(intersector);
   }
}

void IntersectionVisitor::apply(osg::Node& node)
{
   // osg::notify(osg::NOTICE)<<"apply(Node&)"<<std::endl;

   if (!enter(node)) return;

    // osg::notify(osg::NOTICE)<<"inside apply(Node&)"<<std::endl;

   traverse(node);

   leave();
}

void IntersectionVisitor::apply(osg::Group& group)
{
   if (!enter(group)) return;

   traverse(group);

   leave();
}

void IntersectionVisitor::apply(osg::Geode& geode)
{
   // osg::notify(osg::NOTICE)<<"apply(Geode&)"<<std::endl;

   if (!enter(geode)) return;

    // osg::notify(osg::NOTICE)<<"inside apply(Geode&)"<<std::endl;

   for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
   {
      intersect( geode.getDrawable(i) );
   }

   leave();
}

void IntersectionVisitor::apply(osg::Billboard& billboard)
{
   if (!enter(billboard)) return;

   for(unsigned int i=0; i<billboard.getNumDrawables(); ++i)
   {
      intersect( billboard.getDrawable(i) );
   }

   leave();
}

void IntersectionVisitor::apply(osg::LOD& lod)
{
   if (!enter(lod)) return;

   traverse(lod);

   leave();
}


void IntersectionVisitor::apply(osg::PagedLOD& plod)
{
   if (!enter(plod)) return;

   if (plod.getNumFileNames()>0)
   {
      osg::ref_ptr<osg::Node> highestResChild;

      if (plod.getNumFileNames() != plod.getNumChildren() && _readCallback.valid())
      {
         highestResChild = _readCallback->readNodeFile( plod.getDatabasePath() + plod.getFileName(plod.getNumFileNames()-1) );
      }
      else if (plod.getNumChildren()>0)
      {
         highestResChild = plod.getChild( plod.getNumChildren()-1 );
      }

      if (highestResChild.valid())
      {
         highestResChild->accept(*this);
      }
   }

   leave();
}


void IntersectionVisitor::apply(osg::Transform& transform)
{
   if (!enter(transform)) return;

   osg::ref_ptr<osg::RefMatrix> matrix = _modelStack.empty() ? new osg::RefMatrix() : new osg::RefMatrix(*_modelStack.back());
   transform.computeLocalToWorldMatrix(*matrix,this);

   pushModelMatrix(matrix.get());

   // now push an new intersector clone transform to the new local coordinates
   push_clone();

   traverse(transform);
    
   // pop the clone.
   pop_clone();
    
   popModelMatrix();

   // tidy up an cached cull variabes in the current intersector.
   leave();
}


void IntersectionVisitor::apply(osg::Projection& projection)
{
   if (!enter(projection)) return;

   pushProjectionMatrix(new osg::RefMatrix(projection.getMatrix()) );

   // now push an new intersector clone transform to the new local coordinates
   push_clone();

   traverse(projection);
    
   // pop the clone.
   pop_clone();
    
   popProjectionMatrix();

   leave();
}


void IntersectionVisitor::apply(osg::CameraNode& camera)
{
   // osg::notify(osg::NOTICE)<<"apply(CameraNode&)"<<std::endl;

   // note, commenting out right now because default CameraNode setup is with the culling active.  Should this be changed?
   // if (!enter(camera)) return;
    
   // osg::notify(osg::NOTICE)<<"inside apply(CameraNode&)"<<std::endl;

   if (camera.getViewport()) pushWindowMatrix( camera.getViewport() );
   pushProjectionMatrix( new osg::RefMatrix(camera.getProjectionMatrix()) );
   pushViewMatrix( new osg::RefMatrix(camera.getViewMatrix()) );
   pushModelMatrix( new osg::RefMatrix() );

    // now push an new intersector clone transform to the new local coordinates
   push_clone();

   traverse(camera);
    
    // pop the clone.
   pop_clone();
    
   popModelMatrix();
   popViewMatrix();
   popProjectionMatrix();
   if (camera.getViewport()) popWindowMatrix();

   // leave();
}
