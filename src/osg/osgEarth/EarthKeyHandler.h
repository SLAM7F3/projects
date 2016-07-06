// ==========================================================================
// EarthKeyHandler header file 
// ==========================================================================
// Last modified on 1/3/07; 1/22/07; 2/16/07
// ==========================================================================

#ifndef EARTHSKEYHANDLER_H
#define EARTHSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"
#include "osg/osgEarth/EarthManipulator.h"

class Earth;
class ModeController;

class EarthKeyHandler : public GraphicalsKeyHandler
{
  public:

   EarthKeyHandler(Earth* E_ptr,osgGA::EarthManipulator* EM_ptr,
                   ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~EarthKeyHandler();

  private:

   Earth* Earth_ptr;
   osgGA::EarthManipulator* EarthManipulator_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
