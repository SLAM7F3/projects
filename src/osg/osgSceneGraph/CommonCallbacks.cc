// ==========================================================================
// COMMONCALLBACKS class member function definitions
// ==========================================================================
// Last modified on 12/2/11
// ==========================================================================

#include <iostream>
#include "osg/osgSceneGraph/CommonCallbacks.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CommonCallbacks::allocate_member_objects()
{
//   cout << "inside CommonCallbacks::allocate_member_objects()" << endl;
   
   commonUpdateCallback_refptr = new osg::NodeCallback;
   commonCullCallback_refptr = new osg::NodeCallback;
}

void CommonCallbacks::initialize_member_objects()
{
}

CommonCallbacks::CommonCallbacks()
{
//   cout << "inside CommonCallbacks() constructor" << endl;
   
   allocate_member_objects();
   initialize_member_objects();
}

CommonCallbacks::~CommonCallbacks()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

osg::NodeCallback* CommonCallbacks::get_CommonUpdateCallback_ptr()
{
   return commonUpdateCallback_refptr.get();
}

const osg::NodeCallback* CommonCallbacks::get_CommonUpdateCallback_ptr() const
{
   return commonUpdateCallback_refptr.get();
}

osg::NodeCallback* CommonCallbacks::get_CommonCullCallback_ptr()
{
   return commonCullCallback_refptr.get();
}

const osg::NodeCallback* CommonCallbacks::get_CommonCullCallback_ptr() const
{
   return commonCullCallback_refptr.get();
}
