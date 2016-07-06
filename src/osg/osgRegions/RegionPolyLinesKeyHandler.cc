// ==========================================================================
// RegionPolyLinesKeyHandler class member function definitions.  
// ==========================================================================
// Last modified on 12/13/08; 4/9/11
// ==========================================================================

#include "osg/osgRegions/RegionPolyLinesKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RegionPolyLinesKeyHandler::allocate_member_objects()
{
}

void RegionPolyLinesKeyHandler::initialize_member_objects()
{
}

RegionPolyLinesKeyHandler::RegionPolyLinesKeyHandler(
   PolyLinesGroup* PLG_ptr,ModeController* MC_ptr):
   PolyLinesKeyHandler(PLG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
}

RegionPolyLinesKeyHandler::~RegionPolyLinesKeyHandler()
{
}

// ------------------------------------------------------
bool RegionPolyLinesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& gaa)
{
   return PolyLinesKeyHandler::handle(ea,gaa);
}


