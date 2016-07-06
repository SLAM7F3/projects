// ==========================================================================
// PathFinderKeyHandler header file 
// ==========================================================================
// Last modified on 12/2/10
// ==========================================================================

#ifndef PATHFINDERKEYHANDLER_H
#define PATHFINDERKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include <string>
#include "osg/ModeController.h"
#include "osg/osgAnnotators/PathFinder.h"

class PathFinderKeyHandler : public osgGA::GUIEventHandler
{
  public:

   PathFinderKeyHandler(ModeController* MC_ptr,PathFinder* RT_ptr);


   ModeController* const get_ModeController_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~PathFinderKeyHandler();

  private:

   ModeController* ModeController_ptr;
   PathFinder* PathFinder_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ModeController* const PathFinderKeyHandler::get_ModeController_ptr()
{
   return ModeController_ptr;
}

#endif // PathFinderKeyHandler.h
