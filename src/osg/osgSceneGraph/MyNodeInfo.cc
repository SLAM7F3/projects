// ========================================================================
// MyNodeInfo class member function definitions.  Nearly identical to
// Ross Anderson's NodeInfoi class.
// ========================================================================
// Last updated on 7/9/06; 3/21/09
// ========================================================================

#include <iostream>
#include "osg/osgSceneGraph/MyNodeInfo.h"

using namespace model;
using std::cout;
using std::endl;


MyNodeInfo* model::getOrCreateMyInfoForNode( osg::Node& node )
{
   if ( !dynamic_cast<MyNodeInfo *>( node.getUserData() ) )
      node.setUserData( new MyNodeInfo );
	
   return dynamic_cast<MyNodeInfo *>( node.getUserData() );
}

bool MyNodeInfo::colormapNeedsUpdate( unsigned long i )
{
   if ( getColormapUpdateIndex() < i ) 
   {
      setColormapUpdateIndex(i);
      return true;
   }
	
   return false;
}

bool MyNodeInfo::filterNeedsUpdate( unsigned long i )
{
//   cout << "inside MyNodeInfo::filterNeedsUpdate(), i = " << i
//        << " _filterUpdateIndex = " << _filterUpdateIndex << endl;
   if ( getFilterUpdateIndex() < i ) 
   {
      setFilterUpdateIndex(i);
      return true;
   }
	
   return false;
}
