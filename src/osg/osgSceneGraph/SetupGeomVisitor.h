// ========================================================================
// SetupGeomVisitor header file
// ========================================================================
// Last updated on 8/11/06; 1/4/07; 11/27/11; 12/28/11
// ========================================================================

#ifndef SETUPGEOMVISITOR_H
#define SETUPGEOMVISITOR_H

#include <iostream>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include "osg/osgSceneGraph/MyHyperFilter.h"
#include "osg/osgSceneGraph/MyNodeVisitor.h"

// class osg::Geometry;
// class osg::StateSet;

class SetupGeomVisitor : public MyNodeVisitor
{
  public: 

   SetupGeomVisitor(osg::StateSet* SS_ptr);
   virtual ~SetupGeomVisitor(); 

   void set_MyHyperFilter_ptr(model::MyHyperFilter* hf_ptr);
   void add_HyperFilter_Callback();

   virtual void apply(osg::Node& currNode);
   virtual void apply(osg::Geode& currGeode);

  protected:

  private:
   
   osg::ref_ptr<osg::StateSet> StateSet_refptr;
   model::MyHyperFilter* MyHyperFilter_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif 
