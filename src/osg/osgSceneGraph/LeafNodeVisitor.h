// ========================================================================
// LeafNodeVisitor header file
// ========================================================================
// Last updated on 8/21/06; 11/8/06; 11/9/06; 1/3/07; 4/6/14
// ========================================================================

#ifndef LEAFVISITOR_H
#define LEAFVISITOR_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/NodeVisitor>
#include <osg/observer_ptr>
#include "osg/osgSceneGraph/MyNodeVisitor.h"

class LeafNodeVisitor : public MyNodeVisitor 
{
  public: 

   LeafNodeVisitor(); 
   virtual ~LeafNodeVisitor(); 

// Set & get member functions:

   unsigned int get_n_leaf_geodes() const;
   unsigned int get_ntotal_leaf_vertices(bool indices_stored_flag);
   std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
      get_Geodes_ptr();
   const std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
      get_Geodes_ptr() const;
   std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
      get_LeafGeodes_ptr();
   const std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
      get_LeafGeodes_ptr() const;

// Traversal member functions:

   virtual void apply(osg::Node& currNode);
   virtual void apply(osg::Geode& currGeode);
//   virtual void apply(osg::Group& currGroup);
//   virtual void apply(osg::LOD& currLOD);

  private:
   
   int node_counter;
   unsigned int ntotal_leaf_vertices;

// On 1/3/07, Ross Anderson recommended that these next auxilliary STL
// vectors which store pointers to geodes within the scene graph be
// converted to OSG observer pointers rather than to reference
// pointers.  If the geodes are deleted for some reason, then observer
// pointers are set to NULL.  Hopefully, some iteration over such NULL
// pointers would lead to an easily identifiable core dump...

   std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
      Geodes_ptr;
   std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
      LeafGeodes_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   bool is_approx_geode(osg::Geode& currGeode);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline unsigned int LeafNodeVisitor::get_n_leaf_geodes() const
{
   return get_LeafGeodes_ptr()->size();
}

inline std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
LeafNodeVisitor::get_Geodes_ptr()
{
   return Geodes_ptr;
}

inline const std::vector<std::pair<osg::observer_ptr<osg::Geode>,
   osg::Matrix> >* 
LeafNodeVisitor::get_Geodes_ptr() const
{
   return Geodes_ptr;
}

inline std::vector<std::pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >* 
LeafNodeVisitor::get_LeafGeodes_ptr()
{
   return LeafGeodes_ptr;
}

inline const std::vector<std::pair<osg::observer_ptr<osg::Geode>,
   osg::Matrix> >* 
LeafNodeVisitor::get_LeafGeodes_ptr() const
{
   return LeafGeodes_ptr;
}

#endif 
