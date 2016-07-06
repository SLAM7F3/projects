// ==========================================================================
// Header file for GraphNode class
// ==========================================================================
// Last updated on 11/7/06; 11/8/06; 11/10/06; 11/13/06; 1/4/07
// ==========================================================================

#ifndef GraphNode_H
#define GraphNode_H

#include <iostream>
#include <osg/Group>
#include "osg/osgGeometry/Box.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"

class Pass;

class GraphNode : public Box
{

  public:

   enum OSG_NODE_TYPE
   {
      CoordinateSystemNode,Projection,PositionAttitudeTransform,
      MatrixTransform,PagedLOD,LOD,Group,Geode,Node
   };

// Initialization, constructor and destructor functions:

   GraphNode(int id,double w,double l,double h,double face_displacement,
             Pass* PI_ptr);
   virtual ~GraphNode();
   friend std::ostream& operator<< (
      std::ostream& outstream,const GraphNode& G);

// Set & get methods:

   void set_OSGnode_type(OSG_NODE_TYPE nodetype);
   GraphNode::OSG_NODE_TYPE get_OSGnode_type();
   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   osg::Group* get_drawable_group_ptr();

// Drawing methods:

   osg::Group* generate_drawable_group();

// GraphNode manipulation methods:

   void set_posn(double curr_t,int pass_number,const threevector& V);

  protected:

  private:

   OSG_NODE_TYPE OSGnode_type;
   Pass* pass_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   osg::ref_ptr<osg::Group> drawable_group_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const GraphNode& G);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void GraphNode::set_OSGnode_type(OSG_NODE_TYPE nodetype)
{
   OSGnode_type=nodetype;
}

inline GraphNode::OSG_NODE_TYPE GraphNode::get_OSGnode_type()
{
   return OSGnode_type;
}

inline LineSegmentsGroup* GraphNode::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

inline osg::Group* GraphNode::get_drawable_group_ptr()
{
   return drawable_group_refptr.get();
}


#endif // GraphNode.h



