// ==========================================================================
// Header file for MYINTERSECTVISITOR class which is a variant of
// OSG's IntersectVisitor class.
// ==========================================================================
// Last modified on 10/26/06; 11/11/06; 11/12/06; 11/13/06
// ==========================================================================

#ifndef MYINTERSECTVISITOR_H
#define MYINTERSECTVISITOR_H

#include <string>
#include <osgDB/DatabasePager>
#include "osg/osgSceneGraph/CustomIntersectVisitor.h"
#include "datastructures/Tree.h"

class MyIntersectVisitor : public osgUtil::CustomIntersectVisitor
{
  public:

   typedef std::vector<std::string> data_type;
		
   MyIntersectVisitor();
   MyIntersectVisitor(Tree<data_type>* treeptr);
   MyIntersectVisitor(osgDB::DatabasePager* DBP_ptr);
   virtual ~MyIntersectVisitor();
		
// Set & get methods:

   void setDatabasePager( osgDB::DatabasePager* dbp );
   osgDB::DatabasePager* getDatabasePager();
   void set_tree_ptr(Tree<data_type>* treeptr);

// Traversal member functions:

   virtual void apply(osg::Node& node);
//   virtual void apply(osg::Group& group);
//   virtual void apply(osg::Geode& node);
//   virtual void apply(osg::MatrixTransform& node);
 //  virtual void apply(osg::LOD& node);
//   virtual void apply(osg::PagedLOD& node);
	
  private:
		
   std::string paged_filename;
   osg::ref_ptr<osgDB::DatabasePager> _pager;
   Tree<data_type>* tree_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void MyIntersectVisitor::setDatabasePager( osgDB::DatabasePager* dbp ) 
{ 
   _pager = dbp; 
}

inline osgDB::DatabasePager* MyIntersectVisitor::getDatabasePager() 
{ 
   return _pager.get(); 
}

inline void MyIntersectVisitor:: set_tree_ptr(Tree<data_type>* treeptr)
{
   tree_ptr=treeptr;
}



#endif // MYINTERSECTVISITOR
