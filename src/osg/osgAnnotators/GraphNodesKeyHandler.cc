// ==========================================================================
// GraphNodesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 11/10/06; 12/11/06
// ==========================================================================

#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GraphNodesKeyHandler::allocate_member_objects()
{
}

void GraphNodesKeyHandler::initialize_member_objects()
{
   GraphNodesGroup_ptr=NULL;
}

GraphNodesKeyHandler::GraphNodesKeyHandler(
   GraphNodesGroup* GNG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(GNG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   GraphNodesGroup_ptr=GNG_ptr;
}

GraphNodesKeyHandler::~GraphNodesKeyHandler()
{
}

// ------------------------------------------------------
bool GraphNodesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_GRAPHNODE)
      {

// Press "Delete" key to completely destroy a GraphNode:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            GraphNodesGroup_ptr->destroy_GraphNode();
            return true;
         }

      } // mode = MANIPULATE_GRAPHNODE conditional         
      
   } // key down conditional
   
   return false;
}


