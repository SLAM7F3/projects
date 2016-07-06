// ==========================================================================
// GridKeyHandler header file 
// ==========================================================================
// Last modified on 3/27/06; 1/3/07
// ==========================================================================

#ifndef GRIDKEYHANDLER_H
#define GRIDKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include "osg/ModeController.h"

class Grid;

class GridKeyHandler : public osgGA::GUIEventHandler
{
  public:

   GridKeyHandler(ModeController* MC_ptr,Grid* G_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~GridKeyHandler();

   Grid* grid_ptr;
   ModeController* ModeController_ptr;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
