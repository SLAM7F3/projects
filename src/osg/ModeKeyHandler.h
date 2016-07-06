// ==========================================================================
// ModeKeyHandler header file 
// ==========================================================================
// Last modified on 9/30/05
// ==========================================================================

#ifndef MODEKEYHANDLER_H
#define MODEKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include "osg/ModeController.h"

class ModeKeyHandler : public osgGA::GUIEventHandler
{
  public:

// keys control the operation mode's state through ModeController

   ModeKeyHandler(ModeController* p_controller);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

  private:

   ModeController* m_controller; 
   void allocate_member_objects();
   void initialize_member_objects();
}; 


#endif 
