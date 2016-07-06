// ========================================================================
// Ross' UpdateMyHyperFilterCallback class member function definitions
// ========================================================================
// Last updated on 11/27/11; 12/2/11; 12/29/11
// ========================================================================

#include <iostream>
#include <osg/Geode>
#include "osg/osgSceneGraph/UpdateMyHyperFilterCallback.h"

using std::cout;
using std::endl;

namespace model
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void UpdateMyHyperFilterCallback::allocate_member_objects()
   {
   }

   void UpdateMyHyperFilterCallback::initialize_member_objects()
   {
      Callback_name="UpdateMyHyperFilterCallback";
   }
		
   UpdateMyHyperFilterCallback::UpdateMyHyperFilterCallback(
      MyHyperFilter* hf_ptr) 
   { 
//      cout << "inside UpdateMyHyperFilterCallback constructor(), this = " 
//           << this << endl;
      allocate_member_objects();
      initialize_member_objects();
   
      MyHyperFilter_ptr=hf_ptr;
   }

   UpdateMyHyperFilterCallback::~UpdateMyHyperFilterCallback()
   {
   }

// ------------------------------------------------------------------------
   void UpdateMyHyperFilterCallback::operator()(
      osg::Node* node, osg::NodeVisitor* nv)
   {
//      cout << "inside UpdateMyHyperFilterCallback::operator()" << endl;
//      cout << "node = " << node << " nv = " << nv << endl;

      if (node==NULL)
      {
         traverse(node,nv);
         return;
      }

      osg::Geode* geode_ptr = dynamic_cast<osg::Geode*>(node);

      if ( nv )
      {
         MyHyperFilter_ptr->update( *geode_ptr, nv->getNodePath() );
      }
   
      traverse(node,nv);
   }

} // namespace model
