// ==========================================================================
// MYDATABASEPAGER class member function definitions
// ==========================================================================
// Last modified on 3/23/09; 5/28/09; 12/2/11
// ==========================================================================

#include <iostream>
#include <vector>
#include <osg/Node>
#include <osg/NodeCallback>
// #include <osg/Notify>
#include "osg/osgSceneGraph/MyDatabasePager.h"

#include "osg/osgfuncs.h"

using namespace viewer;

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MyDatabasePager::allocate_member_objects()
{
   ParentVisitor_refptr=new ParentVisitor();

// Trump OSG Producer viewer's default database pager with
// MyDatabasePager by creating a singleton registry object and setting
// its DatabasePager pointer:

// Ross says move this registry stuff into main rather than here...

   Registry_refptr=osgDB::Registry::instance(); 
					// *Registry_ptr = singleton object
   Registry_refptr->setDatabasePager(this);
}

void MyDatabasePager::initialize_member_objects()
{

// Use Ross Anderson's empirically determined parameters to speed up
// file paging performance:

   setExpiryDelay(3);	// For scenegraph update display
//   setExpiryDelay(10.0);	// RA 
//   setExpiryDelay(20.0);	// ISDS3D
//   setExpiryDelay(50.0);	

   setUseFrameBlock(false);	// ISDS3D

// Next flag criticially influences whether flythrough is smooth or
// not.  If DoPreCompile==true, approx geodes are retained for much
// longer and flythrough is noticeably smoother than if
// DoPreCompile==false where all leaf geodes are brought in and
// flythrough really stutters on laptop.

   setDoPreCompile(true);	// RA, ISDS3D
//   setDoPreCompile(false);

//   setTargetFrameRate( 10.0 );
   setTargetFrameRate( 15.0 );   // RA   leads to stutter 
//   setTargetFrameRate( 22.0 );    // medium slow paging, but some stuttering
//   setTargetFrameRate( 25.0 );    // medium slow paging, but some stuttering
//   setTargetFrameRate( 30.0 );    // medium slow paging, but some stuttering
//   setTargetFrameRate( 50.0 );    // medium slow paging, but some stuttering
//   setTargetFrameRate( 150.0 );    // slow paging, but less stuttering
//   setTargetFrameRate( 300.0 );    // slow paging, but less stuttering
//   setTargetFrameRate( 900.0 );    // slow paging, but less stuttering ISDS3D

//   setMinimumTimeAvailableForGLCompileAndDeletePerFrame( 0.025 );
   setMinimumTimeAvailableForGLCompileAndDeletePerFrame( 0.5 );	// RA, ISDS3D
   setDrawablePolicy( osgDB::DatabasePager::USE_VERTEX_ARRAYS ); // ISDS3D
}

MyDatabasePager::MyDatabasePager():
   DatabasePager()
{
   allocate_member_objects();
   initialize_member_objects();
}

MyDatabasePager::MyDatabasePager(MyNodeVisitor* nv):
   DatabasePager()
{
   allocate_member_objects();
   initialize_member_objects();
   _setupVisitor_refptr=nv;
}

MyDatabasePager::MyDatabasePager(MyNodeVisitor* nv,ColorGeodeVisitor* nv2):
   DatabasePager()
{
//   cout << "inside MyDatabasePager constructor #3" << endl;

   allocate_member_objects();
   initialize_member_objects();
   _setupVisitor_refptr=nv;
   ColorGeodeVisitor_refptr=nv2;
}

