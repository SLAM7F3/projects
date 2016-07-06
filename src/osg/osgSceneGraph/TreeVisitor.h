// ========================================================================
// TreeVisitor header file
// ========================================================================
// Last updated on 11/10/06; 11/11/06; 11/12/06; 1/3/07
// ========================================================================

#ifndef TREEVISITOR_H
#define TREEVISITOR_H

#include "osg/osgSceneGraph/MyNodeVisitor.h"
#include "datastructures/Tree.h"

typedef std::vector<std::string> DATA_TYPE;

class TreeVisitor : public MyNodeVisitor
{
  public: 

   typedef DATA_TYPE data_type;

   TreeVisitor();
   virtual ~TreeVisitor(); 

// Set & get methods:

   bool get_scenegraph_updated_flag() const;
   void set_scenegraph_updated_flag(bool flag);
   Tree<data_type>* get_tree_ptr();
   const Tree<data_type>* get_tree_ptr() const;
   void set_DataNode_ptr(osg::Node* DN_ptr);
   osg::Node* get_DataNode_ptr();
   const osg::Node* get_DataNode_ptr() const;

// Traversal member functions:

   virtual void apply(osg::Node& currNode);

  private:

   bool scenegraph_updated_flag;
   Tree<data_type>* tree_ptr;
   osg::ref_ptr<osg::Node> DataNode_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline bool TreeVisitor::get_scenegraph_updated_flag() const
{
   return scenegraph_updated_flag;
}

inline void TreeVisitor::set_scenegraph_updated_flag(bool flag)
{
   scenegraph_updated_flag=flag;
}

inline void TreeVisitor::set_DataNode_ptr(osg::Node* DN_ptr)
{
   DataNode_refptr=DN_ptr;
}

inline osg::Node* TreeVisitor::get_DataNode_ptr()
{
   return DataNode_refptr.get();
}

inline const osg::Node* TreeVisitor::get_DataNode_ptr() const
{
   return DataNode_refptr.get();
}

#endif 
