// ==========================================================================
// ArmySymbolsKeyHandler header file 
// ==========================================================================
// Last modified on 9/12/06; 1/3/07
// ==========================================================================

#ifndef ARMYSYMBOLSKEYHANDLER_H
#define ARMYSYMBOLSKEYHANDLER_H

#include "osg/osgGeometry/BoxesKeyHandler.h"

class ArmySymbolsGroup;
class ModeController;

class ArmySymbolsKeyHandler : public BoxesKeyHandler
{
  public:

   ArmySymbolsKeyHandler(ArmySymbolsGroup* ASG_ptr,ModeController* MC_ptr);

   ArmySymbolsGroup* const get_ArmySymbolsGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~ArmySymbolsKeyHandler();

  private:

   ArmySymbolsGroup* ArmySymbolsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
