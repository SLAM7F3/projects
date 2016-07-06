// ==========================================================================
// Header file for MYDATABASEPAGER class
// ==========================================================================
// Last modified on 1/3/07; 3/23/09; 12/2/11
// ==========================================================================

#ifndef MYDATABASEPAGER_H
#define MYDATABASEPAGER_H

#include <osgDB/DatabasePager>
#include <osg/NodeCallback>
#include <osg/observer_ptr>
#include <osgDB/Registry>
#include <vector>
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgSceneGraph/MyNodeVisitor.h"
#include "osg/osgSceneGraph/ParentVisitor.h"

namespace viewer {

// MyDatabasePager is a minor variant of Ross Anderson's
// CustomDatabasePager which can be used to perform operations and
// call embedded callbacks (e.g. colormap updating) after any page is
// loaded in.

   class MyDatabasePager : public osgDB::DatabasePager
      {
	public:
		
         MyDatabasePager();
         MyDatabasePager(MyNodeVisitor* nv);
         MyDatabasePager(MyNodeVisitor* nv,ColorGeodeVisitor* nv2);
         MyDatabasePager(MyNodeVisitor* nv,ColorGeodeVisitor* nv2,
                         MyNodeVisitor* nv3,osg::Node* DN_ptr);

// Set & get member functions:
	
// On 7/7/06, Ross indicated that registerPagedLODs should be
// commented in and updateSceneGraph should be commented out.  He
// believes that the former should be guaranteed to visit every new
// node that is paged in, while the latter may not.  But we are
// experiencing grid coloring troubles with registerPagedLODs.  So for
// now, we stick with updateSceneGraph...
	
// Applies the setup visitor to bring new nodes up to speed.

         virtual void updateSceneGraph(double currentFrameTime);

// Adds a call to apply the standard setup visitor to the tree. 

//         virtual void registerPagedLODs(osg::Node* subgraph);
         
        protected:

// Set the visitor(s) that is (are) called to setup the graph,
// including when LODs are registered (i.e. the initial tree), and
// when additional pages are loaded from disk:

         osg::ref_ptr<MyNodeVisitor> _setupVisitor_refptr;
         osg::ref_ptr<ColorGeodeVisitor> ColorGeodeVisitor_refptr;
         osg::ref_ptr<MyNodeVisitor> _setupVisitor3_refptr;

         virtual ~MyDatabasePager() {};

// For debugging purposes only, we copy the next method from
// osg::DatabasePager:

//         void addLoadedDataToSceneGraph(double currentFrameTime);

        private:

         bool first_traversal_flag;
         osg::ref_ptr<osgDB::Registry> Registry_refptr;
         osg::ref_ptr<ParentVisitor> ParentVisitor_refptr;
         osg::ref_ptr<osg::Node> DataNode_refptr;

// On 1/3/07, Ross Anderson recommended that this next auxilliary STL
// vector which store pointers to Nodes recently paged into the scene
// graph be converted to OSG observer pointers rather than to
// reference pointers.  If the nodes are deleted for some reason, then
// observer pointers are set to NULL.  Hopefully, some iteration over
// such NULL pointers would lead to an easily identifiable core
// dump...

         std::vector<osg::observer_ptr<osg::Node> > PagedNode;

         void allocate_member_objects();
         void initialize_member_objects();

      };

} // namespace viewer

#endif // MYDATABASEPAGER_H
