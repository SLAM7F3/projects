// ==========================================================================
// Header file for MODELPICKHANDLER class
// ==========================================================================
// Last modfied on 1/21/07; 6/16/07; 7/20/07; 9/1/08; 9/2/08
// ==========================================================================

#ifndef NEW_MODEL_PICK_HANDLER_H
#define NEW_MODEL_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

class ModeController;
class MODEL;
class MODELSGROUP;
class PickHandlerCallbacks;
class WindowManager;

class MODELPickHandler : public GeometricalPickHandler
{

  public: 

   MODELPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,MODELSGROUP* MG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr=NULL);

// Set & get methods:

   void set_PickHandlerCallbacks_ptr(PickHandlerCallbacks* PHCB_ptr);
   virtual float get_max_distance_to_Graphical();
   
// Model generation, manipulation and annihiilation methods:

   double select_Zplane_value();

  protected:

   virtual ~MODELPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool pick_3D_point(float X,float Y);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   double Zplane_value;
   MODELSGROUP* MODELSGROUP_ptr;
   PickHandlerCallbacks* PickHandlerCallbacks_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_Model();
   bool select_MODEL();
   bool select_MODEL(int selected_MODEL_ID);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // MODELPickHandler.h



