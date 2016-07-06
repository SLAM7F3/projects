// ==========================================================================
// PlanesKeyHandler header file 
// ==========================================================================
// Last modified on 10/30/06; 11/24/06; 12/26/06; 1/3/07
// ==========================================================================

#ifndef PLANESKEYHANDLER_H
#define PLANESKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include <string>
#include "osg/ModeController.h"
#include "osg/osgGeometry/PlanesGroup.h"

class PlanesKeyHandler : public osgGA::GUIEventHandler
{
  public:

   PlanesKeyHandler(PlanesGroup* PG_ptr,ModeController* MC_ptr);

   ModeController* const get_ModeController_ptr();
   PlanesGroup* const get_PlanesGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~PlanesKeyHandler();

  private:

   ModeController* ModeController_ptr;
   PlanesGroup* PlanesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ModeController* const PlanesKeyHandler::get_ModeController_ptr()
{
   return ModeController_ptr;
}

// ---------------------------------------------------------------------
inline PlanesGroup* const PlanesKeyHandler::get_PlanesGroup_ptr()
{
   return PlanesGroup_ptr;
}

#endif // planeskeyhandler.h
