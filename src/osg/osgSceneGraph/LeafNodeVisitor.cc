// ========================================================================
// LeafNodeVisitor class member function definitions
// ========================================================================
// Last updated on 10/29/06; 11/8/06; 11/9/06; 1/3/07; 4/6/14
// ========================================================================

#include <osg/Geometry>
#include <osg/Group>
#include <osg/Matrix>
#include "osg/osgSceneGraph/LeafNodeVisitor.h"
#include "osg/osgSceneGraph/MyNodeInfo.h"
#include "general/outputfuncs.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"

#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::flush;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LeafNodeVisitor::allocate_member_objects()
{
   Geodes_ptr=new vector<pair<osg::observer_ptr<osg::Geode>,osg::Matrix> >;
   LeafGeodes_ptr=new vector<pair<osg::observer_ptr<osg::Geode>,
      osg::Matrix> >;
}

void LeafNodeVisitor::initialize_member_objects()
{
   node_counter=0;
   ntotal_leaf_vertices=0;
}

LeafNodeVisitor::LeafNodeVisitor() 
{ 
   allocate_member_objects();
   initialize_member_objects();
} 

LeafNodeVisitor::~LeafNodeVisitor()
{
   delete Geodes_ptr;
   delete LeafGeodes_ptr;
}

// ------------------------------------------------------------------------
// Member function get_ntotal_leaf_vertices scans over all current
// leaf geodes and sums up their total number of geometry vertices.

unsigned int LeafNodeVisitor::get_ntotal_leaf_vertices(
   bool indices_stored_flag)
{ 
   ntotal_leaf_vertices=0;
   for (unsigned int l=0; l<get_n_leaf_geodes(); l++)
   {
      osg::observer_ptr<osg::Geode> curr_geode_obsptr=
         get_LeafGeodes_ptr()->at(l).first;
      ntotal_leaf_vertices += scenegraphfunc::get_n_geometry_vertices(
         scenegraphfunc::get_geometry(curr_geode_obsptr.get()),
         indices_stored_flag);
   } 
   return ntotal_leaf_vertices;
}

// ------------------------------------------------------------------------
// Member function apply(osg::Node) computes the total set of integer
// child indices which uniquely label the input node.  It then
// instantiates and inserts a corresponding Treenode into member
// object *tree_ptr.  Once the LeafNodeVisitor has finished traversing
// the entire scenegraph, *tree_ptr contains every node and can later
// be used for scenegraph display purposes.

void LeafNodeVisitor::apply(osg::Node& currNode) 
{ 
//   cout << "Inside LeafNodeVisitor::apply(osg::Node())" 
//        << " counter = " << node_counter++ 
//        << " class name = " << currNode.className()
//        << " Node name = " << currNode.getName()
//        << endl;

   MyNodeVisitor::apply(currNode);
} 

// ------------------------------------------------------------------------
void LeafNodeVisitor::apply(osg::Geode& currGeode) 
{ 
//   cout << "Inside LeafNodeVisitor::apply(osg::Geode)" 
//        << " counter=" << node_counter++ 
//        << " class name = " << currGeode.className()
//        << " Geode name = " << currGeode.getName() 
//        << endl;

// First retrieve and store current Matrix Transform relevant for
// current Geode:

   model::MyNodeInfo* curr_nodeinfo_ptr=
      dynamic_cast<model::MyNodeInfo*>(currGeode.getUserData() );
   if (curr_nodeinfo_ptr != NULL)
   {
      LocalToWorld=*(curr_nodeinfo_ptr->get_transform_ptr());
   }

   osg::observer_ptr<osg::Geode> geode_obsptr=&currGeode;
   pair<osg::observer_ptr<osg::Geode>, osg::Matrix> 
      p(geode_obsptr,LocalToWorld);
   Geodes_ptr->push_back(p);

   if (!is_approx_geode(currGeode)) LeafGeodes_ptr->push_back(p);

   MyNodeVisitor::apply(currGeode);
}

/*
// ------------------------------------------------------------------------
void LeafNodeVisitor::apply(osg::Group& currGroup) 
{ 
   cout << "Inside LeafNodeVisitor::apply(osg::Group)" 
        << " counter=" << node_counter++ 
        << " class name = " << currGroup.className()
//        << " Group name = " << currGroup.getName() 
        << endl;

   MyNodeVisitor::apply(currGroup);
} 

// ------------------------------------------------------------------------
void LeafNodeVisitor::apply(osg::LOD& currLOD) 
{ 
   cout << "Inside LeafNodeVisitor::apply(osg::LOD)" 
//        << " counter=" << node_counter++ 
        << " class name = " << currLOD.className()
//        << " LOD name = " << currLOD.getName() 
        << endl;

   MyNodeVisitor::apply(currLOD);
} 
*/

// ------------------------------------------------------------------------
// Member function is_approx_geode retrieves the parent node for input
// Geode *currGeode_ptr.  If *currGeode_ptr is the only child of the
// parent node, then it does not represent an approximation geode and
// this boolean method returns false.  If the parent has more than one
// child and *currGeode_ptr corresponds to the zeroth child, then we
// assume (as of June 2006) that it does represent an approximation
// geode.  This boolean method then returns true.

bool LeafNodeVisitor::is_approx_geode(osg::Geode& currGeode)
{ 
   vector<osg::Group*> ParentList=currGeode.getParents();

   if (ParentList.size() > 1)
   {
      cout << "Logic error in MyNodeVisitor::is_approx_geode()" << endl;
      cout << "As of Jun 2006, we assume that each Geode has exactly 1 parent"
           << endl;
      cout << "Yet ParentList.size() = " << ParentList.size() << endl;
      exit(-1);
   }

   for (unsigned int j=0; j<ParentList.size(); j++)
   {
      osg::Group* ParentGroup_ptr=ParentList[j];
      if (ParentGroup_ptr->getNumChildren()==1)
      {
         return false;
      }
      else
      {
         unsigned int index=ParentGroup_ptr->getChildIndex(&currGeode);
         if (index==0) return true;
      }
   } // loop over index j labeling Geode parents in ParentList
   return false;
}
