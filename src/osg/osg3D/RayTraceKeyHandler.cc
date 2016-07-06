// ==========================================================================
// RayTraceKeyHandler class member function definitions
// ==========================================================================
// Last modified on 10/30/06
// ==========================================================================

#include "osg/osg3D/RayTraceKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RayTraceKeyHandler::allocate_member_objects()
{
}

void RayTraceKeyHandler::initialize_member_objects()
{
   ModeController_ptr=NULL;
   RayTracer_ptr=NULL;
}

RayTraceKeyHandler::RayTraceKeyHandler(
   ModeController* MC_ptr,RayTracer* RT_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   RayTracer_ptr=RT_ptr;
}

RayTraceKeyHandler::~RayTraceKeyHandler()
{
}

// ------------------------------------------------------
bool RayTraceKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_LINE)
      {
         if (ea.getKey()=='t')
         {
            RayTracer_ptr->trace_ray();
            return true;
         }
      } // Mode conditional
   } // key down conditional
   
   return false;
}


