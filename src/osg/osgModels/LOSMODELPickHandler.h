// ==========================================================================
// Header file for LOSMODELPICKHANDLER class
// ==========================================================================
// Last modfied on 9/19/09
// ==========================================================================

#ifndef LOSMODEL_PICK_HANDLER_H
#define LOSMODEL_PICK_HANDLER_H

#include "osg/osgModels/MODELPickHandler.h"

class ModeController;
class LOSMODEL;
class LOSMODELSGROUP;
class WindowManager;

class LOSMODELPickHandler : public MODELPickHandler
{

  public: 

   LOSMODELPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,LOSMODELSGROUP* LMG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr=NULL);

// Set & get methods:

// Model generation, manipulation and annihiilation methods:

  protected:

   virtual ~LOSMODELPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   LOSMODELSGROUP* LOSMODELSGROUP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_LOSMODEL();
   bool select_LOSMODEL();
   bool select_LOSMODEL(int selected_LOSMODEL_ID);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // LOSMODELPickHandler.h



