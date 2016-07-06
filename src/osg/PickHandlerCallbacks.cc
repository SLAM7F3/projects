// ==========================================================================
// Purely virtual PickHandlerCallbacks class 
// ==========================================================================
// Last modified on 4/30/08
// ==========================================================================

#include <iostream>
#include "osg/PickHandlerCallbacks.h"

using std::cout;
using std::endl;

void PickHandlerCallbacks::allocate_member_objects()
{
}		       

void PickHandlerCallbacks::initialize_member_objects()
{
}		       

PickHandlerCallbacks::PickHandlerCallbacks()
{
   allocate_member_objects();
   initialize_member_objects();
}

PickHandlerCallbacks::~PickHandlerCallbacks() 
{
}
