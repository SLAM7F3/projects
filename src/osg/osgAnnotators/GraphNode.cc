// ==========================================================================
// GraphNode class member function definitions
// ==========================================================================
// Last updated on 11/10/06; 1/4/07; 7/29/07
// ==========================================================================

#include <iostream>
#include "color/colorfuncs.h"
#include "osg/osgAnnotators/GraphNode.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "passes/Pass.h"

using std::cout;
using std::cin;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GraphNode::allocate_member_objects()
{
   LineSegmentsGroup_ptr=new LineSegmentsGroup(3,pass_ptr);
}		       

void GraphNode::initialize_member_objects()
{
   Graphical_name="GraphNode";
   OSGnode_type=Node;
}		       

GraphNode::GraphNode(
   int id,double w,double l,double h,double face_displacement,Pass* PI_ptr):
   Box(w,l,h,face_displacement,id)
{	
   pass_ptr=PI_ptr;
   allocate_member_objects();
   initialize_member_objects();
}		       

GraphNode::~GraphNode()
{
   delete LineSegmentsGroup_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const GraphNode& G)
{
   outstream << "inside GraphNode::operator<<" << endl;
   outstream << static_cast<const Box&>(G) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_group

osg::Group* GraphNode::generate_drawable_group()
{
   drawable_group_refptr = new osg::Group;
   drawable_group_refptr->addChild(Box::generate_drawable_group());

   set_permanent_color(colorfunc::get_OSG_color(colorfunc::white));
//   set_permanent_color(colorfunc::get_OSG_color(colorfunc::purple));
   
   return drawable_group_refptr.get();
}

// ==========================================================================
// GraphNode manipulation methods
// ==========================================================================

// Member function set_posn takes in a time and pass number along with
// threevector V.  It translates a canonical GraphNode so that it's
// centered on V.  This translation information is stored for later
// callback retrieval.

void GraphNode::set_posn(double curr_t,int pass_number,const threevector& V)
{
//   cout << "inside GraphNode::set_posn()" << endl;
//   cout << "V1 = " << V1 << " V2 = " << V2 << endl;

   set_UVW_coords(curr_t,pass_number,V);

//   threevector UVW;
//   get_UVW_coords(curr_t,pass_number,UVW);
//   cout << "UVW = " << UVW << endl;
}
