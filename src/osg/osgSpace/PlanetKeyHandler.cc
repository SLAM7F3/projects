// ==========================================================================
// EarthKeyHandler class member function definitions
// ==========================================================================
// Last modified on 1/29/07
// ==========================================================================

#include <string>
#include "osg/osgSpace/PlanetsGroup.h"
#include "osg/osgSpace/PlanetKeyHandler.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PlanetKeyHandler::allocate_member_objects()
{
}

void PlanetKeyHandler::initialize_member_objects()
{
   PlanetsGroup_ptr=NULL;
}

PlanetKeyHandler::PlanetKeyHandler(
   PlanetsGroup* PG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PlanetsGroup_ptr=PG_ptr;
}

PlanetKeyHandler::~PlanetKeyHandler()
{
}

// ------------------------------------------------------
bool PlanetKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_EARTH)
      {
         switch ( ea.getKey() )
         {
            case 's' :
               PlanetsGroup_ptr->toggle_ambient_sunlight();
               return true;
         }
      } // mode = MANIPULATE_EARTH conditional         
   } // key down conditional
   
   return false;
}


