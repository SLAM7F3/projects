// ==========================================================================
// Header file for POWERPOINTPICKHANDLER class
// ==========================================================================
// Last modfied on 8/24/07
// ==========================================================================

#ifndef POWERPOINT_PICK_HANDLER_H
#define POWERPOINT_PICK_HANDLER_H

#include "osg/osgGeometry/BoxPickHandler.h"

class PowerPoint;
class PowerPointsGroup;
class WindowManager;

class PowerPointPickHandler : public BoxPickHandler
{

  public: 

   PowerPointPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PowerPointsGroup* ASG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// Set and get methods:

// PowerPoint generation, manipulation and annihiilation methods:

   bool instantiate_PowerPoint(double X,double Y);
   bool select_PowerPoint();

  protected:

   virtual ~PowerPointPickHandler();
   PowerPointsGroup* get_PowerPointsGroup_ptr();
   Linkedlist<PowerPoint*>* get_PowerPointlist_ptr();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);

  private:

   PowerPointsGroup* PowerPointsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // PowerPointPickHandler.h



