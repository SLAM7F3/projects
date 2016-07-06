
#ifndef OSGUTIL_INTERSECTIONVISITOR
#define OSGUTIL_INTERSECTIONVISITOR 1

#include <osg/NodeVisitor>
#include <osg/Drawable>
#include <osgUtil/Export>

#include <list>

namespace osgUtil
{

// forward declare to allow Intersector to reference it.
   class IntersectionVisitor;

/** Pure virtual base class for implementating custom intersection
 * technique.  To implement a specific intersection technique on must
 * override all the pure virtue methods, concrete examples of how to
 * do this can be seen in the LineSegmentIntersector. */

   class Intersector : public osg::Referenced
   {
      public:
    
         enum CoordinateFrame
         {
            WINDOW,
            PROJECTION,
            VIEW,
            MODEL
         };
        
         Intersector(CoordinateFrame cf=MODEL):
            _coordinateFrame(cf),
            _disabledCount(0) {}
    
         void setCoordinateFrame(CoordinateFrame cf) { _coordinateFrame = cf; }
        
         CoordinateFrame getCoordinateFrame() const { return _coordinateFrame; }
    

         virtual Intersector* clone(osgUtil::IntersectionVisitor& iv) = 0;
        
         virtual bool enter(const osg::Node& node) = 0;
        
         virtual void leave() = 0;
        
         virtual void intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable) = 0;
        
         virtual void reset() { _disabledCount = 0; }
        
         virtual bool containsIntersections() = 0;
        
         inline bool disabled() const { return _disabledCount!=0; }
        
         inline void incrementDisabledCount() { ++_disabledCount; }
        
         inline void decrementDisabledCount() { if (_disabledCount>0) --_disabledCount; }

      protected:
    
         CoordinateFrame _coordinateFrame;
         unsigned int _disabledCount;

   };

/** Concrent class for implementing line intersections with the scene graph.
 * To be used in conjunction with IntersectionVisitor. */

   class OSGUTIL_EXPORT LineSegmentIntersector : public Intersector
   {
      public:

         /** Construct a LineSegmentIntersector the runs between the secified start and end points in MODEL coordinates. */

         LineSegmentIntersector(const osg::Vec3d& start, const osg::Vec3d& end);
        
         /** Construct a LineSegmentIntersector the runs between the
             secified start and end points in the specified coordinate
             frame. */
         LineSegmentIntersector(CoordinateFrame cf, const osg::Vec3d& start, const osg::Vec3d& end);
        
         /** Convinience constructor for supporting picking in WINDOW,
          * or PROJECTION coorindates In WINDOW coordinates creates a
          * start value of (x,y,0) and end value of (x,y,1).  In
          * PROJECTION coordinates (clip space cube) creates a start
          * value of (x,y,1) and end value of (x,y,-1).  In VIEW and
          * MODEL coordinates creates a start value of (x,y,0) and end
          * value of (x,y,1).*/
         LineSegmentIntersector(CoordinateFrame cf, double x, double y);
        
         struct Intersection
         {
               Intersection():
                  ratio(-1.0),
                  primitiveIndex(0) {}
        
               bool operator < (const Intersection& rhs) const { return ratio < rhs.ratio; }

               typedef std::vector<unsigned int> IndexList;

               double                          ratio;
               osg::NodePath                   nodePath;
               osg::ref_ptr<osg::Drawable>     drawable;
               osg::ref_ptr<osg::RefMatrix>    matrix;
               osg::Vec3d                      localIntersectionPoint;
               osg::Vec3                       localIntersectionNormal;
               IndexList                       indexList;
               unsigned int                    primitiveIndex;
         };
        
         typedef std::multiset<Intersection> Intersections;
        
         inline void insertIntersection(const Intersection& intersection) { getIntersections().insert(intersection); }

         inline Intersections& getIntersections() { return _parent ? _parent->_intersections : _intersections; }

         inline Intersection getFirstIntersection() { Intersections& intersections = getIntersections(); return intersections.empty() ? Intersection() : *(intersections.begin()); }

      public:

         virtual Intersector* clone(osgUtil::IntersectionVisitor& iv);
        
         virtual bool enter(const osg::Node& node);
        
         virtual void leave();
        
         virtual void intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable);
        
         virtual void reset();

         virtual bool containsIntersections() { return !_intersections.empty(); }

      protected:
    
         bool intersects(const osg::BoundingSphere& bs);
         bool intersectAndClip(osg::Vec3d& s, osg::Vec3d& e,const osg::BoundingBox& bb);

         LineSegmentIntersector* _parent;

         osg::Vec3d  _start;
         osg::Vec3d  _end;
        
         Intersections _intersections;
    
   };

