// ==========================================================================
// Header file for OBSFRUSTUMPICKHANDLER class
// ==========================================================================
// Last modfied on 9/12/07; 9/22/07; 9/2/08
// ==========================================================================

#ifndef OBSFRUSTUM_PICK_HANDLER_H
#define OBSFRUSTUM_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

// class osg::Group;
class ModeController;
class ObsFrustum;
class ObsFrustaGroup;
class WindowManager;

class ObsFrustumPickHandler : public GeometricalPickHandler
{

  public: 

   ObsFrustumPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ObsFrustaGroup* OFG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// ObsFrustum generation, manipulation and annihilation methods:


  protected:

   virtual ~ObsFrustumPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   int rotate_about_curr_eyept_counter;
   ObsFrustaGroup* ObsFrustaGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

//    bool instantiate_ObsFrustum(double X,double Y);
   bool select_ObsFrustum();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // ObsFrustumPickHandler.h



