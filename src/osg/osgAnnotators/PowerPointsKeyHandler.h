// ==========================================================================
// PowerPointsKeyHandler header file 
// ==========================================================================
// Last modified on 8/24/07
// ==========================================================================

#ifndef POWERPOINTSKEYHANDLER_H
#define POWERPOINTSKEYHANDLER_H

#include "osg/osgGeometry/BoxesKeyHandler.h"

class PowerPointsGroup;
class ModeController;

class PowerPointsKeyHandler : public BoxesKeyHandler
{
  public:

   PowerPointsKeyHandler(PowerPointsGroup* PPG_ptr,ModeController* MC_ptr);

   PowerPointsGroup* const get_PowerPointsGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~PowerPointsKeyHandler();

  private:

   PowerPointsGroup* PowerPointsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
