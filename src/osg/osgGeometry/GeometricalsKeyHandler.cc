// ==========================================================================
// GeometricalsKeyHandler class member function definitions.  
// ==========================================================================
// Last modified on 5/22/11
// ==========================================================================

#include <string>
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osgGeometry/GeometricalsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GeometricalsKeyHandler::allocate_member_objects()
{
}

void GeometricalsKeyHandler::initialize_member_objects()
{
   Allow_Insertion_flag=Allow_Manipulation_flag=true;
   GeometricalsGroup_ptr=NULL;
}

GeometricalsKeyHandler::GeometricalsKeyHandler(
   GeometricalsGroup* GG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(GG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   GeometricalsGroup_ptr=GG_ptr;
}

GeometricalsKeyHandler::~GeometricalsKeyHandler()
{
}


