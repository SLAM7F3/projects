// ==========================================================================
// Header file for ARMYSYMBOLPICKHANDLER class
// ==========================================================================
// Last modfied on 2/7/07; 6/16/07; 8/13/09
// ==========================================================================

#ifndef ARMYSYMBOL_PICK_HANDLER_H
#define ARMYSYMBOL_PICK_HANDLER_H

#include "osg/osgGeometry/BoxPickHandler.h"

class ArmySymbol;
class ArmySymbolsGroup;
class WindowManager;

class ArmySymbolPickHandler : public BoxPickHandler
{

  public: 

   ArmySymbolPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ArmySymbolsGroup* ASG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// Set and get methods:

// ArmySymbol generation, manipulation and annihiilation methods:

   bool instantiate_ArmySymbol(double X,double Y);
   bool select_ArmySymbol();

  protected:

   virtual ~ArmySymbolPickHandler();
   ArmySymbolsGroup* get_ArmySymbolsGroup_ptr();
   Linkedlist<ArmySymbol*>* get_ArmySymbollist_ptr();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);

  private:

   ArmySymbolsGroup* ArmySymbolsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // ArmySymbolPickHandler.h



