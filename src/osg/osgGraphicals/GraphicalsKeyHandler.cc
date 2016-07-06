// ==========================================================================
// GraphicalsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 11/6/05; 12/7/05
// ==========================================================================

#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgGraphicals/GraphicalsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GraphicalsKeyHandler::allocate_member_objects()
{
}

void GraphicalsKeyHandler::initialize_member_objects()
{
   GraphicalsGroup_ptr=NULL;
}

GraphicalsKeyHandler::GraphicalsKeyHandler(ModeController* MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
}

GraphicalsKeyHandler::GraphicalsKeyHandler(
   GraphicalsGroup* GG_ptr,ModeController* MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   GraphicalsGroup_ptr=GG_ptr;
   ModeController_ptr=MC_ptr;
}

GraphicalsKeyHandler::~GraphicalsKeyHandler()
{
}

// ---------------------------------------------------------------------
ModeController* const GraphicalsKeyHandler::get_ModeController_ptr()
{
   return ModeController_ptr;
}

// ---------------------------------------------------------------------
GraphicalsGroup* const GraphicalsKeyHandler::get_GraphicalsGroup_ptr()
{
   return GraphicalsGroup_ptr;
}

