// ==========================================================================
// Header file for ROI_POLYHEDRONPICKHANDLER class
// ==========================================================================
// Last modfied on 12/21/09
// ==========================================================================

#ifndef ROI_POLYHEDRON_PICK_HANDLER_H
#define ROI_POLYHEDRON_PICK_HANDLER_H

#include "osg/osgGeometry/PolyhedronPickHandler.h"

class ROI_Polyhedron;
class ROI_PolyhedraGroup;
class ModeController;
class WindowManager;

class ROI_PolyhedronPickHandler : public PolyhedronPickHandler
{

  public: 

   ROI_PolyhedronPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      ROI_PolyhedraGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);

// Set and get methods:

// ROI_Polyhedron generation, manipulation and annihilation methods:

  protected:

   virtual ~ROI_PolyhedronPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   int curr_ROI_Polyhedron_ID;
   ROI_PolyhedraGroup* ROI_PolyhedraGroup_ptr;
   ROI_Polyhedron* curr_ROI_Polyhedron_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_ROI_Polyhedron(double X,double Y);
   bool select_ROI_Polyhedron();
   bool select_ROI_Polyhedron(int select_ROI_Polyhedron_ID);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif // ROI_PolyhedronPickHandler.h



