// ==========================================================================
// RayTraceKeyHandler header file 
// ==========================================================================
// Last modified on 10/30/06; 1/3/07
// ==========================================================================

#ifndef RAYTRACEKEYHANDLER_H
#define RAYTRACEKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include <string>
#include "osg/ModeController.h"
#include "osg/osg3D/RayTracer.h"

class RayTraceKeyHandler : public osgGA::GUIEventHandler
{
  public:

   RayTraceKeyHandler(ModeController* MC_ptr,RayTracer* RT_ptr);


   ModeController* const get_ModeController_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~RayTraceKeyHandler();

  private:

   ModeController* ModeController_ptr;
   RayTracer* RayTracer_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ModeController* const RayTraceKeyHandler::get_ModeController_ptr()
{
   return ModeController_ptr;
}

#endif // raytracekeyhandler.h