/** Concrent class for implementing polytope intersections with the
 * scene graph.  To be used in conjunction with
 * IntersectionVisitor. */
   class OSGUTIL_EXPORT PolytopeIntersector : public Intersector
   {
      public:
    
         /** Construct a PolytopeIntersector using speified polytope in
             MODEL coordinates.*/
         PolytopeIntersector(const osg::Polytope& polytope);
        
         /** Construct a PolytopeIntersector using speified polytope in specified coordinate frame.*/
         PolytopeIntersector(CoordinateFrame cf, const osg::Polytope& polytope);
        
         /** Convinience constructor for supporting picking in WINDOW,
          * or PROJECTION coorindates In WINDOW coordinates (clip
          * space cube) creates a five sided polytope box that has a
          * front face at 0.0 and sides around box xMin, yMin, xMax,
          * yMax.  In PROJECTION coordinates (clip space cube) creates
          * a five sided polytope box that has a front face at -1 and
          * sides around box xMin, yMin, xMax, yMax.  In VIEW and
          * MODEL coordinates (clip space cube) creates a five sided
          * polytope box that has a front face at 0.0 and sides around
          * box xMin, yMin, xMax, yMax.*/
         PolytopeIntersector(CoordinateFrame cf, double xMin, double yMin, double xMax, double yMax);

         struct Intersection
         {
               Intersection() {}
        
               bool operator < (const Intersection& rhs) const
               {
                  if (nodePath < rhs.nodePath) return true;
                  if (rhs.nodePath < nodePath ) return false;
                  return (drawable < rhs.drawable);
               }

               osg::NodePath                   nodePath;
               osg::ref_ptr<osg::Drawable>     drawable;
         };
        
         typedef std::set<Intersection> Intersections;
        
         inline void insertIntersection(const Intersection& intersection) { getIntersections().insert(intersection); }

         inline Intersections& getIntersections() { return _parent ? _parent->_intersections : _intersections; }

         inline Intersection getFirstIntersection() { Intersections& intersections = getIntersections(); return intersections.empty() ? Intersection() : *(intersections.begin()); }

      public:

         virtual Intersector* clone(osgUtil::IntersectionVisitor& iv);
        
         virtual bool enter(const osg::Node& node);
        
         virtual void leave();
        
         virtual void intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable);
        
         virtual void reset();

         virtual bool containsIntersections() { return !_intersections.empty(); }

      protected:

         PolytopeIntersector* _parent;

         osg::Polytope _polytope;
        
         Intersections _intersections;
    
   };

/** Concrent class for passing multiple intersectors through the scene graph.
 * To be used in conjunction with IntersectionVisitor. */
   class OSGUTIL_EXPORT IntersectorGroup : public Intersector
   {
      public:
    
         IntersectorGroup();

         /** Add an Intersector. */
         void addIntersector(Intersector* intersector);
        
         typedef std::vector< osg::ref_ptr<Intersector> > Intersectors;

         /** Get the list of intersector. */
         Intersectors& getIntersectors() { return _intersectors; }

         /** Clear the list of intersectors.*/    
         void clear();

      public:

         virtual Intersector* clone(osgUtil::IntersectionVisitor& iv);
        
         virtual bool enter(const osg::Node& node);
        
         virtual void leave();
        
         virtual void intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable);
        
         virtual void reset();

         virtual bool containsIntersections();

      protected:
        
         Intersectors _intersectors;
    
   };

