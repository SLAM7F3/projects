// ==========================================================================
// PolygonsKeyHandler class member function definitions.  
// ==========================================================================
// Last modified on 2/18/08
// ==========================================================================

#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolygonsKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void PolygonsKeyHandler::allocate_member_objects()
      {
      }

   void PolygonsKeyHandler::initialize_member_objects()
      {
         PolygonsGroup_ptr=NULL;
      }

   PolygonsKeyHandler::PolygonsKeyHandler(
      PolygonsGroup* PG_ptr,ModeController* MC_ptr):
      GraphicalsKeyHandler(PG_ptr,MC_ptr)
      {
         allocate_member_objects();
         initialize_member_objects();
         PolygonsGroup_ptr=PG_ptr;
      }

   PolygonsKeyHandler::~PolygonsKeyHandler()
      {
      }

// ------------------------------------------------------
   bool PolygonsKeyHandler::handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
      {
         if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
         {
            if (get_ModeController_ptr()->getState()==
                ModeController::MANIPULATE_POLYGON)
            {
               if (ea.getKey()=='t')
               {
                  PolygonsGroup_ptr->toggle_OSGgroup_nodemask();
                  return true;
               }

// Press "Delete" key to completely destroy a Polygon:

               else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
               {
                  PolygonsGroup_ptr->destroy_Polygon();
                  return true;
               }

            } // mode = MANIPULATE_POLYGON conditional

         } // key down conditional
   
         return false;
      }


} // osgGeometry namespace
