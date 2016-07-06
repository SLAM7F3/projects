// ========================================================================
// ParentVisitor class member function definitions
// ========================================================================
// Last updated on 2/1/07; 6/27/07; 5/27/09; 5/28/09
// ========================================================================

#include <iostream>
#include "osg/osgfuncs.h"
#include "osg/osgSceneGraph/ParentVisitor.h"

using std::cout;
using std::endl;
using std::flush;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ParentVisitor::allocate_member_objects()
{
}

void ParentVisitor::initialize_member_objects()
{
   bottom_matrix_identified_flag=false;
   earth_flag=false;
}

ParentVisitor::ParentVisitor() : 
   osg::NodeVisitor(TRAVERSE_PARENTS)
{ 
   allocate_member_objects();
   initialize_member_objects();
} 

ParentVisitor::~ParentVisitor()
{
}

// ------------------------------------------------------------------------
void ParentVisitor::apply(osg::Node& currNode) 
{ 
//   cout << "inside ParentVisitor::apply(Node), class = " 
//        << currNode.className() << endl;

   node_path=getNodePath();
   for (unsigned int i=0; i<node_path.size(); i++)
   {
//      cout << "i = " << i 
//           << " node_path[i] = " << node_path[i] 
//           << " classname = " << node_path[i]->className() << endl;
//      cout << "node name = " << node_path[i]->getName() << endl;
      if (node_path[i]->getName()=="PointCloud") 
      {
         pointcloud_flag=true;
      } // node_path[i]->Name==PointCloud conditional
      if (node_path[i]->getName()=="Earth") 
      {
         earth_flag=true;
//         cout << "Earth name found" << endl;
      }
   }

   osg::NodeVisitor::apply(currNode);
} 

// ------------------------------------------------------------------------
// For conventional visitors which traverse downwards from parents to
// children, accumulate LocalToWorld by POST multiplying with
// currMT.getMatrix().  But for this ParentVisitor which traverses
// upwards from children to parents, accumulate LocalToWorld by PRE
// multiplying with currMT.getMatrix().

void ParentVisitor::apply(osg::MatrixTransform& currMT)
{
//   cout << "inside ParentVisitor::apply(MT), class = " 
//        << currMT.className() << endl;
   
// Note added on 11/19/06: For point cloud coloring purposes, we need
// to store a copy of the bottommost MatrixTransform within an input
// datagraph tree if it exists.  This bottom matrix is actually
// inserted by Ross Anderson's LODTREE program underneath the DataNode
// Group for each tile within an OSGA archive.  Use the translation
// information stored within this bottom matrix for z-coloring and not
// any higher matrix transforms associated with placing point clouds
// onto earth's surface.

   if (!bottom_matrix_identified_flag)
   {
      bottom_Matrix=currMT.getMatrix();
      bottom_matrix_identified_flag=true;
//      cout << "bottom_matrix = " << endl;
//      osgfunc::print_matrix(bottom_Matrix);
   }

   LocalToWorld = currMT.getMatrix() * LocalToWorld;

//   cout << "LocalToWorld = " << endl;
//   osgfunc::print_matrix(LocalToWorld);

   osg::NodeVisitor::apply(currMT);
}

void ParentVisitor::reset_matrix_transform()
{
   LocalToWorld.makeIdentity();
   bottom_Matrix.makeIdentity();
   bottom_matrix_identified_flag=false;
}