/** InteresectionVisitor is used to testing for intersections with the
 * scene, traversing the scene using generic osgUtil::Intersector's
 * to test against the scene.  To implement different types of
 * intersection techniques, one implements custom versions of the
 * osgUtil::Intersector, and then pass the constructed intersector to
 * the IntersectionVisitor.*/
   class OSGUTIL_EXPORT IntersectionVisitor : public osg::NodeVisitor
   {
      public:

         /** Callback used to implement the reading of external files,
          * allowing support for paged databases to be intergrated
          * with IntersectionVisitor.  A concrete implementation can
          * be found in osgDB.  Note, this loose coupling approach is
          * required as osgUtil is independent from osgDB where the
          * file reading is implemented, and osgDB itself is dependent
          * upon osgUtil so a circular dependency would result from
          * tighter integration.*/
         struct ReadCallback : public osg::Referenced
         {
               virtual osg::Node* readNodeFile(const std::string& filename) = 0;
         };


         IntersectionVisitor(Intersector* intersector=0, ReadCallback* readCallback=0);
        
         virtual void reset();

         /** Set the intersector that will be used to intersect with
             the scene, and to store any hits that occur.*/
         void setIntersector(Intersector* intersector);
        
         /** Get the intersector that will be used to intersect with
             the scene, and to store any hits that occur.*/
         Intersector* getIntersector() { return _intersectorStack.empty() ? 0 : _intersectorStack.front().get(); }

         /** Get the const intersector that will be used to intersect with the scene, and to store any hits that occur.*/
         const Intersector* getIntersector() const { return _intersectorStack.empty() ? 0 : _intersectorStack.front().get(); }


         /** Set the read callback.*/
         void setReadCallback(ReadCallback* rc) { _readCallback = rc; }

         /** Get the read callback.*/
         ReadCallback* getReadCallback() { return _readCallback.get(); }

         /** Get the const read callback.*/
         const ReadCallback* getReadCallback() const { return _readCallback.get(); }
        
        
         void pushWindowMatrix(osg::RefMatrix* matrix) { _windowStack.push_back(matrix); }
         void pushWindowMatrix(osg::Viewport* viewport) { _windowStack.push_back(new osg::RefMatrix( viewport->computeWindowMatrix()) ); }
         void popWindowMatrix() { _windowStack.pop_back(); }
         osg::RefMatrix* getWindowMatrix() { return _windowStack.empty() ? 0 :  _windowStack.back().get(); }

         void pushProjectionMatrix(osg::RefMatrix* matrix) { _projectionStack.push_back(matrix); }
         void popProjectionMatrix() { _projectionStack.pop_back(); }
         osg::RefMatrix* getProjectionMatrix() { return _projectionStack.empty() ? 0 :  _projectionStack.back().get(); }

         void pushViewMatrix(osg::RefMatrix* matrix) { _viewStack.push_back(matrix); }
         void popViewMatrix() { _viewStack.pop_back(); }
         osg::RefMatrix* getViewMatrix() { return _viewStack.empty() ? 0 :  _viewStack.back().get(); }

         void pushModelMatrix(osg::RefMatrix* matrix) { _modelStack.push_back(matrix); }
         void popModelMatrix() { _modelStack.pop_back(); } 
         osg::RefMatrix* getModelMatrix() { return _modelStack.empty() ? 0 :  _modelStack.back().get(); }

      public:

         virtual void apply(osg::Node& node);
         virtual void apply(osg::Geode& geode);
         virtual void apply(osg::Billboard& geode);
         virtual void apply(osg::Group& group);
         virtual void apply(osg::LOD& lod);
         virtual void apply(osg::PagedLOD& lod);
         virtual void apply(osg::Transform& transform);
         virtual void apply(osg::Projection& projection);
         virtual void apply(osg::CameraNode& camera);
    
      protected:
    
         inline bool enter(const osg::Node& node) { return _intersectorStack.empty() ? false : _intersectorStack.back()->enter(node); }
         inline void leave() { _intersectorStack.back()->leave(); }
         inline void intersect(osg::Drawable* drawable) { _intersectorStack.back()->intersect(*this, drawable); }
         inline void push_clone() { _intersectorStack.push_back ( _intersectorStack.front()->clone(*this) ); }
         inline void pop_clone() { if (_intersectorStack.size()>=2) _intersectorStack.pop_back(); }

         typedef std::list< osg::ref_ptr<Intersector> > IntersectorStack;
         IntersectorStack _intersectorStack;
        
         osg::ref_ptr<ReadCallback> _readCallback;
        
         typedef std::list< osg::ref_ptr<osg::RefMatrix> > MatrixStack;
         MatrixStack _windowStack;
         MatrixStack _projectionStack;
         MatrixStack _viewStack;
         MatrixStack _modelStack;

   };

}

#endif

