// ==========================================================================
// MYINTERSECTVISITOR class member function definitions
// ==========================================================================
// Last modified on 11/11/06; 11/12/06; 11/13/06
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LOD>
#include <osg/PagedLOD>
#include <osg/Transform>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/DatabasePager>
#include <osgDB/ReadFile>

#include <model/Metadata.h>
#include "math/basic_math.h"
#include "math/constants.h"
#include "osg/osgSceneGraph/MyIntersectVisitor.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MyIntersectVisitor::allocate_member_objects()
{
}

void MyIntersectVisitor::initialize_member_objects()
{
   _pager=NULL;

   setLODSelectionMode(
      CustomIntersectVisitor::USE_HIGHEST_LEVEL_OF_DETAIL);

   tree_ptr=NULL;
}

MyIntersectVisitor::MyIntersectVisitor()
{
   allocate_member_objects();
   initialize_member_objects();
}

MyIntersectVisitor::MyIntersectVisitor(Tree<data_type>* treeptr)
{
   allocate_member_objects();
   initialize_member_objects();
   tree_ptr=treeptr;
}

MyIntersectVisitor::MyIntersectVisitor(
   osgDB::DatabasePager* DBP_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   _pager=DBP_ptr;
}

MyIntersectVisitor::~MyIntersectVisitor()
{
}

// ========================================================================
// Traversal member functions
// ========================================================================

void MyIntersectVisitor::apply(osg::Node& currNode) 
{ 
//   cout << "Inside MyIntersectVisitor::apply(Node)" << endl;
//   cout << "classname = " << currNode.className() << endl;

   reconstruct_total_indices();
   traversal_history.push_back(total_indices);
//   cout << "node ID = " << tree_ptr->get_ID(total_indices) 
//        << endl;

   CustomIntersectVisitor::apply(currNode);
}

/*
// ------------------------------------------------------------------------
void MyIntersectVisitor::apply(osg::Group& currGroup) 
{ 
   cout << "Inside MyIntersectVisitor::apply(Group)" << endl;
   cout << "classname = " << currGroup.className() << endl;

   reconstruct_total_indices();
   traversal_history.push_back(total_indices);
   cout << "Group ID = " << tree_ptr->get_ID(total_indices) << endl;

   CustomIntersectVisitor::apply(currGroup);
}

// ------------------------------------------------------------------------
void MyIntersectVisitor::apply(osg::MatrixTransform& MT) 
{ 
   cout << "Inside MyIntersectVisitor::apply(MatrixTransform)" << endl;
   cout << "classname = " << MT.className() << endl;

   reconstruct_total_indices();
   traversal_history.push_back(total_indices);
   cout << "MT ID = " << tree_ptr->get_ID(total_indices) << endl;

   CustomIntersectVisitor::apply(MT);
}

// ------------------------------------------------------------------------
void MyIntersectVisitor::apply(osg::Geode& currGeode) 
{ 
   cout << "Inside MyIntersectVisitor::apply(Geode)" << endl;
//   cout << "Geode = " << &currGeode << endl;
   cout << "classname = " << currGeode.className() << endl;

   reconstruct_total_indices();
   traversal_history.push_back(total_indices);
   cout << "Geode ID = " << tree_ptr->get_ID(total_indices) << endl;

   vector<osg::Geometry*> geometries=scenegraphfunc::
      get_geometries(&currGeode);
//   cout << "geometries.size() = " << geometries.size() << endl;

   osg::Geometry* curr_geom_ptr=scenegraphfunc::get_geometry(&currGeode);
//   cout << "curr_geom_ptr = " << curr_geom_ptr << endl;
   if (curr_geom_ptr != NULL)
   {
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_geom_ptr->getVertexArray());
//      cout << "curr_vertices_ptr = " << curr_vertices_ptr << endl;
//      cout << "# vertices = " << curr_vertices_ptr->size() << endl;
   }
   
   CustomIntersectVisitor::apply(currGeode);
}
*/

/*
// ------------------------------------------------------------------------
void MyIntersectVisitor::apply(osg::LOD& node)
{
   cout << "inside MyIntersectVisitor::apply(LOD), LOD_name = " 
        << node.getName() << endl;

   reconstruct_total_indices();
   traversal_history.push_back(total_indices);
   cout << "LOD ID = " << tree_ptr->get_ID(total_indices) << endl;

   CustomIntersectVisitor::apply(node);
*/

/*
   const osg::LOD::RangeList& rangeList = node.getRangeList();
   float range = 0;

   // visit the highest resolution child
   for(unsigned int i=0; i<rangeList.size(); i++)
   {
      range = osg::maximum( range, rangeList[i].first );
      cout << "i = " << i  << " rangeList[i].first = " << rangeList[i].first
           << " range = " << range << endl;
   }
   
   unsigned int numChildren = node.getNumChildren();
   cout << "numChildren = " << numChildren << endl;
   
   if ( rangeList.size() < numChildren) numChildren = rangeList.size();

   for(unsigned int i=0; i<numChildren; i++)
   {    
      cout << "i = " << i << endl;
      if ( rangeList[i].first<=range && range<rangeList[i].second)
         node.getChild(i)->accept(*this);
   }
*/

/*
}

// ------------------------------------------------------------------------
void MyIntersectVisitor::apply(osg::PagedLOD& node)
{
   cout << "inside MyIntersectVisitor::apply, PagedLOD_name = " 
        << node.getName() << endl;

   reconstruct_total_indices();
   traversal_history.push_back(total_indices);
   cout << "PagedLOD ID = " << tree_ptr->get_ID(total_indices) << endl;

   CustomIntersectVisitor::apply(node);
*/

/*
   const osg::PagedLOD::RangeList& rangeList = node.getRangeList();
   float range = 0;

   // visit the highest resolution child

   for(unsigned int i=0; i<rangeList.size(); i++)
      range = osg::maximum( range, rangeList[i].first );
	
   unsigned int numRanges = rangeList.size(); 
   // = total number of pages in and out of memory

   unsigned int numChildren = node.getNumChildren();
   // = number of pages currently residing in memory

   cout << "numRanges = " << numRanges 
        << " numChildren = " << numChildren << endl;

   if (numChildren > 0)
   {
      osg::Node* lodChild = NULL;
      lodChild=node.getChild(numChildren-1);

      if (!enterNode(*lodChild)) return;
      traverse(*lodChild);
      leaveNode();
//      lodChild->accept(*this);
   }
   
*/


/*
   for (unsigned int i=0; i<numRanges; i++)
   {    
      if ( rangeList[i].first<=range && range<rangeList[i].second) 
      {
         osg::ref_ptr<osg::Node> lodChild = NULL;
			
         if ( i < numChildren ) 
         {

// Page already in memory.  So just visit it:

//            lodChild = node.getChild(i);

            lodChild = node.getChild(i);
            lodChild->accept(*this);
         } 
         else 
         {

// Page not already in memory.  Need to load it into scene graph,
// visit it and then unload it from scene graph:

            paged_filename = node.getDatabasePath() + node.getFileName(i);
            lodChild = osgDB::readNodeFile(paged_filename);
				
// Apply standard pager treatment, as if we loaded it in the viewer:

            if ( lodChild.valid() && _pager.valid() ) 
            {
               _pager->registerPagedLODs( lodChild.get() );
//               cout << "Paged in filename = " << paged_filename << endl;
            }

            node.addChild(lodChild.get());
            lodChild->accept(*this);
//            node.removeChild(lodChild.get());
         }
      }
   } // loop over index i labeling ranges



}

*/
