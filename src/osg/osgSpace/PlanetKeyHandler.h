// ==========================================================================
// PlanetKeyHandler header file 
// ==========================================================================
// Last modified on 1/29/07
// ==========================================================================

#ifndef PLANETSKEYHANDLER_H
#define PLANETSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class Planet;
class ModeController;

class PlanetKeyHandler : public GraphicalsKeyHandler
{
  public:

   PlanetKeyHandler(PlanetsGroup* PG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~PlanetKeyHandler();

  private:

   PlanetsGroup* PlanetsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
