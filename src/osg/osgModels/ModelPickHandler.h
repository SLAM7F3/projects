// ==========================================================================
// Header file for MODELPICKHANDLER class
// ==========================================================================
// Last modfied on 12/29/06; 1/3/07; 1/21/07; 6/16/07; 9/2/08
// ==========================================================================

#ifndef MODEL_PICK_HANDLER_H
#define MODEL_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

class ModeController;
class Model;
class ModelsGroup;
class WindowManager;

class ModelPickHandler : public GeometricalPickHandler
{

  public: 

   ModelPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ModelsGroup* MG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr=NULL);

// Model generation, manipulation and annihiilation methods:

   double select_Zplane_value();

  protected:

   virtual ~ModelPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool pick_3D_point(float X,float Y);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   double Zplane_value;
   ModelsGroup* ModelsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_Model();
   bool select_Model();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // ModelPickHandler.h