MyDatabasePager::MyDatabasePager(MyNodeVisitor* nv,ColorGeodeVisitor* nv2,
                                 MyNodeVisitor* nv3,osg::Node* DN_ptr):
   DatabasePager()
{
   allocate_member_objects();
   initialize_member_objects();
   _setupVisitor_refptr=nv;
   ColorGeodeVisitor_refptr=nv2;
   _setupVisitor3_refptr=nv3;
   DataNode_refptr=DN_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function updateSceneGraph first checks whether any new pages
// need to be loaded into memory or whether any stale pages need to be
// removed from memory.  It then forces all newly loaded nodes to
// accept the SetupVisitor.  

void MyDatabasePager::updateSceneGraph(double currentFrameTime)
{
//   cout << "inside MyDataBasePager::updateSceneGraph(), currframetime = " 
//        << currentFrameTime << endl;

   removeExpiredSubgraphs(currentFrameTime);

// Save pointers to new, paged-in nodes within member STL vector
// PagedNode:

   PagedNode.clear();
   if ( _setupVisitor_refptr.valid() ) 
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_dataToMergeListMutex);
		
      for(DatabaseRequestList::iterator itr=_dataToMergeList.begin();
          itr!=_dataToMergeList.end(); ++itr)
      {
//         DatabaseRequest* databaseRequest = itr->get();
			
         osg::observer_ptr<osg::Node> loadedNode_obsptr = 
            itr->get()->_loadedModel.get();
         PagedNode.push_back(loadedNode_obsptr);
      }
   }
   
   addLoadedDataToSceneGraph(currentFrameTime);

// Loop over all paged-in nodes, and apply the Setup visitor to each one:

   for (unsigned int i=0; i<PagedNode.size(); i++)
   {
      ParentVisitor_refptr->reset_matrix_transform();
      ParentVisitor_refptr->set_pointcloud_flag(false);
      PagedNode.at(i)->accept(*(ParentVisitor_refptr.get()));
      bool pointcloud_flag=ParentVisitor_refptr->get_pointcloud_flag();
//      cout << "pointcloud_flag = " << pointcloud_flag << endl;

      if (pointcloud_flag)
      {

// SetupGeomVisitor:

         if (_setupVisitor_refptr.valid())
         {
            _setupVisitor_refptr->set_LocalToWorld(
               ParentVisitor_refptr->get_LocalToWorld());
            PagedNode.at(i)->accept( *_setupVisitor_refptr );
         }

// ColorGeodeVisitor:
      
         if (ColorGeodeVisitor_refptr.valid())
         {

// For point coloring purposes, we do NOT want to set LocalToWorld
// matrix equal to entire cumulative MatrixTransform computed from
// PagedNode all the way up to top of scenegraph.  Instead, we just
// want to use the MatrixTransform embedded at the top of the
// datagraph for point cloud coloring and ignore any higher matrix
// transforms in the total scenegraph that reposition the point cloud
// onto the surface of the earth.  So we set LocalToWorld equal to the
// bottommost MatrixTransform which is first encountered by the
// ParentVisitor as it ascends the scenegraph from the PagedNode:

            ColorGeodeVisitor_refptr->set_LocalToWorld(
               ParentVisitor_refptr->get_bottom_Matrix());

            ColorGeodeVisitor_refptr->get_height_ColorMap_ptr()->
               setEnabled(*(PagedNode.at(i)),true);
            ColorGeodeVisitor_refptr->get_prob_ColorMap_ptr()->
               setEnabled(*(PagedNode.at(i)),true);

            PagedNode.at(i)->accept( *ColorGeodeVisitor_refptr );
         }
      } // pointcloud_flag conditional

      if (_setupVisitor3_refptr.valid())
      {
         DataNode_refptr->accept( *_setupVisitor3_refptr );
      }

//      cout << "pn index = " << i 
//           << " PagedNode[pn] = " << PagedNode[i].get() << endl;
//      osg::NodePath CurrPath(ParentVisitor_refptr->get_node_path());
//      for (unsigned int j=0; j<CurrPath.size(); j++)
//      {
//         cout << "j = " << j << " Node = " << CurrPath[j]
//              << " classname = " << CurrPath[j]->className() << endl;
//      }
      
   } // loop over index i labeling nodes paged in
}

/*
// --------------------------------------------------------------------------
void MyDatabasePager::registerPagedLODs(osg::Node* subgraph) 
{
   if (requiresUpdateSceneGraph()) scenegraph_updated_flag=true;
   DatabasePager::registerPagedLODs(subgraph);

   applySetupVisitor(*subgraph);

   if (scenegraph_updated_flag)
   {
//      DataGraph_ptr->UpdateCurrGeodeInfo();
   }

   scenegraph_updated_flag=false;   
}
*/
